;;
;;   LFNCHK.ASM - Check wether LFN is supported for a specific drive.
;;
;;   Copyright (C) 1999, 2000, 2001, Imre Leber.
;;
;;   This program is free software; you can redistribute it and/or modify
;;   it under the terms of the GNU General Public License as published by
;;   the Free Software Foundation; either version 2 of the License, or
;;   (at your option) any later version.
;;
;;   This program is distributed in the hope that it will be useful,
;;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;   GNU General Public License for more details.
;;
;;   You should have recieved a copy of the GNU General Public License
;;   along with this program; if not, write to the Free Software
;;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;;
;;   If you have any questions, comments, suggestions, or fixes please
;;   email me at:  imre.leber@worlonline.be
;;
;;

%assign FILESYSLENGTH 32     ;; Sufficient according to RBIL

segment _DATA class=DATA

	drive	  DB '?:\'

	filesys  times FILESYSLENGTH DB 0

segment _TEXT class=CODE

;=========================================================================
;=== 		             CheckDriveOnLFN                           ===
;===-------------------------------------------------------------------===
;=== int CheckDriveOnLFN(char drive);                                  ===
;===                                                                   ===
;=== Paramete: drive letter, eg. 'A'                                   ===
;===                                                                   ===
;=== Returns: LFN supported on this disk: 0                            ===
;===          LFN supported on this disk: 1                            ===
;=========================================================================

	global _CheckDriveOnLFN
_CheckDriveOnLFN:
	push  es
	push  di

	mov  bx, sp
	mov  ax, [ss:bx+06h]    ;; Get parameter

	mov  [drive], al        ;; Complete drive spec

	mov  ax, 71A0h	        ;; The function
	mov  dx, drive          ;; Pointer to drive string
	mov  cx, FILESYSLENGTH  ;; Set lentgh of buffer for file system string
	push ds                 ;; Far pointer
	pop  es
	mov  di, filesys        ;; to file system string
	stc                     ;; Set carry for old DOS compatibility

	int  21h                ;; Ask the system

	jc  .NotSupported

	and  bx, 16384          ;; Mask return value
	cmp  bx, 0              ;; See if supported
	je   .NotSupported

	mov  ax, 1              ;; Function supported
	jmp  short .EndOfProc

.NotSupported:
	mov  ax, 0              ;; Function not supported

.EndOfProc:
	pop  di
	pop  es
	ret