/*
 *  Utility functions
 *
 *
 *
 */

#include "freeldr.h"
#include "vsprintf.h"

#define VIDEO_HEIGHT 25   //высота экрана
extern char scratch_buffer[1024];

/*
long __far
freeldr_memcheck (unsigned long addr, long len)
{
    // Physical address:
    if ( (addr < RAW_ADDR (0x1000))
        || ((addr <  RAW_ADDR (0x100000)) && (RAW_ADDR(mbi.mem_lower * 1024) < (addr + len)))
        || ((addr >= RAW_ADDR (0x100000)) && (RAW_ADDR(mbi.mem_upper * 1024) < ((addr - 0x100000) + len))) )
    {
        errnum = ERR_WONT_FIT;
        printk("freeldr_memcheck: ERR_WONT_FIT");
        printk("freeldr_memcheck(): addr = 0x%08lx, len = %u", addr, len);
        return 0;
    }

    return 1;
}
 */

void far * __far
freeldr_memset (void far *start, char c, long len)
{
    char far *p = start;

    //if (freeldr_memcheck ((unsigned long)PHYS_FROM_FP(start), len))
    {
        while (len -- > 0)
            *p ++ = c;
    }

    return start;
}


void far * __far
freeldr_memmove (void far *_to, const void far *_from, long _len)
{

    char far *from = (char far *)_from;
    char far *to   = _to;

    //if (freeldr_memcheck ((unsigned long)PHYS_FROM_FP(_to), _len))
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


unsigned long __far
freeldr_memmove_phys (unsigned long _to, unsigned long _from, long _len)
{
    unsigned long buf, from, to, l, chunk;
    unsigned short ret;

    l    = _len;
    to   = _to;
    from = _from;

    while (l > 0)
    {

        if (l < LOAD_CHUNK_SIZE)
        {
            chunk = l;
            l     = 0;
        }
        else
        {
            chunk = LOAD_CHUNK_SIZE;
            l    -= LOAD_CHUNK_SIZE;
        }

        // Read a chunk of a file to the read buffer
        ret = loadhigh(readbuf, chunk, from);
        if (ret)
        {
            printk("loadhigh0 error, rc = 0x%04x", ret);
            return 0;
        }

        // move it to the proper place
        ret = loadhigh(to, chunk, readbuf);
        if (ret)
        {
            printk("loadhigh1 error, rc = 0x%04x", ret);
            return 0;
        }

        from += chunk;
        to   += chunk;

    }
    return _to;
}


char far * __far
freeldr_strcpy (char far *dest, const char far *src)
{
  freeldr_memmove (dest, src, freeldr_strlen (src) + 1);
  return dest;
}

char far * __far
freeldr_strcat(char far *dst, const char far *src)
{
  int  p;
  char far *_dst = dst;

  freeldr_memmove(_dst + freeldr_strlen(_dst), src, freeldr_strlen(src) + 1);

  return dst;
};


long __far
freeldr_strcmp (const char far *s1, const char far *s2)
{
  while (*s1 || *s2)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1 ++;
      s2 ++;
    }

  return 0;
}


long __far
freeldr_memcmp (const char far *s1, const char far *s2, long n)
{
  while (n)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1++;
      s2++;
      n--;
    }

  return 0;
}


int __far
freeldr_strlen (const char far *str)
{
  int len = 0;

  while (*str++)
    len++;

  return len;
}


int __far
freeldr_pos(const char c, const char far *str)
{
  int len = 0;
  int l   = freeldr_strlen(str);

  while (*str++ != c && len < l)
    len++;

  len++;
  return len;
}


long __far
freeldr_tolower (long c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));

  return c;
}


//Clear screen (actually, just scroll it up)
void __cdecl __far
freeldr_clear()
{
    int i;

    for (i = 0; i < VIDEO_HEIGHT; i++)
    {
        puts("\r\n");
    }
}
