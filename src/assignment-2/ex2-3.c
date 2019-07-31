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
  int i,status;
  pid_t pid, *pid_child;
  pid = fork();

  if (pid<0) {  //Error
    printf("%s :",node->name);
    perror("fork");
    exit(-1);
  }

  if (pid==0) {
    //message for starting
    printf("Name %s, PID = %ld ,starting... \n",node->name,(long)getpid());
    change_pname(node->name);  //change process name
    pid_child = (pid_t *)malloc((node->nr_children)*sizeof(pid_t));

    for (i=0; i<node->nr_children; i++) {
      //store in an array the pids of children
      pid_child[i] = make_proc_tree(node->children+i);
      /* Every process as a father waits for child to
       * be ready (sigstop) before the next (DFS) */
      wait_for_ready_children(1);
    }

    //then stops until SIGCONT
    raise(SIGSTOP);
    //SIGCONT - get's awake - message
    printf("Name %s, PID = %ld is awake\n",node->name,(long)getpid());

    for (i=0; i<node->nr_children; i++) {
      pid = pid_child[i];
      //sends a SIGCONT message to every child
      kill(pid,SIGCONT);
      //then waits for the child to terminate
      pid = wait(&status);
      explain_wait_status(pid, status);
    }

    //and then exit
    printf("Name %s, PID = %ld, exiting... \n",node->name,(long)getpid());
    exit(0);
  }
  return pid;
}

int main(int argc, char *argv[]) {
  pid_t pid;
  int status;
  struct tree_node *root;

  if (argc < 2) {
    fprintf(stderr, "Please, note the proper usage: %s <input_tree_file>\n\n", argv[0]);
    exit(1);
  }

  //get tree (tree_node)
  root = get_tree_from_file(argv[1]);
  //returns pid of root
  pid = make_proc_tree(root);
  //wait for root process to be ready
  wait_for_ready_children(1);
  // Print the process tree root at pid
  show_pstree(pid);
  //send SIGCONT to root
  kill(pid,SIGCONT);
  // Wait for the root of the process tree to terminate
  pid = wait(&status);
  explain_wait_status(pid, status);

  return 0;
}
