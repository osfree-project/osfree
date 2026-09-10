;*****************************************************************************
;    os2.inc
;    (C) 2004-2024 osFree project
;
;    WARNING! Automaticaly generated file! Don't edit it manually!
;*****************************************************************************

		ifndef __OS2_INC__
__OS2_INC__	EQU	1

INCL_OS2DEF	EQU	1
	ifdef INCL_OS2DEF
  	include os2def.inc
	endif

INCL_BSE	EQU	1
	ifdef INCL_BSE
  	include bse.inc
	endif

INCL_PM	EQU	1
	ifdef INCL_PM
  	include pm.inc
	endif



	endif	; __OS2_INC__
