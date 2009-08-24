Copyright (c) 1989-1999,2000 Ralf Brown
--------E-21E3-------------------------------
INT 21 - OS/286, OS/386 - ISSUE REAL INTERRUPT
        AH = E3h
        AL = interrupt number
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AH=E1h"OS/286",INT 31/AX=0300h
--------T-21E3-------------------------------
INT 21 - DoubleDOS - ADD CHARACTER TO KEYBOARD BUFFER OF CURRENT JOB
        AH = E3h
        AL = character
Return: AL = status
            00h successful
            01h buffer full (128 characters)
SeeAlso: AH=E1h"DoubleDOS",AH=E2h"DoubleDOS",AH=E8h"DoubleDOS"
SeeAlso: AH=F3h"DoubleDOS"
--------T-21E400-----------------------------
INT 21 - DoubleDOS - INSTALLATION CHECK/PROGRAM STATUS
        AX = E400h
Return: AL = program status
            00h if DoubleDOS not present
            01h if running in visible DoubleDOS partition
            02h if running in the invisible DoubleDOS partition
SeeAlso: AH=E5h"DoubleDOS",AX=F400h
--------E-21E400-----------------------------
INT 21 - OS/286, OS/386 - CHAIN TO REAL-MODE HANDLER
        AX = E400h
        ???
Return: ???
Note:   protected mode only???
--------E-21E402-----------------------------
INT 21 - OS/286, OS/386 - SET PROTECTED-MODE TASK GATE
        AX = E402h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=E403h
--------E-21E403-----------------------------
INT 21 - OS/286, OS/386 - REMOVE PROTECTED-MODE TASK GATE
        AX = E403h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=E402h
--------T-21E5-------------------------------
INT 21 - DoubleDOS - OTHER PROGRAM STATUS
        AH = E5h
Return: AL = status
            00h no program in other partition
            01h program in other partition is running
            02h program in other partition is suspended
SeeAlso: AX=E400h"DoubleDOS",AH=F5h"DoubleDOS"
--------E-21E500-----------------------------
INT 21 - OS/286, OS/386 - HEAP MANAGEMENT STRATEGY
        AX = E500h
        ???
Return: ???
SeeAlso: AX=E501h
--------E-21E501-----------------------------
INT 21 - OS/286, OS/386 - FORCE HEAP COMPACTION
        AX = E501h
        ???
Return: ???
SeeAlso: AX=E500h
--------E-21E6-------------------------------
INT 21 P - OS/286, OS/386 - ISSUE REAL PROCEDURE SIGNAL FROM PROTECTED MODE
        AH = E6h
        ???
Return: ???
SeeAlso: AH=E2h"OS/286"
--------E-21E7-------------------------------
INT 21 - OS/286, OS/386 - CREATE CODE SEGMENT
        AH = E7h
        ???
Return: ???
SeeAlso: AH=E8h"OS/286",AH=E9h"OS/286",AH=EAh"OS/286"
--------E-21E8-------------------------------
INT 21 - OS/286, OS/386 - SEGMENT CREATION
        AH = E8h
        AL = type
            00h data segment
            01h data window/alias
            02h real segment
            03h real window/alias
                CX:DX = size in bytes
                SI:BX -> start of desired memory block
                Return: AX = selector
            06h shareable segment
        ???
Return: ???
SeeAlso: AH=E7h"OS/286",AH=E9h"OS/286"
--------T-21E8-------------------------------
INT 21 - DoubleDOS - SET/RESET KEYBOARD CONTROL FLAGS
        AH = E8h
        AL = program for which to set flags (00h this program, 01h other)
        DX = keyboard control flags (see #02088)
Return: DX = previous flags
Notes:  disabling Ctrl-PrtSc will allow the program to intercept the keystroke;
          disabling any of the other keystrokes disables them completely
        identical to AH=F8h
SeeAlso: AH=E1h"DoubleDOS",AH=E2h"DoubleDOS",AH=E3h"DoubleDOS"
SeeAlso: AH=F8h"DoubleDOS"

Bitfields for DoubleDOS keyboard control flags:
Bit(s)  Description     (Table 02088)
 0      menu
 1      exchange
 2      entire keyboard enable/disable
 3      Ctrl-C
 4      Ctrl-PrtSc
 5      Alt/Erase
 6      Ctrl-Break
 7      Ctrl-NumLock
 8      shift-PrtSc
 9-13   undefined
 14     cancel key (clear keyboard buffer)
 15     suspend key
Note:   setting a enables the corresponding key or operatin, clearing a
          disables it
--------E-21E9-------------------------------
INT 21 P - OS/286, OS/386 - CHANGE SEGMENTS
        AH = E9h
        AL = function
            01h change code segment parameters
            02h change data segment parameters
            05h adjust segment limit
            06h change segment base address
        ???
Return: ???
SeeAlso: AH=E7h"OS/286",AH=E8h"OS/286",AH=EAh"OS/286",AH=EDh"OS/286"
SeeAlso: INT 31/AX=0007h,INT 31/AX=0008h
--------T-21E9-------------------------------
INT 21 - DoubleDOS - SET TIMESHARING PRIORITY
        AH = E9h
        AL = new priority (see #02089)
Return: AL = priority setting if AL=05h on entry
Note:   identical to AH=F9h
SeeAlso: AH=EAh"DoubleDOS",AH=EBh"DoubleDOS",AH=F9h"DoubleDOS"

(Table 02089)
Values for DoubleDOS timesharing priority:
 00h    visible program gets 70%, invisible gets 30% (default)
 01h    visible program gets 50%, invisible gets 50%
 02h    visible program gets 30%, invisible gets 70%
 03h    Top program gets 70%, bottom program gets 30%
 04h    Top program gets 30%, bottom program gets 70%
 05h    get current priority
--------E-21EA-------------------------------
INT 21 - OS/286, OS/386 - ALLOCATE HUGE SEGMENT
        AH = EAh
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AH=E7h"OS/286",AH=E8h"OS/286",AH=E9h"OS/286"
--------E-21EB00-----------------------------
INT 21 - OS/386 VMM - GET A PAGE TABLE ENTRY BY LINEAR ADDRESS
        AX = EB00h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB02h,AX=EB04h,INT 31/AX=0506h
--------E-21EB02-----------------------------
INT 21 - OS/386 VMM - GET A PAGE TABLE ENTRY BY 16-BIT SEGMENT:OFFSET
        AX = EB02h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB00h,AX=EB04h
--------E-21EB03-----------------------------
INT 21 - OS/386 VMM - FREE MAPPED PAGES
        AX = EB03h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB05h,INT 31/AX=0801h
--------E-21EB04-----------------------------
INT 21 - OS/386 VMM - GET A PAGE TABLE ENTRY BY 32-BIT SEGMENT:OFFSET
        AX = EB04h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB00h,AX=EB02h
--------E-21EB05-----------------------------
INT 21 - OS/386 VMM - MAP PAGES
        AX = EB05h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB03h,INT 31/AX=0800h
--------E-21EB06-----------------------------
INT 21 - OS/386 VMM - LOCK PAGES IN MEMORY
        AX = EB06h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB07h,INT 31/AX=0600h
--------E-21EB07-----------------------------
INT 21 - OS/386 VMM - UNLOCK MEMORY PAGES
        AX = EB07h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AX=EB06h,INT 31/AX=0601h
--------N-21EC-------------------------------
INT 21 - Novell NetWare - SYNCHRONIZATION SERVICES - RELEASE FILE
        AH = ECh
        DS:DX -> ASCIZ filename
Return: AL = status
            00h successful
            FFh file not found
Desc:   unlock the specified file but retain it in the log table
Note:   this function is supported by NetWare 4.6+, Advanced NetWare 1.0+, and
          Alloy NTNX
SeeAlso: AH=CDh,AH=EBh"NetWare",AH=EDh"NetWare"
--------E-21EC-------------------------------
INT 21 - OS/286, OS/386 - BLOCK TRANSFER
        AH = ECh
        ???
Return: ???
--------E-21ED-------------------------------
INT 21 - OS/286, OS/386 - GET SEGMENT OR WINDOW DESCRIPTOR
        AH = EDh
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AH=E9h"OS/286"
--------!---Section--------------------------
