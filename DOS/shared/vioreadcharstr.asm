;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief VioReadCharStr DOS wrapper
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

		@PROLOG	VIOREADCHARSTR
CharStr		DD	?
Length		DD	?
Row		DW	?
Column		DW	?
VioHandle	DW	?
		@START	VIOREADCHARSTR
; code here
		@EPILOG	VIOREADCHARSTR

_TEXT		ENDS

		END
