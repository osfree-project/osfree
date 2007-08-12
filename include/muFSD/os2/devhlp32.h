//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/devhlp32.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel 
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code 
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __DevHlp32_h
#define __DevHlp32_h


#ifndef __infoseg_h
#define __infoseg_h
#include <infoseg.h>
#endif

#ifdef __IBMC__
#include <stdarg.h>
#else
typedef char *va_list;
#endif

#pragma pack(1)
struct PageList {
    unsigned long physaddr;
    unsigned long size;
};
#pragma pack()

#pragma pack(1)
struct ddtable {
    USHORT reserved[3];
    PTR16  idc_entry;
    USHORT idc_ds;
};
#pragma pack()


/**************************************************************************************/
/*** mwdd32.sys exported entry points structure                                     ***/
/**************************************************************************************/
extern struct DevHelp32 DevHelp32;

#define DEVHELP32_MAGIC 0xDF3C4E07
#define DEVHELP32_VERSION 7

struct DevHelp32 {
    unsigned long magic;
    unsigned long version;
    union {
        struct {
            int  (*DH32ENTRY  DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY  DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY  DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*DH32ENTRY2 DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY  DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY  DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY  DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY  DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY  DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY  DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY  DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY  DevHlp32_Yield)(void);
            int  (*DH32ENTRY  DevHlp32_GetInfoSegs)(struct InfoSegGDT **, struct InfoSegLDT **);
            int           (*DH32ENTRY vsprintf)(char *, const char *, va_list);
            unsigned long (*DH32ENTRY strtoul) (const char *, char **, int);
            long          (*DH32ENTRY atol) (const char *);
            char *        (*DH32ENTRY strupr) (char *s);
            char *        (*DH32ENTRY strpbrk) (const char *, const char *);
            int           (*DH32ENTRY fnmatch) (const char *, const char *, int);
            long          (*DH32ENTRY strtol) (const char *, char **, int);
	    int (*DH32ENTRY  sec32_attach_ses)(void *);
            int (*DH32ENTRY  DevHlp32_setIRQ)(unsigned short, unsigned short, unsigned short, unsigned short);
            int (*DH32ENTRY2 DevHlp32_EOI)(unsigned short);
            int (*DH32ENTRY  DevHlp32_UnSetIRQ)(unsigned short, unsigned short);
            int (*DH32ENTRY  DevHlp32_PageListToLin)(unsigned long, struct PageList *, void **);
            int (*DH32ENTRY  DevHlp32_AllocGDTSelector)(unsigned short *, int);
            int (*DH32ENTRY2 DevHlp32_FreeGDTSelector)(unsigned short);
            int (*DH32ENTRY  DevHlp32_AttachDD)(char *, struct ddtable *);
            int (*DH32ENTRY  DevHlp32_GetDosVar)(int, PTR16 *, int);
	    int (*DH32ENTRY  DevHlp32_VerifyAccess)(PTR16, unsigned short, int);
	    int (*DH32ENTRY  DevHlp32_CloseEventSem)(unsigned long);
	    int (*DH32ENTRY  DevHlp32_OpenEventSem)(unsigned long);
	    int (*DH32ENTRY  DevHlp32_PostEventSem)(unsigned long);
	    int (*DH32ENTRY  DevHlp32_ResetEventSem)(unsigned long, int*);
        } version_7;
        struct {
            int  (*DH32ENTRY  DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY  DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY  DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*DH32ENTRY2 DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY  DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY  DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY  DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY  DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY  DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY  DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY  DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY  DevHlp32_Yield)(void);
            int  (*DH32ENTRY  DevHlp32_GetInfoSegs)(struct InfoSegGDT **, struct InfoSegLDT **);
            int           (*DH32ENTRY vsprintf)(char *, const char *, va_list);
            unsigned long (*DH32ENTRY strtoul) (const char *, char **, int);
            long          (*DH32ENTRY atol) (const char *);
            char *        (*DH32ENTRY strupr) (char *s);
            char *        (*DH32ENTRY strpbrk) (const char *, const char *);
            int           (*DH32ENTRY fnmatch) (const char *, const char *, int);
            long          (*DH32ENTRY strtol) (const char *, char **, int);
	    int (*DH32ENTRY  sec32_attach_ses)(void *);
            int (*DH32ENTRY  DevHlp32_setIRQ)(unsigned short, unsigned short, unsigned short, unsigned short);
            int (*DH32ENTRY2 DevHlp32_EOI)(unsigned short);
            int (*DH32ENTRY  DevHlp32_UnSetIRQ)(unsigned short, unsigned short);
            int (*DH32ENTRY  DevHlp32_PageListToLin)(unsigned long, struct PageList *, void **);
            int (*DH32ENTRY  DevHlp32_AllocGDTSelector)(unsigned short *, int);
            int (*DH32ENTRY2 DevHlp32_FreeGDTSelector)(unsigned short);
            int (*DH32ENTRY  DevHlp32_AttachDD)(char *, struct ddtable *);
            int (*DH32ENTRY  DevHlp32_GetDosVar)(int, PTR16 *, int);
        } version_6;
        struct {
            int  (*DH32ENTRY  DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY  DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY  DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*DH32ENTRY2 DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY  DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY  DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY  DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY  DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY  DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY  DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY  DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY  DevHlp32_Yield)(void);
            int  (*DH32ENTRY  DevHlp32_GetInfoSegs)(struct InfoSegGDT **, struct InfoSegLDT **);
            int           (*DH32ENTRY vsprintf)(char *, const char *, va_list);
            unsigned long (*DH32ENTRY strtoul) (const char *, char **, int);
            long          (*DH32ENTRY atol) (const char *);
            char *        (*DH32ENTRY strupr) (char *s);
            char *        (*DH32ENTRY strpbrk) (const char *, const char *);
            int           (*DH32ENTRY fnmatch) (const char *, const char *, int);
            long          (*DH32ENTRY strtol) (const char *, char **, int);
	    int (*DH32ENTRY  sec32_attach_ses)(void *);
            int (*DH32ENTRY  DevHlp32_setIRQ)(unsigned short, unsigned short, unsigned short, unsigned short);
            int (*DH32ENTRY2 DevHlp32_EOI)(unsigned short);
            int (*DH32ENTRY  DevHlp32_UnSetIRQ)(unsigned short, unsigned short);
        } version_5;
        struct {
            int  (*DH32ENTRY DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*_Optlink  DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY DevHlp32_Yield)(void);
            int  (*DH32ENTRY DevHlp32_GetInfoSegs)(struct InfoSegGDT **, struct InfoSegLDT **);
            int           (*DH32ENTRY vsprintf)(char *, const char *, va_list);
            unsigned long (*DH32ENTRY strtoul) (const char *, char **, int);
            long          (*DH32ENTRY atol) (const char *);
            char *        (*DH32ENTRY strupr) (char *s);
            char *        (*DH32ENTRY strpbrk) (const char *, const char *);
            int           (*DH32ENTRY fnmatch) (const char *, const char *, int);
            long          (*DH32ENTRY strtol) (const char *, char **, int);
	    int (*DH32ENTRY sec32_attach_ses)(void *);
        } version_4;
        struct {
            int  (*DH32ENTRY DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*_Optlink  DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY DevHlp32_Yield)(void);
            int  (*DH32ENTRY DevHlp32_GetInfoSegs)(struct InfoSegGDT **, struct InfoSegLDT **);
            int           (*DH32ENTRY vsprintf)(char *, const char *, va_list);
            unsigned long (*DH32ENTRY strtoul) (const char *, char **, int);
            long          (*DH32ENTRY atol) (const char *);
            char *        (*DH32ENTRY strupr) (char *s);
            char *        (*DH32ENTRY strpbrk) (const char *, const char *);
            int           (*DH32ENTRY fnmatch) (const char *, const char *, int);
            long          (*DH32ENTRY strtol) (const char *, char **, int);
        } version_3;
        struct {
            int  (*DH32ENTRY DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*_Optlink  DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY DevHlp32_Yield)(void);
            int  (*DH32ENTRY DevHlp32_GetInfoSegs)(struct InfoSegGDT **, struct InfoSegLDT **);
            int           (*DH32ENTRY vsprintf)(char *, const char *, va_list);
            unsigned long (*DH32ENTRY strtoul) (const char *, char **, int);
            long          (*DH32ENTRY atol) (const char *);
            char *        (*DH32ENTRY strupr) (char *s);
            char *        (*DH32ENTRY strpbrk) (const char *, const char *);
            int           (*DH32ENTRY fnmatch) (const char *, const char *, int);
        } version_2;
        struct {
            int  (*DH32ENTRY DevHlp32_SaveMessage)(char *, int);
            void (*DH32ENTRY DevHlp32_InternalError)(char *, int);
            int  (*DH32ENTRY DevHlp32_VMAlloc)(unsigned long, unsigned long, unsigned long, void **);
            int  (*_Optlink  DevHlp32_VMFree)(void *);
            int  (*DH32ENTRY DevHlp32_VMLock)(unsigned long, void *, unsigned long, void *, void *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_VMUnlock)(void *);
            int  (*DH32ENTRY DevHlp32_VirtToLin)(PTR16, void **);
            int  (*DH32ENTRY DevHlp32_ProcBlock)(unsigned long, long, short);
            int  (*DH32ENTRY DevHlp32_ProcRun)(unsigned long);
            int  (*DH32ENTRY DevHlp32_LinToPageList)(void *, unsigned long, struct PageList *, unsigned long *);
            int  (*DH32ENTRY DevHlp32_Security)(unsigned long, void *);
            void (*DH32ENTRY DevHlp32_Yield)(void);
        } version_1;
    };
};



/**************************************************************************************/
/*** DevHlp32_VerifyAccess                                                          ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_VerifyAccess(
				      PTR16 address,		/* ebp + 8 */
				      int size,			/* ebp + 12 */	
                        	      int flags                 /* ebp + 16  */
                                     ) {
    return DevHelp32.version_7.DevHlp32_VerifyAccess(address,size,flags);
}
#else
extern int DH32ENTRY DevHlp32_VerifyAccess(
				      PTR16 address,		/* ebp + 8 */
				      unsigned short size,	/* ebp + 12 */	
                        	      int flags                 /* ebp + 16  */
                                     );
#endif
#define VERIFY_READONLY	0
#define VERIFY_READWRITE 1


/**************************************************************************************/
/*** DevHlp32_OpenEventSem                                                          ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_OpenEventSem(
                        	      unsigned long handle        /* ebp + 8  */
                                     ) {
    return DevHelp32.version_7.DevHlp32_OpenEventSem(handle);
}
#else
extern int DH32ENTRY DevHlp32_OpenEventSem(
                        	      unsigned long handle        /* ebp + 8  */
                                     );
#endif


/**************************************************************************************/
/*** DevHlp32_CloseEventSem                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_CloseEventSem(
                        	      unsigned long handle        /* ebp + 8  */
                                     ) {
    return DevHelp32.version_7.DevHlp32_CloseEventSem(handle);
}
#else
extern int DH32ENTRY DevHlp32_CloseEventSem(
                        	      unsigned long handle        /* ebp + 8  */
                                     );
#endif


/**************************************************************************************/
/*** DevHlp32_PostEventSem                                                          ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_PostEventSem(
                        	      unsigned long handle        /* ebp + 8  */
                                     ) {
    return DevHelp32.version_7.DevHlp32_PostEventSem(handle);
}
#else
extern int DH32ENTRY DevHlp32_PostEventSem(
                        	      unsigned long handle        /* ebp + 8  */
                                     );
#endif


/**************************************************************************************/
/*** DevHlp32_ResetEventSem                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_ResetEventSem(
                         	      unsigned long handle,       /* ebp + 8  */
                         	      int *nposts                 /* ebp + 12 */
                                     ) {
    return DevHelp32.version_7.DevHlp32_ResetEventSem(handle,nposts);
}
#else
extern int DH32ENTRY DevHlp32_ResetEventSem(
                        	      unsigned long handle,       /* ebp + 8  */
                         	      int *nposts                 /* ebp + 12 */
                                     );
#endif


/**************************************************************************************/
/*** DevHlp32_VMLock                                                                ***/
/**************************************************************************************/
#define VMDHL_WRITE             0x0008
#define VMDHL_LONG              0x0010
#define VMDHL_VERIFY            0x0020

#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_VMLock(                                                             
                                     unsigned long   flags,
                                     void           *addr,
                                     unsigned long   length,
                                     void           *pPageList,
                                     void           *pLockHandle,
                                     unsigned long *pPageListCount
                                    ) {
    return DevHelp32.version_1.DevHlp32_VMLock(flags, addr, length, pPageList, pLockHandle, pPageListCount);
}
#else                                                   
extern int DH32ENTRY DevHlp32_VMLock(                                                             
            unsigned long   flags,
            void           *addr,
            unsigned long   length,
            void           *pPageList,
            void           *pLockHandle,
            unsigned long *pPageListCount
           );                                                            
#endif

/**************************************************************************************/
/*** DevHlp32_setIRQ                                                                ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_setIRQ(
                                     unsigned short offset_irq,		/* ebp + 8  */
      	    	                     unsigned short interrupt_level,	/* ebp + 12 */
		                     unsigned short sharing_flag, 	/* ebp + 16 */
		                     unsigned short data16_segment 	/* ebp + 20 */
                              ) {
    return DevHelp32.version_5.DevHlp32_setIRQ(offset_irq, interrupt_level, sharing_flag, data16_segment);
}
#else
extern int DH32ENTRY DevHlp32_setIRQ(
                                     unsigned short offset_irq,		/* ebp + 8  */
		                     unsigned short interrupt_level,	/* ebp + 12 */
		                     unsigned short sharing_flag, 	/* ebp + 16 */
		                     unsigned short data16_segment 	/* ebp + 20 */
                                    );

#endif

/**************************************************************************************/
/*** DevHlp32_EOI                                                                   ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY2 DevHlp32_EOI(
      	    	                   unsigned short interrupt_level	/* ax */
                                  ) {
    return DevHelp32.version_5.DevHlp32_EOI(interrupt_level);
}
#else
extern int DH32ENTRY2 DevHlp32_EOI(
		                   unsigned short interrupt_level	/* ax */
                                  );

#endif

/**************************************************************************************/
/*** DevHlp32_AllocGDTSelector                                                      ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_AllocGDTSelector(
                                               unsigned short *psel, 
                                               int count
                                              ) {
    return DevHelp32.version_6.DevHlp32_AllocGDTSelector(psel, count);
}
#else
extern int DH32ENTRY DevHlp32_AllocGDTSelector(
                                               unsigned short *psel, 
                                               int count
                                              );
#endif

/**************************************************************************************/
/*** DevHlp32_FreeGDTSelector                                                       ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY2 DevHlp32_FreeGDTSelector(
                                               unsigned short sel      /* ax */ 
                                              ) {
    return DevHelp32.version_6.DevHlp32_FreeGDTSelector(sel);
}
#else
extern int DH32ENTRY2 DevHlp32_FreeGDTSelector(
                                               unsigned short sel      /* ax */ 
                                              );
#endif

/**************************************************************************************/
/*** DevHlp32_UnSetIRQ                                                              ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_UnSetIRQ(
      	    	                       unsigned short interrupt_level,	/* ebp + 8  */
  		                       unsigned short data16_segment 	/* ebp + 12 */
                                      ) {
    return DevHelp32.version_5.DevHlp32_UnSetIRQ(interrupt_level, data16_segment);
}
#else
extern int DH32ENTRY DevHlp32_UnSetIRQ(
      	    	                       unsigned short interrupt_level,	/* ebp + 8  */
  		                       unsigned short data16_segment 	/* ebp + 12 */
                                      );
#endif

/**************************************************************************************/
/*** DevHlp32_VMUnlock                                                              ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_VMUnlock(
                                       void *pLockHandle   /* ebp + 8 */
                                      ) {
    return DevHelp32.version_1.DevHlp32_VMUnlock(pLockHandle);
}
#else
extern int DH32ENTRY DevHlp32_VMUnlock(
                                       void *pLockHandle   /* ebp + 8 */
                                      );
#endif

/**************************************************************************************/
/*** DevHlp32_VMAlloc                                                               ***/
/**************************************************************************************/
#define VMDHA_16M               0x0001 
#define VMDHA_FIXED             0x0002
#define VMDHA_SWAP              0x0004
#define VMDHA_CONTIG            0x0008
#define VMDHA_PHYS              0x0010
#define VMDHA_PROCESS           0x0020
#define VMDHA_SGSCONT           0x0040
#define VMDHA_RESERVE           0x0100
#define VMDHA_USEHIGHMEM        0x0800

#define VMDHA_NOPHYSADDR        0xFFFFFFFF

#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_VMAlloc(
                                      unsigned long  Length,      /* ebp + 8  */
                                      unsigned long  PhysAddr,    /* ebp + 12 */
                                      unsigned long  Flags,       /* ebp + 16 */
                                      void         **LinAddr      /* ebp + 20 */
                                     ) {
    return DevHelp32.version_1.DevHlp32_VMAlloc(Length, PhysAddr, Flags, LinAddr);
}
#else
extern int DH32ENTRY DevHlp32_VMAlloc(
                                      unsigned long  Length,      /* ebp + 8  */
                                      unsigned long  PhysAddr,    /* ebp + 12 */
                                      unsigned long  Flags,       /* ebp + 16 */
                                      void         **LinAddr      /* ebp + 20 */
                                     );
#endif

/**************************************************************************************/
/*** DevHlp32_VMFree                                                                ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int _Optlink DevHlp32_VMFree(
                                    void *addr        /* eax */
                                   ) {
    return DevHelp32.version_1.DevHlp32_VMFree(addr);
}
#else
extern int _Optlink DevHlp32_VMFree(
                                    void *addr        /* eax */
                                   );
#endif

/**************************************************************************************/
/*** DevHlp32_VirtToLin                                                             ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_VirtToLin(
                                        PTR16  virt, // [ebp + 8]
                                        void **plin  // [ebp + 12]
                                       ) {
    return DevHelp32.version_1.DevHlp32_VirtToLin(virt, plin);
}
#else
extern int DH32ENTRY DevHlp32_VirtToLin(
                                        PTR16  virt, // [ebp + 8]
                                        void **plin  // [ebp + 12]
                                       );
#endif

/**************************************************************************************/
/*** DevHlp32_GetDosVar                                                             ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_GetDosVar(
                                        int     index,      /* ebp + 8  */
                                        PTR16  *value,      /* ebp + 12 */
                                        int     member      /* ebp + 16 */
                                       ) {
    return DevHelp32.version_6.DevHlp32_GetDosVar(index, value, member);
}
#else
extern int DH32ENTRY DevHlp32_GetDosVar(
                                        int     index,      /* ebp + 8  */
                                        PTR16  *value,      /* ebp + 12 */
                                        int     member      /* ebp + 16 */
                                       );
#endif

/**************************************************************************************/
/*** DevHlp32_Security                                                              ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_Security(
				       unsigned long   func,     /* ebp + 8  */
                                       void           *ptr       /* ebp + 12 */
                                      ) {
    return DevHelp32.version_1.DevHlp32_Security(func, ptr);
}
#else
extern int DH32ENTRY DevHlp32_Security(
				       unsigned long   func,     /* ebp + 8  */
                                       void           *ptr       /* ebp + 12 */
                                      );
#endif

// #define DHSEC_GETEXPORT  0x48a78df8
// #define DHSEC_SETIMPORT  0x73ae3627
// #define DHSEC_GETINFO    0x33528882

/**************************************************************************************/
/*** DevHlp32_AttachDD                                                              ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_AttachDD(char *ddname, struct ddtable *table) {
    return DevHelp32.version_6.DevHlp32_AttachDD(ddname, table);
}
#else
extern int DH32ENTRY DevHlp32_AttachDD(char *ddname, struct ddtable *table);
#endif

/**************************************************************************************/
/*** DevHlp32_ProcBlock                                                             ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_ProcBlock(
                          		unsigned long  eventid,       /* bp + 8  */
                          		long           timeout,       /* bp + 12 */
                          		short          interruptible  /* bp + 16 */
                         	       ) {
    return DevHelp32.version_1.DevHlp32_ProcBlock(eventid, timeout, interruptible);
}
#else
extern int DH32ENTRY DevHlp32_ProcBlock(
                          unsigned long  eventid,       /* bp + 8  */
                          long           timeout,       /* bp + 12 */
                          short          interruptible  /* bp + 16 */
                         );
#endif

/**************************************************************************************/
/*** DevHlp32_ProcRun                                                               ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_ProcRun(
                        	      unsigned long eventid        /* ebp + 8  */
                                     ) {
    return DevHelp32.version_1.DevHlp32_ProcRun(eventid);
}
#else
extern int DH32ENTRY DevHlp32_ProcRun(
                        	      unsigned long eventid        /* ebp + 8  */
                                     );
#endif



/**************************************************************************************/
/*** DevHlp32_PageListToLin                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_PageListToLin(
                                            unsigned long     size,        /* ebp + 8  */
                                            struct PageList  *pPageList,   /* ebp + 12 */
                                            void            **pLin         /* ebp + 16 */
                                           ) {
    return DevHelp32.version_6.DevHlp32_PageListToLin(size, pPageList, pLin);
}
#else
extern int DH32ENTRY DevHlp32_PageListToLin(
                                            unsigned long     size,        /* ebp + 8  */
                                            struct PageList  *pPageList,   /* ebp + 12 */
                                            void            **pLin         /* ebp + 16 */
                                           );
#endif

/**************************************************************************************/
/*** DevHlp32_LinToPageList                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_LinToPageList(
                                void            *lin,         /* ebp + 8  */
                                unsigned long    size,        /* ebp + 12 */
                                struct PageList *pages,        /* ebp + 16 */
                                unsigned long   *nr_pages    /* ebp + 20 */
                               ) {
    return DevHelp32.version_1.DevHlp32_LinToPageList(lin, size, pages, nr_pages);
}
#else
extern int DH32ENTRY DevHlp32_LinToPageList(
                                void            *lin,         /* ebp + 8  */
                                unsigned long    size,        /* ebp + 12 */
                                struct PageList *pages,        /* ebp + 16 */
                                unsigned long   *nr_pages    /* ebp + 20 */
                               );
#endif

/**************************************************************************************/
/*** DevHlp32_GetInfoSegs                                                           ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_GetInfoSegs(
                    struct InfoSegGDT **ppSysInfoSeg,  /* ebp + 8  */
                    struct InfoSegLDT **ppLocInfoSeg   /* ebp + 12 */
                   ) {
    return DevHelp32.version_2.DevHlp32_GetInfoSegs(ppSysInfoSeg, ppLocInfoSeg);
}
#else
extern int DH32ENTRY DevHlp32_GetInfoSegs(
                    struct InfoSegGDT **ppSysInfoSeg,  /* ebp + 8  */
                    struct InfoSegLDT **ppLocInfoSeg   /* ebp + 12 */
                   );
#endif

/**************************************************************************************/
/*** DevHlp32_SaveMessage                                                           ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY DevHlp32_SaveMessage(
   			                  char *msg,		/* ebp + 8  */
			                  int   len		/* ebp + 12 */
                		         ) {
    return DevHelp32.version_1.DevHlp32_SaveMessage(msg, len);
}
#else
extern int DH32ENTRY DevHlp32_SaveMessage(
   			                  char *msg,		/* ebp + 8  */
			                  int   len		/* ebp + 12 */
                		         );
#endif



/**************************************************************************************/
/*** DevHlp32_InternalError                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE void DH32ENTRY DevHlp32_InternalError(
   			                     char *msg,		/* ebp + 8  */
			                     int   len		/* ebp + 12 */
                		            ) {
    DevHelp32.version_1.DevHlp32_InternalError(msg, len);
}
#else
extern void DH32ENTRY DevHlp32_InternalError(
   			                     char *msg,		/* ebp + 8  */
			                     int   len		/* ebp + 12 */
                		            );
#endif

/**************************************************************************************/
/*** DevHlp32_Yield                                                                 ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE void DH32ENTRY DevHlp32_Yield(void) {
    DevHelp32.version_1.DevHlp32_Yield();
}
#else
extern void DH32ENTRY DevHlp32_Yield(void);
#endif

/**************************************************************************************/
/*** fsh32_yield                                                                    ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE void DH32ENTRY fsh32_yield(void) {
    DevHelp32.version_1.DevHlp32_Yield();
}
#else
INLINE void DH32ENTRY fsh32_yield(void) {
    DevHlp32_Yield();
}
#endif


/**************************************************************************************/
/*** sec32_attach_ses                                                               ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY sec32_attach_ses (void *SecHlp) {
    return DevHelp32.version_4.sec32_attach_ses(SecHlp);
}
#else
extern int DH32ENTRY sec32_attach_ses (void *SecHlp);
#endif

/**************************************************************************************/
/*** vsprintf                                                                       ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY vsprintf(char *buf, const char *fmt, va_list args) {
    return DevHelp32.version_2.vsprintf(buf, fmt, args);
}
#else
extern int DH32ENTRY vsprintf(char *buf, const char *fmt, va_list args);
#endif

/**************************************************************************************/
/*** strtoul                                                                        ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE unsigned long DH32ENTRY __mwdd32_strtoul (const char *string, char **end_ptr, int radix) {
    return DevHelp32.version_2.strtoul (string, end_ptr, radix);
}
#else
extern unsigned long DH32ENTRY __mwdd32_strtoul (const char *string, char **end_ptr, int radix);
#endif

#define strtoul(__s, __e, __r) __mwdd32_strtoul((__s), (__e), (__r))

/**************************************************************************************/
/*** strtol                                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE long DH32ENTRY __mwdd32_strtol (const char *string, char **end_ptr, int radix) {
    return DevHelp32.version_3.strtol (string, end_ptr, radix);
}
#else
extern long DH32ENTRY __mwdd32_strtol (const char *string, char **end_ptr, int radix);
#endif

#define strtol(__s, __e, __r)  __mwdd32_strtol((__s), (__e), (__r))

/**************************************************************************************/
/*** atol                                                                           ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE long DH32ENTRY __mwdd32_atol (const char *string) {
    return DevHelp32.version_2.atol(string);
}
#else
extern long DH32ENTRY __mwdd32_atol (const char *string);
#endif

#define atol(__s)              __mwdd32_atol((__s))

/**************************************************************************************/
/*** strupr                                                                         ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE char * DH32ENTRY __mwdd32_strupr (char *string) {
    return DevHelp32.version_2.strupr(string);
}
#else
extern char * DH32ENTRY __mwdd32_strupr (char *string);
#endif

#define strupr(__s)            __mwdd32_strupr((__s))

/**************************************************************************************/
/*** strpbrk                                                                        ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE char * DH32ENTRY __mwdd32_strpbrk (const char *string1, const char *string2) {
    return DevHelp32.version_2.strpbrk (string1, string2);
}
#else
extern char * DH32ENTRY __mwdd32_strpbrk (const char *string1, const char *string2);
#endif

#define strpbrk(__s1, __s2)    __mwdd32_strpbrk((__s1), (__s2))

/**************************************************************************************/
/*** fnmatch                                                                        ***/
/**************************************************************************************/
#ifdef DYNAMIC_DEVHELP
INLINE int DH32ENTRY __mwdd32_fnmatch (const char *mask, const char *name, int flags) {
    return DevHelp32.version_2.fnmatch (mask, name, flags);
}
#else
extern int DH32ENTRY __mwdd32_fnmatch (const char *mask, const char *name, int flags);
#endif

#define fnmatch(__m, __n, __f) __mwdd32_fnmatch((__m), (__n), (__f))


/**************************************************************************************/
/*** MVDM.DLL entry points                                                          ***/
/**************************************************************************************/
extern void * _Pascal VDHQueryLin(PTR16);

#endif

