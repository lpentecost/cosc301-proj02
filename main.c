
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

setting up path variability


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

struct node {
// basic struct for linked list, used to implement paths
    char value[128];
    struct node *next;
};


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

// job struct: struct for each job. Stores p_id, command, and whether the process is currently running
struct job {
  char process_id;
  char* command;
  bool running;
  struct job* next;
};

void free_jobs(struct job *jobs){
// frees all command name in array of job structs along with the entire node
  while(jobs != NULL) {
    free(jobs->command);
    struct job *tmp = jobs;
    jobs = jobs->next;
    free(tmp);
  }
}

void free_linked_list(struct node* head) {
    // this function frees all nodes in a linked list, including the value string
    while(head != NULL) {
        struct node *tmp = head;
        head = head->next;
        free(tmp->value);
        //free(tmp);
    }
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

struct node *list_insert(char *path, struct node *head){
	//using list_insert_head2 as defined in class, passing in word as value
	struct node *new_node = malloc(sizeof(struct node));
	path[strlen(path)-1] = '/';
	strcpy(new_node->value, path);
	new_node->next = head;
	return new_node;
}

struct node *load_paths(const char *filename, int *num_paths) {
	  FILE *config = fopen(filename, "r");
	  int size = 132;
	  char *this_line = malloc(size*sizeof(char));
	  fgets(this_line, size, config);
          printf("Path from file: %s\n", this_line);
	  *num_paths = 0;
	  //struct node *head = malloc(sizeof(struct node));
	  struct node *head = NULL;
          do{
		head = list_insert(this_line, head);
	    	*num_paths = *num_paths + 1; //dereference to indirectly modify
          }while (fgets(this_line, size, config) != NULL);
          free(this_line);
	  return head;
}


bool check_paths(struct node* paths, char** token) {
// this function takes a linked list of possible paths, a command string and updates the command list so that all commands are valid
    char * this_cmd = *token;
    struct stat statresult;
    bool match = false;
    int rv = stat(this_cmd, &statresult);
    if (rv<0){
      // stat failed on its own, must try with paths
      char* new_path = malloc(sizeof(char)*128);
      while(!match && paths!=NULL) {
        strcpy(new_path,paths->value);
        strcat(new_path,this_cmd);
        struct stat statresult;
        rv = stat(new_path, &statresult);
        if (rv<0) { // still not a match 
          paths = paths->next;
        }
        else{
          printf("Stat succeeded. File %s exists\n", new_path);
          match = true;
          char * temp = *token;
          *token = new_path;
          free(temp);
        }
      }   
    }
    return match;
}

bool execute_line(char **tokens, bool *mode, struct node *paths){
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
        } else{
            if (check_paths(paths, &this_cmd[0])){
                pid_t  pid = fork();
                int childrv = 0;
                if (pid == 0){
                    if(execv(this_cmd[0], this_cmd) <0){
                        printf("execv failed\n");
                        printf("Invaid command was: %s\n", this_cmd[0]);
                        printf("pid: %d\n command: %s\n", pid, this_cmd[0]);
                    }
                } else {
                    if (*mode){ // sequential mode
                        pid = wait(&childrv);
                        printf("pid: %d\n command: %s\n", pid, this_cmd[0]);
                    }
                }
                if (*mode == false){ // parallel mode
                    pid = wait(&childrv);
                }
            }else{
                printf("Invalid command was: %s\n", this_cmd[0]);
            }
        } 
    }
    if (did_mode_change){
        change_mode(mode, new_mode);
    }
    free_cmds(commands, num_commands);
    return is_running;
}

void list_print(const struct node *list) {
    int i = 0;
    printf("In list_print\n");
    while (list != NULL) {
        printf("List item %d: %s\n", i++, list->value);
        list = list->next;
    }
}

int main(int argc, char **argv) {
    char *filename = argv[1];
    int num_paths = 0;
    struct node* paths = NULL;
    paths = load_paths(filename, &num_paths);
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
            printf("Test getting tokens, first command is %s \n", tokened[0]);
            is_running = execute_line(tokened, &current_mode, paths);
            free_tokens(tokened);
        }
    }
    free(current_line);
    free_linked_list(paths);
    return 0;
}

