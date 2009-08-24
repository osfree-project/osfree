/*  Mem

    Copyright (c) Express Software 1997.
    All Rights Reserved.

    Created by: Joseph Cosentino.
    With help from Michal Meller for the convmemfree routine.
    
    Clean up and bug fixes March 2001, Bart Oldeman, with a patch
    applied from Martin Stromberg.
    Thanks to Arkady V.Belousov for suggestions.

    April 2001, tom ehlert
    moved to small memory model to cure some strange bugs
    change largest executable program size from mem_free to
    largest free MCB
        
    Added (for Win98) with the help of Alain support for INT2F AX=4309 (Win98)

    Bart: moved to tiny model and added malloc checks. Then moved
    it back to the small model.

    August 2001, Bart Oldeman. Licensed under the GPL; see \doc\mem\license.txt.
    Bug fixes + XMS enhancements from Martin and Tom.

    MEM 1.4, Mar 2003:
    * use XXMS functions if possible for > 64 MB configurations (bug #1045)
    * implemented /p (initially by Bryan Reifsnyder) (part of bug #1467)
    * XMS version reported as x.xx (bug #1598)
    * code cleanups to reduce size including:
      merging of duplicate code
      disable unused code (search_vectors())
      use our own version of malloc and printf (parts done by Tom Ehlert)
      less use of long ints.
      use UPX.
    * remove dependency on TASM (using a minimal amount of external assembly
      (NASM) or #pragma aux)
    * MEM's own environment is now correctly counted as part of the free space
    * fixed all compiler warnings.
    * options can be concatenated without spaces as in mem/f/e/x/p
    * fixes to bugs and changes noted by Arkady (that are not mentioned above):
      - main: diagnosed all wrong options.
      - check_name: now checks memory block size.
      - removed all strupr(), adapted check_name() with names copying.
      - EMS_list: EMS driver version showed as unsigned values.
      - search_vectors: missed FAR modifier; FP_SEG(ivp) shifted by one.
      - search_sd: check for end should be "<=" instead "<".
      - register_mcb: forgot "mlist =" before "search_sd()".
      - xms_list: free_handles_tmp used outside of "if XMS call successful".
      - XMS and XMS driver minor version should be two BCD digit long.
      - make_mcb_list: added marking as free all MEM's MCBs, not only environment.
      - check_upper: UMB size counted incorrectly.
      - main: command line processed before actions.
      - added support for '-' options prefix.

    MEM 1.5, April 2004
    * implement /c option
    * fix display of sub mcb blocks; detect EBDA, DRIVEDATA and SECTORBUF
    * fix display of device names with garbage beyond the '\0'
    * fix problem with FD EMM386 NOEMS "Internal EMS Error": EMS memory was
      available, just no page frame.
    * merge prf.c with newest version in the kernel source
    * made output more MS compatible, display extended memory without an
      XMS driver installed as well, do NOT count MEM environment as free space,
      don't merge free blocks in the output anymore.
    * various small cleanups

    MEM 1.6, April 2004
    * minor output tweaks, don't upcase names anymore
    * try to detect UMB holes and don't count them as upper memory
    * display UMB holes as "reserved" in mem/f output
    * display version for "mem/?"
*/

#define MEM_MAJOR 1
#define MEM_MINOR 7

/*  Be sure to compile with word alignment OFF !!! */
#if defined(_MSC_VER)
#define asm __asm
#if _MSC_VER >= 700
#pragma warning(disable:4103)
#endif
#pragma pack(1)
#elif defined(_QC) || defined(__WATCOMC__)
#pragma pack(1)
#elif defined(__ZTC__)
#pragma ZTC align 1
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a-
#endif

#ifdef __WATCOMC__
#define PRAGMAS
#define getvect _dos_getvect
#define outportb outp
#define inportb inp
#define biosmemory _bios_memsize
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include <bios.h>
#include <conio.h>
#include "kitten.h"

typedef unsigned char BYTE;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

#define FALSE 0
#define TRUE  1

#define MT_NONE    0
#define MT_FREE    1
#define MT_SYSCODE 2
#define MT_SYSDATA 3
#define MT_PROGRAM 4
#define MT_ENV     5
#define MT_DATA    6
#define MT_RESERVED 7
#define MT_DEVICE  8
#define MT_DOSDATA 9
#define MT_IFS     10

#define _(set,message_number,message) kittengets(set,message_number,message)

int num_lines = -1;

typedef struct device_header
{
    struct device_header far*next;
    ushort attr;
    ushort strategy_rutin;
    ushort interrupt_rutin;
    char name[8];
} DEVICE_HEADER;

typedef struct devinfo
{
    struct device_header far *addr;
    char devname[9];
    char *progname;
    ushort attr;
    uchar firstdrive, lastdrive;
    struct devinfo *next;
} DEVINFO;

typedef struct dpb
{
    uchar drive_num;
    uchar unit_number;
    ushort bytes_in_sector;
    uchar lsec_num;
    uchar log_base;
    ushort reserved_num;
    uchar FAT_num;
    ushort rootentry_num;
    ushort first_sector;
    ushort largest_cluster;
    ushort sectors_in_FAT;
    ushort root_firstsector;
    DEVICE_HEADER far*device_addr;
    uchar media_desc;
    uchar block_flag;
    struct dpb far*next_dpb;
} DPB;

typedef struct
{
    uchar type;
    ushort start;
    ushort size;
    uchar unused[3];
    char name[8];
} SD_HEADER;

typedef struct
{
    uchar type;
    ushort owner;
    ushort size;
    uchar unused[3];
    char name[8];
} MCB;

typedef struct minfo
{
    uchar type;
    ushort seg;
    ushort owner;
    ushort environment;
    char *name;
    ushort size;
    uchar classified;
#if 0 /* presently unused */
    uchar vecnum;
    uchar *vectors;
#endif
    struct minfo *next;
} MINFO;

typedef struct ems_handle
{
    ushort handle;
    ushort pages;
} EMS_HANDLE;

typedef struct xms_handle
{
    ushort handle;
    ulong size;
    ushort locks;
    struct xms_handle *next;
} XMS_HANDLE;

struct MCBheader {
    char type;
    unsigned int pid;
    unsigned int length;
    char reserved[3];
    char progname[8];
};

typedef struct {
    uchar vermajor, verminor;
    unsigned totalhandle, freehandle, usehandle, frame;
    unsigned size, free;
    EMS_HANDLE *handles;
} EMSINFO;

typedef struct {
    ulong total, free, largest;
    unsigned freehandle;
    uchar hma;
    uchar is_386;
    uchar vermajor, verminor, a20;
    uchar drv_vermajor, drv_verminor;
    XMS_HANDLE *handles;
} XMSINFO;

typedef struct upperinfo {
    unsigned total, free, largest;
} UPPERINFO;
    
/* structures for INT 2F AX=4309 */
typedef struct{
    unsigned char flag;
    unsigned char locks;
    unsigned long xmsAddrKb;
    unsigned long xmsBlkSize;
} XMS_HANDLE_DESCRIPTOR;

typedef struct{
    unsigned char checkByte;
    unsigned char sizeOfDesc;
    unsigned short numbOfHandles;
    XMS_HANDLE_DESCRIPTOR far* xmsHandleDescr;
} XMS_HANDLE_TABLE;

#define SMAP  0x534d4150UL
struct e820map 
{
    ulong baselow;
    ulong basehigh;
    ulong lenlow;
    ulong lenhigh;
    ulong type;
};

typedef void far (*xms_drv_t)(void);
void far (*xms_drv)(void);

#ifdef PRAGMAS

uchar get_oem_number(void);
#pragma aux get_oem_number = \
    "mov ax, 0x3000" \
    "int 0x21" \
value [bh]

char dos_in_hma_(void);
#pragma aux dos_in_hma_ = \
    "mov ax, 0x3306" \
    "xor dx, dx" \
    "int 0x21" \
value [dh];

char far * dos_list_of_lists(void);
#pragma aux dos_list_of_lists = \
    "mov ah, 0x52" \
    "int 0x21" \
value [es bx];

uchar get_upperlink(void);
#pragma aux get_upperlink = \
    "mov ax, 0x5802" \
    "int 0x21" \
value [al]

long dos_set_upperlink(uchar link);
#pragma aux dos_set_upperlink = \
    "mov ax, 0x5803" \
    "int 0x21" \
    "sbb dx, dx" \
parm [bx] value [dx ax] ;

int call_ems_driver(unsigned char ah);
#pragma aux call_ems_driver = \
    "int 0x67" \
parm [ah] value [ax]

long call_ems_driver_bx(unsigned char ah);
#pragma aux call_ems_driver_bx = \
    "int 0x67" \
    "cwd" \
parm [ah] value [dx bx]

long call_ems_driver_dx(unsigned char ah);
#pragma aux call_ems_driver_dx = \
    "int 0x67" \
    "mov bx, dx" \
    "cwd" \
parm [ah] value [dx bx]

long ems_get_pages(EMS_HANDLE far *handle);
#pragma aux ems_get_pages = \
    "mov ah, 0x4d" \
    "int 0x67" \
    "cwd" \
value [dx bx] parm [es di]

int ems_get_handle_name(unsigned handle, char far *handlename);
#pragma aux ems_get_handle_name = \
    "mov ax, 0x5300" \
    "int 0x67" \
value [ax] parm [dx] [es di]

long ems_total_num_handles(void);
#pragma aux ems_total_num_handles = \
    "mov ax, 0x5402" \
    "int 0x67" \
    "cwd" \
value [dx bx]

unsigned char xms_available(void);
#pragma aux xms_available = \
    "mov ax, 0x4300" \
    "int 0x2f" \
value [al]

XMS_HANDLE_TABLE far* get_xmsHanTab(void);
#pragma aux get_xmsHanTab = \
    "mov ax, 0x4309" \
    "int 0x2f" \
    "cmp al, 0x43" \
    "je ok" \
    "xor bx, bx" \
    "mov es, bx" \
"ok:" \
value [es bx];

xms_drv_t get_xms_drv(void);
#pragma aux get_xms_drv = \
    "mov ax,0x4310" \
    "int 0x2f" \
value [es bx];

ulong call_xms_driver_dx_bl_al(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_dx_bl_al = \
    "call dword ptr [xms_drv]" \
    "mov ah, bl" \
parm [ah] [dx] value [dx ax] modify [bx];

ulong call_xms_driver_bx_ax(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_bx_ax = \
    "call dword ptr [xms_drv]" \
parm [ah] [dx] value [bx ax];

ulong call_xms_driver_edx(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_edx = \
    "call dword ptr [xms_drv]" \
    ".386" \
    "test bl, bl" \
    "jz noerror" \
    "xor edx, edx" \
"noerror:" \
    "mov ax, dx" \
    "shr edx, 16" \
    ".8086" \
parm [ah] [dx] value [dx ax] modify [bx]

ulong call_xms_driver_eax(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_eax = \
    "call dword ptr [xms_drv]" \
    ".386" \
    "mov edx, eax" \
    "test bl, bl" \
    "jz noerror" \
    "xor edx, edx" \
"noerror:" \
    "mov ax, dx" \
    "shr edx, 16" \
    ".8086" \
parm [ah] [dx] value [dx ax] modify [bx]

unsigned check_8800(void);
#pragma aux check_8800 =\
    "mov ax, 0x8800" \
    "int 0x15" \
    "jnc noerror" \
    "xor ax, ax" \
"noerror:"\
value [ax] modify [si]

ulong check_e801(void);
#pragma aux check_e801 =\
    "mov ax, 0xe801" \
    "xor bx, bx" \
    "int 0x15" \
    "jnc noerror" \
    "or bx, bx" \
    "jnz noerror" \
    "xor ax, ax" \
"noerror:" \
    "mov dx, 1024" \
    "mul dx" \
    "add dx, bx" \
value [dx ax] modify [si bx]

int is_386_(void);
#pragma aux is_386_ = \
    "pushf" \
    "pushf" \
    "pop ax" \
    "or ax, 7000h" \
    "push ax"\
    "popf"\
    "pushf"\
    "pop ax"\
    "popf"\
value [ax];

/* and now we can use some help from the linux kernel */
/* adapted from v 2.4.1 linux/arch/i386/boot/setup.S */
ulong check_e820(struct e820map far *e820map, ulong counter);
#pragma aux check_e820 = \
     ".386" \
     "shl ebx, 16" \
     "mov bx, ax" \
     "mov eax, 0x0000e820"        /* e820, upper word zeroed    */ \
     "mov edx, 0x534d4150"        /* ascii 'SMAP'               */ \
     "mov ecx, 20"                /* size of the e820map        */ \
     "int 15h"                    /* make the call              */ \
     "jc no386"                   /* fall to e801 if it fails   */ \
     "cmp eax, 0x534d4150"        /* check the return is `SMAP` */ \
     "je yes386"                  /* fall to e801 if it fails   */ \
"no386:" \
     "xor ebx, ebx" \
"yes386:" \
     "mov ax, bx"  \
     "shr ebx, 16" \
     ".8086" \
parm [es di] [bx ax] value [bx ax] modify [si];

#else

/* Get OEM number. */
static uchar get_oem_number(void)
{
    union REGS regs;
    regs.x.ax = 0x3000;
    intdos(&regs, &regs);
    return regs.h.bh;
}

static char dos_in_hma_(void)
{
    union REGS regs;

    regs.x.ax = 0x3306;
    regs.x.dx = 0;
    intdos(&regs, &regs);
    return regs.h.dh & 0x10;
}

/* Get pointer to LL struct. */
static char far * dos_list_of_lists(void)
{
    union REGS regs;
    struct SREGS sregs;

    regs.h.ah = 0x52;
    intdosx(&regs, &regs, &sregs);
    return MK_FP(sregs.es, regs.x.bx);
}

static uchar get_upperlink(void)
{
    union REGS regs;

    regs.x.ax = 0x5802;
    intdos(&regs, &regs);
    return regs.h.al;
}

static long dos_set_upperlink(uchar link)
{
    union REGS regs;

    regs.x.ax = 0x5803;
    regs.x.bx = link;
    intdos(&regs, &regs);
    if (regs.x.cflag)
        return regs.x.ax | 0xffff0000L;
    else
        return regs.x.ax;
}

static int call_ems_driver(unsigned char ah)
{
    union REGS regs;
    regs.h.ah = ah;
    int86(0x67, &regs, &regs);
    return regs.x.ax;
}

static long call_ems_driver_bx(unsigned char ah)
{
    union REGS regs;
    regs.h.ah = ah;
    int86(0x67, &regs, &regs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.bx;
}

static long call_ems_driver_dx(unsigned char ah)
{
    union REGS regs;
    regs.h.ah = ah;
    int86(0x67, &regs, &regs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.dx;
}

static long ems_get_pages(EMS_HANDLE far *handle)
{
    union REGS regs;
    struct SREGS sregs;
        
    regs.h.ah = 0x4d;
    regs.x.di = FP_OFF(handle);
    sregs.es = FP_SEG(handle);
    int86x(0x67, &regs, &regs, &sregs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.bx;
}

static int ems_get_handle_name(unsigned handle, char far *handlename)
{
    union REGS regs;
    struct SREGS sregs;
        
    regs.x.ax = 0x5300;
    regs.x.dx = handle;
    regs.x.di = FP_OFF(handlename);
    sregs.es = FP_SEG(handlename);
    int86x(0x67, &regs, &regs, &sregs);
    return regs.x.ax;
}

static long ems_total_num_handles(void)
{
    union REGS regs;
        
    regs.x.ax = 0x5402;
    int86(0x67, &regs, &regs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.bx;
}

static unsigned char xms_available(void)
{
    union REGS regs;
        
    regs.x.ax = 0x4300;
    int86(0x2f, &regs, &regs);
    return regs.h.al;
}

static XMS_HANDLE_TABLE far* get_xmsHanTab(void)
{
    union REGS regs;
    struct SREGS sregs;
    
    regs.x.ax = 0x4309;
    int86x(0x2f, &regs, &regs, &sregs);
    if (regs.h.al == 0x43)
        return MK_FP(sregs.es, regs.x.bx);
    else
        return NULL;
}

static xms_drv_t get_xms_drv(void)
{
    union REGS regs;
    struct SREGS sregs;

    regs.x.ax = 0x4310;
    int86x(0x2f, &regs, &regs, &sregs);
    return MK_FP(sregs.es, regs.x.bx);
}

extern ulong cdecl call_xms_driver_dx_bl_al(unsigned char rah, ushort rdx);
extern ulong cdecl call_xms_driver_bx_ax(unsigned char rah, ushort rdx);
extern ulong cdecl call_xms_driver_edx(unsigned char rah, ushort rdx);
extern ulong cdecl call_xms_driver_eax(unsigned char rah, ushort rdx);
extern int cdecl is_386_(void);
extern ulong cdecl check_e820(struct e820map far *e820map, ulong counter);

static unsigned check_8800(void)
{
    union REGS regs;
        
    regs.x.ax = 0x8800;
    int86(0x15, &regs, &regs);
    if (regs.x.cflag)
        return 0;
    else
        return regs.x.ax;
}

static ulong check_e801(void)
{
    union REGS regs;
        
    regs.x.ax = 0xe801;
    regs.x.bx = 0;
    int86(0x15, &regs, &regs);
    if (regs.x.cflag || regs.x.bx == 0)
        return 0;
    else
        return regs.x.ax * 1024UL + regs.x.bx * 65536UL;
}

#endif

#ifdef __WATCOMC__
/* WATCOM's getenv is case-insensitive which wastes a lot of space
   for our purposes. So here's a simple case-sensitive one */
char *getenv(const char *name)
{
  char **envp, *ep;
  const char *np;
  char ec, nc;

  for (envp = environ; (ep = *envp) != NULL; envp++) {
    np = name;
    do {
      ec = *ep++;
      nc = *np++;
      if (nc == 0) {
        if (ec == '=')
          return ep;
        break;
      }
    } while (ec == nc);
  }
  return NULL;
}
#endif

static char malloc_buffer[30000];
static size_t mbuffer_idx;

/* dumb malloc replacement without possibility to free. that's
   enough for us here */
void *malloc(size_t size)
{
    char *ret = &malloc_buffer[mbuffer_idx];
    mbuffer_idx += size;
    return ret;
}

#define calloc(nitems,size) (malloc((nitems) * (size)))

void free(void *foo)
{
    foo = foo;
}

static void fatal(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    printf("MEM: ");
    vprintf(format, ap);
    va_end(ap);
    exit(1);
}

static void *xmalloc (size_t size)
{
    register void *value = malloc (size);
    if (value == NULL)
        fatal (_(0,0,"Out of memory. Need %ld more bytes.\n"), (long int)size);
    return value;
}

#define xcalloc(nitems,size) (xmalloc((nitems) * (size)))

static unsigned long round_kb(unsigned long bytes)
{
    return ((bytes + 512) / 1024L);
}

static unsigned round_seg_kb(unsigned para)
{
    return ((para + 32) / 64);
}

static void convert(const char *format, ulong num)
{
    int c, i, j, n;
    char des[4*sizeof(ulong)];

    n = sprintf(des, "%lu", num);
    /* insert commas in the string */
    c = 3;
    for (i = n - 3; i > 0; i--) {
        if (c%3==0) {
            for (j = n; j >= i; j--)
                des[j+1] = des[j];
            des[i]=',';
            n++;
        }
        c++;
    }
    printf(format, des);
}

static char* get_os(void)
{
    switch (get_oem_number())
        {
	case 0xFD:
            return "FreeDOS";
        case 0xFF:
	    if (_osmajor <= 6)
                return "MS-DOS";
            else
		return "Windows";
        case 0x00:
            return "PC-DOS";
        case 0xEE:
            return "DR-DOS";
        case 0xEF:
            return "Novell";
        case 0x66:
	    return "PTS-DOS";
        case 0x5E:
            return "RxDOS";
        default:
            return _(1,0,"An unknown operating system");

        }
}

static EMSINFO *ems_error(void)
{
    printf(_(5,0,"EMS INTERNAL ERROR.\n"));
    return NULL;
}

#define ems_frame() call_ems_driver_bx(0x41)
#define ems_size() call_ems_driver_dx(0x42)
#define ems_free() call_ems_driver_bx(0x42)
#define ems_version() call_ems_driver(0x46)
#define ems_num_handles() call_ems_driver_bx(0x4b)

static EMSINFO *check_ems(void)
{
    char far *int67;
    static EMSINFO *ems_static_pointer = NULL;
    static EMSINFO ems_static;
    EMSINFO *ems = ems_static_pointer;
    char *emsname = "EMMXXXX0";
    unsigned len;
    long tmp;
    
    if (ems != NULL)
        return ems;
    
    int67=(char far *)getvect(0x67);
    if (int67 == NULL)
        return ems;

    int67 = MK_FP(FP_SEG(int67),10);
    for (len = 8; len; emsname++,int67++,len--)
	if (*emsname != *int67)
            return ems;
            
    ems = ems_static_pointer = &ems_static;
    /* no frame is not an error -- FD EMM386 has that for noems */
    if ((tmp = ems_frame()) < 0)
        tmp = 0;
    ems->frame = (ushort)tmp;

    if ((tmp = ems_version()) < 0)
        return ems_error();
    ems->vermajor = ((ushort)tmp >> 4) & 0xf;
    ems->verminor = (ushort)tmp & 0xf;

    if ((tmp = ems_size()) < 0)
        return ems_error();
    ems->size = (ushort)tmp;

    if ((tmp = ems_free()) < 0)
        return ems_error();
    ems->free = (ushort)tmp;

    if ((tmp = ems_num_handles()) < 0)
        return ems_error();
    ems->handles=xmalloc((ushort)tmp*sizeof(EMS_HANDLE));

    if ((tmp = ems_get_pages(ems->handles)) < 0)
        return ems_error();
    ems->usehandle=(ushort)tmp;

    if (ems->vermajor >= 4)
        {
        if ((tmp = ems_total_num_handles()) < 0)
            ems->totalhandle=ems->usehandle;
        else
            ems->totalhandle = (unsigned)tmp;
        }
    else
        {
        ems->totalhandle=ems->usehandle;
        }

    ems->freehandle=ems->totalhandle - ems->usehandle;

    return(ems);
}

/* check for 386+ before doing e820 stuff */
/* load value 0x7000 into FLAGS register */
/* then check whether all bits are set */
#define is_386() ((is_386_() & 0x7000) == 0x7000)

#define xms_version() (call_xms_driver_bx_ax(0, 0))
#define xms_hma() ((uchar)(call_xms_driver_dx_bl_al(0, 0)>>16))
#define xms_largest() ((ushort)call_xms_driver_bx_ax(8, 0))
#define xms_totalfree() ((ushort)(call_xms_driver_dx_bl_al(8, 0)>>16))

static ulong xms_exthandlesize(ushort handle)
{
    return call_xms_driver_edx(0x8e, handle);
}

static ulong xms_exttotalfree(void)
{
    return call_xms_driver_edx(0x88, 0);
}

static ulong xms_extlargest(void)
{
    return call_xms_driver_eax(0x88, 0);
}

static XMSINFO *check_xms(void)
{
    struct e820map e820map;
    ulong total;
    static XMSINFO *xms_static_pointer = NULL;
    static XMSINFO xms_static;
    XMSINFO *xms = xms_static_pointer;

    if (xms != NULL) return xms;

    xms = xms_static_pointer = &xms_static;
    total = 0;
    e820map.lenlow = 0;
    xms->is_386 = is_386();
    if (xms->is_386) {
        /* yes: we have a 386! and can use ax=0xe820 for int15 */
        ulong counter = 0;

        do {
            counter = check_e820(&e820map, counter);
            /* has to be system memory above 1 Meg. */
            if (e820map.type == 1 && e820map.baselow >= 1024*1024UL)
                total += e820map.lenlow;
        } while (counter != 0); /* check to see if ebx is set to EOF  */
    }
    if (total == 0)
        total = check_e801();
    if (total == 0) {
        total = check_8800();
	if (total == 0)
	    /* Try the CMOS approach required by Alain from Ralf */
            if (*(uchar far *)MK_FP(0xF000, 0xFFFE) == 0xFC) { /*is_AT*/
                outportb(0x70, 0x17);
                total = inportb(0x71);
                outportb(0x70, 0x18);
                total |= inportb(0x71) << 8;
	    }
        total *= 1024L;
    }

    xms->total=total;
    if (xms_available() != 0x80)
        return xms;

    xms_drv = get_xms_drv();

    total = xms_version();
    xms->verminor=total & 0xff;
    xms->vermajor=(total >> 8) & 0xff;
    xms->drv_vermajor=total >> 24;
    xms->drv_verminor=(total >> 16) & 0xff;
    xms->hma=xms_hma();

    if (xms->is_386 && xms->vermajor >= 3) {
	xms->largest=xms_extlargest();
	xms->free=xms_exttotalfree();
    }
    if (!xms->largest)
        xms->largest=xms_largest();
    if (!xms->free)
        xms->free=xms_totalfree();
    return(xms);
}

static int set_upperlink(uchar link)
{
    long res = dos_set_upperlink(link);
    
    if (res < 0) {
        if ((ushort)res == 1)
            return 0;
        return -1;
    }
    return 1;
}

static UPPERINFO *check_upper(MINFO *mlist)
{
    static UPPERINFO *upper = NULL;
    uchar origlink;
    ushort lastconseg;

    if (upper!=NULL)
        return upper;
    
    origlink=get_upperlink();
    switch (set_upperlink(1))
        {
	case 1:
            set_upperlink(origlink);
            break;

        case 0:
            return upper;
            
        case -1:
            fatal(_(0,1,"SYSTEM MEMORY TRASHED! (int 21.5803 failure)\n"));

	}

    upper = xcalloc(1, sizeof(UPPERINFO));
    lastconseg = biosmemory()*64;
    /* assert(mlist!=NULL) -- comes from make_mcb_list */
    for (;;) {
        unsigned short seg = mlist->seg;
        if (seg == lastconseg)
            break;
        mlist=mlist->next;
        if (mlist==NULL) {
            fatal(_(0,2,"UMB Corruption: Chain doesn't reach top of low RAM at %dk. Last=0x%x.\n"),
                lastconseg/64, seg);
        }
    }

    mlist=mlist->next;
    while (mlist!=NULL) {
        unsigned size = mlist->size + 1;
        if (mlist->type == MT_FREE)
        {
            upper->free += size;
            if (size > upper->largest)
                upper->largest = size;

	}
	if (mlist->type < MT_RESERVED)
	    upper->total += size;
        mlist=mlist->next;
    }
    return(upper);
}

#define is_psp(mcb) (*(ushort far*)MK_FP(mcb+1, 0) == 0x20CD)

static unsigned env_seg(unsigned mcb)
{
    unsigned env = *(ushort far *)MK_FP(mcb+1, 0x2C);
    return (((MCB far *)MK_FP(env-1,0))->owner==mcb+1 ? env : 0);
}

#if 0 /* presently unused */
static void search_vectors(MINFO *m)
{
    ushort i, begin, end, iv;
    uchar far *ivp;
    uchar vectors[256];

    begin=m->seg + 1;
    end=begin + m->size;
    for (i=0;i<256;i++)
        {
        ivp = *(uchar far * far *)MK_FP(0, i*4);
	iv = FP_SEG(ivp) + FP_OFF(ivp) >> 4;
        if ((iv >= begin) && (iv < end))
            vectors[m->vecnum++]=(uchar)i;
	}
    if (m->vecnum != 0)
	{
	m->vectors = xmalloc(m->vecnum);
	memcpy(m->vectors, vectors, m->vecnum);
	}
}
#endif

static void check_name(char *dest, const char far *src, size_t length)
{
    dest[length] = '\0';
    while(length--) {
        unsigned char ch = (unsigned char)*src++;
        *dest++ = (ch == '\0' ? '\0' : ch <= ' ' ? ' ' : ch);
    }
}

static MINFO *search_sd(MINFO *mlist)
{
    ushort begin, end, i;
    SD_HEADER far *sd;
    static struct {char c; char *s;} sdtype[] = 
    {
      { 'E', "DEVICE" },
      { 'F', "FILES" },
      { 'X', "FCBS" },
      { 'C', "BUFFERS" },
      { 'B', "BUFFERS" },
      { 'L', "LASTDRV" },
      { 'S', "STACKS" }
    };

    begin=mlist->seg + 1;
    end=mlist->seg + mlist->size;
    sd=MK_FP(begin, 0);
    while ((FP_SEG(sd) >= begin) && (FP_SEG(sd) < end))
        {
        char type = sd->type;
        mlist->next = xcalloc(1, sizeof(MINFO));
	mlist = mlist->next;
        mlist->owner = mlist->seg = sd->start;
        mlist->size=sd->size;
        mlist->type=MT_DOSDATA;
	if (type == 'I' && *(unsigned short far *)MK_FP(0, 0x40e) == mlist->seg)
            mlist->name = "EBDA";
	else if (type == 'D' || (type == 'I' && _osmajor < 7))
            {
            mlist->name = xmalloc(10);
            check_name(mlist->name, sd->name, 8);
            mlist->type=sd->type == 'D' ? MT_DEVICE : MT_IFS;
            }
        else if (type == 'I')
            {
            /* this is a hack but SECTORBUFs can only have this size
	       and DRIVEDATA areas never */
            mlist->name = mlist->size == 34 ? "SECTORBUF" : "DRIVEDATA";
            }
        else
            {
            mlist->name = " ??????";
            for (i = 0; i< sizeof(sdtype)/sizeof(sdtype[0]); i++)
                {
                if (sdtype[i].c == sd->type)
                    {
                    mlist->name = sdtype[i].s;
                    }
                }
            }
	sd=MK_FP(sd->start + sd->size, 0);
        }
    return(mlist);
}

static MINFO *register_dos_mcb(MINFO *mlist)
{
    if (!mlist->owner) {
        mlist->type=MT_FREE;
    } else if (mlist->owner <= 0x0008) {
        MCB far *mcb = MK_FP(mlist->seg, 0);
        mlist->name = "DOS";
        if (mcb->name[0]=='S' && mcb->name[1]=='D') {
            mlist->type=MT_SYSDATA;
            mlist=search_sd(mlist);
        } else {
	    /* can be either system code or an UMB hole */
	    unsigned seg = mlist->seg + 1;
	    unsigned convmemsize = biosmemory()*64;
	    mlist->type = MT_SYSCODE;
	    /* no  holes in conv mem */
	    if (seg == convmemsize || (seg > convmemsize
	    /* the heuristic: starts at 512 byte boundary, 
	       has size a multiple of 512 (32 paragraphs) */
		&& ((seg & 511) == 0) && ((mlist->size & 31) == 0))) {
		mlist->name = "";
		mlist->type = MT_RESERVED;
	    }
        }
    }
    return mlist;
}

static void program_mcb(MINFO *mlist)
{
    MCB far *mcb;

    mlist->name = xmalloc(9);
    mcb = MK_FP(mlist->seg, 0);
    check_name(mlist->name, mcb->name, 8);
    mlist->environment=env_seg(mlist->seg);
    mlist->type=MT_PROGRAM;
}

static MINFO *register_mcb(MINFO *mlist)
{
    MCB far *mcb = MK_FP(mlist->seg, 0);

    mlist->name="";    
    mlist->owner=mcb->owner;
    mlist->size=mcb->size;
    if (mlist->owner && (is_psp(mlist->seg) || mlist->owner == mlist->seg + 1))
        program_mcb(mlist);
    else
        mlist=register_dos_mcb(mlist);
    return mlist;
}

static MINFO *make_mcb_list(unsigned *convmemfree)
{
    MCB far *cur_mcb;
    uchar origlink;
    MINFO *mlist;
    static MINFO *mlistroot = NULL;
    unsigned freemem, convtopseg;

    if(mlistroot!=NULL)
	return(mlistroot);

    origlink=get_upperlink();
    set_upperlink(1);

    mlistroot = mlist = xcalloc(1, sizeof(MINFO));

    /* In LL in offset -02 there's pointer to first mem block (segment only). */
    cur_mcb=MK_FP(*(ushort far *)(dos_list_of_lists()-2), 0);

    while(cur_mcb->type == 'M')
    {
        mlist->seg = FP_SEG(cur_mcb);
        mlist=register_mcb(mlist);
        cur_mcb = MK_FP(FP_SEG(cur_mcb) + cur_mcb->size + 1, FP_OFF(cur_mcb));
        mlist->next=xcalloc(1, sizeof(MINFO));
        mlist=mlist->next;
    }
    if (cur_mcb->type != 'Z')
        fatal(_(0,3,"The MCB chain is corrupted (no Z MCB after last M MCB, but %c at seg 0x%x).\n"),
            cur_mcb->type, cur_mcb);
    mlist->seg = FP_SEG(cur_mcb);
    register_mcb(mlist);
    set_upperlink(origlink);

    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
        MINFO *mlistj;
        
        if (mlist->type != MT_RESERVED && mlist->next != NULL &&
            mlist->next->type == MT_RESERVED) {
	    /* adjust to make the reserved area clear */
	    mlist->next->seg++;
	}
        if (mlist->type == MT_PROGRAM) {
            for(mlistj=mlistroot;mlistj!=NULL;mlistj=mlistj->next) {
                if ((mlist->seg != mlistj->seg)
                    && (mlistj->owner == mlist->seg+1)) {
                    mlistj->name = mlist->name;
                    mlistj->type = MT_ENV;
                    if (mlist->environment != mlistj->seg+1)
                        mlistj->type = MT_DATA;
                }
            }
        }

#if 0 /* presently unused */
        if (mlist->type != MT_SYSDATA)
            search_vectors(mlist);
#endif

    }

    convtopseg = biosmemory()*64;
    freemem = 0;

    /* get free memory */
    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
        if ((mlist->type == MT_FREE || mlist->seg + 1 == _psp)
            && mlist->seg < convtopseg)
            freemem += mlist->size + 1;
    }
    
    *convmemfree = freemem;
    return(mlistroot);
}

/*
    return largest possible segment size
*/

static unsigned mcb_largest(void)
{
    MINFO *mlist;
    unsigned largest = 0;

    for (mlist=make_mcb_list(NULL); mlist!=NULL ;mlist = mlist->next)
        if (mlist->type == MT_FREE || mlist->seg + 1 == _psp) {
            unsigned size = mlist->size;
            if (mlist->type != MT_FREE && mlist->next != NULL &&
		mlist->next->type == MT_FREE)
                size += mlist->next->size + 1; /* adjustment for MEM himself */
            if (size > largest) {
                largest = size;
            }
        }
    return largest;
}    

static DEVINFO *make_dev_list(MINFO *mlist)
{
    DEVICE_HEADER far *cur_dev;
    DPB far*cur_dpb;
    DEVINFO *dlistroot, *dlist;
    MINFO *mlistroot = mlist;
    
    dlist = dlistroot = xcalloc(1, sizeof(DEVINFO));

    cur_dev = (DEVICE_HEADER far *)(dos_list_of_lists() + 0x22);
    
    while (FP_OFF(cur_dev) != 0xFFFF) {
	dlist->addr=cur_dev;
        dlist->attr=cur_dev->attr;
        dlist->progname="";
        check_name(dlist->devname, cur_dev->name, 8);
        cur_dev=cur_dev->next;
        if (FP_OFF(cur_dev) != 0xFFFF) {
            dlist->next=xcalloc(1, sizeof(DEVINFO));
            dlist=dlist->next;
        }
    }

    for (dlist=dlistroot;dlist!=NULL;dlist=dlist->next)
        for (mlist=mlistroot;mlist!=NULL;mlist=mlist->next)
            if (mlist->seg == FP_SEG(dlist->addr))
                dlist->progname = mlist->name;

    for  (cur_dpb = *((DPB far *far*)dos_list_of_lists());
          FP_OFF(cur_dpb) != 0xFFFF; cur_dpb=cur_dpb->next_dpb)
	{
        for (dlist=dlistroot;dlist!=NULL && dlist->addr != cur_dpb->device_addr;
                 dlist=dlist->next)
	    ;
        
        if (dlist!=NULL)
	{
            uchar drive_num = cur_dpb->drive_num+'A';
	    if (dlist->firstdrive==0)
                dlist->firstdrive=drive_num;
            else
                dlist->lastdrive=drive_num;
            }
        }

    for (dlist=dlistroot;dlist!=NULL;dlist=dlist->next)
        {
        if ((dlist->attr & 0x8000) == 0)
            dlist->devname[0]='\0';

        if (dlist->firstdrive != 0)
            {
            sprintf(dlist->devname, "%c:", dlist->firstdrive);
            if (dlist->lastdrive != 0)
                sprintf(&dlist->devname[2], " - %c:", dlist->lastdrive);
            }
        }

    return dlistroot;
}

#define dos_in_hma() (dos_in_hma_() & 0x10)

static void print_normalized_ems_size(unsigned n)
{
    if (n > 624) /* 9984 is the highest "K" value */
	convert("%4sM ", (n + 32) / 64);
    else
	convert("%4sK ", n * 16);
    convert(_(1,2,"(%s bytes)\n"), n * 16384L);
}

static void print_normal_entry(char *text, unsigned long total, 
			       unsigned long used, unsigned long free)
{
    printf("%-17s", text);
    convert("%8sK ", total);
    convert("%9sK ", used);
    convert("%9sK\n", free);
}

static void normal_list(unsigned memfree, UPPERINFO *upper)
{
    unsigned memory, memused, largest_executable, reserved;
    unsigned umbfree = 0, umbtotal = 0;
    unsigned long xms_total_k;
    XMSINFO *xms;
    EMSINFO *ems;

    ems=check_ems();
    xms=check_xms();
    memory=biosmemory();
    memfree=round_seg_kb(memfree);
    memused=memory - memfree;
    printf("\n");
    printf(_(2,0,"Memory Type        Total       Used       Free\n"));
    printf(      "----------------  --------   --------   --------\n");
    print_normal_entry(_(2,1,"Conventional"), memory, memused, memfree);
    if (upper) {
	umbfree=round_seg_kb(upper->free);
	umbtotal=round_seg_kb(upper->total);
    }
    print_normal_entry(_(2,2,"Upper"), umbtotal, umbtotal-umbfree, umbfree);
    reserved = 1024 - memory - umbtotal;
    print_normal_entry(_(2,3,"Reserved"), reserved, reserved, 0);
    xms_total_k = round_kb(xms->total);
    print_normal_entry(_(2,4,"Extended (XMS)"), xms_total_k, xms_total_k - xms->free,
		       xms->free);
    printf("----------------  --------   --------   --------\n");
    print_normal_entry(_(2,5,"Total memory"), 1024 + xms_total_k,
		       1024 - memfree - umbfree + xms_total_k - xms->free,
		       memfree + umbfree + xms->free);
    printf("\n");
    print_normal_entry(_(2,6,"Total under 1 MB"), 1024 - reserved,
	   memused + umbtotal - umbfree, memfree + umbfree);
    printf("\n");
    if (ems != NULL) {
	printf("%-36s",_(2,7,"Total Expanded (EMS)"));
	print_normalized_ems_size(ems->size);
	printf("%-36s",_(2,8,"Free Expanded (EMS)"));
	print_normalized_ems_size(ems->free);
        printf("\n");
    }

    largest_executable = mcb_largest();

    printf("%-38s%3uK", _(2,9,"Largest executable program size"), round_seg_kb(largest_executable));
    convert(_(1,3," (%7s bytes)\n"), (ulong)largest_executable*16);
    if (upper != NULL) {
	printf("%-38s%3uK", _(2,10,"Largest free upper memory block"), round_seg_kb(upper->largest));
	convert(_(1,3," (%7s bytes)\n"), (ulong)upper->largest*16);
    }

    if (dos_in_hma())
        printf(_(2,11,"%s is resident in the high memory area.\n"), get_os());
}    

static void print_entry(MINFO *entry)
{
    static char *typenames[]= { "", "free", "system code", "system data",
				"program", "environment", "data area",
				"reserved", "device driver",
				"data area", "IFS" };
    char *space = "";
    if (entry->type >= MT_DEVICE)
        space = " ";
    printf("  %04X%9lu   %s%-12s%-13s\n",
           entry->seg, (ulong)entry->size*16, space, entry->name, 
	   _(3,entry->type,typenames[entry->type]));
}

static void print_classify_value(const char *format, unsigned n)
{
    char kbuf[8];
    convert(format, n*16UL);
    sprintf(kbuf, "(%uK)", round_seg_kb(n));
    printf("%8s", kbuf);
}

static void print_classify_entry(char *name, unsigned total_conv, unsigned total_umb)
{
    printf("  %-9s", name);
    print_classify_value("%9s", total_conv + total_umb);
    print_classify_value("%11s", total_conv);
    print_classify_value("%11s", total_umb);
    printf("\n");
}

static void classify_list(unsigned convmemfree, unsigned umbmemfree)
{
    MINFO *ml, *ml2;
    unsigned total_conv, total_umb;
    unsigned convtopseg = biosmemory()*64;

    printf(_(4,0,"\nModules using memory below 1 MB:\n\n"));
    printf(_(4,1,
	   "  Name           Total           Conventional       Upper Memory\n"));
    printf("  --------  ----------------   ----------------   ----------------\n");
    /* figure out code used by "SYSTEM" */
    ml = make_mcb_list(NULL);
    total_conv = ml->seg; total_umb = 0;
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next) {
        if (ml->owner == _psp || ml->type == MT_DOSDATA 
	    || ml->type == MT_RESERVED)
            ml->classified = 1;
        else if (ml->type == MT_SYSCODE || ml->type == MT_SYSDATA
		 || ml->type >= MT_DEVICE) {
            int size = ml->size + 1;
	    if (ml->type == MT_DEVICE)
		size = -(size - 1);
	    else
		ml->classified = 1;
            if (ml->seg < convtopseg)
                total_conv += size;
            else
                total_umb += size;
        }
    }
    print_classify_entry(_(4,2,"SYSTEM"), total_conv, total_umb);
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next)
        if (ml->type > MT_FREE && !ml->classified) {
            total_conv = total_umb = 0;
            for (ml2 = ml; ml2 != NULL; ml2 = ml2->next) {
                if (!ml2->classified && ml2->type > MT_FREE &&
                    ml2->owner == ml->owner) {
                    ml2->classified = 1;
                    if (ml2->seg < convtopseg)
                        total_conv += ml2->size + 1;
                    else
                        total_umb += ml2->size + 1;
                }
            }
            print_classify_entry(ml->name, total_conv, total_umb);
        }

    print_classify_entry(_(4,3,"Free"), convmemfree, umbmemfree);
}

static void upper_list(void)
{
    MINFO *ml;

    printf(_(4,4,"\nSegment   Size       Name         Type\n"));
    printf(        "------- --------  ----------  -------------\n");
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next)
        if (ml->type != MT_NONE && (ml->type < MT_ENV || ml->type == MT_DEVICE))
		print_entry(ml);
}

static void full_list(void)
{
    MINFO *ml;
    
    printf(_(4,4,"\nSegment   Size       Name         Type\n"));
    printf(        "------- --------  ----------  -------------\n");
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next)
        print_entry(ml);
}

static void device_list(void)
{
    DEVINFO *dl;

    printf(_(4,5,"\n   Address     Attr    Name       Program\n"));
    printf(        " -----------  ------ ----------  ----------\n");
             /*  XXXX:XXXX    XXXX   XXXXXXXX    XXXXXXXX */
    for (dl=make_dev_list(make_mcb_list(NULL));dl!=NULL;dl=dl->next)
	printf("  %p    %04X   %-8s    %-8s\n", dl->addr, dl->attr,
                dl->devname, dl->progname);
}

static void ems_list(void)
{
    EMSINFO *ems;
    ushort i;
    static char handlename_other[9];
    char *handlename, *handlename_sys;
    static char format[] = "  %-20s";
    
    ems=check_ems();
    if (ems==NULL)
    {
        printf(_(5,1,"  EMS driver not installed in system.\n"));
    }
    else
    {
	printf(format, _(5,2,"\nEMS driver version"));
	printf("%1u.%1u\n", ems->vermajor, ems->verminor);
	if (ems->frame) {
	    printf(format, _(5,3,"EMS page frame"));
	    printf("%04X\n", ems->frame);
	}
	printf(format, _(5,4,"Total EMS memory"));
	printf(_(1,1,"%lu bytes\n"), ems->size * 16384L);
	printf(format, _(5,5,"Free EMS memory"));
	printf(_(1,1,"%lu bytes\n"), ems->free * 16384L);
	printf(format, _(5,6,"Total handles"));
	printf("%u\n", ems->totalhandle);
	printf(format, _(5,7,"Free handles"));
	printf("%u\n", ems->freehandle);

	printf(_(5,8,"\n  Handle   Pages    Size       Name\n"));
	printf(        " -------- ------  --------   ----------\n");
	handlename_sys = _(4,2,"SYSTEM");
        for (i=0;i<ems->usehandle;i++)
        {
            handlename = handlename_sys;
            if (ems->vermajor >= 4)
            {
                if (ems->handles[i].handle != 0)
                {
                    handlename = handlename_other;
                    memset(handlename, 0, 9);
                    ems_get_handle_name(ems->handles[i].handle, handlename);
		    check_name(handlename, handlename, 8);
                }
            }

            printf("%9u%7u%10lu%11s\n", ems->handles[i].handle,
                   ems->handles[i].pages, (ulong)ems->handles[i].pages * 16384L, handlename);
        }
    }
}

static long xms_common(unsigned char al, signed char bl, ushort result)
{
     return (al != 0 ? result : (long)bl << 24);
}

static long xms_handleinfo(ushort handle)
{
     /* Get handle information */
     ulong bx_ax = call_xms_driver_bx_ax(0xe, handle);
     return xms_common((uchar)bx_ax, (uchar)(bx_ax >> 16),
                       (ushort)(bx_ax >> 16));
}

/* Get handle size information */

#define xms_handlesize(handle) ((ushort)(call_xms_driver_dx_bl_al(0xe, handle) >> 16))

static long xms_alloc(ushort kbytes)
{
     /* "Allocate extended memory block" */
     ulong dx_bl_al = call_xms_driver_dx_bl_al(0x9, kbytes);
     return xms_common((uchar)dx_bl_al,
                       (uchar)(dx_bl_al>>8), (ushort)(dx_bl_al>>16));
}

static void xms_free(ushort handle)
{
     /* "Free extended memory block" */
     call_xms_driver_dx_bl_al(0xa, handle);
}

static long xms_query_a20(void)
{
    ulong bx_ax = call_xms_driver_bx_ax(0x7, 0);
    return (((uchar)(bx_ax>>16))!=0 ? ((long)bx_ax<<8) : (uchar)(bx_ax));
}

static void xms_list(void)
{
    UPPERINFO *upper;
    XMSINFO *xms;
    XMS_HANDLE *handle = NULL;
    ushort i;
    long lhandle;
    static char format[] = "%-26s";
    XMS_HANDLE_TABLE far* xmsHanTab;

    xms = check_xms();

    if (xms_drv==NULL)
	{
	printf(_(6,0,"XMS driver not installed in system.\n"));
	return;
	}

    printf(_(6,1,"\nTesting XMS memory ...\n"));

    lhandle = xms_query_a20();
    if (lhandle < 0) {
	printf(_(6,2,"XMS INTERNAL ERROR.\n"));
	return;
    }
    xms->a20 = lhandle & 0xff;

    /*
    // 01/4/27 tom + alain
    //
    // although the 'old' method to search the handle table should be OK,
    // it crashes somehow and for unknown reason under Win98. So, a 'new' method to
    // query all handles was implemented, using INT 2F, AX=4309
       test support for INT2F AX=4309 first */
    xmsHanTab = get_xmsHanTab();
    if (xmsHanTab != NULL    && /* test returned OK */
	xmsHanTab->sizeOfDesc == sizeof(XMS_HANDLE_DESCRIPTOR)) /* assert correct size */
    {
	XMS_HANDLE_DESCRIPTOR far* descr = xmsHanTab->xmsHandleDescr;

	printf(_(6,3,"INT 2F AX=4309 supported\n"));

        for (i=0;i<xmsHanTab->numbOfHandles;i++,descr++)
        {
            if (descr->flag != 0x01 && /* not free */
                descr->xmsBlkSize != 0)   /* and takes memory */
            {
                if (handle==NULL)
                    xms->handles=handle=xmalloc(sizeof(XMS_HANDLE));
                else {
                    handle->next=xmalloc(sizeof(XMS_HANDLE));
                    handle=handle->next;
                }
                handle->handle=FP_OFF(descr);
                handle->size=descr->xmsBlkSize*1024L;
                handle->locks=descr->locks;
                handle->next=NULL;
            }
        }
    }
    else
    {
        /* old method */
        /* query all handles 0..0xffff */

        for (i=0;i<65535u;i++)
        {
            /* Get handle information */
            if ((lhandle = xms_handleinfo(i)) >= 0) {
                uchar free_handles_tmp = lhandle & 0xff;
                if (handle==NULL)
                    xms->handles=handle=xcalloc(1, sizeof(XMS_HANDLE));
                else {
                    handle->next=xcalloc(1, sizeof(XMS_HANDLE));
                    handle=handle->next;
                }
                handle->handle=i;
                if (xms->vermajor >= 3 && xms->is_386)
                    handle->size=xms_exthandlesize(i);
                if (!handle->size)
                    handle->size=xms_handlesize(i);
                handle->size *= 1024L;
                handle->locks=lhandle >> 8;
                handle->next=NULL;
                if (xms->freehandle < free_handles_tmp)
                {
                    xms->freehandle = free_handles_tmp;
                }
            }
        }
    }

    /* First try to get a handle of our own. */
    /* First we try 1kB. I'm not sure if XMS driver
       must support a zero sized allocate. */
    /* "Allocate extended memory block" */
    if (((lhandle = xms_alloc(1)) >= 0) ||
	/* Now try a zero sized allocate just in case there was no free memory. */
	((lhandle = xms_alloc(0)) >= 0))
    {
        long info = xms_handleinfo((ushort)lhandle);
	/* else nothing worked out. Use whatever we got from the loop above. */
	/* We can't do much if the free call fails, so it ends here. */
	if (info >= 0)
	    /* Hey! We got some info. Put it in a safe place. */
	    xms->freehandle = (uchar)info + 1;
	/* Add one for the handle we have allocated. */
	xms_free((ushort)lhandle);
    }

    printf(format, _(6,4,"XMS version"));
    printf("%u.%02u \t\t", xms->vermajor, xms->verminor);
    printf(format, _(6,5,"XMS driver version"));
    printf("%u.%02u\n", xms->drv_vermajor, xms->drv_verminor);
    printf(format, _(6,6,"HMA state"));
    printf("%s \t", (xms->hma) ? _(6,7,"exists") : _(6,8,"does not exist"));
    printf(format, _(6,9,"A20 line state"));
    printf("%s\n", (xms->a20) ? _(6,10,"enabled") : _(6,11,"disabled"));
    printf(format, _(6,12,"Free XMS memory"));
    printf(_(1,1,"%lu bytes\n"), xms->free*1024L);
    printf(format, _(6,13,"Largest free XMS block"));
    printf(_(1,1,"%lu bytes\n"), xms->largest*1024L);
    printf(format, _(6,14,"Free handles"));
    printf("%u\n", xms->freehandle);
    printf("\n");
    if (xms->handles != NULL)
    {
	printf(_(6,15," Block   Handle     Size     Locks\n"));
	printf(       "------- --------  --------  -------\n");
	for (i=0, handle=xms->handles;handle!=NULL;handle=handle->next, i++)
	    printf("%7u %8u  %8lu  %7u\n", i, handle->handle,
		   handle->size, handle->locks);

    }

    upper = check_upper(make_mcb_list(NULL));
    if (upper != NULL)
    {
	printf(format, _(6,16,"Free upper memory"));
	printf(_(1,1,"%lu bytes\n"), upper->free*16L);
	printf(format, _(6,17,"Largest upper block"));
	printf(_(1,1,"%lu bytes\n"), upper->largest*16L);
    }
    else
    {
	printf(_(6,18,"Upper memory not available\n"));
    }

}

/* function to obtain the number of lines on the screen...added by brian reifsnyder.  */
static uchar get_font_info(void)
{
    uchar number_of_lines = *((uchar far *)MK_FP(0x40, 0x84));
    if (number_of_lines == 0)
        number_of_lines = 25;
    else
        number_of_lines++;
    return number_of_lines;
}

enum {F_HELP = 1, F_DEVICE = 2, F_EMS = 4, F_FULL = 8, F_UPPER = 16,
      F_XMS = 32, F_PAGE = 64, F_CLASSIFY = 128 };

int main(int argc, char *argv[])
{
    uchar flags = 0;
    UPPERINFO *upper;
    unsigned memfree;
    int i = 1, j;
    char *argvi = argv[i];
    static struct {char c; uchar flag;} optype[] = 
    {
      { '?', F_HELP },
      { 'C', F_CLASSIFY },
      { 'D', F_DEVICE },
      { 'E', F_EMS },
      { 'F', F_FULL },
      { 'P', F_PAGE },
      { 'U', F_UPPER },
      { 'X', F_XMS }
    };

    argc = argc;

    kittenopen("mem");

    while (argvi != NULL && (flags & F_HELP)==0) {
        char ch = *argvi++;
        if (ch == '-' || ch == '/')
            ch = toupper(*argvi);
        else
            ch = '/';
        for (j = 0; j < sizeof(optype)/sizeof(optype[0]); j++) {
            if (ch == optype[j].c) {
                flags |= optype[j].flag;
                break;
            }
        }
        if (j == sizeof(optype)/sizeof(optype[0]))
            fatal(_(0,4,"unknown option: %s\nUse /? for help\n"), argvi);

        if (*++argvi == '\0')
        {
            i++;
            argvi = argv[i];
        }
    }

    upper=check_upper(make_mcb_list(&memfree));

    if (flags & F_PAGE)   num_lines=get_font_info();
    if (flags & F_DEVICE) device_list();
    if (flags & F_EMS)    ems_list();
    if (flags & F_FULL)   full_list();
    if (flags & F_UPPER)  upper_list();
    if (flags & F_XMS)    xms_list();
    if (flags & F_CLASSIFY)    classify_list(memfree, upper ? upper->free : 0);
        
    if (flags & F_HELP)
      {
	printf(_(7, 0, "FreeDOS MEM version %d.%d\n"),
	       MEM_MAJOR, MEM_MINOR);
	printf("%s\n\n%s\n  %s\n  %s\n  %s\n  %s\n  %s\n  %s\n  %s\n  %s\n",
	_(7, 1, "Displays the amount of used and free memory in your system."),
	_(7, 2, "Syntax: MEM [/E] [/F] [/C] [/D] [/U] [/X] [/P] [/?]"),
	_(7, 3, "/E  Reports all information about Expanded Memory"),
	_(7, 4, "/F  Full list of memory blocks"),
	_(7, 5, "/C  Classify modules using memory below 1 MB"),
	_(7, 6, "/D  List of device drivers currently in memory"),
	_(7, 7, "/U  List of programs in conventional and upper memory"),
	_(7, 8, "/X  Reports all information about Extended Memory"),
	_(7, 9, "/P  Pauses after each screenful of information"),
	_(7, 10, "/?  Displays this help message"));
	return 1;
      }

    normal_list(memfree, upper);
    return 0;
}

/*
        TE - some size optimizations for __TURBOC__
    
        as printf() is redefined in PRF.C to use no stream functions,
        rather calls DOS directly, these Stream operations are nowhere used,
        but happen to be in the executable.

        so we define some dummy functions here to save some precious bytes :-)

        this is in no way necessary, but saves us some 1500 bytes
*/

#ifdef __TURBOC__

#define UNREFERENCED_PARAMETER(x) if (x);

int _Cdecl flushall(void){return 0;}

int _Cdecl fprintf(FILE *__stream, const char *__format, ...)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER ( __format);    return 0;}
int _Cdecl fseek(FILE *__stream, long __offset, int __whence)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER (__offset);
 UNREFERENCED_PARAMETER ( __whence);
 return 0;}

int _Cdecl setvbuf(FILE *__stream, char *__buf, int __type, size_t __size)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER ( __buf);
 UNREFERENCED_PARAMETER ( __type);
 UNREFERENCED_PARAMETER ( __size);   return 0;}

void _Cdecl _xfflush (void){}
void _Cdecl _setupio (void){}

#endif

