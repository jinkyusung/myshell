# Project 1 / Phase 1  
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
When you execute this program, 

* __Step 1. Load existing history log__  
The history log file which name `history_log.txt`  is loaded on  
`char** cmd_history`  if the log file exists.  
Otherwise, Generates the log file.


* __Step 2. Get an user command input by `stdin`__  
If input command is `exit`, then this program be terminated.


* __Step 3. Replace `!!`, `!#` commands into appropriate commands.__  
Refer `char* convert(char* tmp, char* buf)` function in `myshell.h`.  


* __Step 4. Record converted commands to log file and memory.__  
Refer `void record(char*)` function in `myshell.h`


* __Step 5. Do appropriate action for each inputs.__  
Refer `int builtin_command(char**)` in `myshell.c`  
If `argv[0]` is not builtin command, then it is executed by using `fork()`, `execvp()`  
and terminated by using `wait()` functions.