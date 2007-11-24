;
; fsd.inc
;


BOOT_MAGIC   equ 0x2BADB002   ; extended boot info magic

MFS_BASE     equ 0x7c0        ; minifsd base

; GRUB takes 0x52200 - 0x7c860 for buffers
EXT_BUF_BASE equ 0x80e00      ; buffer for extensions (blackboxes) (size: 0x4000 bytes)
EXT2BUF_BASE equ 0x7ce00      ; second buffer (size: 0x4000 bytes)

BOOTSEC_BASE equ 0x84E00      ; bootsector relocation base
STAGE0_BASE  equ 0x85000      ; Stage0 load base
UFSD_BASE    equ 0x8C000      ; boot drive uFSD save buffer base
LDR_BASE     equ 0x10000      ; os2ldr base

BOOTSEC_SEG  equ 0x84e0       ; bootsector relocation segment
STAGE0_SEG   equ 0x8500       ; preldr segment
EXT_BUF_SEG  equ 0x80e0       ; buffer for preldr extensions (blackboxes)
MFS_SEG      equ 0x7c         ; minifsd segment
LDR_SEG      equ 0x1000       ; os2ldr segment

STAGE0_LEN   equ (UFSD_BASE - STAGE0_BASE)
EXT_LEN      equ 0x4000       ; extension (blackbox) buffer size

FSYS_MAX     equ 0x200        ; max number of filesystems
