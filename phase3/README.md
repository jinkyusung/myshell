# Project 1 / Phase 3  
Author : Jinkyu sung  
ID     : 20191598  
Major  : Computer Science / Mathematics  

***
## Files
`csapp.{c,h}`  
> CS:APP3e functions

`myshell.c`  
> A file contains main() functions.

`myshell1.h`  
>`history` and `!!`, `!#` commands implementations.  
> `cd` command implementation.  

`myshell2.h`  
> Signal handlers and some built-in command implementations.  

`Makefile`  
>Makefile to compile.
---
## How to execute
```shell
> make
> ./myshell
```
---
## Overview  

Note that a basic sequence of program is equal to PHASE 1, 2.

### **_Signal handler_**
`SIGTSTP`  
: Find the foreground job and set state of job to 's(stopped)'.  
Send signal 'SIGSTOP' to target process.

`SIGINT`  
: Find the foreground job and set state of job to 't(terminated)'.  
Send signal 'SIGKILL' to target process.

`SIGCHLD`  
: Reaping killed job and Remove them from Jobs.

Remark; After receiving SIGKILL signal, SIGCHLD handler always be executed.
So, every reaping and removing from Jobs should run in SIGCHLD handler.


### **_Function_**
``` C
void _kill(int);
void _bg(int);
void _fg(int);
void _jobs();
void add_jobs(pid_t _pid, char state, char** argv);
```


* **Step1.** _All Non-built-in commands_ which must be executed by `fork()`  
add to Array, which is `job_t Jobs[MAXBUF]`.

* **Step2.** *If a job is background*. Then DO NOT wait SIGCHLD in parent process.  
background jobs only would be killed by built-in command `kill %#` or terminated by themselves.  
*Else THE job is foreground*. (Note that every foreground job is unique.)  
Then the algorithm which is equivalent to Phase1, 2 will run.

* **Step3.** In Case of the foreground job, `SIGINT` or `SIGTSTP` signal only 
affect to children of `myshell` process.

* **Step4.** *Jobs always be updated*. This mean all result of job commands  
such as *generating, terminating, suspending, or running* be updated for each call-up.
