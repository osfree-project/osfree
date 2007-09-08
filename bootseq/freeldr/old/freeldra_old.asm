; freeldra.asm

.386P

pic_mask equ 21h
pic_eoi equ 20h

com_data equ 00h
com_ier equ 01h
com_iir equ 02h
com_lcr equ 03h
com_mcr equ 04h
com_sts equ 05h   ;;; line status register
com_msr equ 06h

EXTRN _printf:NEAR
EXTRN _auxil:NEAR, _KernelLoader:NEAR
EXTRN _FindIODelay:NEAR,_IODelay:NEAR,_SetPITChip:NEAR,_EnableA20Line:NEAR
PUBLIC _DispNTS, _DebugNTS, _DispC, muOpen_, muRead_, muClose_, muTerminate_


; .MODEL small
DGROUP GROUP _TEXT,_DATA
;ASSUME DS:DGROUP,CS:DGROUP

; .CODE
_TEXT SEGMENT WORD PUBLIC 'CODE' USE16

org 0h

; We starting here. First of all we need to store registers, which filled
; by pointers to required tables.
; ax,di        FileTblPtr       Pointer to current memory map (which file
;                               where placed)

start:
        ; First store the "File Table" pointer
        mov ax, es
        mov word ptr cs:FileTblPtr+2,ax
        mov word ptr cs:FileTblPtr,di

        ; Now store the BPB pointer
        mov ax, ds
        mov word ptr cs:BPBPtr+2,ax
        mov word ptr cs:BPBPtr,si

        ; Now store the boot drive and boot flags
        mov cs:BootFlags,dh
        mov cs:BootDrive,dl

        ; Set all segment regs equal to CS
        mov ax, cs
        mov ds, ax
        mov es, ax

        cli
        mov ss, ax
        mov sp, 0a342h ; FIX THIS!! What should it be, really?
        sti

        lea si, ds:STARTMSG
        push si
        call _DispNTS

        call _auxil ; calls C code

        ; Now display BPB ptr, File table ptr, boot drive, boot flags
        mov al,BootFlags
        xor ah,ah
        push ax
        mov al,BootDrive
        xor ah,ah
        push ax
        mov dx,word ptr FileTblPtr+2 ; segment
        mov ax,word ptr FileTblPtr   ; offset
        push ax
        push dx
        mov dx,word ptr BPBPtr+2 ; segment
        mov ax,word ptr BPBPtr   ; offset
        push ax
        push dx
        lea dx,ds:HelloMessage
        push dx
        call _printf
        add sp,0Eh

        ; Now copy contents of File Table
        push ds
        lea di,FileTable
        lds si,FileTblPtr
        mov cx,0015h
        rep movsw
        pop ds

        ; Now copy contents of BPB
        push ds
        lea di,BPBTable
        lds si,BPBPtr
        mov cx,001fh
        rep movsb
        pop ds

        ; Display contents of File Table

        push word ptr [FileTable + 10h]
        push word ptr [FileTable + 12h]

        push word ptr [FileTable + 0ch]
        push word ptr [FileTable + 0eh]

        push word ptr [FileTable + 08h]
        push word ptr [FileTable + 0ah]

        push word ptr [FileTable + 04h]
        push word ptr [FileTable + 06h]

        push word ptr [FileTable + 00h]
        push word ptr [FileTable + 02h]

        lea dx,ds:FTMsg1
        push dx
        call _printf
        add sp,16h

        ; Display micro-FSD entry points
        push word ptr [FileTable + 26h]
        push word ptr [FileTable + 28h] ; terminate

        push word ptr [FileTable + 22h]
        push word ptr [FileTable + 24h] ; close

        push word ptr [FileTable + 1Eh]
        push word ptr [FileTable + 20h] ; read

        push word ptr [FileTable + 1Ah]
        push word ptr [FileTable + 1Ch] ; open

        lea dx,ds:FTMessage
        push dx
        call _printf
        add sp,12h


        ; Now find and display the I/O delay value
        call _FindIODelay

        push ax   ; the IO Delay
        lea dx,ds:IODelayMsg
        push dx
        call _printf
        add sp,4 ; 6


        call _SetPITChip

        ; OK, time to do the memory thing.
        int 12h
        mov Int12Value,ax
        push ax   ; the return value
        lea dx,ds:Int12Msg
        push dx
        call _printf
        add sp,4 ;6

        mov ah,88h
        int 15h
        mov Int1588Value,ax
        push ax   ; the return value
        lea dx,ds:Int1588Msg
        push dx
        call _printf
        add sp,4 ;6

        ; And now the moment you've all been waiting for: loading the kernel!!
        mov bx,offset DGROUP:FileTable
        mov ax,ds
        push ax
        push bx
        call _KernelLoader
        add sp,4

        call _IODelay  ; just for kicks

        ;;; Now terminate program

        ;Show message about end of game
        lea si, ds:ENDMSG
        push si
        call _DispNTS

; Move execution to L4 KickStart

cli     ;Отключение прерываний

        ;Установка базового вектора прерывания в 0x20:
mov al,00010001b
out 0x20,al
mov al,0x20
out 0x21,al
mov al,00000100b
out 0x21,al
        ;ручной EOI:
mov al,00000001b
out 0x21,al

        call _EnableA20Line

        ;Загрузка глобальной таблицы дескрипторов (GDT):
lgdt [gd_reg]

        ;Обнуление регистра флагов:
push byte 2
popf

        ;Включение защищенного режима (бит PE в регистре CR0):
mov eax, cr0
or al, 1
mov cr0, eax

HltInst2:
        hlt
        jmp HltInst2

         ;Загрузка селектора сегмента кода в CS путем длинного прыжка
         ;в 32-битный сегмент

;jmp 16:_protected

jmp _protected



        ;;; END of Program !!

_DispNTS proc near   ; display null-terminated string at ds:si on screen
        push bp
        mov bp,sp

        mov si,[bp+4]

Disp1:  mov al,[si]
        cmp al,0
        je Disp2
        mov ah, 0eh  ; int 10h/ah=0eh: write char on screen in teletype mode
        mov bx,0
        push si
        int 10h
        pop si
        inc si
        jmp Disp1
Disp2:
        pop bp
        ret

_DispNTS endp

_DispC proc near   ; display char at ds:si on screen
        push bp
        mov bp,sp
        mov si,[bp+4]
        mov al,[si]
        mov ah, 0eh  ; int 10h/ah=0eh: write char on screen in teletype mode
        mov bx,0
        push si
        int 10h
        pop si
        pop bp
        ret

_DispC endp

_DebugNTS proc near ; display null-terminated string at ds:si on COM port
        push bp
        mov bp,sp

        mov si,[bp+4]

Disp3:  mov al,[si]
        cmp al,0
        je Disp4
        push si

        push ax
        call _com_out
        pop ax

        pop si
        inc si
        jmp Disp3
Disp4:
        pop bp
        ret

_DebugNTS endp


; Comm routines are courtesy of Ray Duncan, *Advanced MS-DOS Programming*,
; 2d. ed., pp. 113-125
;;;;;;;;; void _com_out(int);
_com_out     proc near
    push bp
    mov bp,sp

    mov ax,[bp+4]
    push dx
    push ax
    mov dx,sp_base
    add dx,com_sts  ;;; line status register (base reg + 05h)
co1:
    in al,dx
    and al,20h   ;; is transmit buffer empty?
    jz co1       ;; no, must wait

    pop ax
    mov dx,sp_base
    add dx,com_data   ;;; transmitter holding register (base reg + 0h)
    out dx,al
    pop dx

    pop bp
    ret
_com_out endp


;;;;;;;;;  void asc_enb() ; port, brd, lcr are HARDCODED in this version
_asc_enb     proc near
    push bp
    mov bp,sp
    ;; here we could subtract something from sp for local vars.

    mov ax,2   ;; the port # (1-4)  -- HARDCODE 2 (COM2) for now
    mov port,al
    cmp al,1
    je port1
    cmp al,2
    je port2
    cmp al,3
    je port3

    ;; So al=4
    mov WORD PTR sp_base, 02e8h
    mov BYTE PTR com_int,0bh
    mov BYTE PTR int_mask,08h
    jmp portx

port1:
    mov WORD PTR sp_base, 03f8h
    mov BYTE PTR com_int,0ch
    mov BYTE PTR int_mask,10h
    jmp portx

port2:
    mov WORD PTR sp_base, 02f8h
    mov BYTE PTR com_int,0bh
    mov BYTE PTR int_mask,08h
    jmp portx

port3:
    mov WORD PTR sp_base, 03e8h
    mov BYTE PTR com_int,0ch
    mov BYTE PTR int_mask,10h

portx:

    ;; Now set the baud rate.
    ;; (See Wyatt, _Advanced Assembly Language_, p. 81)
    mov dx, sp_base
    add dx, com_lcr  ;; line control register
    in al,dx
    or al,10000000b  ;; set bit 7 to enable alternate registers
    out dx,al

    mov ax,0ch ;; the brd  ; HARDCODE 0ch for 9600 baud
    mov dx,sp_base
    add dx,com_data  ;; or BRDL
    out dx,al

    mov al,ah
    mov dx,sp_base
    add dx,com_ier  ;; or BRDH
    out dx,al

    mov dx, sp_base
    add dx, com_lcr  ;; line control register
    in al,dx
    and al,01111111b  ;; clear bit 7 to enable normal registers
    out dx,al

    ;; Now set the data format, e.g. N81, E71, etc.
    mov ax,3 ;; the lcr  ; HARDCODE 3 for "N81"
    and al,01111111b  ;; make sure bit 7 is off
    out dx,al   ;; Here, DX already points to Line Control Register

    ;; Now set DTR/RTS and enable interrupts.
    mov dx,sp_base
    add dx,com_mcr    ;;; modem control register (02fch)
    mov al,0bh        ;;; 1011b -- set DTR, RTS, and OUT2 bits
    out dx,al
    mov dx,sp_base
    add dx,com_ier    ;;; interrupt enable register (02f9h)
    mov al,1          ;;; set "data available in RDR" interrupt
    out dx,al
    in al,pic_mask         ;;; read current 8259 mask  (pic_mask is 21h)
    mov dl,int_mask    ;;; int_mask is 08h for COM2
    not dl
    and al, dl
    out pic_mask,al

    ;; here we would add back to sp whatever we subtracted for local vars
    pop bp
    ret
_asc_enb    endp


;int __cdecl muOpen(char far * fileName, unsigned long int far * fileSize);
muOpen_ proc far   ; Open file using MicroFSD
;    pop ax ; pop arguments
;    pop bx
;    pop cx
;    pop dx

;    push cx
;    push dx

;    push ax
;    push bx ; push in reverse order (Pascal style)

;    push bp ; store bp

;        lea si, ds:STARTMSG
;        push si
;        call _DispNTS

    jmp [FileTable + 1Ah] ; call MSD_OPEN of MicroFSD

;    call _DispNTS

;    pop bp ; restore bp
;        ret

muOpen_ endp

;unsigned long int __cdecl muRead(unsigned long int far seekOffset,
;                        unsigned char far * dataBuffer,
;                        unsigned long int far bufferSize);
muRead_ proc far   ; Read file using MicroFSD

    pop ax ; pop arguments
    pop bx
    pop cx
    pop dx
    mov word ptr cs:tmp1, ax
    mov word ptr cs:tmp2, bx
    pop ax
    pop bx

    push bx ; push in reverse order (Pascal style)
    push ax
    push dx
    push cx
    mov ax, word ptr cs:tmp1
    mov bx, word ptr cs:tmp2
    push bx
    push ax

    push bp

    lea si, ds:STARTMSG
        push si
        call _DispNTS

        call [FileTable + 1Eh]
    pop bp
        ret

muRead_ endp

muClose_ proc far   ; Close file using MicroFSD
    call [FileTable + 22h]
    ret

muClose_ endp

muTerminate_ proc far   ; Open file using MicroFSD
    call [FileTable + 26h]
        ret

muTerminate_ endp


_TEXT ENDS

_TEXT32 SEGMENT WORD PUBLIC 'CODE32' USE32
;=======================================================;
_protected:
;Этот код исполняется в защищенном режиме


;Загрузка сегментных регистров нужными селекторами:
mov ax, 1000b
mov es, ax
mov ds, ax
mov ss, ax

mov [$], byte 0

;mov [0B8000h], byte 'S' ; Вякнем в левый верний угол экрана

xor eax, eax
xor ecx, ecx

;===========================================;

;jmp 16:0x200000        ;Переход на ядро TODO указать адрес KickStart
HltInst:
        hlt
        jmp HltInst

;       movl    $0x2BADB002, %eax
;       movl    0x8(%esp), %ebx
;
;       /* boot kernel here (absolute address call) */
;       call    *0x4(%esp)
;
;       /* error */
;       call    EXT_C(stop)


_TEXT32 ENDS


;.DATA
_DATA SEGMENT WORD PUBLIC 'DATA' USE16

;        EXTRN    _Repetitions:WORD        ;externally defined
;        PUBLIC   _StartingValue           ;available to other modules
_StartingValue    DW 0
STARTMSG          DB '==> OS2LDR:FREELDRA.ASM Started',13,10
                  DB '(C) 1999 David C. Zimmerli', 13,10
                  DB '(C) 2003, 2005 Yuri Prokushev', 13,10
                  DB '(C) 2005 Sascha Schmidt', 13,10,13,10
                  DB 'This program is free software; you can redistribute it and/or modify', 13,10
                  DB 'it under the terms of the GNU General Public License version 2 as published by', 13,10
                  DB 'the Free Software Foundation;' ,13,10,13,10
                  DB 0,0
ENDMSG            DB '==> OS2LDR:FREELDRA.ASM Finished',13,10,0,0
HelloMessage      DB 'OS2LDR:FREELDRA.ASM BPB address: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM FileTblPtr: %h:%h Bootdr: %h BootFl %h',13,10
                  DB 0,0
FTMsg1            DB 'OS2LDR:FREELDRA.ASM Number of loaded files: %h',13,10
                  DB 'OS2LDR:FREELDRA.ASM OS2LDR: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM MICROFSD: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM MINIFSD: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM RIPLDATA: not supported yet',13,10
                  DB 0,0
FTMessage         DB 'OS2LDR:FREELDRA.ASM muOpen: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM muRead: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM muClose: %h:%h',13,10
                  DB 'OS2LDR:FREELDRA.ASM muTerm: %h:%h',13,10
                  DB 0,0
IODelayMsg        DB 'OS2LDR:FREELDRA.ASM IO Delay count: %hh',13,10,0,0
Int12Msg          DB 'OS2LDR:FREELDRA.ASM Int 12h returns %hh',13,10,0,0
Int1588Msg        DB 'OS2LDR:FREELDRA.ASM Int 15h/88h returns %hh',13,10,0,0
MsgBuff DB 200 dup (32)

tmp1  db 0000h
tmp2  db 0000h

port db 02h         ; COM2 by default
sp_base dw 02f8h    ; serial port base address (02f8h = COM2)
com_int db 0bh  ;; IRQ vector (IRQ3 = 0bh, IRQ4 = 0ch)
int_mask db 08h ; mask for IRQ3 is 08h, for IRQ4 it's 10h

FileTblPtr dd 00000000h
BPBPtr dd 00000000h
BootFlags db 0
BootDrive db 0

FileTable dw 21 dup (0)
BPBTable db 31 dup (0)

Int12Value dw 0h
Int1588Value dw 0h

DummyTable dw 10 dup (0)

gd_table:       ;Глобальная таблица дескрипторов
dw 0xFFFF,0x0000,0x9200,0x00CF  ;Дескриптор данных
dw 0xFFFF,0x0000,0x9A00,0x00CF  ;Дескриптор кода
db 0x67, 0,0,0,0 ,10001001b, 01000000b, 0   ;сегмент состояния задачи

gd_reg:
dw 255  ;Лимит GDT
dd gd_table-8   ;Линейный адрес GDT

_DATA ENDS

      END



