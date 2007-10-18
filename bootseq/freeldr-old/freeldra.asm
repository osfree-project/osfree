;
; freeldra.asm
;

.386p

extrn   printk:NEAR
extrn   auxil:NEAR, KernelLoader:NEAR
extrn   FindIODelay:NEAR, IODelay:NEAR, SetPITChip:NEAR, EnableA20Line:NEAR
extrn   DispNTS:NEAR
extrn   DebugNTS:NEAR
extrn   DispC:NEAR
extrn   mbi:BYTE
extrn   entry_addr:FAR
extrn   protected:FAR

public  go_protected

public  muOpen, muRead, muClose, muTerminate
public  Int12Value, Int1588Value
public  current_seg
public  loadhigh
public  gd_table
public  _freeldr_stack_bottom
public  _freeldr_stack_top


_DATA SEGMENT WORD PUBLIC 'DATA' USE16

STARTMSG          db '==> OS2LDR:FREELDRA.ASM Started',13,10,13,10
                  db '(C) 1999 David C. Zimmerli', 13,10
                  db '(C) 2003, 2005 Yuri Prokushev', 13,10
                  db '(C) 2005 Sascha Schmidt', 13,10,13,10
                  db 'This program is free software; you can redistribute it and/or modify', 13,10
                  db 'it under the terms of the GNU General Public License version 2 as published by', 13,10
                  db 'the Free Software Foundation;' ,13,10,13,10
                  db 0

ENDMSG            db 13,10,'==> OS2LDR:FREELDRA.ASM Finished',13,10,0

HelloMessage      db 'OS2LDR:FREELDRA.ASM BPB address: 0x%04x:0x%04x',13,10
                  db 'OS2LDR:FREELDRA.ASM FileTblPtr:  0x%04x:0x%04x Bootdr: 0x%x BootFl 0x%x',13,10
                  db 0

FTMsg1            db 'OS2LDR:FREELDRA.ASM Number of loaded files: %u',13,10
                  db 'OS2LDR:FREELDRA.ASM OS2LDR:   seg:0x%04x, size: %lu',13,10
                  db 'OS2LDR:FREELDRA.ASM MICROFSD: seg:0x%04x, size: %lu',13,10
                  db 'OS2LDR:FREELDRA.ASM MINIFSD:  seg:0x%04x, size: %lu',13,10
                  db 'OS2LDR:FREELDRA.ASM RIPLDATA: not supported yet',13,10
                  db 0

FTMessage         db 'OS2LDR:FREELDRA.ASM muOpen:   0x%04x:0x%04x',13,10
                  db 'OS2LDR:FREELDRA.ASM muRead:   0x%04x:0x%04x',13,10
                  db 'OS2LDR:FREELDRA.ASM muClose:  0x%04x:0x%04x',13,10
                  db 'OS2LDR:FREELDRA.ASM muTerm:   0x%04x:0x%04x',13,10
                  db 0

IODelayMsg        db 'OS2LDR:FREELDRA.ASM IO Delay count: 0x%04x',0
Int12Msg          db 'OS2LDR:FREELDRA.ASM Int 12h returns %u Kb',0
Int1588Msg        db 'OS2LDR:FREELDRA.ASM Int 15h/88h returns %u Kb',13,10,0

entry_msg         db 'loadhigh  (',0
args_msg          db '  buf     = 0x%08lX,',13,10
                  db '  chunk   = %d,',13,10
                  db '  readbuf = 0x%04X%04X',0
above_1mb_msg     db '  [above 1 mb]',0
exit_msg          db ')',0

int15h_msg        db '    srcdesc = <%04x, %04x, %04x, %04x>',13,10
                  db '    tgtdesc = <%04x, %04x, %04x, %04x>',13,10
                  db '    cx      = %u',13,10
                  db '    es:si   = %04x:%04x',0

gdtaddr_msg       db 'gdtr = <%04x:%08lx>',0

current_seg       dw 0h

FileTblPtr        dd 00000000h
BPBPtr            dd 00000000h
BootFlags         db 0
BootDrive         db 0

FileTable         dw 21 dup (0)
BPBTable          db 31 dup (0)

Int12Value        dw 0h
Int1588Value      dw 0h

; Ring0 DS access rights
DS_ACC_RING0      dw   00092h
; Ring0 CS access rights
CS_ACC_RING0      dw   0CF9Ah

;
; segment descriptor structure
;
desc struc
ds_limit          dw   ?
ds_baselo         dw   ?
ds_basehi1        db   ?
ds_acclo          db   ?
ds_acchi          db   ?
ds_basehi2        db   ?
desc ends

;
; GDT for loadhigh function
;
loadhigh_gdt      desc <>              ; place for BIOS descriptors
                  desc <>              ; place for BIOS descriptors
src_desc          desc <>              ; source desc
tgt_desc          desc <>              ; target desc
                  desc <>              ; CS desc
                  desc <>              ; SS desc

loadhigh_gdt_size equ (($ - loadhigh_gdt) / size desc)

;
; GDT for the final switch to PM
;
gd_table          label dword
                  dw 0, 0, 0, 0                                  ;                                    0h
                  dw 0FFFFh, 0000h, 9200h, 00CFh                 ; flat DS descriptor                 8h
                  dw 0FFFFh, 0000h, 9A00h, 00CFh                 ; flat CS descriptor                 10h
                  dw 0D000h, 0000h, 9603h, 00CFh                 ; SS: base = 30000h, limit -3000h    18h
                  dw 0FFFFh, 0000h, 9A01h, 0040h                 ; CS of 64k segment of loader        20h
                  dw 0FFFFh, 0000h, 9201h, 0040h                 ; DS of 64k segment of loader        28h
                  db 67h, 0, 0, 0, 0, 10001001b, 01000000b, 0    ; TSS                                30h

; Structure to load into GDTR
dtr struc
p_limit           dw 0                                           ; GDT limit
p_base            dd 0                                           ; GDT linear address
dtr ends

gd_reg            dtr <>

_DATA ENDS

FREELDR_STACK_SIZE equ 2AFCh           ; < 12 Kbytes

_TEXT           segment word  public 'CODE'     USE16
_TEXT           ends
_DATA           segment word  public 'DATA'     USE16
_DATA           ends
CONST           segment word  public 'DATA'     USE16
CONST           ends
CONST2          segment word  public 'DATA'     USE16
CONST2          ends
freeldrc13_DATA segment word  public 'FAR_DATA' use16
freeldrc13_DATA ends
common13_DATA   segment word  public 'FAR_DATA' use16
common13_DATA   ends

_TEXT32         segment word  public 'CODE'     USE32
_TEXT32         ends
_DATA32         segment word  public 'DATA'     USE32
_DATA32         ends

_BSS            segment word  public 'BSS'      USE16
_BSS            ends
_STACK          segment dword public 'STACK'    use16
_STACK          ends

;
; Stack is dword-aligned, so, buffer[] array is also
; dword-aligned (it depends on it). See load_image()
; routine in freeldrc.c
;
_STACK  segment
_freeldr_stack_bottom   db    FREELDR_STACK_SIZE dup (?)
_freeldr_stack_top      label byte
_STACK  ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_BSS
DGROUP32 group _TEXT32,_DATA32
assume  DS:DGROUP,CS:DGROUP,SS:DGROUP

_TEXT   SEGMENT WORD PUBLIC 'CODE' USE16

org 0h

; We starting here. First of all we need to store registers, which filled
; by pointers to required tables.
; ax,di        FileTblPtr       Pointer to current memory map (which file
;                               where placed)

start proc near
        ; First store the "File Table" pointer
        mov  ax, es
        mov  word ptr cs:FileTblPtr + 2, ax
        mov  word ptr cs:FileTblPtr, di

        ; Now store the BPB pointer
        mov  ax, ds
        mov  word ptr cs:BPBPtr + 2, ax
        mov  word ptr cs:BPBPtr, si

        ; Now store the boot drive and boot flags
        mov  cs:BootFlags, dh
        mov  cs:BootDrive, dl

        ; Set all segment regs equal to CS
        mov  ax, cs
        mov  ds, ax
        mov  es, ax

        mov  current_seg, ax

        cli
        mov  ss, ax
        mov  sp, offset DGROUP:_freeldr_stack_top
        ;mov  ax, 2000h
        ;mov  ss, ax
        ;mov  sp, 0FFFCh
        sti

        lea  si, ds:STARTMSG
        push si
        call printk
        add  si, 2

        call auxil ; calls C code

        ; Now display BPB ptr, File table ptr, boot drive, boot flags
        mov  al, BootFlags
        xor  ah, ah
        push ax

        mov  al, BootDrive
        xor  ah, ah
        push ax

        mov  dx, word ptr FileTblPtr+2      ; segment
        mov  ax, word ptr FileTblPtr        ; offset
        push ax
        push dx

        mov  dx, word ptr BPBPtr+2          ; segment
        mov  ax, word ptr BPBPtr            ; offset
        push ax
        push dx

        lea  dx, ds:HelloMessage
        push dx
        call printk
        add  sp, 0Eh

        ; Now copy contents of File Table
        push ds
        lea  di, FileTable
        lds  si, FileTblPtr
        mov  cx, 15h
        rep  movsw
        pop  ds

        ; Now copy contents of BPB
        push ds
        lea  di, BPBTable
        lds  si, BPBPtr
        mov  cx, 1fh
        rep  movsb
        pop  ds

        ; Display contents of File Table
;        push word ptr [FileTable + 10h]    ;
;        push word ptr [FileTable + 12h]    ; ripl data

        push dword ptr [FileTable + 10h]    ; minifsd len
        push word  ptr [FileTable + 0eh]    ; minifsd seg

        push dword ptr [FileTable + 0ah]    ; microfsd len
        push word  ptr [FileTable + 08h]    ; microfsd seg

        push dword ptr [FileTable + 04h]    ; os2ldr len
        push word  ptr [FileTable + 02h]    ; os2ldr seg

        push word  ptr [FileTable + 00h]    ; Number of files
;        push word ptr [FileTable + 02h]     ; flags

        lea dx,ds:FTMsg1
        push dx
        call printk
        add sp, 16h

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
        call printk
        add sp, 12h

        ; Now find and display the I/O delay value
        call FindIODelay

        push ax   ; the IO Delay
        lea dx,ds:IODelayMsg
        push dx
        call printk
        add sp, 4

        call SetPITChip

        ; OK, time to do the memory thing.
        int 12h
        mov Int12Value,ax
        push ax   ; the return value
        lea dx,ds:Int12Msg
        push dx
        call printk
        add sp, 4

        mov ah,88h
        int 15h
        mov Int1588Value,ax
        push ax   ; the return value
        lea dx,ds:Int1588Msg
        push dx
        call printk
        add sp, 4

        ; And now the moment you've all been waiting for: loading the kernel!!
        mov  bx, offset DGROUP:FileTable
        mov  ax, ds
        push ax
        push bx
        call KernelLoader
        add  sp, 4

        call IODelay  ; just for kicks

        ; Show message about end of game
        lea  si, ds:ENDMSG
        push si
        call printk
        add  sp, 2

        ; Switch to protected mode and call the kernel
        call go_protected

        ;;; Now terminate program
HltInst2:
        hlt
        jmp HltInst2

        ;;; END of Program !!
start endp


;
;  unsigned short __cdecl loadhigh(unsigned long buf,
;                                  unsigned short chunk,
;                                  unsigned long readbuf);
;
;  Copy chunk bytes from readbuf 0:32 address to
;  buf 0:32 address, which can be above 1 Megabyte
;
;  Stack state:
;
;  word  ptr [bp + 2]  - return address
;  dword ptr [bp + 4]  - buf
;  word  ptr [bp + 8]  - chunk
;  dword ptr [bp + 10] - readbuf
;
;  Note: chunk should be <= 32 Kb (a restriction of
;  function 87h of int 15h)
;

loadhigh proc near

        push bp
        mov  bp, sp

        push si
        push di
        push bx
        push cx
        push dx
        push ds
        push es

;        ; Signal about function entry
;        lea  ax, entry_msg
;        push ax
;        call printk
;        add  sp, 2

;        ; Print function arguments
;        mov  ax,   word ptr [bp + 10]  ; readbuf low word
;        push ax
;        mov  ax,   word ptr [bp + 12]  ; readbuf high  word
;        push ax
;        mov  ax,   word ptr [bp + 8]   ; chunk
;        push ax
;        mov  ax,   word ptr [bp + 6]   ; buf high word
;        push ax
;        mov  ax,   word ptr [bp + 4]   ; buf low  word
;        push ax
;        lea  ax, args_msg
;        push ax
;        call printk
;        add  sp, 12

        ; Choose two variants:
        ; below or above 1 Mb.
        cmp  dword ptr [bp + 4],  100000h
        jae  above_1mb
        cmp  dword ptr [bp + 10], 100000h
        jae  above_1mb

        ; buf and readbuf are below 1 Mb
        ; just copy contents of readbuf into buf
        ; as string

        ; Copy strings
        mov  eax, dword ptr [bp + 10]
        shr  eax, 4       ; Get segment from phys address
        mov  ds,  ax

        shl  eax, 4
        sub  eax, dword ptr [bp + 10]
        neg  eax
        mov  si,  ax


        mov  eax, dword ptr [bp + 4]
        shr  eax, 4       ; Get segment from phys address
        mov  es,  ax

        shl  eax, 4
        sub  eax, dword ptr [bp + 4]
        neg  eax
        mov  di,  ax


        mov  cx,  word ptr [bp + 8]


        cld
        rep  movsb

        ; success
        mov  ax, 0

        jmp  exit3
above_1mb:
        ;
        ; Above 1 Mb
        ;

        ; fill descriptors access rights
        mov  ax,  DS_ACC_RING0
        lea  bx,  loadhigh_gdt
        mov  cx,  loadhigh_gdt_size
lp1:
        mov  [bx].ds_acclo, al
        mov  [bx].ds_acchi, ah
        add  bx,  size desc
        loop short lp1

        ; set limits
        mov  cx,  word ptr [bp + 8]
        dec  cx
        or   cx,  1
        mov  ax,  cx     ; limits

        inc  cx
        jcxz is64k
        shr  cx,  1
        jmp  short lb1
is64k:
        mov  cx, 8000h
lb1:
        lea  bx,  src_desc
        mov  [bx].ds_limit, ax

        ; set src address
        mov  dx,  word ptr [bp + 10] ; low word
        mov  [bx].ds_baselo, dx
        mov  dx,  word ptr [bp + 12] ; high word
        mov  [bx].ds_basehi1, dl
        mov  [bx].ds_basehi2, dh

        lea  bx,  tgt_desc
        mov  [bx].ds_limit, ax

        ; set tgt address
        mov  dx,  word ptr [bp + 4]  ; low word
        mov  [bx].ds_baselo, dx
        mov  dx,  word ptr [bp + 6]  ; high word
        mov  [bx].ds_basehi1, dl
        mov  [bx].ds_basehi2, dh

        ; es:si --> loadhigh_gdt
        push ds
        pop  es
        ;mov  ax, seg loadhigh_gdt
        ;mov  es, ax
        lea  si, loadhigh_gdt

;        ; print int15h/87h function input data
;        push si
;        push es
;
;        push cx
;
;        mov  ax, word ptr [tgt_desc + 6]
;        push ax
;        mov  ax, word ptr [tgt_desc + 4]
;        push ax
;        mov  ax, word ptr [tgt_desc + 2]
;        push ax
;        mov  ax, word ptr [tgt_desc]
;        push ax
;
;        mov  ax, word ptr [src_desc + 6]
;        push ax
;        mov  ax, word ptr [src_desc + 4]
;        push ax
;        mov  ax, word ptr [src_desc + 2]
;        push ax
;        mov  ax, word ptr [src_desc]
;        push ax
;
;        lea  ax, int15h_msg
;        push ax
;
;        call printk
;
;        add  sp, 24

        ; call int 15h
        ; func number
        mov  ah, 87h
        ; save flags
        pushf
        int 15h

        jnc  exit1
        jmp  exit2
exit1:
        ; successful return
        mov  ax, 0
exit2:
        popf
exit3:
        pop  es
        pop  ds
        pop  dx
        pop  cx
        pop  bx
        pop  di
        pop  si

;        lea  ax, exit_msg
;        push ax
;        call printk
;        add  sp, 2

        pop  bp

        ret

loadhigh endp

;
; Move execution to L4 KickStart
;
go_protected proc near

        ; Disable interrupts
        cli

;        ; Set base interrupt vector to 0x20:
;        mov  al, 00010001b
;        out  0x20, al
;        mov  al, 0x20
;        out  0x21, al
;        mov  al, 00000100b
;        out  0x21, al
;        ; send EOI:
;        mov  al, 00000001b
;        out  0x21, al

        ; Enable A20 line
        call EnableA20Line

        lea  bx, gd_reg
        ; GDT limit
        mov  word ptr [bx].p_limit, 00FFh

        xor  eax, eax
        push ds
        pop  ax
        shl  eax, 4
        xor  edx, edx
        lea  dx,  gd_table
        add  eax, edx
        ; GDT linear address
        mov  [bx].p_base, eax

        ; Print GDT address
        push word ptr [bx + 4]
        push word ptr [bx + 2]
        push word ptr [bx]
        lea  ax, gdtaddr_msg
        push ax
        call printk
        add  sp, 8

        ; Load GDTR:
        lgdt fword ptr [bx]


        ; Zero-out flags:
        push 0002h
        popf

        ; Enable protected mode (PE bit in CR0 register):
        mov  eax, cr0
        or   al, 1
        mov  cr0, eax

        ; jmp 0x20:protected
        db   0eah                      ; jmp command opcode
        dw   offset DGROUP:protected   ; offset
        dw   20h                       ; selector
go_protected endp


;
; uFSD entry point wrappers:
;


; Open file using MicroFSD
;
; unsigned short __cdecl muOpen(char *fileName,
;                               unsigned long *fileSize);
;
; Note: this function accepts two NEAR (!) pointers, but converts them
; into FAR ones (adds DS segment) and passes to mu_Open() in blackbox.
muOpen proc near

         ; save a previous stack frame
         ; and create the new one
         push bp
         mov  bp, sp

         push ds                             ; fileSize segment
         push word ptr [bp + 6]              ; fileSize offset
         push ds                             ; fileName segment
         push word ptr [bp + 4]              ; fileName offset
         call dword ptr [FileTable + 1Ah]    ; !!!!!!!!!!!!!

         ; take off the func. params from stack
         add sp, 8

         ; take off the current stack frame
         ; and restore the previous one
         pop  bp

         ret
muOpen endp

; Read file using MicroFSD
;
; unsigned long __cdecl muRead(unsigned long seekOffset,
;                              unsigned char far *dataBuffer,
;                              unsigned long bufferSize);
;
muRead proc near

         ; save the previous stack frame
         ; and create a new one
         push bp
         mov  bp, sp

         push dword ptr [bp + 12]            ; bufferSize, low  word
         push dword ptr [bp + 8]             ; dataBuffer, offset
         push dword ptr [bp + 4]             ; seekOffset, low  word

         call dword ptr [FileTable + 1Eh]    ; !!!!!!!!!!!!!
         add sp, 12

         ; take off this stack frame
         ; and restore the previous one
         pop bp

         ret
muRead endp

; Close file using MicroFSD
;
; void __cdecl muClose(void)
;
muClose proc near
    call dword ptr [FileTable + 22h]
    ret
muClose endp

; Terminate MicroFSD
;
; void __cdecl muTerminate(void)
;
muTerminate proc near
   call dword ptr [FileTable + 26h]
   ret
muTerminate endp

_TEXT ENDS

      END start
