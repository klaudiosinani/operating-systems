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
#define TASK_NAME_SZ 60 /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

/*--------------- Structures. ---------------*/

typedef struct proc
{
  pid_t PID;

  int id;

  char priority_status;
  char name[TASK_NAME_SZ];

  struct proc * next;

} process;

/*--------------- Global Variables. ---------------*/

static int Proc_num;
static process * dummy;
static process * present_proc;

/*--------------- Functions. ---------------*/

/*
 * Insertion of a process into our
 * linked list!
 */
static void
insert(int id, pid_t p, char * name) {
    process * dummy = (struct proc *) malloc(sizeof(struct proc));

  dummy->id = id;
  dummy->PID = p;
  dummy->priority_status = 'l';

  strcpy(dummy->name,name);

  dummy->next = present_proc->next;
  present_proc->next = dummy;
  present_proc = dummy;
}

/*
 * Removal of a process
 * from our linked list!
 */
static void
withdraw(int id) {
  dummy = present_proc;

  while (dummy->next->id != id)
    dummy=dummy->next;

  dummy->next = dummy->next->next;
}

static void
do_process(char * executable) {
  char * newargv[] = {executable ,NULL, NULL, NULL};
  char * newenviron[] = {NULL};

  /*
   * Stoping of a process!
   */
  raise(SIGSTOP);

  /*
   * Program execution!
   */
  execve( executable , newargv, newenviron);

  /*
   * Return on error on behave of execve()!
   */
  perror("scheduler: child: execve");
  exit(1);
}

/*
 * Print a list of all
 * tasks currently being scheduled.
 */
static void
sched_print_tasks(void) {
  dummy = present_proc;

  char * priority;

  if(dummy->priority_status == 'h')
  {
    priority = "high";
  }
  else
  {
    priority = "low";
  }

  printf("Process INFO: ID: %d | PID: %d | Name: %s | Priority: %s -- !Present Process!\n", dummy->id, dummy->PID, dummy->name, priority);

  while(dummy->next != present_proc)
  {
      dummy = dummy->next;

      if(dummy->priority_status == 'h')
      {
          priority = "high";
      }
      else
      {
          priority = "low";
      }
      printf("Process INFO: ID: %d | PID: %d | Name: %s | Priority: %s\n", dummy->id, dummy->PID, dummy->name, priority);
  }
}

/*
 * Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id) {
  dummy = present_proc;

  /*
   * Process Searching!
   */
  while (dummy->id != id)
  {
    dummy = dummy->next;

    if(dummy == present_proc)
      return -1;
  }

  printf("Process-child: %d, bearing PID: %d, was terminated.\n", id, dummy->PID);

  kill(dummy->PID, SIGKILL);
  return(id);
}

/*
 * Creating a new task.
 */
static void
sched_create_task(char * executable) {
  Proc_num++;

  printf("Process under creation! Process named: %s, with ID: %d \n", executable, Proc_num-1);

  /*
   * Create process!
   */
  pid_t p = fork();

  if (p<0)
  {
    perror("fork");
  }
  if (p==0)
  {
    do_process(executable);
  }
  else
  {
    /*
     * Process insertion to linked list!
     */
    insert(Proc_num-1, p, executable);
  }
}

static int
sched_high_priority(int id) {
  dummy = present_proc;

  while (dummy->id != id)
  {
    dummy = dummy->next;

    if(dummy == present_proc)
      return -1;
  }

  dummy->priority_status = 'h';

  printf("!PRIORITY CHANGE! > Process: %s | ID: %d | PID: %d < !HAS NOW HIGH PRIORITY!\n", dummy->name, dummy->id, dummy->PID);

  return(dummy->id);
}

static int
sched_low_priority(int id) {
  dummy = present_proc;

  while (dummy->id != id)
  {
    dummy = dummy->next;

    if(dummy == present_proc)
      return -1;
  }

  dummy->priority_status = 'l';

  printf("!PRIORITY CHANGE! > Process: %s | ID: %d | PID: %d < !HAS NOW LOW PRIORITY!\n", dummy->name, dummy->id, dummy->PID);

  return(dummy->id);
}

/*
 * Process requests
 * by the shell.
 */
static int
process_request(struct request_struct *rq) {
  switch (rq->request_no)
  {
    case REQ_PRINT_TASKS:
      sched_print_tasks();
      return 0;

    case REQ_KILL_TASK:
      return sched_kill_task_by_id(rq->task_arg);

    case REQ_EXEC_TASK:
      sched_create_task(rq->exec_task_arg);
      return 0;

    case REQ_HIGH_TASK:
      return sched_high_priority(rq->task_arg);

    case REQ_LOW_TASK:
      return sched_low_priority(rq->task_arg);

    default:
      return -ENOSYS;
  }
}

/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum) {
  /* ----- Removed! -----
   * assert(0 && "Please fill me!");
   */

  kill(present_proc->PID, SIGSTOP);
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

    //explain_wait_status(p, status);

    while(present_proc->PID != p)
      present_proc = present_proc->next;

    if (WIFEXITED(status) || WIFSIGNALED(status))
    {
      /*
       * The appearence of the following message indicates
       * the termination of a process-child.
       */
      if(present_proc->next == present_proc)
      {
        printf("The total number of processes-children was terminated.\n");
        exit(0);
      }

      /*
       * Otherwise continue to the
       * removal of the process from
       * our linked list!
       */
      withdraw(present_proc->id);
    }

    if (WIFSTOPPED(status));
      dummy = present_proc->next;

      while(dummy->priority_status != 'h')
      {
        dummy = dummy->next;

        if(dummy == present_proc)
        {
          if(present_proc->priority_status == 'h')
          {
            break;
          }
          else
          {
            dummy = dummy->next;
            break;
          }
        }
      }

      present_proc = dummy;

      char * priority;

      if(present_proc->priority_status == 'l')
      {
        priority = "low";
      }
      else
      {
        priority = "high";
      }

      printf("!PROCESS ACTIVATION! > Process: %s | ID: %d | PID: %d | Priority: %s <\n", present_proc->name, present_proc->id, present_proc->PID, priority);

      alarm(SCHED_TQ_SEC);
      kill(present_proc->PID, SIGCONT);
  }
}

/*
 * Disable delivery of
 * SIGALRM and SIGCHLD.
 */
static void
signals_disable(void) {
  sigset_t sigset;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGALRM);
  sigaddset(&sigset, SIGCHLD);

  if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0)
  {
    perror("signals_disable: sigprocmask");
    exit(1);
  }
}

/*
 * Enable delivery of
 * SIGALRM and SIGCHLD.
 */
static void
signals_enable(void) {
  sigset_t sigset;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGALRM);
  sigaddset(&sigset, SIGCHLD);

  if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0)
  {
    perror("signals_enable: sigprocmask");
    exit(1);
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
do_shell(char *executable, int wfd, int rfd) {
  char arg1[10], arg2[10];
  char *newargv[] = {executable, NULL, NULL, NULL};
  char *newenviron[] = {NULL};

  sprintf(arg1, "%05d", wfd);
  sprintf(arg2, "%05d", rfd);

  newargv[1] = arg1;
  newargv[2] = arg2;

  raise(SIGSTOP);
  execve(executable, newargv, newenviron);

  /*
   * execve() only returns on error
   */
  perror("scheduler: child: execve");
  exit(1);
}

/*
 * Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char * executable, int * request_fd, int * return_fd) {
  pid_t p;
  int pfds_rq[2], pfds_ret[2];

  if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0)
  {
    perror("pipe");
    exit(1);
  }

  p = fork();
  if (p < 0)
  {
    perror("scheduler: fork");
    exit(1);
  }

  if (p == 0)
  {
    /*
     * Child
     */
    close(pfds_rq[0]);
    close(pfds_ret[1]);

    /*
    * Create the shell!
    */
    do_shell(executable, pfds_rq[1], pfds_ret[0]);
    assert(0);
  }

  /*
   * Parent!
   */
  present_proc=(struct proc *) malloc(sizeof(struct proc));
  present_proc->PID = p;

  strcpy(present_proc->name,SHELL_EXECUTABLE_NAME);

  present_proc->priority_status = 'h';

  present_proc->next = present_proc;

  close(pfds_rq[1]);
  close(pfds_ret[0]);
  *request_fd = pfds_rq[0];
  *return_fd = pfds_ret[1];
}

static void
shell_request_loop(int request_fd, int return_fd) {
  int ret;
  struct request_struct rq;

  /*
   * Keep receiving requests from the shell.
   */
  for (;;)
  {
    if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
      perror("scheduler: read from shell");
      fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
      break;
    }

    signals_disable();
    ret = process_request(&rq);
    signals_enable();

    if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret))
    {
      perror("scheduler: write to shell");
      fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
      break;
    }
  }
}

/*--------------- Main Function. ---------------*/

int main(int argc, char *argv[]) {
  /*
   * Two file descriptors for
   * communication with the shell
   */
   static int request_fd, return_fd;

  pid_t p;

  int i;

  /*
   * Total number
   * of processes!
   */
  Proc_num = argc;

  /*
   * Shell creation
   * is happenning!
   */
  sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);

  for (i=1; i < Proc_num; i++)
  {
    p = fork();

    if (p < 0)
    {
      perror("fork");
      exit(1);
    }
    if (p == 0)
    {
      /*
       * Hand the executable program!
       */
      do_process(argv[i]);
    }
    else
    {
      /*
       * Carefully place the
       * process into our linked list!
       */
      insert(i,p,argv[i]);
    }
  }

  /*
   * Wait for all children
   * to raise SIGSTOP before exec()ing.
   */  wait_for_ready_children(Proc_num);

  /*
   * Install SIGALRM and
   * SIGCHLD handlers.
   */
  install_signal_handlers();

  if (Proc_num == 0)
  {
    fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
    exit(1);
  }
  else
  {
    alarm(SCHED_TQ_SEC);
    kill(present_proc->PID, SIGCONT);
  }

  shell_request_loop(request_fd, return_fd);

  /*
   * Now that the shell is gone, just loop forever
   * until we exit from inside a signal handler.
   */
  while (pause())
    ;

  /* Unreachable */
  fprintf(stderr, "Internal error: Reached unreachable point\n");
  return 1;
}
