all: myshell

myshell : shell.c
		gcc -o myshell shell.c