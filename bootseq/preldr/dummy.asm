;
; dummy.asm
;

.386p

public   __CHK
public   __I8LS

_TEXT    segment byte public 'CODE'  use32
__CHK:
__I8LS:
         ret 4
_TEXT    ends

         end
