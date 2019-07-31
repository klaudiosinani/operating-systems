#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <signal.h>// Manualy Added
#include <errno.h>// Manualy Added
#include <semaphore.h>// Manualy Added
#include <pthread.h>// Manualy Added

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
* Compile-time parameters *
***************************/

// Manualy Added
#define perror_pthread(ret, msg) \
                do { errno = ret; perror(msg); } while (0)

void Ctrlcproof(int sign)  /* Use Ctrl + C to stop the process
                              * & reset the color of the terminal
                              */
{
                signal(sign, SIG_IGN);
                // Just ignore the sig
                reset_xterm_color(1);
                // move on w/ color reset
                exit(-1);
}

/*
 * Output at the terminal is is x_chars wide by y_chars long
 */
int y_chars = 50;
int x_chars = 90;

// Manualy Added
int n;
// Threads variable
// we set it as global

// Manualy Added
typedef struct
{
                pthread_t tid;
                // oh_my_ThreadID
                int l;
                // oh_my_line
                sem_t mutex;
                // oh_my_semaphore
}mystruct;
mystruct *saved;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
 */
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;

/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
/*
 * x and y traverse the complex plane.
 */
                double x, y;

                int n;
                int val;

/* Find out the y value corresponding to this line */
                y = ymax - ystep * line;

/* and iterate for all points on this line */
                for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

/* Compute the point's color value */
                                val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
                                if (val > 255)
                                                val = 255;

/* And store it in the color_val[] array */
                                val = xterm_color(val);
                                color_val[n] = val;
                }
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
                int i;

                char point ='@';
                char newline='\n';

                for (i = 0; i < x_chars; i++) {
/* Set the current color, then output the point */
                                set_xterm_color(fd, color_val[i]);
                                if (write(fd, &point, 1) != 1) {
                                                perror("compute_and_output_mandel_line: write point");
                                                exit(1);
                                }
                }

/* Now that the line is done, output a newline character */
                if (write(fd, &newline, 1) != 1) {
                                perror("compute_and_output_mandel_line: write newline");
                                exit(1);
                }
}


void *compute_and_output_mandel_line(void *arg) // Edited
{
                int k;
                // oh_my_current_line
                int line=*(int*)arg;
                // thread's first
                for(k = line; k < y_chars; k+=n) {
                //Run them w/ step n
/*
 * A temporary array, used to hold color values for the line being drawn
 */
                                int color_val[x_chars];
                                compute_mandel_line(k, color_val);
                                // In parallel calculation of the k-th

                                //oh_my_synchronization
                                sem_wait(&saved[(k % n)].mutex);
                                // Wait semaphore of their thread
                                output_mandel_line(1, color_val);
                                // Synchronized coloring!!
                                sem_post(&saved[((k % n)+1)%n].mutex);
                                // Bring the next one

                }
                return 0;
}

int main(void)
{
// Manualy Added
                signal(SIGINT, Ctrlcproof);
                // You are no CtrlC-proof

                int line;

// Manualy Added
                int ret;

                xstep = (xmax - xmin) / x_chars;
                ystep = (ymax - ymin) / y_chars;

/*
 * draw the Mandelbrot Set, one line at a time.
 * Output is sent to file descriptor '1', i.e., standard output.
 */


// Manualy Added
                printf("Hi, give some threads please! ");

                if (scanf("%d", &n) == 1) {
                  // scanf is annoying
                                printf("%d", n);
                } else {
                                printf("failed to read integer.\n");
                }


                if ((n < 1) || (n > y_chars-1)) {
                                printf("invalid input \n");
                                return -1;
                }
                printf("\n");
                saved = (mystruct*)malloc(n*sizeof(mystruct));
                // mallocify
                sem_init(&saved[0].mutex, 0, 1);
                // Semaphore no.0 got 1
                if (n > 1) {
                                for (line = 1; line < n; line++) {
                                  // semaphorefy them
                                                sem_init(&saved[line].mutex, 0, 0);
                                                // now wait
                                }
                }

                for (line = 0; line < n; line++) {
                  // threads created -- running them
                                saved[line].l=line;
                                ret = pthread_create(&saved[line].tid, NULL, compute_and_output_mandel_line, &saved[line].l);
                                if (ret) {
                                                perror_pthread(ret, "pthread_create");
                                                exit(1);
                                }
                }

                for (line = 0; line < n; line++) {
                  // you can now join
                                ret = pthread_join(saved[line].tid, NULL);
                                if (ret)
                                                perror_pthread(ret, "pthread_join");
                }
                for (line = 0; line < n; line++) {
                  // put them to sleep
                                sem_destroy(&saved[line].mutex);
                }



                reset_xterm_color(1);
                return 0;
}
