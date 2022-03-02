run: compile
	./mysh

compile: mysh.c
	gcc -o mysh -Wall -Werror -g mysh.c

clean:
	rm -f mysh
