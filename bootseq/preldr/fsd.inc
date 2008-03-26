;
; fsd.inc
;

KB              equ 0x400
KSHIFT          equ 10
KMASK           equ (KB - 1)
MEMPAGE         equ 0x1000
PAGESHIFT       equ 12
PAGEMASK        equ (MEMPAGE - 1)

MFS_BASE        equ 0x7c0                 ; minifsd base
LDR_BASE        equ 0x10000               ; os2ldr base

MFS_SEG         equ 0x7c                  ; minifsd segment
LDR_SEG         equ 0x1000                ; os2ldr segment

PREFERRED_BASE  equ 0x6f000               ; preferred load base (addresses 0x6f000-0x7f000 occupied by pre-loader
                                          ; when loaded initially without relocation)
BOOT_PART_TABLE equ 0x600                 ;
FSYS_BUFLEN     equ 0x8000
;
; buffers at the end of 3rd megabyte
;
FSYS_BUF        equ 0x3e0000              ;
UFSD_BASE       equ 0x3e8000              ; boot drive uFSD save buffer base
EXT2BUF_BASE    equ 0x3ea800              ; second buffer (size: 0x2800 bytes)
;EXT_BUF_BASE    equ 0x3ed000              ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

;STACK_SP        equ 0x400000              ; stack
STACK_SP        equ (0x72000 + SHIFT)     ; stack
STACKLEN        equ 0x4000

BOOTSEC_BASE    equ (0x72000 + SHIFT)     ; bootsector relocation base
STAGE0_BASE     equ (0x72200 + SHIFT)     ; Stage0 load base
EXT_BUF_BASE    equ (0x7c800 + SHIFT)     ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

SCRATCHADDR     equ (0x6f000 + SHIFT)     ; 512-byte scratch area
BUFFERADDR      equ SCRATCHADDR + 0x200   ;
BUFFERLEN       equ 0xe00                ; 0x7e00

EXT_LEN         equ 0x2800                ; extension (blackbox) buffer size
EXT2LEN         equ 0x5000
STAGE0_LEN      equ (EXT_BUF_BASE - STAGE0_BASE)
;STAGE0_LEN      equ (0x7f000 + SHIFT - STAGE0_BASE)

FSYS_MAX        equ 0x20                  ; max number of filesystems
BOOT_MAGIC      equ 0x2BADB002            ; extended boot info magic

;
;  Linux setup parameters
;

LINUX_MAGIC_SIGNATURE        equ   0x53726448      ; "HdrS"
LINUX_DEFAULT_SETUP_SECTS    equ   4
LINUX_FLAG_CAN_USE_HEAP      equ   0x80
LINUX_INITRD_MAX_ADDRESS     equ   0x38000000
LINUX_MAX_SETUP_SECTS        equ   64
LINUX_BOOT_LOADER_TYPE       equ   0x71
LINUX_HEAP_END_OFFSET        equ   (0x9000 - 0x200)

LINUX_BZIMAGE_ADDR           equ   0x100000
LINUX_ZIMAGE_ADDR            equ   0x10000
LINUX_OLD_REAL_MODE_ADDR     equ   0x90000
LINUX_SETUP_STACK            equ   0x9000

LINUX_FLAG_BIG_KERNEL        equ   0x1

; Linux's video mode selection support. Actually I hate it!
LINUX_VID_MODE_NORMAL        equ   0xFFFF
LINUX_VID_MODE_EXTENDED      equ   0xFFFE
LINUX_VID_MODE_ASK           equ   0xFFFD

LINUX_CL_OFFSET              equ   0x9000
LINUX_CL_END_OFFSET          equ   0x90FF
LINUX_SETUP_MOVE_SIZE        equ   0x9100
LINUX_CL_MAGIC               equ   0xA33F
