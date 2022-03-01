//#include <process.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX 512
#define clear() printf("\033[H\033[J")

/*
 * Written by Edward
 * Some (UNTESTED!!!!!) code to parse commands and break it up
 * into something that execv() can hopefully use
 *
 * Things to probably note:
 * 	We don't need to use a method, just wrote it as one
 * 	so it didnt mess up your flow in the shell init stuff
 *
 * 	We need to free all the stuff in **tokens;
 * 	probably after each loop + exec call just to be safe
 * 	since we can just alloc it again next time
 *
 * 	Error handling? If the input is real wacky? Don't know
 * 	if we need to account for hyper-weird stuff where 
 * 	fgets() literally cannot read it (as opposed to
 * 	commands that don't exist)
 */
void parse_command(FILE* fp) {
    char line[512];
    if(!fgets(line, 512, fp))		// if program forcibly terminates (like CTRL+D)
        return -1;			// returns -1

    if(strcmp(line, "exit\n" == 0)	// if we get "exit" as command
        return 0;			// returns 0

    char *token;			// creates an token to represent one parameter (eg. "ls" or "-la")
    char **tokens = malloc(sizeof(char *) * size);	// creates a list of tokens
    token = strtok(line, " ");		// isolates the first parameter
    int i = 0;
    while(token != NULL) {
        tokens[i] = strdup(token);	// adds a null-terminating char to end of each token
	token = strtok(NULL, " ");	// isolates next parameter
	i++;				// increments index
    }
    tokens[i] = token;			// adds final token to the end

    // I haven't tested this yet, you can probably just loop
    // through tokens[] and print out
    
}

void init_shell()
{
//    clear();
    write(1, "\n\n\n\n******************"
        "************************", 46);
    write(1, "\n\n\n\t****MY SHELL****", 22);
    write(1, "\n\n\t-USE AT YOUR OWN RISK-", 26);
    write(1, "\n\n\n\n*******************"
        "***********************", 47);
    char* username = getenv("USER");
    write(1, "\n\nHello ", 9);
    write(1, username, sizeof(username) + 1);
    write(1, "\n", 2);
    sleep(1);
//    clear();
}

int main(int argc, char **argv) {
    char buffer[512];
    FILE *fp = stdin;
    if (fp == NULL) {
        exit(1);
    }
    init_shell();
    while (1) {
        
        write(1, "mysh> ", 7); 
        int status;
        int retVal = fork();
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            write(1, buffer, sizeof(buffer));
        }
        if (retVal == 0) {
            // child process
            // get program name and program args, and pass it into exec()
            char *const argv[3] = {
                "/usr/bin/ls",
                "-l", 
                NULL
            };
            printf("I am child with pid: %d\n", getpid());
            int ret =  execv(argv[0], argv);
            printf("failed to execute %s, execv() ret val: %d\n", argv[0], ret);
            exit(1);
        } else {
            // parent process
            printf("I am parent with pid: %d\n", getpid());
            int pid = retVal;
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}
