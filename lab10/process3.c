#include <stdio.h>
#include "smsh.h"

int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();
int builtin_command(char **, int *);

int process(char **args) {
    int rv = 0;
    int is_background = 0;
    int cpid;

    if (args[0] == NULL) {
        rv = 0;
    } else if (strcmp(args[0], "exit") == 0) { // Check for exit command
        exit(0);
    } else if (is_control_command(args[0])) {
        rv = do_control_command(args);
    } else if (ok_to_execute()) {
        // Check for background command (&)
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "&") == 0) {
                is_background = 1;
                args[i] = NULL; // Remove the '&' from arglist
                break;
            }
        }

        if (!builtin_command(args, &rv)) {
            cpid = fork();

            switch(cpid) {
                case -1: // fork failed
                    perror("fork failed");
                    exit(1);
                case 0: // child process
                    execvp(args[0], args);
                    perror("Command not found...");
                    exit(1);
                default: // parent process
                    if (!is_background) {
                        while(wait(&rv) != cpid)
                            ;
                    }
            }
        }
    }
    return rv;
}
