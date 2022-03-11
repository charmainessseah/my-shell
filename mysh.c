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
#define STDERR_FILENO 2

struct node {
	char* alias;
	char** command;
	int commandLength;
	struct node* next;
};

struct node* head = NULL;

// prints a node
void print_node(struct node* node) {
    write(1, node->alias, strlen(node->alias));
    for(int i = 0; i < node->commandLength; i++) {
        write(1, " ", 1);
        write(1, node->command[i], strlen(node->command[i]));
    }
    write(1, "\n", 1);
}


// frees a node
void free_node(struct node* node) {

	free(node->alias);
	for(int i = 0; i < node->commandLength; i++) {
		free(node->command[i]);
	}
	free(node->command);
	free(node);

}

// frees the list
void free_list() {
	//write(1, "freeing list\n", 13);
	while(head != NULL) {
		struct node* current = head;
		head = head->next;
		free_node(current);
	}
}

// prints the list
void print_list() {
	struct node* current = head;
	while(current != NULL) {
		print_node(current);
		current = current->next;
	}
}

// returns node with matching alias, otherwise NULL if not found
struct node* search(char* alias) {
	if(head == NULL)
		return NULL;
	struct node* current = head;
	while(current != NULL) {
		if(strcmp(current->alias, alias) == 0) {
			return current;
        }
		current = current->next;
	}
	return NULL;
}

// adds a node to the list of aliases
void add_alias(char* alias, char** command, int length) {

	if(head == NULL) {
		head = (struct node*) malloc(sizeof(struct node));
		//printf("new head address		: %p\n", head);
		//printf("head's next address	: %p\n", head->next);

		head->alias = strdup(alias);

		char** dupeCommand = (char**) malloc(sizeof(char*) * length);
		for(int i = 0; i < length; i++) {
			dupeCommand[i] = strdup(command[i]);
		}
		head->command = dupeCommand;
		head->commandLength = length;
		head->next = NULL;
	}
	else {
		struct node* newNode = (struct node*) malloc(sizeof(struct node));
		//printf("current head address	: %p\n", head);
		//printf("new node address		: %p\n", newNode);

		newNode->alias = strdup(alias);

		char** dupeCommand = (char**) malloc(sizeof(char*) * length);
		for(int i = 0; i < length; i++) {
			dupeCommand[i] = strdup(command[i]);
		}
		newNode->command = dupeCommand;
		newNode->commandLength = length;
		newNode->next = head;
		head = newNode;
		//printf("new node's next address		: %p\n", newNode->next);
	}

}

// updates a given node with a new command and command length
void update_alias(struct node* node, char** command, int length) {

	char** dupeCommand = (char**) malloc(sizeof(char*) * length);
	for(int i = 0; i < length; i++) {
		dupeCommand[i] = strdup(command[i]);
		free(node->command[i]);
	}
	free(node->command);
	node->command = dupeCommand;
	node->commandLength = length;

        

}

// removes a node from the list with matching alias
void remove_alias(char* alias) {

	struct node* current = head;
	struct node* previous = NULL;

	if(head == NULL)
		return;

	if(strcmp(head->alias, alias) == 0) {
		head = head->next;
		free_node(current);
		return;
	}

	while(current != NULL && strcmp(current->alias, alias) != 0) {
		previous = current;
		current = current->next;
	}

	if(current == NULL)
		return;

	previous->next = current->next;
	free_node(current);

}

// returns 0 if no alias or unalias (so execute command as normal)
// returns 1 if alias- or unalias-related command is executed (so don't send to syscall)
// THIS FUNCTION WILL EXECUTE THE ALIAS-RELATED COMMAND BY ITSELF
int alias_mode(char** tokens, int length) {
	// first token is NOT "alias" or "unalias"
	if(strcmp(tokens[0], "alias") != 0 && strcmp(tokens[0], "unalias") != 0)
		return 0;

	// first token is alias
	else if(strcmp(tokens[0], "alias") == 0) {

		// if length = 1 --> command is just "alias", prints out list
		if(length == 1) {
			return 1;
		}
		// then length > 1, first checks if dangerous aliasing
		else if(strcmp(tokens[1], "alias") == 0
				|| strcmp(tokens[1], "unalias") == 0
				|| strcmp(tokens[1], "exit") == 0) {
			fprintf(stderr, "alias: Too dangerous to alias that.\n");
			return -1;
		}
		// if length = 2 --> command is "alias [name]"
		// print node with matching alias if it exists
		if(length == 2) {
			return 2;
		}
		// otherwise, length > 2 --> add alias
		// or update command if alias already exists
		else {
			return 3;
		}
	}

	// first token is unalias
	else if(strcmp(tokens[0], "unalias") == 0) {
		// checks if command is just "unalias" or has too many arguments
		if(length != 2) {
			printf("unalias: Incorrect number of arguments.\n");
			return -1;
		}
		// otherwise, unaliases
		else {
			return 4;
		}
	}
	return 0;
}
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
 * Returns the index of the file to redirect to in tokens array. Returns -1 if invalid redirection command, 0 if not a redirection command
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
    if (redirectionOperatorIndex == 0) {
        invalidRedirection = 1;
    } else if (numRedirectionOperators == 1) {
        // check that only one file is specified to the right of the redirection op
        if (redirectionOperatorIndex == numArgs - 2) {
            return redirectionOperatorIndex + 1;
        } 
        invalidRedirection = 1;
    } else if (numRedirectionOperators > 1) {
        invalidRedirection = 1;
    } 
    if (invalidRedirection) {
        write(STDERR_FILENO, "Redirection misformatted.\n", 26);
        return -1;
    }
    return 0;
}

int parse_command(char** tokens, char* line) {
    char *token;			            // creates an token to represent one parameter (eg. "ls" or "-la")
    token = strtok(line, " \t");        // isolates the first parameter
    int i = 0;
    while(token != NULL) {
        tokens[i] = strdup(token);      // adds a null-terminating char to end of each token
        token = strtok(NULL, " \t");    // isolates next parameter
        i++;				            // increments index
    }
    tokens[i] = token;			        // adds final token to the end
    int numArgs = i;
    return numArgs;
}

// replaces each instance of ">" with " > " so the delimiter will fit
// the implementation of parsing tokens for redirection
char* replace_redirection(char* original, int length) {
	char* result;
	int count = 0;
	for(int i = 0; i < length; i++) {
		if(original[i] == 0)
			count++;
	}

	result = (char*) malloc(sizeof(char) * (length + count*2));

	count = 0;
	for(int i = 0; i < length; i++) {
		if(original[i] != '>')
			result[i + count*2] = original[i];
		else {
			result[i + count*2] = ' ';
			result[i+1 + count*2] = '>';
			result[i+2 + count*2] = ' ';
			count++;
		}
	}

	return result;
}

int main(int argc, char **argv) {
    //TODO: the command <./mysh .> starts up shell interactive mode and exits right away. It should return
    // <Error: Cannot open file .>
    int batchMode = 0;
    if (argc == 2) {
        batchMode = 1;
    } else if (argc > 2) {
        write(STDERR_FILENO, "Usage: mysh [batch-file]\n", 25);
        _exit(1);
    }
    FILE *fp = NULL;
    FILE *fpChild = NULL;
    if (batchMode) {
        fp = fopen(argv[1], "r");
    } else {
        fp = stdin;
    }
    if (fp == NULL && batchMode) {
        write(STDERR_FILENO, "Error: Cannot open file ", 24);
        write(STDERR_FILENO, argv[1], strlen(argv[1]));
        write(STDERR_FILENO, ".\n", 2);
        _exit(1);
    }
    if (fp == NULL) {
        _exit(1);
    }
    char **tokens = NULL;
    char **childArgv = NULL;
   char **newCommand = NULL;
    int exit = 0;
    while (1) {
        if (exit) {
            _exit(0);
        }
        if (!batchMode) {
            write(1, "mysh> ", 6); 
        }
        char    *linePtr = NULL;
        size_t   lineSize = 0;
        ssize_t  lineLength;
        lineLength = getline(&linePtr, &lineSize, fp);
        if (linePtr == NULL || lineLength == -1) {
            fclose(fp);
	    free_list();
            exit = 1;
            goto CLEANUP;
     // here       //_exit(0);
        }
        int result = strcmp(linePtr, "exit\n");
        if (batchMode) { // echo user command
            write(1, linePtr, lineLength);
        }

        if (!result) {
            exit = 1;
	    free_list();
            goto CLEANUP;
         // here   //   _exit(0);
        } 
        if(linePtr[lineLength - 1] == '\n')
            linePtr[lineLength - 1] = '\0';
        //tokensBefore = malloc(sizeof(char*) * 512);
	char* converted = replace_redirection(linePtr, lineLength);
        tokens = malloc(sizeof(char*) * 512);
        int numArgs = parse_command(tokens, converted);
        free(converted);
        if (numArgs == 0) { // empty command
	    free_list();
            goto CLEANUP;
        }
        //----------------------------------------------
        // test for aliasing here
        int doAliasing = alias_mode(tokens, numArgs);
        //printf("alias result: %d\n", doAliasing);
        //printf("num args: %d\n", numArgs);
        //printf("head	: %p\n", head);
        //fflush(stdout);
        if (doAliasing == -1) {
            //printf("error encountered\n");
            //fflush(stdout);
            goto CLEANUP;
         }
        if (doAliasing == 1) {
            //printf("printing list\n");
            //fflush(stdout);
            print_list();
            goto CLEANUP;
        }
        if (doAliasing == 2) {
            //printf("printing alias\n");
            //fflush(stdout);
            struct node* alias = search(tokens[1]);
            if(alias == NULL) { }
            else
                print_node(alias);
            goto CLEANUP;
        }
        if (doAliasing == 3) {
            //printf("adding or updating alias\n");
            //fflush(stdout);
            char** command = &tokens[2];
            struct node* dupe = search(tokens[1]);
            if(dupe == NULL) {
                struct node* newNode = (struct node*) malloc(sizeof(struct node));

                //printf("head	: %p\n", head);
                //printf("newNode	: %p\n", newNode);

                newNode->alias = strdup(tokens[1]);

                char** dupeCommand = (char**) malloc(sizeof(char*) * numArgs-2);
                for(int i = 0; i < numArgs-2; i++) {
                    dupeCommand[i] = strdup(command[i]);
                }
                newNode->command = dupeCommand;
                newNode->commandLength = numArgs-2;
                newNode->next = head;
                head = newNode;
                //printf("next	: %p\n", newNode->next);
            }
            else {
                //printf("updatingn alias \n");
		//printf("%p", dupe);
		update_alias(dupe, (char**)&tokens[2], numArgs-2);
	    }
            goto CLEANUP;
        }
        if (doAliasing == 4) {
            //printf("removing alias\n");
            //fflush(stdout);
            struct node* dupe = search(tokens[1]);
            if(dupe == NULL) { }
            else
                remove_alias(tokens[1]);
            goto CLEANUP;
        }
        
        // ---------------------------------------------
        // check for invalid redirection here
        int indexToken = file_redirection(tokens, numArgs);
        if (indexToken == -1) {
           goto CLEANUP;
        }
        // --------------------------------------------------------
        // check if input is an alias
        int execAlias = 0;
        struct node* alias;
        //printf("token: %s\n", tokens[0]);
        alias = search(tokens[0]);
        if (alias != NULL) {
            execAlias = 1;
            //printf("this is an alias command 1\n");
        }
        // create child process
        int status;
        int retVal = fork();
        if (retVal == 0) {
            if (execAlias) {
                //TODO: this line below was passing test 17, 18, 19 but it actually was not working as expected for echo and cat alias
                newCommand = malloc(sizeof(char*) * 512);
                printf("command length: %d\n", alias->commandLength);
                for (int i = 0; i < (alias->commandLength); i++) {
                    newCommand[i] = alias->command[i];
                }
                int currIndex = alias->commandLength;
                for (int i = 0; i < numArgs - 1; i++) {
                    newCommand[currIndex++] = tokens[i + 1];
                }
                newCommand[currIndex] = NULL;
                execv(alias->command[0], newCommand);
            }else if (indexToken != 0) { // handle redirection
                // we create a new childAgrv to remove all tokens including the redirection token and what comes after
                // we want it such that tokens being passed into execv only contains the command before > as file redirection is taken care of
                childArgv = malloc(sizeof(char*) * 512);
                handle_child_argv(tokens, childArgv, indexToken);
                close(1);
                fpChild = fopen(tokens[indexToken], "w");
                open(tokens[indexToken], O_WRONLY);
                if (access(tokens[indexToken], W_OK)) { // check user permissions
                    write(1, "Cannot write to file ", 21); 
                    write(1, tokens[indexToken], strlen(tokens[indexToken]));
                    write(1, "\n", 1);
                    goto CLEANUP;
                }   
                execv(childArgv[0], childArgv);
            } else { // regular command, no redirection
                execv(tokens[0], tokens);
            }
        // --------------------------------------------------------
            // execv failed so we exit
            write(STDERR_FILENO, tokens[0], strlen(tokens[0]));
            write(STDERR_FILENO, ": Command not found.\n", 21);
            exit = 1;
            goto CLEANUP;
  // here          //_exit(1); // this means execv() fails
        } else {
            // parent process
            int pid = retVal;
            waitpid(pid, &status, 0);
        }
 CLEANUP:
        // free up resources
	//printf("freeing\n");
        free(linePtr);
	//free(converted);
        linePtr = NULL;
        lineSize = 0;
    }
    printf("freeing 2 \n");
    // free up resources
    if (fp != NULL) {
        fclose(fp);
    }
    if (fpChild != NULL) {
        fclose(fpChild);
    }
    if (tokens != NULL) {
	//printf("freeing tokens\n");
        for(int i = 0; i < 512; i++) {
            free(tokens[i]); 
        }                
        free(tokens);
    }
//    if (newCommand != NULL) {
//         for(int i = 0; i < 512; i++) {
//             free(newCommand[i]);
//         }
//         free(newCommand);
//     }
    if (childArgv != NULL) {
        for (int i = 0; i < 512; i++) {
            free(childArgv[i]);
        }
        free(childArgv);
    }
    //free_list();
    return 0;
}

