;
; start.asm:
; uFSD startup code
;

name start

extern  init:near
extern  _fs_name:dword

public  fsd_init

public   exe_end
;public   bss_end
;public   bss_start
;public   bss_len

public   _pdisk_read_func
public   _pdisk_read_hook
public   _pdevread
public   _prawread
public   _psubstring
public   _pgrub_memcmp
public   _pgrub_memmove
public   _pgrub_memset
public   _pgrub_strcmp
public   _pgrub_isspace
public   _pgrub_tolower
public   _pgrub_read
public   _pmem_lower
public   _pmem_upper

; filesystem common variables
public   _pfilepos
public   _pfilemax

; disk buffer parameters
public   _pbuf_drive
public   _pbuf_track
public   _pbuf_geom

public   _perrnum

public   _psaved_drive
public   _psaved_partition

public   _pcurrent_drive
public   _pcurrent_partition
public   _pcurrent_slice
public   _ppart_start
public   _ppart_length
public   _pfsmax

public   _pprint_possibilities
public   _pprint_a_completion
public   _pprintf

public start

include fsd.inc
;include fsname.inc

.386

_TEXT    segment dword public 'CODE'  use32
_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
align 4
exe_end:
_end1    ends
_end2    segment dword public 'BSS'   use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
align 4
bss_end:
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4

;lala     equ 'ext2fs'

_TEXT    segment dword public 'CODE'  use32
           org EXT_BUF_BASE
;
;           jmp  fsd_init
;           nop
;           nop
;           nop
;           ; return
;           db   0c3h
;           org EXT_BUF_BASE + 8h
;           ;
;modname    db mdl
;           org EXT_BUF_BASE + 20h
;           ; base of realmode part segment
;base       dd    0
;
;           org EXT_BUF_BASE + 40h
start:
fsd_init:
         jmp  real_start
_bss_start     dd (exe_end - EXT_BUF_BASE)
_bss_end       dd (bss_end - EXT_BUF_BASE)
fsname_ptr     dd _fs_name

           org EXT_BUF_BASE + 0x18
_pdisk_read_func dd 0
_pdisk_read_hook dd 0
_pdevread        dd 0
_prawread        dd 0
_psubstring      dd 0
_pgrub_memcmp    dd 0
_pgrub_memmove   dd 0
_pgrub_memset    dd 0
_pgrub_strcmp    dd 0
_pgrub_isspace   dd 0
_pgrub_tolower   dd 0
_pgrub_read      dd 0
_pmem_lower      dd 0
_pmem_upper      dd 0

; filesystem common variables
_pfilepos        dd 0
_pfilemax        dd 0

; disk buffer parameters
_pbuf_drive      dd 0
_pbuf_track      dd 0
_pbuf_geom       dd 0

_perrnum         dd 0

_psaved_drive    dd 0
_psaved_partition dd 0

_pcurrent_drive  dd 0
_pcurrent_partition dd 0
_pcurrent_slice  dd 0
_ppart_start     dd 0
_ppart_length    dd 0
_pfsmax          dd 0
_pprint_possibilities dd 0
_pprint_a_completion  dd 0
_pprintf         dd 0

real_start:
_TEXT    ends

         end fsd_init
