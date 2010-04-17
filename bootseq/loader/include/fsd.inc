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
OS2LDR_BASE     equ 0x10000               ; os2ldr base
LDR_BASE        equ 0x20000               ; freeldr base

MFS_SEG         equ 0x7c                  ; minifsd segment
OS2LDR_SEG      equ 0x1000                ; os2ldr segment
LDR_SEG         equ 0x2000                ; freeldr segment

; multiboot kernel load base
KERN_BASE       equ 0x400000
REL_BASE        equ 0x90000
REL1_BASE       equ (0x90000 + SHIFT)

PREFERRED_BASE  equ 0x5c000               ; preferred load base (addresses 0x6f000-0x7f000 occupied by pre-loader
                                          ; when loaded initially without relocation)
BOOT_PART_TABLE equ (0x5d000 + SHIFT)     ;
FSYS_BUFLEN     equ 0x8000

;
; buffers at the end of 3rd megabyte
;
PRELDR_BASE       equ 0x3d0000            ; final location of a pre-loader
FSYS_BUF          equ 0x3e0000            ;
EXT1HIBUF_BASE    equ 0x3e8000            ; blackbox high memory buffer1
EXT2HIBUF_BASE    equ 0x3ec000            ; blackbox high memory buffer2
EXT3HIBUF_BASE    equ 0x3f0000            ; blackbox high memory buffer3
EXT4HIBUF_BASE    equ 0x3f4000
UFSD_BASE         equ 0x3e8000            ; EXT1HIBUF_BASE ; boot drive uFSD save buffer base
TERM_BASE         equ 0x3ec000            ; EXT3HIBUF_BASE

NEW_STACK_SP    equ 0x3fe000              ; stack
STACKLEN        equ 0x4000

;STACK_SP        equ (0x64800 + SHIFT)     ; stack
;
;EXT1LOBUF_BASE  equ 0x64400               ; blackbox low memory buffer1
;EXT2LOBUF_BASE  equ 0x64800               ; blackbox low memory buffer2
;EXT3LOBUF_BASE  equ 0x64400               ; blackbox low memory buffer3
;TERMLO_BASE     equ 0x64800               ; terminal blackbox base (low)
;BOOTSEC_BASE    equ (0x64c00 + SHIFT)     ; bootsector relocation base
;STAGE0_BASE     equ (0x64e00 + SHIFT)     ; Stage0 load base
;EXT_BUF_BASE    equ (0x6c000 + SHIFT)     ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

STACK_SP        equ (0x63800 + SHIFT)     ; stack

EXT1LOBUF_BASE  equ 0x63400               ; blackbox low memory buffer1
EXT2LOBUF_BASE  equ 0x63800               ; blackbox low memory buffer2
EXT3LOBUF_BASE  equ 0x63c00               ; blackbox low memory buffer3
TERMLO_BASE     equ 0x63800               ; terminal blackbox base (low)
BOOTSEC_BASE    equ (0x63c00 + SHIFT)     ; bootsector relocation base
STAGE0_BASE     equ (0x63e00 + SHIFT)     ; Stage0 load base
EXT_BUF_BASE    equ (0x73e00 + SHIFT)     ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

SCRATCHADDR     equ (0x5c000 + SHIFT)     ; 512-byte scratch area
BUFFERADDR      equ SCRATCHADDR + 0x200   ;
BUFFERLEN       equ 0xe00                 ; 0x7e00

EXT_LEN         equ 0x3000                ; extension (blackbox) buffer size
EXTLO_LEN       equ 0x400                 ; low portion
EXT2LEN         equ 0x5000
STAGE0_LEN      equ (EXT_BUF_BASE - STAGE0_BASE)

FSYS_MAX        equ 0x20                  ; max number of filesystems
BOOT_MAGIC      equ 0x2BADB002            ; extended boot info magic

; Protected mode stack address
PROTSTACK         equ  0x500
; Real mode stack address
RMSTACK           equ  0x504
; protect mode IDT save
;IDTR              equ  0x508
; real mode IDT save
;IDTR_OLD          equ  0x510

; GDT location
;GDT_ADDR          equ 0x520
; IDT location
;IDT_ADDR          equ 0x600

; stacks initial values
PROTSTACKINIT   equ STACK_SP - 10h
REALSTACKINIT   equ STAGE0_LEN
