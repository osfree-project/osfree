;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief Family API implementation helpers
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

@PROLOG		MACRO	NAME

		EXTERN	@INIT : NEAR
		EXTERN	@DONE : NEAR

		PUBLIC	NAME : FAR

ARGS		STRUC
OLDBP		DW	?
RETADDR		DD	?
		ENDM

@START		MACRO	NAME
ARGS		ENDS

ARG_SIZE	EQU	(SIZE ARGS - 6)

NAME		PROC	FAR
		CALL	@INIT

		ENDM

@EPILOG		MACRO	NAME
		CALL	@DONE
		RET	ARG_SIZE
NAME		ENDP
		ENDM
