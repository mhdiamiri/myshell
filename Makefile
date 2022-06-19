CC = gcc

main: main.c myshell.c
		${CC} -o shell main.c myshell.c