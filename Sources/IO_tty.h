/*** Basic functions to do with TTY ***/

#ifndef IO_TTY_H
#define IO_TTY_H

enum Term_mode {
    MODE_CANONICAL,
    MODE_RAW
};

/** Switch between normal or raw mode **/
void set_mode(int);

/** Get current terminal mode **/
int get_mode(void);

/** Get a character from standard input (keyboard) **/
char getchr(void);

/** Get terminal dimension **/
void get_termsize(short*);

/** Set the region affected by the scrolling **/
void set_scroll_region(short height);

/** Set cursor to screen postion **/
void set_cursor_pos(short y, short x);

/** Init (1) or remove (0) signal handler SIGINT and SIGWINCH **/
typedef void (*sighandler_t)(int);
void init_signals(int on, sighandler_t sig_int, sighandler_t sig_winch);

#endif
