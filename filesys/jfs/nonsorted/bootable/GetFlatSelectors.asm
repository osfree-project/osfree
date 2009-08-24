; $Id: GetFlatSelectors.asm,v 1.5 2002/06/24 23:15:53 bird Exp $
;
; Gets the flag DS and CS from the Dostables.
;
; Copyright (c) 2002 knut st. osmundsen (bird@anduin.net)
;
; GPL
;

    .286


;
;   Header Files
;
include devhlp.inc


;
;   Externals
;
extrn  _Device_Help:far


;
;   Global Variables
;
DATA16 segment
public  FlatR0DS
FlatR0DS    dw  ?
public  FlatR0CS
FlatR0CS    dw  ?
DATA16 ends



CODE16 segment

;;
; Gets the Ring 0 Flat DS and CS.
; Places these in FlatR0DS and FlatR0CS.
; @cproto   int cdecl GetFlatSelectors(void)
; @returns  0 on success, FlatR0DS and FlatR0CS is set.
;           -1 on error, FlatR0DS and FlatR0CS is undefined.
; @uses     ax
; @sketch
;   Get the dostable far pointer.
;   Skip the first table.
;   Move ds and cs to the global vars.
;
; @status   completely implemented.
; @author   knut st. osmundsen (bird@anduin.net)
GetFlatSelectors proc near
    ASSUME  ds:DATA16, ss:NOTHING, es:NOTHING
    ; prolog.
    push    bp
    mov     bp, sp

    ;
    ; Save registers. (paranoia)
    ;
    push    es
    push    ds
    push    bx
    push    cx
    push    dx


    ;
    ; Get the DosTables
    ;
    xor     cx, cx
    mov     al, 9                       ; Undocumented by IBM, gets the dostables.
    mov     dl, DevHlp_GetDOSVar
    call far ptr [_Device_Help]
    jnc     ok
    mov     ax, 0ffffh                  ; just set some value which isn't 0.
    jmp     return
ok:

    ;
    ; Now we have to skip the first table cause the good stuff is
    ; in the second one.
    ; The tables have an byte field at the start giving it's length in
    ; number of dwords.
    ;
    mov     es, ax                      ; now es:bx points to the first dos table.
    xor     ax, ax
    mov     al, es:[bx]                 ; Read the number of DWORDs in the table
    shl     al, 2                       ; Calculate the end of the table.
    inc     al                          ; (+1 for the char at start of the table)
    add     bx, ax                      ; now es:bx points to the second dos table.

    inc     bx                          ; skip the leading dword count.
    mov     ax, es:[bx+(9*4)]           ; R0FlatCS
    mov     FlatR0CS, ax
    mov     ax, es:[bx+(10*4)]          ; R0FlatDS
    mov     FlatR0DS, ax

    ;
    ; Success!
    ;
    xor     ax, ax

    ;
    ; Restore registers and return.
    ;
return:
    pop     dx
    pop     cx
    pop     bx
    pop     ds
    pop     es
    leave
    ret
GetFlatSelectors endp

CODE16 ends


end
