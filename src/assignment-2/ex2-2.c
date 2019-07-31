#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

pid_t make_proc_tree(struct tree_node *node) {
  int status,i;
  pid_t pid;

  pid = fork();

  if (pid<0){  //Error
    printf("%s :",node->name);
    perror("fork");
    exit(-1);
  }

  if (pid==0){
    // Change the process name
    change_pname(node->name);
    printf("%s : Created \n", node->name);  //Message "created"

    for (i=0; i<node->nr_children; i++)
      make_proc_tree(node->children+i);  //recursion to create children

    if (node->nr_children==0){      //if leaf sleep
      printf("%s: Sleeping...\n", node->name);
      sleep(SLEEP_PROC_SEC);
      printf("%s: Exiting...\n", node->name);
      exit(0);
    }
    else {
      printf("%s: Waiting...\n", node->name);  //else procedure is father of other procedures
      for(i=0; i<node->nr_children; i++){
         pid = wait(&status);      //Waiting for children to be terminated
        explain_wait_status(pid, status);
      }
       printf("%s: Exiting...\n", node->name);
      exit(0);
    }
  }
  return pid;
}

int main(int argc, char *argv[]) {
  pid_t pid;
  int status;
  struct tree_node *root;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
    exit(1);
  }

  root = get_tree_from_file(argv[1]);  //get tree (tree_node)
  pid = make_proc_tree(root);  //returns pid of root (the first call of the function)
  sleep(SLEEP_TREE_SEC);   //sleep until all procedures of tree created
  show_pstree(pid);  // Print the process tree root at pid
  pid = wait(&status);  // Wait for the root of the process tree to terminate
  explain_wait_status(pid, status);

  return 0;
}
