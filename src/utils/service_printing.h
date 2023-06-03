#include <errno.h>
#include <stdio.h>

#include "bash_format.h"

#ifndef __SEVICE_PRINT_H
#define __SEVICE_PRINT_H

#define QUOTE(expression) #expression
#define QUOTE_X(expression) QUOTE(expression)

#ifdef DEBUG
#define PRINTD(...)                                          \
    {                                                        \
        char buffer[4096];                                   \
        char *tmp = buffer;                                  \
        tmp += sprintf(tmp, "%s(%d): ", __FILE__, __LINE__); \
        tmp += sprintf(tmp, __VA_ARGS__);                    \
        tmp += sprintf(tmp, "\n");                           \
        fprintf(stderr, "%s", buffer);                       \
    }

#define DbgPrint(str, ...) \
    fprintf(stderr, BASH_DBOLD "%s: " str "\n" BASH_DEFAULT, __FILE__ "(" QUOTE_X(__LINE__) ")", ##__VA_ARGS__)
#else
#define PRINTD(...)
#define DbgPrint(str, ...)
#endif /* DEBUG */

#define PrintErr(str, ...)                                                                                           \
    fprintf(stderr,                                                                                                  \
            BASH_LRED BASH_BLINK "ERROR! " BASH_DEFAULT BASH_LRED "(%s): " BASH_DEFAULT BASH_BOLD BASH_UNDERLINE str \
                                 "\n" BASH_DEFAULT,                                                                  \
            strerror(errno),                                                                                         \
            ##__VA_ARGS__)

#define PrintWarn(str, ...)                                                                                   \
    fprintf(stderr,                                                                                           \
            BASH_LMAGENTA BASH_BLINK "WARNING! " BASH_DEFAULT BASH_BOLD BASH_UNDERLINE str "\n" BASH_DEFAULT, \
            ##__VA_ARGS__)

#endif /* __SEVICE_PRINT_H */
