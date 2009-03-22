; $Id: xms_2e.asm 1276 2006-09-04 19:39:46Z blairdude $
;	Int2e handler for XMS Swap
;	variant
; int process_input(int xflag, char *commandline)

if 1

TEXT segment word public 'CODE' use16

public _lowlevel_int_2e_handler
    _lowlevel_int_2e_handler:
        mov ax, 0FFFFh
        iret

TEXT ends

else

TEXT segment word public 'CODE' use16

extrn _residentCS, _mySS, _mySP, _XMSsave, _XMSdriverAdress, _SwapTransientSize, _SwapResidentSize :word
extrn _my2e_parsecommandline, SWAPXMSdirection :near

callXMS                EQU   call far ptr [cs:_XMSdriverAdress]
currentSegmOfFreeCOM   EQU   _XMSsave+8

    public myfar2e_parsecommandline
myfar2e_parsecommandline:
    call _my2e_parsecommandline
    retf

	public _lowlevel_int_2e_handler
_lowlevel_int_2e_handler:
        cld
        push si
;       lss sp, word ptr [_mySP]
        mov ss, word ptr [cs:_mySS]
        mov sp, word ptr [cs:_mySP]
;       mov cs, word ptr [_residentCS]
;       mov ds,sp
;       mov es,ds
;       mov ds,ss
        mov ah,48h       ; move into conventional memory
        mov bx, word ptr [cs:_SwapTransientSize]
        int 21h
        jc swaperr
        mov bx,ax
        sub bx, word ptr [cs:currentSegmOfFreeCOM]
        push bx
        mov word ptr [cs:currentSegmOfFreeCOM], ax
        call SWAPXMSdirection
        mov ah,0bh
        mov si, _XMSsave
        callXMS
;        call SWAPXMSdirection
        pop bx
        cmp ax,1
        jnz swaperr
;       mov ax, word ptr [_mySS]
;       add ax, bx
;       mov ss, ax
;       mov sp, word ptr [_mySP]
if 0
        mov ax, word ptr [_mySS]
        sub ax, 10
        mov ss, ax
        mov sp, word ptr [_mySP]
endif
       ; calling stuff here
;       push es
        push ds
        pop si
        push si
        mov ds,sp
;       push word ptr 1
;       call _process_input
        call far ptr [myfar2e_parsecommandline]
if 0
        push ax
        mov  ah, 0bh       ; move into XMS again
        mov  si, _XMSsave
        callXMS
        call SWAPXMSdirection
        pop  ax
endif
        jmp finish
swaperr:
        mov ax,0FFFFh
finish:
        iret

TEXT ends

endif

      end
