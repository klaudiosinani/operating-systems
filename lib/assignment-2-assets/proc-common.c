#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "proc-common.h"

void
wait_forever(void) {
  do {
    sleep(100);
  } while (1);
}

/*
 * This function performs some not-so-useful computation.
 * Its amount is determined by the value of count.
 */
void compute(int count) {
  long i;
  volatile long junk;

  junk = 0;
  for (i = 0; i < count * 1000000; i++) {
    junk++;
  }
}

/*
 * Changes the process name, as appears in ps or pstree,
 * using a Linux-specific system call.
 */
void
change_pname(const char *new_name) {
  int ret;
  ret = prctl(PR_SET_NAME, new_name);

  if (ret == -1){
    perror("prctl set_name");
    exit(1);
  }
}

/*
 * This function receives an integer status value,
 * as returned by wait()/waitpid() and explains what
 * has happened to the child process.
 *
 * The child process may have:
 *    * been terminated because it received an unhandled signal (WIFSIGNALED)
 *    * terminated gracefully using exit() (WIFEXITED)
 *    * stopped because it did not handle one of SIGTSTP, SIGSTOP, SIGTTIN, SIGTTOU
 *      (WIFSTOPPED)
 *
 * For every case, a relevant diagnostic is output to standard error.
 */
void explain_wait_status(pid_t pid, int status) {
  if (WIFEXITED(status)) {
    fprintf(stderr, "My PID = %ld: Child PID = %ld terminated normally, exit status = %d\n",
      (long)getpid(), (long)pid, WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
    fprintf(stderr, "My PID = %ld: Child PID = %ld was terminated by a signal, signo = %d\n",
      (long)getpid(), (long)pid, WTERMSIG(status));
  } else if (WIFSTOPPED(status)) {
    fprintf(stderr, "My PID = %ld: Child PID = %ld has been stopped by a signal, signo = %d\n",
      (long)getpid(), (long)pid, WSTOPSIG(status));
  } else {
    fprintf(stderr, "%s: Internal error: Unhandled case, PID = %ld, status = %d\n",
      __func__, (long)pid, status);
    exit(1);
  }

  fflush(stderr);
}

/*
 * Make sure all the children have raised SIGSTOP,
 * by using waitpid() with the WUNTRACED flag.
 *
 * This will NOT work if children use pause() to wait for SIGCONT.
 */
void wait_for_ready_children(int cnt) {
  int i;
  pid_t p;
  int status;

  for (i = 0; i < cnt; i++) {
    /* Wait for any child, also get status for stopped children */
    p = waitpid(-1, &status, WUNTRACED);
    explain_wait_status(p, status);

    if (!WIFSTOPPED(status)) {
      fprintf(stderr, "Parent: Child with PID %ld has died unexpectedly!\n",
        (long)p);
      exit(1);
    }
  }
}

/*
 * Print the process tree rooted at process with PID p.
 */
void show_pstree(pid_t p) {
  int ret;
  char cmd[1024];

  snprintf(cmd, sizeof(cmd), "echo; echo; pstree -G -c -p %ld; echo; echo",
    (long)p);

  cmd[sizeof(cmd)-1] = '\0';
  ret = system(cmd);

  if (ret < 0) {
    perror("system");
    exit(104);
  }
}

/*
 * Create a shared memory area, usable by all descendants of the calling process.
 */
void *create_shared_memory_area(unsigned int numbytes) {
  int pages;
  void *addr;

  if (numbytes == 0) {
    fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
    exit(1);
  }

  /* Determine the number of pages needed, round up the requested number of pages */
  pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

  /* Create a shared, anonymous mapping for this number of pages */
  addr = mmap(NULL, pages * sysconf(_SC_PAGE_SIZE),
    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  if (addr == MAP_FAILED) {
    perror("create_shared_memory_area: mmap failed");
    exit(1);
  }

  return addr;
}
