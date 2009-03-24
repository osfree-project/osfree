; $Id: xms_2e.asm 1276 2006-09-04 19:39:46Z blairdude $
;	Int2e handler for XMS Swap
;	variant
; int process_input(int xflag, char *commandline)

if 1

_TEXT segment word public 'CODE' use16

public lowlevel_int_2e_handler_
    lowlevel_int_2e_handler_:
        mov ax, 0FFFFh
        iret

_TEXT ends

else

_TEXT segment word public 'CODE' use16

extrn _residentCS, _mySS, _mySP, XMSdriverAdress, SwapTransientSize, SwapResidentSize :word
extrn _my2e_parsecommandline, SWAPXMSdirection :near
extrn XMSsave :far

callXMS                EQU   call far ptr [cs:XMSdriverAdress]
currentSegmOfFreeCOM   EQU   XMSsave+8

    public myfar2e_parsecommandline
myfar2e_parsecommandline:
    call _my2e_parsecommandline
    retf

	public lowlevel_int_2e_handler_
lowlevel_int_2e_handler_:
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
        mov bx, word ptr [cs:SwapTransientSize]
        int 21h
        jc swaperr
        mov bx,ax
        sub bx, word ptr [cs:currentSegmOfFreeCOM]
        push bx
        mov word ptr [cs:currentSegmOfFreeCOM], ax
        call SWAPXMSdirection
        mov ah,0bh
        mov si, XMSsave
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
        mov  si, XMSsave
        callXMS
        call SWAPXMSdirection
        pop  ax
endif
        jmp finish
swaperr:
        mov ax,0FFFFh
finish:
        iret

_TEXT ends

endif

      end
