;
; fsd.inc
;


BOOT_MAGIC   equ 0x2BADB002   ; extended boot info magic

MFS_BASE     equ 0x7c0        ; minifsd base

UFSD_BASE    equ 0xb9000           ; !!! 0x8C800  ; boot drive uFSD save buffer base
EXT2BUF_BASE equ 0xbb800           ; !!!          ; second buffer (size: 0x4000 bytes)

BOOTSEC_BASE equ (0x87000 - SHIFT) ; 0x86000      ; bootsector relocation base
STAGE0_BASE  equ (0x87200 - SHIFT) ; 0x86200      ; Stage0 load base
EXT_BUF_BASE equ (0x8c800 - SHIFT) ; 0x81800      ; buffer for extensions (blackboxes) (size: 0x2800 bytes)

LDR_BASE     equ 0x10000      ; os2ldr base

MFS_SEG      equ 0x7c         ; minifsd segment
LDR_SEG      equ 0x1000       ; os2ldr segment

STAGE0_LEN   equ (EXT_BUF_BASE - STAGE0_BASE)
EXT_LEN      equ 0x2800       ; extension (blackbox) buffer size

STACK_SP     equ 0x7f000      ; !!! 0x88000  ; 0x8c800      ; 0x52200     ; stack
STACKLEN     equ 0xc00

SCRATCHADDR  equ 0xbe000      ; !!! 0x86000  ; 0x84000      ; 512-byte scratch area
BUFFERADDR   equ SCRATCHADDR + 0x200
BUFFERLEN    equ 0xe00        ; 0x7e00

BOOT_PART_TABLE equ 0x600     ;
FSYS_BUFLEN     equ 0x8000
FSYS_BUF        equ (0x7f000 - SHIFT)  ; BUFFERADDR + BUFFERLEN

FSYS_MAX        equ 0x20      ; max number of filesystems
