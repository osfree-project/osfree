;
; fsd.inc
;


BOOT_MAGIC   equ 0x2BADB002   ; extended boot info magic

MFS_BASE     equ 0x7c0        ; minifsd base

; GRUB takes 0x52200 - 0x7c860 for buffers
EXT_BUF_BASE equ 0x8E800      ; buffer for extensions (blackboxes) (size: 0x4000 bytes)
EXT2BUF_BASE equ 0x83800      ; second buffer (size: 0x4000 bytes)

BOOTSEC_BASE equ 0x86000      ; bootsector relocation base
STAGE0_BASE  equ 0x86200      ; Stage0 load base
UFSD_BASE    equ 0x8C000      ; boot drive uFSD save buffer base
LDR_BASE     equ 0x10000      ; os2ldr base

;PRELDR_SEG   equ 0x8100       ; the lowest segment address used by preldr
EXT2BUF_SEG  equ 0x8380       ;
EXT_BUF_SEG  equ 0x8E80       ; buffer for preldr extensions (blackboxes)

BOOTSEC_SEG  equ 0x8600       ; bootsector relocation segment
STAGE0_SEG   equ 0x8620       ; preldr segment
MFS_SEG      equ 0x7c         ; minifsd segment
LDR_SEG      equ 0x1000       ; os2ldr segment

;PRELDR_LEN   equ 0x3EE00      ;
STAGE0_LEN   equ (UFSD_BASE - STAGE0_BASE)
EXT_LEN      equ 0x2800       ; extension (blackbox) buffer size

FSYS_MAX     equ 0x200        ; max number of filesystems
