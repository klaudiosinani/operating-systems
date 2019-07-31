#ifndef MANDEL_LIB_H__
#define MANDEL_LIB_H__

/* Function prototypes */
int mandel_iterations_at_point(double x, double y, int max);
unsigned char xterm_color(int color_val);
ssize_t insist_write(int fd, const char *buf, size_t count);
void set_xterm_color(int fd, unsigned char color);
void reset_xterm_color(int fd);

#endif /* MANDEL_LIB_H__ */
