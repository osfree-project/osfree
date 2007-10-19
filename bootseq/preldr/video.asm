;
; video.asm:
; screen output
;

name video

public   message
public   printmsg
public   printb
public   printw
public   printd
extrn    call_rm       :near

.386

include fsd.inc

_TEXT16  segment byte public 'CODE'  use16

message:
        ; convert FLAT pointer in esi
        ; to far pointer in ds:si
        mov     eax, esi
        mov     bx,  ax
        and     bx,  0fh
        mov     si,  bx
        shr     eax, 4
        mov     ds,  ax

        jmp     short mess1
up1:
        mov     bx, 1
        mov     ah, 0Eh
        int     10h             ; display a byte

mess1:
        lodsb
        or      al, al
        jne     up1              ; if not end of string, jmp to display
        retf

;
; printhex[248]: Write a hex number in (AL, AX, EAX) to the console
;

printhex2:
        pusha
        mov     ax, si
        rol     eax, 24
        mov     cx, 2
        jmp     pp1
printhex4:
        pusha
        mov     ax, si
        rol     eax, 16
        mov     cx, 4
        jmp     pp1
printhex8:
        pusha
        mov     eax, esi
        mov     cx, 8
pp1:
        rol     eax, 4
        push    eax
        and     al, 0Fh
        cmp     al, 10
        jae     high1
low1:
        add     al, '0'
        jmp     pp2
high1:
        add     al, 'A' - 10
pp2:
        mov     bx, 0001h
        mov     ah, 0Eh
        int     10h              ; display a char
        pop     eax
        loop    pp1
        popa

        retf

_TEXT16  ends

_TEXT    segment byte public 'CODE'  use32

;
; void __cdecl printmsg(char *);
;

printmsg proc near
        push ebp
        mov  ebp, esp

        mov     esi, [ebp + 8]
        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:message
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        pop  ebp

        ret
printmsg endp

;
; void __cdecl printb(unsigned char);
;

printb  proc near
        push ebp
        mov  ebp, esp

        mov  esi, [ebp + 8]
        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:printhex2
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        pop  ebp

        ret
printb  endp

;
; void __cdecl printw(unsigned short);
;

printw  proc near
        push ebp
        mov  ebp, esp

        mov  esi, [ebp + 8]
        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:printhex4
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        pop  ebp

        ret
printw  endp

;
; void __cdecl printd(unsigned long);
;

printd  proc near
        push ebp
        mov  ebp, esp

        mov  esi, [ebp + 8]
        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:printhex8
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        pop  ebp

        ret
printd  endp


_TEXT    ends

         end
