#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smsh2.h"

#define is_delim(x) ((x) == ' ' || (x) == '\t')

char **next_cmd(char *prompt, FILE *fp) {
    char *buf;
    char *ptr;
    int bufspace = 0;
    int pos = 0;
    int c;
    int argi = 0;
    int trigger = 0;
    char **argv;

    argv = emalloc(BUFSIZ);

    printf("%s", prompt);
    while ((c = getc(fp)) != EOF) {
        if (pos + 1 >= bufspace) {
            if (bufspace == 0) {
                buf = emalloc(BUFSIZ);
            } else {
                buf = erealloc(buf, bufspace + BUFSIZ);
            }
            bufspace += BUFSIZ;
        }

        if (c == ';') {
            trigger = 1;
        }

        if (c == '\n') {
            break;
        }
        buf[pos++] = c;
    }

    // if nothing input //i.e. ctrl + d initially pressed
    if (c == EOF && pos == 0) {
        return NULL;
    }

    // lastly put null char on last space
    buf[pos] = '\0';

    if (trigger == 1) {
        ptr = strtok(buf, ";");
        while (ptr != NULL) {
            argv[argi] = ptr;
            argi++;
            ptr = strtok(NULL, ";");
        }
    }

    return argv;
}

char **splitline(char *line) {
    char *newstr();
    char **args;
    int spots = 0;
    int bufspace = 0;
    int argnum = 0;
    char *cp = line;
    char *start;
    int len;

    if (line == NULL) {
        return NULL;
    }

    args = emalloc(BUFSIZ);
    bufspace = BUFSIZ;
    spots = BUFSIZ / sizeof(char *);

    while (*cp != '\0') {
        while (is_delim(*cp)) {
            cp++;
        }
        if (*cp == '\0') {
            break;
        }
        if (argnum + 1 >= spots) {
            args = erealloc(args, bufspace + BUFSIZ);
            bufspace += BUFSIZ;
            spots += (BUFSIZ / sizeof(char *));
        }
        start = cp;
        len = 1;
        while ((*++cp != '\0') && !(is_delim(*cp))) {
            len++;
        }
        args[argnum++] = newstr(start, len);
    }
    args[argnum] = NULL;
    return args;
}

// create new string and return pointer of zero index
char *newstr(char *s, int l) {
    char *rv = emalloc(l + 1);

    rv[l] = '\0';
    strncpy(rv, s, l);
    return rv;
}

// enhanced free function
void freelist(char **list) {
    char **cp = list;
    while (*cp) {
        free(*cp++);
    }
    free(list);
}

// enhanced malloc
void *emalloc(size_t n) {
    void *rv;
    if ((rv = malloc(n)) == NULL) {
        fatal("out of memory", "", 1);
    }
    return rv;
}

// enhanced realloc
void *erealloc(void *p, size_t n) {
    void *rv;
    if ((rv = realloc(p, n)) == NULL) {
        fatal("realloc() failed", "", 1);
    }
    return rv;
}