/**
 * \file   l4vfs/term_server/include/vt100.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_INCLUDE_VT100_H_
#define __L4VFS_TERM_SERVER_INCLUDE_VT100_H_

#include <l4/sys/types.h>
#include <l4/l4vfs/types.h>
#include <l4/semaphore/semaphore.h>

#include <termios.h>
#ifdef USE_UCLIBC
#include <sys/ioctl.h>    // for struct winsize
#endif

#define NUM_PAR      15   // maximal number of parameter for one esc-seq.

#define KEYLIST_SIZE    4096
#define MAX_TABS        30

// Keyboard input modes
#define VT100MODE_COOKED        0
#define VT100MODE_SEMICOOKED    1
#define VT100MODE_RAW           2

// insert mode
#define VT100_INSMODE_REPLACE   0
#define VT100_INSMODE_INSERT    1

// origin mode
// SCROLL - line numbers relative to currently
//          defined scrolling region
#define VT100_ORIGIN_SCROLL     0
// GLOBAL - line numbers relative to the whole
//          terminal
#define VT100_ORIGIN_GLOBAL     1

struct term_select_info
{
    struct term_select_info *next, *prev; // queue info
    l4_threadid_t   handler; // whom to call if new data arrives
    int             mode;    // select mode
    object_handle_t fd;      // fd used for select
};

/* There are three types of screens involved here:
 *
 *  phys is the screen we draw our text to. It is normally displayed.
 *
 *  virt may be much longer than phys, thereby providing a history.
 *       Initially phys and virt start at the same point.  Over time phys
 *       gets moved down.  virt must be at least as big as phys.  Then no
 *       history is present.  All vt100 operations work on phys.
 *
 *  vis  is the currently visible screen.  Normally this is the same as
 *       phys, but may be moved in virt upwards, showing history lines.
 *
 *
 *  +---------------+   0 --> Start of virt
 *  |               |
 *  |               |
 *  |               |
 *  |               |
 *  |+-------------+|   phys_off - vis_off --> 1st line of vis
 *  ||             ||     = VIS_START
 *  ||             ||
 *  ||             ||
 *  ||             ||
 *  ||             ||
 *  |+-------------+|   phys_off - vis_off + phys_h - 1 --> Last line of vis
 *  |               |     = VIS_END
 *  |               |
 *  |               |
 *  +---------------+   phys_off --> first line of phys
 *  |    ^          |
 *  |    |          |
 *  |<---X--------->|   cursor is always within phys
 *  |    |          |
 *  |    v          |
 *  +---------------+   phys_off + phys_h - 1 --> Last line of phys
 *  .               .     = PHYS_END
 *  .               .
 *  .               .
 *  +---------------+   virt_h - 1 --> Limit of virt
 */

typedef struct termstate_s
{
    int           cursor_vis;       // is the cursor visible
    int           echo;             // echo on/off
    int           autorepeat;       // autorepeat on/off
    int           autowrap;         // autowrap on/off (on = start new line
                                    // after last column)
    int           term_mode;        // terminal mode (see defines above)
    int           origin_mode;      // origin mode (see defines above)
    int           insert_mode;      // insert mode (1 = insert char, 0 = 
                                    // replace char, default = 0);
    int           cur_stored_x;     // stored cursor position & attribute
    int           cur_stored_y;
    int           cur_stored_attr;
    int           newline;          // mode for \n -> 1 = start next line,
                                    //                0 = line down
    int           keylist[KEYLIST_SIZE]; // keyboard buffer
    int           keylist_next_write; // next index to write to the buffer
    int           keylist_next_read;  // next index to read from the buffer
//    int           __key_init;       // accept key events
    int           __shift;          // shift pressed
    int           __alt;            // alt pressed
    int           __altgr;          // alt gr pressed
    int           __ctrl;           // ctrl pressed
    int           __keydown;        // any key pressed
    int           returns;          // counter for returns in buffer

    int           w;                // terminal width
    int           virt_h;           // virtual terminal height
    int           phys_h;           // physical terminal height
    int           cur_x;            // current cursor position in phys
    int           cur_y;
    int           hist_len;         // current history length
    int           phys_off;         // offset lines virt to phys
    int           vis_off;          // how many lines the visible screen is above
                                    // the physical (0 means equal)
    l4_int8_t  * text;              // virtual screen buffer
    l4_int8_t  * attrib;            // virtual attrib buffer
    l4_int8_t    attrib_mode;        // current fg & bg colors, other attributes
    int           inverse;          // flag, whether painting should be inverse
    int           decscnm;          // flag, whether whole screen is inv.
    int           esc_state;        // state for esc-seq. state machine
    int           par[NUM_PAR];     // recognized parameter in current esc-seq.
    int           used_par;         // number of recognized parameters
    int           ques;             // question mode, command started with '?'
    int           scroll_top;       // first line of scrolling region (incl.)
    int           scroll_bottom;    // last line of scrolling region (excl.)

    int           tabs[MAX_TABS];   // an array to store all the tabulators set
    int           tab_last;         // last tab in the array

    l4semaphore_t termsem;          // protects the term struct
    l4semaphore_t keybufsem;        // protects the keybuffer
    l4semaphore_t keysem;           // this semaphore is used for a
                                    // classical producer-consumer
                                    // problem on the read buffer
    /* 
     * Select info stuff.
     */
    struct term_select_info *select_list; // stored select infos

    struct termstate_spec_s * spec; // pointer to specific data structure
} termstate_t;

/* It is advised to use the init function only braced with:
 *
 *     l4semaphore_down(&term->termsem);
 *     ...
 *     init_termstate(...);
 *     ...
 *     l4semaphore_up(&term->termsem);
 *
 * And don't forget to init. "term->termsem = L4SEMAPHORE_UNLOCKED;"
 * before, if it is not setup yet.
 */
int        init_termstate(termstate_t * term, int w, int h, int hist);
void       parse_character(termstate_t * term, l4_uint8_t c);
int        vt100_write(termstate_t * term, const l4_int8_t * buf, int count);
int        vt100_read(termstate_t * term, l4_int8_t * buf, int count, int rw_mode);
void       vis_up(termstate_t *term, int up);
void       vis_down(termstate_t *term, int down);
int        xy2index(termstate_t * term, int x, int y);
int        xy2index_virt(termstate_t * term, int x, int y);
l4_int8_t  pack_attribs(int fg_color, int bg_color, int intesity);
void       unpack_attribs(l4_int8_t a, int * fg_color, int * bg_color,
                          int * intesity);

// echo a character
void       vt100_echo(termstate_t *term, l4_int8_t c, l4_int8_t old);
// add a key / key sequence to the char buffer
void       vt100_add_key(termstate_t *term, int keycode);
void       vt100_add_key_if_not_too_busy(termstate_t *term, int keycode);
// add ascii string to the char buffer
void       vt100_add_ascii(termstate_t *term, unsigned char * c);
// getchar functions
int        vt100_getchar(termstate_t *term);
int        vt100_trygetchar(termstate_t *term);
int        vt100_char_avail(termstate_t *term);

// tcgetattr and tcsetattr
int         vt100_tcgetattr(termstate_t *term, struct termios *termios_p);
int         vt100_tcsetattr(termstate_t *term, struct termios *termios_p);
// supply current window size
int         vt100_getwinsize(termstate_t *term, struct winsize *win);

// 'de', 'us'
void vt100_set_keymap(char * keymap);

/* Select stuff */
void vt100_set_select_info(termstate_t *term, object_handle_t handle,
                           int mode, const l4_threadid_t *notify_handler);
void vt100_unset_select_info(termstate_t *term, object_handle_t handle,
                             int mode, const l4_threadid_t *handler);
void vt100_select_notify(termstate_t *term);
int vt100_data_avail(termstate_t *term);

/* Must be provided externally, i.e. if you implement a special output
 * lib, e.g. for dope or con, you must provide these functions.
 */
extern void vt100_redraw(termstate_t *term);
extern void vt100_redraw_xy(termstate_t *term, int x, int y);
extern void vt100_hide_cursor(termstate_t *term);
extern void vt100_show_cursor(termstate_t *term);

#endif
