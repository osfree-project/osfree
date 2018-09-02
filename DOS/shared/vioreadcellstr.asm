;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief VioReadCellStr DOS wrapper
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

		@PROLOG	VIOREADCELLSTR
CellStr		DD	?
Length		DD	?
Row		DW	?
Column		DW	?
VioHandle	DW	?
		@START	VIOREADCELLSTR
; code here
		@EPILOG	VIOREADCELLSTR

_TEXT		ENDS

		END
