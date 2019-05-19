#include "userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "threads/malloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int *p = f->esp;

  check_addr(p);
  uint8_t system_call = *(uint8_t *)p;

  switch (system_call)
  {
    case SYS_PRACTICE:
      check_addr(p + 1);
      f->eax = *((char *)*(p + 1) + 1);
      break;

    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      check_addr(p + 1);
      exit_proc(*(p + 1));
      break;

    case SYS_EXEC:
      check_addr(p + 1);
      check_addr(p + 2);
      check_addr((void *)*(p + 1));
      f->eax = exec_proc((char *)*(p + 1));
      break;
    
    case SYS_WAIT:
      check_addr(p + 1);
      f->eax = process_wait(*(p + 1));
      break;
    
    case SYS_CREATE:
      check_addr(p + 5);
      check_addr((void *)*(p + 4));
      lock_acquire(&filesys_lock);
      f->eax = filesys_create((char *)*(p + 4), *(p + 5));
      lock_release(&filesys_lock);
      break;

    case SYS_REMOVE:
      check_addr(p + 1);
      check_addr((void *)*(p + 1));
      lock_acquire(&filesys_lock);
      f->eax = filesys_remove((char *)*(p + 1));
      lock_release(&filesys_lock);
      break;

    case SYS_OPEN:
      check_addr(p + 1);
      check_addr((void *)*(p + 1));
      lock_acquire(&filesys_lock);
      struct file *fptr = filesys_open((char *)*(p + 1));
      lock_release(&filesys_lock);
      if (fptr == NULL)
        f->eax = -1;
      else {
        struct proc_file *pfile = malloc(sizeof(*pfile));
        pfile->ptr = fptr;
        pfile->fd = thread_current()->fd_count;
        thread_current()->fd_count++;
        list_push_back(&thread_current()->files, &pfile->elem);
        f->eax = pfile->fd;
      }
      break;

    case SYS_FILESIZE:
      check_addr(p + 1);
      lock_acquire(&filesys_lock);
      f->eax = file_length(list_search(&thread_current()->files, *(p + 1))->ptr);
      lock_release(&filesys_lock);
      break;
    
    case SYS_READ:
      check_addr(p + 7);
      check_addr((void *)*(p + 6));
      if (*(p + 5) == 0) {
        uint8_t *buffer = (uint8_t *)*(p + 6);
        for (int i = 0; i < *(p + 7); i++) {
          buffer[i] = (char)input_getc();
        }
        f->eax = *(p + 7);
      } else {
        struct proc_file *fptr = list_search(&thread_current()->files, *(p + 5));
        if (fptr == NULL)
          f->eax = -1;
        else {
          lock_acquire(&filesys_lock);
          f->eax = file_read(fptr->ptr, (void *)*(p + 6), *(p + 7));
          lock_release(&filesys_lock);
        }
      }
        break;

    case SYS_WRITE:
      check_addr(p + 7);
      check_addr((void *)*(p + 6));
      if (*(p + 5) == 1) {
        putbuf((char *)*(p + 6), *(p + 7));
        f->eax = *(p + 7);
      } else {
        struct proc_file *fptr = list_search(&thread_current()->files, *(p + 5));
        if (fptr == NULL)
          f->eax = -1;
        else {
          lock_acquire(&filesys_lock);
          f->eax = file_write(fptr->ptr, (void *)*(p + 6), *(p + 7));
          lock_release(&filesys_lock);
        }
      }
      break;

    case SYS_SEEK:
      check_addr(p + 5);
      struct proc_file *fptr = list_search(&thread_current()->files, *(p + 5));
      if (fptr == NULL)
        f->eax = -1;
      else {
        lock_acquire(&filesys_lock);
        file_seek(list_search(&thread_current()->files, *(p + 4))->ptr, *(p + 5));
        lock_release(&filesys_lock);
      }
      break;

    case SYS_TELL:
      check_addr(p + 1);
      lock_acquire(&filesys_lock);
      f->eax = file_tell(list_search(&thread_current()->files, *(p + 1))->ptr);
      lock_release(&filesys_lock);
      break;

    case SYS_CLOSE:
      check_addr(p + 1);
      lock_acquire(&filesys_lock);
      close_file(&thread_current()->files, *(p + 1));
      lock_release(&filesys_lock);
      break;
    
    default:
      exit_proc(-1);
      break;
    }
}

int exec_proc(char *file_name) {
  check_addr(file_name);

  char *check = file_name;
  while (*check) {
    check++;
    check_addr(check);
  }

  lock_acquire(&filesys_lock);

  char *fn_cp = strcpy2(file_name);

  char *save_ptr;
  fn_cp = strtok_r(fn_cp, " ", &save_ptr);

  struct file *f = filesys_open(fn_cp);
  free(fn_cp);

  if (f == NULL) {
    lock_release(&filesys_lock);
    return -1;
  } else {
    file_close(f);
    lock_release(&filesys_lock);
    return process_execute(file_name);
  }
}

void exit_proc(int status) {
  struct list_elem *e;

  for_list(e, &thread_current()->parent->children) {
    struct child *f = list_entry(e, struct child, elem);
    if (f->tid == thread_current()->tid) {
      f->used = true;
      f->exit_error = status;
    }
  }

  thread_current()->exit_error = status;

  if (thread_current()->parent->waiting == thread_current()->tid)
    sema_up(&thread_current()->parent->child_lock);

  thread_exit();
}

void *check_addr(const void *vaddr) {
  if (!is_user_vaddr(vaddr)) {
    exit_proc(-1);
    return 0;
  }

  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr) {
    exit_proc(-1);
    return 0;
  }

  return ptr;
}

struct proc_file *list_search(struct list *files, int fd) {
  struct list_elem *e;

  for_list(e, files) {
    struct proc_file *f = list_entry(e, struct proc_file, elem);
    if (f->fd == fd)
      return f;
  }
  return NULL;
}

void close_file(struct list *files, int fd) {
  struct list_elem *e;
  struct proc_file *f = NULL;

  for_list(e, files) {
    f = list_entry(e, struct proc_file, elem);
    if (f->fd == fd) {
      file_close(f->ptr);
      list_remove(e);
    }
  }

  free(f);
}

void close_all_files(struct list *files) {
  struct list_elem *e;

  while (!list_empty(files)) {
    e = list_pop_front(files);
    struct proc_file *f = list_entry(e, struct proc_file, elem);
    file_close(f->ptr);
    list_remove(e);
    free(f);
  }
}
