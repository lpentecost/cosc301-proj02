
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

char **tokenify(const char *s) {
    char *space = " \t\n";
    char *s_copy1 = strdup(s);
    char *s_copy2 = strdup(s);
    char *next_token = strtok(s_copy1, space);
    int token_count = 0;
    if (next_token == NULL){
         char **tokens_null = malloc(sizeof(char*));
         tokens_null[0] = NULL;
         return tokens_null;
    }
    while (next_token != NULL){
         token_count++;
         next_token = strtok(NULL, space);
    }
    char **tokens = malloc((token_count+1)*sizeof(char*));
    char *this_token = strtok(s_copy2, space);
    char *next = strdup(this_token);
    tokens[0] = next;
    int i = 1;
    while (i < token_count){
         this_token = strtok(NULL, space);
         next = strdup(this_token);
         tokens[i] = next;
         i++;
    }
    tokens[token_count] = NULL;
    free(s_copy1);
    free (s_copy2);   
    return tokens; 
}                                                                          

bool execute_line(char ***tokens, bool *mode){
    
    if (*mode){
    
    }else{
    
    }
    return false;
    //Given current mode and current set of tokens, this function will:
    //split tokens into commands
    //execute commands according to current mode
    //execv(path, args[]);
    //update mode, if applicable
    //return whether or not an exit command was called
}

int main(int argc, char **argv) {
    bool current_mode = true; //use true = sequential, false = parallel
    bool is_running = true;
    int buff_size = 1024;
    char *current_line = malloc(sizeof(char)*buff_size);
    char **tokened = NULL;
    while (is_running){
        printf(">>");
        fgets(current_line, sizeof(char)*buff_size, stdin);
        tokened = tokenify(current_line);
        printf("Test getting tokens, first token is %s \n", tokened[0]);
        is_running = execute_line(&tokened, &current_mode);
        free(tokened);
    }
    free(current_line);
    return 0;
}

