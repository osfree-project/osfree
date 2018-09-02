;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief VioScrLock DOS wrapper
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

		@PROLOG	VIOSCRLOCK
WaitFlag	DW	?
Status		DD	?
VioHandle	DW	?
		@START	VIOSCRLOCK
; code here
		@EPILOG	VIOSCRLOCK

_TEXT		ENDS

		END
