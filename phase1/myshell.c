/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

#include "myshell1.h"


/* Functions */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 


int main() {
    char cmdline[MAXLINE]; /* Command line */

    /* Save history_log.txt to memory */
    history_log = fopen("history_log.txt", "a+");
    restore_log();

    while (1) {
	    /* Read */
        printf("CSE4100-MP-P1> ");                   
        Fgets(cmdline, MAXLINE, stdin); 
        if (feof(stdin)){
            // File pointer close.
            fclose(history_log);
            exit(0);
        }
        /* Evaluate */
        eval(cmdline);
    }    
}
/* $end shellmain */
  
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) {
    char *argv[MAXARGS];  /* Argument list execve() */
    char buf[MAXLINE];    /* Holds modified command line */
    int bg;               /* Should the job run in bg or fg? */
    pid_t pid;            /* Process id */

    strcpy(buf, cmdline);

    /* Replace !! and !# commands in buffer */
    char tmp[MAXLINE] = "";
    strcpy(tmp, convert(tmp, buf));
    if (strcmp(tmp, "")) {
        strcpy(buf, tmp);
        record(buf);  // record logs.
    }

    bg = parseline(buf, argv);

    if (argv[0] == NULL) return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        /* Execute other commands by fork() */
        if ((pid = fork()) == 0) {
            if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        /* Parent waits for foreground job to terminate */
        if (!bg) { 
            int status;
            /* Reaping */
            if (wait(&status) < 0){
                unix_error("waitfg: waitpid error");
            }
        }
        else//when there is backgrount process!
            printf("%d %s", pid, cmdline);
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) {

    if (!strcmp(argv[0], "exit")) {/* quit command */
        fclose(history_log);  // File pointer close
        free_record();        // Memory free of history log array
	    exit(0);
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	    return 1;
    
    if (!strcmp(argv[0], "history")) { 
        _history();
        return 1;
    }

    if (!strcmp(argv[0], "cd")) {
        _cd(argv[1]);
        return 1;
    }
    return 0;                     /* Not a builtin command */
}
/* $end eval */


/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) {

    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
                buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	    return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;

    return bg;
}
/* $end parseline */
