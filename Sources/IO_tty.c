/****************************************************************
***** io_tty.c: Various terminal related specific procedures ****
*****           Taken from less source code by Mark Nudelman ****
*****           but greatly simplified by T.Pierron          ****
****************************************************************/

#include "IO_tty.h"
#include "AGReader.h"
#include <signal.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

char underlined = 1; /* 1 if terminal support underlined mode */

/** This sequence is to get a private mode in a xterm & co ***
*** while viewing a file (quoted from konsole tech specs)  **/
char SET_PRIV[] = "\0337" /* Save cursor position */
                  "\033[?47;1h"; /* Enter in private mode (47) and set mode AppCuKeys (1) */

/** This sequence is to get back in normal mode **/
char SET_PUB[] = "\033[2J" /* Clears screen */
                 "\033[?1;47l" /* Reset AppCuKeys (1) and private (47) mode */
                 "\0338"; /* Restore cursor position */


/* Change terminal to "raw mode", or restore to "normal" mode. This means:
 *	1. An outstanding read will complete on receipt of a single keystroke.
 *	2. Input is not echoed.
 *	3. On output, \n is mapped to \r\n.
 *	4. \t is NOT expanded into spaces.
 *	5. Signal-causing characters such as ctrl-C (interrupt),
 *	   etc. are NOT disabled.
 * It doesn't matter whether an input \n is mapped to \r, or vice versa.
 */
void set_mode(int mode)
{
    static struct termios save_term;
    struct termios s;
    int cur_mode = get_mode();

    /* Do not set twice the same mode!! */
    if (cur_mode == mode) {
        return;
    }

    if (mode == MODE_RAW) {
        /* Get terminal modes */
        tcgetattr(STDIN_FILENO, &s);

        /* Save modes and set certain variables dependent on modes */
        save_term = s;

        /* Set the modes to the way we want them */
        s.c_lflag &= ~(0
#ifdef ICANON
            | ICANON
#endif
#ifdef ECHO
            | ECHO
#endif
#ifdef ECHOE
            | ECHOE
#endif
#ifdef ECHOK
            | ECHOK
#endif
#if ECHONL
            | ECHONL
#endif
        );

        s.c_oflag |= (0
#ifdef XTABS
            | XTABS
#else
#ifdef TAB3
            | TAB3
#else
#ifdef OXTABS
            | OXTABS
#endif
#endif
#endif
#ifdef OPOST
            | OPOST
#endif
#ifdef ONLCR
            | ONLCR
#endif
        );

        s.c_oflag &= ~(0
#ifdef ONOEOT
            | ONOEOT
#endif
#ifdef OCRNL
            | OCRNL
#endif
#ifdef ONOCR
            | ONOCR
#endif
#ifdef ONLRET
            | ONLRET
#endif
        );

        /* Get some feature of host terminal */
        {
            char* env;
            if ((env = getenv("TERM")) != NULL && !strcasecmp(env, "linux"))
                /* Unfortunately linux console disabled all previous defined **
                ** styles in an ANSI sequence that contains underlining mode */
                underlined = 0;
        }

        s.c_cc[VMIN] = 1;
        s.c_cc[VTIME] = 0;

        /* let's enter in private mode */
        write(STDOUT_FILENO, SET_PRIV, sizeof(SET_PRIV) - 1);
    } else {
        /* Restore saved modes */
        s = save_term;
        /* and old display mode */
        write(STDOUT_FILENO, SET_PUB, sizeof(SET_PUB) - 1);
    }

    tcsetattr(STDIN_FILENO, TCSADRAIN, &s);
}

/*** Test whether raw mode is on ***/
int get_mode(void)
{
    struct termios s;
    tcgetattr(STDIN_FILENO, &s);

    return s.c_lflag & ICANON ? MODE_CANONICAL : MODE_RAW;
}

char getchr(void)
{
    char c;
    if (read(STDIN_FILENO, &c, sizeof(c)) != sizeof(c))
        return 0;

    return c;
}

/*** Set the scrolling region ***/
void set_scroll_region(short height)
{
    /* This sequence sets the vertical scrolling region */
    printf("\033[1;%dr", height);
    fflush(stdout);
}

/*** Get dimension of the current terminal ***/
void get_termsize(short* wh)
{
    struct winsize w;
    char* s;
    wh[0] = 0;
    wh[1] = 0;

    /* Send a command to the terminal, to get its size */
    if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        && (w.ws_col > 0)
        && (w.ws_row > 0)) {
            wh[0] = w.ws_col;
            wh[1] = w.ws_row;
    /* else try to get environment variables */
    } else if ((s = getenv("COLUMNS")) != NULL && (s = getenv("LINES")) != NULL) {
        wh[0] = atoi(s);
        wh[1] = atoi(s);
    /* If this fails too, use fixed dimension */
    } else {
        wh[0] = 80;
        wh[1] = 24;
    }
}

/*** Set the cursor to the specified screen position ***/
void set_cursor_pos(short line, short col)
{
    /* The special escape sequence is: \e[Py;PxH */
    printf("\033[%d;%dH", line, col);
}

/*** Setup various handler ***/
void init_signals(int on, sighandler_t sig_int, sighandler_t sig_winch)
{
    if (on) {
        /* Set signal handlers */
        signal(SIGINT, sig_int);
        signal(SIGWINCH, sig_winch);
    } else {
        /* Reset original signal handlers */
        signal(SIGINT, SIG_DFL);
        signal(SIGWINCH, SIG_DFL);
    }
}
