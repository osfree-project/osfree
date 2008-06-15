
Open Watcom on Linux can't call the system call for mmap and munmap, they are 
not part of the runtime library. Some attempt to do it manually. Not working yet.

When a userspace application makes a system call, the arguments are passed via registers and 
the application executes 'int 0x80' instruction. This causes a trap into kernel mode and 
processor jumps to system_call entry point in entry.S. What this does is: 

Save registers. 
Set %ds and %es to KERNEL_DS, so that all data (and extra segment) references are made 
in kernel address space. 
If the value of %eax is greater than NR_syscalls (currently 256), fail with ENOSYS error. 
If the task is being ptraced (tsk->ptrace & PF_TRACESYS), do special processing. This is 
to support programs like strace (analogue of SVR4 truss(1)) or debuggers. 
Call sys_call_table+4*(syscall_number from %eax). This table is initialised in the same 
file (arch/i386/kernel/entry.S) to point to individual system call handlers which under
 Linux are (usually) prefixed with sys_, e.g. sys_open, sys_exit, etc. These C system 
 call handlers will find their arguments on the stack where SAVE_ALL stored them. 
Enter 'system call return path'. This is a separate label because it is used not only 
by int 0x80 but also by lcall7, lcall27. This is concerned with handling tasklets 
(including bottom halves), checking if a schedule() is needed (tsk->need_resched != 0), 
checking if there are signals pending and if so handling them.

Linux supports up to 6 arguments for system calls. 
They are passed in %ebx, %ecx, %edx, %esi, %edi (and %ebp used temporarily, see _syscall6() 
in asm-i386/unistd.h). The system call number is passed via %eax.

call_linux_syscall

eax, sys_nr

6 argument i register
%ebx, %ecx, %edx, %esi, %edi

	.long old_mmap		/* 90 */
	.long sys_munmap    /* 91 */

    .long sys_mmap2     /* 192 */

#define  size_t unsigned long int
#define off_t unsigned long int
//_WCRTLINK extern 
void *mmap( void *__addr, size_t __len, int __prot, int __flags, int __fd, off_t __offset )
{return 0;}

//_WCRTLINK extern 
int munmap( void *__addr, size_t __len )
{
eax, 91
ebx, __addr
ecx, __len
edx, __prot
esi, __flags
edi, __fd
return 0;}






