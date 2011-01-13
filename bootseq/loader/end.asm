;
; end.asm:
; ending labels definition
;

.386p

public   exe_end
public   bss_end
public   exe_end_off
public   bss_end_off

_TEXT    segment dword public 'CODE'  use32
exe_end_off   dd (offset _TEXT:exe_end)
bss_end_off   dd (offset _TEXT:bss_end)
_TEXT    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
align 4
exe_end label dword
_end1    ends
_end2    segment dword public 'BSS'   use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
align 4
bss_end label dword
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends

         end
