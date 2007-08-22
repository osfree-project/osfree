;
; uFSD entry points wrappers
;

_TEXT segment

public stub_mu_Open
public stub_mu_Read
public stub_mu_Close
public stub_mu_Terminate

EXTRN mu_Open              : near      ; in open.c
EXTRN mu_Read              : near      ; in read.c
EXTRN mu_Close             : near      ; in close.c
EXTRN mu_Terminate         : near      ; in term.c

EXTRN _microfsd_stack_top : byte      ; in entry.asm

;***********************
;* mu_Open entry point *
;***********************
stub_mu_Open proc far
        push bp
        mov bp, sp

        push ds
        push es
        push cs
        pop ax
        mov ds, ax
        mov es, ax
.386p
        mov ax, ss
        mov word ptr [_microfsd_stack_top-2], ax
        mov word ptr [_microfsd_stack_top-4], sp

        mov eax, dword ptr [bp+10]
        mov dword ptr [_microfsd_stack_top-8], eax

        mov eax, dword ptr [bp+6]
        mov dword ptr [_microfsd_stack_top-12], eax

        push ds
        mov ax, offset DGROUP:_microfsd_stack_top - 12
        push ax
        lss sp, dword ptr [esp]

.286p
        call mu_Open
        add sp, 8
.386p
        lss sp, dword ptr [_microfsd_stack_top - 4]
.286p
        pop es
        pop ds
        leave
        retf
stub_mu_Open endp


;***********************
;* mu_Read entry point *
;***********************
stub_mu_Read proc far
        push bp
        mov bp, sp

        push ds
        push es
        push cs
        pop ax
        mov ds, ax
        mov es, ax
.386p
        ;
        ; Saves current ss:sp on top of microfsd_stack
        ;
        mov ax, ss
        mov word ptr [_microfsd_stack_top-2], ax
        mov word ptr [_microfsd_stack_top-4], sp

        ;
        ; Copies cbBuf on our new stack
        ;
        mov eax, dword ptr [bp+14]                      ; cbBuf
        mov dword ptr [_microfsd_stack_top-8], eax

        ;
        ; Copies pBuf on our new stack
        ;
        mov eax, dword ptr [bp+10]                      ; pBuf
        mov dword ptr [_microfsd_stack_top-12], eax

        ;
        ; Copies loffseek on our new stack
        ;
        mov eax, dword ptr [bp+6]                       ; loffseek
        mov dword ptr [_microfsd_stack_top-16], eax

        ;
        ; New ss:sp
        ;
        push ds
        mov ax, offset DGROUP:_microfsd_stack_top - 16
        push ax
        lss sp, dword ptr [esp]

.286p
        call mu_Read
        add sp, 12
.386p
        lss sp, dword ptr [_microfsd_stack_top - 4]
.286p
        pop es
        pop ds
        leave
        retf
stub_mu_Read endp

;************************
;* mu_Close entry point *
;************************
stub_mu_Close proc far
        push bp
        mov bp, sp

        push ds
        push es
        push cs
        pop ax
        mov ds, ax
        mov es, ax
.386p
        mov ax, ss
        mov word ptr [_microfsd_stack_top-2], ax
        mov word ptr [_microfsd_stack_top-4], sp


        push ds
        mov ax, offset DGROUP:_microfsd_stack_top - 4
        push ax
        lss sp, dword ptr [esp]

.286p
        call mu_Close
.386p
        lss sp, dword ptr [_microfsd_stack_top - 4]
.286p
        pop es
        pop ds
        leave
        retf
stub_mu_Close endp

;****************************
;* mu_Terminate entry point *
;****************************
stub_mu_Terminate proc far
        push bp
        mov bp, sp

        push ds
        push es
        push cs
        pop ax
        mov ds, ax
        mov es, ax
.386p
        mov ax, ss
        mov word ptr [_microfsd_stack_top-2], ax
        mov word ptr [_microfsd_stack_top-4], sp


        push ds
        mov ax, offset DGROUP:_microfsd_stack_top - 4
        push ax
        lss sp, dword ptr [esp]

.286p
        call mu_Terminate
.386p
        lss sp, dword ptr [_microfsd_stack_top - 4]
.286p
        pop es
        pop ds
        leave
        retf
stub_mu_Terminate endp

_TEXT ends

      end
