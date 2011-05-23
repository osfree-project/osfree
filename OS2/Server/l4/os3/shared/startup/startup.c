/* $Id: startup.c 283 2008-01-22 19:28:21Z l4check $ */
/**
 * \file   l4env/lib/src/startup.c
 * \brief  Task startup (static version).
 *
 * \date   09/11/2000
 * \author Lars Reuther <reuther@os.inf.tu-dresden.de>
 *         Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *
 * L4Env task startup code.
 * Version for static linking */

/* (c) 2003 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */

/* standard includes */
#include <stdio.h>
#include <stdlib.h>

/* L4 includes */
#include <l4/crtx/crt0.h>
#include <l4/env/env.h>
#include <l4/l4rm/l4rm.h>
#include <l4/semaphore/semaphore.h>
#include <l4/sys/consts.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/syscalls.h>
#include <l4/thread/thread.h>
#include <l4/util/util.h>
#include <l4/util/mbi_argv.h>
#include <l4/util/l4_macros.h>
#include <l4/log/l4log.h>

/* binary test/data segments from linker script */
extern l4_uint8_t _prog_img_start __attribute__((weak));
extern l4_uint8_t _prog_img_end   __attribute__((weak));

/* exported symbols */
l4util_mb_info_t *l4env_multiboot_info;
l4env_infopage_t *l4env_infopage;

/* list of reserved VM regions (binary segments, boot modules, ...) */
//#define MAX_FIXED  32
#define MAX_FIXED  128
static l4rm_vm_range_t  fixed[MAX_FIXED];
static int              fixed_type[MAX_FIXED];
static int              num_fixed;

/* one if l4env startup done */
static unsigned __startup_done;

extern int main(int argc, char *argv[]);

/**
 * \brief Application worker thread.
 * Initializes memory and then starts up the application.
 */
static void
__startup_main(void)
{
  /* The ldso linker normally executes the dynamic_info[DT_INIT] function
   * of each loaded shared library. Initialization of l4rm and semaphore
   * is performed _after_ ldso has finished. Therefore we execute these
   * functions here. */
  if (l4env_infopage)
    {
      unsigned i;

      for (i=0; i<l4env_infopage->num_init_fn; i++)
	{
	  ((void(*)(void))(l4env_infopage->init_fn[i]))();
	}
    }

  /* call constructors, register destructors */
  crt0_construction();

  /* call application */
  exit(main(l4util_argc, l4util_argv));
}

/**
 * \brief Setup fixed VM regions.
 *
 * Reserve fixed program sections we know about (text, data, bss...)
 */
static void
__setup_fixed(void)
{
  if (l4env_infopage)
    {
      int i;

#ifdef DEBUG
      LOG_printf("Startup: Found %d sections\n", l4env_infopage->section_num);
#endif
      /* Attach all sections passed from the loader */
      for (i=0; i<l4env_infopage->section_num; i++)
	{
	  fixed[num_fixed].addr = l4env_infopage->section[i].addr;
	  fixed[num_fixed].size = l4env_infopage->section[i].size;
	  fixed_type[num_fixed] =
	    l4dm_is_invalid_ds(l4env_infopage->section[i].ds)
	       ? L4RM_REGION_PAGER
	       : L4RM_REGION_DATASPACE;
#ifdef DEBUG
	  LOG_printf("  "l4_addr_fmt"-"l4_addr_fmt" at "l4util_idfmt
	             " type %04x\n",
	      fixed[num_fixed].addr,
	      fixed[num_fixed].addr+fixed[num_fixed].size,
	      l4util_idstr(l4env_infopage->section[i].ds.manager),
	      l4env_infopage->section[i].info.type);
#endif
	  num_fixed++;
	}
#if 0
      /* reserve the infopage */
      fixed[num_fixed].addr = l4_trunc_page(l4env_infopage);
      fixed[num_fixed].size = L4_PAGESIZE;
      fixed_type[num_fixed] = L4RM_REGION_PAGER;
      num_fixed++;
#endif
      /* reserve the thread 0 stack */
      fixed[num_fixed].addr = l4_trunc_page(l4env_infopage->stack_low);
      fixed[num_fixed].size = l4_round_page(l4env_infopage->stack_high)
			    - l4_trunc_page(l4env_infopage->stack_low);
      fixed_type[num_fixed] = L4RM_REGION_PAGER;
      num_fixed++;

      /* reserved area of an OS/2 app address space before 0x10000 */
      fixed[num_fixed].addr = 0x1000;
      fixed[num_fixed].size = 0x9000 - 0x1000;
      fixed_type[num_fixed] = L4RM_REGION_BLOCKED;
      num_fixed++;

      /* reserved area of an OS/2 app address space before 0x10000 */
      fixed[num_fixed].addr = 0xd000;
      fixed[num_fixed].size = 0x10000 - 0xd000;
      fixed_type[num_fixed] = L4RM_REGION_BLOCKED;
      num_fixed++;
    }
  else
    {
      /* program text and data sections */
      if (&_prog_img_start && &_prog_img_end)
	{
	  fixed[num_fixed].addr = l4_trunc_page(&_prog_img_start);
	  fixed[num_fixed].size = l4_round_page(&_prog_img_end)
				   - l4_trunc_page(&_prog_img_start);
	  fixed_type[num_fixed] = L4RM_REGION_PAGER;
	  num_fixed++;
	}
      else
	{
	  /* can only happen if we are not linked using main_stat.ld nor
	   * main_dyn.ld */
	  LOG_printf("Warning: _prog_img_start/_prog_img_end not defined!\n");
	}
#if 0
      /* RMGR/Loader trampoline page */
      fixed[num_fixed].addr = l4_trunc_page(crt0_tramppage);
      fixed[num_fixed].size = L4_PAGESIZE;
      fixed_type[num_fixed] = L4RM_REGION_PAGER;
      num_fixed++;

      if (l4_trunc_page(crt0_tramppage) != l4_trunc_page(l4env_multiboot_info))
	{
	  /* roottask uses a separate page for multiboot info */
	  fixed[num_fixed].addr = l4_trunc_page(l4env_multiboot_info);
	  fixed[num_fixed].size = L4_PAGESIZE;
	  fixed_type[num_fixed] = L4RM_REGION_PAGER;
	  num_fixed++;
	}
#endif
      /* reserved area of an OS/2 app address space before 0x10000 */
      fixed[num_fixed].addr = 0x1000;
      fixed[num_fixed].size = 0x10000 - 0x1000;
      fixed_type[num_fixed] = L4RM_REGION_BLOCKED;
      num_fixed++;
    }
#if 0
// defined(ARCH_x86) || defined(ARCH_amd64)
  if (!l4env_infopage || l4env_infopage->loader_info.has_x86_vga)
    {
      /* adapter area (graphics memory) */
      fixed[num_fixed].addr = 0xA0000;
      fixed[num_fixed].size = 0xC0000 - 0xA0000;
      fixed_type[num_fixed] = L4RM_REGION_PAGER;
      num_fixed++;
    }
  if (!l4env_infopage || l4env_infopage->loader_info.has_x86_bios)
    {
      /* adapter area (BIOS memory) */
      fixed[num_fixed].addr = 0xC0000;
      fixed[num_fixed].size = 0x100000 - 0xC0000;
      fixed_type[num_fixed] = L4RM_REGION_PAGER;
      num_fixed++;
    }
#endif

  /* areas where our modules (e.g. files for memfs) live */
  if (l4env_multiboot_info->flags & L4UTIL_MB_MODS)
    {
      l4_umword_t i;
      l4util_mb_mod_t *m;

      m = (l4util_mb_mod_t*)(l4_addr_t) (l4env_multiboot_info->mods_addr);

      for (i=0; i<l4env_multiboot_info->mods_count; i++,m++)
	{
	  l4_addr_t addr     = l4_trunc_page(m->mod_start);
	  l4_addr_t end_addr = l4_round_page(m->mod_end);
	  l4_addr_t size     = end_addr - addr;

	  if (num_fixed == MAX_FIXED)
	    {
	      LOG_printf("Startup: too many modules!\n");
	      enter_kdebug("PANIC");
	      return;
	    }

	  fixed[num_fixed].addr = addr;
	  fixed[num_fixed].size = size;
	  fixed_type[num_fixed] = L4RM_REGION_PAGER;
	  num_fixed++;

	  /* page in module */
	  l4_touch_ro((void*)addr, size);
	}
    }
}

/**
 * \brief Task startup.
 */
void __main(void);

void
__main(void)
{
  int ret, i, j;
  l4_addr_t stack_low, stack_high;

#if defined(ARCH_x86) && defined(__PIC__)
  l4sys_fixup_abs_syscalls();
#endif

  if (crt0_multiboot_flag == ~(L4UTIL_MB_VALID))
    {
      crt0_multiboot_flag = L4UTIL_MB_VALID;
      l4env_infopage = crt0_l4env_infopage;
    }

  /* parse command line and initialize l4util_argc/l4util_argv */
  l4util_mbi_to_argv(crt0_multiboot_flag, crt0_multiboot_info);

  /* set pointer to multiboot info */
  l4env_multiboot_info = (l4util_mb_info_t*) crt0_multiboot_info;

  // prepare the log tag
  // skip os2app name and '--stdin <> --stdout <> --stderr <>'
  // and take the next word which is the name of the started 
  // OS/2 program. Skip its path and extension, and leave basename only.
  if (!strcmp(l4util_argv[0], "os2app") && l4util_argc > 7)
  {
    char *p = LOG_tag;        // the log tag
    char *q = l4util_argv[l4util_argc - 1]; // program path
    // start from the path end
    q = q + strlen(q);
    // search for the last backslash
    while (*q != '\\') q--;
    // skip it
    q++;
    // copy next < 8 chars until extension
    for (i = 0; *q && *q != '.' && i < 8; i++, p++, q++) *p = *q;
    // append zero byte
    *p++ = '\0';
  }

  /* Setup the LOG tag in the log library */
  LOG_setup_tag();

  /* init some internal L4env stuff */
  l4env_set_sigma0_id(l4_thread_ex_regs_pager(l4_myself()));

  /* setup fixed VM areas */
  __setup_fixed();

  /* init region mapper */
  if ((ret = l4rm_init(1, fixed, num_fixed)) < 0)
    {
      LOG_printf("Startup: setup region mapper failed (%d)!\n",ret);
      enter_kdebug("PANIC");
    }

  /* reserve VM areas */
  for (i=0, j=0; i<num_fixed; i++)
    {
      if (fixed_type[i] != L4RM_REGION_DATASPACE)
	{
	  if ((ret = l4rm_direct_area_setup_region(fixed[i].addr,
						   fixed[i].size,
						   L4RM_DEFAULT_REGION_AREA,
						   fixed_type[i], 0,
						   L4_INVALID_ID)) < 0)
	    {
	      LOG_printf("Startup: setup region "l4_addr_fmt"-"l4_addr_fmt
		         " failed (%d)!\n",
		  fixed[i].addr, fixed[i].addr + fixed[i].size, ret);
	      l4rm_show_region_list();
	      enter_kdebug("PANIC");
	    }
	}
      else
	{
	  while (l4dm_is_invalid_ds(l4env_infopage->section[j].ds))
	    {
	      j++;
	      if (j >= l4env_infopage->section_num)
		{
		  LOG_printf("Startup: did not find dataspace\n");
		  enter_kdebug("PANIC");
		}
	    }

	  if ((ret = l4rm_direct_attach_to_region(
		  &l4env_infopage->section[j].ds,
		  (void*)fixed[i].addr, fixed[i].size, 0, 
		  l4env_infopage->section[j].info.type & L4_DSTYPE_WRITE
		    ? L4DM_RW : L4DM_RO)))
	    {
	      LOG_printf("Startup: attach ds "l4_addr_fmt"-"l4_addr_fmt
		         " failed (%d)!\n",
		  fixed[i].addr, fixed[i].addr + fixed[i].size, ret);
	      enter_kdebug("PANIC");
	    }
	  j++;
	}
    }

  /* init thread lib */
  l4thread_init();

  if (l4env_infopage)
    {
      stack_low  = l4env_infopage->stack_low;
      stack_high = l4env_infopage->stack_high;
    }
  else
    {
      stack_low  = (l4_addr_t)&crt0_stack_low;
      stack_high = (l4_addr_t)&crt0_stack_high;
    }

  /* setup region mapper tcb */
  if ((ret = l4thread_setup(l4_myself(), ".rm", stack_low, stack_high)) < 0)
    {
      LOG_printf("Startup: setup region mapper tcb failed (%d)!\n",ret);
      enter_kdebug("PANIC");
    }

  /* init semaphore lib */
  if ((ret = l4semaphore_init()) < 0)
    {
      LOG_printf("Startup: semaphore lib initialization failed (%d)!\n",ret);
      enter_kdebug("PANIC");
    }

  __startup_done = 1;
  asm volatile("" : : : "memory");

  /* start main thread */
  if ((ret = l4thread_create_long(L4THREAD_INVALID_ID,
                                  (l4thread_fn_t)__startup_main,
                                  ".main", L4THREAD_INVALID_SP,
                                  L4THREAD_DEFAULT_SIZE, L4THREAD_DEFAULT_PRIO,
                                  NULL,
			          L4THREAD_CREATE_ASYNC |
			            L4THREAD_CREATE_SETUP)) < 0)
    {
      LOG_printf("Startup: create main thread failed (%d)!\n",ret);
      enter_kdebug("PANIC");
    }

  /* start service loop */
  l4rm_service_loop();
}

/**
 * \brief  Return pointer to L4 environment page
 *
 * \return NULL
 *
 * Since this is a sigma0-style L4 task, it has no L4 environment infopage
 * mapped from the L4 Loader. But nevertheless, it could be started by the
 * compatibility mode of the L4 loader.
 */
l4env_infopage_t *
l4env_get_infopage(void)
{
  return l4env_infopage;
}

/**
 * \brief Return if L4Env startup has been completed
 * \return true for yes, false for no
 */
unsigned
l4env_startup_done(void)
{
  return __startup_done;
}
