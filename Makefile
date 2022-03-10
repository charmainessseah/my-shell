run: compile
	./mysh

compile: mysh.c
	gcc -o mysh -Wall -Werror -g -ggdb3 mysh.c

clean:
	rm -f mysh
