#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define VAL1 1.5
#define VAL2 4.0

/*
 * POSIX thread functions do not return error numbers in errno,
 * but in the actual return value of the function call instead.
 * This macro helps with error reporting in this case.
 */
#define perror_pthread(ret, msg) \
  do { errno = ret; perror(msg); } while (0)

/*
 * A (distinct) instance of this structure
 * is passed to each thread
 */
struct thread_info_struct {
  pthread_t tid; /* POSIX thread id, as returned by the library */

  double *arr; /* Pointer to array to manipulate */
  int len;
  double mul;

  int thrid; /* Application-defined thread id */
  int thrcnt;
};

int safe_atoi(char *s, int *val) {
  long l;
  char *endp;

  l = strtol(s, &endp, 10);
  if (s != endp && *endp == '\0') {
    *val = l;
    return 0;
  } else
    return -1;
}

void *safe_malloc(size_t size) {
  void *p;

  if ((p = malloc(size)) == NULL) {
    fprintf(stderr, "Out of memory, failed to allocate %zd bytes\n",
      size);
    exit(1);
  }

  return p;
}

void usage(char *argv0) {
  fprintf(stderr, "Usage: %s thread_count array_size\n\n"
    "Exactly two argument required:\n"
    "    thread_count: The number of threads to create.\n"
    "    array_size: The size of the array to run with.\n",
    argv0);
  exit(1);
}


/* Start function for each thread */
void *thread_start_fn(void *arg) {
  int i;

  /* We know arg points to an instance of thread_info_struct */
  struct thread_info_struct *thr = arg;

  fprintf(stderr, "Thread %d of %d. START.\n", thr->thrid, thr->thrcnt);

  for (i = thr->thrid; i < thr->len; i += thr->thrcnt)
    thr->arr[i] *= thr->mul;

  fprintf(stderr, "Thread %d of %d. END.\n", thr->thrid, thr->thrcnt);

  return NULL;
}

int main(int argc, char *argv[]) {
  double *arr;
  int i, ret, arrsize, thrcnt;
  struct thread_info_struct *thr;

  /*
   * Parse the command line
   */
  if (argc != 3) {
    usage(argv[0]);
  }

  if (safe_atoi(argv[1], &thrcnt) < 0 || thrcnt <= 0) {
    fprintf(stderr, "`%s' is not valid for `thread_count'\n", argv[1]);
    exit(1);
  }

  if (safe_atoi(argv[2], &arrsize) < 0 || arrsize <= 0) {
    fprintf(stderr, "`%s' is not valid for `array_size'\n", argv[2]);
    exit(1);
  }

  /*
   * Allocate and initialize big array of doubles
   */
  arr = safe_malloc(arrsize * sizeof(*arr));

  for (i = 0; i < arrsize; i++) {
    arr[i] = VAL1;
  }

  /*
   * Multiply it by VAL2 using thrcnt threads, in parallel
   */
  thr = safe_malloc(thrcnt * sizeof(*thr));

  for (i = 0; i < thrcnt; i++) {
    /* Initialize per-thread structure */
    thr[i].arr = arr;
    thr[i].len = arrsize;
    thr[i].mul = VAL2;
    thr[i].thrid = i;
    thr[i].thrcnt = thrcnt;

    /* Spawn new thread */
    ret = pthread_create(&thr[i].tid, NULL, thread_start_fn, &thr[i]);
    if (ret) {
      perror_pthread(ret, "pthread_create");
      exit(1);
    }
  }

  /*
   * Wait for all threads to terminate
   */
  for (i = 0; i < thrcnt; i++) {
    ret = pthread_join(thr[i].tid, NULL);
    if (ret) {
      perror_pthread(ret, "pthread_join");
      exit(1);
    }
  }

  /*
   * Verify resulting values
   */
  for (i = 0; i < arrsize; i++)
    if (arr[i] != VAL1 * VAL2) {
      fprintf(stderr, "Internal error: arr[%d] = %f, not %f\n",
        i, arr[i], VAL1 * VAL2);
      exit(1);
  }

  printf("OK.\n");

  return 0;
}
