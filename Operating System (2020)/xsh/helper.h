#ifndef _XSH_HELPER_H_
#define _XSH_HELPER_H_

#include <stdlib.h>

#define NON_ERROR 0x00
#define NON_BUILTIN 0x01

#define FIRST_CMD 0x01
#define MIDDLE_CMD 0x02
#define LAST_CMD 0x03

#define READ_END 0x00
#define WRITE_END 0x01

#define die(code, msg) \
    do                 \
    {                  \
        puts((msg));   \
        _exit((code));  \
    } while (0)

#define lenof(p) __lenof__((void **)(p))

size_t __lenof__(void **);

void show_promot(void);

void waitn(int);

int read_line(char *, size_t);

int excute(char **, int *, int);

int excute_builtin(char **);

int parse_arguments(char *, char **);

int parse_commands(char *, char **, char ***);

#endif