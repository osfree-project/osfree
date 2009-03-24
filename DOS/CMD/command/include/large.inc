
; Large header
DGROUP group		_DATA, _BSS


TEXT  segment word public 'CODE' use16
TEXT  ends
_TEXT segment word public 'CODE' use16
_TEXT ends
_DATA segment word public 'DATA' use16
_DATA ends
_BSS  segment word public 'BSS'  use16
_BSS  ends

;@CodeSize EQU 1
;@DataSize EQU 1

ret macro amt
	retf amt
endm
