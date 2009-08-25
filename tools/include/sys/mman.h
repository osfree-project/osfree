/*

Memory Mapped Files Emulation Layer v1.75
(c) 1998, 1999 Maurilio Longo - md2520@mclink.it

*/

#ifndef _SYS_MMAN_H		/* As per miyata@kusm.kyoto-u.ac.jp suggestion */
#define _SYS_MMAN_H

#ifndef _SYS_TYPES_H_  // valerius
#include <os2/types.h>
#endif

#if defined (__cplusplus)
extern "C" {
#endif

#define INCL_DOSEXCEPTIONS
#include <os2.h>

typedef EXCEPTIONREGISTRATIONRECORD mmap_reg_t;

#define HAVE_MSYNC	0x1
#define MSYNC_3		0x1
#define	PROT_READ	0x0001
#define	PROT_WRITE	0x0002
#define	PROT_EXEC	0x0004
#define	PROT_NONE	0x0000
#define	MAP_SHARED	0x1
#define	MAP_PRIVATE	0x2
#define	MAP_FIXED	0x10
#define	MCL_CURRENT	0x1
#define	MCL_FUTURE	0x2
#define	MS_ASYNC	0x1
#define	MS_INVALIDATE   0x2
#define MS_SYNC		0x4

#define	MS_MUNMAP	0x10     /* not standard, used internally by munmap() */


int		getpagesize(void);
int		mprotect(caddr_t pAddr, int cbLen, int fProtection);
int		mlockall(int fFlags);
#ifndef _MMAP_DECLARED //valerius
#define _MMAP_DECLARED
caddr_t	mmap(caddr_t pAddr, size_t cbLen, int fProtection, int fFlags, int hFile, off_t cbOffset);
#endif
int		msync(caddr_t pAddr, int cbLen, int fFlags);
int		munlockall(void);
int		munmap(caddr_t pAddr, int cbLen);
int		merror(void);
int      mregister(mmap_reg_t *pReg);
int      mderegister(mmap_reg_t *pReg);

//added by valerius:
/* Return value of `mmap' in case of an error.  */
#define MAP_FAILED	((void *) -1)


#if defined (__cplusplus)
}
#endif
#endif /* not _SYS_MMAN_H */
