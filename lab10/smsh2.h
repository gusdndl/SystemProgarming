#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define YES 1
#define NO 0

char **next_cmd(char *prompt, FILE *fp);
char **splitline(char *line);
void freelist(char **list);
void *emalloc(size_t n);
void *erealloc(void *p, size_t n);
int execute(char **);
void fatal(char *, char *, int);
int process(char **);