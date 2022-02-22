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
