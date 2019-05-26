
;--- defines procedures
;--- PowerOf10
;--- FloatToBCD
;--- FloatToStr

; These are bits in the FP status word.

FP_LESSTHAN	equ	01h
FP_EQUALTO	equ	40h

ten		dq	10.0
ten16	dq	1.0e16
rounder	dq	5.0e10

ten_1	dt	1.0e1	;10.0
	dt	1.0e2		;100.0
	dt	1.0e3		;1,000.0
	dt	1.0e4		;10,000.0
	dt	1.0e5		;100,000.0
	dt	1.0e6		;1,000,000.0
	dt	1.0e7		;10,000,000.0
	dt	1.0e8		;100,000,000.0
	dt	1.0e9		;1,000,000,000.0
	dt	1.0e10		;10,000,000,000.0
	dt	1.0e11		;100,000,000,000.0
	dt	1.0e12		;1,000,000,000,000.0
	dt	1.0e13		;10,000,000,000,000.0
	dt	1.0e14		;100,000,000,000,000.0
	dt	1.0e15		;1,000,000,000,000,000.0

ten_16	dt	1.0e16
	dt	1.0e32
	dt	1.0e48
	dt	1.0e64
	dt	1.0e80
	dt	1.0e96
	dt	1.0e112
	dt	1.0e128
	dt	1.0e144
	dt	1.0e160
	dt	1.0e176
	dt	1.0e192
	dt	1.0e208
	dt	1.0e224
	dt	1.0e240

ten_256	dt	1.0e256

; The remaining exponents are only necessary if we decide to support
; 10-byte doubles.  FloatToStr and StrToFloat only support 8-byte,
; but PowerOf10 doesn't care, so we'll include them.

	dt	1.0e512
	dt	1.0e768
	dt	1.0e1024
	dt	1.0e1280
	dt	1.0e1536
	dt	1.0e1792
	dt	1.0e2048
	dt	1.0e2304
	dt	1.0e2560
	dt	1.0e2816
	dt	1.0e3072
	dt	1.0e3328
	dt	1.0e3584
	dt	1.0e4096
	dt	1.0e4352
	dt	1.0e4608
	dt	1.0e4864

	cpufpu 386

PowerOf10:

    mov ecx, eax
    cmp eax, 0		;.IF	(SDWORD PTR eax < 0)
    jge pot1
	neg eax
pot1:    
    fld1

    movzx ebx, al
    and bl, 0fh		;bits 0-3
    je pot2
	lea ebx, [ebx+ebx*4]
	;fld tword [ebx*2-10+ten_1]
	fld [ebx*2-10+ten_1]
	;fmulp st1, st0
	fmulp st(1), st(0)
pot2:    

    mov bl, al
    shr bl, 4
    and bl, 0fh		;bits 4-7
	je pot3
	lea ebx, [ebx+ebx*4]
	;fld tword [ebx*2-10+ten_16]
	fld [ebx*2-10+ten_16]
	;fmulp st1, st0
	fmulp st(1), st(0)
pot3:    

    mov bl, ah
    and bl, 1fh		;bits 8-12
	je pot4
	lea ebx, [ebx+ebx*4]
	;fld tword [ebx*2-10+ten_256]
	fld [ebx*2-10+ten_256]
	;fmulp st1, st0
	fmulp st(1), st(0)
pot4:    
	cmp ecx, 0		;.IF (SDWORD PTR ecx < 0)
	jge pot5
	;fdivp st1, st0
	fdivp st(1), st(0)
    ret
pot5:    
	;fmulp st1, st0
	fmulp st(1), st(0)
    ret

;PowerOf10 ENDP

; Convert a floating point register to ASCII.  For internal use.
; The result always has exactly 18 digits, with zero padding on the
; left if required.
;
; Entry: ST(0) = a number to convert, 0 <= ST(0) < 1E19.
;	di = an 18-character buffer.
;

;FloatToBCD PROC public uses esi
FloatToBCD:

	push bp
    mov bp,sp
    sub sp, 12

	; The fbstp instruction converts the top of the stack to a
	; packed BCD form in ten bytes, with two digits per byte.  The top 
	; byte has the sign, which we ignore.

    ;fbstp [bp-12]
    fbstp tbyte ptr [bp-12]

	; Now we need to unpack the BCD to ASCII.

    lea si, [bp-4]
    mov cx, 9

nextdigits:
	mov al, [si]		; xxxx xxxx AAAA BBBB
	dec si
	rol ax, 12			; BBBB xxxx xxxx AAAA
	rol ah, 4			; xxxx BBBB xxxx AAAA
	and ax, 0f0fh		; 0000 BBBB 0000 AAAA
	add ax, 3030h		; 3B3A
	mov [di], ax
	add di, 2
	dec cx
    jnz nextdigits
	mov sp,bp
    pop bp
    ret

;FloatToBCD ENDP

;
; Convert a double precision number to a string.
;
; Entry:	fpin = 8-byte double to convert
;		szDbl = character buffer
;
; Exit:		szDbl = converted value
;
; szDbl should be at least 19 bytes long.
;		

;FloatToStr PROC stdcall public USES esi edi pfpin: ptr, szDbl: PTR CHAR
FloatToStr:

	push bp
    mov bp,sp
    sub sp,4+2+10+2+22
    push si
    push di
    
pfpin	equ [bp+4]
szDbl	equ [bp+6]
iExp 	equ [bp-4]
mystat  equ [bp-6]
fpin	equ [bp-16]
stat	equ [bp-18]
szTemp	equ [bp-40]

;    LOCAL iExp: DWORD
;    LOCAL mystat: WORD
;    local fpin: TBYTE
;    LOCAL stat: WORD
;	local szTemp[22]:BYTE


; Special case zero.  fxtract fails for zero.
	
    ;mov si, [pfpin]
    mov si, pfpin
    
    lea di, fpin
    xor dx, dx
    mov cx, 5
ftsn1:    
    lodsw
    or dx,ax
    stosw
    loop ftsn1
    
    mov di, szDbl
	and dx,dx
    jnz notnull
    mov al,'0'
    stosb
    jmp ftsExit2
notnull:    

; Check for a negative number.

	;test byte [fpin+9],80h
	test byte ptr fpin+9,80h
    jz ispositive
    ;and byte [fpin+9], 07fh		; change to positive
    and byte ptr fpin+9, 07fh		; change to positive
    mov al,'-'        			; store a minus sign
    stosb
ispositive:

; Initialize the floating point unit and load our value onto the stack.

    fclex
    fstcw stat
    ;mov word [mystat], 027fh
    mov word ptr mystat, 027fh
    ;fldcw word [mystat]
    fldcw word ptr mystat

    ;fld tword [fpin]
    fld tbyte ptr fpin
    ;fld st0
    fld st(0)

; Compute the closest power of 10 below the number.  We can't get an
; exact value because of rounding.  We could get close by adding in
; log10(mantissa), but it still wouldn't be exact.  Since we'll have to
; check the result anyway, it's silly to waste cycles worrying about
; the mantissa.
;
; The exponent is basically log2(fpin).  Those of you who remember
; algebra realize that log2(fpin) x log10(2) = log10(fpin), which is
; what we want.

    fxtract				; ST=> mantissa, exponent, fpin
    ;fstp st0			; drop the mantissa
    fstp st(0)			; drop the mantissa
    fldlg2				; push log10(2)
    ;fmulp st1, st0		; ST = log10(fpin), fpin
    fmulp st(1), st(0)		; ST = log10(fpin), fpin
    fistp dword ptr iExp	; ST = fpin

; An 8-byte double can carry almost 16 digits of precision.  Actually, it's
; 15.9 digits, so some numbers close to 1E17 will be wrong in the bottom
; digit.  If this is a concern, change the '16' to a '15'.
;
; A 10-byte double can carry almost 19 digits, but fbstp only stores the
; guaranteed 18.  If you're doing 10-byte doubles, change the '16' to '18'.

	cmp dword ptr iExp,18
    jnc notbelow18
    ;fld st0			; ST = fpin, fpin
    fld st(0)			; ST = fpin, fpin
    frndint			; ST = int(fpin), fpin
    ;fcomp st1		; ST = fpin, status set
    fcomp st(1)		; ST = fpin, status set
    fstsw ax
    test ah, FP_EQUALTO
    jz notequal

; We have an integer!  Lucky day.  Go convert it into a temp buffer.

	push di
	lea di, szTemp
	call FloatToBCD
    pop di

	mov eax, 17
	mov ecx, iExp
	sub eax, ecx
	inc cx
	lea si, szTemp
    add si, ax

; The off-by-one order of magnitude problem below can hit us here.  
; We just trim off the possible leading zero.

	;cmp byte [si],'0'
	cmp byte ptr [si],'0'
    jnz not0digit
    inc si
	dec cx
not0digit:    

; Copy the rest of the converted BCD value to our buffer.

	rep movsb
	jmp ftsExit

notequal:
notbelow18:

; Have fbstp round to 17 places.

    mov eax, 16			; experiment
    sub eax, iExp		; adjust exponent to 17
    call PowerOf10

; Either we have exactly 17 digits, or we have exactly 16 digits.  We can
; detect that condition and adjust now.

    ;fcom qword [ten16]
    fcom qword ptr [ten16]
    ; x0xxxx00 means top of stack > ten16
    ; x0xxxx01 means top of stack < ten16
    ; x1xxxx00 means top of stack = ten16
    fstsw ax
    test ah,1
    jz notset1
    ;fmul qword [ten]
    fmul qword ptr [ten]
    ;dec dword [iExp]
    dec dword ptr iExp
notset1:      

; Go convert to BCD.

	push di
	lea  di, szTemp
    call FloatToBCD
    pop di

    lea si, szTemp+1		; point to converted buffer

; If the exponent is between -15 and 16, we should express this as a number
; without scientific notation.

    mov ecx, iExp
    push ecx
    add ecx,15
    cmp ecx,15+16
    pop ecx
    ja fts6

; If the exponent is less than zero, we insert '0.', then -ecx
; leading zeros, then 16 digits of mantissa.  If the exponent is
; positive, we copy ecx+1 digits, then a decimal point (maybe), then 
; the remaining 16-ecx digits.

    inc ecx
    cmp ecx, 0
    jg  ispos1
    mov ax,'0.'
    stosw
	neg ecx
	mov al, '0'
	rep stosb
	mov cx, 16
    jmp fts3
ispos1:    
    rep movsb
    mov al,'.'
    stosb
    mov ecx, 16
    sub ecx, iExp
fts3:      
    rep movsb

; Trim off trailing zeros.

nextitem2:
	;cmp byte [di-1],'0'
	cmp byte ptr [di-1],'0'
    jnz fts1
    dec di
    jmp nextitem2
fts1:

; If we cleared out all the decimal digits, kill the decimal point, too.

	;cmp byte [di-1], '.'
	cmp byte ptr [di-1], '.'
    jnz fts2
	dec di
fts2:    

; That's it.

    jmp ftsExit
fts6:


; Now convert this to a standard, usable format.  If needed, a minus
; sign is already present in the outgoing buffer, and edi already points
; past it.

    movsb				; copy the first digit
    ;mov byte [di], '.'		; plop in a decimal point
    mov byte ptr [di], '.'		; plop in a decimal point
    inc di
    movsd				; copy four more digits
    movsw				; copy two more digits

if 0

; The printf %g specified trims off trailing zeros here.  I dislike
; this, so I've disabled it.  Comment out the if 0 and endif if you
; want this.

fts61:
	cmp byte ptr [di-1],'0'
    jz fts62
    dec di
    jmp fts61
fts62:    
endif

; Shove in the exponent.  If you support 10-byte reals, remember to
; allow 4 digits for the exponent.

    mov byte ptr [di], 'e'	; start the exponent
    mov ax, iExp
    and ax, ax
    jns fts7
    mov byte ptr [di+1], '-'
    neg ax
    jmp fts8
fts7:    
    mov byte ptr [edi+1], '+'
fts8:      

    mov cx, 10

    xor dx, dx
    div cx
    add dl, '0'
    mov [di+4], dl	; shove in the ones exponent digit

    xor dx, dx
    div cx
    add dl, '0'
    mov [di+3], dl	; shove in the tens exponent digit

    xor dx, dx
    div cx
    add dl, '0'
    mov [di+2], dl	; shove in the hundreds exponent digit

    add di, 5		; point to terminator

; Clean up and go home.

ftsExit:
    fldcw stat		; restore control word
    fwait
ftsExit2:
	mov ax,di
    sub ax,szDbl
    mov cx,21
    sub cx,ax
    mov al,' '
    rep stosb
	pop di
    pop si
	mov sp,bp
    pop bp
    ret 4

;FloatToStr ENDP

pfpin equ
szDbl equ
iExp  equ
mystat equ
fpin equ
stat equ
szTemp equ

;	cpufpu 8086
.8086
