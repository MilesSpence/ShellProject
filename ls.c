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
#include <sys/user.h>
#include <dirent.h>
#include <time.h>
#include <stdbool.h> 

/*
 * Implementation of my own version of the Linux program ls
 */

int file_select(const struct dirent *entry) { // for filtering
	return (entry->d_name[0] != '.');
}

void lsl(char * input, bool filter) { // used for when -l flag is called
    struct dirent **files;
	struct stat statbuf;
	int num_entries;
    if (filter)
		num_entries = scandir(input, &files, file_select, alphasort);
	else
		num_entries = scandir(input, &files, NULL, alphasort);

	for(int i = 0; i < num_entries; i++) {
		if(stat(files[i]->d_name, &statbuf) == 1) // retrieve file info
			fprintf(stderr, "error: No such file or directory: %s\n", files[i]->d_name);

		if (S_ISREG(statbuf.st_mode)) // file type
			printf("-");
		else
			printf("d");

		if (statbuf.st_mode & S_IRUSR) // permissions
			printf("r");
		else
			printf("-");
		if (statbuf.st_mode & S_IWUSR)
			printf("w");
		else
			printf("-");
		if (statbuf.st_mode & S_IXUSR)
			printf("x");
		else
			printf("-");
		if (statbuf.st_mode & S_IRGRP)
			printf("r");
		else
			printf("-");
		if (statbuf.st_mode & S_IWGRP)
			printf("w");
		else
			printf("-");
		if (statbuf.st_mode & S_IXGRP)
			printf("x");
		else
			printf("-");
		if (statbuf.st_mode & R_OK)
	    	printf("r");
		else
			printf("-");
	    if (statbuf.st_mode & W_OK)
	    	printf("w");
	    else
	    	printf("-");
	    if (statbuf.st_mode & X_OK)
	    	printf("x");
	    else
	    	printf("-");
	    
	    printf(" ");

	    printf("%ld ", statbuf.st_nlink); // number of hard links

	    char filename[] = "/etc/passwd";  // user (owner) name
		FILE *file = fopen (filename, "r");
		if (file != NULL) {
			char line [100];
			while(fgets(line,sizeof line,file)!= NULL) {
		    	char* name = strtok(line, ":");
		    	char * nid;
		    	for (int i = 0; i < 2; i++)
		    		nid = strtok(NULL, ":");
		    	if(atoi(nid) == statbuf.st_uid)
		    		fprintf(stdout,"%s ", name);
		    }
		    fclose(file);
		} else {
			perror(filename);
		}

	    char newfilename[] = "/etc/group";   // group name
		FILE *newfile = fopen (newfilename, "r");
		if (newfile != NULL) {
			char line [100];
			while(fgets(line,sizeof line,newfile)!= NULL) {
		    	char* name = strtok(line, ":");
		    	char * nid;
		    	for (int i = 0; i < 2; i++)
		    		nid = strtok(NULL, ":");
		    	if(atoi(nid) == statbuf.st_uid)
		    		fprintf(stdout,"%s ", name);
		    }
		    fclose(newfile);
		} else {
			perror(newfilename);
		}

	    if (statbuf.st_size < 10) // size
	    	printf("    %ld ", statbuf.st_size);
	    else if (statbuf.st_size < 100)
	    	printf("   %ld ", statbuf.st_size);
		else if (statbuf.st_size < 1000)
	    	printf("  %ld ", statbuf.st_size);
		else if (statbuf.st_size < 10000)
	    	printf(" %ld ", statbuf.st_size);
	    else
	    	printf("%ld ", statbuf.st_size);

	    struct tm dt; // date/time last modified
	    dt = *(localtime(&statbuf.st_mtime));

	    const char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	    printf("%s ", months[dt.tm_mon]); // month

	    if(dt.tm_mday < 10) // day
	    	printf(" %d ", dt.tm_mday);
	    else
	    	printf("%d ", dt.tm_mday);

	    if(dt.tm_hour < 10) // hour
	    	printf("0%d:", dt.tm_hour);
	    else
	    	printf("%d:", dt.tm_hour);

	    if(dt.tm_min < 10) // minute
	    	printf("0%d ", dt.tm_min);
	    else
	    	printf("%d ", dt.tm_min);

	    printf("%s\n", files[i]->d_name); // filename
	    free(files[i]);
	}
	free(files);
}

void ls(char * input, bool a, bool l) {
	struct dirent **files;
	if(a && l) {
		lsl(input, false);
	} else if (l) {
		lsl(input, true);
	} else if (a) {
		int num_entries = scandir(input, &files, NULL, alphasort);
		for(int i = 0; i < num_entries; i++) {
    		printf("%s\n", files[i]->d_name);
    		free(files[i]);
   		}
   		free(files);
	} else {
		int num_entries = scandir(input, &files, file_select, alphasort);
		for(int i = 0; i < num_entries; i++) {
    		printf("%s\n", files[i]->d_name);
    		free(files[i]);
   		}
   		free(files);
	}
}

int main(int argc, char *argv[]) {
	int lspot = 0;
	bool aflag = false;
	bool lflag = false;
	for(int i = 1; i < argc; i++) { // look for flags
		if(argv[i][0] == '-') {
			lspot = i;
			if (argv[i][1] == 'a' || argv[i][2] == 'a') {
				aflag = true;
			}
			if (argv[i][1] == 'l' || argv[i][2] == 'l') {
				lflag = true;
			}
		}
	}
	if (argc == 1) { // go through every single case and call ls accordingly
		ls(".", false, false);
	} else if (aflag && lflag && argc == 2) {
		ls(".", true, true);
	} else if (lflag && argc == 2) {
		ls(".", false, true);
	} else if (aflag && argc == 2) {
		ls(".", true, false);
	} else if (argc == 2){
		ls(argv[1], false, false);
	} else if (aflag && lflag && argc == 3) {
		for(int i = 1; i < argc; i++) {
			if (i == lspot)
				continue;
			ls(argv[i], true, true);
		}
	} else if (lflag && argc == 3) {
		for(int i = 1; i < argc; i++) {
			if (i == lspot)
				continue;
			ls(argv[i], false, true);
		}
	} else if (aflag && argc == 3) {
		for(int i = 1; i < argc; i++) {
			if (i == lspot)
				continue;
			ls(argv[i], true, false);
		}
	} else if (argc == 3) {
		for(int i = 1; i < argc; i++) {
			printf("%s:\n", argv[i]);
			ls(argv[i], false, false);
			if(i != argc-1)
				printf("\n");
		}
	} else if (aflag && lflag && argc > 3) {
		for(int i = 1; i < argc; i++) {
			if (i == lspot)
				continue;
			printf("%s:\n", argv[i]);
			ls(argv[i], true, true);
			if(i != argc-1)
				printf("\n");
		}
	} else if (lflag && argc > 3) {
		for(int i = 1; i < argc; i++) {
			if (i == lspot)
				continue;
			printf("%s:\n", argv[i]);
			ls(argv[i], false, true);
			if(i != argc-1)
				printf("\n");
		}
	} else if (aflag && argc > 3) {
		for(int i = 1; i < argc; i++) {
			if (i == lspot)
				continue;
			printf("%s:\n", argv[i]);
			ls(argv[i], true, false);
			if(i != argc-1)
				printf("\n");
		}
	} else {
		for(int i = 1; i < argc; i++) {
			printf("%s:\n", argv[i]);
			ls(argv[i], false, false);
			if(i != argc-1)
				printf("\n");
		}
	}
	return 0;
}
