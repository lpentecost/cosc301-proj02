
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

void check_paths(struct node* paths, char*** commands, int num_commands) {
// this function takes a linked list of possible paths, a list of commands, and the size of the list and updates the command list so that all commands are valid
  for (int i = 0; i < num_commands;i++) {
    struct stat statresult;
    int rv = stat(*commands[i], &statresult);
    if (rv<0){
      // stat failed, must try again with paths
      printf("Stat failed. File %s doesn't exist\n", *commands[i]);
      struct node t_path = *paths;
      bool no_match = true;
      int attempts = 0;
      while(no_match && attempts < 7) {
        char* new_path = strcat(t_path.value, *commands[i]);
        struct stat statresult;
        rv = stat(new_path, &statresult);
        if (rv<0) { // still not a match
          printf("Stat failed. File %s doesn't exist\n", new_path);
          t_path = *paths->next;
        }
        else{
          printf("Stat succeeded. File %s exists\n", new_path);
          no_match = false;
          *commands[i] = strdup(new_path); // copy valid command into commands
        }
        attempts++;
      }   
    }
  }
}

void create_paths(struct node** head) {
// given a head to a linked list, this function returns the head of a linked list with the possible paths as nodes
    int num_paths =7;
    char* paths[7] = {"/bin/","/usr/bin/","/usr/sbin/","/sbin/","/usr/local/bin/","./","/usr/games/"};
    
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
  int num_commands = 1;

  char*** command = malloc(num_commands*sizeof(char**));

  *command[0] = "ls";
   printf("hi\n");
  *command[1] = "echo 'blah'";
  *command[2] = "ls -l";

  check_paths(paths, command,1);
  free(command[0]);
  return 0;
}





