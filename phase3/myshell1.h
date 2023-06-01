#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#define MAXLINE 8192


/* Static variables */
char **cmd_history;
int last = -1;
FILE* history_log;


/* Declarations */
void restore_log();
char* convert(char* tmp, char* buf);
void record(char* buf);
void show_all();
void show_nth(int n);
int wrapped_chdir(char* path);


/* Implementations */
/* Restore logs from 'history_log.txt" file. */
void restore_log(){
    char tmplog[MAXLINE];
    while(fgets(tmplog, (int)MAXLINE, history_log)){
        if (last < 0) {
            last++;
            cmd_history = (char**)malloc(sizeof(char*) * (last + 1));
            cmd_history[last] = (char*)malloc(sizeof(char) * (strlen(tmplog) + 1));
            strcpy(cmd_history[last], tmplog);
        }
        else if (strcmp(cmd_history[last], tmplog)) {  // Duplicate handling.
            last++;
            cmd_history = (char**)realloc(cmd_history, sizeof(char*) * (last + 1));
            cmd_history[last] = (char*)malloc(sizeof(char) * (strlen(tmplog) + 1));
            strcpy(cmd_history[last], tmplog);
        }
    }
}

/* Convert !! or !# command inputs. */
char* convert(char* tmp, char* buf){

    char* recent = "";
    if (last >= 0)
        recent = cmd_history[last];
    int i, j, k;
    int n;
    int offset;
    int max_j = strlen(buf) - 1;

    i = j = k = 0;
    while (j < max_j){
        // Find "!!"
        if (j+1 < max_j && buf[j] == '!' && buf[j+1] == '!'){
            if (last < 0)
                return "";

            // Successfully find
            offset = strlen(recent) - 1;
            strncpy(tmp + i, recent, offset);
            i += offset;
            j += 2;
        }
        // Find "!#"
        else if (buf[j] == '!'){
            k = 0;
            char numstr[MAXLINE] = "";
            while (++j < max_j && isdigit(buf[j]))
                numstr[k++] = buf[j];
            n = atoi(numstr) - 1;

            // Successfully find
            if (0 <= n && n <= last){
                offset = strlen(cmd_history[n]) - 1;
                strncpy(tmp + i, cmd_history[n], offset);
                i += offset;
            }
            else
                return "";
        }
        else {
            tmp[i] = buf[j];
            i++;
            j++;
        }
    }    
    tmp[i] = '\n';
    return tmp;
}

/* Record each input buffer */
void record(char* buf){
    
    if (last < 0) {
        last++;
        cmd_history = (char**)malloc(sizeof(char*) * (last + 1));
        cmd_history[last] = (char*)malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(cmd_history[last], buf);
        fprintf(history_log, "%s", buf);
        fflush(history_log);
    } 
    else if (strcmp(cmd_history[last], buf)) {  // Duplicate handling
        last++;
        cmd_history = (char**)realloc(cmd_history, sizeof(char*) * (last + 1));
        cmd_history[last] = (char*)malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(cmd_history[last], buf);
        fprintf(history_log, "%s", buf);
        fflush(history_log);
    }
}

/* Memory free which occupied by Recorded-buffer */
void free_record(){
    for (int i=0; i <= last; i++) 
        free(cmd_history[i]);
    free(cmd_history);
    last = -1;
}

/* Show all cmd history */
void _history(){
    for (int i=0; i<=last; i++)
        printf("%-4d %s", i+1, cmd_history[i]);
}

/* "cd" command implementation */
int _cd(char* path) {
    int check;
    
    if (path == NULL)
        if((check = chdir(getenv("HOME"))) == 0) return 1;
    else if((check = chdir(path)) == 0) return 1;
    else { 
        printf("bash: cd: %s: No such file or directory\n", path);
        return 0;
    }
}