;
; vbe.asm:
; vbe control functions
;

name vbe

public reset_vbe_mode
public get_vbe_controller_info
public get_vbe_mode_info
public set_vbe_mode
public get_vbe_pmif

extrn  call_rm          :near
extrn  stage0base      :dword

.386p

include fsd.inc

_TEXT16  segment dword public 'CODE'  use16

get_vbe_controller_info_rm:
        mov     es, bx
        mov     ax, 4F00h
        int     10h

        mov     bx, ax

        retf

get_vbe_mode_info_rm:
        mov     es, bx
        mov     ax, 4F01h
        int     10h

        mov     bx, ax

        retf

set_vbe_mode_rm:
        mov     ax, 4F02h
        int     10h

        mov     bx, ax

        retf

reset_vbe_mode_rm:
        mov     ax, 0003h
        int     10h

        retf

get_vbe_pmif_rm:
        mov     ax, 4F0Ah
        xor     bx, bx
        xor     di, di
        int     10h

        xor     ebx, ebx
        cmp     ax, 004Fh
        jnz     nopm

        mov     bx, es
        shl     ebx, 16
        mov     bx, di
nopm:
        retf

_TEXT16  ends

_TEXT    segment dword public 'CODE'  use32
;
;   int get_vbe_controller_info (struct vbe_controller *controller_ptr)
;
;   Get VBE controller information.
;

get_vbe_controller_info:
        push    ebp
        mov     ebp, esp

        push    esi
        push    edi
        push    ebx

        ; Convert the linear address to segment:offset
        mov     eax, [ebp + 8]
        mov     edi, eax
        and     edi, 0000000fh
        shr     eax, 4
        mov     ebx, eax

        ; enter real mode
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_vbe_controller_info_rm
        push    eax
        call    call_rm
        add     esp, 4

        movzx   eax, bx

        pop     ebx
        pop     edi
        pop     esi
        pop     ebp

        ret


;
;   int get_vbe_mode_info (int mode_number, struct vbe_mode *mode_ptr)
;
;   Get VBE mode information.
;
get_vbe_mode_info:
        push    ebp
        mov     ebp, esp

        push    esi
        push    edi
        push    ebx

        ; Convert the linear address to segment:offset
        mov     eax, [ebp + 0ch]
        mov     edi, eax
        and     edi, 0000000fh
        shr     eax, 4
        mov     ebx, eax

        ; Save the mode number in %cx
        mov     ecx, [ebp + 8]

        ; enter real mode
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_vbe_mode_info_rm
        push    eax
        call    call_rm
        add     esp, 4

        movzx   eax, bx

        pop     ebx
        pop     edi
        pop     esi
        pop     ebp

        ret


;
;   int set_vbe_mode (int mode_number)
;
;   Set VBE mode. Don't support user-specified CRTC information.
;
set_vbe_mode:
        push    ebp
        mov     ebp, esp

        push    ebx
        push    esi
        push    edi

        ; Save the mode number in %bx
        mov     ebx, [ebp + 8]
        ; Clear bit D11
        and     ebx, 0F7FFh

        ; enter real mode
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl     eax, 12
        mov     ax,  offset _TEXT16:set_vbe_mode_rm
        push    eax
        call    call_rm
        add     esp, 4

        movzx   eax, bx

        pop     edi
        pop     esi
        pop     ebx
        pop     ebp

        ret

reset_vbe_mode:
        push    ebp
        mov     ebp, esp
        push    ebx

        ; enter real mode
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl     eax, 12
        mov     ax,  offset _TEXT16:reset_vbe_mode_rm
        push    eax
        call    call_rm
        add     esp, 4

        pop     ebx
        pop     ebp

        ret


get_vbe_pmif:
        push    ebp
        mov     ebp, esp

        push    ebx
        push    esi
        push    edi

        push    ebp

        pop     ebp

        mov     eax, [ebp + 8]
        mov     [eax], ebx
        mov     eax, [ebp + 0ch]
        and     ecx, 0FFFFh
        mov     [eax], ecx

        pop     edi
        pop     esi
        pop     ebx
        pop     ebp

        ret

_TEXT   ends

        end

