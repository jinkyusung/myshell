#include <stdio.h>
#include <signal.h>
#include <unistd.h>


typedef struct {
    int order;           /* order in Jobs */
    pid_t job_id;        /* process group id */
    char state;          /* f - foreground, r - running, s - stopped, t - terminated */
    char argv[MAXLINE];  /* command string for printing */
} job_t;


/* Static variable */
volatile int max_job = 0;
volatile sig_atomic_t pid;        /* pid > 0 then child process already done */
volatile sig_atomic_t child_pid;  /* Signal target pid */
volatile sig_atomic_t is_bg;      /* bg cmd flag */
volatile sig_atomic_t is_fg;      /* fg cmd flag */


int status;          /* Reaping state */
job_t Jobs[MAXBUF];  /* job list */


/* Declarations */
void sigint_handler(int);
void sigchld_handler(int);
void sigtstp_handler(int);

void _kill(int);
void _bg(int);
void _fg(int);
void _jobs();
void add_jobs(pid_t job_id, char state, char** argv);


/* Custom handlers */
void sigtstp_handler(int sig) {
    is_bg = 0;
    is_fg = 0;
    int i;
    int target = -1;  // target non-exists
    sigset_t mask, prev;
    Sigfillset(&mask);
    Sigprocmask(SIG_BLOCK, &mask, &prev);
    for (i = 0; i < max_job; i++) {
        if (Jobs[i].state == 'f') {
            target = i;
            break;
        }
    }
    if (target > -1) {
        // target exists, send signal
        child_pid = Jobs[target].job_id;
        pid = child_pid;
        Jobs[target].state = 's';
        Kill(child_pid, SIGSTOP);
    }
    Sigprocmask(SIG_SETMASK, &prev, NULL);
}


void sigint_handler(int sig) {
    int i;
    int target = -1;  // target non-exists
    sigset_t mask, prev;
    Sigfillset(&mask);
    Sigprocmask(SIG_BLOCK, &mask, &prev);
    for (i = 0; i < max_job; i++) {
        if (Jobs[i].state == 'f') {
            target = i;
            break;
        }
    }
    if (target > -1) {
        // target exists, send signal
        child_pid = Jobs[target].job_id;
        pid = child_pid;
        Jobs[target].state = 't';
        Kill(child_pid, SIGKILL);
    }
    Sigprocmask(SIG_SETMASK, &prev, NULL);
}


void sigchld_handler(int sig) {
    if (is_bg) {
        is_bg = 0;
    }
    else if (is_fg){
        is_fg = 0;    
    }
    else if (child_pid > 0) {
        // Reaping a terminated job.
        int i;
        int target = -1;
        pid = Waitpid(-1, &status, WUNTRACED);
        if (!WIFSTOPPED(status)) { /* Get SIGSTOP case - NO Remove */
            for (i = 0; i < max_job; i++) {
                if (Jobs[i].job_id == pid) {
                    target = i;
                    break;
                }
            }
            if (target > -1) {     /* Otherwise - Remove from Jobs */
                for (i = target; i < max_job; i++) {
                    if (i + 1 < MAXLINE) {
                        Jobs[i] = Jobs[i + 1];
                    }
                }
                max_job--;
            }
        }
    }
}


/* Built-in commands implementations */

/* Force terminate to some job */
void _kill(int target) {
    /* Invalid job information */
    int target_idx = -1;
    for (int i = 0; i < max_job; i++) {
        if (Jobs[i].order == target) {
            target_idx = i;
            break;
        }
    }
    /* Signal sending */
    if (target_idx > -1) {
        child_pid = Jobs[target_idx].job_id;
        Kill(child_pid, SIGKILL);
    }
    else {
        printf("kill: no such job\n");
        fflush(stdout);
    }
}

/* bg %# command implementation */
void _bg(int target){
    /* Invalid job information */
    int target_idx = -1;
    for (int i = 0; i < max_job; i++) {
        if (Jobs[i].order == target) {
            target_idx = i;
            break;
        }
    }

    if (target_idx > -1 && Jobs[target_idx].state == 's') {
        is_bg = 1;
        /* Convert foreground job to background job */
        char tmp = ' ';
        Jobs[target_idx].state = 'r';
        if (target_idx == max_job - 2) tmp = '-';
        if (target_idx == max_job - 1) tmp = '+';
        printf("[%d]%c  Running  %s\n", Jobs[target_idx].order, tmp, Jobs[target_idx].argv);
        /* Signal sending */
        child_pid = Jobs[target_idx].job_id;
        pid = child_pid;
        Kill(child_pid, SIGCONT);
    }
    else {
        printf("bg: no such job\n");
        fflush(stdout);
    }
}

/* fg %# command implementation */
void _fg(int target) {
    /* Invalid job information */
    int target_idx = -1;
    for (int i = 0; i < max_job; i++) {
        if (Jobs[i].order == target) {
            target_idx = i;
            break;
        }
    }

    if (target_idx > -1 && Jobs[target_idx].state != 'f') {
        is_fg = 1;
        /* Convert background job to foreground job */
        char tmp = ' ';
        Jobs[target_idx].state = 'f';
        if (target_idx == max_job - 2) tmp = '-';
        if (target_idx == max_job - 1) tmp = '+';
        printf("[%d]%c  Running  %s\n", Jobs[target_idx].order, tmp, Jobs[target_idx].argv);
        /* Signal sending */
        child_pid = Jobs[target_idx].job_id;
        pid = child_pid;
        Kill(child_pid, SIGCONT);
        pid = 0;
        while (!pid)
            ;
        is_fg = 0;
    }
    else {
        printf("fg: no such job\n");
        fflush(stdout);
    }
}

/* Show all jobs */
void _jobs(){
    int i;
    char tmp = ' ';
    char tmp_state[11];

    for (i = 0; i < max_job; i++){
        /* Mark recent jobs */
        if (i == max_job - 2)
            tmp = '-';
        if (i == max_job - 1)
            tmp = '+';

        /* Mark State of jobs*/
        switch (Jobs[i].state) {
            case 'f':
            case 'r':
                strcpy(tmp_state, "Running"); 
                break;
            case 's':
                strcpy(tmp_state, "Stopped");
                break;
            case 't':
                strcpy(tmp_state, "Terminated");
                break;
        }
        printf("[%d]%c  %s  %s\n", Jobs[i].order, tmp, tmp_state, Jobs[i].argv);
    }
}

/* Add a job to "Jobs[]" : array of job_t structure */
void add_jobs(pid_t job_id, char state, char** argv){    

    int max_order;
    if (max_job == 0) max_order = 1;
    else max_order = Jobs[max_job - 1].order + 1;

    Jobs[max_job].order = max_order;
    Jobs[max_job].job_id = job_id;
    Jobs[max_job].state = state;

    child_pid = job_id;

    /* Concate given char *argv[] to char* (i.e. string) */
    while (*argv != NULL) {        
        strcat(Jobs[max_job].argv, " ");
        strcat(Jobs[max_job].argv, *argv); 
        argv++;
    }
    max_job++;
}
