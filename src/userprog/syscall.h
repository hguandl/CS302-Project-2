#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>
#include "userprog/process.h"

struct proc_file {
    struct file *ptr;
    int fd;
    struct list_elem elem;
};

int exec_proc(char *file_name);
void exit_proc(int status);

void syscall_init (void);
void *check_addr(const void *vaddr);
struct proc_file *list_search(struct list *files, int fd);

void close_file(struct list *files, int fd);
void close_all_files(struct list *files);

extern bool running;

#endif /* userprog/syscall.h */
