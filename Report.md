<center>
    <h3>OS Project 2<br>
        Final Report</h3>
    <h5>11612803 CHENG Yiwei &nbsp&nbsp 11612831 GUAN Hao</h5>
</center>

##### 1.Summary

We basically did the job together. Specifically, I (Yiwei) did  the main parts of modification on the `thread`, argument passing and the process syscall part and test-case-oriented debugging and I (Hao)  did the main parts of file syscall, reconstruct the code and left comments. The good thing is that we can pass all the tests while we believe the system is still not robust enough since what we did after finishing the skeleton is just for passing the tests we couldn't.

##### 2.Memory

The problems in the code related to the memory are mostly caused by the test cases which intentionally put the data range in dangerous area. For other situations we haven;t observed such problems.

##### 3. Style

This is fixed much in the reconstructing part.

##### 4.Understandable

Yes, we left many comments in the code to help understand.

##### 5.

Yes, as stated above.

##### 6.

We believe we have given enough comments.

##### 7. Code Reuse

No, we did it at the initial stage but they are reasonably extracted now.

##### 8. Line Length

No.

##### 9. Reimplementation of Library

No. We just use what the lib has given.



### Report on Each Task

#### 1.Argument Parsing

##### Data structure and Function

```C
//Mainly this function is modified
static bool setup_stack (void **esp, const char *file_name);
```

##### Algorithms

To realize argument parsing, we use `strtok_r()` to tokenize the input string and count the number of arguments at the same time, then we used an array `argv` to store the content of the arguments and at the same time decrease the stack pointer to store them.  Additional stack push operation is needed to make the address a 4's multiple and add representing the end of the arguments.

##### Synchronization

There shouldn't be synchronization issues.

##### Rationale

We think this is a straightforward and effective way to do the job, so it's good.

#### 2. Process Control Syscalls

##### Data Structures and Functions

```c
struct thread{
	...
	/* For process.c and syscall.c. */
	bool success;
    int exit_error;
    int waiting;
    struct list children;
    struct thread *parent;        
    struct semaphore child_lock;  /* Mutex for child process. */
    
	...
}

/* Light-weight data structure for child thread. */
struct child {
  tid_t tid;              /* Thread identifier. */
  bool used;              /* Used status. */
  int exit_error;         /* Exited with error code. */
  struct list_elem elem;  /* List element. */
};

exit_proc(int status);
check_addr(const void *vaddr); // Check the address to see if it's and illegal one. If it's illegal, exit the process.
```

##### Algorithms

The syscalls should all be in `syscall.c/syscall_handler()`, to pass the tests, we add many lines of checking address, which we think may not be a good way but it at least helped to pass.

###### Practice

Take the args[1] , increment it and then store it into the `eax`.

###### Halt

Simply call `shut_down_power_off()`

###### Exec

Call `process_execute()` to execute, before that ...... and in `process_execute()`, a new thread is created through `thread_create()` and decreasing the semaphore of the current thread.

###### Exit

`exit_proc()` is called where the process's exit status is assigned and stuffs about parent and child processes is done.

###### Wait

Look for a child in the current thread's children list whose `tid` equals the given `tid`. If one does not exist, return -1. Otherwise, call `sema_down ()` on that parent's semaphore. Then, return its `exit_error`and remove its `list_elem` from `child` list.

And correspondingly, a thread will call `sema_up ()` on his parent when he exits in `process_exit()` to wake up the parent.

##### Synchronization

The semaphore operation has the most things to do with synchronization, however we made sure that every `sema_down()` is effective and there will be a `sema_up()` at the reasonable position through the test cases.

##### Rationale

We decided to create a new struct to hold information about child processes that need to persist even after the child is terminated

#### 3.File Operation Syscalls

##### Data Structures and Functions

```c
struct thread{
    ...
    /* Owned by filesys/file.c. */
    struct file *self;
    struct list files;
    int fd_count;
    ...
}
//Information for a file
struct proc_file {
    struct file *ptr;
    int fd;
    struct list_elem elem;
}; 
struct lock filesys_lock; // 
```

##### Algorithms

Before any operation, the `file_lock` should be acquired to ensure that no process can interrupt the process. And  release the `file_lock` afterwards.

**Create**:  Sse `filesys_create()` to create the file.

**Remove**: Get the file's name from the `f->esp` and use ` filesys_remove()` to remove.

**Open**: Use `filesys_open()` to open the file, if failed set `eax` to -1, otherwise,  create a `proc_file` for the file and assign it with some initial values, increase the `fd_count` of the current thread and then add the file into the thread's `files` list, then return the `fd` in the eax.

**Filesize**: Simply call `file_length()` and return the value in the eax.

**Read**: First check the `fd`, if it's 0, use `input_getc()` to read the content into the buffer, else first get the pointer to the `proc_file` containing the file's information using `list_search()` , return -1 if not found and call `file_read()` if found.

**Write**: First check the `fd`, if it's 1, use `putbuf()` to write the output, else follow the similar steps in `read` part but just change the call function into `file_write()`.

**Seek**: Simply call `file_seek()` for the current file's fd following the similar steps.

**Tell**: Simply call `file_tell()` for the current file's fd following the similar steps.

**Close** : Call `file_close()` for the file and remove it from the file list of the current thread.

##### Synchronization

The lock is acquired at the start and release at the end of the process so there should not be synchronization problems.

##### Rationale

The most operations are done by the existing functions so for functions there is nothing to talk about. As for data structures, we add a file list to the thread struct which made our implementation simpler.







