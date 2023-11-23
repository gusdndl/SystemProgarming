#include <stdio.h>
#include "smsh.h"

int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();
int builtin_command(char **, int *);

int process(char **args)
{
    int rv = 0;
    if (args[0] == NULL)
    {
        rv = 0;
    }
    else if (is_control_command(args[0]))
    {
        rv = do_control_command(args);
    }
    else if (ok_to_execute())
    {
        if (!builtin_command(args, &rv))
        {
            rv = execute(args);
        }
    }
    return rv;
}