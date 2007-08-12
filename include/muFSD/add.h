/*
 *  misc defines.
 *  (c) osFree project,
 *  valerius, 2006/10/17
 */


#include <os2/types.h>
#include <stdarg.h>

#include <i86.h>

#define NO_ERROR 0
#define ERROR_HANDLE_EOF 38

#ifndef OFFSETOF
#define OFFSETOF(p) ((unsigned short) (p))
#endif

#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKEP(sel, off)  ((PVOID)MAKEULONG(off, sel))

//#define MAKEP(sel, off)  MK_FP(sel, off)

int vsprintf(char *, const char *, __va_list);

int VFS_readdir(struct file *file, struct dirent *dirent);
int VFS_read(struct file *file, char far *buf, loff_t len, unsigned long far *pLen);
int VFS_write(struct file *file, char far *buf, loff_t len, unsigned long far *pLen);
