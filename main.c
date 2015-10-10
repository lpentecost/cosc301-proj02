
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

path variability 
background processes
jobs linked list implementation, function etc
updating jobs linked list
pause/resume function
Additional Stage 2 features

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
// frees tokens in an array of tokens
    int i=0;
    while (tokens[i] != NULL){
        free(tokens[i]);
        i++;
    }
    free(tokens);
}

void free_cmds(char ***cmd, int num_cmd){
// frees commands in array of tokens
    for(int i=0; i<num_cmd; i++){
        free_tokens(cmd[i]);
    }    
    free(cmd);
}

// job struct: struct for each job. Stores p_id, command, and whether the process is currently running
struct job {
  pid_t process_id;
  char command[128];
  bool running;
  struct job* next;
};

struct job* job_find_by_command(char* c, struct job *head) {
  // modified from class notes, returns pointer to the node with value pid
  while(head!=NULL) {
    if (!strcasecmp(head->command, c)) {
      return head;
    }
    head = head->next;
  }
  return NULL;
}

struct job* job_find(pid_t pid, struct job *head) {
  // modified from class notes, returns pointer to the node with value pid
  while(head!=NULL) {
    if (head->process_id == pid) {
      return head;
    }
    head = head->next;
  }
  return NULL;
}

// list delete
struct job *delete_job_by_command(const char *c, struct job *head) {
    // handle special case of NULL list
    if (head == NULL) {
        return NULL;
    }

    // handle special case of deleting the head
    if (!strcasecmp(head->command, c)) {
        struct job *dead = head;
        head = head->next;
        free(dead);
        free(dead->command);
        return head;
    }
    struct job *tmp = head;
    while (tmp->next != NULL) {
        if (strcasecmp(tmp->next->command, c)) {
            struct job *dead = tmp->next;
            tmp->next = dead->next;
            free(dead);
            free(dead->command);
            return head;
        }
        tmp = tmp->next;
    }
    return head;
}


// job delete
struct job *delete_job(int pid, struct job *head) {
    // handle special case of NULL list
    if (head == NULL) {
        return NULL;
    }

    // handle special case of deleting the head
    if (head->process_id == pid) {
        struct job *dead = head;
        head = head->next;
        free(dead);
        free(dead->command);
        return head;
    }
    struct job *tmp = head;
    while (tmp->next != NULL) {
        if (tmp->next->process_id == pid) {
            struct job *dead = tmp->next;
            tmp->next = dead->next;
            free(dead);
            free(dead->command);
            return head;
        }
        tmp = tmp->next;
    }
    return head;
}

// prints out list of jobs
void job_print(const struct job *head) {
  int i = 0;
  char* bools[2] = {"Paused", "Running"};
  if (head == NULL) {
    printf("No processes currently running.\n");
  }else{
    printf("Printing Jobs...\n");
    while (head != NULL) {
      printf("Job %d.) \tCommand: %s\tProcess Id: %d\t Running: %s\n", ++i, head->command, head->process_id, bools[(head->running)]);
      head = head->next;
    }
  }
}

// deletes jobs that have completed
struct job* clean_up_jobs(struct job* head) {
  while (head!=NULL) {
    printf("Process_id : %d\t",head->process_id); 
    if (head->process_id == 0) {
      printf("Computer says 0\n");
      head = delete_job(head->process_id,head);
    }
    else{
      printf("Computer says NON-zero\n");
    }
    head = head->next;
  }
  return head;
}


struct job* job_insert(char *c, int p_id, struct job *head){
	//using list_insert_head2 as defined in class, passing in command & process ids as values, set running to true, modifies in place linked list of jobs
	
	struct job *new_job = malloc(sizeof(struct job));
	strcpy(new_job->command,c);
	new_job->process_id = p_id;
	new_job->next = head;
	new_job->running = true;
	return new_job;
}

struct node *list_insert(char *path, struct node *head){
	//using list_insert_head2 as defined in class, passing in word as value
	struct node *new_node = malloc(sizeof(struct node));
	path[strlen(path)-1] = '/';
	strcpy(new_node->value, path);
	new_node->next = head;
	return new_node;
}

void free_jobs(struct job *jobs){
// frees all command name in array of job structs along with the entire node
  while(jobs != NULL) {
    struct job *tmp = jobs;
    jobs = jobs->next;
    free(tmp);
    free(tmp->command);
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

// loads paths from filename
struct node *load_paths(const char *filename, int *num_paths) {
	  FILE *config = fopen(filename, "r");
	  int size = 132;
	  char *this_line = malloc(size*sizeof(char));
	  fgets(this_line, size, config);
    //printf("Path from file: %s\n", this_line);
	  *num_paths = 0;
	  //struct node *head = malloc(sizeof(struct node));
	  struct node *head = NULL;
          do{
		head = list_insert(this_line, head);
	    	*num_paths = *num_paths + 1; //dereference to indirectly modify
          }while (fgets(this_line, size, config) != NULL);
          free(this_line);
    fclose(config);
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
        //printf("Stat succeeded. File %s exists\n", new_path);
        match = true;
        char * temp = *token;
        *token = new_path;
        free(temp);
      }
    }   
  }
  else { // stat passed without additional path
    match = true;
  }
  return match;
}


bool execute_line(char **tokens, bool *mode, struct node *paths, struct job** jobs){
    //form set of commands instead of tokens, check if any are exit commands and set flag, then execute commands based on which mode we are in    
  int num_commands = 0;
  int n = 0;
  bool is_running = true;
  int num_forks = 0;
  //int pid;
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

 
  for (int i=0; i<num_commands; i++){ // loop through commands
    char **this_cmd = commands[i];
    if (this_cmd[0] == NULL){
      printf("Empty Command?\n");
      continue;
    }
    if (strcmp(this_cmd[0], "exit")==0){
      if (*jobs == NULL) {
        is_running = false;
      }else {
        printf("You can not exit when processes are running.\n");
      }
    } else if (strcmp(this_cmd[0], "mode")==0){
      if ((this_cmd[2] != NULL) || (this_cmd[1] == NULL)){
        if (*mode) {
          printf("The current mode is sequential.\n");
        }else {
          printf("The current mode is parallel.\n");
        } 
      }
      else{
        did_mode_change = true;
        new_mode = this_cmd[1];
      }
    } else if (strcmp(this_cmd[0], "jobs")==0){
      job_print(*jobs);
    } 
    else if (strcmp(this_cmd[0], "pause")==0) {
      pid_t pid_temp = atoi(this_cmd[1]);
      printf("pid_temp is : %d\n", pid_temp);
      struct job* temp = job_find(pid_temp, *jobs);
      //printf("Temp Command : %s\n",temp->command); 
      if (temp != NULL) {
        kill(temp->process_id,SIGSTOP);
        temp->running = false;
      }else {
        printf("This is not a current process.\n");
      }
    } else if (strcmp(this_cmd[0], "resume")==0) {
      pid_t pid_temp = atoi(this_cmd[1]);
      struct job* temp = job_find(pid_temp, *jobs);
      if (temp != NULL) {
        kill(temp->process_id, SIGCONT);
        temp->running = true;
      } else {
        printf("This is not a current process.\n");
      }
    }
    else{ // new process
      if (check_paths(paths, &this_cmd[0])){
        num_forks++;
        pid_t  pid = fork();// must create new job on linked list        
        int childrv = 0;
        if (pid == 0){
          if(execv(this_cmd[0], this_cmd) <0){
            printf("execv failed\n");
            printf("Invaid command was: %s\n", this_cmd[0]);
          }
        } 
        else {
          *jobs = job_insert(this_cmd[0], pid, *jobs);  // this_cmd[0]
//          job_print(jobs);
          if (*mode){ // sequential mode
            pid = wait(&childrv);
            printf("Process: %d\t completed\n",  pid);
          }
        }
      }else{ // execv failed
        printf("Invalid command was: %s\n", this_cmd[0]);
        *jobs = delete_job_by_command(this_cmd[0], *jobs); // UNCOMMENT ME
      }
    } 
  } // end of for loop
  
  if (did_mode_change){
    change_mode(mode, new_mode);
  }
  free_cmds(commands, num_commands);
  return is_running;
}

// prints lists
void list_print(const struct node *list) {
  int i = 0;
  printf("In list_print\n");
  while (list != NULL) {
    printf("List item %d: %s\n", i++, list->value);
    list = list->next;
  }
  printf("\n\n");
}

int main(int argc, char **argv) {
  char *filename = argv[1];
  //char *filename = "/home/csvm/cosc301/cosc301-proj02/shell-config";
  int num_paths = 0;
  struct node* paths = NULL;
  paths = load_paths(filename, &num_paths);
  struct job * jobs = NULL;
  bool current_mode = true; //use true = sequential, false = parallel
  bool is_running = true;
  int buff_size = 1024;
  char *current_line = malloc(sizeof(char)*buff_size);
  char **tokened = NULL;
  int pid = 0;
  while (is_running){
    struct pollfd pfd[1];
    pfd[0].fd=0;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
        
    poll(&pfd[0], 1, 1000);
    printf(">>");
    fflush(stdout);
    //jobs = clean_up_jobs(jobs);
    char *result = fgets(current_line, sizeof(char)*buff_size, stdin);
    if (result != NULL && (strcmp(current_line, "\n")!=0)){
      for(int i = 0; i <strlen(current_line); i++){
        if(current_line[i] == '#'){
          current_line[i] = '\0'; //set to null char to end string here and avoid comment
          break;
        }
      }
      tokened = tokenify(current_line, ";");
      is_running = execute_line(tokened, &current_mode, paths, &jobs);
      free_tokens(tokened);
    }
    else {
      printf("You did not enter a command.\n");
    }  
    // update jobs & check in on hanging jobs   
    while((pid=waitpid(-1,NULL, WNOHANG))>0) { // pid has returned, delete job node
      printf("Process: %d completed\n",  pid);
      jobs = delete_job(pid, jobs); 
    } 
    jobs = clean_up_jobs(jobs); 
  }
  free(current_line);
  free_linked_list(paths);
  free_jobs(jobs); //should be freed
  return 0;
}

