#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "proc-common.h"
#include "request.h"

#define SHELL_CMDLINE_SZ 100

void issue_request(int wfd, int rfd, struct request_struct *rq) {
  int ret;

  /* Issue the request */
  fprintf(stderr, "Shell: issuing request...\n");
  if (write(wfd, rq, sizeof(*rq)) != sizeof(*rq)) {
    perror("Shell: write request struct");
    exit(1);
  }

  /* Block until a reply has been received */
  fprintf(stderr, "Shell: receiving request return value...\n");

  if (read(rfd, &ret, sizeof(ret)) != sizeof(ret)) {
    perror("Shell: read request return value");
    exit(1);
  }

  if(ret < 0){
    fprintf(stderr, "Shell: request return value ret = %d\n", ret);
    fprintf(stderr, "       %s\n", strerror(-ret));
  }
}

/*
 * Read a command line from the stream pointed to by fp
 * [a standard C library stream, *not* a file descriptor],
 * strip any trailing newlines.
 */
void get_cmdline(FILE *fp, char *buf, int bufsz) {
  if (fgets(buf, bufsz, fp) == NULL) {
    fprintf(stderr, "Shell: could not read command line, exiting.\n");
    exit(1);
  }
  if (buf[strlen(buf) - 1] == '\n')
    buf[strlen(buf) - 1] = '\0';
}

/* print help */
void help(void) {
  printf(" ?          : print help\n"
         " q          : quit\n"
         " p          : print tasks\n"
         " k <id>     : kill task identified by id\n"
         " e <program>: execute program\n"
         " h <id>     : set task identified by id to high priority\n"
         " l <id>     : set task identified by id to low priority\n");
}

/*
 * Parse a command line, construct and
 * issue the relevant request to the scheduler.
 *
 * Parsing is very simple, a better way would be to
 * break up the command line in tokens.
 */
void process_cmdline(char *cmdline, int wfd, int rfd) {
  struct request_struct rq;

  if (strlen(cmdline) == 0 || strcmp(cmdline, "?") == 0){
    help();
    return;
  }

  /* Quit */
  if (strcmp(cmdline, "q") == 0 || strcmp(cmdline, "Q") == 0) {
    fprintf(stderr, "Shell: Exiting. Goodbye.\n");
    exit(0);
  }

  /* Print Tasks */
  if (strcmp(cmdline, "p") == 0 || strcmp(cmdline, "P") == 0) {
    rq.request_no = REQ_PRINT_TASKS;
    issue_request(wfd, rfd, &rq);
    return;
  }

  /* Kill Task */
  if ((cmdline[0] == 'k' || cmdline[0] == 'K') &&
      cmdline[1] == ' ') {
    rq.request_no = REQ_KILL_TASK;
    rq.task_arg = atoi(&cmdline[2]);
    issue_request(wfd, rfd, &rq);
    return;
  }

  /* Exec Task */
  if ((cmdline[0] == 'e' || cmdline[0] == 'E') && cmdline[1] == ' ') {
    rq.request_no = REQ_EXEC_TASK;
    strncpy(rq.exec_task_arg, &cmdline[2], EXEC_TASK_NAME_SZ);
    rq.exec_task_arg[EXEC_TASK_NAME_SZ - 1] = '\0';
    issue_request(wfd, rfd, &rq);
    return;
  }

  /* High-prioritize task */
  if ((cmdline[0] == 'h' || cmdline[0] == 'H') && cmdline[1] == ' ') {
    rq.request_no = REQ_HIGH_TASK;
    rq.task_arg = atoi(&cmdline[2]);
    issue_request(wfd, rfd, &rq);
    return;
  }

  /* Low-prioritize task */
  if ((cmdline[0] == 'l' || cmdline[0] == 'L') && cmdline[1] == ' ') {
    rq.request_no = REQ_LOW_TASK;
    rq.task_arg = atoi(&cmdline[2]);
    issue_request(wfd, rfd, &rq);
    return;
  }

  /* Parse error, malformed command, whatever... */
  printf("command `%s': Bad Command.\n", cmdline);
}

int main(int argc, char *argv[]) {
  int rfd, wfd;
  char cmdline[SHELL_CMDLINE_SZ];

  /*
   * Communication with the scheduler happens over two UNIX pipes.
   *
   * The scheduler first creates the pipes, then execve()s the shell
   * program. It passes two file descriptors as command-line arguments:
   *
   * argument 1: wfd: the file descriptor to write request structures into.
   * argument 2: rfd: the file descriptor to read request return values from.
   */

  if (argc != 3) {
    fprintf(stderr, "Shell: must be called with exactly two arguments.\n");
    exit(1);
  }

  wfd = atoi(argv[1]);
  rfd = atoi(argv[2]);
  if (!wfd || !rfd) {
    fprintf(stderr, "Shell: descriptors must be non-zero: wfd = %d, rfd = %d\n",
      wfd, rfd);
    exit(1);
  }

  /*
   * Forever: show the prompt, read a command line, then process it.
   */
  printf("\nThis is the Shell. Welcome.\n\n");

  for (;;) {
    printf("Shell> ");
    fflush(stdout);
    get_cmdline(stdin, cmdline, SHELL_CMDLINE_SZ);
    process_cmdline(cmdline, wfd, rfd);
  }

  /* Unreachable */
  fprintf(stderr, "Shell: reached unreachable point.\n");
  return 1;
}
