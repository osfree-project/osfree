// ;
// ; fsd.inc
// ;

#define KB  0x400
#define KSHIFT  10
#define KMASK  (KB - 1)
#define MEMPAGE  0x1000
#define PAGESHIFT  12
#define PAGEMASK  (MEMPAGE - 1)

#define MFS_BASE  0x7c0 // minifsd base
#define LDR_BASE  0x10000 // os2ldr base

#define MFS_SEG  0x7c // minifsd segment
#define LDR_SEG  0x1000 // os2ldr segment

#define PREFERRED_BASE  0x6f000 // preferred load base (addresses 0x6f000-0x7f000 occupied by pre-loader
#define BOOT_PART_TABLE  (0x70000 + SHIFT) //
#define FSYS_BUFLEN  0x8000

// ;
// ; buffers at the end of 3rd megabyte
// ;
#define FSYS_BUF  0x3e0000 //
#define EXT1HIBUF_BASE  0x3e8000 // blackbox high memory buffer1
#define EXT2HIBUF_BASE  0x3ec000 // blackbox high memory buffer2
#define EXT3HIBUF_BASE  0x3f0000 // blackbox high memory buffer3
#define EXT4HIBUF_BASE  0x3f4000
#define UFSD_BASE  0x3e8000 // EXT1HIBUF_BASE ; boot drive uFSD save buffer base
#define TERM_BASE  0x3ec000 // EXT3HIBUF_BASE

#define NEW_STACK_SP  0x3fe000 // stack
#define STACKLEN  0x4000

// ;STACK_SP        equ (0x74800 + SHIFT)     ; stack
// ;
// ;EXT1LOBUF_BASE  equ 0x74400               ; blackbox low memory buffer1
// ;EXT2LOBUF_BASE  equ 0x74800               ; blackbox low memory buffer2
// ;EXT3LOBUF_BASE  equ 0x74400               ; blackbox low memory buffer3
// ;TERMLO_BASE     equ 0x74800               ; terminal blackbox base (low)
// ;BOOTSEC_BASE    equ (0x74c00 + SHIFT)     ; bootsector relocation base
// ;STAGE0_BASE     equ (0x74e00 + SHIFT)     ; Stage0 load base
// ;EXT_BUF_BASE    equ (0x7c000 + SHIFT)     ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

#define STACK_SP  (0x73800 + SHIFT) // stack

#define EXT1LOBUF_BASE  0x73400 // blackbox low memory buffer1
#define EXT2LOBUF_BASE  0x73800 // blackbox low memory buffer2
#define EXT3LOBUF_BASE  0x73c00 // blackbox low memory buffer3
#define TERMLO_BASE  0x73800 // terminal blackbox base (low)
#define BOOTSEC_BASE  (0x73c00 + SHIFT) // bootsector relocation base
#define STAGE0_BASE  (0x73e00 + SHIFT) // Stage0 load base
#define EXT_BUF_BASE  (0x7c000 + SHIFT) // buffer for extensions (blackboxes) (size: 0x2800 bytes)

#define SCRATCHADDR  (0x6f000 + SHIFT) // 512-byte scratch area
#define BUFFERADDR  SCRATCHADDR + 0x200 //
#define BUFFERLEN  0xe00 // 0x7e00

#define EXT_LEN  0x3000 // extension (blackbox) buffer size
#define EXTLO_LEN  0x400 // low portion
#define EXT2LEN  0x5000
#define STAGE0_LEN  (EXT_BUF_BASE - STAGE0_BASE)
// ;STAGE0_LEN      equ (0x7f000 + SHIFT - STAGE0_BASE)

#define FSYS_MAX  0x20 // max number of filesystems
#define BOOT_MAGIC  0x2BADB002 // extended boot info magic

// ; Protected mode stack address
#define PROTSTACK  0x600
// ; Real mode stack address
#define RMSTACK  0x604
// ; protect mode IDT save
#define IDTR  0x608
// ; real mode IDT save
#define IDTR_OLD  0x610

// ; GDT location
#define GDT_ADDR  0x640
// ; IDT location
#define IDT_ADDR  0x6a0

// ; stacks initial values
#define PROTSTACKINIT  STACK_SP - 10h
#define REALSTACKINIT  STAGE0_LEN
