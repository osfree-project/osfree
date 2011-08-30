/**
 * \file   l4vfs/term_server/lib/vt100/echo.c
 * \brief  character echoing
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/term_server/vt100.h>
#include "lib.h"

void displaymap(unsigned char c, char *buf);

void vt100_echo(termstate_t *term, l4_int8_t c, l4_int8_t old)
{
    char buf[2];

    switch(c)
    {
    case '\b':
        if (IS_CTRL(old))
        {
            vt100_write( term,"\b \b", 3);
        }
        vt100_write(term, "\b \b", 3);
        break;
    case 13:  // RETURN is a ctrl-character but should not print as ^M
        vt100_write(term, "\n", 1);
        break;
    case 10:  // ^R is a ctrl-character but should print a LF additionally
        displaymap(c, buf);
        vt100_write(term, buf, 2);
        vt100_write(term, "\n", 1);
        break;
    default:
        if (IS_CTRL(c))
        {
            displaymap(c, buf);
            vt100_write(term, buf, 2);
        }
        else if (term->__ctrl)   // fixme: what is this used for???
        {
            displaymap(c, buf);
            vt100_write( term, buf, 2 );
        }
        else
        {
            vt100_write( term, &c, 1 );
        }
        break;
    }
}

void displaymap(unsigned char c, char *buf)
{
    buf[0] = '^';
    if (IS_CTRL(c))
        buf[1] = c + 64;
    else
        buf[1] = uppercase(c);
}

// why not just use toupper ???
char uppercase(char c)
{
    if ( c < 'Z' && c > 'A' )
        return c;
    if ( c < 'z' && c > 'a' )
        return c - 32;

    return c;
}
