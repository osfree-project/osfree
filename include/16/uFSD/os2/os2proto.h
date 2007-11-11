//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/os2proto.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
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



#ifndef __OS2PROTO_H
#define __OS2PROTO_H

#include <os2/types.h>
#ifndef MINIFSD
#include <os2/fsd32.h>
#endif
#include <linux/fs.h>
#ifndef MINIFSD
#include <os2/request_list.h>
#endif
#include <os2/minifsd.h>
#include <stdarg.h>

//
// From isfat.c
//
int isfat(char *component);

//
// From pathutil.c
//
pchar DecoupePath(pchar path, pchar component);
void ExtractPath(pchar Name, pchar Path);
void ExtractName(pchar Name, pchar Nom);


//
// From log.c
//
int kernel_printf(const char *fmt, ...);
#define printk kernel_printf


//
// From util.c
//
unsigned long updcrc(unsigned char *s, unsigned n);

//
// From fs_find.c
//
extern void date_unix2dos(long unix_date,unsigned short *time, unsigned short *date);
extern long date_dos2unix(unsigned short time, unsigned short date);

//
// From attr.c
//
void DOS_To_Linux_Attrs(struct inode *inode, unsigned short DOS_attrs);
unsigned short Linux_To_DOS_Attrs(struct inode *inode, char *component);

//
// From volume.c
//
#ifndef MINIFSD
extern int FS32CALLBACK inherit_minifsd_data(PTR16 mfs_data);
#endif

//
// from maperr.c
//
extern int map_err(int err);


#ifdef MINIFSD
//
// Memory allocation routines for mini FSD
//
extern char *stage1_G_malloc(unsigned long cbSize);
extern int   stage1_G_free(char *ptr);

extern char *stage2_G_malloc(unsigned long cbSize);
extern int   stage2_G_free(char *ptr);

extern char *(*G_malloc)(unsigned long);
extern int   (*G_free)(char *);

#endif /* #ifdef MINIFSD */

//
// From panic.c
//
extern void ext2_os2_panic(int sync, const char *fmt, ...);




//
// From vsprintf.c
//
int sprintf(char * buf, const char *fmt, ...);

//
// From vfs/f_table.c
//
void invalidate_files(struct super_block *sb, int iput_allowed);

//
// From case.c
//
int is_case_retensive(void);

//
// From vfs/strat2.asm
//
#ifndef MINIFSD
extern unsigned long stub_request_list_completed;
extern unsigned long stub_request_completed;
extern unsigned long stub_pager_RLH_completed;
extern unsigned long stub_pager_RLE_completed;
extern void _System  send_RLH_to_driver(PTR16 pRLH, PTR16 strat2);
#endif
//
// From vfs/pageio.c
//
#ifndef MINIFSD
void do_pageio(struct PageCmdHeader *pPageCmdHeader, struct file *f);
#endif

//
// From vfs/reqlist.c
//
#ifndef MINIFSD
extern int reqlist_init(int nr_seg);
extern void put_reqlist(struct reqlist *s);
extern struct reqlist *get_reqlist(void);
#endif

//
// From vfs/super.c
//
extern int put_super(struct super_block *sb);
extern struct super_block *get_empty_super(void);
extern void invalidate_supers(void);

//
// From fsd32/fs32_mount.c
//
#ifndef MINIFSD
extern struct super_block * do_mount(
                                     struct vpfsi32          *pvpfsi,
                                     union  vpfsd32          *pvpfsd, 
                                     unsigned short           hVPB, 
			             struct file_system_type *type
                                    );
extern int do_remount_sb(struct super_block *sb, int flags, char *data);
extern int do_unmount(struct super_block *sb);
#endif

#ifdef MINIFSD
//
// From devhelp.c and devhelp2.asm
//
int FS_DevHelp_VMAlloc(unsigned long taille, unsigned long PhysAddr, unsigned long Flags, PUINT32 LinAddr);
int FS_DevHelp_VMFree(unsigned long LinAddr);
int FS_DevHelp_GetDOSVar(unsigned char index, char _FS_PTR _FS_PTR value);
extern int ProcRun(unsigned long event_id);
extern int ProcBlock(unsigned long event_id, long timeout, unsigned short flag);
extern int Beep(int frequency, int duration);
extern int VirtToPhys(ULONG virt, PULONG phys);
extern int VirtToLin(void *virt, unsigned long *plin);
int VMLock(
           unsigned long  flags,
           unsigned long  lin,            // FLAT pointer
           unsigned long  length,
           unsigned long  pPageList,      // FLAT pointer
           unsigned long  pLockHandle,    // FLAT pointer
           unsigned long *pPageListCount
          );
int VMUnlock(
             unsigned long plock_lin      // FLAT pointer
            );

#define LOCK_WRITE 1
#define LOCK_READ  0
int LockUserBuffer(void *buf, unsigned long size, void *lock, int write, unsigned long *plock_lin);
int LockBuffer(void *buf, unsigned long size, void *lock, int write, unsigned long *plock_lin);
#endif

/*
 * files.c
 */
#ifndef MINIFSD
struct file  *open_by_name(struct inode *base, char *pathname, unsigned long openmode);
#else
struct file  *_open_by_name(struct super_block *sb, char *pName, unsigned long openmode);
#endif
struct file  *_open_by_inode(struct super_block * p_volume, UINT32 ino_no, UINT32 openmode);
int vfs_close(struct file *f);

#endif
