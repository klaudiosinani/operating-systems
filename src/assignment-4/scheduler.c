#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/*--------------- Compile-time parameters. ---------------*/

#define SCHED_TQ_SEC 2  /* time quantum */
#define TASK_NAME_SZ 60  /* maximum size for a task's name */

/*--------------- Global Variables. ---------------*/

int * ID;
int * Running;
int Present_proc;
int Proc_num;

/*--------------- Functions. ---------------*/

/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum) {
  /* ----- Removed! -----
   * assert(0 && "Please fill me!");
   */

    kill(ID[Present_proc], SIGSTOP);
}

/*
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum) {
  /* ----- Removed! -----
   * assert(0 && "Please fill me!");
   */

  pid_t p;

  int i;
  int procs_end;
  int status;

  for(;;)
  {
      p = waitpid(-1, &status, WUNTRACED | WNOHANG);

    /*
     * waitpid(pid_t pid, int *status, int options);
     * In the case of success, waitpid() gets us the process
     * ID of a child-process whose status condition has been modified.
     * The value -1 means, we wait for any child process whose process
     * group ID is equal to the absolute value of pid.
     * The constant WNOHANG means that we return immediately
     * if no child has exited, and the constant WUNTRACED
     * means that we also return if a child has stopped,
     * but not traced via ptrace(2). Status for traced children which have
     * stopped is provided even if this option is not specified.
     */

    if(p == 0)
      break;

    ID[Present_proc] = (int) p;

    if (WIFEXITED(status) || WIFSIGNALED(status))
    {
      /*
       * The appearence of the following message indicates
       * the termination of a process-child.
       */
      printf("The signal SIGCHLD was received. Process-child: %d, was terminated.\n\n", ID[Present_proc]);

      Running[Present_proc] = 0;
    }
    if (WIFSTOPPED(status))
    {
      /*
       * The appearence of the following message indicates
       * the pause of a process-child because a signal
       * forced it to do so.
       */
      printf("\nThe Process-child: %d, was paused.\n\n", ID[Present_proc]);
    }

    printf("Continuing onwards to: ");

    procs_end = 1;

    for (i = 0; i < Proc_num; i++)
    {
      if (Running[i] == 1)
        procs_end = 0;
    }

    if (procs_end == 1)
    {
      printf("\nThe total number of processes-children was terminated.\n");
      exit(0);
    }

    do
    {
      Present_proc = ((Present_proc + 1) % Proc_num);
    }
    while (Running[Present_proc] == 0);

    printf("%d.\n", Present_proc);

    alarm(SCHED_TQ_SEC);

    kill(ID[Present_proc], SIGCONT);
  }
}

/*
 * Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void) {
  sigset_t sigset;
  struct sigaction sa;

  sa.sa_handler = sigchld_handler;
  sa.sa_flags = SA_RESTART;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGCHLD);
  sigaddset(&sigset, SIGALRM);

  sa.sa_mask = sigset;
  if (sigaction(SIGCHLD, &sa, NULL) < 0)
  {
    perror("sigaction: sigchld");
    exit(1);
  }

  sa.sa_handler = sigalrm_handler;
  if (sigaction(SIGALRM, &sa, NULL) < 0)
  {
    perror("sigaction: sigalrm");
    exit(1);
  }

  /*
   * Ignore SIGPIPE, so that write()s to pipes
   * with no reader do not result in us being killed,
   * and write() returns EPIPE instead.
   */
  if (signal(SIGPIPE, SIG_IGN) < 0)
  {
    perror("signal: sigpipe");
    exit(1);
  }
}

static void
do_child(char * executable) {
  char * new_argv[] = {executable, NULL, NULL, NULL};
  char * new_environ[] = {NULL};

  /*
   * Stoping of a process!
   */
  raise(SIGSTOP);

  /*
   * Program execution!
   */
  execve(executable , new_argv, new_environ);

  /*
   * Return on error on behave of execve()!
   */
  perror("execve");
  exit(1);
}

/*--------------- Main Function. ---------------*/

int main(int argc, char * argv[]) {
  pid_t p;

  int i;

  /*
   * For each of argv[1] to argv[argc - 1],
   * create a new child process, add it to the process list.
   */

  Present_proc = 0;  /* Num of present proccess */

  Proc_num = argc - 1;  /* Total num of proccesses */

  ID = malloc((Proc_num) * sizeof(int));

  Running = malloc((Proc_num) * sizeof(int));

  for (i=0; i < Proc_num; i++)
  {
    printf("Forking activated: %d %d\n", argc, i);

    p = fork();

    if (p < 0)
    {
      perror("fork");
      exit(1);
    }
    if (p == 0)
    {
      do_child(argv[i + 1]);
    }
    else
    {
      ID[i] = (int) p;
      Running[i] = 1;
    }
  }

  /* Wait for all children to raise SIGSTOP before exec()ing. */
  wait_for_ready_children(Proc_num);

  /* Install SIGALRM and SIGCHLD handlers. */
  install_signal_handlers();

  if (Proc_num == 0)
  {
    fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
    exit(1);
  }
  else
  {
    alarm(SCHED_TQ_SEC);
    kill(ID[0], SIGCONT);
  }


  /* loop forever  until we exit from inside a signal handler. */
  while (pause())
    ;

  /* Unreachable */
  fprintf(stderr, "Internal error: Reached unreachable point\n");
  return 1;
}
