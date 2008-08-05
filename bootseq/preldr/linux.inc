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
