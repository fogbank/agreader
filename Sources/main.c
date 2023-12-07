/**********************************************************
** main.c : Definition of main loop for AGReader, a tool **
**          designed for AmigaGuide file browsing.       **
**          Free software under GNU license (21/8/2000)  **
**-------------------------------------------------------**
** Written by T.Pierron, from less source code.    :ts=3 **
**********************************************************/

#include "AGNodes.h"
#include "AGReader.h"
#include "IO_tty.h"
#include "Input.h"
#include "Navig.h"

#include <stdlib.h>
#include <unistd.h>

struct scrpos terminfo; /* Information about visited node & screen */

/*** Exit the program ***/
void quit(char* msg, int status)
{
    raw_mode(0);
    PopAGNodes();
#ifdef DEBUG_MEM
    {
        extern long nb_alloc, nb_free, mem_alloc;
        printf("Mem usage: %ld alloc (%ld bytes), %ld free\n",
            nb_alloc, mem_alloc, nb_free);
    }
#endif
    if (msg) {
        fputs(msg, stderr);
    }

    exit(status);
}

/*** Handler called when window size changed ***/
void sig_winch(int type)
{
    struct scrpos old = terminfo;

    (void)type; /* silence the unused param warning */

    get_termsize(&terminfo.width);
    set_scroll_region(terminfo.height - 1);
    if (old.height != terminfo.height || old.width != terminfo.width) {
        if (old.width != terminfo.width)
            FormatPara(terminfo.node, terminfo.width),
                SetTopLine(terminfo.node),
                SetActiveLine(terminfo.node);
        ReRenderAGNode();
    }
}

/*** Program interrupted ***/
void sig_int(int type)
{
    (void)type; /* silence the unused param warning */
    /* Some cleanup should be done before */
    if (is_rawmode())
        set_scroll_region(terminfo.height);
    quit("*** User abort\n", EXIT_SUCCESS);
}

/*** MAIN LOOP ***/
int main(int argc, char* argv[])
{
    struct _AGLink link;

    if (!isatty(STDIN_FILENO)) {
        printf("Reading through pipes is not yet supported, sorry.\n");
        quit(NULL, EXIT_FAILURE);
    } else if (argc != 2) {
        printf("\033[1musage:\033[0m %s File.guide\n"
               "Navigate through an AmigaGuide file on default tty\n",
            argv[0]);
        quit(NULL, EXIT_FAILURE);
    } else {
        /* Set parameters describing the file. Because file-type **
        ** can't be known for now, let Navigate() determines it. */
        link.node = "MAIN";
        link.file = argv[1];
        link.line = 0;
        link.type = LINK_TO_DOC;
        get_termsize(&terminfo.width);

        if (Navigate("", &link)) {
            /* Waits the last moment for changing terminal attributes, thus **
            ** starting error messages can still be displayed on stderr.    */
            raw_mode(1);
            init_signals(1, sig_int, sig_winch);

            /* Process user input */
            ReRenderAGNode();
            ProcessKeys();
            /* Reset standard scrolling region */
            set_scroll_region(terminfo.height);
            quit(NULL, EXIT_SUCCESS);
        } else {
            /* Errors will be displayed in Navigate() */
            quit(NULL, EXIT_FAILURE);
        }
    }

    return 0;
}
