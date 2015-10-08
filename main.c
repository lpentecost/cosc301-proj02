
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
/*
Welcome to Lillie & Sam's Bash !! (Cat Emoji)

Lillie worked mostly on these things:

modifying tokenify
setting up execution of commands for both modes
setting up change_mode
Additional Stage 1 features (commenting, corner cases, etc.)

Sam worked mostly on these things:

But you rarely see us apart so it's pretty damn collaborative (two Cat Emojis)
*/

char **tokenify(const char *s, char* delim) {
    char *s_copy1 = strdup(s);
    char *s_copy2 = strdup(s);
    char *next_token = strtok(s_copy1, delim);
    int token_count = 0;
    if (next_token == NULL){
         char **tokens_null = malloc(sizeof(char*));
         tokens_null[0] = NULL;
         return tokens_null;
    }
    while (next_token != NULL){
         token_count++;
         next_token = strtok(NULL, delim);
    }
    char **tokens = malloc((token_count+1)*sizeof(char*));
    char *this_token = strtok(s_copy2, delim);
    char *next = strdup(this_token);
    tokens[0] = next;
    int i = 1;
    while (i < token_count){
         this_token = strtok(NULL, delim);
         next = strdup(this_token);
         tokens[i] = next;
         i++;
    }
    tokens[token_count] = NULL;
    free(s_copy1);
    free(s_copy2);   
    return tokens; 
}    

void free_tokens(char **tokens) {
    int i=0;
    while (tokens[i] != NULL){
        free(tokens[i]);
        i++;
    }
    free(tokens);
}

void free_cmds(char ***cmd, int num_cmd){
    for(int i=0; i<num_cmd; i++){
        free_tokens(cmd[i]);
    }    
    free(cmd);
}

void change_mode(bool *mode, char *new_mode){
    if((strcmp(new_mode, "s")==0) || (strcmp(new_mode, "sequential")==0)){
        *mode = true;
    } else if ((strcmp(new_mode, "p")==0) || (strcmp(new_mode, "parallel")==0)){
        *mode = false;
    } else{
        printf("Invalid mode switch, current mode is: ");
        if (*mode){
            printf("Sequential\n");
        } else{
            printf("Parallel\n");
        }
    }
}

bool execute_line(char **tokens, bool *mode){
    //form set of commands instead of tokens, check if any are exit commands and set flag, then execute commands based on which mode we are in    
    int num_commands = 0;
    int n = 0;
    bool is_running = true;
    while (tokens[n] != NULL){
        num_commands++;
        n++;
    }
    if (num_commands == 0){
        return true;
    }
    char ***commands = malloc(num_commands*sizeof(char**));
    for (int i = 0; i<num_commands; i++){
        commands[i] = tokenify(tokens[i], " \t\n");
    }
    bool did_mode_change = false;
    char *new_mode = NULL;
    for (int i=0; i<num_commands; i++){
        char **this_cmd = commands[i];
        if (this_cmd[0] == NULL){
            printf("Empty Command?\n");
            continue;
        }
        if (strcmp(this_cmd[0], "exit")==0){
            is_running = false;
        } else if (strcmp(this_cmd[0], "mode")==0){
            if ((this_cmd[2] != NULL) || (this_cmd[1] == NULL)){
                printf("Invalid mode switch\n");
            }else{
                did_mode_change = true;
                new_mode = this_cmd[1];
            }
        }else{
            pid_t  pid = fork();
            int childrv = 0;
            if (pid == 0){
                if(execv(this_cmd[0], this_cmd) <0){
                    printf("execv failed\n");
                    printf("Invaid command was: %s\n", this_cmd[0]);
                }
            } else {
                if (*mode){
                    pid = wait(&childrv);
                }
            }
            if (*mode == false){
                pid = wait(&childrv);
            }
        }
    }
    if (did_mode_change){
        change_mode(mode, new_mode);
    }
    free_cmds(commands, num_commands);
    return is_running;
}

int main(int argc, char **argv) {
    bool current_mode = true; //use true = sequential, false = parallel
    bool is_running = true;
    int buff_size = 1024;
    char *current_line = malloc(sizeof(char)*buff_size);
    char **tokened = NULL;
    while (is_running){
        printf(">>");
        fflush(stdout);
        char *result = fgets(current_line, sizeof(char)*buff_size, stdin);
        if (result != NULL && (strcmp(current_line, "\n")!=0)){
            for(int i = 0; i <strlen(current_line); i++){
                if(current_line[i] == '#'){
                    current_line[i] = '\0'; //set to null char to end string here and avoid comment
                    break;
                }
            }
            tokened = tokenify(current_line, ";");
            //printf("Test getting tokens, first command is %s \n", tokened[0]);
            is_running = execute_line(tokened, &current_mode);
            free_tokens(tokened);
        }
    }
    free(current_line);
    return 0;
}

