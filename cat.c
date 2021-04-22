#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/*
 * Implementation of my own version of the Linux program cat
 */

void cat(char * input) { 
	int file;
    if ((file = open(input, O_RDONLY)) < 0) { // open file
        perror("error with file open");
        exit(1);
    }
    char contents[2000];
    while (read(file, &contents, 1) > 0) // read file 1 char at a time
    	write(1, &contents, 1);			 // print 1 char at a time
    close(file);
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr, "error: too few arguments\n");
		exit(1);
	} else {
		for(int i = 1; i < argc; i++) 
			cat(argv[i]);
	}
	return 0;
}
