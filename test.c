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


struct job {
  int process_id;
  char* command;
  bool running;
  struct job* next;
};

struct job *list_delete(int pid, struct job *list) {
  struct job *head = list;
  //check for first node, then iterate through
  if (head->process_id == pid){
	  struct job *tmp = head;
	  head = head->next;
	  free(tmp);
	  return head;
  }
  while((list -> next) != NULL){
	  if (list->next->process_id == pid){
	    struct job *tmp = list->next;
	    list->next = list->next->next;
	    free(tmp);
	    return head;
	  } else {list = list->next;}
  }
  return head;
}




/*Helper function for expire job, given a pointer to a linked list of jobs and a node that is to be deleted, this function deletes the node
*/

/*
void delete_job(struct job* expired_job, struct job** head) {
  if (expired_job->process_id == (*head)->process_id) {
    struct job* temp = *head;
    *head = (*head)->next;
    free(temp);
    return; 
  }
  struct job* current = (*head)->next;
  struct job* previous = *head;
  while (current != NULL && previous != NULL) {
    if (expired_job->process_id == current->process_id) {
      struct job* temp = current;
      previous->next = current->next;
      free(temp);
      return; 
    }
    current = current->next;
    previous = previous->next;
  }
  return;
}
*/




int main() {
  struct job* jobs = NULL;
  jobs = job_insert("Happy", 12345, jobs);
  jobs = job_insert("Marry", 32145, jobs);
  jobs = job_insert("Bob", 12246, jobs);
  job_print(jobs);
  expire_job(12345, &jobs);
  printf("After deletions\n");
  job_print(jobs);
  return 0;
}


