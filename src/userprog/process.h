#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include <string.h>
#include "threads/thread.h"
#include "userprog/syscall.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/* For each substring splitted by seperator. */
#define for_str(elem, str, sep, last) \
    for ((elem) = strtok_r((str), (sep), (last)); \
        (elem) != NULL; \
        (elem) = strtok_r(NULL, (sep), (last)))

/* Make a deep copy of string. */
char *strcpy2(const char *src);

/* Push one byte into the stack. */
void push_stack_char(void **esp, char val);

/* Push four bytes into the stack. */
void push_stack_int(void **esp, int val);

#endif /* userprog/process.h */
