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

    MEM 1.6, April 2004 (CVS revision 1.6)
    * minor output tweaks, don't upcase names anymore
    * try to detect UMB holes and don't count them as upper memory
    * display UMB holes as "reserved" in mem/f output
    * display version for "mem/?"


    MEM 1.7 beta, August 2004, Bart Oldeman (CVS revision 1.10)
    * kittenize (support localization)
    * Fix "UMB corruption" problem. As it turned out FreeCOM put the
      environment just below the top of conv mem where it used to be
      placed in an UMB.	 That's not very nice in terms of memory
      consumption, but MEM should work in either case.
    * 64L -> 64 (optimization)
    * avoid NULL derefs.
    * Improved diagnostics (Eric Auer) Adjusted help.
    * Correct NULL check for loop that check the last conv mem seg.
    * update mem to 1.7beta and fix a bug when a free MCB looks like a
      PSP.

    Unreleased, September 2004, Bart Oldeman (CVS revision 1.11)
    * Replace getenv() by our own (~1700 bytes uncompressed).

    MEM 1.8 (alpha 1), 29 August 2005, David O'Shea (doshea revision 26)
    * make malloc() cause a fatal error if its buffer is full (this change
      reverted later)
    * include Interrupt Vector Table, BIOS Data Area and kernel areas for
      which no MCBs exist in the output of the new /DEBUG command
    * MINFO structure contains pointers to first and last DEVINFO
      structures that describe devices in the memory block and each
      DEVINFO contains a back-pointer to the MINFO of the memory
      block it lies in
    * provide details for STACKS, FILES, BUFFERS and LASTDRIVE blocks,
      although the STACKS details will not work with FreeDOS
    * show memory available via interrupt 15
    * add /ALL command which shows how much HMA is free, HMAMIN, etc.;
      HMAMIN is probed via a binary search of allocation/free attempts;
      without /ALL we still show whether MS-DOS is managing the HMA
    * add new /MODULE and /FREE commands which use new functions to take the
      full MINFO list and return a new filtered copy and re-implement /U
      this way
    * support long command-line option names such as /CLASSIFY in place
      of /C with shortest unique prefix matching
    * add /H[ELP] command as a synonym for /?
    * add /NOSUMMARY command to skip default output

    MEM 1.9a2 (alpha 2), 18 November 2005, David O'Shea (doshea revision 43)
    * add support for MUSCHI
    * restore support for Turbo C compiler by adding get_ext_mem_size()
      to MEMSUPT.ASM
    * replace minfo_typenames[] array and accesses to it with a function
      containing a switch statement so we never have to pass a variable
      argument to the _() macro and hence so that MUSCHI works
    * classify_list() used to treat ml->seg as the amount of "SYSTEM" memory
      in pages before the first MINFO, but now there are actually a number of
      MINFO entries for all the reserved memory blocks, e.g. MT_IVT and
      MT_BDA, which are explicitly counted in classify_system()
    * correct malloc() to return NULL instead of trigering a fatal error in
      case of its buffer being full; allocators that want to have a fatal
      error triggered in case of insufficient memory can (and do) call
      xmalloc()
    * kitten-ize strings added in 1.8 alpha 1
    * a MEM.EXE compiled with -DDEBUG will no longer unconditionally
      print all debugging information, instead providing command-line
      options for the different debug flags (which start with /DBG);
      note that you can't debug the command-line parser code this way
      because you obviously need to have completed at least part of
      the parse to know that the option to debug it was specified, but
      you could add such a command-line option which, after parsing,
      causes flags to be reset to 0 and the parsing code to be
      re-invoked with debugging enabled; for now use #define
      DEBUG_PARSER
    * change method of generation of the help output so that new flags
      cannot be accidentally left undocumented in this output: for each
      entry in opts[] that corresponds to a unique flag, we either
      display the help string for the flag as returned by
      help_for_flag() or we show the name of the parameter with a note
      that no help text is available
    * version string in help output includes compile date/time, compiler
      information and an indication of whether DEBUG was defined
    * provide generic_split_list function to split MINFO list into two
      parts (conventional and upper) which is used to show some of the
      output split into these parts
    * in case of MEM /MODULE <name> being specified and <name> not
      existing in memory, we now return errorlevel 2 instead of 0;
      note that all fatal errors result in errorlevel 1
    * replace opt_flag_t enumeration with a set of #defines and a
      'typedef unsigned long' so we can force it to be long and
      #define masks/sets of flags
    * get_opts(): add documentation; allow one or more colons to be
      used in place of spaces between arguments; allow e.g.
      /MODULE<name> or /M<name> with no space/colon in between
    * provide /OLD option which causes /D to map to /DEVICE instead of
      /DEBUG and /F to map to /FULL instead of /FREE, i.e. causes /D
      and /F to take on their MEM 1.7 beta meanings instead of MS-DOS
      meanings
    * provide /SUMMARY option which always overrides /NOSUMMARY
    * avoid producing no output when /NOSUMMARY specified by itself
      or with /P

    MEM 1.9a3 (alpha 3), 15 April 2006, David O'Shea
    * kittenize "FILES=%u (%u in this block)" (4.12)
    * support "SYSTEM" as argument to "/MODULE" which will include
      everything we'd show under "/MODULE IO" and "/MODULE DOS"
    * remove comment on MT_DEVICE that is outdated due to the use of a
      tree data structure
    * update MUSCHI compile method to avoid #including a .c file
    * update changelog entries for MEM 1.7b onwards
    * prevent /DEBUG and /FREE from showing empty tables when there is no
      or no free upper memory respectively, instead they will explicitly
      indicate that the specified type of memory is not installed/free;
      kitten message 1.6 updated and 1.9 added

    MEM 1.10, 24/26 August 2006, Eric Auer
    * applied a patch from Joris van Rantwijk to fix the 386 detection:
      the old version failed to recognize 8086 as being pre-386
    * changed the version number display to just use 1 string, and
      updated the mem.* message files accordingly
    * fixed the build.bat to make building on Watcom C easier
    * fixed the memory summary XMS part, made handling more consistent
    * if XMS total (from BIOS, if nothing found, from XMS) is smaller
      XMS free, assume that XMS total is equal to XMS free. Avoids
      negative values for XMS used in DOSEMU :-)
    * changed linebreaks to CRLF (downloading files with a Linux cvs
      client made linebreaks Linux style in version 1.8 or 1.9)
    * fixed the get_ext_mem_size error checking (failed to detect
      the carry flag), thanks to Joris for that one...

    MEM 1.11, 27 August 2006, Eric Auer
    * no longer suppress the EMS infos in NOEMS case
    * split mem.c into 2 parts to allow editing with DOS editors
    * convert() pretty-printer now also used for EMS/XMS/UMB free/largest
      and for int15h info (but not yet for ems_list() yet)
    * convert now uses the DOS NLS API to use the local 1000s sep char
    * converted spaces to tabs in the sources, but not inside strings

    MEM 1.12, 01 March 2022, Robert Riebisch
    * rebuilt using Open Watcom C/C++ version 1.9 to fix reporting 0K
      conventional memory free on some machines, e.g., pce-ibmpc with
      Turbo-XT BIOS
*/

#define MEM_VERSION "1.12"

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
#define biosmemory _bios_memsize /* returns kilobytes */
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

#define OEM_FREEDOS 0xFD

/*
 * Size of a page in conventional memory and in EMS.
 */
#define CONV_BYTES_PER_PAGE	16UL
#define EMS_BYTES_PER_PAGE	16UL*1024UL

#define CONV_PARA_PER_KB	64UL /* was "bytes/seg" */

#define BYTES_PER_KB		1024UL


/* Types for MINFO.type */
#define MT_NONE	   0
#define MT_FREE	   1
#define MT_SYSCODE 2
#define MT_SYSDATA 3
#define MT_PROGRAM 4
#define MT_ENV	   5
#define MT_DATA	   6
#define MT_RESERVED 7 /* check_upper() doesn't include entries with a type
		       * of MT_RESERVED or higher in total size */
#define MT_IVT	   8 /* inferred - Interrupt Vector Table */
#define MT_BDA	   9 /* inferred - BIOS Data Area */
#define MT_KERNEL  10 /* inferred - area between BIOS and first MCB */
#define MT_DEVICE  11
#define MT_DOSDATA 12
#define MT_IFS	   13

#ifdef MUSCHI
#include "mem_nls.h"
#define _(set,message_number,message) kittengets(set,message_number,muschi_ ## set ## _ ## message_number)
#else
#define _(set,message_number,message) kittengets(set,message_number,message)
#endif

int num_lines = -1;

typedef struct device_header
{
    struct device_header far*next;
    ushort attr;
    ushort strategy_rutin;
    ushort interrupt_rutin;
    char name[8];
} DEVICE_HEADER;

/* forward declarations as these structures link between each other */
struct devinfo;
struct minfo;

/* is a character device, as opposed to a block device */
#define DEVICE_ATTR_IS_CHAR 0x8000

typedef struct devinfo
{
    struct device_header far *addr;
    char devname[9];
    struct minfo *minfo;
    ushort attr;
    uchar firstdrive, lastdrive;
    struct devinfo *next;
    struct devinfo *next_in_minfo;
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

typedef struct
{
    uchar dummy1[22]; /* fields we're not interested in */
    ushort parent_pid;
    uchar file_handles[20];
    ushort env_seg;
    ulong dummy2;
    ushort dummy3;
    ulong dummy4;
    ulong previous_psp; /* default FFFFFFFFh in DOS 3.x */
    uchar dummy5[68];
    uchar cmd_tail_len;
    uchar cmd_tail[127];
} PSP;


#define MCB_NAME_MAX_LEN 8

#define OWNER_FREE	0x0000
#define OWNER_DOS	0x0008

typedef void (*print_minfo_field)(struct minfo *entry);

/*
 * MINFO elements in the list returned by make_mcb_list do not overlap.
 * For a particular MINFO 'm', the linked list pointed to by 'first_child'
 * contains entries that all overlap with m but not with each other.
 */
typedef struct minfo
{
    uchar type;
    ushort seg;
    ushort owner;
    ushort environment;
    char *name;
    ushort size;
    uchar classified; /* flag set to TRUE when entry processed for /C */
#if 0 /* presently unused */
    uchar vecnum;
    uchar *vectors;
#endif
    struct minfo *next;
    struct minfo *first_child;
#if 0 /* presently unused */
    struct minfo *caller;
#endif
    struct devinfo *first_dev;
    struct devinfo *last_dev;
    print_minfo_field print_name;
    print_minfo_field print_type;
    void *specific;
} MINFO;

/*
 * Format of data at start of STACKS code segment (if present)
 * Comments are from Table 01634 in Ralf Brown's Interrupt List 60
 */
typedef struct
{
    ushort unknown;		/* ??? */
    ushort count;		/* number of stacks (the x in STACKS=x,y) */
    ushort array_size;		/* size of stack control block array (should
				 * be 8*x */
    ushort stack_size;		/* size of each stack (the y in STACKS=x,y) */
    void far *dseg;		/* pointer to STACKS data segment */
				/* offset in STACKS data segment of ... */
    ushort array_offset;	/* ... stack control block array */
    ushort last_offset;		/* ... last element of that array */
    ushort free_offset;		/* ... the entry in that array for the next
				 * stack to be allocated */
} STACKS;

/*
 * Format of data at start of FILES segment
 */
typedef struct files
{
    struct files far *next;
    ushort count;
} FILES;

/*
 * Value of FILES= in config.sys.  We add to this each time we see a FILES
 * sub-block.
 */
ushort files_total_count = 5;

/*
 * Information pointed to by MINFO.specific
 */
typedef struct
{
    ushort count;
    ushort stack_size;
} STACKSINFO;

typedef struct
{
    ushort count;
    ushort secondary_count;
} BUFFERSINFO;

typedef struct
{
    ushort count;
} FILESINFO;

typedef struct
{
    ushort count;
} LASTDRIVEINFO;

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

/*
 * The last segment address that is in conventional memory.
 */
unsigned int last_conv_seg;

void setup_globals(void)
{
    last_conv_seg = biosmemory() * CONV_PARA_PER_KB;
}

/*
 * What xms_available() returns if XMS is available.
 */
#define XMS_AVAILABLE_RESULT 0x80

/*
 * Return values from assembler function get_ext_mem_size().
 */
#define GET_EXT_MEM_SIZE_OK(result)	((result >> 16) == 0)
#define GET_EXT_MEM_SIZE_VALUE(result)	(result)
#define GET_EXT_MEM_SIZE_ERROR(result)	((result & 0xFF00) >> 8)

#define GET_EXT_MEM_SIZE_ERROR_INVALID_CMD	0x80
#define GET_EXT_MEM_SIZE_ERROR_UNSUPPORTED	0x86


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
    "and ax,7fffh" \
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
     "mov eax, 0x0000e820"	  /* e820, upper word zeroed	*/ \
     "mov edx, 0x534d4150"	  /* ascii 'SMAP'		*/ \
     "mov ecx, 20"		  /* size of the e820map	*/ \
     "int 15h"			  /* make the call		*/ \
     "jc no386"			  /* fall to e801 if it fails	*/ \
     "cmp eax, 0x534d4150"	  /* check the return is `SMAP` */ \
     "je yes386"		  /* fall to e801 if it fails	*/ \
"no386:" \
     "xor ebx, ebx" \
"yes386:" \
     "mov ax, bx"  \
     "shr ebx, 16" \
     ".8086" \
parm [es di] [bx ax] value [bx ax] modify [si];

/*
 * Get the extended memory size.  Ralf Brown's Interrupt List notes that
 * some BIOSes don't properly set the carry flag on failure.  If this
 * happens, we'll incorrectly show memory as available on systems that
 * do not support extended memory.
 *
 * If (result >> 16) != 0, then result & 0xFF00 = ah = error code,
 * else result = number of continuous KB starting at absolute address 100000h.
 */
ulong get_ext_mem_size(void);
#pragma aux get_ext_mem_size =			\
    "mov ah, 0x88"				\
    "int 15h"					\
    "mov dx, 0"					\
    "jnc success"				\
    "mov dx, 1"					\
"success:"					\
value [dx ax];

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
extern ulong cdecl get_ext_mem_size(void);

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

#define HMA_FREE_NOT_DOS 0x0000 /* DOS is not in HMA */
#define HMA_FREE_UNKNOWN 0xFFFF /* DOS doesn't support querying free HMA */

static unsigned int dos_hma_free(void)
{
    union REGS regs;

    regs.x.ax = 0x4A01;
    regs.x.bx = HMA_FREE_UNKNOWN;
    int86(0x2F, &regs, &regs);
    return regs.x.bx;
}

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

#ifdef DEBUG
/*
 * Debug flags set on the command-line.
 */
int dbgdevaddr;
int dbghmamin;
int dbgcpu;
#endif

#define MALLOC_BUFFER_SIZE 30000
static char malloc_buffer[MALLOC_BUFFER_SIZE];
static size_t mbuffer_idx;

/* dumb malloc replacement without possibility to free. that's
   enough for us here */
void *malloc(size_t size)
{
    char *ret = &malloc_buffer[mbuffer_idx];

    if (mbuffer_idx + size > MALLOC_BUFFER_SIZE) {
	return NULL;
    }
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

static char *xstrdup (char *src)
{
    char *dst = xmalloc(strlen(src) + 1);
    strcpy(dst, src);
    return dst;
}

static unsigned long round_kb(unsigned long bytes)
{
    return ((bytes + 512) / 1024UL);
}

static unsigned round_seg_kb(unsigned para)
{
    return ((para + 32) / 64);
}

/*
 * As for printf(), but format may only contain a single format specifier,
 * which must be "%s" and is replaced with the string form of num with commas
 * separating groups of three digits.
 *
 * e.g. convert("%s bytes", 1234567) -> "1,234,567 bytes"
 */
static void convert(const char *format, ulong num)
{
    int c, i, j, n;
    char des[4*sizeof(ulong)+3];
    union REGS regs;
    struct SREGS sregs;
    char mycountry[48]; /* probably 34 bytes are enough... */
    char ksep = ',';	/* or . */

    regs.x.ax = 0x3800;
    sregs.ds = FP_SEG(&mycountry);
    regs.x.dx = FP_OFF(&mycountry);
    intdosx(&regs,&regs,&sregs);
    if (regs.x.cflag == 0) {
      ksep = mycountry[7];	  /* 1000's separator  */
      /* dsep = mycountry[9];	  ** decimal separator */
    }

    n = sprintf(des, "%lu", num);
    /* insert commas in the string */
    c = 3;
    for (i = n - 3; i > 0; i--)	{
	if (c%3==0) {
	    for (j = n; j >= i; j--)
		des[j+1] = des[j];
	    des[i]=ksep;	/* ',' */
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
	case OEM_FREEDOS:
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

/*
 * Returns TRUE if the far string and near string which are both of length
 * max (not NUL-terminated) are equal, FALSE otherwise.
 */
int fnstreqn(const char far *s1, const char *s2, size_t max)
{
    for (; max > 0; max--) {
	if (*s1 != *s2)	{
	    return FALSE;
	}
	s1++;
	s2++;
    }
    return TRUE;
}

static EMSINFO *check_ems(void)
{
    char far *int67;
    static EMSINFO *ems_static_pointer = NULL;
    static EMSINFO ems_static;
    EMSINFO *ems = ems_static_pointer;
    long tmp;

    if (ems != NULL)
	return ems;

    int67=(char far *)getvect(0x67);
    if (int67 == NULL)
	return ems; /* NULL */

    int67 = MK_FP(FP_SEG(int67),10);

    /*
     * The driver name is "EMMQXXX0" if EMM386 /NOEMS was used.	 This
     * is true for both MS-DOS and FreeDOS EMM386.
     * Update 8/2006: NOEMS means no EMS 3.2 page frame, but EMS is
     * still available!
     */
    if (!fnstreqn(int67, "EMMXXXX0", 8) && !fnstreqn(int67, "EMMQXXX0", 8))
	return ems; /* NULL */

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
#define is_386() ((is_386_() & 0xf000) == 0x7000)

#define xms_version() (call_xms_driver_bx_ax(0, 0))
#define xms_hma() ((uchar)(call_xms_driver_dx_bl_al(0, 0)>>16))
#define xms_largest() ((ushort)call_xms_driver_bx_ax(8, 0))
#define xms_totalfree() ((ushort)(call_xms_driver_dx_bl_al(8, 0)>>16))

#define XMS_HMA_SIZE ((64 * 1024) - 16)
#define XMS_HMA_AX(result) (result & 0xFFFF)
#define XMS_HMA_BL(result) ((result >> 16) & 0xFF)
#define XMS_HMA_AX_OK		0x0001
#define XMS_HMA_AX_FAILED	0x0001
#define XMS_HMA_BL_NOT_IMPL	0x80
#define XMS_HMA_BL_VDISK	0x81
#define XMS_HMA_BL_NOT_EXIST	0x90
#define XMS_HMA_BL_IN_USE	0x91
#define XMS_HMA_BL_HMAMIN	0x92
#define XMS_HMA_BL_NOT_IN_USE	0x93

#define xms_hma_request(amount) (call_xms_driver_bx_ax(0x01, amount))
#define xms_hma_release() (call_xms_driver_bx_ax(0x02, 0))

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
#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: is_386\n");
    }
#endif
    xms->is_386 = is_386();
#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: 386: %s\n", xms->is_386 ? "yes" : "no");
    }
#endif
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

#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: total=%lu, check_e801\n", total);
    }
#endif
    if (total == 0)
	total = check_e801();
#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: total=%lu, check_8800\n", total);
    }
#endif
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
	total *= 1024UL;
    }

    xms->total=total;
#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: xms_available\n");
    }
#endif
    if (xms_available() != XMS_AVAILABLE_RESULT)
	return xms;

#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: get_xms_drv\n");
    }
#endif
    xms_drv = get_xms_drv();

#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_xms: xms_version\n");
    }
#endif
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
    /* new 8/2006 */
    xms->free = 1024UL * xms->free; /* was in kbytes, convert to bytes */
    xms->largest = 1024UL * xms->largest; /* was in kbytes, convert to bytes */
    /* dosemu workaround: total (from CMOS) can be < free (from XMS) there */
    if (xms->total < xms->free)	{
#ifdef DEBUG
	if (dbgcpu) {
	    printf("check_xms: more XMS free than XMS/int15 total: %lu > %lu\n", xms->free, xms->total);
	}
#endif
	xms->total = xms->free;
    }
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
    /* assert(mlist!=NULL) -- comes from make_mcb_list */
    for (;;) {
	unsigned short seg = mlist->seg;
	if (seg == last_conv_seg)
	    break;
	mlist=mlist->next;
	if (mlist==NULL) {
	    fatal(_(0,2,"UMB Corruption: Chain doesn't reach top of low RAM at %dk. Last=0x%x.\n"),
		last_conv_seg/64, seg);
	}
    }

    mlist=mlist->next;
    while (mlist!=NULL)	{
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

static void print_minfo_name_default(MINFO *entry)
{
    printf("%-8s", entry->name);
}

char *minfo_typename(int type)
{
    switch (type) {
    case MT_NONE:	return (_(3,0,""));
    case MT_FREE:	return (_(3,1,"free"));
    case MT_SYSCODE:	return (_(3,2,"system code"));
    case MT_SYSDATA:	return (_(3,3,"system data"));
    case MT_PROGRAM:	return (_(3,4,"program"));
    case MT_ENV:	return (_(3,5,"environment"));
    case MT_DATA:	return (_(3,6,"data area"));
    case MT_RESERVED:	return (_(3,7,"reserved"));
    case MT_IVT:	return (_(3,8,"interrupt vector table"));
    case MT_BDA:	return (_(3,9,"BIOS data area"));
    case MT_KERNEL:	return (_(3,10,"system data"));
    case MT_DEVICE:	return (_(3,11,"device driver"));
    case MT_DOSDATA:	return (_(3,12,"data area"));
    case MT_IFS:	return (_(3,13,"IFS"));
    default:		return (_(3,14,"(error)"));
    }
}

static void print_minfo_type_default(MINFO *entry)
{
    printf("%s", minfo_typename(entry->type));
}

static void print_minfo_type_stacks(MINFO *entry)
{
    STACKSINFO *specific = (STACKSINFO *) entry->specific;

    printf("STACKS=%u,%u", specific->count, specific->stack_size);
}

static void print_minfo_type_buffers(MINFO *entry)
{
    BUFFERSINFO *specific = (BUFFERSINFO *) entry->specific;

    printf("BUFFERS=%u,%u", specific->count, specific->secondary_count);
}

static void print_minfo_type_files(MINFO *entry)
{
    FILESINFO *specific = (FILESINFO *) entry->specific;

    printf("FILES=%u", files_total_count);
    printf(_(4, 12, " (%u in this block)"), specific->count);
}

static void print_minfo_type_lastdrive(MINFO *entry)
{
    LASTDRIVEINFO *specific = (LASTDRIVEINFO *) entry->specific;
    char c;

    c = specific->count + 'A' - 1;
    if (c > 'Z') {
	c = 'Z';
    } else if (c < 'A')	{
	c = '?';
    }
    printf("LASTDRIVE=%c", c);
}

static MINFO *new_minfo(void)
{
    MINFO *entry = xcalloc(1, sizeof(MINFO));

    entry->name = "";
    entry->print_name = print_minfo_name_default;
    entry->print_type = print_minfo_type_default;
    return (entry);
}

/*
 * Obviously MCBs that are actually free should be shown as such, but we also
 * show our own MCB as free because the user isn't too interested in how much
 * memory MEM takes.  Note that MEM /DEBUG or MEM /FULL should still show
 * MEM in their output, but MEM /FREE should show MEM's block as free and
 * the plain MEM output should include MEM's own MCB in the free memory total.
 */
int show_minfo_as_free(MINFO *entry)
{
    return (entry->type == MT_FREE || (entry->seg + 1 == _psp));
}

int filter_free(MINFO *entry, void *data)
{
    return (show_minfo_as_free(entry));
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

static void sd_stacks(MINFO *mlist)
{
    STACKS far *stacks;
    STACKSINFO *specific;

    mlist->name = "STACKS";

    /*
     * FreeDOS doesn't set up the beginning of the STACKS code segment in
     * the way that MS-DOS does.
     */
/*
 * FIXME: for FreeDOS, need to show STACKS in type column instead of
 * name since we do that for MS-DOS where we show STACKS=x,y but for FreeDOS
 * we don't know what x and y are.
 */
    if (get_oem_number() != OEM_FREEDOS) {
	stacks = MK_FP(mlist->seg, 0);
	specific = xmalloc(sizeof(STACKSINFO));
	specific->count = stacks->count;
	specific->stack_size = stacks->stack_size;
	mlist->specific = specific;
	mlist->print_type = print_minfo_type_stacks;
    }
}

static void sd_buffers(MINFO *mlist)
{
    BUFFERSINFO *specific;
    char far *list_of_lists = dos_list_of_lists();

    mlist->name = "BUFFERS";
    specific = xmalloc(sizeof(BUFFERSINFO));
    /*
     * These values are only in the list of lists for MS-DOS version 4.x
     * or higher.  We don't need to check for that version though as earlier
     * versions don't have subsegment descriptors either.
     */
    specific->count = *(ushort far *) (list_of_lists + 0x3F);
    specific->secondary_count = *(ushort far *) (list_of_lists + 0x41);
    mlist->specific = specific;
    mlist->print_type = print_minfo_type_buffers;
}

static void sd_files(MINFO *mlist)
{
    FILES far *files;
    FILESINFO *specific;

    specific = xmalloc(sizeof(FILESINFO));
    mlist->name = "FILES";
    files = MK_FP(mlist->seg, 0);
    specific->count = files->count;
    files_total_count += files->count;
    mlist->specific = specific;
    mlist->print_type = print_minfo_type_files;
}

static void sd_lastdrive(MINFO *mlist)
{
    LASTDRIVEINFO *specific;

    specific = xmalloc(sizeof(LASTDRIVEINFO));
    mlist->name = "LASTDRV";
/*
 * FIXME: offset into list of lists is different (1Bh) for DOS 3.0 and
 * doesn't exist in previous versions.
 */
    specific->count = * (uchar far *) (dos_list_of_lists() + 0x21);
    mlist->specific = specific;
    mlist->print_type = print_minfo_type_lastdrive;
}

static MINFO *search_sd(MINFO *parent)
{
    MINFO *first_child, *mlist;

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

    begin=parent->seg + 1;
    end=parent->seg + parent->size;
    sd=MK_FP(begin, 0);
    mlist = NULL;
    while ((FP_SEG(sd) >= begin) && (FP_SEG(sd) < end))
	{
	char type = sd->type;
	if (mlist == NULL) {
	    mlist = first_child = new_minfo();
	} else {
	    mlist->next = new_minfo();
	    mlist = mlist->next;
	}
	mlist->owner = mlist->seg = sd->start;
	mlist->size=sd->size;
	mlist->type=MT_DOSDATA;
	if (type == 'I'
	    && *(unsigned short far *)MK_FP(0, 0x40e) == mlist->seg)
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
	else if (type == 'S')
	    sd_stacks(mlist);
	else if (type == 'C' || type == 'B')
	    sd_buffers(mlist);
	else if (type == 'F')
	    sd_files(mlist);
	else if (type == 'L')
	    sd_lastdrive(mlist);
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
    return(first_child);
}

static void register_dos_mcb(MINFO *mlist)
{
    if (mlist->owner == OWNER_FREE) {
	mlist->type=MT_FREE;
    } else if (mlist->owner <= OWNER_DOS) {
	MCB far *mcb = MK_FP(mlist->seg, 0);
	mlist->name = "DOS";
	if (mcb->name[0]=='S' && mcb->name[1]=='D') {
	    mlist->type=MT_SYSDATA;
	    mlist->first_child=search_sd(mlist);
	} else {
	    /* can be either system code or an UMB hole */
	    unsigned seg = mlist->seg + 1;
	    mlist->type = MT_SYSCODE;
	    /* no  holes in conv mem */
	    if (seg == last_conv_seg || (seg > last_conv_seg
	    /* the heuristic: starts at 512 byte boundary,
	       has size a multiple of 512 (32 paragraphs) */
		&& ((seg & 511) == 0) && ((mlist->size & 31) == 0))) {
		mlist->name = "";
		mlist->type = MT_RESERVED;
	    }
	}
    }
}

static void program_mcb(MINFO *mlist)
{
    MCB far *mcb;

    mlist->name = xmalloc(MCB_NAME_MAX_LEN + 1);
    mcb = MK_FP(mlist->seg, 0);
    check_name(mlist->name, mcb->name, MCB_NAME_MAX_LEN);
    mlist->environment=env_seg(mlist->seg);
    mlist->type=MT_PROGRAM;
}

static void register_mcb(MINFO *mlist)
{
    MCB far *mcb = MK_FP(mlist->seg, 0);

    mlist->owner=mcb->owner;
    mlist->size=mcb->size;

    /*
     * The call to is_psp() below is redundant as owner should always
     * be equal to seg + 1.
     */
    if (mlist->owner && (is_psp(mlist->seg) || mlist->owner == mlist->seg + 1))
	program_mcb(mlist);
    else
	register_dos_mcb(mlist);
}

static MINFO *make_mcb_list(unsigned *convmemfree)
{
    MCB far *cur_mcb;
    uchar origlink;
    MINFO *mlist;
    static MINFO *mlistroot = NULL;
    unsigned freemem;
    ushort mlist_pid, find_pid;
    PSP far *psp;

    if(mlistroot!=NULL)
	return(mlistroot);

    origlink=get_upperlink();
    set_upperlink(1);

    mlistroot = mlist = new_minfo();

    /*
     * Set up BIOS memory blocks which are always fixed in size and location.
     */
    mlist->seg = FP_SEG(0);
    /* num. interrupts * sizeof(far ptr) / para */
    mlist->size = 256 * 4 / CONV_BYTES_PER_PAGE;
    mlist->type = MT_IVT;
    mlist->next = new_minfo();
    mlist->next->seg = mlist->size;
    mlist = mlist->next;

    mlist->size = 0x30;
    mlist->type = MT_BDA;
    mlist->next = new_minfo();
    mlist->next->seg = mlist->seg + mlist->size;
    mlist = mlist->next;

    mlist->name = "IO";
    /*
     * Get the segment address of the first MCB (which will be for
     * DOS) from the list of lists.  The memory between the end of the
     * BIOS Data Area and the start of the first DOS MCB is the
     * kernel.
     */
    mlist->size = *(ushort far *)(dos_list_of_lists()-2) - mlist->seg;
    mlist->type = MT_KERNEL;
    mlist->next = new_minfo();
    mlist = mlist->next;


    /* In LL in offset -02 there's pointer to first mem block (segment only). */
    cur_mcb=MK_FP(*(ushort far *)(dos_list_of_lists()-2), 0);

    while(cur_mcb->type == 'M')
    {
	mlist->seg = FP_SEG(cur_mcb);
	register_mcb(mlist);
	cur_mcb = MK_FP(FP_SEG(cur_mcb) + cur_mcb->size + 1, FP_OFF(cur_mcb));
	mlist->next=new_minfo();
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
	    mlist_pid = mlist->seg+1;
	    psp = MK_FP(mlist_pid, 0);
	    if (psp->parent_pid != 0 && psp->parent_pid != mlist_pid) {
		/* this program has a parent program we should search for */
		find_pid = psp->parent_pid;
	    } else {
		find_pid = 0;
	    }

	    for(mlistj=mlistroot;mlistj!=NULL;mlistj=mlistj->next) {
		if (mlist->seg != mlistj->seg) {
		    /* did mlist allocate mlistj? */
		    if (mlistj->owner == mlist_pid) {
			mlistj->name = mlist->name;
			mlistj->type = MT_ENV;
			if (mlist->environment != mlistj->seg+1)
			    mlistj->type = MT_DATA;
		    }
#if 0 /* process list functionality not complete yet */
		    /* did mlistj execute mlist? */
		    if (mlistj->seg + 1 == find_pid) {
			mlist->caller = mlistj;
		    }
#endif
		}
	    }
	}

#if 0 /* presently unused */
	if (mlist->type != MT_SYSDATA)
	    search_vectors(mlist);
#endif

    }

    freemem = 0;

    /* get free memory */
    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
	if (show_minfo_as_free(mlist) && mlist->seg < last_conv_seg)
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
	    if (size > largest)	{
		largest = size;
	    }
	}
    return largest;
}

int addr_in_block(void far *addr, MINFO *minfo)
{
    ulong base = (ulong) minfo->seg * CONV_BYTES_PER_PAGE;
    ulong limit = base + ((ulong) minfo->size) * CONV_BYTES_PER_PAGE;
    ulong normal_addr = (ulong) FP_SEG(addr) * CONV_BYTES_PER_PAGE
			+ (ulong) FP_OFF(addr);

#ifdef DEBUG
    if (dbgdevaddr) {
	printf("is %05lX in range %05lX(%04X)-%05lX (program %s)\n",
	       normal_addr, base, minfo->seg, limit, minfo->name);
    }
#endif
    return (normal_addr >= base && normal_addr <= limit);
}

/*
 * Set the MINFO that a device is inside of.
 */
static void set_dev_minfo(DEVINFO *dlist, MINFO *mlist)
{
    if (dlist->minfo != NULL) {
	printf(_(1,4,
"Warning: device appears to be owned by multiple memory blocks (%s\n"
"and %s)\n"),
	       mlist->name, dlist->minfo->name);
	dlist->next_in_minfo = NULL;
    }
    dlist->minfo = mlist;
    /*
     * Maintain per-MCB list of devices.
     */
    dlist->next_in_minfo = NULL;
    if (mlist->first_dev == NULL) {
	mlist->first_dev = dlist;
	mlist->last_dev = dlist;
    } else {
	mlist->last_dev->next_in_minfo = dlist;
	mlist->last_dev = dlist;
    }
}

static DEVINFO *make_dev_list(MINFO *mlist)
{
    DEVICE_HEADER far *cur_dev;
    DPB far*cur_dpb;
    DEVINFO *dlistroot, *dlist;
    MINFO *mlistroot = mlist;
    MINFO *mchild;
    int found_in_child;

    dlist = dlistroot = xcalloc(1, sizeof(DEVINFO));

    cur_dev = (DEVICE_HEADER far *)(dos_list_of_lists() + 0x22);

    while (FP_OFF(cur_dev) != 0xFFFF) {
	dlist->addr=cur_dev;
	dlist->attr=cur_dev->attr;
	dlist->minfo=NULL;
	dlist->next_in_minfo=NULL;
	check_name(dlist->devname, cur_dev->name, 8);
	cur_dev=cur_dev->next;
	if (FP_OFF(cur_dev) != 0xFFFF) {
	    dlist->next=xcalloc(1, sizeof(DEVINFO));
	    dlist=dlist->next;
	}
    }

    /*
     * For each device, scan mlist for the memory block containing the
     * device so we can get the program name.  For mlist entries with
     * children, we scan the children first, because if one of the children
     * matches, the parent will also match, but we are more interested in
     * which child matches.
     */
    for (dlist=dlistroot;dlist!=NULL;dlist=dlist->next)
	for (mlist=mlistroot;mlist!=NULL;mlist=mlist->next) {
	    found_in_child = FALSE;
	    for (mchild=mlist->first_child;mchild!=NULL;mchild=mchild->next) {
		if (addr_in_block(dlist->addr, mchild))	{
		    found_in_child = TRUE;
		    set_dev_minfo(dlist, mchild);
		}
	    }
	    if (!found_in_child && addr_in_block(dlist->addr, mlist)) {
		set_dev_minfo(dlist, mlist);
	    }
	}

    for	 (cur_dpb = *((DPB far *far*)dos_list_of_lists());
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
	if ((dlist->attr & DEVICE_ATTR_IS_CHAR) == 0)
	    {
	    if (dlist->firstdrive != 0)
		{
		sprintf(dlist->devname, "%c:", dlist->firstdrive);
		if (dlist->lastdrive != 0)
		    sprintf(&dlist->devname[2], " - %c:", dlist->lastdrive);
		}
	    else
		{
		strcpy(dlist->devname, _(1,5,"(no drv)"));
		}
	    }
	}

    return dlistroot;
}

MINFO *minfo_dup(MINFO *minfo)
{
    MINFO *new_minfo;

    new_minfo = xmalloc(sizeof(MINFO));
    memcpy(new_minfo, minfo, sizeof(MINFO));
    return new_minfo;
}

typedef enum {
    memory_conv,
    memory_upper,
    memory_num_types
} memory_t;

char *memory_typename(memory_t type)
{
    switch (type) {
    case memory_conv:	return (_(9,0,"Conventional"));
    case memory_upper:	return (_(9,1,"Upper"));
    default:		return (_(9,2,"(error)"));
    }
}


/*
 * Given an MINFO list, split it into two separate MINFO lists, one for blocks
 * in conventional memory and one for blocks in upper memory.  Returns a
 * pointer to an array of two MINFOs, indexed by elements of memory_t, i.e.
 * memory_conv and memory_upper.
 */
MINFO **split_mlist_conv_upper(MINFO *mlisthead)
{
    MINFO **head;
    MINFO *tail[memory_num_types];
    memory_t memory_type;

    head = xcalloc(memory_num_types, sizeof(MINFO *));

    while (mlisthead != NULL) {
	/*
	 * Work out if this block is in conventional or upper memory.
	 */
	if (mlisthead->seg < last_conv_seg) {
	    memory_type = memory_conv;
	} else {
	    memory_type = memory_upper;
	}

	/*
	 * Duplicate the block and put it on the appropriate list.
	 */
	if (head[memory_type] != NULL) {
	    tail[memory_type]->next = minfo_dup(mlisthead);
	    tail[memory_type] = tail[memory_type]->next;
	} else {
	    head[memory_type] = tail[memory_type] = minfo_dup(mlisthead);
	}

	mlisthead = mlisthead->next;
    }

    /*
     * Make sure both lists are terminated properly and don't contain links
     * from the duplicated nodes.
     */
    for (memory_type = memory_conv; memory_type < memory_num_types;
	 memory_type++)	{
	if (head[memory_type] != NULL) {
	    tail[memory_type]->next = NULL;
	}
    }

    return head;
}

/*
 * Flags to be ORed together and passed as 'flags' to filter_mlist().
 */
#define FILTER_MLIST_NO_FLAGS	0x0000

/*
 * If this flag is set, then if an MINFO entry matches the filter, all of the
 * entries below it will be included in the results, otherwise only entries
 * under it that match the filter will be included.
 */
#define FILTER_MLIST_EXPANDED	0x0001

/*
 * If this flag is set, then even if an MINFO entry doesn't match the
 * filter, if one or more of its children does match, then the parent
 * and the matching children are included in the results.
 */
#define FILTER_MLIST_SEARCH_CHILDREN 0x0002

#include "mem2.c"	/* continue with part 2 */
