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
// void parse_command(FILE* fp) {
//     char line[512];
//     if(!fgets(line, 512, fp))		// if program forcibly terminates (like CTRL+D)
//         return -1;			// returns -1

//     if(strcmp(line, "exit\n" == 0)	// if we get "exit" as command
//         return 0;			// returns 0

//     char *token;			// creates an token to represent one parameter (eg. "ls" or "-la")
//     char **tokens = malloc(sizeof(char *) * size);	// creates a list of tokens
//     token = strtok(line, " ");		// isolates the first parameter
//     int i = 0;
//     while(token != NULL) {
//         tokens[i] = strdup(token);	// adds a null-terminating char to end of each token
// 	token = strtok(NULL, " ");	// isolates next parameter
// 	i++;				// increments index
//     }
//     tokens[i] = token;			// adds final token to the end

//     // I haven't tested this yet, you can probably just loop
//     // through tokens[] and print out
    
// }

void init_shell()
{
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
}

int main(int argc, char **argv) {
    int batchMode = 0;
    if (argc == 2) {
        batchMode = 1;
    }
    FILE *fp = NULL;
    if (batchMode) {
        fp = fopen(argv[1], "r");
        printf("batch mode \n");
    } else {
        fp = stdin;
        printf("stdin\n");
    }
    if (fp == NULL) {
        exit(1);
    }
    init_shell();

    while (1) {
        if (!batchMode) {
            write(1, "mysh> ", 7); 
        }
        char    *linePtr = NULL;
        size_t   lineSize = 0;
        ssize_t  lineLength;
        lineLength = getline(&linePtr, &lineSize, stdin);
        if (linePtr == NULL) {
            exit(1);
        }
        int result = !strcmp(linePtr, "exit");
        if (!result) {
            write(1, "exiting shell...", 17);
            exit(1);
        }
        if (batchMode) {
            write(1, linePtr, lineLength);
        }
        // create a new process
        int status;
        int retVal = fork();
        if (retVal == 0) {
            // child process
            printf("I am child with pid: %d\n", getpid());
            // get program name - argv[0]
            // second argument is pointer to our entire argument array?

            // prog name is argv[0] and we can pass in 
            // char *const argv[3] = {
            //     "/usr/bin/ls",
            //     "-l", 
            //     NULL
            // };
            
            int ret =  execv(argv[0], argv);
            printf("failed to execute %s, execv() ret val: %d\n", argv[0], ret);
            exit(1);
        } else {
            // parent process
            printf("I am parent with pid: %d\n", getpid());
            int pid = retVal;
            waitpid(pid, &status, 0);
        }

        // free up resources
        free(linePtr);
        linePtr = NULL;
        lineSize = 0;
    }
    return 0;
}
