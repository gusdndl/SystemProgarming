#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define YES 1
#define NO 0

char *next_cmd(char *prompt, FILE *fp);
char **splitline(char *line);
void freelist(char **list);
void *emalloc(size_t n);
void *erealloc(void *p, size_t n);
int execute(char *argv[]);
void fatal(char *s1, char *s2, int n);
int process(char **);
char *newstr(char *s, int l);
void setup();