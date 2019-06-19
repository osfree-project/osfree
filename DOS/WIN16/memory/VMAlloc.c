
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "Log.h"
#include "VM.h"

static VM_MAP vm_cache[256];
static VM_MAP *vm_list = vm_cache;
static VM_MAP *vm_free;
static VM_MAP *vm_map;

static int  vm_cnt = 256;

/* GLOBAL FUNCTIONS */
void *VirtualMemory(int,void *);

/* INTERNAL FUNCTIONS */
static void mmap_set(void *,int ,int );
static VM_MAP * vm_getvm();
static int mmap_fd();

/*
 * External Interface to internal VM structures and functions
 * Initialize VM
 * Free VM
 * GetVMMAP
 */
void *
VirtualMemory(int flag,void *map)
{
	void   *start;
	size_t size;
	int 	prot,flags;
	int     fd;

	void   *base;
	int	i;

	switch(flag) {
	case 0:
		prot  = PROT_READ|PROT_WRITE|PROT_EXEC;
		flags = MAP_PRIVATE;

		size = PAGEGRAN;
		start = 0;

		fd = mmap_fd();

		for(;;) {
			base = mmap(start,size,prot,flags,fd,(off_t) start);

			i = (start == base);
				
			if(base > 0)
				munmap(base,size);	

			if(base == (void *) 0xffffffff)
				break;

			mmap_set(start,size,i);

			start += size; 
		}	
		break;
	case 1:
		return (void *) vm_map;
	}
	return 0;
}

static VM_MAP *
vm_setvm(VM_MAP *vm,int where,void *base,void *limit,DWORD type,DWORD protect)
{
	VM_MAP *vp = 0;

	/* desired is embedded inside entry */
	if(where == 0) {
		vp = vm_getvm();
	
		/* link this one in */
		vp->next = vm->next;
		vm->next = vp;

		/* initialize it */
		vp->type    = vm->type;	
		vp->protect = vm->protect;	

		vp->base    = limit;
		vp->limit   = vm->limit;
		vp->flag    = vm->flag;
		
		/* fix limit */
		vm->limit   = limit;
		
		where = 2;
	}

	/* desired is at start of vm */
	if(where & 1) {

		vp = vm_getvm();
		/* link this one in */
		vp->next = vm->next;
		vm->next = vp;

		/* initialize it */
		vp->type    = vm->type;	
		vp->protect = vm->protect;	
		vp->limit   = vm->limit;
		vp->base    = limit;
		
		/* new limit on lower entry */
		vm->limit   = limit;
		vp = vm;
	}

	/* desired is at end of vm */
	if(where & 2) {

		vp = vm_getvm();
		/* link this one in */
		vp->next = vm->next;
		vm->next = vp;

		/* initialize it */
		vp->type    = vm->type;	
		vp->protect = vm->protect;	
		vp->base    = base;
		vp->limit   = vm->limit;
		
		/* fix limit */
		vm->limit   = base;
	}

	return vp;
}

static VM_MAP *
vm_getvm()
{
	VM_MAP *vm;

	/* do we have a free element */
	/* if so, pull from list     */
	if(vm_free) {
		vm = vm_free;
		vm->next = 0;
		vm_free = vm_free->next;
		return vm;
	}
	
	/* do we have any in our cache? */
	/* if not, allocate new cache   */
	if(vm_cnt == 0) {
		vm_list = (VM_MAP *) WinMalloc(sizeof(VM_MAP)*256);
		vm_cnt  = 256;
	}

	/* pull the first from the cache */
	vm = &vm_list[256 - vm_cnt];
	vm_cnt--;
	vm->next = 0;
	return vm;
}

static void
mmap_set(void *base,int size,int flag)
{
	VM_MAP *vm,*vl,*vp = 0;
	void *limit = base + size;

	/* find the element that would be infront of this one */
	for(vm=vm_map;vm;vm=vm->next) {
		if(vm->limit == base) {
			vp = vm;
			break;
		}
		if(vm->limit < base)
			vp = vm;
	}

	if(vm && vm->flag == flag) {
		vm->limit = limit;
		return;	
	}

	vl = vm_getvm();
	vl->base = base;
	vl->limit = limit;
	vl->protect = 0;
	vl->type    = 0;	

	vl->flag  = flag;
	vl->next  = 0;
		
	if(vp) {
		vl->next = vp->next;
		vp->next = vl;
		return;
	}

	/* link it in front... */
	vl->next = vm_map;
	vm_map = vl;
}

static int
mmap_fd()
{
	static int fd;

	if(fd)
	  return fd;

	fd = open("/dev/zero",O_RDONLY);
	if(fd < 0) {
	     fd = 0;
	}
	return fd;
}

LPVOID 
VirtualAlloc( LPVOID base, DWORD size, DWORD type, DWORD protect )
{
	VM_MAP *vm,*vp = 0;
	void *lbase    = 0;
	void *limit = base + size;
	
	// have we figured out what we have available...
	// if not, initialize the vm subsystem
	if(vm_map == 0)
		VirtualMemory(0,0);

	// look in our maps for the desired memory...
	// it is sorted by address, each pointing to next...
	for(vm=vm_map;vm;vm = vm->next) {

		// is it in use already?
		if(vm->flag != 1)
			continue;

		// candidate block...
		if(vm->base + size < vm->limit) {
			vp = vm;
		}	

		// is the desired block in the range of this map?
		if((vm->base <= base) && (vm->limit >= limit)) { 
			int  where = 0;

			// yes, and starts at beginning
			if(vm->base == base)
				where |= 1;

			// yes, and goes to the end
			if(vm->limit == limit) 
				where |= 2;
			
			// oh, it IS this block
			if(where == 3) {
				vp = vm;
				break;
			}
				
			// allocate a SUBMAP from a given MAP
			vp = vm_setvm(vm,where,base,limit,type,protect);
			break;
		}
	}

	// we have a SUBMAP, no setup its attributes
	if(vp) {
		vp->protect = protect;
		vp->type    = type;

		if(type & MEM_COMMIT) {
			int  prot;
			int  flags;
			int  fd;

			prot  = PROT_READ|PROT_WRITE|PROT_EXEC;
			flags = MAP_PRIVATE;
			fd    = mmap_fd();
			lbase = mmap(vp->base,size,prot,flags,fd,(off_t) base);
		} else {
			lbase = vp->base;
		}
	} else {
		// seterrormode
		// cannot allocate desired memory
		lbase = 0;
	}

	return lbase;
	
}
