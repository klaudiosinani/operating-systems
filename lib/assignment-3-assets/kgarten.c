#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/*
 * POSIX thread functions do not return error numbers in errno,
 * but in the actual return value of the function call instead.
 * This macro helps with error reporting in this case.
 */
#define perror_pthread(ret, msg) \
  do { errno = ret; perror(msg); } while (0)

/* A virtual kindergarten */
struct kgarten_struct {

  /*
   * Here you may define any mutexes / condition variables / other variables
   * you may need.
   */

  /* ... */

  /*
   * You may NOT modify anything in the structure below this
   * point.
   */
  int vt;
  int vc;
  int ratio;

  pthread_mutex_t mutex;
};

/*
 * A (distinct) instance of this structure
 * is passed to each thread
 */
struct thread_info_struct {
  pthread_t tid; /* POSIX thread id, as returned by the library */

  struct kgarten_struct *kg;
  int is_child;  /* Nonzero if this thread simulates children, zero otherwise */

  int thrid;     /* Application-defined thread id */
  int thrcnt;
  unsigned int rseed;
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
  fprintf(stderr, "Usage: %s thread_count child_threads c_t_ratio\n\n"
    "Exactly two argument required:\n"
    "    thread_count: Total number of threads to create.\n"
    "    child_threads: The number of threads simulating children.\n"
    "    c_t_ratio: The allowed ratio of children to teachers.\n\n",
    argv0);
  exit(1);
}

void bad_thing(int thrid, int children, int teachers) {
  int thing, sex;
  int namecnt, nameidx;
  char *name, *p;
  char buf[1024];

  char *things[] = {
    "Little %s put %s finger in the wall outlet and got electrocuted!",
    "Little %s fell off the slide and broke %s head!",
    "Little %s was playing with matches and lit %s hair on fire!",
    "Little %s drank a bottle of acid with %s lunch!",
    "Little %s caught %s hand in the paper shredder!",
    "Little %s wrestled with a stray dog and it bit %s finger off!"
  };
  char *boys[] = {
    "George", "John", "Nick", "Jim", "Constantine",
    "Chris", "Peter", "Paul", "Steve", "Billy", "Mike",
    "Vangelis", "Antony"
  };
  char *girls[] = {
    "Maria", "Irene", "Christina", "Helena", "Georgia", "Olga",
    "Sophie", "Joanna", "Zoe", "Catherine", "Marina", "Stella",
    "Vicky", "Jenny"
  };

  thing = rand() % 4;
  sex = rand() % 2;

  namecnt = sex ? sizeof(boys)/sizeof(boys[0]) : sizeof(girls)/sizeof(girls[0]);
  nameidx = rand() % namecnt;
  name = sex ? boys[nameidx] : girls[nameidx];

  p = buf;
  p += sprintf(p, "*** Thread %d: Oh no! ", thrid);
  p += sprintf(p, things[thing], name, sex ? "his" : "her");
  p += sprintf(p, "\n*** Why were there only %d teachers for %d children?!\n",
    teachers, children);

  /* Output everything in a single atomic call */
  printf("%s", buf);
}

void child_enter(struct thread_info_struct *thr) {
  if (!thr->is_child) {
    fprintf(stderr, "Internal error: %s called for a Teacher thread.\n",
      __func__);
    exit(1);
  }

  fprintf(stderr, "THREAD %d: CHILD ENTER\n", thr->thrid);

  pthread_mutex_lock(&thr->kg->mutex);
  ++(thr->kg->vc);
  pthread_mutex_unlock(&thr->kg->mutex);
}

void child_exit(struct thread_info_struct *thr) {
  if (!thr->is_child) {
    fprintf(stderr, "Internal error: %s called for a Teacher thread.\n",
      __func__);
    exit(1);
  }

  fprintf(stderr, "THREAD %d: CHILD EXIT\n", thr->thrid);

  pthread_mutex_lock(&thr->kg->mutex);
  --(thr->kg->vc);
  pthread_mutex_unlock(&thr->kg->mutex);
}

void teacher_enter(struct thread_info_struct *thr) {
  if (thr->is_child) {
    fprintf(stderr, "Internal error: %s called for a Child thread.\n",
      __func__);
    exit(1);
  }

  fprintf(stderr, "THREAD %d: TEACHER ENTER\n", thr->thrid);

  pthread_mutex_lock(&thr->kg->mutex);
  ++(thr->kg->vt);
  pthread_mutex_unlock(&thr->kg->mutex);
}

void teacher_exit(struct thread_info_struct *thr) {
  if (thr->is_child) {
    fprintf(stderr, "Internal error: %s called for a Child thread.\n",
      __func__);
    exit(1);
  }

  fprintf(stderr, "THREAD %d: TEACHER EXIT\n", thr->thrid);

  pthread_mutex_lock(&thr->kg->mutex);
  --(thr->kg->vt);
  pthread_mutex_unlock(&thr->kg->mutex);
}

/*
 * Verify the state of the kindergarten.
 */
void verify(struct thread_info_struct *thr) {
  struct kgarten_struct *kg = thr->kg;
  int t, c, r;

  c = kg->vc;
  t = kg->vt;
  r = kg->ratio;

  fprintf(stderr, "            Thread %d: Teachers: %d, Children: %d\n",
          thr->thrid, t, c);

  if (c > t * r) {
    bad_thing(thr->thrid, c, t);
    exit(1);
  }
}

/*
 * A single thread.
 * It simulates either a teacher, or a child.
 */
void *thread_start_fn(void *arg) {
  /* We know arg points to an instance of thread_info_struct */
  struct thread_info_struct *thr = arg;
  char *nstr;

  fprintf(stderr, "Thread %d of %d. START.\n", thr->thrid, thr->thrcnt);

  nstr = thr->is_child ? "Child" : "Teacher";

  for (;;) {
    fprintf(stderr, "Thread %d [%s]: Entering.\n", thr->thrid, nstr);

    if (thr->is_child) {
      child_enter(thr);
    } else {
      teacher_enter(thr);
    }

    fprintf(stderr, "Thread %d [%s]: Entered.\n", thr->thrid, nstr);

    /*
     * We're inside the critical section,
     * just sleep for a while.
     */
    /* usleep(rand_r(&thr->rseed) % 1000000 / (thr->is_child ? 10000 : 1)); */
    pthread_mutex_lock(&thr->kg->mutex);
    verify(thr);
    pthread_mutex_unlock(&thr->kg->mutex);

    usleep(rand_r(&thr->rseed) % 1000000);

    fprintf(stderr, "Thread %d [%s]: Exiting.\n", thr->thrid, nstr);
    /* CRITICAL SECTION END */

    if (thr->is_child) {
      child_exit(thr);
    } else {
      teacher_exit(thr);
    }

    fprintf(stderr, "Thread %d [%s]: Exited.\n", thr->thrid, nstr);

    /* Sleep for a while before re-entering */
    /* usleep(rand_r(&thr->rseed) % 100000 * (thr->is_child ? 100 : 1)); */
    usleep(rand_r(&thr->rseed) % 100000);
    pthread_mutex_lock(&thr->kg->mutex);
    verify(thr);
    pthread_mutex_unlock(&thr->kg->mutex);
  }

  fprintf(stderr, "Thread %d of %d. END.\n", thr->thrid, thr->thrcnt);

  return NULL;
}


int main(int argc, char *argv[]) {
  int i, ret, thrcnt, chldcnt, ratio;
  struct thread_info_struct *thr;
  struct kgarten_struct *kg;

  /*
   * Parse the command line
   */
  if (argc != 4) {
    usage(argv[0]);
  }

  if (safe_atoi(argv[1], &thrcnt) < 0 || thrcnt <= 0) {
    fprintf(stderr, "`%s' is not valid for `thread_count'\n", argv[1]);
    exit(1);
  }

  if (safe_atoi(argv[2], &chldcnt) < 0 || chldcnt < 0 || chldcnt > thrcnt) {
    fprintf(stderr, "`%s' is not valid for `child_threads'\n", argv[2]);
    exit(1);
  }

  if (safe_atoi(argv[3], &ratio) < 0 || ratio < 1) {
    fprintf(stderr, "`%s' is not valid for `c_t_ratio'\n", argv[3]);
    exit(1);
  }

  /*
   * Initialize kindergarten and random number generator
   */
  srand(time(NULL));

  kg = safe_malloc(sizeof(*kg));
  kg->vt = kg->vc = 0;
  kg->ratio = ratio;

  ret = pthread_mutex_init(&kg->mutex, NULL);
  if (ret) {
    perror_pthread(ret, "pthread_mutex_init");
    exit(1);
  }

  /* ... */

  /*
   * Create threads
   */
  thr = safe_malloc(thrcnt * sizeof(*thr));

  for (i = 0; i < thrcnt; i++) {
    /* Initialize per-thread structure */
    thr[i].kg = kg;
    thr[i].thrid = i;
    thr[i].thrcnt = thrcnt;
    thr[i].is_child = (i < chldcnt);
    thr[i].rseed = rand();

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

  printf("OK.\n");

  return 0;
}
