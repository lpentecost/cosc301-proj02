
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

struct node {
  char* value;
  struct node* next;
};

void check_paths(struct node* paths, char** token) {
// this function takes a linked list of possible paths, a command string and updates the command list so that all commands are valid
  char * this_cmd = *token;
    struct stat statresult;
    int rv = stat(this_cmd, &statresult);
    if (rv<0){
      // stat failed, must try again with paths
      printf("Stat failed. File %s doesn't exist\n", this_cmd);
      char* new_path = malloc(sizeof(char)*32);
      bool no_match = true;
      while(no_match && paths!=NULL) {
        //struct node* t_path = paths;
        printf("\n\n");
        new_path = strcat(paths->value,this_cmd);
        printf("tokens[i]:  %s\n", this_cmd);
        printf("t_path.value : %s\n", paths->value);
        struct stat statresult;
        rv = stat(new_path, &statresult);
        if (rv<0) { // still not a match
          printf("Stat failed. File %s doesn't exist\n", new_path);
          paths = paths->next;
        }
        else{
          printf("Stat succeeded. File %s exists\n", new_path);
          no_match = false;
          //char** temp = &this_cmd;
          //this_cmd = malloc(sizeof(char)*(strlen(new_path)+1)); 
          //strcpy(this_cmd,new_path); // copy valid command into commands
          //free(temp);
          *token = new_path;
        }
      }   
    }
}


void free_linked_list(struct node* head) {
    // this function frees all nodes in a linked list, including the value string
    while(head != NULL) {
        struct node *tmp = head;
        head = head->next;
        free(tmp->value);
        free(tmp);
    }
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

void create_paths(struct node** head) {
// given a head to a linked list, this function returns the head of a linked list with the possible paths as nodes
    int num_paths =7;
    char* paths[7] = {"/bin/\0","/usr/bin/\0","/usr/sbin/\0","/sbin/\0","/usr/local/bin/\0","./\0","/usr/games/\0"};
    
    for (int i = 0; i <num_paths; i++) {
        struct node* new_node = (struct node *)malloc(sizeof(struct node));
        new_node->value = strdup(paths[i]);
        new_node->next = *head;
        *head = new_node;
    }
}

int main() {
//  struct stat statresult;
//  char path[] = "/bin";
//  strcat(path,command);
//  printf("Path: %s\n", path);
//  int rv = stat("/bin/ls", &statresult);
//  if (rv<0){
//    printf("Stat failed. File doesn't exist\n");
//  }
//  else{
//    printf("Stat succeeded. File exists\n");
//  }
 
  struct node* paths = NULL;
  create_paths(&paths);


  //char** tokens = malloc(num_tokens*sizeof(char*));
  char *token = "ls";
  char **tokens = &token;
  //tokens[1] = "echo";
  //tokens[2] = "ls";

  check_paths(paths, tokens);
 // free_tokens(tokens);
  free_linked_list(paths);
  return 0;
}





