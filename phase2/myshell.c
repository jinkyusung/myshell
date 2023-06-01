/* $begin shellmain */
#include "csapp.h"
#include <errno.h>
#define MAXARGS   128

#include "myshell1.h"
#include <string.h>
#define WRITE   1
#define READ    0

/* Function prototypes */
static void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

/* Static variable */
int is_pipeline = 0;


int main() {
    char cmdline[MAXLINE]; /* Command line */
    history_log = fopen("history_log.txt", "a+");
    restore_log();

    while (1) {
	    /* Read */
        printf("CSE4100-MP-P2> ");                   
        Fgets(cmdline, MAXLINE, stdin); 
        if (feof(stdin)){
            fclose(history_log);
            exit(0);
        }

        /* Evaluate */
        eval(cmdline);
        is_pipeline = 0;
    }    
}
/* $end shellmain */
  

/* $begin eval */
/* eval - Evaluate a command line */
static void eval(char *cmdline) {

    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */

    strcpy(buf, cmdline);

    char tmp[MAXLINE] = "";
    strcpy(tmp, convert(tmp, buf));
    if (strcmp(tmp, "")) {
        strcpy(buf, tmp);
        record(buf);
    }

    /* Pipeline implementations */
    pid_t pid;
    int fd[2];             // File descipter
    int fd_backup;         // Back-up
    
    int max_idx = 0;       // the number of commands.
    int idx;               // index of { char* splited_bufs[MAXBUF] }
    
    /* Split buffer by "|" and Save these to array */
    char* splited = strtok(buf, "|");
    char* splited_bufs[MAXBUF];

    while (splited != NULL) {
        splited_bufs[max_idx++] = splited;
        splited = strtok(NULL, "|");
    }

    if (max_idx > 1)
        is_pipeline = 1;

    /* For all commands, piping them by using fd[2] */
    for (idx = 0; idx < max_idx; idx++) {

        bg = parseline(splited_bufs[idx], argv);

        if (argv[0] == NULL) return;  /* Ignore empty lines */
        if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
            
            if (pipe(fd) == -1) 
                fprintf(stderr, "error");

            if ((pid = fork()) == 0) {
                /* Child process run. */
                dup2(fd_backup, STDIN_FILENO);
                if (idx + 1 < max_idx) /* Not last argv */
                    dup2(fd[WRITE], STDOUT_FILENO);
                close(fd[READ]);
                
                /* History constructs pipeline */
                if (!strcmp(argv[0], "history")) {
                    for (int i=0; i<=last; i++)
                        /* Output of history becomes next process input */
                        dprintf(fd[WRITE], "%-4d %s", i+1, cmd_history[i]);
                    exit(1);
                }
                if (execvp(argv[0], argv) < 0) { //ex) /bin/ls ls -al &
                    printf("%s: Command not found.\n", argv[0]);
                    exit(1);
                }
            }
            else {
                /* Parent process run. */
                if (!bg) {
                    int status;
                    if (wait(&status) < 0)
                        unix_error("waitfg: waitpid error");
                    close(fd[WRITE]);
                    fd_backup = fd[READ];
                }
                else //when there is backgrount process!
                    printf("%d %s", pid, cmdline);
            }
        }
    }
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) {
    if (!strcmp(argv[0], "exit")) {/* quit command */
        fclose(history_log);
        free_record();
	    exit(0);
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	    return 1;
    
    if (!strcmp(argv[0], "history") && !is_pipeline){  // pipeline handling.
        _history();
        return 1;
    }

    if (!strcmp(argv[0], "cd")){
        _cd(argv[1]);
        return 1;
    }
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) {
    char* tmp;
    char* delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        /* Do not split string in "__" by ' ' */
        if (*buf == '"'){
            tmp = ++buf;
            while (*tmp && (*tmp != '"'))
                tmp++;
            delim = tmp;
        }
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
