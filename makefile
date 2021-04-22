all: shell.c cat.c ls.c
	gcc -g -Wall -o shell shell.c

	gcc -g -Wall -o cat cat.c

	gcc -g -Wall -o ls ls.c

clean: 
	$(RM) shell cat ls
