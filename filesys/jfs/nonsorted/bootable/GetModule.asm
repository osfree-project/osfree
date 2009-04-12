; $Id: GetModule.asm,v 1.8 2002/06/24 23:15:34 bird Exp $
;
; Finds a given loaded module.
;
; Copyright (c) 2002 knut st. osmundsen (bird@anduin.net)
;
; GPL
;

    .386


;
;   Header Files
;
include SegDefs.inc
include sas.inc
include ldr.inc


;
;   Externals
;
DATA16 segment
extrn  FlatR0DS:word
extrn  FlatR0CS:word
DATA16 ends



CODE16 segment


;;
; Gets a module searched by module name from the module list.
; @cproto   int cdecl GetModule(const char FAR *fpszModName, short cchModName, struct farptr48 FAR *fp48Mte)
; @returns  0 on success, *fp48Mte is set.
;           2 if not found, *fp48Mte is unchanged.
; @param    fpszModName     Far pointer to a modulename.
;                           This is uppercased and maximum of 8 chars long.
;                           The '\0' isn't strictly needed as cchModName is
;                           used for determining the end of the string. (Input)
;                           (Oops, 8 bytes must be accessessible, see code below.)
; @param    cchModName      Length of fpszModName. Max 8! (Input)
; @param    fp48Mte         Far pointer to a 16:32 MTE pointer. (output)
;                           Upon successful return this will hold a far
;                           32-bit pointer to the MTE for the module.
; @uses     ax
; @sketch
;   Get SAS.SAS_vm_section.SAS_vm_all_mte (32-bit pointer to head of the mte list).
;
;
; @status   completely implemented.
; @author   knut st. osmundsen (bird@anduin.net)
GetModule proc near
    ASSUME  ds:DATA16, ss:NOTHING, es:NOTHING
    ; prolog.
    push    bp
    mov     bp, sp

    ;
    ; Save registers. (paranoia)
    ;
    push    es
    push    ds
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi
    push    eax                         ; need to preserve high part of eax.


    ;
    ; Make a SAS selector.
    ; (This is a special kernel selector (70h readonly / 78h readwrite?))
    ;
    mov     ax, SAS_selector
    mov     es, ax                      ; es:0 is now pointing to 'struct SAS'.


    ;
    ; We now wanna find the vm section.
    ; And then we read the module chain.
    ;
    xor     bx, bx                      ; stupid me / alp
    ASSUME bx:ptr SAS
    mov     bx, es:[bx].SAS_vm_data     ; es:bx is now pointing to 'struct SAS_vm_section'
    ASSUME bx:ptr SAS_vm_section
    mov     eax, es:[bx].SAS_vm_all_mte ; This is a flat 32-bit pointer.

    ;
    ; Now we need to do flat stuff. ie. load flat ds into es.
    ;
    mov     bx, FlatR0DS
    mov     es, bx
    ASSUME es:nothing
    mov     eax, es:[eax]               ; This was a flat pointer to mte_h.
                                        ; We need mte_h which is the head pointer.
    ASSUME eax:far ptr MTE              ; (hope this is ok)


    ;
    ; Before walking the list we should prepare the modulename compare
    ; We load the module name into ecx and edx and padds it with zeros.
    ; ASSUMES (correctly) cchModName <= 8.
    ;
    mov     bx, ss:[bp + 8]             ; length of the module name.
    lds     si, ss:[bp + 4]             ; load far pointer to module name. (ds:si)
    xor     edx, edx
    xor     ecx, ecx
highdword:
    dec     bx
    cmp     bx, 3
    jle     lowdword
    shl     edx, 8
    mov     dl, byte ptr ds:[si+bx]
    jmp     highdword

lowdword:
    shl     ecx, 8
    mov     cl, byte ptr ds:[si+bx]
    dec     bx
    jns     lowdword

    ASSUME ds:nothing


    ;
    ; Walk thru the loaded modules looking for one with a
    ; matching internal name.
    ;
moduleloop:
    ; check if end of list. (NULL pointer)
    or      eax, eax                    ; NULL?
    jnz     ok
    pop     eax                         ; preserve high word.
    mov     ax, 2                       ; error file not found.
    jmp     return
ok:

    ; Compare filename
    cmp     dword ptr es:[eax].mte_modname, ecx
    jne     next
    cmp     dword ptr es:[eax + 4].mte_modname, edx
    jne     next
    jmp     found

    ; next module
next:
    mov     eax, es:[eax].mte_link      ; mte_link points to the next mte in the list.
    jmp     moduleloop


    ;
    ; We found the module we looked for.
    ;
found:
    lds     bx, ss:[bp + 0ah]           ; pointer to where to store the 16:32
                                        ; of the module MTE.
    mov     dword ptr ds:[bx], eax      ; Store the flat address
    mov     ax, es
    mov     word ptr ds:[bx+4], ax      ; Store the flat selector
    pop     eax
    xor     ax, ax                      ; return success.


    ;
    ; Restore registers and return.
    ;
return:
    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ds
    pop     es
    leave
    ret
GetModule endp

CODE16 ends

end
