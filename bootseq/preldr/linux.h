/*
 *  Linux setup parameters
 */

#define LINUX_MAGIC_SIGNATURE		0x53726448	/* "HdrS" */
#define LINUX_DEFAULT_SETUP_SECTS	4
#define LINUX_FLAG_CAN_USE_HEAP		0x80
#define LINUX_INITRD_MAX_ADDRESS	0x38000000
#define LINUX_MAX_SETUP_SECTS		64
#define LINUX_BOOT_LOADER_TYPE		0x71
#define LINUX_HEAP_END_OFFSET		(0x9000 - 0x200)

#define LINUX_BZIMAGE_ADDR		RAW_ADDR (0x100000)
#define LINUX_ZIMAGE_ADDR		RAW_ADDR (0x10000)
#define LINUX_OLD_REAL_MODE_ADDR	RAW_ADDR (0x90000)
#define LINUX_SETUP_STACK		0x9000

#define LINUX_FLAG_BIG_KERNEL		0x1

/* Linux's video mode selection support. Actually I hate it!  */
#define LINUX_VID_MODE_NORMAL		0xFFFF
#define LINUX_VID_MODE_EXTENDED		0xFFFE
#define LINUX_VID_MODE_ASK		0xFFFD

#define LINUX_CL_OFFSET			0x9000
#define LINUX_CL_END_OFFSET		0x90FF
#define LINUX_SETUP_MOVE_SIZE		0x9100
#define LINUX_CL_MAGIC			0xA33F

/* For the Linux/i386 boot protocol version 2.03.  */
_Packed struct linux_kernel_header
{
  char code1[0x0020];
  unsigned short cl_magic;              /* Magic number 0xA33F */
  unsigned short cl_offset;             /* The offset of command line */
  char code2[0x01F1 - 0x0020 - 2 - 2];
  unsigned char setup_sects;            /* The size of the setup in sectors */
  unsigned short root_flags;            /* If the root is mounted readonly */
  unsigned short syssize;               /* obsolete */
  unsigned short swap_dev;              /* obsolete */
  unsigned short ram_size;              /* obsolete */
  unsigned short vid_mode;              /* Video mode control */
  unsigned short root_dev;              /* Default root device number */
  unsigned short boot_flag;             /* 0xAA55 magic number */
  unsigned short jump;                  /* Jump instruction */
  unsigned long header;                 /* Magic signature "HdrS" */
  unsigned short version;               /* Boot protocol version supported */
  unsigned long realmode_swtch;         /* Boot loader hook */
  unsigned long start_sys;              /* Points to kernel version string */
  unsigned char type_of_loader;         /* Boot loader identifier */
  unsigned char loadflags;              /* Boot protocol option flags */
  unsigned short setup_move_size;       /* Move to high memory size */
  unsigned long code32_start;           /* Boot loader hook */
  unsigned long ramdisk_image;          /* initrd load address */
  unsigned long ramdisk_size;           /* initrd size */
  unsigned long bootsect_kludge;        /* obsolete */
  unsigned short heap_end_ptr;          /* Free memory after setup end */
  unsigned short pad1;                  /* Unused */
  char *cmd_line_ptr;                   /* Points to the kernel command line */
  unsigned long initrd_addr_max;        /* The highest address of initrd */
}; //__attribute__ ((packed));

