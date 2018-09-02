;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosSelectDisk DOS wrapper
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;--------D-210E-------------------------------
;INT 21 - DOS 1+ - SELECT DEFAULT DRIVE
;        AH = 0Eh
;        DL = new default drive (00h = A:, 01h = B:, etc)
;Return: AL = number of potentially valid drive letters
;Notes:  under Novell NetWare, the return value is always 32, the number of
;          drives that NetWare supports
;        under DOS 3.0+, the return value is the greatest of 5, the value of
;          LASTDRIVE= in CONFIG.SYS, and the number of drives actually present
;        on a DOS 1.x/2.x single-floppy system, AL returns 2 since the floppy
;          may be accessed as either A: or B:
;        otherwise, the return value is the highest drive actually present
;        DOS 1.x supports a maximum of 16 drives, 2.x a maximum of 63 drives,
;          and 3+ a maximum of 26 drives
;        under Novell DOS 7, this function returns the correct LASTDRIVE value
;          even when the undocumented LASTDRIVE=27..32 directive was used in
;          CONFIG.SYS
;        "parse FCB" (see AH=29h) can be used to determine whether a drive
;          letter is valid
;SeeAlso: AH=19h,AH=3Bh,AH=DBh
;
;
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSSELECTDISK
DRIVENUMBER	DW	?
		@START	DOSSELECTDISK
		MOV	DX, [DS:BP].DRIVENUMBER
		INC	DX
		MOV	AH, 0EH
		INT	21H
		JC	EXIT
		XOR	AX, AX
EXIT:
		@EPILOG	DOSSELECTDISK

_TEXT		ENDS

		END

