/*
 *
 */

#pragma aux m     "*"
#pragma aux l     "*"
#pragma aux stop  "*"
#pragma aux linux_boot     "*"
#pragma aux big_linux_boot "*"
#pragma aux linux_data_real_addr "*"
#pragma aux linux_data_tmp_addr  "*"
#pragma aux linux_text_len       "*"
#pragma aux errnum               "*"

#include <lip.h>
#include "linux.h"
#include "struc.h"
#include "term.h"

extern void stop(void);
extern void big_linux_boot(void);
extern void linux_boot(void);
struct multiboot_info *m;
char *kernel, *initrd;
char *kernel_cmdline;
int kernel_size, initrd_size;
int big_linux;
static int linux_mem_size;
struct linux_kernel_header *lh;
int len;
unsigned long moveto;
unsigned long max_addr;

unsigned int data_len, text_len;
unsigned int cur_addr;

grub_error_t errnum;
lip2_t *l;
struct term_entry *t;

void init(void)
{

}

int kernel_ldr(void)
{
  /* Use BUFFER as a linux kernel header, if the image is Linux zImage
     or bzImage.  */
  lh = (struct linux_kernel_header *) kernel;
 
  if (lh->boot_flag == BOOTSEC_SIGNATURE
	   && lh->setup_sects <= LINUX_MAX_SETUP_SECTS)
  {
      int setup_sects = lh->setup_sects;
      big_linux = 0;

      if (lh->header == LINUX_MAGIC_SIGNATURE && lh->version >= 0x0200)
	{
	  big_linux = (lh->loadflags & LINUX_FLAG_BIG_KERNEL);
	  lh->type_of_loader = LINUX_BOOT_LOADER_TYPE;

	  /* Put the real mode part at as a high location as possible.  */
	  linux_data_real_addr
	    = (char *) ((m->mem_lower << 10) - LINUX_SETUP_MOVE_SIZE);
	  /* But it must not exceed the traditional area.  */
	  if (linux_data_real_addr > (char *) LINUX_OLD_REAL_MODE_ADDR)
	    linux_data_real_addr = (char *) LINUX_OLD_REAL_MODE_ADDR;

	  if (lh->version >= 0x0201)
	    {
	      lh->heap_end_ptr = LINUX_HEAP_END_OFFSET;
	      lh->loadflags |= LINUX_FLAG_CAN_USE_HEAP;
	    }

	  if (lh->version >= 0x0202)
	    lh->cmd_line_ptr = linux_data_real_addr + LINUX_CL_OFFSET;
	  else
	    {
	      lh->cl_magic = LINUX_CL_MAGIC;
	      lh->cl_offset = LINUX_CL_OFFSET;
	      lh->setup_move_size = LINUX_SETUP_MOVE_SIZE;
	    }
	}
      else
	{
	  /* Your kernel is quite old...  */
	  lh->cl_magic = LINUX_CL_MAGIC;
	  lh->cl_offset = LINUX_CL_OFFSET;
	  
	  setup_sects = LINUX_DEFAULT_SETUP_SECTS;

	  linux_data_real_addr = (char *) LINUX_OLD_REAL_MODE_ADDR;
	}
      
      /* If SETUP_SECTS is not set, set it to the default (4).  */
      if (! setup_sects)
	setup_sects = LINUX_DEFAULT_SETUP_SECTS;

      data_len = setup_sects << 9;
      text_len = kernel_size - data_len - SECTOR_SIZE;

      linux_data_tmp_addr = (char *) LINUX_BZIMAGE_ADDR + text_len;
      
      if (! big_linux
	  && text_len > linux_data_real_addr - (char *) LINUX_ZIMAGE_ADDR)
	{
	  grub_printf (" linux 'zImage' kernel too big, try 'make bzImage'\n");
	  errnum = ERR_WONT_FIT;
	}
      else if (linux_data_real_addr + LINUX_SETUP_MOVE_SIZE
	       > RAW_ADDR ((char *) (m->mem_lower << 10)))
	errnum = ERR_WONT_FIT;
      else
	{
	  grub_printf ("   [Linux-%s, setup=0x%x, size=0x%x]\r\n",
		       (big_linux ? "bzImage" : "zImage"), data_len, text_len);

	  /* Video mode selection support. What a mess!  */
	  /* NOTE: Even the word "mess" is not still enough to
	     represent how wrong and bad the Linux video support is,
	     but I don't want to hear complaints from Linux fanatics
	     any more. -okuji  */
	  {
	    char *vga;
	
	    /* Find the substring "vga=".  */
	    vga = grub_strstr (kernel_cmdline, "vga=");
	    if (vga)
	      {
		char *value = vga + 4;
		int vid_mode;
	    
		/* Handle special strings.  */
		if (substring ("normal", value) < 1)
		  vid_mode = LINUX_VID_MODE_NORMAL;
		else if (substring ("ext", value) < 1)
		  vid_mode = LINUX_VID_MODE_EXTENDED;
		else if (substring ("ask", value) < 1)
		  vid_mode = LINUX_VID_MODE_ASK;
		else if (safe_parse_maxint (&value, &vid_mode))
		  ;
		else
		  {
		    /* ERRNUM is already set inside the function
		       safe_parse_maxint.  */
		    //grub_close ();
		    return 0;
		  }
	    
		lh->vid_mode = vid_mode;
	      }
	  }

	  /* Check the mem= option to limit memory used for initrd.  */
	  {
	    char *mem;
	
	    mem = grub_strstr (kernel_cmdline, "mem=");
	    if (mem)
	      {
		char *value = mem + 4;
	    
		safe_parse_maxint (&value, &linux_mem_size);
		switch (errnum)
		  {
		  case ERR_NUMBER_OVERFLOW:
		    /* If an overflow occurs, use the maximum address for
		       initrd instead. This is good, because MAXINT is
		       greater than LINUX_INITRD_MAX_ADDRESS.  */
		    linux_mem_size = LINUX_INITRD_MAX_ADDRESS;
		    errnum = ERR_NONE;
		    break;
		
		  case ERR_NONE:
		    {
		      int shift = 0;
		  
		      switch (grub_tolower (*value))
			{
			case 'g':
			  shift += 10;
			case 'm':
			  shift += 10;
			case 'k':
			  shift += 10;
			default:
			  break;
			}
		  
		      /* Check an overflow.  */
		      if (linux_mem_size > (MAXINT >> shift))
			linux_mem_size = LINUX_INITRD_MAX_ADDRESS;
		      else
			linux_mem_size <<= shift;
		    }
		    break;
		
		  default:
		    linux_mem_size = 0;
		    errnum = ERR_NONE;
		    break;
		  }
	      }
	    else
	      linux_mem_size = 0;
	  }
      
	  /* It is possible that DATA_LEN + SECTOR_SIZE is greater than
	     MULTIBOOT_SEARCH, so the data may have been read partially.  */
	  //if (data_len + SECTOR_SIZE <= MULTIBOOT_SEARCH)
	  //  grub_memmove (linux_data_tmp_addr, buffer,
	  //		  data_len + SECTOR_SIZE);
	  //else
	  //  {
	  //    grub_memmove (linux_data_tmp_addr, buffer, MULTIBOOT_SEARCH);
	  //    grub_read (linux_data_tmp_addr + MULTIBOOT_SEARCH,
          //			 data_len + SECTOR_SIZE - MULTIBOOT_SEARCH);
	  //  }
	  
          grub_memmove(linux_data_tmp_addr, kernel, data_len + SECTOR_SIZE);

	  if (lh->header != LINUX_MAGIC_SIGNATURE ||
	      lh->version < 0x0200)
	    /* Clear the heap space.  */
	    grub_memset (linux_data_tmp_addr + ((setup_sects + 1) << 9),
			 0,
			 (64 - setup_sects - 1) << 9);
      
	  /* Copy command-line plus memory hack to staging area.
	     NOTE: Linux has a bug that it doesn't handle multiple spaces
	     between two options and a space after a "mem=" option isn't
	     removed correctly so the arguments to init could be like
	     {"init", "", "", NULL}. This affects some not-very-clever
	     shells. Thus, the code below does a trick to avoid the bug.
	     That is, copy "mem=XXX" to the end of the command-line, and
	     avoid to copy spaces unnecessarily. Hell.  */
	  {
	    char *src = skip_to (0, kernel_cmdline);
	    char *dest = linux_data_tmp_addr + LINUX_CL_OFFSET;
	
	    while (dest < linux_data_tmp_addr + LINUX_CL_END_OFFSET && *src)
	      *(dest++) = *(src++);
	
	    /* Old Linux kernels have problems determining the amount of
	       the available memory.  To work around this problem, we add
	       the "mem" option to the kernel command line.  This has its
	       own drawbacks because newer kernels can determine the
	       memory map more accurately.  Boot protocol 2.03, which
	       appeared in Linux 2.4.18, provides a pointer to the kernel
	       version string, so we could check it.  But since kernel
	       2.4.18 and newer are known to detect memory reliably, boot
	       protocol 2.03 already implies that the kernel is new
	       enough.  The "mem" option is added if neither of the
	       following conditions is met:
	       1) The "mem" option is already present.
	       2) The "kernel" command is used with "--no-mem-option".
	       3) GNU GRUB is configured not to pass the "mem" option.
	       4) The kernel supports boot protocol 2.03 or newer.  */
	    if (! grub_strstr (kernel_cmdline, "mem=")
		//&& ! (load_flags & KERNEL_LOAD_NO_MEM_OPTION)                   //!!
		&& lh->version < 0x0203		/* kernel version < 2.4.18 */
		&& dest + 15 < linux_data_tmp_addr + LINUX_CL_END_OFFSET)
	      {
		*dest++ = ' ';
		*dest++ = 'm';
		*dest++ = 'e';
		*dest++ = 'm';
		*dest++ = '=';
	    
		dest = convert_to_ascii (dest, 'u', (m->mem_upper + 0x400));
		*dest++ = 'K';
	      }
	
	    *dest = 0;
	  }
      
	  /* offset into file */
	  //grub_seek (data_len + SECTOR_SIZE);
      
	  cur_addr = (int) linux_data_tmp_addr + LINUX_SETUP_MOVE_SIZE;
	  //grub_read ((char *) LINUX_BZIMAGE_ADDR, text_len);
          grub_memmove((char *) LINUX_BZIMAGE_ADDR, kernel + data_len + SECTOR_SIZE, text_len);
      
	  if (errnum == ERR_NONE)
	    {
	      //grub_close ();
	  
	      /* Sanity check.  */
	      //if (suggested_type != KERNEL_TYPE_NONE
	      //	  && ((big_linux && suggested_type != KERNEL_TYPE_BIG_LINUX)
	      //          || (! big_linux && suggested_type != KERNEL_TYPE_LINUX)))
	      //	{
	      //	  errnum = ERR_EXEC_FORMAT;
	      //	  return KERNEL_TYPE_NONE;
	      //	}
	  
	      /* Ugly hack.  */
	      linux_text_len = text_len;
	  
	      //return big_linux ? KERNEL_TYPE_BIG_LINUX : KERNEL_TYPE_LINUX;
	    }
	}

    return 1;
  }

  return 0;
}

int initrd_ldr(void)
{
  lh = (struct linux_kernel_header *) (cur_addr - LINUX_SETUP_MOVE_SIZE);
  
#ifndef NO_DECOMPRESSION
  no_decompression = 1;
#endif
  
  //if (! grub_open (initrd))
  //  goto fail;

  //len = grub_read ((char *) cur_addr, -1);
  cur_addr = (unsigned int)initrd;
  len = initrd_size;

  //if (! len)
  //  {
  //    grub_close ();
  //    goto fail;
  //  }

  if (linux_mem_size)
    moveto = linux_mem_size;
  else
    moveto = (m->mem_upper + 0x400) << 10;
  
  moveto = (moveto - len) & 0xfffff000;
  max_addr = (lh->header == LINUX_MAGIC_SIGNATURE && lh->version >= 0x0203
	      ? lh->initrd_addr_max : LINUX_INITRD_MAX_ADDRESS);
  if (moveto + len >= max_addr)
    moveto = (max_addr - len) & 0xfffff000;
  
  /* XXX: Linux 2.3.xx has a bug in the memory range check, so avoid
     the last page.
     XXX: Linux 2.2.xx has a bug in the memory range check, which is
     worse than that of Linux 2.3.xx, so avoid the last 64kb. *sigh*  */
  moveto -= 0x10000;
  memmove ((void *) RAW_ADDR (moveto), (void *) cur_addr, len);

  printf ("   [Linux-initrd @ 0x%x, 0x%x bytes]\r\n", moveto, len);

  /* FIXME: Should check if the kernel supports INITRD.  */
  lh->ramdisk_image = RAW_ADDR (moveto);
  lh->ramdisk_size = len;

  //grub_close ();

  return 1;

  //fail:
  
  #ifndef NO_DECOMPRESSION
    no_decompression = 0; 
  #endif

  return 0;
}


int loader(void)
{
  kernel_ldr();

  if (initrd)
     initrd_ldr();

  return 1;
}

int check_lip(char *mods_addr, unsigned long mods_count)
{
  struct mod_list *mod;

  // last module in the list
  mod = (struct mod_list *)(mods_addr + (mods_count - 1) * sizeof(struct mod_list));
  if (grub_strstr((char *)(mod->cmdline), "*lip"))
  {
    // set LIP pointer
    l = (lip2_t *)mod->mod_start;
    // check if the LIP begins with a 
    // magic number of 0x3badb002
    if (*((unsigned long *)l) == LIP2_MAGIC)
    {
      t = l->u_termctl(-1);
      printf("boot_linux started\r\n");
      return 1;
    }
    else
      return 0;
  }

  return 0;
}

void cmain(void)
{
  char *mods_addr;
  int mods_count;
  struct mod_list *mod; 

  mods_addr  = (char *)m->mods_addr;
  mods_count = m->mods_count;

  // kernel is the first module in the list,
  // and initrd is the second 
  mod = (struct mod_list *)mods_addr;

  kernel = (char *)mod->mod_start;
  kernel_size = mod->mod_end - mod->mod_start;
  kernel_cmdline = (char *)mod->cmdline;

  if (mods_count < 2)
    stop();
  else
  {
    if (!check_lip(mods_addr, mods_count))
      stop();

    if (mods_count > 2)
    {
      ++mod;

      initrd = (char *)mod->mod_start;
      initrd_size = mod->mod_end - mod->mod_start;
    }
    else
    {
      initrd = 0;
      initrd_size = 0;
    }

  }

  if (loader())
  { 
    //printf("loader() finished\r\n");

    //__asm {
    //  cli
    //  hlt
    //}

    if (big_linux)
       big_linux_boot();
    else
       linux_boot();
  }
  else
  {
    printf("Error loading linux!\r\n");
  }
}
