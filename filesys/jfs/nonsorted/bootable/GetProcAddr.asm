; $Id: GetProcAddr.asm,v 1.5 2002/06/24 23:14:36 bird Exp $
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
INCL_DOSERRORS equ 1
include bseerr.inc
include ldr.inc


;
;   Externals
;
DATA16 segment
extrn  FlatR0DS:word
extrn  FlatR0CS:word
DATA16 ends
CODE16 segment
extrn GetModule:near
CODE16 ends



CODE16 segment


;;
; Looks for and entry point in a module.
; (Searching the module by name and the entry point by name)
; @cproto   int cdecl GetProcAddr(const char FAR *fpszModName, short cchModName,
;                                 const char FAR *fpszProcName, short cchProcName,
;                                 struct farptr48 FAR *fp48EP)
; @returns
; @param    fpszModName     Far pointer to a modulename.
;                           This is uppercased and maximum of 8 chars long.
;                           The '\0' isn't strictly needed as cchModName is
;                           used for determining the end of the string. (Input)
; @param    cchModName      Length of fpszModName. Max 8! (Input)
; @param    fpszProcName    Name of the procedure. Case sensitive!
; @param    cchProcName     Length of fpszProcName. (Input)
; @param    fp48EP          Far pointer to the variable which upon successful
;                           return will hold the 16:32 address of the entrypoint.
; @uses     ax
; @sketch
;   rc = GetModule(fpszModName, cchModName, &pMod);
;   If (!rc)
;   {
;       Search the resident name table for the proc name.
;       Lookup that ordinal in the entrypoint table.
;   }
;   return rc;
;
; @status   completely implemented.
; @author   knut st. osmundsen (bird@anduin.net)
GetProcAddr proc near
    ASSUME  ds:DATA16, ss:NOTHING, es:NOTHING
    ; prolog.
    push    bp
    mov     bp, sp
    sub     sp, 10h                         ; reserve space for variables
fpMte48         EQU bp - 10h
fpszModName     EQU dword ptr bp + 4
cchModName      EQU word  ptr bp + 8
fpszProcName    EQU dword ptr bp + 0ah
cchProcName     EQU word  ptr bp + 0eh
fp48EP          EQU dword ptr bp + 10h


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


    ;
    ; Get the Module MTE.
    ;
    ;
    push    ss
    lea     ax, [fpMte48]
    push    ax
    push    [cchModName]
    push    [fpszModName]
    call    GetModule
    add     sp, 0ah
    or      ax, ax
    jz      module_ok
    jmp     return


    ;
    ; Get swapmte
    ;
module_ok:
    push    eax
    mov     ax, [fpMte48+4]
    mov     ds, ax
    mov     eax, [fpMte48]
    ASSUME ds:nothing, eax:far ptr MTE
    mov     eax, [eax].mte_swapmte
    ASSUME ds:nothing, eax:far ptr SMTE
    or      eax, eax
    jnz     swapmte_ok
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return

    ;
    ; Get resident name table
    ;
swapmte_ok:
    mov     ecx, [eax].smte_restab
    or      ecx, ecx
    jnz     restab_ok
    mov     ax, ERROR_PROC_NOT_FOUND
    jmp     return


    ;
    ; Now we're gonna search this table.
    ; The layout is repetitions of this structure:
    ;   <db length><string><dw ordinal>
    ; The table is terminated by a string of length zero.
    ;
restab_ok:
    ; skip the first name as that is the module name.
    xor     edx, edx
    mov     dl, [ecx]
    cmp     word ptr [ecx + edx + 1], 0 ; check that's it's ordinal 0.
    je      nameloop
    add     ecx, edx
    add     ecx, 3

nameloop:
    xor     edx, edx
    mov     dl, [ecx]
    or      dl, dl
    jnz     namenotlast
    jmp     namenotfound
namenotlast:
    cmp     dx, [cchProcName]
    jne     namenext

    ;
    ; Compare the names.
    ;
    les     di, [fpszProcName]          ; load ds:di with proc name pointer.
    mov     esi, ecx
    inc     esi                         ; skip size byte. esi points to name now.
    dec     dx
namecomp:
    mov     bl, ds:[esi]
    cmp     bl, es:[di]
    jnz     namenext
    inc     esi
    inc     di
    dec     dx
    jns     namecomp
    ; match!
    mov     dx, [esi]                   ; read the ordinal for this name.
    jmp     namefound

namenext:
    ; Advance the pointer to the next name
    xor     edx, edx
    mov     dl, [ecx]
    add     ecx, edx                    ; + length of name
    add     ecx, 3                      ; + size byte and ordinal word
    jmp     nameloop

    ;
    ; Didn't find the name.
    ;
namenotfound:
    pop     eax
    mov     ax, ERROR_PROC_NOT_FOUND
    jmp     return


    ;
    ; Found name. Now we got to search the entry table for this.
    ;
namefound:
    mov     ecx, [eax].smte_enttab
    or      ecx, ecx
    jnz     enttab_ok
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return

    ;
    ; The entry table is organized in bundles.
    ; (DX is the ordinal we should find.)
    ;
enttab_ok:
entloop:
    mov     bx, [ecx]                   ; cnt & type
    or      bl, bl
    jnz     entnotlast
    jmp     entnotfound
entnotlast:
    mov     si, bx
    and     si, 0ffh                    ; 16-bit count.
    or      bh, bh
    jz      entnextskip
    cmp     si, dx
    jnge    entnextused
    jmp     entfound

entnextused:
    and     bh, 0fh
    dec     bh
    jnz     next_2
    mov     bl, 3                       ; type 1
    jmp     entnext
next_2:
    dec     bh
    jnz     next_3
    mov     bl, 5                       ; type 2
    jmp     entnext
next_3:
    dec     bh
    jnz     next_4
    mov     bl, 5                       ; type 3
    jmp     entnext
next_4:
    dec     bh
    jnz     next_5
    mov     bl, 7                       ; type 4
    jmp     entnext
next_5:
    jmp     entnotfound

entnextskip:
    sub     dx, si
    jl      entnotfound                 ; if underflow.
    add     ecx, 2
    jmp     entloop

entnext:      ; this sucks!!!!!!!!!
    push    eax
    push    edx
    xor     eax, eax
    mov     al, bl
    mul     si
    add     ecx, eax
    add     ecx, 2                      ; the object index
    pop     edx
    pop     eax
    jmp     entnextskip

entnotfound:
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return


    ;
    ; Found the right bundle.
    ;   ecx points to the bundle
    ;   dl is the 1 based index of the entry we want in the bundle.
    ;   bl is the count.
    ;   bh is the flags.
    ;
entfound:
    and     bh, 0fh
    cmp     bh, 1
    je      ent_1
    cmp     bh, 3
    jz      ent_3
    jmp     entnotfound                 ; sorry, 4 and 2 isn't supported!

    ;
    ; Load the offset into edx.
    ;
ent_1:
    mov     bl, 3
    jmp     entcalc_1_3

ent_3:
    mov     bl, 5
    jmp     entcalc_1_3

entcalc_1_3:
    push    eax
    xor     eax, eax
    dec     dx
    mov     al, bl
    mul     dx
    add     eax, ecx                    ; Add bundle base.
    add     eax, 4                      ; now points to the entry in question.
    mov     edx, dword ptr [eax+1]
    pop     eax
    cmp     bl, 5                       ; 16-bit offset?
    je      entobject
    and     edx, 0ffffh

    ;
    ; We now need to get the base address and selector of the object.
    ; (edx is now a 32-bit offset into the object)
    ;
entobject:
    mov     cx, [ecx + 2]               ; the object table index.
    or      cx, cx
    jz      storeptr
    cmp     cx, word ptr [eax].smte_objcnt
    jng     objcnt_ok
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return

objcnt_ok:
    mov     eax, [eax].smte_objtab
    push    eax
    push    edx
    xor     eax, eax
    mov     ax, size OTE
    dec     cx                          ; 1-based!
    mul     cx
    mov     ecx, eax
    pop     edx
    pop     eax
    add     eax, ecx                    ; points to OTE entry.
    ASSUME  eax: far ptr OTE

    ;
    ; Get the right selector and evt. add object base to offset.
    ; (bh is the bundle type.)
    ;
    cmp     bh, 1
    je      obj_1
    cmp     bh, 3
    je      obj_3
    int     3                           ; someone have messed with bh!!!

obj_1:
    mov     cx, [eax].ote_sel
    jmp storeptr

obj_3:
    add     edx, [eax].ote_base
    mov     cx, seg DATA16
    mov     es, cx
    ASSUME es:DATA16
    test    [eax].ote_flags, OBJEXEC
    jz      obj_3_DS
    mov     cx, es:FlatR0CS
    jmp storeptr

obj_3_DS:
    mov     cx, es:FlatR0DS
    jmp storeptr


    ;
    ; Store the pointer, cx:edx.
    ;
storeptr:
    les     di, [fp48EP]
    mov     es:[di], edx
    mov     es:[di+4], cx
    pop     eax
    xor     ax, ax

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
GetProcAddr endp

CODE16 ends


end
