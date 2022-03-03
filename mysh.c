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
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAX 512
#define clear() printf("\033[H\033[J")

/*
 * Takes in the token array and removes all arguments after including and after the redirection operator. 
 * Appends a null character at the end. Goal: Correct array for execv to use when redirection is enabled
 */
void handle_child_argv(char **tokens, char **childArgv, int tokenLength) {
    for (int i = 0; i < tokenLength - 1; i++) {
        childArgv[i] = tokens[i];
    }
    childArgv[tokenLength - 1] = NULL;
}
/*
 * Returns the index of the file to redirect to in tokens array. Returns -1 if invalid command, 0 otherwise
 */
int file_redirection(char **tokens, int numArgs) {
    int numRedirectionOperators = 0;
    int redirectionOperatorIndex = -1;
    for (int i = 0; i < numArgs; i++) {
        if (!strcmp(tokens[i], ">")) { // if they are equal
            numRedirectionOperators++;
            redirectionOperatorIndex = i;
        }
    }

    int invalidRedirection = 0;
    if (numRedirectionOperators == 1) {
        // check that only one file is specified to the right of the redirection op
        if (redirectionOperatorIndex == numArgs - 2) {
            return redirectionOperatorIndex + 1;
        } 
        invalidRedirection = 1;
    } else if (numRedirectionOperators > 1) {
        invalidRedirection = 1;
    } 
    if (invalidRedirection) {
        write(1, "Redirection misformatted.\n", 27);
        return -1;
    }
    return 0;
}
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
int parse_command(char** tokens, char* line) {
//     char line[512];
//     if(!fgets(line, 512, fp))		// if program forcibly terminates (like CTRL+D)
//         return -1;			// returns -1

//     if(strcmp(line, "exit\n" == 0)	// if we get "exit" as command
//         return 0;			// returns 0

    char *token;			// creates an token to represent one parameter (eg. "ls" or "-la")
//     char **tokens = malloc(sizeof(char *) * size);	// creates a list of tokens
    token = strtok(line, " \t");		// isolates the first parameter
    int i = 0;
    while(token != NULL) {
        tokens[i] = strdup(token);	// adds a null-terminating char to end of each token
	    token = strtok(NULL, " \t");	// isolates next parameter
	    i++;				// increments index
    }
    tokens[i] = token;			// adds final token to the end

//     // I haven't tested this yet, you can probably just loop
//     // through tokens[] and print out

    // ----------------------------------------------------------------------------------
    int numArgs = i;
    return numArgs;
}

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
    FILE *fpChild = NULL;
    if (batchMode) {
        fp = fopen(argv[1], "r");
    } else {
        fp = stdin;
    }
    if (fp == NULL) {
        _exit(1);
    }
    init_shell();

    while (1) {
        if (!batchMode) {
            write(1, "mysh> ", 7); 
        }
        char    *linePtr = NULL;
        size_t   lineSize = 0;
        ssize_t  lineLength;
        lineLength = getline(&linePtr, &lineSize, fp);
        if (linePtr == NULL || lineLength == -1) {
            write(1, "exiting shell...\n", 18);
            fclose(fp);
            _exit(1);
        }
        int resultInteractive = strcmp(linePtr, "exit\n");
//        int resultBatch = strcmp(linePtr, "exit");
        if (!resultInteractive) {
            write(1, "exiting shell...\n", 18);
            _exit(1);
        }
        printf("line %s\n", linePtr);
        if (batchMode) { // echo user command
            write(1, linePtr, lineLength);
            // write(1, "\n", 2);
        }
        // create a new process
        int status;
        int retVal = fork();
        if (retVal == 0) {
            // child process
            // printf("I am child with pid: %d\n", getpid());
            // get program name - argv[0]
            // second argument is pointer to our entire argument array?

            // prog name is argv[0] and we can pass in 
            // char *const argv[3] = {
            //     "/usr/bin/ls",
            //     "-l", 
            //     NULL
            // };
            
            // int ret =  execv(argv[0], argv);
	    if(linePtr[lineLength - 1] == '\n')
                linePtr[lineLength - 1] = '\0';

	    char** tokens = malloc(sizeof(char*) * 512);
        /*****
        modified this section such that file_redirection returns 3 diff values - see function header
        numArgs gets the number of arguments from user input
        have yet to check for user permissions first on line 179
        issues: output is still not being redirected to the output file
        ****/
	    int numArgs = parse_command(tokens, linePtr);
        int indexToken = file_redirection(tokens, numArgs);
        // printf("num args: %d\n", numArgs);
        // printf("index file: %d\n", indexToken);
        // printf("filename: %s\n", tokens[indexToken]);
        if (indexToken == -1) {
            continue; // invalid command - do not execute
        }
        if (indexToken != 0) {
            // handle redirection
            close(1); //close stdout
            // first check for user permissions using access()
            open(tokens[indexToken], O_WRONLY);
        } 
        // --------------------------------------------------------
            // we want it such that tokens being passed into execv only contains the command before > as file redirection is taken care of
            char** childArgv = malloc(sizeof(char*) * 512);
            handle_child_argv(tokens, childArgv, indexToken);
            int ret = -1;
            if (indexToken != 0) {
                fpChild = fopen(tokens[indexToken], "w");
                ret = execv(childArgv[0], childArgv);
            } else {
                ret = execv(tokens[0], tokens);
            }
        // --------------------------------------------------------
            // int ret = execv(tokens[0], tokens);

            printf("failed to execute %s, execv() ret val: %d\n", tokens[0], ret);
            for(int i = 0; i < 512; i++) {
                free(tokens[i]);
	        }
	    free(tokens);
	    _exit(1); // this means execv() fails
        } else {
            // parent process
            // printf("I am parent with pid: %d\n", getpid());
            int pid = retVal;
            waitpid(pid, &status, 0);
        }
        // free up resources
        free(linePtr);
        linePtr = NULL;
        lineSize = 0;
        
    }
    fclose(fp);
    if (fpChild != NULL) {
        fclose(fpChild);
    }
    return 0;
}

