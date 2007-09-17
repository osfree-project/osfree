#include "freeldr.h"
#include "vsprintf.h"

/* filesystem common variables */
unsigned long filepos;
unsigned long filemax;

//read buffer pointer for freeldr_read
unsigned char far *readbuf1;
//read buffer physical address
unsigned long readbuf;

/*********************************\
**                               **
**  Filesystem access functions. **
**  ~~~~~~~~~~~~~~~~~~~~~~~~~~~  **
\*********************************/


/* Open a file or directory on the active device. */
long __cdecl __far
freeldr_open (char far *filename)
{
    unsigned long  far *fSize;
    unsigned short n;
    char far buf[256];
    char far *s;
    char far *p;

    printk("freeldr_open() started");

    s = filename;
    n = freeldr_pos(' ', s);
    //printk("n = %d", n);

    if (n > 1)
    {
      freeldr_memmove(buf, s, n - 1);
      buf[n - 1] = '\0';
      filename = buf;
    }

    if (*filename == '/') filename++;

    for (p = filename; *p; p++)
      if (*p == '/') *p = '\\';

    //printk("filename = %s", FP_OFF(filename));

    if (muOpen(filename, fSize))
        return 0;

    filemax = (*fSize);
    filepos = 0;

    return (*fSize);
}

/* Read len bytes to the physical address buf
   from the current seek position           */
unsigned long __cdecl __far
freeldr_read (unsigned long buf, unsigned long len)
{
    unsigned short chunk;
    unsigned long l, fp, rc, read = 0;
    int ret;

    //printk("grub_read()");
    /* Make sure "filepos" is a sane value */
    //if ((filepos < 0) || (filepos > filemax))
    //    filepos = filemax;

    /* Make sure "len" is a sane value */
    //if ((len < 0) || (len > (filemax - filepos)))
    //    len = filemax - filepos;

    /* if target file position is past the end of
       the supported/configured filesize, then
       there is an error */
    if (filepos + len > filemax)
    {
        printk("ERR_FILELENGTH");
        return 0;
    }

    fp = filepos;
    l  = len;

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
        rc = muRead((unsigned long) fp,
                    (unsigned char far *) readbuf1,
                    (unsigned long) chunk);

        // move it to the proper place
        ret = loadhigh(buf, chunk, readbuf);
        if (ret)
        {
            printk("loadhigh1 error, rc = 0x%04x", ret);
            return 0;
        }

        read += rc;
        fp   += rc;
        buf  += rc;

        if (rc < chunk) break;

    }

    return read;
}


/* Reposition a file offset.  */
unsigned long __cdecl __far
freeldr_seek (unsigned long offset) // noch nicht fertig!
{
    //if (offset > filemax || offset < 0)
    //    return (long)(-1);

    filepos = offset;
    return offset;
}


/* Close current file. */
void __cdecl __far
freeldr_close (void)
{
    muClose();
}


/* Terminate the work with files. */
void __cdecl __far
freeldr_term (void)
{
    muTerminate();
}
