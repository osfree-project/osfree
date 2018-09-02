;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosInsMessage DOS wrapper
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

		@PROLOG	DOSINSMESSAGE
IvTable		DD	?
IvCount		DW	?
MsgInput	DD	?
MsgInLength	DW	?
DataArea	DD	?
DataLength	DW	?
MsgLength	DD	?
		@START	DOSINSMESSAGE
; code here
		@EPILOG	DOSINSMESSAGE

_TEXT		ENDS

		END
