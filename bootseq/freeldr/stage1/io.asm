;
; io.asm: (I/O routines)
;

.286

.model tiny

public DispNTS
public DebugNTS
public DispC

pic_mask  equ 21h
pic_eoi   equ 20h

com_data  equ 00h
com_ier   equ 01h
com_iir   equ 02h
com_lcr   equ 03h
com_mcr   equ 04h
com_sts   equ 05h        ;;; line status register
com_msr   equ 06h

_TEXT segment word public 'CODE' USE16

;
; display null-terminated string at ds:si on screen
;
DispNTS proc near
        push bp
        mov  bp, sp

        mov  si, [bp + 4]

Disp1:  mov  al, [si]
        cmp  al, 0
        je   Disp2
        mov  ah, 0eh     ; int 10h/ah=0eh: write char on screen in teletype mode
        mov  bx, 0
        push si
        int  10h
        pop  si
        inc  si
        jmp  Disp1
Disp2:
        pop  bp
        ret

DispNTS endp

;
; display char at ds:si on screen
;
DispC proc near

        push bp
        mov  bp, sp

        mov  si, [bp + 4]
        mov  al, [si]
        mov  ah, 0eh     ; int 10h/ah=0eh: write char on screen in teletype mode
        mov  bx, 0
        push si
        int  10h
        pop  si

        pop  bp

        ret

DispC endp

;
; display null-terminated string at ds:si on COM port
;
DebugNTS proc near
        push bp
        mov  bp, sp

        mov  si, [bp + 4]

Disp3:  mov  al, [si]
        cmp  al, 0
        je   Disp4
        push si

        push ax
        call com_out
        pop  ax

        pop  si
        inc  si
        jmp  Disp3
Disp4:
        pop  bp
        
        ret

DebugNTS endp


; Comm routines are courtesy of Ray Duncan, *Advanced MS-DOS Programming*,
; 2d. ed., pp. 113-125
;;;;;;;;; void com_out(int);
com_out     proc near

    push bp
    mov  bp, sp

    mov  ax, [bp + 4]
    push dx
    push ax
    mov  dx, sp_base
    add  dx, com_sts     ;; line status register (base reg + 05h)
co1:
    in   al, dx
    and  al, 20h         ;; is transmit buffer empty?
    jz   co1             ;; no, must wait

    pop  ax
    mov  dx, sp_base
    add  dx, com_data    ;; transmitter holding register (base reg + 0h)
    out  dx, al
    pop  dx

    pop  bp
    
    ret
    
com_out endp


;
; void asc_enb() ; port, brd, lcr are HARDCODED in this version
;
asc_enb     proc near

    push bp
    mov  bp, sp
    ;; here we could subtract something from sp for local vars.

    mov  ax, 2           ;; the port # (1-4)  -- HARDCODE 2 (COM2) for now
    mov  port, al
    cmp  al, 1
    je   port1
    cmp  al, 2
    je   port2
    cmp  al, 3
    je   port3

    ;; So al=4
    mov  WORD PTR sp_base,  02e8h
    mov  BYTE PTR com_int,  0bh
    mov  BYTE PTR int_mask, 08h
    jmp  portx

port1:
    mov  WORD PTR sp_base,  03f8h
    mov  BYTE PTR com_int,  0ch
    mov  BYTE PTR int_mask, 10h
    jmp  portx

port2:
    mov  WORD PTR sp_base,  02f8h
    mov  BYTE PTR com_int,  0bh
    mov  BYTE PTR int_mask, 08h
    jmp  portx

port3:
    mov  WORD PTR sp_base,  03e8h
    mov  BYTE PTR com_int,  0ch
    mov  BYTE PTR int_mask, 10h

portx:

    ;; Now set the baud rate.
    ;; (See Wyatt, _Advanced Assembly Language_, p. 81)
    mov  dx, sp_base
    add  dx, com_lcr     ;; line control register
    in   al, dx
    or   al, 10000000b   ;; set bit 7 to enable alternate registers
    out  dx, al

    mov  ax, 0ch         ;; the brd  ; HARDCODE 0ch for 9600 baud
    mov  dx, sp_base
    add  dx, com_data    ;; or BRDL
    out  dx, al

    mov  al, ah
    mov  dx, sp_base
    add  dx, com_ier     ;; or BRDH
    out  dx, al

    mov  dx, sp_base
    add  dx, com_lcr     ;; line control register
    in   al, dx
    and  al, 01111111b   ;; clear bit 7 to enable normal registers
    out  dx, al

    ;; Now set the data format, e.g. N81, E71, etc.
    mov  ax, 3           ;; the lcr  ; HARDCODE 3 for "N81"
    and  al, 01111111b   ;; make sure bit 7 is off
    out  dx, al          ;; Here, DX already points to Line Control Register

    ;; Now set DTR/RTS and enable interrupts.
    mov  dx, sp_base
    add  dx, com_mcr     ;;; modem control register (02fch)
    mov  al, 0bh         ;;; 1011b -- set DTR, RTS, and OUT2 bits
    out  dx, al
    mov  dx, sp_base
    add  dx, com_ier     ;;; interrupt enable register (02f9h)
    mov  al, 1           ;;; set "data available in RDR" interrupt
    out  dx, al
    in   al, pic_mask    ;;; read current 8259 mask  (pic_mask is 21h)
    mov  dl, int_mask    ;;; int_mask is 08h for COM2
    not  dl
    and  al, dl
    out  pic_mask, al

    ;; here we would add back to sp whatever we subtracted for local vars
    pop  bp

    ret
    
asc_enb    endp

_TEXT ends

_DATA segment word public USE16

port db 02h              ; COM2 by default
sp_base dw 02f8h         ; serial port base address (02f8h = COM2)
com_int db 0bh           ; IRQ vector (IRQ3 = 0bh, IRQ4 = 0ch)
int_mask db 08h          ; mask for IRQ3 is 08h, for IRQ4 it's 10h

_DATA ends

      END
