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





