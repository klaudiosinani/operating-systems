#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_SEC 10

int main(void) {
  pid_t p;
  int status;

  fprintf(stderr, "Parent, PID = %ld: Creating child...\n",
    (long)getpid());
  p = fork();
  if (p < 0) {
    /* fork failed */
    perror("fork");
    exit(1);
  }
  if (p == 0) {
    /* In child process */
    change_pname("child");
    sleep(SLEEP_SEC);
    exit(101);
  }

  change_pname("father");
  /*
   * In parent process. Wait for the child to terminate
   * and report its termination status.
   */
  printf("Parent, PID = %ld: Created child with PID = %ld, waiting for it to terminate...\n",
    (long)getpid(), (long)p);
  p = wait(&status);
  explain_wait_status(p, status);

  printf("Parent: All done, exiting...\n");

  return 0;
}
