;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosSetFileInfo DOS wrapper
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

		@PROLOG	DOSSETFILEINFO
FileHandle	DW	?
FileInfoLevel	DW	?
FileInfoBuf	DD	?
FileInfoBufSize	DW	?
		@START	DOSSETFILEINFO
; code here
		@EPILOG	DOSSETFILEINFO

_TEXT		ENDS

		END
