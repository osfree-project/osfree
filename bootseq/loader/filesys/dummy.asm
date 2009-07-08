;
; dummy.asm
;

.386p

public   __CHK
;public   __I8LS
;public   __U8RS
;public   __U8LS
;public   __U8M

_TEXT    segment dword public 'CODE'  use32
__CHK:
         ret 4
;__I8LS:
;         ret
;__U8RS:
;         ret
;__U8LS:
;         ret
;__U8M:
;         ret
_TEXT    ends

         end
