;
; start32.asm: Multiboot kernel startup 32-bit code
;

.386p

extrn mbi:BYTE
extrn entry_addr:DWORD
extrn current_seg:WORD

;public gd_table, gd_reg
public protected

_TEXT           segment word  public 'CODE'     USE16
_TEXT           ends
_DATA           segment word  public 'DATA'     USE16
_DATA           ends

_TEXT32         segment word  public 'CODE'     USE32
_TEXT32         ends
_DATA32         segment word  public 'DATA'     USE32
_DATA32         ends


DGROUP   group _TEXT,_DATA
DGROUP32 group _TEXT32,_DATA32
assume cs:DGROUP, ds:DGROUP

_TEXT32 SEGMENT WORD PUBLIC 'CODE' USE32
;=======================================================;
protected proc far

        ; Load selectors:
        ; set all segment registers
        ; (except for DS)
        ; to flat 4 Gig segments
        ; with the limit of 0FFFFFh
        ; and base 0h

        mov  ax, 28h                        ; 64k  DS segment of loader
        mov  ds, ax                         ;

;        mov  ax, 18h                        ; stack selector
;        mov  ss, ax                         ;
;        mov  esp, 30000h                    ; stack base

        mov  ax, 8h                         ; flat DS
        mov  es, ax                         ;
        mov  fs, ax                         ;
        mov  gs, ax                         ;
        mov  ss, ax                         ;

        mov  esp, 30000h                    ; stack base

        xor  ebx, ebx                       ;
        mov  bx,  current_seg               ;
        shl  ebx, 4                         ; Loader segment base
        mov  ldr_base, ebx                  ;

        ; Output string to video buffer
        lea  esi, pm_msg                     ; message
        mov  edi, 0B8000h                    ; Video buffer base
        mov  ecx, pm_msg_len                 ; message length
msg_move:
        mov  al, byte ptr [esi]
        mov  byte ptr es:[edi], al           ; symbol
        mov  byte ptr es:[edi + 1], 3        ; attribute
        inc  esi
        inc  edi
        inc  edi
        loop msg_move

;===========================================;
        ;
        ; Now we will start the kernel!
        ;

        ; Fill-in the jump address
        lea  ebx,  __kernel
        mov  word  ptr [ebx].a_sel, 10h     ; selector
        lea  edx,  entry_addr               ;
        mov  edx,  dword ptr [edx]          ; edx == entry_addr
        mov  dword ptr [ebx].a_offset, edx  ; offset

        ; pointer to the multiboot structure
        lea  ebx, mbi                       ; mbi offset from the loader beginning
        add  ebx, dword ptr ldr_base        ; mbi physical address

        ; Jump to the kernel
        lea  edx, __kernel                  ;
        add  edx, dword ptr ldr_base        ; __kernel phys address

        mov  ax,  8h                        ;
        mov  ds,  ax                        ; flat DS

        ; magic
        mov  eax, 2BADB002h

        ; jump to the kernel
        jmp  fword ptr [edx]

        ; We should not return here
hlt_inst:
        hlt
        jmp hlt_inst
protected endp

_TEXT32 ENDS

_DATA segment word public 'DATA' USE16

pm_msg       db    'Hello from protected mode!'
pm_msg_len   dd    $ - pm_msg

; 48-bit address structure
faddr struc
a_offset     dd    ?
a_sel        dw    ?
faddr ends

; Kernel 48-bit address (selector:offset)
__kernel     faddr <?,?>

; loader segment base
ldr_base     dd    ?

_DATA ends

        END
