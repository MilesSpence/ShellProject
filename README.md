# ShellProject
## Created my own shell program with the following features:
- Basic reading of a command line and breaking into its parts
- cd and pwd commands
- fork() and exec() of command that are programs such as % cat file.txt
- Connect two commands with a pipe such as % cat file.txt | grep string
- Redirect output such as % cat file.txt > outputfile.txt
- Redirect input such as % grep string < inputfile.txt
- Processes Control-D to exit the shell
- Ability to run a program in the background such as % ls &
- Processes Control-C such that the shell does not terminate
- Implemented my own version of the ls program
- Implemented my own version of the Linux program cat
