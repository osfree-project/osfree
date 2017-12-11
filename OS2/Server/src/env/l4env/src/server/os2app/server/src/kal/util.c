/*
 *  API implementations
 *  parts (on the client side)
 *  These functions are those which are
 *  exported from the KAL.DLL virtual library.
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/kal.h>

/* libc includes */
#include <string.h>
#include <ctype.h>

int
strlstlen(char *p)
{
  int l, len = 0;

  if (!p || !*p)
    return 2;

  while (*p) // skip all lines; break if NULL
  {
    /* skip one string */
    l = strlen(p) + 1;
    /* skip NULL character */
    p   += l;
    len += l;
  }

  if (!len)
    len++;

  len++; // include NULL symbol

  return len;
}


/* A becomes 1. */
int char_to_disknum(unsigned char chr_dsk) {
    /*'A' + ulDriveNum - 1*/
    return  toupper(chr_dsk) - 'A' + 1;
}
/* 1 becomes A. */
unsigned char disknum_to_char(int i_dsk) {
    return 'A' + toupper(i_dsk) - 1;
}

/* If path begins with everything else but '\' or '/', assume relative path. */
int isRelativePath(char *path) {
    if((path[0] == '\\') || (path[0] == '/'))
        return 0;
    return 1;
}

/* Return device from a path or else zero. */
unsigned char parse_drv(char *path) {
    int i=0;
    if((path != NULL) && (path[0] != 0) && (path[1] != 0))
        i = 2;
    /*int i = strlen(path);*/
    if((i >= 2) && (path[1] == ':')) {
        return path[0];
    }
    return '\0';
}

/*  
  Only return path from presumed absolute path.*/
char *parse_path(char *path, char *ret_buffer, int buf_len) {

    unsigned char drive = parse_drv(path);
    if(drive)/*If there is a device letter. */
        /* Skip drive and copy path. */
        strncpy(ret_buffer, (char*)&path[2], buf_len);

    else /* Or else copy entire path. */
        strncpy(ret_buffer, path, buf_len);

    return ret_buffer;
}



