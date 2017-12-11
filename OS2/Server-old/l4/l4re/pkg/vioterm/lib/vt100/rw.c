/**
 * \file   l4vfs/term_server/lib/vt100/rw.c
 * \brief  read and write functions
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <l4/log/l4log.h>

#include "lib.h"
#include "keymap.h"

extern int _DEBUG;

int vt100_read(termstate_t *term, l4_int8_t *buf, int count, int rw_mode)
{
    int c;
    int i              = 0;    // # of chars in return buffer
    int newline        = 0;    // newline seen
//    l4_int8_t oldchar = 'a';  // init. to non-ctrl character

    /* In blocked cooked mode, we return only after a return on the
     * keyboad, delivering
     * MIN(<all_chars_in_buffer_upto_and_including_1_return>, <asked>)
     * chars back.
     *
     * In unblocked cooked mode, we return immediately, returning
     * MIN(<all_chars_in_buffer_upto_and_including_1_return>, <asked>)
     * chars if there was a return on the keyboard, or -1 (errno = 11,
     * Resource temporarily unavailable) if there was no completed
     * line.
     *
     * For both cooked modes, it is necessary to know, whether return
     * was hit, before it can be actually consumed from the buffer.
     * Therefore we should use a atomic counter which is increased,
     * each time a return is inserted into the buffer and decreased,
     * each time one is consumed.  Thereby we can replace the
     * 'newline' check below, using a compare on the counter.
     *
     * In blocked raw mode, we return exactly <asked> chars.
     *
     * In unblocked raw mode, we return
     * MIN(<asked>, <current_line_buffer_length>) chars.
     */

    // no complete line in buffer, return 
    if ((term->term_mode == VT100MODE_COOKED) &&
        (rw_mode & (O_NONBLOCK | O_NDELAY)) && (term->returns <= 0))
    {
        LOGd(_DEBUG,"return from non-blocking read");
        return -EAGAIN;
    }

    // fixme: term->returns could be a semaphore to directly sleep on it
    //        instead of kind of busy waiting here
    if ((term->term_mode == VT100MODE_COOKED) &&
        ((rw_mode & (O_NONBLOCK | O_NDELAY)) == 0) && (term->returns <= 0))
    { // there are no complete lines and we are in cooked blocked mode
        while (term->returns <= 0)
            l4_sleep(10);  // idle a bit
    }

    while(1)
    {
        if (term->term_mode == VT100MODE_COOKED)  // cooked mode
        {
            if ((term->returns > 0 && i >= count) || newline)
            {
                break;
            }
        }
        else                                 // raw mode
        {
            if (i >= count)
                break;
        }

        // check if we are in a non-blocking read
        if (rw_mode & (O_NONBLOCK | O_NDELAY))
        {
            c = vt100_trygetchar(term);
            if (c < 0) // no char pending --> return
            {
                if (i == 0 && count != 0)
                    return -EAGAIN;
                else
                    break;
            }
        }
        else
        {
            LOGd(_DEBUG,"before getchar()");
            c = vt100_getchar(term);
        }

        LOGd(_DEBUG, "char = %c (%d)", c, c);

        if (c < 0)
        {
            LOGd(_DEBUG, "invalid char");
            return -EINVAL;
        }

        switch(c)
        {
        case '\a':       // beep
            break;
        case '\r':
            l4util_dec32(&term->returns);  // we consumed one return
            newline = 1;
            buf[i++] = '\n';
            break;
        case '\n':       // newline - stop here, we are linebuffered
            l4util_dec32(&term->returns);  // we consumed one return
            newline = 1;
            // fall through
        default:                   // normal char
            buf[i++] = c;
        }
    }

    LOGd(_DEBUG, "read: '%s'", buf);
    return i;
}

int vt100_write(termstate_t *term, const l4_int8_t * buf, int count)
{
    const l4_int8_t * act;

    // hide the cursor --> this restores the window without cursor
    // stuff in it. we set the cursor to its new position later
    vt100_hide_cursor(term);

    // set virt back to phys. without that, the following could happen:
    // 1. The user scrolls up to the beginning of the history buffer
    // 2. The history becomes filled up.
    // 3. As we have a ring buffer, the user sees the new text coming
    //    in from above.

    term->vis_off = 0;

    for(act = buf; act < buf + count; act++)
    {
        parse_character(term, (l4_uint8_t)*act);
    }

    // show the cursor
    vt100_show_cursor(term);

    // redraw
    vt100_redraw(term);

    return count;
}

/* Take a key's scancode, translate it to ascii and add it to the
 * input buffer.
 *
 * Note, that there is now more than one thread, who might insert data
 * into the key buffer, so we need a semaphore protecting it.
 */
void vt100_add_key(termstate_t *term, int code)
{
    int new_top;
    unsigned char * result;
    unsigned char oldchar = 'a';

    // wrong keycode?
    if (code > 127)
        return;

    if (term->__shift)
        result = (*(vt100_keymap))[code][1];
    else
        result = (*(vt100_keymap))[code][0];

    l4semaphore_down(&term->keybufsem);
    while (*result)
    {
        if ((*result == '\b') &&                      // handle backspaces
            (term->term_mode == VT100MODE_COOKED))         // in cooked mode
        {
            if (term->keylist_next_write == term->keylist_next_read)
            {                                   // empty buffer, nothing to bs
                l4semaphore_up(&term->keybufsem);
                return;
            }

            new_top = (term->keylist_next_write - 1); // compute old position
            if (new_top < 0)
                new_top = KEYLIST_SIZE - 1;

            if (term->keylist[new_top] == '\r')       // don't bs returns
            {
                l4semaphore_up(&term->keybufsem);
                return;
            }

            term->keylist_next_write = new_top;       // finally do the bs
            oldchar = term->keylist[new_top];
            l4semaphore_down(&term->keysem);          // we backspaced one char
        }
        else                                          // handle normal chars
        {
            new_top = (term->keylist_next_write + 1) % KEYLIST_SIZE;
            if (new_top == term->keylist_next_read)
            {
                LOG("key buffer full!");
                l4semaphore_up(&term->keybufsem);
                return;
            }

            if (*result == '\r')  // atomically count returns in buffer
                l4util_inc32(&term->returns);

            if (!term->__ctrl) 
                term->keylist[term->keylist_next_write] = *result;
            else
                term->keylist[term->keylist_next_write] = CTRL(*result);

            term->keylist_next_write = new_top;

            l4semaphore_up(&term->keysem);
        }

        // echo on? - write c
        if (term->echo)
            vt100_echo(term, *result, oldchar);
        result++;
    }

    l4semaphore_up(&term->keybufsem);
    vt100_select_notify(term);
}

// this one is probably obsolete
void vt100_add_key_if_not_too_busy(termstate_t *term, int keycode)
{
    if (term->keysem.counter < 2)
        vt100_add_key( term, keycode );
}

/* Add a zero terminated ascii string directly to the input buffer.
 *
 * Note. that there is now more than on thread, who might insert data
 * into the read buffer, so we need a semaphore protecting it.
 *
 * todo: maybe we should handle bs here too?
 */
void vt100_add_ascii(termstate_t *term, unsigned char * c)
{
    int new_top;

    l4semaphore_down(&term->keybufsem);
    while (*c)
    {
        new_top = (term->keylist_next_write + 1) % KEYLIST_SIZE;
        if (new_top == term->keylist_next_read)
        {
            LOG("key buffer overrun!");
            l4semaphore_up(&term->keybufsem);
            return;
        }
        if (*c == '\r')  // atomically count returns in buffer
            l4util_inc32(&term->returns);
        term->keylist[term->keylist_next_write] = *c;
        term->keylist_next_write = new_top;

        l4semaphore_up(&term->keysem);
        c++;
    }
    l4semaphore_up(&term->keybufsem);
}
