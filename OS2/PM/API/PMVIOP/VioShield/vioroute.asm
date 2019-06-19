.386
        .MODEL  FLAT

        PUBLIC  VioRouter
        PUBLIC  _SkipFlag

        EXTRN   _Entry32Main:NEAR
        EXTRN   DosSelToFlat:NEAR

STACK_SIZE  =   24*1024

CODE16          SEGMENT WORD PUBLIC USE16 'CODE'
                ASSUME  CS:CODE16

        ALIGN   2

; 16-bit entry point
VioRouter:
        push    ds
        push    es
        jmp far ptr FLAT:Entry32bit

        ALIGN   2
Return16bit:
        pop es
        pop ds
        retf
CODE16          ENDS


CODE32          SEGMENT DWORD PUBLIC USE32 'CODE'
                ASSUME  CS:FLAT,DS:FLAT

        ALIGN   4

; 32-bit entry point
Entry32bit:
        mov ax,seg DATA32
        mov ds,eax              ; ds = FLAT
        mov es,eax              ; es = FLAT

        cmp dword ptr [_SkipFlag], 0
        jz  continue
        mov ax, -1
        jmp far ptr CODE16:Return16bit

continue:
        mov word ptr [STACK16],sp       ; sp - lo word
        mov word ptr [STACK16+2],ss     ; ss - hi word

        mov ss,eax              ; }
        mov esp,offset TMP_STACK+STACK_SIZE ; } set new stack

        pushad                  ; save all regs

        mov eax,[STACK16]           ; 16:16 old stack
        call    DosSelToFlat
        push    eax             ; 32-bit flat old stack

; for now: ds=es=ss=FLAT DATA, esp=new 32 bit stack
; on stack - old 16-bit stack pointer.
; call pure 32-bit entry point
        call    _Entry32Main

; return to 16-bit code
Return32bit:
        add esp,4               ; C calling conv.

        mov [esp+7*4],eax           ; replace eax
        popad                   ; restore all regs

        lss sp,dword ptr [STACK16]      ; restore old ss:sp
        jmp far ptr CODE16:Return16bit
CODE32          ENDS


DATA32      SEGMENT DWORD PUBLIC USE32 'DATA'

        ALIGN   4
_SkipFlag       DD  0
STACK16         DD  ?
TMP_STACK   DB  STACK_SIZE DUP (0)

DATA32      ENDS


                END
