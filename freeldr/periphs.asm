;
; periphs.asm
;

.286

PUBLIC FindIODelay,IODelay,SetPITChip,EnableA20Line

; .MODEL tiny
DGROUP GROUP _TEXT,_DATA
ASSUME DS:DGROUP,CS:DGROUP

; .CODE
_TEXT SEGMENT WORD PUBLIC 'CODE' USE16

; NB: The routines _FindIODelay and IODelay are taken from
; Frank van Gilluwe, *The Undocumented PC*, 2d. ed., pp 42-44.

FindIODelay proc near

    pushf
    push bx
    push dx
    push es

    mov  ax, ds
    mov  es, ax

    mov  al, 0
    mov  io_flag, al

    mov  ax, 8300h                  ; set wait interval
    mov  cx, 0
    mov  dx, 5000                   ; delay 5 ms, until wait flag

    mov  bx, offset DGROUP:io_flag  ; see TASM User's Guide pp 253 ff
    int  15h
    jc   io_failed_15

    dec  cx

io_delay_loop2:
    test BYTE PTR io_flag, 80h
    jnz  io_5ms_done
    jmp  short $ + 2
    loop io_delay_loop2

    mov  ax, 100                    ; got here if >>400MHz CPU, so use large value
    jmp  io_set

io_5ms_done:
    mov  ax, 0ffffh
    sub  ax, cx                     ; after this line, ax = # of times looped
    mov  bx, 1500
    xor  dx, dx
    div  bx
    cmp  ax, 0
    je   io_1_delay
    jmp  io_set

io_failed_15:
    or   ah, ah
    jz   io_exit

io_1_delay:
    mov  ax, 1
io_set:
    mov  io_count, ax
io_exit:
    pop  es
    pop  dx
    pop  bx
    popf

    ret

FindIODelay endp


IODelay proc near

    push cx

    mov  cx, io_count
io_delay_loop:
    loop io_delay_loop

    pop  cx
    ret

IODelay endp


SetPITChip proc near

    ; This routine sets counter 0 of the 8254 chip to interrupt
    ; in mode 2 18.2 times per second

    cli

    mov  al, 00110100b
    ; Bits 7-6: 00 : Select Counter (0-2)
    ; Bits 5-4: 11 : Command: (11) means R/W 7-0, then 15-8
    ; Bits 3-1: 010 : Mode: (010) means rate generator (divide by n)
    ; Bit 0: 0: BCD
    out  43h, al

    call IODelay

    ; In the next stanza we write 0 twice to port 40h.  This has the effect
    ; of writing the 16-bit word 0, which the PIT chip interprets as the
    ; value 64k.  Since the clock tick is 1.193180 MHz, the PIT chip will
    ; then cause an IRQ0 about every 56ms, or about 18.2 times per second.
    mov  al, 0
    out  40h, al
    call IODelay
    out  40h, al

    sti

    ret

SetPITChip endp


EnableA20Line proc near

    ; Enable A20 address line:
    push ax
    in   al, 0x92
    or   al, 2
    out  0x92, al
    pop  ax

    ret

EnableA20Line endp



_TEXT ENDS


;.DATA
_DATA SEGMENT WORD PUBLIC 'DATA' USE16
    io_count dw 0
    io_flag  db 0
_DATA ENDS

      END
