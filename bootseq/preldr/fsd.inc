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
BOOT_PART_TABLE equ (0x70000 + SHIFT)     ;
FSYS_BUFLEN     equ 0x8000

;
; GDT location
;
GDT_ADDR          equ 0x680

;
; buffers at the end of 3rd megabyte
;
FSYS_BUF          equ 0x3e0000            ;
EXT1HIBUF_BASE    equ 0x3e8000            ; blackbox high memory buffer1
EXT2HIBUF_BASE    equ 0x3ec000            ; blackbox high memory buffer2
EXT3HIBUF_BASE    equ 0x3f0000            ; blackbox high memory buffer3
EXT4HIBUF_BASE    equ 0x3f4000
UFSD_BASE         equ 0x3e8000            ; EXT1HIBUF_BASE ; boot drive uFSD save buffer base
TERM_BASE         equ 0x3ec000            ; EXT3HIBUF_BASE

NEW_STACK_SP    equ 0x3fe000              ; stack
STACKLEN        equ 0x4000

;STACK_SP        equ (0x74800 + SHIFT)     ; stack
;
;EXT1LOBUF_BASE  equ 0x74400               ; blackbox low memory buffer1
;EXT2LOBUF_BASE  equ 0x74800               ; blackbox low memory buffer2
;EXT3LOBUF_BASE  equ 0x74400               ; blackbox low memory buffer3
;TERMLO_BASE     equ 0x74800               ; terminal blackbox base (low)
;BOOTSEC_BASE    equ (0x74c00 + SHIFT)     ; bootsector relocation base
;STAGE0_BASE     equ (0x74e00 + SHIFT)     ; Stage0 load base
;EXT_BUF_BASE    equ (0x7c000 + SHIFT)     ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

STACK_SP        equ (0x73800 + SHIFT)     ; stack

EXT1LOBUF_BASE  equ 0x73400               ; blackbox low memory buffer1
EXT2LOBUF_BASE  equ 0x73800               ; blackbox low memory buffer2
EXT3LOBUF_BASE  equ 0x73c00               ; blackbox low memory buffer3
TERMLO_BASE     equ 0x73800               ; terminal blackbox base (low)
BOOTSEC_BASE    equ (0x73c00 + SHIFT)     ; bootsector relocation base
STAGE0_BASE     equ (0x73e00 + SHIFT)     ; Stage0 load base
EXT_BUF_BASE    equ (0x7c000 + SHIFT)     ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

SCRATCHADDR     equ (0x6f000 + SHIFT)     ; 512-byte scratch area
BUFFERADDR      equ SCRATCHADDR + 0x200   ;
BUFFERLEN       equ 0xe00                ; 0x7e00

EXT_LEN         equ 0x3000                ; extension (blackbox) buffer size
EXTLO_LEN       equ 0x400                 ; low portion
EXT2LEN         equ 0x5000
STAGE0_LEN      equ (EXT_BUF_BASE - STAGE0_BASE)
;STAGE0_LEN      equ (0x7f000 + SHIFT - STAGE0_BASE)

FSYS_MAX        equ 0x20                  ; max number of filesystems
BOOT_MAGIC      equ 0x2BADB002            ; extended boot info magic

; Protected mode stack address
PROTSTACK         equ  0x600
; Real mode stack address
RMSTACK           equ  0x604

; stacks initial values
PROTSTACKINIT   equ STACK_SP - 10h
REALSTACKINIT   equ STAGE0_LEN
