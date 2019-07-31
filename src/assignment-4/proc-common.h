#ifndef PROC_COMMON_H
#define PROC_COMMON_H

/******************************************************************************
 * Helper Functions
 */

/* does useless computation */
void compute(int count);

/* Does nothing and never returns. */
void wait_forever(void);

/*
 * Print a nice diagnostic based on {pid, status}
 * as returned by wait() or waitpid().
 */
void explain_wait_status(pid_t pid, int status);

/*
 * This function makes sure a number of children processes
 * have raised SIGSTOP, by using waitpid() with the WUNTRACED flag.
 *
 * This will NOT work if children use pause() to wait for SIGCONT.
 */
void wait_for_ready_children(int cnt);

/* Change the name of the process. */
void change_pname(const char *new_name);

/* Print the process tree rooted at process with PID p. */
void show_pstree(pid_t p);

/*
 * Create a shared memory area, usable by all descendants of the calling process.
 */
void *create_shared_memory_area(unsigned int numbytes);

#endif /* PROC_COMMON_H */
