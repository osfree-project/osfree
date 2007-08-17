//
// loader.h
// Interface between different parts of loader
//

#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#undef  INCL_PM
#include <os21x/os2.h>

#include <add.h>
#include <i86.h>
#include <string.h>

#include <os2/types.h>
#include <os2/magic.h>

#define BOOTFLAG_NOVOLIO   0x0100
#define BOOTFLAG_RIPL      0x0200
#define BOOTFLAG_MINIFSD   0x0400
#define BOOTFLAG_RESERVED3 0x0800
#define BOOTFLAG_MICROFSD  0x1000
#define BOOTFLAG_RESERVED5 0x2000
#define BOOTFLAG_RESERVED6 0x4000
#define BOOTFLAG_RESERVED7 0x8000

struct FileTableExt {
    // struct FileTable
    unsigned short ft_cfiles;
    unsigned short ft_ldrseg;
    unsigned long  ft_ldrlen;
    unsigned short ft_museg;
    unsigned long  ft_mulen;
    unsigned short ft_mfsseg;
    unsigned long  ft_mfslen;
    unsigned short ft_ripseg;
    unsigned long  ft_riplen;
    // microfsd functions
    unsigned short __cdecl (far *ft_muOpen)(char far *pName, unsigned long far *pulFileSize);
    unsigned long  __cdecl (far *ft_muRead)(long loffseek, char far *pBuf, unsigned long cbBuf);
    void           __cdecl (far *ft_muClose)(void);
    void           __cdecl (far *ft_muTerminate)(void);

    void __cdecl (far *ft_muMount) (void);

    // stage0 functions and variables
    BIOSPARAMETERBLOCK far *ft_pbpb;
    unsigned short ft_stage0_seg;
    unsigned short ft_stage0_stack_top;
    unsigned short ft_stage0_stack;
    dev_t          ft_bios_device;
    int   __cdecl (far *ft_printk) (const char *fmt, ...);
    void  __cdecl (far *ft_microfsd_panic) (const char *format, ...);
    char * __cdecl (far *ft_DecoupePath) (char *path, char *component);
    int __pascal (far *ft_bios_read_hd) (
                              unsigned short dev,
                              unsigned short head,
                              unsigned short cyl,
                              unsigned short sect,
                              unsigned short nbsect,
                              unsigned long  buffer
                );

    // microxfd functions
};


extern void _pascal video_output(char *msg, unsigned short msglen);
extern void _pascal video_crlf(void);
extern void _pascal video_init(void);

int sprintf(char * buf, const char *fmt, ...);


/*
 * Standard micro-fsd entry points
 */
unsigned short __cdecl mu_Open (char far *pName, unsigned long far *pulFileSize);
unsigned long  __cdecl mu_Read (long loffseek, char far *pBuf, unsigned long cbBuf);
void __cdecl mu_Close (void);
void __cdecl mu_Terminate (void);
void __cdecl mu_Mount (void);

void __cdecl microfsd_panic (const char *format, ...);

/*
 * pathutil.c
 */
char * __cdecl DecoupePath (char *path, char *component);

int __cdecl printk (const char *fmt, ...);

/*
 * From entry.asm
 */
int __pascal bios_read_hd (
                        unsigned short dev,
                        unsigned short head,
                        unsigned short cyl,
                        unsigned short sect,
                        unsigned short nbsect,
                        unsigned long buffer
                       );

/*
 * Current microfsd segment register
 */
extern unsigned short current_seg;
extern dev_t bios_device;

void __cdecl stage0_Init(void);
void __cdecl mu_Init(struct FileTableExt far * filetab, unsigned short far *dh);
