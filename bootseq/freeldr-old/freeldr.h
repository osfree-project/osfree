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
muOpen(char *fileName,
       unsigned long *fileSize);

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

long
freeldr_memcheck (unsigned long addr, long len);

void far *
freeldr_memset (void far *start, char c, long len);

void far *
freeldr_memmove (void far *_to, const void far *_from, long _len);

char far *
freeldr_strcpy (char far *dest, const char far *src);

long
freeldr_strcmp (const char far *s1, const char far *s2);

long
freeldr_memcmp (const char far *s1, const char far *s2, long n);

int
freeldr_strlen (const char far *str);

void __cdecl
freeldr_clear();

