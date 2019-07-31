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
  /* Child A*/
    printf("A created! \n");
    change_pname("A");
    pid = fork();

    if (pid < 0) {
      perror("A: fork");
      exit(1);
    }

    if (pid==0) {  /*Child B of A*/
      change_pname("B");
      printf("B created! \n");
      pid = fork();

      if (pid < 0) {
        perror("B: fork");
        exit(1);
      }

      if (pid==0) {  /*Child D of B*/
        change_pname("D");
        printf("D created! \n");
        printf("D: Sleeping...\n");
                    sleep(SLEEP_PROC_SEC);  /*D is leaf - sleeping*/
        printf("D: Exiting...\n");
        exit(13);
      }  /*Father B of D*/

      printf("B: Waiting... \n");
      pid = wait(&status);  /*Father B waiting for his children to terminate*/
      explain_wait_status(pid,status);
      printf("B: Exiting...\n");
      exit(19);
    }

    pid = fork();

    if (pid < 0) {
      perror("A: fork");
      exit(1);
    }

    if (pid==0) {  /*Child C of A*/
      change_pname("C");
      printf("C created! \n");
      printf("C: Sleeping...\n");
      sleep(SLEEP_PROC_SEC);  /*C is leaf - sleeping*/
      printf("C: Exiting...\n");
      exit(17);
    }  /*Father A of B-C*/

    printf("A: Waiting... \n");
    pid = wait(&status);  /*Father A waiting for his two children to terminate*/
    explain_wait_status(pid, status);
    pid = wait(&status);
    explain_wait_status(pid, status);
    printf("A: Exiting...\n");
    exit(16);
  }

  sleep(SLEEP_TREE_SEC); /*sleep until all procedures of tree created*/
  /* Print the process tree root at pid */

  show_pstree(pid);
  /* Wait for the root of the process tree (A) to terminate */

  pid = wait(&status);
  explain_wait_status(pid, status);
  return 0;
}
