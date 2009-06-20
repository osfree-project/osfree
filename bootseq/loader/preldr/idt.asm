;
; trap screen,
; exception handlers
; and IDT initialization
;

;
; Idea of this trap screen got from GrindarsOS
; sources, (c) Grindars and Andrey Valyaev
; aka Dron
;

name trapscreen

public idt_init
public idt_initted

.386p

include fsd.inc
include struc.inc

_TEXT   segment dword public 'CODE' use32

VIDEO_BUF       equ     0b8000h         ; video buffer address
ATTR            equ     5               ; chars color for TRAP screen
SCR_ROWS        equ     25
SCR_COLS        equ     80

regs_save struc
_sslim          dd      ?
_ssbase         dd      ?
_ssacc          dd      ?
_gslim          dd      ?
_gsbase         dd      ?
_gsacc          dd      ?
_fslim          dd      ?
_fsbase         dd      ?
_fsacc          dd      ?
_eslim          dd      ?
_esbase         dd      ?
_esacc          dd      ?
_dslim          dd      ?
_dsbase         dd      ?
_dsacc          dd      ?
_cslim          dd      ?
_csbase         dd      ?
_csacc          dd      ?
_edi            dd      ?
_esi            dd      ?
_ebp            dd      ?
_esp            dd      ?
_ebx            dd      ?
_edx            dd      ?
_ecx            dd      ?
_eax            dd      ?
_ss             dd      ?
_gs             dd      ?
_fs             dd      ?
_es             dd      ?
_ds             dd      ?
_trapno         dd      ?
_errcode        dd      ?
_eip            dd      ?
_cs             dd      ?
_eflags         dd      ?
_stk            dd      ?
regs_save ends


;
; exception handlers which are
; not given an error code.
;

handlers_start:

trap_0:
        push  0h
        push  00h
        jmp   go_on

trap_1:
        push  0h
        push  01h
        jmp   go_on

trap_2:
        push  0h
        push  02h
        jmp   go_on

trap_3:
        push  0h
        push  03h
        jmp   go_on

trap_4:
        push  0h
        push  04h
        jmp   go_on

trap_5:
        push  0h
        push  05h
        jmp   go_on

trap_6:
        push  0h
        push  06h
        jmp   go_on

trap_7:
        push  0h
        push  07h
        jmp   go_on

trap_9:
        push  0h
        push  09h
        jmp   go_on

trap_f:
        push  0h
        push  0fh
        jmp   go_on

trap_10:
        push  0h
        push  10h
        jmp   go_on

trap_12:
        push  0h
        push  12h
        jmp   go_on

trap_13:
        push  0h
        push  13h
        jmp   go_on

trap_14:
        push  0h
        push  14h
        jmp   go_on

trap_15:
        push  0h
        push  15h
        jmp   go_on

trap_16:
        push  0h
        push  16h
        jmp   go_on

trap_17:
        push  0h
        push  17h
        jmp   go_on

trap_18:
        push  0h
        push  18h
        jmp   go_on

trap_19:
        push  0h
        push  19h
        jmp   go_on

trap_1a:
        push  0h
        push  1ah
        jmp   go_on

trap_1b:
        push  0h
        push  1bh
        jmp   go_on

trap_1c:
        push  0h
        push  1ch
        jmp   go_on

trap_1d:
        push  0h
        push  1dh
        jmp   go_on

trap_1e:
        push  0h
        push  1eh
        jmp   go_on

trap_1f:
        push  0h
        push  1fh
        jmp   go_on



;
; exception handlers which are
; given an error code
;

trap_8:
        push  08h
        jmp   go_on

trap_a:
        push  0ah
        jmp   go_on

trap_b:
        push  0bh
        jmp   go_on

trap_c:
        push  0ch
        jmp   go_on

trap_d:
        push  0dh
        jmp   go_on

trap_e:
        push  0eh
        jmp   go_on

trap_11:
        push  11h

go_on:
        push  ds
        push  es
        push  fs
        push  gs
        push  ss
        pushad

        mov   esi, esp

        sub   esp, 3 * 6 * 4

        mov   ebp, esp
        add   [ebp]._esp, _stk - _ss

        mov   ebx, GDT_ADDR
        xor   eax, eax

        mov   ax, cs
        call  get_gdt_desc

        mov   ax, ds
        call  get_gdt_desc

        mov   ax, es
        call  get_gdt_desc

        mov   ax, fs
        call  get_gdt_desc

        mov   ax, gs
        call  get_gdt_desc

        mov   ax, ss
        call  get_gdt_desc

        call  show_trap_screen

        add   esp, 3 * 6 * 4    ; segment descriptors

        popad
        pop   ss
        pop   gs
        pop   fs
        pop   es
        pop   ds

        add   esp, 8    ; take off errcode and trap number

        iret

;
; Input: ebx == GDT base, eax = GDT selector,
; esi --> where in the stack to put info from
; GDT descriptors
;
get_gdt_desc:
        push  ebx
        push  eax

        and   eax, 0fff8h
        add   ebx, eax  ; get descriptor address

        ; access
        xor   eax, eax
        mov   al, byte ptr [ebx].ds_acclo
        mov   ah, byte ptr [ebx].ds_acchi
        sub   esi, 4
        mov   [esi], eax

        ; base
        mov   al, byte ptr [ebx].ds_basehi1
        mov   ah, byte ptr [ebx].ds_basehi2
        shl   eax, 16
        mov   ax, word ptr [ebx].ds_baselo
        sub   esi, 4
        mov   [esi], eax

        ; limit
        xor   eax, eax
        mov   ax, word ptr [ebx].ds_limit       ; limit two low bytes
        rol   eax, 16
        mov   al, byte ptr [ebx].ds_acchi       ; limit high nibble
        mov   cl, al
        shr   cl, 7     ; Granularity bit in CL
        ror   eax, 16

        cmp   cl, 0
        jz    no_mul

        ; if granularity bit is set, then
        ; multiply the limit by page size
        inc   eax
        shl   eax, 12
        dec   eax
no_mul:
        sub   esi, 4
        mov   [esi], eax

        pop   eax
        pop   ebx

        ret


show_trap_screen:

        ; clear screen
        cld
        mov   ax, 10h
        mov   ds, ax
        mov   es, ax
        xor   eax, eax
        mov   al, ' '
        mov   ah, 7
        mov   edi, VIDEO_BUF
        mov   ecx, SCR_COLS * SCR_ROWS * 2 / 4
        rep   stosd

        ; show trap screen
        lea   esi, trap_screen  ; from where to take symbols
        mov   edi, VIDEO_BUF    ; to where output chars

next_char:
        mov   ah, char_color    ; attribute
        lodsb                   ; load one symbol
        cmp   al, 0
        jz    halt_system       ; if the end of line
        cmp   al, '%'
        jnz   print_char
        call  process_format_char
        jmp   next_char
print_char:
        stosw
        jmp   next_char

halt_system:
        cli
        hlt
        ;ret


process_format_char:
        lodsb
char_b:
        ; print byte
        cmp   al, 'b'
        jnz   char_w
        xor   eax, eax
        lodsb
        mov   al, byte ptr [ebp + eax]
        call  print_byte
        jmp   char_exit

char_w:
        ; print word
        cmp   al, 'w'
        jnz   char_d
        xor   eax, eax
        lodsb
        mov   ax, word ptr [ebp + eax]
        call  print_word
        jmp   char_exit

char_d:
        ; print dword
        cmp   al, 'd'
        jnz   char_n
        xor   eax, eax
        lodsb
        mov   eax, dword ptr [ebp + eax]
        call  print_dword
        jmp   char_exit

char_n:
        ; output CR LF
        cmp   al, 'n'
        jnz   char_c
        mov   eax, edi
        add   eax, SCR_COLS * 2
        sub   eax, VIDEO_BUF
        mov   dl, SCR_COLS * 2
        div   dl
        mul   dl
        add   eax, VIDEO_BUF
        mov   edi, eax
        jmp   char_exit

char_c:
        ; set color
        cmp   al, 'c'
        jnz   char_percent
        lodsb
        mov   char_color, al
        jmp   char_exit

char_percent:
        ; print literal '%' symbol
        cmp   al, '%'
        jnz   char_exit
        ;lodsb
        mov   ah, char_color
        stosw

char_exit:
        ret


print_dword:
        push  eax
        ror   eax, 16
        and   eax, 0ffffh
        call  print_word

        pop   eax
        and   eax, 0ffffh
        call  print_word

        ret


print_word:
        push  eax
        ror   eax, 8
        and   eax, 0ffh
        call  print_byte

        pop   eax
        and   eax, 0ffh
        call  print_byte

        ret


print_byte:
        push  eax
        ror   eax, 4
        and   eax, 0fh
        call  print_nibble

        pop   eax
        and   eax, 0fh
        call  print_nibble

        ret


print_nibble:
        cmp   al, 0ah
        jb    dec_digit

hex_letter:
        add   al, 'a' - 0ah
        jmp   got_ascii

dec_digit:
        add   al, '0'

got_ascii:
        mov   ah, char_color
        stosw

        ret


idt_init:
        pushad

        cld
        mov   ecx, 20h
        mov   edi, IDT_ADDR     ; IDT start
        ;xor   edx, edx         ; zero selector
        lea   ebx, trap_handlers
hloop:
        movzx eax, byte ptr [ebx]
        add   eax, offset _TEXT:handlers_start
        call  set_trap_handler

        inc   ebx
        loop  hloop

        mov   al, byte ptr idt_initted
        cmp   al, 0
        jnz   initted

        mov   eax, IDTR_OLD
        sidt  fword ptr [eax]

        mov   byte ptr idt_initted, 1
initted:
        mov   eax, IDTR
        mov   [eax].g_limit, 20h * 8 - 1
        mov   [eax].g_base, IDT_ADDR
        lidt  fword ptr [eax]

        popad

        ret


;
; Input:
; eax -- trap handler address
;
set_trap_handler:
        push  eax

        ;and   eax, 0ffffh
        rol   eax, 16
        mov   ax, 8h    ; code selector
        ror   eax, 16

        ; store low IDT descriptor dword
        stosd

        pop   eax

        xor   al, al
        mov   ah, 8eh   ; 32-bit interrupt gate with ring0 privileges, present

        ; store high IDT descriptor dword
        stosd

        ret


_TEXT   ends


_DATA   segment dword public 'DATA' use32

;
; trap screen template
;
trap_screen:
        db      'A pre-loader fatal error: trap %b',_trapno,'%n'
        db      '%n'
        db      '  error code=%d',_errcode,'%n'
        db      '  cs:eip=%w',_cs,':%d',_eip,' ss:esp=%w',_ss,':%d',_esp,'%n'
        db      '%n'
        db      '  eflags=%d',_eflags,'%n'
        db      '%n'
        db      '  eax=%d',_eax,' ebx=%d',_ebx,' ecx=%d',_ecx,' edx=%d',_edx,'%n'
        db      '  esi=%d',_esi,' edi=%d',_edi,' esp=%d',_esp,' ebp=%d',_ebp,'%n'
        db      '%n'
        db      '  cs=%w',_cs,' csbase=%d',_csbase,' cslim=%d',_cslim,' csacc=%w',_csacc,'%n'
        db      '  ds=%w',_ds,' dsbase=%d',_dsbase,' dslim=%d',_dslim,' dsacc=%w',_dsacc,'%n'
        db      '  es=%w',_es,' esbase=%d',_esbase,' eslim=%d',_eslim,' esacc=%w',_esacc,'%n'
        db      '  fs=%w',_fs,' fsbase=%d',_fsbase,' fslim=%d',_fslim,' fsacc=%w',_fsacc,'%n'
        db      '  gs=%w',_gs,' gsbase=%d',_gsbase,' gslim=%d',_gslim,' gsacc=%w',_gsacc,'%n'
        db      '  ss=%w',_ss,' ssbase=%d',_ssbase,' sslim=%d',_sslim,' ssacc=%w',_ssacc,'%n'
        db      '%n',0

;
; trap handlers offset table
;
trap_handlers:
        db      (trap_0  - handlers_start)
        db      (trap_1  - handlers_start)
        db      (trap_2  - handlers_start)
        db      (trap_3  - handlers_start)
        db      (trap_4  - handlers_start)
        db      (trap_5  - handlers_start)
        db      (trap_6  - handlers_start)
        db      (trap_7  - handlers_start)
        db      (trap_8  - handlers_start)
        db      (trap_9  - handlers_start)
        db      (trap_a  - handlers_start)
        db      (trap_b  - handlers_start)
        db      (trap_c  - handlers_start)
        db      (trap_d  - handlers_start)
        db      (trap_e  - handlers_start)
        db      (trap_f  - handlers_start)
        db      (trap_10 - handlers_start)
        db      (trap_11 - handlers_start)
        db      (trap_12 - handlers_start)
        db      (trap_13 - handlers_start)
        db      (trap_14 - handlers_start)
        db      (trap_15 - handlers_start)
        db      (trap_16 - handlers_start)
        db      (trap_17 - handlers_start)
        db      (trap_18 - handlers_start)
        db      (trap_19 - handlers_start)
        db      (trap_1a - handlers_start)
        db      (trap_1b - handlers_start)
        db      (trap_1c - handlers_start)
        db      (trap_1d - handlers_start)
        db      (trap_1e - handlers_start)
        db      (trap_1f - handlers_start)
;
; current color of symbols
;
char_color      db      ATTR

; A flag which defines, that IDT is initted or not
idt_initted     db      0

_DATA   ends

        end
