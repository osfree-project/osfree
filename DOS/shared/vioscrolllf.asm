;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief VioScrollLf DOS wrapper
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

		@PROLOG	VIOSCROLLLF
TopRow		DW	?
LeftCol		DW	?
BotRow		DW	?
RightCol	DW	?
Lines		DW	?
Cell		DD	?
VioHandle	DW	?
		@START	VIOSCROLLLF
; code here
		@EPILOG	VIOSCROLLLF

_TEXT		ENDS

		END
