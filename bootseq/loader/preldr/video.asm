;
; video.asm:
; screen output
;

name video

ifndef STAGE1_5

public   printmsg
public   printb
public   printw
public   printd
public   printhex2
public   printhex4
public   printhex8
public   printhex4
public   printhex2

endif

extrn    call_rm       :near

ifndef MICROFSD
extrn    stage0base   :dword
endif

.386

include fsd.inc

_TEXT16  segment dword public 'CODE'  use16

ifndef STAGE1_5

message:
        cld
        push    ds
        push    ebx
        push    esi
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

        pop     esi
        pop     ebx
        pop     ds

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

endif

_TEXT16  ends

_TEXT    segment dword public 'CODE'  use32


ifndef STAGE1_5

;
; void printmsg(char *);
;

printmsg proc near
        ;push    ebp
        ;mov     ebp, esp

        push    esi
        ;mov     esi, [ebp + 8]
        mov     esi, eax
        ; enter real mode
ifdef MICROFSD
        mov     eax, REL1_BASE
else
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:message
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        pop     esi

        ;pop     ebp

        ret
printmsg endp

;
; void printb(unsigned char);
;

printb  proc near
        ;push    ebp
        ;mov     ebp, esp

        push    esi
        ;mov     esi, [ebp + 8]
        mov     esi, eax
        ; enter real mode
ifdef MICROFSD
        mov     eax, REL1_BASE
else
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:printhex2
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4
        pop     esi

        ;pop     ebp

        ret
printb  endp

;
; void printw(unsigned short);
;

printw  proc near
        ;push    ebp
        ;mov     ebp, esp

        push    esi
        ;mov     esi, [ebp + 8]
        mov     esi, eax
        ; enter real mode
ifdef MICROFSD
        mov     eax, REL1_BASE
else
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:printhex4
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4
        pop     esi

        ;pop     ebp

        ret
printw  endp

;
; void printd(unsigned long);
;

printd  proc near
        ;push    ebp
        ;mov     ebp, esp

        push    esi
        ;mov     esi, [ebp + 8]
        mov     esi, eax
        ; enter real mode
ifdef MICROFSD
        mov     eax, REL1_BASE
else
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:printhex8
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4
        pop     esi

        ;pop     ebp

        ret
printd  endp

endif

_TEXT    ends

         end
