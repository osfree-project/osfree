/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

void *
grub_memmove (void *_to, const void *_from, int _len)
{

    char *from = (char *)_from;
    char *to   = _to;

    if (grub_memcheck ((unsigned long)(_to), _len))
    {
        if ( from == to )
        {
            return( to );
        }
        if ( from < to  &&  from + _len > to )
        {
            to += _len;
            from += _len;
            while( _len != 0 )
            {
                *--to = *--from;
                _len--;
            }
        }
        else
        {
            while( _len != 0 )
            {
                *to++ = *from++;
                _len--;
            }
        }
    }

    return( to );
}

