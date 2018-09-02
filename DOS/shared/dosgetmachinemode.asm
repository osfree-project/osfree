;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosGetMachineMode DOS wrapper
;
;   @param MACHINEMODE      pointer to BYTE, which will contain mode
;
;   @return NO_ERROR
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSGETMACHINEMODE
MACHINEMODE	DD	?	; Pointer to byte
		@START	DOSGETMACHINEMODE
		XOR AX, AX
		LDS BP,[DS:BP].MACHINEMODE
		MOV [DS:BP], AL
		@EPILOG	DOSGETMACHINEMODE

_TEXT		ENDS

		END
