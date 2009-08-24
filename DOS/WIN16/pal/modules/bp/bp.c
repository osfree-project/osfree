/* Hardware breakpoint module
 *
 * Copyright 1995, Willows Software, Inc.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/malloc.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/unistd.h>
#include <linux/debugreg.h>
#include <linux/user.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <stdarg.h>
#include "bp.h"

char kernel_version[] = UTS_RELEASE;

typedef asmlinkage int (*sys_call_entry_t)(long request, long pid, 
					 long addr, long data);

extern sys_call_entry_t sys_call_table[];

sys_call_entry_t old_sys_ptrace;

asmlinkage int bp_sys_ptrace(long request, long pid, long addr, long data)
{
    struct user *dummy = NULL;
    unsigned long tmp;
    int error;

    if (pid == current->pid && 
	(request == PTRACE_PEEKUSR || request == PTRACE_POKEUSR) &&
	addr >= (long) &dummy->u_debugreg[0] &&
	addr <= (long) &dummy->u_debugreg[7])
    {
	switch (request)
	{
	  case PTRACE_PEEKUSR:
	    error = verify_area(VERIFY_WRITE, (void *) data, sizeof(long));
	    if (error)
	    {
		printk("self ptrace PEEK ERROR %d\n", error);
		return error;
	    }

	    addr -= (long) &dummy->u_debugreg[0];
	    addr >>= 2;
	    tmp = current->debugreg[addr];

	    put_fs_long(tmp, (unsigned long *) data);
	    return 0;
	    
	  case PTRACE_POKEUSR:
	    addr -= (long) &dummy->u_debugreg[0];
	    addr >>= 2;

	    if (addr == 4 || addr == 5 || 
		(addr < 4 && (unsigned long) data >= 0xbffffffd))
	    {
		return -EIO;
	    }

	    if (addr == 7)
	    {
		int i;
		
		data &= ~DR_CONTROL_RESERVED;
		for (i = 0; i < 4; i++)
		    if ((0x5f54 >> ((data >> (16 + 4*i)) & 0xf)) & 1)
			return -EIO;
	    }

	    current->debugreg[addr] = data;
	    return 0;
	}
    }

    return (*old_sys_ptrace)(request, pid, addr, data);
}

int init_module(void)
{
    old_sys_ptrace = sys_call_table[__NR_ptrace];
    sys_call_table[__NR_ptrace] = bp_sys_ptrace;
    
    return 0;
}

void cleanup_module(void)
{
    sys_call_table[__NR_ptrace] = old_sys_ptrace;
}
