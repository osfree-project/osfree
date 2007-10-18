//
// freeldr.h
//

#include <i86.h>

// Make physical address from far pointer
#define PHYS_FROM_FP(x) ((((unsigned long)(FP_SEG(x))) << 4) + (FP_OFF(x)))
// Make physical address from near pointer
#define PHYS_FROM_NP(seg, x) ((((unsigned long)(seg)) << 4) + (FP_OFF(x)))
// Make far pointer from physical address
#define FP_FROM_PHYS(x) (MK_FP( (((x) >> 4) & 0xFFFF), ((x) & 0xf)))

// freeldr segment
extern unsigned short current_seg;

/* The drive number of an invalid drive.  */
#define GRUB_INVALID_DRIVE      0xFF

void __cdecl
DispNTS(char *CMsgBuff);

void __cdecl
DispC(char *CMsgBuff);

void __cdecl
DebugNTS(char *CMsgBuff);

unsigned short __cdecl
muOpen(char far *fileName,
       unsigned long far *fileSize);

unsigned long  __cdecl
muRead(unsigned long seekOffset,
       unsigned char far *dataBuffer,
       unsigned long bufferSize);

void __cdecl
muClose();

void __cdecl
muTerminate();

unsigned short __cdecl
loadhigh(unsigned long buf,
         unsigned short chunk,
         unsigned long readbuf);

/* filesystem common variables */
extern unsigned long filepos;
extern unsigned long filemax;

//read buffer pointer for freeldr_read
extern unsigned char far *readbuf1;
//read buffer physical address
extern unsigned long readbuf;

// The size of a read buffer
#define LOAD_CHUNK_SIZE 0x4000

//long
//freeldr_memcheck (unsigned long addr, long len);

void far * __far
freeldr_memset (void far *start, char c, long len);

void far * __far
freeldr_memmove (void far *_to, const void far *_from, long _len);

unsigned long __far
freeldr_memmove_phys (unsigned long _to, unsigned long _from, long _len);

char far * __far
freeldr_strcpy (char far *dest, const char far *src);

long __far
freeldr_strcmp (const char far *s1, const char far *s2);

long __far
freeldr_memcmp (const char far *s1, const char far *s2, long n);

int __far
freeldr_strlen (const char far *str);

int __far
freeldr_pos(const char c, const char far *str);

void __cdecl __far
freeldr_clear();

long __cdecl __far
freeldr_open (char far *filename);

/* Read len bytes to the physical address buf
   from the current seek position           */
unsigned long __cdecl __far
freeldr_read (unsigned long buf, unsigned long len);

/* Reposition a file offset.  */
unsigned long __cdecl __far
freeldr_seek (unsigned long offset);

/* Close current file. */
void __cdecl __far
freeldr_close (void);

/* Terminate the work with files. */
void __cdecl __far
freeldr_term (void);

char far * __far
freeldr_strcat(char far *dst, const char far *src);
