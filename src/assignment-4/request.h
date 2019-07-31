#ifndef REQUEST_H_
#define REQUEST_H_

#include <unistd.h>
#include <sys/types.h>

/* request ids */
enum request_enum {
  REQ_PRINT_TASKS,  /* print tasks */
  REQ_KILL_TASK,    /* kill ->task_arg task */
  REQ_EXEC_TASK,    /* execute ->exec_task_arg with priority ->prio_arg */
  REQ_HIGH_TASK,    /* set ->task_arg to be of high priority */
  REQ_LOW_TASK,     /* set ->task_arg to be of low priority */
};

#define EXEC_TASK_NAME_SZ 60

/* Structure describing system call. */
struct request_struct {
  /* System call number */
  enum request_enum request_no;

  /*
   * System call arguments.
   * Can you think of a better organization for these fields?
   * Contact the lab assistants before any changes to the structure.
   */
  int task_arg;
  char exec_task_arg[EXEC_TASK_NAME_SZ];
};

#endif /* REQUEST_H_ */
