//
// $Header: /cur/cvsroot/boot/include/muFSD/linux/sched.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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




#ifndef __LINUX_SCHED_H
#define __LINUX_SCHED_H

#ifndef OS2
#error "sched.h is OS/2 specific for the moment ..."
#endif

#ifndef MINIFSD
#include <builtin.h>
#endif

#include <os2/types.h>
#ifndef MINIFSD
#include <os2/devhlp32.h>
#endif
#include <os2/os2misc.h>
#include <linux/resource.h>
#include <linux/wait.h>

#ifndef __infoseg_h
#define __infoseg_h
#include <infoseg.h>
#endif

//
// Dummy task_struct structure - This is to implement current->rlim[] in various VFS routines
//
struct task_struct {
    struct rlimit rlim[RLIM_NLIMITS];
};

extern struct task_struct current[];
extern long timezone;

//
// This is the current time in the OS/2 global infoseg (time in seconds since 1970-1-1)
// You MUST #include <infoseg.h> before this file
//
#if defined(MINIFSD)
#define CURRENT_TIME 0
#else
#define CURRENT_TIME (pSysInfoSeg->SIS_BigTime + timezone)
#endif


#ifndef MINIFSD
extern void __down(struct semaphore * sem);
#define wake_up(__wait__) DevHlp32_ProcRun((unsigned long)(__wait__))
// extern void wake_up(void *wait);
INLINE void sleep_on(void *wait) {
    _disable();
    DevHlp32_ProcBlock((unsigned long)wait, -1, 1);
}
#else
extern void (*wake_up)();
extern void (*sleep_on)();
#endif

#if !defined(MICROFSD) && !defined(MINIFSD)
INLINE void __down(struct semaphore * sem)
{
    _disable();
    while (sem->count <= 0) {
        DevHlp32_ProcBlock((unsigned long)(&(sem->wait)), -1, 1);
        _disable();
    }
    _enable();
}

INLINE void down(struct semaphore * sem) {
        if (sem->count <= 0)
                __down(sem);
        sem->count--;
}

INLINE void up(struct semaphore * sem)
{
        sem->count++;
        wake_up(&sem->wait);
}

#endif

#endif

