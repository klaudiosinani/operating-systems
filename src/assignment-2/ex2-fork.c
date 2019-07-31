#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 * Create this process tree:
 * A-+-B---D
 *   `-C
 */
void fork_procs(void) {
  /*
   * initial process is A.
   */

  change_pname("A");
  printf("A: Sleeping...\n");
  sleep(SLEEP_PROC_SEC);

  /* ... */

  printf("A: Exiting...\n");
  exit(16);
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */
int main(void) {
  pid_t pid;
  int status;

  /* Fork root of process tree */
  pid = fork();
  if (pid < 0) {
    perror("main: fork");
    exit(1);
  }
  if (pid == 0) {
    /* Child */
    fork_procs();
    exit(1);
  }

  /*
   * Father
   */
  /* for ask2-signals */
  /* wait_for_ready_children(1); */

  /* for ask2-{fork, tree} */
  sleep(SLEEP_TREE_SEC);

  /* Print the process tree root at pid */
  show_pstree(pid);

  /* for ask2-signals */
  /* kill(pid, SIGCONT); */

  /* Wait for the root of the process tree to terminate */
  pid = wait(&status);
  explain_wait_status(pid, status);

  return 0;
}
