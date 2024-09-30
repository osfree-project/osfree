;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosGetMessage DOS wrapper
;
;   (c) osFree Project 2018, 2024 <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;
;    DosTrueGetMessage (IvTable, IvCount, DataArea, DataLength, MsgNumber, FileName, MsgLength, MsgSeg)
;
;    IvTable (PCHAR FAR *) - input : Address of a list of double-word pointers. Each pointer points to an ASCIIZ or null-terminated DBCS string (variable insertion text). 0 to 9 strings can be present.
;    IvCount (USHORT) - input:Count of variable insertion text strings is 0-9. If IvCount is 0, IvTable is ignored.
;    DataArea (PCHAR) - output:Address of the requested message. If the message is too long to fit in the caller's buffer, as much of the message text is returned as possible, with the appropriate error return code.
;    DataLength (USHORT) - input:Length, in bytes, of the user's storage area.
;    MsgNumber (USHORT) - input:Requested message number.
;    FileName (PSZ) - input:Address of the optional drive, path, and filename of the file where the message can be found. If messages are bound to the .EXE file using MSGBIND utility, then filename is the name of the message file from which the messages are extracted.
;    MsgLength (PUSHORT) - output:Address of the length, in bytes, of the message.
;    MsgSeg (VOID FAR *) - input: Pointer to binded Messages segment.
;
;    0 NO_ERROR
;    2 ERROR_FILE_NOT_FOUND
;    206 ERROR_FILENAME_EXCED_RANGE
;    316 ERROR_MR_MSG_TOO_LONG
;    317 ERROR_MR_MID_NOT_FOUND
;    318 ERROR_MR_UN_ACC_MSGF
;    319 ERROR_MR_INV_MSFG_FORMAT
;    320 ERROR_MR_INV_IVCOUNT
;    321 ERROR_MR_UN_PERFORM
;
;*/

.8086

EXTERN DOSTRUEGETMESSAGE: FAR

_MKMSGSEG	SEGMENT BYTE PUBLIC 'CODE' USE16
		DB        0FFH, 'MKMSGSEG', 0, 1, 0, 0, 0, 0, 0


;    DosGetMessage (IvTable, IvCount, DataArea, DataLength, MsgNumber, FileName, MsgLength)
;
;    IvTable (PCHAR FAR *) - input : Address of a list of double-word pointers. Each pointer points to an ASCIIZ or null-terminated DBCS string (variable insertion text). 0 to 9 strings can be present.
;    IvCount (USHORT) - input:Count of variable insertion text strings is 0-9. If IvCount is 0, IvTable is ignored.
;    DataArea (PCHAR) - output:Address of the requested message. If the message is too long to fit in the caller's buffer, as much of the message text is returned as possible, with the appropriate error return code.
;    DataLength (USHORT) - input:Length, in bytes, of the user's storage area.
;    MsgNumber (USHORT) - input:Requested message number.
;    FileName (PSZ) - input:Address of the optional drive, path, and filename of the file where the message can be found. If messages are bound to the .EXE file using MSGBIND utility, then filename is the name of the message file from which the messages are extracted.
;    MsgLength (PUSHORT) - output:Address of the length, in bytes, of the message.
;
;    0 NO_ERROR
;    2 ERROR_FILE_NOT_FOUND
;    206 ERROR_FILENAME_EXCED_RANGE
;    316 ERROR_MR_MSG_TOO_LONG
;    317 ERROR_MR_MID_NOT_FOUND
;    318 ERROR_MR_UN_ACC_MSGF
;    319 ERROR_MR_INV_MSFG_FORMAT
;    320 ERROR_MR_INV_IVCOUNT
;    321 ERROR_MR_UN_PERFORM

		PUBLIC	DOSGETMESSAGE
DOSGETMESSAGE:
		XOR       AX,AX			; Message segment
		PUSH      AX
		PUSH      CS
		PUSH      BP			; Exchange return address and Message segment
		MOV       BP,SP
		XCHG      AX,WORD PTR 6H[BP]
		XCHG      AX,WORD PTR 2H[BP]
		XCHG      AX,WORD PTR 8H[BP]
		MOV       WORD PTR 4H[BP],AX
		POP       BP
		JMP       FAR PTR DOSTRUEGETMESSAGE
MSGSEGDATA:
		DW        0
    
_MKMSGSEG	ENDS
	
		END
