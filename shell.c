#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#define DELIMETERS " \t"            // used by strtok, skips whitespace - spaces and tabs
#define BUFSZ 100                   // max size of input line containing commands
#define CMDWORDS 10                 // max words on command line
static char line[BUFSZ];            // cmd line read into line[]
static char *cmd_words[CMDWORDS];   // words on line pointed to by cmd_words[]
                                    // addresses in cmd_words are &line[6] for example
static int num_words = 0;           // number of words on cmd line
static int symbol = 0;              // all four of these are for handeling IO redirect, pipes, and background programs
static int insymbol = 0;
static int pipesymbol = 0;
static int backgroundsymbol = 0;



/*
 * signal handler for CTL-C
 * CTL-C does not terminate shell, but it does terminate the foreground process. 
 * (Use kill to get rid of procs in the background.)
 */
void signal_handler(int siginput) {
    if (siginput == SIGINT) {
        fprintf(stdout, "signal_handler caught CTL-C \n");
    }
}

/*
 * Called when execvp() returns, which happens when the cmd does not exist
 * Prints error message and exits - the exit is from the child, not the shell
 */
void cmd_not_found(char *cmd) {
    fprintf(stderr, "mysh: cmd not found: %s\n", cmd);
    exit(1);                            // exit() from child, not shell
}

/*
 * Checks to see if cmd is cd, and changes directory
 * Returns 1 if the cmd is cd; otherwise returns 0
 */
int cd_cmd() {
    if(line[0] == 'c' && line[1] == 'd' && line[2] == ' '){ // Clumsy 
        if(chdir(line+3) < 0) // Chdir has no effect on the parent if run in the child.
            fprintf(stderr, "cannot cd %s\n", line+3);
        return 1;
    }
    return 0;
}

/*
 * Separates line into words on line
 * For each word on line, cmd_words[] points to the word.
 * num_words assigned the number of words on the line
 * returns 0 for line without command
 */
int get_cmd_words() {
    // Collect words on line into cmd_words
    num_words = 0;
    char *p;
    p = strtok(line, DELIMETERS);               // strtok() returns pointer to word on line
    while (p != NULL) {                         // p has address in line, e.g., &line[0]
        cmd_words[num_words] = p;               // cmd_words[] points to words on line
        num_words++;                            // count words on line
        p = strtok(NULL, DELIMETERS);           // get next word on line
    }
    cmd_words[num_words] = NULL;                // 0 marks end of words in cmd_words
    return num_words;                           // return num of words found
}

int main() {
    int num = 1, backgroundrun = 0;             // backgroundrun set to 1 after % ./loop &
    while(1) {
        signal(SIGINT, signal_handler);
        memset(line, 0, BUFSZ);                 // Zero line before each use
        fprintf(stdout, "mysh%d %% ", num);     // display prompt
        fflush(stdout);                         // flush prompt to terminal
        if (fgets(line, BUFSZ, stdin) == 0) {   // CTL-D terminates shell
            printf("\n");
            break;                              // fgets returns LF at end of string
        }
        line[strcspn(line, "\n")] = '\0';       // trim lf from line
        if (cd_cmd() || !get_cmd_words())       // if cd or a blank line
            continue;
        if (strcmp(cmd_words[0], "exit") == 0)  // "exit" terminates shell
            break;

        symbol = 0;
        insymbol = 0;
        pipesymbol = 0;
        backgroundsymbol = 0;
        for(int loop = 0; loop < num_words; loop++) { // For finding symbols  >, <, |, or &
            if (strcmp(cmd_words[loop], ">") == 0) {
                symbol = loop;
            }
            if (strcmp(cmd_words[loop], "<") == 0) {
                insymbol = loop;
            }
            if (strcmp(cmd_words[loop], "|") == 0) {
                pipesymbol = loop;
            }
            if (strcmp(cmd_words[loop], "&") == 0) {
                backgroundsymbol = loop;
            }
        }

        if (num_words >= 3 && symbol > 0) {                 // Redirect output
            int stdout_saved = dup(1);
            int file;
            if ((file = open(cmd_words[num_words-1], O_WRONLY | O_CREAT | O_TRUNC, 644)) < 0) {
                perror("error with file open");
            }

            if (dup2(file, 1) == -1) {
                perror("error with dup2");
            }

            char *out_cmd_words[symbol+1];
            for(int loop = 0; loop < symbol; loop++)
                out_cmd_words[loop] = cmd_words[loop];
            out_cmd_words[symbol+1] = NULL;

            if (num_words == 4) {
                if (fork() == 0) {
                    execlp(out_cmd_words[0], out_cmd_words[0], out_cmd_words[1], NULL);
                    cmd_not_found(out_cmd_words[0]);        // Successful execlp() does not return
                }
            } else {
                if (fork() == 0) {
                    execvp(cmd_words[0], out_cmd_words);
                    cmd_not_found(out_cmd_words[0]);        // Successful execvp() does not return
                }
            }

            wait(NULL);
            dup2(stdout_saved, 1);
            close(stdout_saved);
            num++;
            symbol = 0;
            continue;
        } else if (num_words >= 3 && insymbol > 0) {        // Redirect input
            int stdin_saved = dup(0);     
            int file;
            if ((file = open(cmd_words[num_words-1], O_RDONLY)) < 0) {
                perror("error with file open");
                //exit(1);
            }

            if (dup2(file, 0) == -1) {
                perror("error with dup2");
            }    
            
            char *in_cmd_words[insymbol+2];
            for(int loop = 0; loop < insymbol; loop++)
                in_cmd_words[loop] = cmd_words[loop];

            in_cmd_words[insymbol] = 0;
            in_cmd_words[insymbol+1] = NULL;

            if (fork() == 0) {
                execvp(cmd_words[0], in_cmd_words);
                cmd_not_found(in_cmd_words[0]);        // Successful execvp() does not return
            }
            wait(NULL);

            dup2(stdin_saved, 0);
            close(stdin_saved);
            num++;
            insymbol = 0;
            continue;
        } else if(num_words >= 2 && backgroundsymbol > 0) { // Background programs
            backgroundrun = 1;
            cmd_words[num_words-1] = NULL;
            pid_t fpid = fork();
            if (fpid < 0) { 
                return 1;
            }
            if (fpid == 0) {
                setpgid(0, 0);
                execvp(cmd_words[0],cmd_words);
                cmd_not_found(cmd_words[0]);
            } else if (fpid && !backgroundrun) {
                waitpid(-1, NULL, 0);
            }
            backgroundrun = 0;
            continue;
        } else if(num_words >= 3 && pipesymbol > 0) {       // Piping
            int p[2];
            pipe(p);
            if(fork() == 0) {  // Left Child
                dup2(p[1], 1); // dup pipe write end on top of stdout
                close(p[0]);   // close pipe fd's
                close(p[1]);
                char *pipepart[pipesymbol+1];
                for(int i = 0; i < pipesymbol; i++)
                    pipepart[i] = cmd_words[i];
                pipepart[pipesymbol] = 0;
                execvp(pipepart[0], pipepart);
                cmd_not_found(cmd_words[0]);
            } else {
                if (fork() == 0) { // Right Child
                    dup2(p[0], 0); // dup pipe read end on top of stdin
                    close(p[0]);   // close pipe fd's
                    close(p[1]);
                    char *pipepart[num_words-pipesymbol];
                    for(int i = 0; i < num_words-pipesymbol-1; i++)
                        pipepart[i] = cmd_words[i+pipesymbol+1];
                    pipepart[num_words-pipesymbol-1] = 0;
                    execvp(pipepart[0], pipepart);
                    cmd_not_found(cmd_words[0]);
                } else { // Parent
                    close(p[0]);
                    close(p[1]);
                    wait(NULL);
                    wait(NULL);
                }
            }
            wait(NULL);
            num++;
            continue;
        }

        if (fork() == 0) {
            execvp(cmd_words[0], cmd_words);        
            cmd_not_found(cmd_words[0]);        // Successful execvp() does not return
        } else if (!backgroundrun) {                    
            waitpid(-1, NULL, 0);               // If a background process was run, wait for the child
        }
        backgroundrun = 0;                      // Set back to 0 in case it was added to
        num++;                                  // command number, blank lines do not increment num
    }
}
