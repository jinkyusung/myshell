# Project 1 / Phase 2  
Author : Jinkyu sung  
ID : 20191598  
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
Note that a basic sequence of program is equal to PHASE 1.

* Step1. Splits a buffer by `"|"`
Regard all substrings of buffer in `"__"` as a RAW string.  
So, DO NOT any preprocessing in this cases. Refer line 182 of `myshell.c`

* Step2. Constructs pipeline to connect each child processes  
If a given command is the `history` than execute in child process.  
Refer line 93 of `myshell.c`  
