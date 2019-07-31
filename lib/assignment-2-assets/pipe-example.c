#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"

void child(int fd) {
  double val;

  printf("Child: My PID is %ld. Receiving a double value from the parent.\n",
    (long)getpid());

  if (read(fd, &val, sizeof(val)) != sizeof(val)) {
    perror("child: read from pipe");
    exit(1);
  }

  printf("Child: received value %f from the pipe. Will now compute.\n", val);
  compute(1000);
  exit(7);
}

int main(void) {
  pid_t p;
  int pfd[2];
  int status;
  double something;

  printf("Parent: Creating pipe...\n");

  if (pipe(pfd) < 0) {
    perror("pipe");
    exit(1);
  }

  printf("Parent: Creating child...\n");
  p = fork();

  if (p < 0) {
    /* fork failed */
    perror("fork");
    exit(1);
  }

  if (p == 0) {
    /* In child process */
    child(pfd[0]);
    /*
     * Should never reach this point,
     * child() does not return
     */
    assert(0);
  }

  /*
   * In parent process.
   */

  /* Write a value into the pipe */
  something = 1234.567;

  if (write(pfd[1], &something, sizeof(something)) != sizeof(something)) {
    perror("parent: write to pipe");
    exit(1);
  }

  /* Wait for the child to terminate */
  printf("Parent: Created child with PID = %ld, waiting for it to terminate...\n",
    (long)p);
  p = wait(&status);
  explain_wait_status(p, status);

  printf("Parent: All done, exiting...\n");

  return 0;
}
