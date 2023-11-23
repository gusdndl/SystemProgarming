#include "smsh2.h"

#define DFL_PROMPT "> ";

void setup();

int main() {
    char **cmdline, *prompt;
    int result;

    prompt = DFL_PROMPT;
    setup();

    while((cmdline = next_cmd(prompt, stdin)) != NULL) {
        for (int i = 0; cmdline[i] != NULL; i ++) {
            char **arglist;
            if ((arglist = splitline(cmdline[i])) != NULL) {
                result = execute(arglist);
            }
            freelist(arglist);
        }
        free(cmdline);
    }
    return 0;
}

void setup() {
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n) {
    fprintf(stderr, "Error: %s, %s\n", s1, s2);
    exit(n);
}