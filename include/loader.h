/*
 * loader.inc
 * Interface between different parts of loader
 */

#ifndef __LOADER_H__
#define __LOADER_H__

#include <types.h>

// boot flags
#define BOOTFLAG_NOVOLIO   0x0100
#define BOOTFLAG_RIPL      0x0200
#define BOOTFLAG_MINIFSD   0x0400
#define BOOTFLAG_NOPICINIT 0x0800
#define BOOTFLAG_MICROFSD  0x1000
#define BOOTFLAG_RESERVED5 0x2000
#define BOOTFLAG_RESERVED6 0x4000
#define BOOTFLAG_RESERVED7 0x8000

typedef _Packed struct _FileTable {

    // FileTable struc
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
    fp_t  ft_muOpen;
    fp_t  ft_muRead;
    fp_t  ft_muClose;
    fp_t  ft_muTerminate;

    // protected mode RIPL info
    unsigned long ft_resofs;
    unsigned long ft_reslen;

} FileTable;

#endif
