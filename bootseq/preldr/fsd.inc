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

TERM_BASE       equ (0x72000 + SHIFT)     ; terminal blackbox base
BOOTSEC_BASE    equ (0x75000 + SHIFT)     ; bootsector relocation base
STAGE0_BASE     equ (0x75200 + SHIFT)     ; Stage0 load base
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

