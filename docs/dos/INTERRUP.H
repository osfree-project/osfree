Interrupt List, part 8 of 18
Copyright (c) 1989-1999,2000 Ralf Brown
--------D-215E00-----------------------------
INT 21 - DOS 3.1+ network - GET MACHINE NAME
        AX = 5E00h
        DS:DX -> 16-byte buffer for ASCII machine name
Return: CF clear if successful
            CH = validity
                00h name invalid
                nonzero valid
                    CL = NetBIOS number for machine name
                    DS:DX buffer filled with blank-paded name
        CF set on error
            AX = error code (01h) (see #01680 at AH=59h)
Note:   supported by OS/2 v1.3+ compatibility box, PC-NFS
SeeAlso: AX=5E01h
--------D-215E01CH00-------------------------
INT 21 - DOS 3.1+ network - SET MACHINE NAME
        AX = 5E01h
        CH = 00h undefine name (make it invalid)
           <> 0  define name
        CL = name number
        DS:DX -> 15-character blank-padded ASCIZ name
SeeAlso: AX=5E00h
--------D-215E02-----------------------------
INT 21 - DOS 3.1+ network - SET NETWORK PRINTER SETUP STRING
        AX = 5E02h
        BX = redirection list index (see AX=5F02h)
        CX = length of setup string
        DS:SI -> setup string
Return: CF clear if successful
        CF set on error
            AX = error code (01h) (see #01680 at AH=59h/BX=0000h)
Note:   also supported by 10NET v5.0
SeeAlso: AX=5E03h,INT 2F/AX=111Fh
--------D-215E03-----------------------------
INT 21 - DOS 3.1+ network - GET NETWORK PRINTER SETUP STRING
        AX = 5E03h
        BX = redirection list index (see AX=5F02h)
        ES:DI -> 64-byte buffer for setup string
Return: CF clear if successful
            CX = length of setup string
            ES:DI buffer filled
        CF set on error
            AX = error code (01h) (see #01680 at AH=59h/BX=0000h)
Note:   also supported by 10NET v5.0, but 10NET is documented as using DS:SI
          instead of ES:DI
SeeAlso: AX=5E02h,INT 2F/AX=111Fh
--------D-215E04-----------------------------
INT 21 - DOS 3.1+ network - SET PRINTER MODE
        AX = 5E04h
        BX = redirection list index (see AX=5F02h)
        DX = mode
            bit 0: set if binary, clear if text (tabs expanded to blanks)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
Notes:  calls INT 2F/AX=111Fh with 5E04h on stack
        supported by DR DOS 3.41+
SeeAlso: AX=5E05h"DOS",INT 2F/AX=111Fh
--------D-215E05-----------------------------
INT 21 - DOS 3.1+ network - GET PRINTER MODE
        AX = 5E05h
        BX = redirection list index (see AX=5F02h)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            DX = printer mode (see AX=5E04h)
Notes:  calls INT 2F/AX=111Fh with 5E05h on stack
        supported by DR DOS 3.41+
SeeAlso: AX=5E04h"DOS",INT 2F/AX=111Fh
--------D-215F00-----------------------------
INT 21 - DOS 3.1+ network - GET REDIRECTION MODE
        AX = 5F00h
        BL = redirection type
            03h printer
            04h disk drive
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            BH = redirection state
                00h off
                01h on
Note:   calls INT 2F/AX=111Eh with AX on top of the stack
SeeAlso: AX=5F01h,INT 2F/AX=111Eh
--------D-215F01-----------------------------
INT 21 - DOS 3.1+ network - SET REDIRECTION MODE
        AX = 5F01h
        BL = redirection type
            03h printer
            04h disk drive
        BH = redirection state
            00h off
            01h on
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
Notes:  when redirection is off, the local device (if any) rather than the
          remote device is used
        calls INT 2F/AX=111Eh with AX on top of the stack
SeeAlso: AX=5F00h,INT 2F/AX=111Eh,INT 60/AX=0002h
--------D-215F02-----------------------------
INT 21 - DOS 3.1+ network - GET REDIRECTION LIST ENTRY
        AX = 5F02h
        BX = zero-based redirection list index
        CX = driver signature
            0000h LANtastic
            4E57h ('NW') NetWare
        DS:SI -> 16-byte buffer for ASCIZ local device name or drive letter
                  followed by colon
        ES:DI -> 128-byte buffer for ASCIZ network name
Return: CF clear if successful
            BH = device status
                00h valid
                01h invalid
                02h valid (connected from inside Windows for Workgroups v3.11)
            BL = device type
                03h printer
                04h disk drive
            CX = user data previously set with AX=5F03h
            DS:SI and ES:DI buffers filled
            DX,BP destroyed
        CF set on error
            AX = error code (01h,12h) (see #01680 at AH=59h/BX=0000h)
Notes:  this function is passed through to INT 2F/AX=111Eh by the DOS kernel
        error code 12h is returned if BX is greater than the size of the list
        also supported by Banyan VINES, PC-NFS, LANtastic, and 10NET
        supported by LapLink RemoteAccess but returns the local drive letter;
          the remote drive letter can be obtained with INT 2F/AX=Cxxxh
          (see INT 2F/AX=C000h"LapLink")
        the returned device name may or may not include a colon, depending on
          the network software
SeeAlso: AX=5F03h,AX=5F46h,INT 2F/AX=111Eh,INT 2F/AX=C000h"LapLink"
--------D-215F03-----------------------------
INT 21 - DOS 3.1+ network - REDIRECT DEVICE
        AX = 5F03h
        BL = device type
            03h printer
            04h disk drive
        CX = user data to save
                0000h for LANtastic
                4E57h ("NW") for NetWare 4.0 requester
        DS:SI -> ASCIZ local device name (16 bytes max)
        ES:DI -> ASCIZ network name + ASCIZ password (128 bytes max total)
Return: CF clear if successful
        CF set on error
            AX = error code (01h,03h,05h,08h,0Fh,12h) (see #01680 at AH=59h)
Notes:  if device type is disk drive, DS:SI must point at either a null string
          or a string consisting the drive letter followed by a colon; if a
          null string, the network attempts to access the destination without
          redirecting a local drive
        the DOS kernel calls INT 2F/AX=111Eh with AX on top of the stack
        also supported by Banyan VINES, LANtastic, and 10NET
SeeAlso: AX=5F02h,AX=5F04h,INT 2F/AX=111Eh,INT 60/AX=0002h
--------D-215F04-----------------------------
INT 21 - DOS 3.1+ network - CANCEL REDIRECTION
        AX = 5F04h
        DS:SI -> ASCIZ local device name or path
        CX = 4E57h ("NW") for NetWare 4.0 requester
Return: CF clear if successful
        CF set on error
            AX = error code (01h,03h,05h,08h,0Fh,12h) (see #01680 at AH=59h)
Notes:  the DS:SI string must be either a local device name, a drive letter
          followed by a colon, or a network directory beginning with two
          backslashes
        the DOS kernel calls INT 2F/AX=111Eh with AX on top of the stack
        also supported by Banyan VINES, LANtastic, and 10NET
        for NetWare, if only a server name is specified (i.e. "\\SERVER"),
          the connection to that server will be closed
SeeAlso: AX=5F03h,INT 2F/AX=111Eh,INT 60/AX=0003h
--------D-215F05-----------------------------
INT 21 - DOS 4.0+ network - GET EXTENDED REDIRECTION LIST ENTRY
        AX = 5F05h
        BX = redirection list index
        DS:SI -> buffer for ASCIZ source device name
        ES:DI -> buffer for destination ASCIZ network path
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX = server's network process ID handle (10NET)
            BH = device status flag (bit 0 clear if valid)
            BL = device type (03h if printer, 04h if drive)
            CX = stored parameter value (user data) from AX=5F03h
            BP = NETBIOS local session number
            DS:SI buffer filled
            ES:DI buffer filled
Notes:  the local session number allows sharing the redirector's session number
        if an error is caused on the NETBIOS LSN, the redirector may be unable
          to correctly recover from errors
        the DOS kernel calls INT 2F/AX=111Eh with AX on top of the stack
        supported by DR DOS 5.0
        also supported by 10NET v5.0
SeeAlso: AX=5F06h"Network",INT 2F/AX=111Eh
--------O-215F05-----------------------------
INT 21 - STARLITE architecture - MAP LOCAL DRIVE LETTER TO REMOTE FILE SYSTEM
        AX = 5F05h
        DL = drive number (0=A:)
        DS:SI -> ASCIZ name of the object to map the drive to
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
SeeAlso: AX=5F06h"STARLITE",INT 60/AX=0002h
--------O-215F06-----------------------------
INT 21 - STARLITE architecture - UNMAP DRIVE LETTER
        AX = 5F06h
        DL = drive to be unmapped (0=A:)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
SeeAlso: AX=5F05h"STARLITE",INT 60/AX=0003h
--------D-215F07-----------------------------
INT 21 - DOS 5+ - ENABLE DRIVE
        AX = 5F07h
        DL = drive number (0=A:)
Return: CF clear if successful
        CF set on error
            AX = error code (0Fh) (see #01680 at AH=59h/BX=0000h)
Notes:  simply sets the "valid" bit in the drive's CDS
        this function is not supported by Novell DOS 7 through at least
          Update 4, but support was added by Update 13
SeeAlso: AH=52h,AX=5F08h"DOS"
--------O-215F07-----------------------------
INT 21 - STARLITE architecture - MAKE NAMED OBJECT AVAILABLE ON NETWORK
        AX = 5F07h
        DS:SI -> ASCIZ name of object to offer to network
        ES:DI -> ASCIZ name under which object will be known on the network
                MUST begin with three slashes
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
SeeAlso: AX=5F08h"STARLITE"
--------D-215F08-----------------------------
INT 21 - DOS 5+ - DISABLE DRIVE
        AX = 5F08h
        DL = drive number (0=A:)
Return: CF clear if successful
        CF set on error
            AX = error code (0Fh) (see #01680 at AH=59h/BX=0000h)
Notes:  simply clears the "valid" bit in the drive's CDS
        this function is not supported by Novell DOS 7 through at least
          Update 4, but support was added by Update 13
SeeAlso: AH=52h,AX=5F07h"DOS"
--------O-215F08-----------------------------
INT 21 - STARLITE architecture - REMOVE GLOBAL NETWORK NAME OF OBJECT
        AX = 5F08h
        DS:SI -> ASCIZ network name (not local name) of object to unshare
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
SeeAlso: AX=5F07h"STARLITE"
--------O-215F09-----------------------------
INT 21 - STARLITE architecture - BIND TO NETWORK DEVICE
        AX = 5F09h
        DS:DX -> ASCIZ name of the device driver to attach to
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
Note:   the STARLITE distributed file system can attach to multiple networks
          simultaneously
SeeAlso: AX=5F0Ah
--------O-215F0A-----------------------------
INT 21 - STARLITE architecture - DETACH FROM NETWORK DEVICE
        AX = 5F0Ah
        DS:DX -> ASCIZ name of device driver to detach from
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
SeeAlso: AX=5F09h
--------N-215F30-----------------------------
INT 21 U - LAN Manager Enhanced DOS - GET REDIRECTOR VERSION
        AX = 5F30h
Return: AX = version (AH=major,AL=minor)
--------N-215F32-----------------------------
INT 21 u - Named Pipes - LOCAL DosQNmPipeInfo
        AX = 5F32h
        BX = handle
        CX = size of _PIPEINFO structure
        DX = level (must be 0001h)
        DS:SI -> _PIPEINFO structure (see #01701)
Return: CF clear if successful
            _PIPEINFO structure filled in
        CF set on error
            AX = error code
Note:   this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F33h,AX=5F34h

Format of Named Pipes _PIPEINFO structure:
Offset  Size    Description     (Table 01701)
 00h    WORD    size of outgoing buffer
 02h    WORD    size of incoming buffer
 04h    BYTE    maximum number of instances allowed
 05h    BYTE    current number of instances
 06h    BYTE    length of the name (including terminating NUL)
 07h  N BYTEs   name
--------N-215F33-----------------------------
INT 21 u - Named Pipes - LOCAL DosQNmPHandState
        AX = 5F33h
        BX = handle
Return: CF clear if successful
            AH = pipe mode bit mask (see #01702)
            AL = maximum number of instances
        CF set on error
            AX = error code
Note:   this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F32h,AX=5F34h

Bitfields for Named Pipes pipe mode:
Bit(s)  Description     (Table 01702)
 7      set if nonblocking, clear if blocking
 6      set if server end, clear if client end
 2      set if write in message mode, clear if write in byte mode
 0      set if read in message mode, clear if read in byte mode
--------N-215F34-----------------------------
INT 21 u - Named Pipes - LOCAL DosSetNmPHandState
        AX = 5F34h
        BX = handle
        CX = pipe mode bit mask
            bit 15: set if nonblocking, clear if blocking
            bit  8: set if read in message mode, clear if read in byte mode
Return: CF clear if successful
        CF set if error
            AX = error code
Note:   this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F32h,AX=5F33h,AX=5F36h
--------N-215F35-----------------------------
INT 21 u - Named Pipes - LOCAL DosPeekNmPipe
        AX = 5F35h
        BX = handle
        CX = buffer length
        DS:SI -> buffer
Return: CF set on error
            AX = error code
        CF clear if successful (LAN Manager v1-v2)
        AX = 0000h if successful (LAN Manager 3.x)
        ---if successful---
            CX = bytes read
            SI = bytes left in the pipe
            DX = bytes left in the current message
            AX = pipe status (v1-v2) (see #01703)
            DI = pipe status (v3.x)
Note:   this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F38h,AX=5F39h,AX=5F51h

(Table 01703)
Values for pipe status:
 0001h  disconnected
 0002h  listening
 0003h  connected
 0004h  closing
--------N-215F36-----------------------------
INT 21 u - Named Pipes - LOCAL DosTransactNmPipe
        AX = 5F36h
        BX = handle
        CX = in buffer length
        DS:SI -> in buffer
        DX = out buffer length
        ES:DI -> out buffer
Return: CF clear if successful
            CX = bytes read
        CF set on error
            AX = error code
Note:   this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F34h,AX=5F37h
--------N-215F37-----------------------------
INT 21 u - Named Pipes - DosCallNmPipe
        AX = 5F37h
        DS:SI -> DosCallNmPipe stack frame (see #01704)
Return: CF clear if successful
            CX = bytes read
        CF set on error
            AX = error code
Note:   this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F36h,AX=5F38h

Format of Named Pipes DosCallNmPipe stack frame:
Offset  Size    Description     (Table 01704)
 00h    DWORD   timeout
 04h    DWORD   -> bytes read WORD (not used!)
 08h    WORD    out buffer length
 0Ah    DWORD   address of out buffer
 0Eh    WORD    in buffer length
 10h    DWORD   address of in buffer
 14h    DWORD   address of pipe name
--------N-215F38-----------------------------
INT 21 u - Named Pipes - LOCAL DosWaitNmPipe - AWAIT AVAIL. OF PIPE INSTANCE
        AX = 5F38h
        DS:DX -> pipe name
        BX:CX = timeout value
Return: CF clear if successful
        CF set if error
            AX = error code
Notes:  when a client gets a return code of ERROR_PIPE_BUSY on attempting to
          open a pipe, it should issue this call to wait until the pipe
          instance becomes available again; on return from this call, the
          client must attempt to open the pipe once again
        this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
SeeAlso: AX=5F37h,AX=5F39h
--------N-215F39-----------------------------
INT 21 U - Named Pipes - LOCAL DosRawReadNmPipe
        AX = 5F39h
        BX = handle
        CX = buffer length
        DS:DX -> buffer
Return: CF clear if successful
            CX = bytes read
        CF set if error
            AX = error code
Notes:  this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
        not documented in the LAN Manager Toolkit
SeeAlso: AX=5F35h,AX=5F3Ah,INT 2F/AX=1186h
--------N-215F3A-----------------------------
INT 21 U - Named Pipes - LOCAL DosRawWriteNmPipe
        AX = 5F3Ah
        BX = handle
        CX = buffer length
        DS:DX -> buffer
Return: CF clear if successful
            CX = bytes written
        CF set if error
            AX = error code
Notes:  this function was introduced by LAN Manager but is also supported by
          the Novell DOS Named Pipe Extender, Banyan VINES, OS/2 Virtual DOS
          Machines, and others
        not documented in the LAN Manager Toolkit
SeeAlso: AX=5F39h,AX=5F3Bh,INT 2F/AX=118Fh
--------N-215F3B-----------------------------
INT 21 u - LAN Manager Enhanced DOS - NetHandleSetInfo
        AX = 5F3Bh
        BX = handle
        CX = handle_info_1 structure length or sizeof DWORD
        DI = parameter number to set
            0000h all
            0001h number of milliseconds
            0002h number of characters
        DS:DX -> handle_info_1 structure (DI=0000h) (see #01705)
                or DWORD (DI=0001h or 0002h)
        SI = level of information (0001h)
Return: CF clear if successful
            CX = total bytes available
        CF set if error
            AX = error code
SeeAlso: AX=5F3Ch

Format of LAN Manager handle_info_1 structure:
Offset  Size    Description     (Table 01705)
 00h    DWORD   number of milliseconds which workstation collects data before
                  it sends the data to the named pipe
 04h    DWORD   number of characters which workstation collects before it
                  sends the data to the named pipe
--------N-215F3C-----------------------------
INT 21 u - LAN Manager Enhanced DOS - NetHandleGetInfo
        AX = 5F3Ch
        BX = handle
        CX = length of handle_info_1 structure
        DS:DX -> handle_info_1 structure (see #01705)
        SI = level of information (must be 0001h)
Return: CF clear if successful
            CX = total bytes available
        CF set if error
            AX = error code
SeeAlso: AX=5F3Bh
--------N-215F3D-----------------------------
INT 21 U - LAN Manager Enhanced DOS - WRITE MAILSLOT???
        AX = 5F3Dh
        ???
Return: ???
--------N-215F3E-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetSpecialSMB
        AX = 5F3Eh
        ???
Return: ???
Note:   This function is not documented anywhere in the LAN Manager 2.x Toolkit
          but was documented in LAN Manager 1.x manuals.
--------N-215F3F-----------------------------
INT 21 U - LAN Manager Enhanced DOS - REMOTE API CALL
        AX = 5F3Fh
        CX = api number
        ES:DI -> data descriptor
        ES:SI -> parameter descriptor
        ES:DX -> auxiliary descriptor (if DX <> 0)
        ???
Return: ???
--------N-215F40-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetMessageBufferSend
        AX = 5F40h
        DS:DX -> NetMessageBufferSend parameter structure (see #01706)
Return: AX = error code

Format of LAN Manager NetMessageBufferSend parameter structure:
Offset  Size    Description     (Table 01706)
 00h    DWORD   -> recipient name (name for specific user, name* for domain
                      wide name, * for broadcast)
 04h    DWORD   -> buffer
 08h    WORD    length of buffer
--------N-215F41-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetServiceEnum
        AX = 5F41h
        BL = level of detail (0000h, 0001h or 0002h)
        CX = buffer length
        ES:DI -> buffer of service_info_0, service_info_1, or service_info_2
                (see #01707,#01708,#01709)
Return: CF clear if successful
            CX = entries read
            DX = total available
        CF set on error
            AX = error code

Format of LAN Manager service_info_0 structure:
Offset  Size    Description     (Table 01707)
 00h 16 BYTEs   name
SeeAlso: #01708,#01709

Format of service_info_1 structure:
Offset  Size    Description     (Table 01708)
 00h 16 BYTEs   name
 10h    WORD    status bitmask (see #01710)
 12h    DWORD   status code (see #01711)
                (also see Microsoft LAN Manager Programmer's Reference)
 16h    WORD    process id
SeeAlso: #01707,#01709

Format of service_info_2 structure:
Offset  Size    Description     (Table 01709)
 00h 16 BYTEs   name
 10h    WORD    status bitmask (see #01710)
 12h    DWORD   status code (see #01711)
 16h    WORD    process id
 18h 64 BYTEs   text
SeeAlso: #01707,#01708

Bitfields for LAN Manager status bitmask:
Bit(s)  Description     (Table 01710)
 0,1    00 uninstall
                01 install pending
                10 uninstall pending
                11 installed
 2,3    00 active
                01 Continue pending
                10 Pause pending
                11 paused
 4      uninstallable
 5      pausable
 8      disk redirector paused
 9      spooled device redirector paused (printing)
 10     communication device redirector paused

(Table 01711)
Values for LAN Manager status code:
 high word
    3051 Bad parameter value
    3052 A parameter is missing
    3053 An unknown parameter was specified
    3054 The resource is insufficient
    3055 Configuration is faulty
    3056 An MS-DOS or MS OS/2 error occured
    3057 An internal error occured
    3058 An ambiguous parameter name was given
    3059 A duplicate parameter exists
    3060 The service was terminated by NetSeviceControl when it did not respond
    3061 The service program file could not be executed
    3062 The subservice failed to start
    3063 There is a conflict in the value or use of these parameters
    3064 There is a problem with the file
 low word
    3070 There is insufficient memory
    3071 There is insufficeient disk space
    3072 Unable to create thread
    3073 Unable to create process
    3074 A security failure occured
    3075 There is bad or missing default path
    3076 Network software is not installed
    3077 Server software is not installed
    3078 The server could not access the UAS database
    3079 The action requires user-level security
    3080 The log directory is invalid
    3081 The LAN group specificed could not be used
    3082 The computername is being used as a message alias on another computer
    3083 The workstation failed to announce the servername
    3084 The user accounts system is not configured properly
--------N-215F42-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetServiceControl
        AX = 5F42h
        DH = opcode
            00h interrogate status
            01h pause
            02h continue
            03h uninstall
        DL = argument
            01h disk resource
            02h print resource
            04h communications resource (not implemented for DOS)
        ES:BX -> NetServiceControl parameter structure (see #01712)
Return: CF clear if successful
        CF set on error
            AX = error code

Format of LAN Manager NetServiceControl parameter structure:
Offset  Size    Description     (Table 01712)
 00h    DWORD   -> service name
 04h    WORD    result buffer size
 06h    DWORD   -> result buffer as service_info_2 structure
--------N-215F43-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL DosPrintJobGetId
        AX = 5F43h
        BX = handle of remote print job
        CX = size of PRIDINFO struture
        ES:DI -> PRIDINFO structure (see #01713)
Return: CF clear if successful
            PRIDINFO filled in
        CF set on error
            AX = error code

Format of LAN Manager PRIDINFO structure:
Offset  Size    Description     (Table 01713)
 00h    WORD    job id
 02h 16 BYTEs   server name
 12h 13 BYTEs   queue name
 1Fh  1 BYTE    pad
--------N-215F44-----------------------------
INT 21 - LAN Manager Enhanced DOS - LOCAL NetWkstaGetInfo
        AX = 5F44h
        BX = information level (00h, 01h, or 0Ah)
        CX = buffer size
        DX = 0000h
        ES:DI -> buffer in which to store info (see #01714,#01715,#01716),
              including any returned strings
Return: AX = error code
        DX = amount of buffer required, unchanged if supplied buffer large
              enough to hold data
SeeAlso: AX=5F45h,AX=5F49h

Format of LAN Manager wksta_info_0 structure:
Offset  Size    Description     (Table 01714)
 00h    WORD     reserved (0)
 02h    DWORD    reserved (0)
 06h    DWORD    -> path to computer's LANMAN directory
 0Ah    DWORD    -> computername of the workstation
 0Eh    DWORD    -> username of user logged onto workstation
 12h    DWORD    -> domain to which workstation belongs
 16h    WORD     LAN Manager version number (2 bytes, Major, Minor)
 18h    DWORD    reserved (0)
 1Ch    WORD     number of seconds workstation waits for resource availability
 1Eh    DWORD    delay (in millsecs) before sending data to resource
 22h    WORD     reserved (0)
 24h    WORD     reserved (0)
 26h    WORD     ???
 28h    WORD     number of seconds to maintain an inactive connection
 2Ah    WORD     number of seconds an inactive search continues
 2Ch    WORD     threads to dedicate to network (not supported in MSDOS)
 2Eh    WORD     number of simultaneous commands sent to network
 30h    WORD     reserved6 (must be 0)
 32h    WORD     number of internal buffers
 34h    WORD     size (in bytes) of each internal buffer
 36h    WORD     max size (in bytes) of an internal cache buffer (not MSDOS)
 38h    WORD     seconds before disconnecting inactive session (not MSDOS)
 3Ah    WORD     size (in bytes) of an internal error buffer (not MSDOS)
 3Ch    WORD     number of clients that can receive alert messages (not MSDOS)
 3Eh    WORD     number of services that can be started on workstation
 40h    WORD     max size (in kilobytes) of error log (not MSDOS)
 42h    WORD     number of secs before closing inactive print jobs (not MSDOS)
 44h    WORD     number of character buffers for workstation
 46h    WORD     max size (in bytes) of character buffer
 48h    DWORD    -> name of server that validated logon
 4Ch    DWORD    -> workstation heuristics
 50h    WORD     number of mailslots allowed
Note:   pointers to strings are set to 0000h:0000h if there is insufficient
          space in the buffer to hold them
SeeAlso: #01715,#01716

Format of LAN Manager wksta_info_1 structure:
Offset  Size    Description     (Table 01715)
 00h 82 BYTEs   wksta_info_0 structure (see #01714)
 52h    DWORD    -> name of domain which user is logged on to
 56h    DWORD    -> all domains in which computer is enlisted
 5Ah    WORD     number of buffers to allocate for receiving datagrams
SeeAlso: #01716

Format of LAN Manager wksta_info_10 structure:
Offset  Size    Description     (Table 01716)
 00h    DWORD    -> computername of the workstation
 04h    DWORD    -> username of user logged onto workstation
 08h    DWORD    -> domain to which workstation belongs
 0Ch    WORD     LAN Manager version number (2 bytes, Major, Minor)
 0Eh    DWORD    -> name of domain which user is loggod on to
 12h    DWORD    -> all domains in which computer is enlisted
Note:   pointers to strings are set to 0000h:0000h if there is insufficient
          space in the buffer to hold them
SeeAlso: #01714,#01715
--------N-215F45-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetWkstaSetInfo
        AX = 5F45h
        BX = level (0000h or 0001h)
        CX = buffer size
        DX = parameter to set
        ES:DI -> buffer
Return: CF clear if successful
        CF set if error
            AX = error code
SeeAlso: AX=5F44h
--------N-215F46-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetUseEnum
        AX = 5F46h
        BX = level (0000h or 0001h)
        CX = size of buffer
        ES:DI -> buffer of use_info_0 or use_info_1 structures
                  (see #01717,#01718)
Return: CF clear if successful
            CX = entries read
            DX = total available entries
        CF set if error
            AX = error code
SeeAlso: AX=5F47h,AX=5F48h,AX=5F4Ch

Format of LAN Manager use_info_0 structure:
Offset  Size    Description     (Table 01717)
 00h  9 BYTEs   local device name
 09h    BYTE    padding
 0Ah    DWORD   -> remote device name in UNC form \\server\share
SeeAlso: #01718

Format of LAN Manager use_info_1 structure:
Offset  Size    Description     (Table 01718)
 00h  9 BYTEs   Local device name
 09h    BYTE    padding
 0Ah    DWORD   -> remote device name in UNC form \\server\share
 0Eh    DWORD   -> password
 12h    WORD    network link status
                (00h OK, 02h disconnected, else unsure)
 14h    WORD    use type (-1 wildcard, 0 disk, 1 print, 2 com, 3 ipc)
 16h    WORD    ignored
 18h    WORD    ignored
SeeAlso: #01717
--------N-215F47-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetUseAdd
        AX = 5F47h
        BX = level (0001h)
        CX = size of use_info_1 structure
        ES:DI -> use_info_1 structure (see #01718)
Return: CF clear on success
        CF set on error
            AX = error code
SeeAlso: AX=5F46h,AX=5F48h
--------N-215F48-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetUseDel
        AX = 5F48h
        BX = force level
            0000h no force
            0001h force
            0002h lots of force
        ES:DI -> buffer as either the local device name or UNC remote name
Return: CF clear on success
        CF set on error
            AX = error code
SeeAlso: AX=5F46h,AX=5F48h,AX=5F49h
--------N-215F49-----------------------------
INT 21 u - LAN Manager Enhanced DOS - NetUseGetInfo
        AX = 5F49h
        DS:DX -> NetUseGetInfo parameter structure (see #01719)
Return: CF clear on success
            DX = total available
        CF set on error
            AX = error code
SeeAlso: AX=5F44h,AX=5F47h

Format of LAN Manager NetUseGetInfo parameter structure:
Offset  Size    Description     (Table 01719)
 00h    DWORD   pointer to either the local device name or UNC remote name
 04h    WORD    level of information (0000h or 0001h)
 06h    DWORD   pointer to buffer of use_info_0 or use_info_1 structures
 0Ah    WORD    length of buffer
--------N-215F4A-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetRemoteCopy
        AX = 5F4Ah
        DS:DX -> NetRemoteCopy parameter structure (see #01720)
Return: CF clear if successful
        CF set on error
            AX = error code
SeeAlso: AX=5F4Bh

Format of LAN Manager NetRemoteCopy parameter structure:
Offset  Size    Description     (Table 01720)
 00h    DWORD   -> source name as UNC
 04h    DWORD   -> destination name as UNC
 08h    DWORD   -> source password
 0Ch    DWORD   -> destination password
 10h    WORD    destination open bitmap
                if destination path exists
                    0000h open fails
                    0001h file is appended
                    0002h file is overwritten
                if destination path doesn't exist
                    0000h open fails
                    0010h file is created
 12h    WORD    copy control bitmap (see #01721)
 14h    DWORD   -> copy_info buffer
 18h    WORD    length of copy_info buffer

Bitfields for LAN Manager copy control:
Bit(s)  Description     (Table 01721)
 0      destination must be a file
 1      destination must be a directory
 2      destination is opened in ascii mode instead of binary
 3      source is opened in ascii mode instead of binary
 4      verify all write operations
--------N-215F4B-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetRemoteMove
        AX = 5F4Bh
        DS:DX -> NetRemoteMove parameter structure (see #01722)
Return: CF clear if successful
        CF set on error
            AX = error code
SeeAlso: AX=5F4Ah

Format of LAN Manager NetRemoteMove parameter structure:
Offset  Size    Description     (Table 01722)
 00h    DWORD   -> source name as UNC
 04h    DWORD   -> destination name as UNC
 08h    DWORD   -> source password
 0Ch    DWORD   -> destination password
 10h    WORD    destination open bitmap
                if destination path exists
                    0000h open fails
                    0001h file is appended
                    0002h file is overwritten
                if destination path doesn't exist
                    0000h open fails
                    0010h file is created
 12h       WORD    move control bitmap
                    0001h destination must be a file
                    0002h destination must be a directory
 14h    DWORD   -> move_info buffer
 18h    WORD    length of move_info buffer
--------N-215F4C-----------------------------
INT 21 u - LAN Manager Enhanced DOS - LOCAL NetServerEnum
        AX = 5F4Ch
        BX = level (0000h or 0001h)
        CX = buffer length
        ES:DI -> buffer in which to store information
Return: CF clear if successful
            ES:DI -> server_info_X structures (depending on level)
                  (see #01723,#01724)
            BX = entries read
            CX = total entries available
        CF set on error
            AX = error code
Notes:  this function is also supported by the Novell DOS Named Pipe Extender
        this function has been obseleted by NetServerEnum2
SeeAlso: AX=5F53h

Format of LAN Manager server_info_0 structure:
Offset  Size    Description     (Table 01723)
 00h 16 BYTEs   name
SeeAlso: #01724

Format of LAN Manager server_info_1 structure:
Offset  Size    Description     (Table 01724)
 00h 16 BYTEs   name
 10h    BYTE    major version in lower nibble
 11h    BYTE    minor version
 12h    DWORD   server type bitmask (see #01725)
 16h    DWORD   -> comment string
SeeAlso: #01723

Bitfields for LAN Manager server type:
Bit(s)  Description     (Table 01725)
 0      workstation
 1      server
 2      SQL server
 3      primary domain controller
 4      backup domain controller
 5      time server
 6      Apple File Protocol (AFP) server
 7      Novell server
 8      Domain Member (v2.1+)
 9      Print Queue server (v2.1+)
 10     Dialin server (v2.1+)
 11     Unix server (v2.1+)
Note:   set all (FFFFFFFFh) for All Types
--------N-215F4D-----------------------------
INT 21 u - LAN Manager Enhanced DOS - DosMakeMailslot
        AX = 5F4Dh
        BX = message size
        CX = mailslot size (must be bigger than message size by at least 1)
                           (minimum 1000h, maximum FFF6h)
                           (buffer must be 9 bytes bigger than this)
        DS:SI -> name
        ES:DI -> memory buffer
Return: CF clear if successful
            AX = handle
        CF set on error
            AX = error code
SeeAlso: AX=5F4Eh,AX=5F4Fh,AX=5F50h,AX=5F51h
--------N-215F4E-----------------------------
INT 21 u - LAN Manager Enhanced DOS - DosDeleteMailslot
        AX = 5F4Eh
        BX = handle
Return: CF clear if successful
            ES:DI -> memory to be freed (allocated during DosMakeMailslot)
        CF set on error
            AX = error code
SeeAlso: AX=5F4Dh,AX=5F4Fh
--------N-215F4F-----------------------------
INT 21 u - LAN Manager Enhanced DOS - DosMailslotInfo
        AX = 5F4Fh
        BX = handle
Return: CF clear if successful
            AX = max message size
            BX = mailslot size
            CX = next message size
            DX = next message priority
            SI = number of messages waiting
        CF set on error
            AX = error code
SeeAlso: AX=5F4Dh,AX=5F4Eh,AX=5F50h
--------N-215F50-----------------------------
INT 21 u - LAN Manager Enhanced DOS - DosReadMailslot
        AX = 5F50h
        BX = handle
        DX:CX = timeout
        ES:DI -> buffer
Return: CF clear if successful
            AX = bytes read
            CX = next item's size
            DX = next item's priority
        CF set on error
            AX = error code
SeeAlso: AX=5F4Dh,AX=5F4Fh,AX=5F51h,AX=5F52h
--------N-215F51-----------------------------
INT 21 u - LAN Manager Enhanced DOS - DosPeekMailslot
        AX = 5F51h
        BX = handle
        ES:DI -> buffer
Return: CF clear if successful
            AX = bytes read
            CX = next item's size
            DX = next item's priority
        CF set on error
            AX = error code
SeeAlso: AX=5F35h,AX=5F4Fh,AX=5F50h,AX=5F52h
--------N-215F52-----------------------------
INT 21 u - LAN Manager Enhanced DOS - DosWriteMailslot
        AX = 5F52h
        BX = class
        CX = length of buffer
        DX = priority
        ES:DI -> DosWriteMailslot parameter structure (see #01726)
        DS:SI -> mailslot name
Return: CF clear if successful
        CF set on error
            AX = error code
SeeAlso: AX=5F4Fh,AX=5F50h,AX=5F51h

Format of LAN Manager DosWriteMailslot parameter structure:
Offset  Size    Description     (Table 01726)
 00h    DWORD   timeout
 04h    DWORD   -> buffer
--------N-215F53-----------------------------
INT 21 u - LAN Manager Enhanced DOS - NetServerEnum2
        AX = 5F53h
        DS:SI -> NetServerEnum2 parameter structure (see #01727)
Return: CF clear if successful
            BX = entries read
            CX = total entries available
        CF set on error
            AX = error code
SeeAlso: AX=5F4Ch

Format of LAN Manager NetServerEnum2 parameter structure:
Offset  Size    Description     (Table 01727)
 00h    WORD    level (0000h or 0001h)
 02h    DWORD   -> buffer as array of server_info_??? structures (see #01723)
 06h    WORD    length of buffer
 08h    DWORD   server type bitmask (see #01725)
 0Ch    DWORD   -> Domain name (may be 0000h:0000h for all local domains)
--------N-215F55----------------------------
INT 21 U - LAN Manager Enhanced DOS - KILL ALL CONNECTIONS???
        AX = 5F55h
        BX = ???
Return: CF clear if successful
        CF set on error
            AX = error code
--------D-2160-------------------------------
INT 21 - DOS 3.0+ - "TRUENAME" - CANONICALIZE FILENAME OR PATH
        AH = 60h
        DS:SI -> ASCIZ filename or path
        ES:DI -> 128-byte buffer for canonicalized name
Return: CF set on error
            AX = error code
                02h invalid component in directory path or drive letter only
                03h malformed path or invalid drive letter
            ES:DI buffer unchanged
        CF clear if successful
            AH = 00h or 3Ah (DOS 6.1/6.2 for character device)
            AL = destroyed (00h or 2Fh or 5Ch or last character of current
                  directory on drive)
            buffer filled with qualified name of form D:\PATH\FILE.EXT or
              \\MACHINE\PATH\FILE.EXT
Desc:   determine the canonical name of the specified filename or path,
          corresponding to the undocumented TRUENAME command in COMMAND.COM
Notes:  the input path need not actually exist
        letters are uppercased, forward slashes converted to backslashes,
          asterisks converted to appropriate number of question marks, and
          file and directory names are truncated to 8.3 if necessary.  (DR DOS
          3.41 and 5.0 do not expand asterisks)
        '.' and '..' in the path are resolved
        filespecs on local drives always start with "d:", those on network
          drives always start with "\\"
        if path string is on a JOINed drive, the returned name is the one that
          would be needed if the drive were not JOINed; similarly for a
          SUBSTed, ASSIGNed, or network drive letter.   Because of this, it is
          possible to get a qualified name that is not legal under the current
          combination of SUBSTs, ASSIGNs, JOINs, and network redirections
        under DOS 3.3 through 6.00, a device name is translated differently if
          the device name does not have an explicit directory or the directory
          is \DEV (relative directory DEV from the root directory works
          correctly).  In these cases, the returned string consists of the
          unchanged device name and extension appended to the string X:/
          (forward slash instead of backward slash as in all other cases) where
          X is the default or explicit drive letter.
        under MS-DOS 7.0, this call returns the short name for any
          long-filename portions of the provided pathname or filename
        functions which take pathnames require canonical paths if invoked via
          INT 21/AX=5D00h
        supported by OS/2 v1.1 compatibility box
        NetWare 2.1x does not support characters with the high bit set; early
          versions of NetWare 386 support such characters except in this call.
          In addition, NetWare returns error code 3 for the path "X:\"; one
          should use "X:\." instead.
        Novell DOS 7 reportedly has difficulty with non-MS-DOS filenames on
          network drives, and can return "D:" instead of "SERVER/VOLUME"
        for DOS 3.3-6.0, the input and output buffers may be the same, as the
          canonicalized name is built in an internal buffer and copied to the
          specified output buffer as the very last step
        for DR DOS 6.0, this function is not automatically called when on a
          network.  Device drivers reportedly cannot make this call from their
          INIT function.  Using the same pointer for both input and output
          buffers is not supported in the April 1992 and earlier versions of
          DR DOS
        Windows for Workgroups 3.11, Windows95 and even MS-DOS 7.00 only
          return the local drive path; to obtain network paths use
          INT 21/AX=5F02h or INT 21/AX=5F46h instead
        Corel's CORELCDX and MSCDEX without the /S switch return canonical
          names of the form "\\D.\A.\path", where "D" is the CD-ROM drive
          letter and "A" appears to indicate the first physical CD-ROM drive;
          MSCDEX with the /S switch returns a canonical name with embedded
          blanks.  Novell DOS 7 NWCDEX as of the 11/16/94 update returns the
          same canonical path as MSCDEX; earlier revisions returned
          "Cdex.   D:\path", where "D" is the CD-ROM drive letter
        the Windows95 MSCDEX-replacement VxD returns "D:\path", even though the
          MS-DOS 7.00 MSCDEX behaves identically to older versions (above)
BUG:    Windows95 incorrectly treats filenames where the first two characters
          after the drive letter and colon are both slashes (either forward
          or backward) as a UNC (network name) and requires several seconds
          to attempt to resolve the name before returning an unchanged
          string
SeeAlso: AX=5F02h,AX=5FB3h,AX=7160h/CL=00h,INT 2F/AX=1123h,INT 2F/AX=1221h
--------D-2161-------------------------------
INT 21 - DOS 3.0+ - UNUSED (RESERVED FOR NETWORK USE)
        AH = 61h
Return: AL = 00h
Note:   this function does nothing and returns immediately
--------O-2161--BP6467-----------------------
INT 21 U - OS/2 v1.x FAPI - OS/2 FILE SYSTEM JOIN/SUBST
        AH = 61h
        BP = 6467h ("dg")
        AL = function
            00h list (i.e. get)
            01h add
            02h delete
        BX = drive number
        CX = size of buffer
        SI = type (0002h JOIN, 0003h SUBST)
        ES:DI -> buffer
Return: CF clear if successful
            AX = 0000h
            ES:DI buffer filled, if appropriate
        CF set on error
            AX = error code
Notes:  used by JOIN and SUBST to communicate with the OS/2 file system
        for function 00h (list), the ES:DI buffer is filled with the ASCIZ
          JOIN/SUBST path or an empty string if the drive is not JOINed/SUBSTed
        also supported by OS/2 v2.0+ Virtual DOS Machines
--------D-2162-------------------------------
INT 21 - DOS 3.0+ - GET CURRENT PSP ADDRESS
        AH = 62h
Return: BX = segment of PSP for current process
Notes:  this function does not use any of the DOS-internal stacks and may
          thus be called at any time, even during another INT 21h call
        the current PSP is not necessarily the caller's PSP
        identical to the undocumented AH=51h
SeeAlso: AH=50h,AH=51h
--------U-216262SI1994-----------------------
INT 21 - ENVLOCK - INSTALLATION CHECK
        AX = 6262h
        SI = 1994h
Return: AX = 1994h if installed
            ES = ENVLOCK's resident segment
Notes:  to deactivate ENVLOCK, zero out the byte at ES:[0102h]
Program: ENVLOCK is a TSR by Alexander Yanovsky that forces other TSRs to
          deallocate their environment when they stay resident
--------D-216300-----------------------------
INT 21 - DOS 2.25 only - GET LEAD BYTE TABLE ADDRESS
        AX = 6300h
Return: CF clear if successful
            DS:SI -> lead byte table (see #01746)
        CF set on error
            AX = error code (01h) (see #01680 at AH=59h/BX=0000h)
Notes:  does not preserve any registers other than SS:SP
        the US version of MS-DOS 3.30 treats this as an unused function,
          setting AL=00h and returning immediately
SeeAlso: AX=6301h,AH=07h,AH=08h,AH=0Bh

Format of double-byte character set lead byte table entry:
Offset  Size    Description     (Table 01746)
 00h  2 BYTEs   low/high ends of a range of leading byte of double-byte chars
 02h  2 BYTEs   low/high ends of a range of leading byte of double-byte chars
        ...
  N   2 BYTEs   00h,00h end flag
--------D-216300-----------------------------
INT 21 - DOS 3.2+ - GET DOUBLE BYTE CHARACTER SET LEAD-BYTE TABLE
        AX = 6300h
Return: AL = error code
            00h successful
                DS:SI -> DBCS table (see #01746)
                all other registers except CS:IP and SS:SP destroyed
            FFh not supported
Notes:  probably identical to AH=63h/AL=00h for DOS 2.25
        the US version of MS-DOS 3.30 treats this as an unused function,
          setting AL=00h and returning immediately, WITHOUT setting DS:SI;
          only the Far East versions of MS-DOS 3.2 and 3.3 supported this call
        the US version of DOS 4.0+ accepts this function, but returns an empty
          list
        IBM DOS 6.1 SYS.COM assumes that CF is set on error
SeeAlso: AX=6300h"DOS 2.25"
--------D-216301-----------------------------
INT 21 - DOS 2.25, DOS 3.2+ - SET KOREAN (HANGEUL) INPUT MODE
        AX = 6301h
        DL = new mode
            00h return only full characters on DOS keyboard input functions
            01h return partially-formed (interim) characters also
Return: AL = status
            00h successful
            FFh invalid mode
Notes:  Novell DOS 7 simply stores DL in the caller's PSP (see #01378 at AH=26h)
        the US version of MS-DOS 3.30 treats this as an unused function,
          setting AL=00h and returning immediately; only the Far East versions
          of MS-DOS 3.2 and 3.3 supported this call
SeeAlso: AH=07h,AH=08h,AH=0Bh,AX=6300h,AX=6302h
--------D-216302-----------------------------
INT 21 - DOS 2.25, DOS 3.2+ - GET KOREAN (HANGEUL) INPUT MODE
        AX = 6302h
Return: AL = status
            00h successful
                DL = current input mode
                    00h return only full characters (clears interim flag)
                    01h return partial characters (sets interim flag)
            FFh not supported
Notes:  Novell DOS 7 simply reads the value out of the caller's PSP, so it
          can return values other than 00h or 01h if the last call to AX=6301h
          used another value
        the US version of MS-DOS 3.30 treats this as an unused function,
          setting AL=00h and returning immediately, WITHOUT setting DL; only
          the Far East versions of MS-DOS 3.2 and 3.3 supported this call
SeeAlso: AH=07h,AH=08h,AH=0Bh,AX=6300h,AX=6301h
--------D-2164-------------------------------
INT 21 - DOS 3.2+ internal - SET DEVICE DRIVER LOOKAHEAD FLAG
        AH = 64h
        AL = flag
                00h (default) call device driver function 5 (non-dest read)
                        before INT 21/AH=01h,08h,0Ah
            nonzero don't call driver function 5
Return: nothing (MS-DOS)
        CF set, AX=error code??? (DR DOS 5.0, which does not support this call)
Notes:  this function is called by the DOS 3.3+ PRINT.COM
        under MS-DOS, this function does not use any of the DOS-internal stacks
          and may thus be called at any time, even during another DOS call
SeeAlso: AH=01h,AH=08h,AH=0Ah,AX=5D06h
--------O-2164--DX0000-----------------------
INT 21 U - OS/2 v2.0+ Virtual DOS Machine - ENABLE AUTOMATIC TITLE SWITCH
        AH = 64h
        DX = 0000h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
Note:   if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
SeeAlso: AH=64h/DX=0001h,INT 21/AH=4Bh
--------O-2164--DX0001-----------------------
INT 21 U - OS/2 v2.0+ Virtual DOS Machine - SET SESSION TITLE
        AH = 64h
        DX = 0001h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
        ES:DI -> new ASCIZ title (max 12 char) or "" to restore default title
Note:   if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
SeeAlso: AH=64h/DX=0000h,AH=64h/DX=0002h,INT 15/AH=12h/BH=05h
--------O-2164--DX0002-----------------------
INT 21 U - OS/2 v2.0+ Virtual DOS Machine - GET SESSION TITLE
        AH = 64h
        DX = 0002h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
        ES:DI -> 13-byte buffer for current title
Return: buffer filled (single 00h if title never changed)
Note:   if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
SeeAlso: AH=64h/DX=0000h,AH=64h/DX=0001h,INT 15/AH=12h/BH=05h
--------O-2164--DX0003-----------------------
INT 21 U - OS/2 v2.1 Virtual DOS Machine - GET LASTDRIVE
        AH = 64h
        DX = 0003h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
Return: AL = highest drive supported
Notes:  if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
        used by WinOS2
        not supported by OS/2 Warp 3.0, check list of lists instead (see #01627)
SeeAlso: AH=52h
--------O-2164--DX0004-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - GET SIZE OF PTDA JFT
        AH = 64h
        DX = 0004h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
Return: AX = number of entries in OS/2 JFT for VDM
Notes:  if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
        in an OS/2 VDM, the DOS Job File Table in the PSP contains an index
          into the OS/2 JFT in the Per-Task Data Area rather than an SFT index
          because the OS/2 SFT can contain more than 255 entries
--------O-2164--DX0005-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - GET SECOND SFT FLAGS WORD
        AH = 64h
        DX = 0005h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
        DI = DOS file handle
Return: AX = value of second flags word from OS/2 SFT entry for file
Notes:  if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
        the OS/2 SFT has two flags words rather than DOS's one word, and this
          function provides access to the word which is not present in DOS
--------O-2164--DX0006-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - UNLOAD DOSKRNL SYMBOLS & LOAD PROGR
        AH = 64h
        DX = 0006h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
        ES:DI -> ASCIZ filespec
        DS = base address for loading
Notes:  if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
        this function is only supported by the kernel debugging version of
          OS2KRNL
--------O-2164--DX0007-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - GET WinOS2 CALL GATE ADDRESS
        AH = 64h
        DX = 0007h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
Return: AX = call gate address
Notes:  if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
        used by WinOS2 to make direct calls to OS2KRNL, bypassing the overhead
          of DOSKRNL
--------O-2164--DX0008-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - GET LOADING MESSAGE
        AH = 64h
        DX = 0008h (function number)
        CX = 636Ch (magic value, "cl")
        BX = 0000h (indicates special request)
Return: DS:DX -> '$'-terminated message "Loading.  Please wait."
Notes:  if CX is not 636Ch on entry, INT 21/AH=6Ch is invoked, because a bug
          in OS/2 1.x FAPI erroneously called AH=64h instead of AH=6Ch
        this function permits National Language Support for the initial message
          displayed while WinOS2 starts a full-screen session
--------O-2164--CX636C-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - OS/2 API support
        AH = 64h
        CX = 636Ch ("cl")
        BX = API ordinal (see #01747)
        other registers as appropriate for API call
Return: as appropriate for API call
SeeAlso: AH=64h/BX=0025h,AH=64h/BX=00B6h,AH=64h/BX=00CBh

(Table 01747)
Values for OS/2 API ordinal:
 0025h  DOS32StartSession
 0082h  DosGetCP
 00B6h  DosQFSAttach
 00BFh  DosEditName
 00CBh  DosForceDelete
 0144h  Dos32CreateEventSem
 0145h  Dos32OpenEvenSem
 0146h  Dos32CloseEventSem
 0147h  Dos32ResetEventSem
 0148h  Dos32PostEventSem
 0149h  Dos32WaitEventSem
 014Ah  Dos32QueryEventSem
 014Bh  Dos32CreateMutexSem
 014Ch  Dos32OpenMutexSem
 014Dh  Dos32CloseMutexSem
 014Eh  Dos32RequestMutexSem
 014Fh  Dos32ReleaseMutexSem
 0150h  Dos32QueryMutexSem
 0151h  Dos32CreateMuxWaitSem
 0152h  Dos32OpenMuxWaitSem
 0153h  Dos32CloseMuxWaitSem
 0154h  Dos32WaitMuxWaitSem
 0155h  Dos32AddMuxWaitSem
 0156h  Dos32DeleteMuxWaitSem
 0157h  Dos32QueryMuxWaitSem
--------O-2164--BX0025-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - OS/2 API DOS32StartSession
        AH = 64h
        BX = 0025h (API ordinal)
        CX = 636Ch ("cl")
        DS:SI -> STARTDATA structure (see #01748)
Return: AX = return code
SeeAlso: AH=64h/CX=636Ch,AH=64h/BX=00B6h

Format of OS/2 Virtual DOS Machine STARTDATA structure:
Offset  Size    Description     (Table 01748)
 00h    WORD    length of structure (must be 0018h,001Eh,0020h,0032h,or 003Ch)
 02h    WORD    relation of new process to caller (00h independent, 01h child)
 04h    WORD    fore/background (00h foreground, 01h background)
 06h    WORD    trace options (00h-02h, 00h = no trace)
 08h    DWORD   pointer to ASCIZ program title (max 62 chars) or 0000h:0000h
 0Ch    DWORD   pointer to ASCIZ program name (max 128 chars) or 0000h:0000h
 10h    DWORD   pointer to ASCIZ program args (max 144 chars) or 0000h:0000h
 14h    DWORD   "TermQ" (currently reserved, must be 00000000h)
 18h    DWORD   pointer to environment (max 486 bytes) or 0000h:0000h
 1Ch    WORD    inheritance (00h or 01h)
 1Eh    WORD    session type
                00h OS/2 session manager determines type (default)
                01h OS/2 full-screen
                02h OS/2 window
                03h PM
                04h VDM full-screen
                07h VDM window
 20h    DWORD   pointer to ASCIZ icon filename (max 128 chars) or 0000h:0000h
 24h    DWORD   "PgmHandle" (currently reserved, must be 00000000h)
 28h    WORD    "PgmControl"
 2Ah    WORD    initial column
 2Ch    WORD    initial row
 2Eh    WORD    initial width
 30h    WORD    initial height
 32h    WORD    reserved (0)
 34h    DWORD   "ObjectBuffer" (currently reserved, must be 00000000h)
 38h    DWORD   "ObjectBufferLen" (currently reserved, must be 00000000h)
--------O-2164--BX00B6-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - OS/2 API DosQFSAttach
        AH = 64h
        BX = 00B6h (API ordinal)
        CX = 636Ch (magic value "cl")
        DS = user's data segment
        ES:DI -> FSQAttachStruc (see #01749)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX = 0000h
            data buffer filled
SeeAlso: AH=64h/CX=636Ch

Format of OS/2 Virtual DOS Machine FSQAttachStruc:
Offset  Size    Description     (Table 01749)
 00h    DWORD   reserved
 04h    DWORD   pointer to the offset of the data buffer length
 08h    DWORD   pointer to the offset of the data buffer
 0Ch    WORD    FSA Info level
 0Eh    WORD    ordinal index into table
 10h    DWORD   pointer to the offset of the device name
Notes:  The segment value of the buffer, buffer length, and device
          name MUST all be the same.  It is defined on entry in the DS
          register.  The details of each info level are defined in the
          OS/2 CP Reference.
--------O-2164--BX00CB-----------------------
INT 21 U - OS/2 v2.1+ Virtual DOS Machine - OS/2 API DosForceDelete
        AH = 64h
        BX = 00CBh (API ordinal)
        CX = 636Ch (magic value "cl")
        DS:DX -> ASCIZ filename
Return: CF clear if successful
            AX destroyed
        CF set on error
            AX = error code (02h,03h,05h) (see #01680 at AH=59h/BX=0000h)
Desc:   delete a file without saving it to the undelete directory
SeeAlso: AH=41h,AH=64h/CX=636Ch
----------216500-----------------------------
INT 21 - Windows95 (OSR2) - SET GENERAL INTERNATIONALIZATION INFO
        AX = 6500h
        ES:DI -> buffer containing internationalization info (see #01750)
        CX = size of buffer (>= 7)
Return: CF clear if successful
            AX = system code page
            CX = number of bytes copied from supplied buffer
        CF set on error
            AX = error code
Note:   this function will not overwrite the country or code-page numbers,
          but can be used to modify all other data about the country and
          code page
SeeAlso: AH=65h"GET EXTENDED",AH=70h
--------D-2165-------------------------------
INT 21 - DOS 3.3+ - GET EXTENDED COUNTRY INFORMATION
        AH = 65h
        AL = info ID
            01h get general internationalization info (see also AX=6500h)
            02h get pointer to uppercase table
            03h (DOS 6.2+ COUNTRY.SYS) get pointer to lowercase table
            04h get pointer to filename uppercase table
            05h get pointer to filename terminator table
            06h get pointer to collating sequence table
            07h (DOS 4.0+) get pointer to Double-Byte Character Set table
        BX = code page (FFFFh=global code page) (see #01757)
        DX = country ID (FFFFh=current country)
        ES:DI -> country information buffer (see #01750)
        CX = size of buffer (>= 5)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            CX = size of country information returned
            ES:DI -> country information (see #01750)
Notes:  AL=05h appears to return same info for all countries and codepages; it
          has been documented for DOS 5+, but was undocumented in earlier
          versions
        NLSFUNC must be installed to get info for countries other than the
          default
        subfunctions 02h and 04h are identical under OS/2
        subfunction 03h apparently supports only codepage 866 in DOS 6.2x
SeeAlso: AH=38h,AH=70h"MS-DOS 7",INT 2F/AX=1401h,INT 2F/AX=1402h
SeeAlso: INT 2F/AX=14FEh

Format of country information:
Offset  Size    Description     (Table 01750)
 00h    BYTE    info ID
---if info ID = 01h---
 01h    WORD    size of following info in bytes
 03h    WORD    country ID (see #01400 at AH=38h)
 05h    WORD    code page (see #01757)
 07h 34 BYTEs   country-dependent info (see #01399 at AH=38h)
---if info ID = 02h---
 01h    DWORD   pointer to uppercase table (see #01751)
---if info ID = 03h---
 01h    DWORD   pointer to lowercase table (see #01752)
---if info ID = 04h---
 01h    DWORD   pointer to filename uppercase table (see #01753)
---if info ID = 05h---
 01h    DWORD   pointer to filename character table (see #01754)
---if info ID = 06h---
 01h    DWORD   pointer to collating table (see #01755)
---if info ID = 07h (DOS 4.0+)---
 01h    DWORD   pointer to DBCS lead byte table (see #01756)
SeeAlso: #01775

Format of uppercase table:
Offset  Size    Description     (Table 01751)
 00h    WORD    table size (0080h)
 02h 128 BYTEs  uppercase equivalents (if any) of chars 80h to FFh
SeeAlso: #01750,#01753

Format of lowercase table:
Offset  Size    Description     (Table 01752)
 00h    WORD    table size (0100h)
 02h 256 BYTEs  lowercase equivalents (if any) of chars 00h to FFh
SeeAlso: #01750,#01753

Format of filename uppercase table:
Offset  Size    Description     (Table 01753)
 00h    WORD    table size (0080h)
 02h 128 BYTEs  uppercase equivalents (if any) of chars 80h to FFh
SeeAlso: #01750,#01751

Format of filename terminator table:
Offset  Size    Description     (Table 01754)
 00h    WORD    table size (not counting this word)
 02h    BYTE    ??? (01h for MS-DOS 3.30-6.00)
 03h    BYTE    lowest permissible character value for filename
 04h    BYTE    highest permissible character value for filename
 05h    BYTE    ??? (00h for MS-DOS 3.30-6.00)
 06h    BYTE    first excluded character in range \ all characters in this
 07h    BYTE    last excluded character in range  / range are illegal
 08h    BYTE    ??? (02h for MS-DOS 3.30-6.00)
 09h    BYTE    number of illegal (terminator) characters
 0Ah  N BYTEs   characters which terminate a filename:  ."/\[]:|<>+=;,
Note:   partially documented for DOS 5+, but undocumented for earlier versions
SeeAlso: #01750

Format of collating table:
Offset  Size    Description     (Table 01755)
 00h    WORD    table size (0100h)
 02h 256 BYTEs  values used to sort characters 00h to FFh
SeeAlso: #01750

Format of DBCS lead byte table:
Offset  Size    Description     (Table 01756)
 00h    WORD    length of table in ranges
 02h 2N BYTEs   start/end for N lead byte ranges
        WORD    0000h   (end of table)
SeeAlso: #01750
--------D-2165-------------------------------
INT 21 - DOS 4.0+ - COUNTRY-DEPENDENT CHARACTER CAPITALIZATION
        AH = 65h
        AL = function
            20h capitalize character
                DL = character to capitalize
                Return: DL = capitalized character
            21h capitalize string
                DS:DX -> string to capitalize
                CX = length of string
            22h capitalize ASCIZ string
                DS:DX -> ASCIZ string to capitalize
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
Note:   these calls have been documented for DOS 5+, but were undocumented in
          DOS 4.x.
BUG:    Novell DOS 7 Update 15 crashes on AX=6521h when CX=0000h
--------D-216523-----------------------------
INT 21 U - DOS 4.0+ - DETERMINE IF CHARACTER REPRESENTS YES/NO RESPONSE
        AX = 6523h
        DL = character
        DH = second character of double-byte character (if applicable)
Return: CF set on error
        CF clear if successful
            AX = type
                00h no
                01h yes
                02h neither yes nor no
Note:   supported by Novell DOS 7, though prior to Update 14, the results
          depended on the kernel variant rather than the YESCHAR= or COUNTRY=
          setting in CONFIG.SYS
BUG:    Novell DOS commands and tools all use internal settings rather than
          this function, so the Yes/No responses are dependent on the specific
          country's variant being run rather than on the COUNTRY= setting
--------D-2165-------------------------------
INT 21 U - DOS 4.0+ internal - COUNTRY-DEPENDENT FILENAME CAPITALIZATION
        AH = 65h
        AL = function
            A0h capitalize filename character
                DL = character to capitalize
                Return: DL = capitalized character
            A1h capitalize counted filename string
                DS:DX -> filename string to capitalize
                CX = length of string
            A2h capitalize ASCIZ filename
                DS:DX -> ASCIZ filename to capitalize
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
Note:   nonfunctional in MS-DOS 4.00 through 7.10 (Win98) due to a bug (the
          code sets a pointer depending on the high bit of AL, but doesn't
          clear the bit before branching by function number).  Supported and
          functional(!) in Novell DOS 7 (Update 15), as well as PC-DOS 7 and
          OS/2 MVDM
--------D-216601-----------------------------
INT 21 - DOS 3.3+ - GET GLOBAL CODE PAGE TABLE
        AX = 6601h
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            BX = active code page (see #01757)
            DX = system code page (see #01757)
SeeAlso: AX=6602h
--------D-216602-----------------------------
INT 21 - DOS 3.3+ - SET GLOBAL CODE PAGE TABLE
        AX = 6602h
        BX = active code page (see #01757)
        DX = system code page (active page at boot time)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX = EB41h (Novell NWDOS v7.0 when NLSFUNC not installed and
                  request was for previously-active code page)
SeeAlso: AX=6601h,INT 2F/AX=14FFh

(Table 01757)
Values for code page:
 0      Reduced 7-bit ASCII                     [NetWare]
 37     EBCDIC: US/Canada English (CECP)        [Windows NT 3.51+]
 38     EBCDIC: International (old)
 111    Greek
 112    Turkish
 113    Yugoslavian
 161    Arabic                                  [Linux]
 162    Arabic                                  [Linux]
 163    Arabic                                  [Linux]
 164    Arabic                                  [Linux]
 165    Arabic                                  [Linux]
 237    EBCDIC: Germany (CECP)
 273    EBCDIC: ??? (CECP)
 274    EBCDIC: Belgium
 275    EBCDIC: Brazilian
 277    EBCDIC: Danish/Norwegian (CECP)
 278    EBCDIC: Finnish/Swedish (CECP)
 280    EBCDIC: Italian (CECP)
 281    EBCDIC: Japanese-E
 284    EBCDIC: Latin-American/Spanish (CECP)
 285    EBCDIC: UK English (CECP)
 290    EBCDIC: Japanese Kana
 297    EBCDIC: French (CECP)
 367    US-ASCII (ISO 646-US, 7-bit)
 420    EBCDIC: Arabic 1
 423    EBCDIC: Greek
 424    EBCDIC: Hebrew
 437    US / English / PC-8 / IBM-2
 500    EBCDIC: Belgium/Switzerland (CECP)
 500    EBCDIC: International
 646    (??? reserved for ISO 646 7-bit codepages)
 667    Eastern Europe (Polish)
 668    Eastern Europe (Slavic)
 708    Arabic/Middle East
 737    Greek (2)
 775    Baltic / Baltic Rim
 819    Latin-1 (ISO 8859-1)
 850    Multilingual (Latin-1)
 851    Greek
 852    Slavic/Easter Europe (Latin-2)          [DOS 5+]
 853    Turkish (Latin-2)
 854    Spanish
 855    Cyrilic (1)
 857    Turkish
 860    Portugese
 861    Icelandic
 862    Hebrew
 863    French Canadian
 864    Arabic/Middle East
 865    Nordic (Norwegian/Danish)
 866    Russian (Cyrillic 2)
 867    Czech
 868    Arabic
 869    Greek (1)
 870    EBCDIC: Yugoslavia (Roece)
 871    EBCDIC: Icelandic
 874    Thailand
 875    EBCDIC: Greek
 880    Russian (Cyrillic GOST)
 880      EBCDIC: Cyrillic
 881    Latin 1 (ISO 8859-1)
 882    Latin 2 (ISO 8859-2)
 883    Latin 3 (ISO 8859-3)
 884    Latin 4 (ISO 8859-4)
 885    Latin 5 (ISO 8859-5)
 891    unknown
 897    Japanese (Shift-JIS)
 903    unknown
 904    unknown
 905    EBCDIC: Turkish
 912    Latin 2 (ISO 8859-2: Eastern Europe)
 913    (??? reserved for Latin 3)
 914    (??? reserved for Latin 4)
 915    Cyrillic (ISO 8859-5: Latin/Cyrillic)
 916    (??? reserved for ISO 8859-6: Latin/Arabic)
 917    (??? reserved for ISO 8859-7: Latin/Greek)
 918    EBCDIC: Arabic 2
 919    (??? reserved for ISO 8859-9: Latin 5)
 920    (??? reserved for ISO 8859-10: Latin 6/Sami)
 932    DBCS: Japanese (Shift-JIS)
 934    DBCS: Korean
 936    DBCS: Chinese (PRC/ROC, Simplified/xGB)
 938    DBCS: Taiwan
 938    DBCS: Chinese (PRC/ROC)
 942    DBCS: Japanese SAA
 944    DBCS: Korean SAA
 948    DBCS: Chinese SAA (PRC/ROC)
 949    Korean (Unified Hangul; Extended Wansung)
 950    Chinese Traditional, Big5 (Taiwan, Hong Kong)
 966    Saudi Arabian
 972    Hebrew (Israeli VT100)
 999    reserved for user-definable codepages
1004    Desktop Publishing
1026    EBCDIC: Turkish (Latin 5)
1047    EBCDIC: International (CECP, de-facto EBCDIC-US)
1250    MSWIN: Eastern Europe (Latin 2)
1251    MSWIN: Cyrillic
1252    MSWIN: English/W. Europe/Standard (Latin 1)
1253    MSWIN: Greek (GRC)
1254    MSWIN: Turkish
1255    MSWIN: Hebrew
1256    MSWIN: Arabic
1257    MSWIN: Baltic (Estonian, Latvian, Lithuanian)
1258    MSWIN: Vietnamese
1361    ANSI???: Korean (Johab)
10000   MAC: International/Standard (Roman)
10006   MAC: Greek
10007   MAC: Cyrillic
10029   MAC: Latin 2
10079   MAC: Icelandic
10081   MAC: Turkish
10646   (should be reserved for the future ISO 10646 32-bit codepage???)
65400   OS/2: reserved for Glyphs
Notes:  not all code pages are available in all versions of DOS or
          DOS-compatibles, and many (particularly EBCDIC) have not been
          implemented for *any* DOS to date
        CECP = 'Country Extended CodePage' by IBM.
        Unicode (UCS-2) is a 16-bit character codeset, covering all commonly
          used characters from almost any language. Not all definitions are
          fixed at the time of this writing. Unicode will be the future of
          character coding for the foreseeable future, but is only the
          "basic multilingual plane" (BMP) subset of 32-bit ISO 10646 codes
          (UCS-4), a single character set standard covering requirements for
          all countries and languages, which is still under construction.
        The MS Windows 'ANSI' codepage 1252 (based on the MS Windows 3.0+
          implementation) appears to be 100% compatible with the code sets
          used by Amiga OS and Acorn Archimedes RISC-OS and is also a linear
          subset of the 16bit UniCode code set (UCS-2); the actual ANSI
          codepage is defined by ISO 8859-1 (Latin 1).
        At least applications for OS/2 Warp 3 Presentation Manager can use
          EBCDIC codepages, but the codepage ID assignments for EBCDIC
          codepages are not known for OS/2. OS/2 SAA codepages are not
          supported in CONFIG.SYS.  Codepage 65400 "Glyphs" is not actually a
          codepage, but a way to directly access the first 256 of the 383
          glyphs from the current font set.
        Novell DOS 7/DR DOS 6/Caldera OpenDOS undocumented codepage 853
          does not necessarily match with MS-DOS' undocumented codepage 853.
        Undocumented codepages 667 and 668 can be found in Russian's PTS/DOS
          6.51 and S/DOS 1.x DISPLAY.CPI and contain some Eastern European
          characters.
        Novell NetWare 3.xx clients support UniCode and codepages 437, 850,
          860, 863, 865, 897, 932, and 1252 (possibly more). NetWare 4.xx
          clients also support 1250, 1251, 1256. Personal NetWare 1.0 (PNW),
          as it was distributed in Europe, supports UniCode and codepages 437,
          850 and 1252.  Novell's Client32 for DOS/Windows supports 874, 932,
          936, 949, 950, 1250 - 1257. For codesets not yet available, Novell
          offers a reduced ASCII 7-bit support through a codepage 0 used as a
          translation table to UniCode, that supports characters 32-127 except
          92 ('\').

Format of DOS .CPI (Code Page Information) file header:
Offset  Size    Description     (Table 01758)
 00h    BYTE    ID tag
                FFh FONT file (Standard for generic display or
                      printer font files used by MS-DOS, PC-DOS, DR DOS
                      and Novell DOS)
                7Fh DRFONT file (Used by DR DOS 6.0 / Novell DOS 7 for
                      enhanced & compressed display font files. DR DOS 6.0
                      and Novell DOS 7 still support the standard FONT
                      files, thus allowing leaning of .CPI files from
                      MS-DOS to DR DOS / Novell DOS!)
 01h  7 BYTEs   ID string
                "FONT   " = FONT file (Standard for display or printer)
                "DRFONT " = DRFONT file (Enhanced compressed format used
                            by DR DOS 6.0 / Novell DOS 7 for display fonts)
 08h  8 BYTEs   reserved (0)
 10h    WORD    number of pointers (1)
 12h    BYTE    type of pointers (1)
 13h    DWORD   pointer to file offset of FontInfoHeader
                (Generally pointing to the byte just after FontFileHeader,
                     that is 0000h:0017h. Due to extra data at offset 17h, this
                     value has changed with DR DOS 6.0 / Novell DOS 7 DRFONTs!
                     "MS-DOS 4.0 programmers reference" claimed word offset
                     +15h as an endmarker (0000h), but actually it is the
                     High-Word of the pointer.)
--- Extended FontFileHeader with DR DOS 6.0 / Novell DOS 7 DRFONTs: ---
 17h    BYTE    number of fonts per codepage supported by this file
                     (N=4 with both DR DOS 6.0 / Novell DOS 7 DRFONT files)
 18h  N BYTEs   cellsize (Height) of fonts 1..N
                the cellsize corresponds with the character boxes height,
                  but is also the count of bytes used for each of the
                  characters inside the font data (as currently all fonts
                  are organized heightx8 and 8 pixel width is just one byte).
 var  N DWORDs  file offsets of DisplayFontData.

Format of DOS .CPI file Font Information Header:
Offset  Size    Description     (Table 01759)
 00h    WORD    number of codepage entries
        var     N codepage entry headers (see #01760)
SeeAlso: #01758

Format of DOS .CPI file CodePage Entry Header:
Offset  Size    Description     (Table 01760)
 00h    WORD    size of this header (normally 1Ch)
 02h    DWORD   offset of next entry, or 0000h:0000h or FFFFh:FFFFh if last
                (if a valid "next" pointer but all of the fonts indicated in
                  the .CPI header have been processed, this field normally
                  points at an optional text area at the end of the .CPI file
                  containing copyright information)
 06h    WORD    device type
                01h display (FONT or DRFONT)
                02h printer (FONT)
 08h  8 BYTEs   blank-padded device name string
 10h    WORD    code page (see #01757)
 12h  3 WORDs   reserved (0)
 18h    DWORD   pointer to Font Data Header (see #00222)
                normally immediately follows this header
SeeAlso: #01758

Format of DOS .CPI file Font Data Header:
Offset  Size    Description     (Table 01761)
 00h    WORD    record type
                0001h FONT
                0002h DRFONT (DR DOS 6.0/Novell DOS 7 display font)
 02h    WORD    number of fonts
 04h    WORD    length of font data (display fonts)
                ??? (printer fonts)
 06h    var     font data (#fonts * fontlength) bytes
SeeAlso: #01758

Format of DOS .CPI file ScreenFONT Header:
Offset  Size    Description     (Table 01762)
 00h  6 BYTEs   display-font header (see #01764)
 06h    var     display font data
SeeAlso: #01758

Format of .CPI file DRFONT Header:
Offset  Size    Description     (Table 01763)
 00h 6N BYTEs   DisplayFONT headers for N fonts (see #01764)
      M WORDs   character index table for cell offsets in font data
                currently 256 words in length
SeeAlso: #01758

Format of .CPI file DisplayFONT header:
Offset  Size    Description     (Table 01764)
 00h    BYTE    height of character cell
 01h    BYTE    width of character cell (currently always 08h)
 02h    BYTE    aspect ratio (height) (currently 00h, unused)
 03h    BYTE    aspect ratio (width) (currently 00h, unused)
 04h    WORD    number of characters per font (256)
SeeAlso: #01758

Format of .CPI file PrinterFONT header:
Offset  Size    Description     (Table 01765)
 00h    WORD    type of printer
                0001h (4201.CPI, 1050.CPI, EPS.CPI)
                0002h (4208.CPI, 5202.CPI, PPDS.CPI)
 02h    WORD    bytes per hardware/download codepage-select escape sequence
                (max 31, typically 12)
 04h  N BYTEs   escape sequence to select hardware codepage
      N BYTEs   escape sequence to select download codepage
        var     download data for printer font (including escape sequence to
                  transfer data)
SeeAlso: #01758
--------D-2167-------------------------------
INT 21 - DOS 3.3+ - SET HANDLE COUNT
        AH = 67h
        BX = size of new file handle table for process
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
Desc:   adjust the size of the per-process open file table, thus raising or
          lowering the limit on the number of files the caller can open
          simultaneously
Notes:  if BX <= 20, no action is taken if the handle limit has not yet been
          increased, and the table is copied back into the PSP if the limit
          is currently > 20 handles
        for file handle tables of > 20 handles, DOS 3.30 never reuses the
          same memory block, even if the limit is being reduced; this can lead
          to memory fragmentation as a new block is allocated and the existing
          one freed
        only the first 20 handles are copied to child processes in DOS 3.3-6.0
        increasing the file handles here will not, in general, increase the
          number of files that can be opened using the runtime library of a
          high-level language such as C
        Novell DOS 7 reportedly terminates the calling program if the JFT is
          being reduced in size and there are any open file handles beyond
          the portion of the JFT being retained
BUGS:   the original release of DOS 3.30 allocates a full 64K for the handle
          table on requests for an even number of handles
        DR DOS 3.41 and 5.0 will lose track of any open file handles beyond
          the portion of the JFT retained after the call; MS-DOS will indicate
          error 04h if any of the JFT entries to be removed are open
SeeAlso: AH=26h,AH=86h
--------D-2168-------------------------------
INT 21 - DOS 3.3+ - "FFLUSH" - COMMIT FILE
        AH = 68h
        BX = file handle
Return: CF clear if successful
            all data still in DOS disk buffers is written to disk immediately,
              and the file's directory entry is updated
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=5D01h,AH=6Ah,INT 2F/AX=1107h
--------D-2169-------------------------------
INT 21 U - DOS 4.0+ internal - GET/SET DISK SERIAL NUMBER
        AH = 69h
        AL = subfunction
            00h get serial number
            01h set serial number
        BL = drive (0=default, 1=A, 2=B, etc)
        BH = info level (00h only for DOS; OS/2 allows other levels)
        DS:DX -> disk info (see #01766)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX destroyed
            (AL = 00h) buffer filled with appropriate values from extended BPB
            (AL = 01h) extended BPB on disk set to values from buffer
Notes:  does not generate a critical error; all errors are returned in AX
        error 0005h given if no extended BPB on disk
        does not work on network drives (error 0001h)
        buffer after first two bytes is exact copy of bytes 27h thru 3Dh of
          extended BPB on disk
        this function is supported under Novell NetWare versions 2.0A through
          3.11; the returned serial number is the one a DIR would display,
          the volume label is the NetWare volume label, and the file system
          is set to "FAT16".
        this function is not supported by Novell DOS 7 through Update 13, but
          Personal NetWare 1.0 and Update 15 do support this function
        the serial number is computed from the current date and time when the
          disk is created; the first part is the sum of the seconds/hundredths
          and month/day, the second part is the sum of the hours/minutes and
          year
        the volume label which is read or set is the one stored in the extended
          BPB on disks formatted with DOS 4.0+, rather than the special root
          directory entry used by the DIR command in COMMAND.COM (use AH=11h
          to find that volume label)
SeeAlso: AX=440Dh"DOS 3.2+"

Format of disk info:
Offset  Size    Description     (Table 01766)
 00h    WORD    0000h (info level)
 02h    DWORD   disk serial number (binary)
 06h 11 BYTEs   volume label or "NO NAME    " if none present
 11h  8 BYTEs   (AL=00h only) filesystem type (see #01767)
Note:   under MS-DOS 7.0 or a Windows95 DOS box, the volume label field can be
          all blanks even when a volume label has been set (the Win95
          installation seems to blank the volume label field in the partition
          boot sector; once LABEL has been run, the volume label is reported
          correctly)
SeeAlso: AH=4Eh

(Table 01767)
Values for filesystem type:
 "FAT12   "     12-bit FAT
 "FAT16   "     16-bit FAT
 "CDROM   "     High-Sierra CD-ROM filesystem
 "CD001   "     ISO 9660 CD-ROM filesystem
 "CDAUDIO "     audio CD
SeeAlso: #01766
--------O-2169-------------------------------
INT 21 - DR DOS 5.0 - NULL FUNCTION
        AH = 69h
Return: AL = 00h
SeeAlso: AH=18h
--------d-2169FFDX0000-----------------------
INT 21 U - CUBIT v4.00 - GET CUBIT INT 21 HANDLER
        AX = 69FFh
        DX = 0000h
        BX = CB00h (magic value)
Return: ES:BX -> CUBITR.EXE handler for INT 21
InstallCheck:   test that the first eight bytes at the returned interrupt
          handler are EBh 07h "CUBITR" (a short jump around the signature
          followed by the signature)
Note:   the byte following the signature (i.e. ES:[BX+8]) indicates whether
          CUBITR is active (01h) or disabled (00h)
SeeAlso: AX=69FFh/DX=CFBFh
Index:  installation check;CUBIT
--------d-2169FFDXCFBF-----------------------
INT 21 U - CUBIT v4.00 - UNINSTALL
        AX = 69FFh
        DX = CFBFh
        CX = EFCFh
        BX = CB00h (magic value)
Return: ES:BX -> CUBITR.EXE handler for INT 21
        CX = status
            2020h successful
            2222h failed
Note:   if DX is neither 0000h nor CFBFh on entry, some other code is executed
SeeAlso: AX=69FFh/DX=0000h
--------D-216A-------------------------------
INT 21 U - DOS 4.0+ - COMMIT FILE
        AH = 6Ah
        BX = file handle
Return: CF clear if successful
            AH = 68h
        CF set on error
            AX = error code (06h) (see #01680 at AH=59h/BX=0000h)
Note:   identical to AH=68h in DOS 5.0-6.0; not known whether this is the case
          in DOS 4.x
SeeAlso: AH=68h
--------D-216B-------------------------------
INT 21 U - DOS 4.0 internal - IFS IOCTL
        AH = 6Bh
        AL = subfunction
            00h ???
                DS:SI -> Current Directory Structure???
                CL = drive (1=A:)
            01h ???
                DS:SI -> ???
                CL = file handle???
            02h ???
                DS:SI -> Current Directory Structure???
                DI = ???
                CX = drive (1=A:)
Return: CF set on error
            AX = error code (see #01680 at INT 21/AH=59h/BX=0000h)
        CF clear if successful
Notes:  passed through to INT 2F/AX=112Fh with AX on top of stack
        Novell DOS 7 Update 15 returns CF set/AX=0001h (invalid function)
SeeAlso: AH=6Bh"DOS 5",INT 2F/AX=112Fh
--------D-216B-------------------------------
INT 21 U - DOS 5+ - NULL FUNCTION
        AH = 6Bh
Return: AL = 00h
Note:   this function does nothing and returns immediately
SeeAlso: AH=6Bh"DOS 4"
--------D-216C00-----------------------------
INT 21 - DOS 4.0+ - EXTENDED OPEN/CREATE
        AX = 6C00h
        BL = open mode as in AL for normal open (see also AH=3Dh)
            bit 7: inheritance
            bits 4-6: sharing mode
            bit 3 reserved
            bits 0-2: access mode
                100 read-only, do not modify file's last-access time (DOS 7.0)
        BH = flags
            bit 6 = auto commit on every write (see also AH=68h)
            bit 5 = return error rather than doing INT 24h
            bit 4 = (FAT32) extended size (allow 4GB files instead of 2GB)
        CX = create attribute (see #01769)
        DL = action if file exists/does not exist (see #01770)
        DH = 00h (reserved)
        DS:SI -> ASCIZ file name
Return: CF set on error
           AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
           AX = file handle
           CX = status (see #01768)
Notes:  the PC LAN Program only supports existence actions (in DL) of 01h,
          10h with sharing=compatibility, and 12h
        DR DOS reportedly does not support this function and does not return
          an "invalid function call" error when this function is used.
        the documented bits of BX are stored in the SFT when the file is opened
          (see #01641,#01642)
BUG:    this function has bugs (at least in DOS 5.0 and 6.2) when used with
          drives handled via the network redirector (INT 2F/AX=112Eh):
            - CX (attribute) is not passed to the redirector if DL=11h,
            - CX does not return the status, it is returned unchanged because
              DOS does a PUSH CX/POP CX when calling the redirector.
SeeAlso: AH=3Ch,AH=3Dh,AX=6C01h,AX=7160h/CL=00h,INT 2F/AX=112Eh

(Table 01768)
Values for extended open function status:
 01h    file opened
 02h    file created
 03h    file replaced

Bitfields for file create attribute:
Bit(s)  Description     (Table 01769)
 6-15   reserved
 5      archive
 4      reserved
 3      volume label
 2      system
 1      hidden
 0      readonly

Bitfields for action:
Bit(s)  Description     (Table 01770)
 7-4    action if file does not exist
        0000 fail
        0001 create
 3-0    action if file exists
        0000 fail
        0001 open
        0010 replace/open
--------O-216C01-----------------------------
INT 21 U - OS/2 v2.0 - "DosOpen2"
        AX = 6C01h
        BL = open mode as in AL for normal open (see also AH=3Dh)
            bit 7: inheritance
            bits 4-6: sharing mode
            bit 3 reserved
            bits 0-2: access mode
        BH = flags
            bit 6 = auto commit on every write (see also AH=68h)
            bit 5 = return error rather than doing INT 24h
        CX = create attribute (see #01769)
        DL = action if file exists/does not exist (see #01770)
        DH = 00h (reserved)
        DS:SI -> ASCIZ file name
        ES:DI -> EAOP structure
Return: CF set on error
           AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
           AX = file handle
           CX = status (see #01768)
Note:   this function is virtually identical to AX=6C00h, but supports OS/2's
          extended attributes
SeeAlso: AX=5704h,AX=6C00h,AH=6Fh"OS/2"
--------D-216D-------------------------------
INT 21 U - DOS 5+ ROM - FIND FIRST ROM PROGRAM
        AH = 6Dh
        DS:DX -> ASCIZ program name (may contain wildcrds)
Return: CF clear if found
            Disk Transfer Area filled with ROM search structure (see #01771)
        CF set if not found
            AX = error code
                0002h name not found in ROM
                0003h name contains colon or backslash
        ---if not supported (DOS <5, MS-DOS 5+ non-ROM versions)---
        AL = 00h
Notes:  the '*' wildcard matches all remaining characters in a ROM program's
          name; any following characters in the search mask are ignored up to
          another asterisk, which must be matched by an asterisk in the
          found program's name.
        the search mask and program names may contain multiple periods
SeeAlso: AH=1Ah,AH=4Eh,AH=6Eh,AX=6F00h,AX=6F02h,INT 2F/AX=160Ch

Format of ROM search structure:
Offset  Size    Description     (Table 01771)
 00h 13 BYTEs   ASCIZ name of found ROM program
 0Dh    DWORD   address at which to resume search (do not modify)
 11h    var     ASCIZ search mask passed in (do not modify)
--------O-216D-------------------------------
INT 21 U - OS/2 v1.x FAPI - "DosMkDir2"
        AH = 6Dh
        ???
Return: ???
Desc:   create a new directory, with extended attribute information
Note:   also supported by OS/2 v2.0+ Virtual DOS Machines
BUG:    does not work under OS/2 v2.0 because MVDM does not translate the
          real-mode segment pointer in the Extended Attribute structure
          (see #01673) into a protected-mode selector; use AH=39h followed by
          AX=5703h instead
SeeAlso: AH=39h,AX=5702h"OS/2",AX=5703h"OS/2"
--------O-216D-------------------------------
INT 21 U - Novell DOS 7 - NOP
        AH = 6Dh
Return: AX = 0000h
Note:   this function invokes the same code as other NOP functions such as
          AH=18h and AH=61h
--------D-216E-------------------------------
INT 21 U - DOS 5+ ROM - FIND NEXT ROM PROGRAM
        AH = 6Eh
        Disk Transfer Area contains result of previous FindFirst ROM
          (see AH=6Dh)
Return: CF clear if found
            Disk Transfer Area filled with updated ROM search structure
              (see #01771)
        CF set if not found
            AX = 0012h (no more matches)
        ---if not supported (DOS <5, MS-DOS 5+ non-ROM versions)---
        AL = 00h
SeeAlso: AH=4Fh,AH=6Dh
--------O-216E-------------------------------
INT 21 U - OS/2 v1.x FAPI - "DosEnumAttrib"
        AH = 6Eh
        DS:SI -> parameter packet (see #01772)
Return: CF clear if successful
            AX = 0000h
            DS:SI buffer updated
        CF set on error
            AX = error code
Note:   also supported by OS/2 v2.0+ Virtual DOS Machines
SeeAlso: AX=5703h,AH=6Fh"OS/2",INT 2F/AX=112Dh

Format of OS/2 DosEnumAttrib parameter packet:
Offset  Size    Description     (Table 01772)
 00h    DWORD   reserved (0)
 04h    DWORD   info level (always 00000001h)
 08h    DWORD   (call) number of entries requested
                (ret) actual number of entries returned
 0Ch    DWORD   length of buffer
 10h    DWORD   pointer to buffer for results
 14h    DWORD   number of first entry to return
 18h    DWORD   -> file handle or ASCIZ pathname
 1Ch    WORD    flag: 00h = previous field is file handle, 01h = pathname
--------D-216F00-----------------------------
INT 21 U - DOS 5+ ROM - GET ROM SCAN START ADDRESS
        AX = 6F00h
Return: CF clear
        AL = 00h
            BX = current ROM scan starting segment if function supported
SeeAlso: AH=6Dh,AX=6F01h,AX=6F02h
--------O-216F00-----------------------------
INT 21 U - OS/2 v1.x FAPI - "DosQMaxEASize" - GET MAXIMUM SIZE OF EXTENDED ATTR
        AX = 6F00h
        DS:SI -> DWORD buffer for maximum size of an extended attribute
Return: CF clear if successful
            AX = 0000h
            buffer filled
        CF set on error
            AX = error code
Note:   also supported by OS/2 v2.0+ Virtual DOS Machines
SeeAlso: AX=5703h,AX=6C01h,AH=6Eh"OS/2"
--------D-216F01-----------------------------
INT 21 U - DOS 5+ ROM - SET ROM SCAN START ADDRESS
        AX = 6F01h
        BX = new ROM scan starting address
Return: CF clear
        AL = 00h
SeeAlso: AX=6F00h,AX=6F03h
--------D-216F02-----------------------------
INT 21 U - DOS 5+ ROM - GET EXCLUSION REGION LIST
        AX = 6F02h
        ES:BX -> buffer for exclusion region list (see #01773)
Return: CF clear
        AL = 00h
        ES:BX = 0000h:0000h on error, unchanged if buffer filled
Note:   for DOS versions which do not support this function, the return value
          is AL=00h, CF unchanged, ES:BX unchanged, and the ES:BX buffer
          unchanged
SeeAlso: AX=6F00h,AX=6F03h

Format of ROM exclusion region list:
Offset  Size    Description     (Table 01773)
 00h    WORD    number of entries
 02h 2N WORDs   start/end segments of N excluded regions
--------D-216F03-----------------------------
INT 21 U - DOS 5+ ROM - SET EXCLUSION REGION LIST
        AX = 6F03h
        DS:DX -> new exclusion region list (see #01773)
Return: CF clear
        AL = 00h
Notes:  DOS saves only the pointer and assumes that the contents of the list
          are never changed, and that regions do not overlap
        if AL > 03h on entry, DOS returns CF set/AL=01h
SeeAlso: AX=6F01h,AX=6F02h
--------D-2170-------------------------------
INT 21 - MS-DOS 7 (Windows95) - GET/SET INTERNATIONALIZATION INFORMATION
        AH = 70h
        AL = subfunction
            00h get ??? info
                CX = buffer size (3Ah bytes needed)
                ES:DI -> buffer
            01h set above info
                CX = number of bytes to set
                DS:SI -> buffer containing ??? info (see #01774)
            02h set general internationalization info (see also AX=6500h)
                DS:SI -> buffer containing info (see #01775)
                CX = buffer size in bytes (up to 26h bytes used)
                first three bytes are skipped, the rest is copied to somewhere
                  in the DOS data segment
Return: CF clear if successful
            ES:DI buffer filled (func 00h) (see #01774)
            CX = number of bytes actually set or returned
                  (max 003Ah for functions 00h and 01h under v7.00, 0026h for
                  function 02h)
        CF set on error
            AX = error code
                7000h if function not supported
SeeAlso: AH=38h,AH=65h

Format of MS-DOS v7.0 ??? table:
Offset  Size    Description     (Table 01774)
 00h 58 BYTEs   ??? country-specific information
                returned was (among others) "ENU USA GR"..."AM PM M/d/yy"...
                 "dddd,MMMMdd,yyyy" in the German Preview version, and "US"
                 instead of "GR" in the US build 450 version (with German
                 country setting) and the US build 950a version with US
                 country settings
SeeAlso: #01775

Format of MS-DOS v7.0 internationalization table:
Offset  Size    Description     (Table 01775)
 00h  3 BYTEs   unused (and ignored by DOS)
 03h    WORD    country ID (see #01400 at AH=38h)
 05h    WORD    code page (see #01757)
 07h    WORD    date format
 09h  5 BYTEs   ASCIZ currency symbol string
 07h  2 BYTEs   ASCIZ thousands separator
 09h  2 BYTEs   ASCIZ decimal separator
 0Bh  2 BYTEs   ASCIZ date separator
 0Dh  2 BYTEs   ASCIZ time separator
 0Fh    BYTE    currency format
                bit 2 = set if currency symbol replaces decimal point
                bit 1 = number of spaces between value and currency symbol
                bit 0 = 0 if currency symbol precedes value
                        1 if currency symbol follows value
 10h    BYTE    number of digits after decimal in currency
 11h    BYTE    time format
                bit 0 = 0 if 12-hour clock
                        1 if 24-hour clock
 12h    DWORD   address of case map routine
                (FAR CALL, AL = character to map to upper case [>= 80h])
 16h  2 BYTEs   ASCIZ data-list separator
 18h 10 BYTEs   reserved
Note:   this table has the identical format to the extended country information
          retrieved via AH=65h with info ID = 01h
SeeAlso: #01399,#01750
----------217070BX6060-----------------------
INT 21 - PCW Weather Card interface - GET DATA SEGMENT
        AX = 7070h
        BX = 6060h
        CX = 7070h
        DX = 7070h
        SI = 7070h
        DI = 7070h
Return: AX = segment of data structure (see #01776)
Notes:  the data structure is at offset 516 from this segment
        the update byte is at offset 514 from this segment.  Updates are
          once per second while this byte is nonzero and it is decremented
          once per second.  While this byte is 0 updates are once per minute.
SeeAlso: AX=7070h/BX=7070h

Format of PCW Weather Card data structure:
Offset  Type    Description     (Table 01776)
 00h    WORD    hour
 02h    WORD    minute
 04h    WORD    second
 06h    WORD    day
 08h    WORD    month
 0Ah    WORD    year
 0Ch    WORD    ???
 0Eh    WORD    relative barometric pressure (in 1/100 inches)
 10h    WORD    ???
 12h    WORD    ???
 14h    WORD    temperature 1 (in 1/10 degrees F)
 16h    WORD    temperature 1 lowest (in 1/10 degrees F)
 18h    WORD    temperature 1 highest (in 1/10 degrees F)
 1Ah    WORD    temperature 2 (in 1/10 degrees F)
 1Ch    WORD    temperature 2 lowest (in 1/10 degrees F)
 1Eh    WORD    temperature 2 highest (in 1/10 degrees F)
 20h    WORD    wind speed (in MPH)
 22h    WORD    average of 60 wind speed samples (in MPH)
 24h    WORD    highest wind speed (in MPH)
 26h    WORD    wind chill factor  (in 1/10 degrees F)
 28h    WORD    lowest wind chill factor (in 1/10 degrees F)
 2Ah    WORD    ???
 2Ch    WORD    wind direction (in degrees)
 2Eh    WORD    accumulated daily rainfall (in 1/10 inches)
 30h    WORD    accumulated annual rainfall (in 1/10 inches)
----------217070BX7070-----------------------
INT 21 - PCW Weather Card interface - INSTALLATION CHECK
        AX = 7070h
        BX = 7070h
        CX = 7070h
        DX = 7070h
        SI = 7070h
        DI = 7070h
Return: AX = 0070h
        BX = 0070h
        CX = 0070h
        DX = 0070h
        SI = 0070h
        DI = 0070h
SeeAlso: AX=7070h/BX=6060h,AX=8080h
--------D-2171-------------------------------
INT 21 - Windows95 - LONG FILENAME FUNCTIONS
        AH = 71h
        AL = function
            0Dh reset drive (see AX=710Dh)
            39h create directory (see AX=7139h)
            3Ah remove directory (see AX=713Ah)
            3Bh set current directory (see AX=713Bh)
            41h delete file (see AX=7141h)
            43h get/set file attributes (see AX=7143h)
            47h get current directory (see AX=7147h)
            4Eh find first file (see AX=714Eh)
            4Fh find next file (see AX=714Fh)
            56h move (rename) file (see AX=7156h)
            60h truename (see AX=7160h/CL=00h,AX=7160h/CL=02h)
            6Ch create/open file (see AX=716Ch)
            A0h get volume information (see AX=71A0h)
            A1h terminate FindFirst/FindNext (see AX=71A1h)
            A6h get file information (see AX=71A6h)
            A7h time conversion (see AX=71A7h/BL=00h,AX=71A7h/BL=01h)
            A8h generate short filename (see AX=71A8h)
            A9h server create/open file (see AX=71A9h)
            AAh create/terminate SUBST (see AX=71AAh/BH=00h,AX=71AAh/BH=02h)
Return: CF set on error
            AX = error code (see #01680)
                7100h if function not supported
        CF clear if successful
            other registers as for corresponding "old" DOS function
Notes:  if error 7100h is returned, the old-style function should be called
        AX=714Eh returns a "search handle" which must be passed to AX=714Fh;
          when the search is complete, AX=71A1h must be called to terminate
          the search
        for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
        Caldera's DPMS-enabled LONGNAME.EXE BETA 1 extension for DR-DOS 7
          supports the following sub-set of LFN functions: 39h, 3Ah, 3Bh, 41h,
          43h (BL = 0, 1 only), 47h, 4Eh, 4Fh, 56h, 60h (CL = 0, 1, 2), 6Ch,
          A0h, A1h, A8h. BETA 2 fixes LFN directory entry checksums, which
          were causing wrong LFNs to be attached to a file. The 8.3 short
          names for filenames with exactly 8 chars are no longer abbreviated
          (e.g. LONGNAME.TXT -> LONGNAME.TXT, not LONGNA~1.TXT). BETA 3 has
          A7h (BL=0, 1) functions added, and 4Eh/4Fh can return file times
          in both DOS and 64 bit formats, BETA 4 has support added for
          Caldera's DRFAT32 redirector extension (see INT 2F/AX=15xxh).
        Caldera's DR-OpenDOS 7.02+ COMMAND.COM utilizes the LFN API as soon
          as it detects it (mind, that LONGNAME.EXE can be dynamically loaded
          and unloaded at runtime). This COMMAND.COM shell also works under
          MS-DOS/PC DOS and in DOS boxes of Windows9x, NT, 2000, and OS/2.
        For 4DOS 6.02+ to work with 3rd party LFN providers, the Win95LFN=Yes
          directive should be inserted into the 4DOS.INI file.
        Mike Podanoffsky's RxDOS 7.2 provides most of this API natively,
          including functions 39h, 3Ah, 3Bh, 41h, 43h (BL = ???), 47h, 4Bh,
          4Eh, 4Fh, 56h, 60h (CL = 0, 1, 2, no CH), 6Ch, A0h, A1h and A7h.
          However, not all sub-functions seem to be supported yet.
SeeAlso: AH=39h,AH=3Ah,AH=3Bh,AH=41h,AX=4300h,AX=4301h,AX=4304h,AX=4306h
SeeAlso: AX=4307h,AH=47h,AH=4Eh,AH=4Fh,AH=56h,AH=6Ch,AX=714Eh,AX=714Fh
--------D-21710D-----------------------------
INT 21 - Windows95 - RESET DRIVE
        AX = 710Dh
        CX = action (see #01777)
        DX = drive number
Return: CF clear
Note:   for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=0Dh

(Table 01777)
Values for drive reset action:
 0000h  flush filesystem buffers for drive, and reset drive
 0001h  flush filesystem buffers and cache for drive, and reset drive
 0002h  remount DriveSpace volume
--------D-217139-----------------------------
INT 21 - Windows95 - LONG FILENAME - MAKE DIRECTORY
        AX = 7139h
        DS:DX -> ASCIZ long directory name (including path)
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=39h,AX=713Ah,AX=713Bh,AX=43FFh/BP=5053h
--------D-21713A-----------------------------
INT 21 - Windows95 - LONG FILENAME - REMOVE DIRECTORY
        AX = 713Ah
        DS:DX -> ASCIZ long name of directory to remove
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=3Ah,AX=7139h
--------D-21713B-----------------------------
INT 21 - Windows95 - LONG FILENAME - CHANGE DIRECTORY
        AX = 713Bh
        DS:DX -> ASCIZ long name of directory to make current
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=0Eh,AH=3Bh,AX=7139h
--------D-217141-----------------------------
INT 21 - Windows95 - LONG FILENAME - DELETE FILE
        AX = 7141h
        DS:DX -> ASCIZ long name of file to delete
        SI = wildcard and attributes flag
                0000h wildcards are not allowed, and search attributes are
                        ignored
                0001h wildcards are allowed, and only files with matching
                        names and attributes are deleted
        CL = search attributes
        CH = must-match attributes
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=41h
--------D-217143-----------------------------
INT 21 - Windows95 - LONG FILENAME - EXTENDED GET/SET FILE ATTRIBUTES
        AX = 7143h
        DS:DX -> ASCIZ filename
        BL = action
            00h retrieve attributes
                Return: CX = file attributes (see #01420)
            01h set attributes
                CX = attributes
            02h get physical size of compressed file
                Return: DX:AX = actual disk usage of file, in bytes
            03h set last write date/time
                DI = new last-write date (see #01666)
                CX = new last-write time (see #01665)
            04h get last write date/time
                Return: CX = last write time (see #01665)
                        DI = last write date (see #01666)
            05h set last access date
                DI = new last-access date (see #01666)
            06h get last access date
                Return: DI = last access date (see #01666)
            07h set creation date/time
                DI = new creation date (see #01666)
                CX = new creation time (see #01665)
                SI = hundredths (10-millisecond units past time in CX, 0-199)
            08h get creation date/time
                Return: CX = creation time (see #01665)
                        DI = creation date (see #01666)
                        SI = hundredths (10-millisecond units past time in CX)
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AX=4300h,AX=4301h
--------D-217147-----------------------------
INT 21 - Windows95 - LONG FILENAME - GET CURRENT DIRECTORY
        AX = 7147h
        DL = drive number (00h = current, 01h = A:, etc.)
        DS:SI -> buffer for ASCIZ directory name
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Notes:  the returned pathname does not include the drive letter, colon, or
          leading backslash, and is not necessarily a long filename -- this
          function returns whatever path was used when changing to the
          current directory, and may include a mixture of long and short
          components
        the provided buffer must be at least as large as the value indicated
          by AX=71A0h
        for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=47h,AX=713Bh,AX=7160h,AX=71A0h
--------D-21714E-----------------------------
INT 21 - Windows95 - LONG FILENAME - FIND FIRST MATCHING FILE
        AX = 714Eh
        CL = allowable-attributes mask (see #01420 at AX=4301h)
              (bits 0 and 5 ignored)
        CH = required-attributes mask (see #01420)
        SI = date/time format (see #01778)
        DS:DX -> ASCIZ filespec (both "*" and "*.*" match any filename)
        ES:DI -> FindData record (see #01779)
Return: CF clear if successful
            AX = filefind handle (needed to continue search)
            CX = Unicode conversion flags (see #01780)
        CF set on error
            AX = error code
                7100h if function not supported
Notes:  this function is only available when IFSMgr is running, not under bare
          MS-DOS 7
        the application should close the filefind handle with AX=71A1h as soon
          as it has completed its search
        for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=4Eh,AX=714Fh,AX=71A1h

(Table 01778)
Values for Windows95 date/time format:
 0000h  use 64-bit file time format
 0001h  use MS-DOS date/time values (see #01665,#01666) in low double-word of
          file time QWORD (date is high word, time is low word of double-word)
SeeAlso: #01779

Format of Windows95 long filename FindData record:
Offset  Size    Description     (Table 01779)
 00h    DWORD   file attributes
                bits 0-6 standard DOS attributes (see #01420 at INT 21/AX=4301h)
                bit 8: temporary file
 04h    QWORD   file creation time (number of 100ns intervals since 1/1/1601)
 0Ch    QWORD   last access time
 14h    QWORD   last modification time
 1Ch    DWORD   file size (high 32 bits)
 20h    DWORD   file size (low 32 bits)
 24h  8 BYTEs   reserved (apparently unused)
 2Ch 260 BYTEs  ASCIZ full filename
130h 14 BYTEs   ASCIZ short filename (for backward compatibility)
Note:   under Windows95B, the ASCIZ short filename will be the empty string
          if the directory does not contain a long filename entry for the
          file; in that case, the application should use the full filename
SeeAlso: #01780

Bitfields for Windows95 Unicode conversion flags:
Bit(s)  Description     (Table 01780)
 0      the returned full filename contains underscores for un-convertable
          Unicode characters
 1      the returned short filename contains underscores for un-convertable
          Unicode characters
SeeAlso: #01779
--------D-21714F-----------------------------
INT 21 - Windows95 - LONG FILENAME - FIND NEXT MATCHING FILE
        AX = 714Fh
        BX = filefind handle (from AX=714Eh)
        SI = date/time format (see #01778)
        ES:DI -> buffer for FindData record (see #01779)
Return: CF clear if successful
            AH = 4Fh (undocumented)
            AL destroyed (becomes low byte of filefind handle in Win95B)
            CX = Unicode conversion flags (see #01780)
        CF set on error
            AX = error code
                7100h if function not supported
Notes:  this function is only available when IFSMgr is running, not under bare
          MS-DOS 7
        for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
SeeAlso: AH=4Eh,AX=714Eh,AX=71A1h,AX=71A2h
--------D-217156-----------------------------
INT 21 - Windows95 - LONG FILENAME - RENAME FILE
        AX = 7156h
        DS:DX -> ASCIZ old file or directory name (long names allowed)
        ES:DI -> ASCIZ new name (long names allowed)
Return: CF clear if successful
        CF set on error
            AX = error code
                7100h if function not supported
Note:   the file may be renamed into a different directory, but not across
          disks
SeeAlso: AH=56h,AX=7141h,AX=43FFh/BP=5053h
--------D-217160CL00-------------------------
INT 21 - Windows95 - LONG FILENAME - "TRUENAME" - CANONICALIZE PATH
        AX = 7160h
        CL = 00h
        CH = SUBST expansion flag
            00h return a path containing true path for a SUBSTed drive letter
            80h return a path containing the SUBSTed drive letter
        DS:SI -> ASCIZ filename or path (either long name or short name)
        ES:DI -> 261-byte buffer for canonicalized name
Return: CF set on error
            AX = error code
                02h invalid component in directory path or drive letter only
                03h malformed path or invalid drive letter
            ES:DI buffer unchanged
        CF clear if successful
            ES:DI buffer filled with fully qualified name
            AX destroyed
Desc:   determine the canonical name of the specified filename or path,
          corresponding to the undocumented TRUENAME command in COMMAND.COM
Notes:  if a complete path is given, the result will be a short-form complete
          path; otherwise, the given relative path is appended to the
          short-form current directory name, '.'/'..'/'...'/etc. are resolved,
          and the final result uppercased without converting any remaining
          long-form names to short-form
        for compatibility with DOS versions prior to v7.00, the carry flag
          should be set on call to ensure that it is set on exit
BUG:    Windows95 incorrectly treats filenames where the first two characters
          after the drive letter and colon are both slashes (either forward
          or backward) as a UNC (network name) and requires several seconds
          to attempt to resolve the name before returning an unchanged
          string
SeeAlso: AH=60h,AX=7160h/CL=01h
--------D-217160CL01-------------------------
INT 21 - Windows95 - LONG FILENAME - GET SHORT (8.3) FILENAME FOR FILE
        AX = 7160h
        CL = 01h
        CH = SUBST expansion flag
            00h return a path containing true path for a SUBSTed drive letter
            80h return a path containing the SUBSTed drive letter
        DS:SI -> ASCIZ long filename or path
        ES:DI -> 67-byte (possibly 128-byte) buffer for short filename
Return: CF set on error
            AX = error code
                02h invalid component in directory path or drive letter only
                03h malformed path or invalid drive letter
            ES:DI buffer unchanged
        CF clear if successful
            ES:DI buffer filled with equivalent short filename (full path,
                  even if relative path given, and all uppercase)
Note:   this call returns the short name for any long-filename portions of
          the provided pathname or filename
SeeAlso: AH=60h,AX=7160h/CL=00h,AX=7160h/CL=02h
--------D-217160CL02-------------------------
INT 21 - Windows95 - LONG FILENAME - GET CANONICAL LONG FILENAME OR PATH
        AX = 7160h
        CL = 02h
        CH = SUBST expansion flag
            00h return a path containing true path for a SUBSTed drive letter
            80h return a path containing the SUBSTed drive letter
        DS:SI -> ASCIZ short filename or path
        ES:DI -> 261-byte buffer for canonicalized long name
Return: CF set on error
            AX = error code
                02h invalid component in directory path or drive letter only
                03h malformed path or invalid drive letter
            ES:DI buffer unchanged
        CF clear if successful
            ES:DI buffer filled with qualified long name (can contain
                  lowercase letters)
Desc:   determine the canonical name of the specified filename or path,
          corresponding to the undocumented TRUENAME command in COMMAND.COM
BUGS:   even though the maximum length of a complete long pathname is 260
          characters, Windows95 returns CF set/AX=0003h even if the file
          exists whenever the full pathname is more than 255 characters
        Windows95 incorrectly treats filenames where the first two characters
          after the drive letter and colon are both slashes (either forward
          or backward) as a UNC (network name) and requires several seconds
          to attempt to resolve the name before returning an unchanged
          string
SeeAlso: AH=60h,AX=7160h/CL=00h,AX=7160h/CL=01h
--------D-21716C-----------------------------
INT 21 - Windows95 - LONG FILENAME - CREATE OR OPEN FILE
        AX = 716Ch
        BX = access mode and sharing flags (see #01782,also AX=6C00h)
        CX = attributes
        DX = action (see #01781)
        DS:SI -> ASCIZ filename
        DI = alias hint (number to append to short filename for disambiguation)
Return: CF clear if successful
            AX = file handle
            CX = action taken
                0001h file opened
                0002h file created
                0003h file replaced
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
SeeAlso: AX=6C00h,AX=7141h,AX=7156h,AX=71A9h

Bitfields for Windows95 long-name open action:
Bit(s)  Description     (Table 01781)
 0      open file (fail if file does not exist)
 1      truncate file if it already exists (fail if file does not exist)
 4      create new file if file does not already exist (fail if exists)
Note:   the only valid combinations of multiple flags are bits 4&0 and 4&1

Bitfields for Windows95 file access/sharing modes:
Bit(s)  Description     (Table 01782)
 2-0    file access mode
        000 read-only
        001 write-only
        010 read-write
        100 read-only, do not modify file's last-access time
 6-4    file sharing modes
 7      no-inherit flag
 8      do not buffer data (requires that all reads/writes be exact physical
          sectors)
 9      do not compress file even if volume normally compresses files
 10     use alias hint in DI as numeric tail for short-name alias
 12-11  unused??? (0)
 13     return error code instead of generating INT 24h if critical error
          while opening file
 14     commit file after every write operation
SeeAlso: #01402
--------D-2171A0-----------------------------
INT 21 - Windows95 - LONG FILENAME - GET VOLUME INFORMATION
        AX = 71A0h
        DS:DX -> ASCIZ root name (e.g. "C:\")
        ES:DI -> buffer for file system name
        CX = size of ES:DI buffer
Return: CF clear if successful
            AX destroyed (0000h and 0200h seen)
            BX = file system flags (see #01783)
            CX = maximum length of file name [usually 255]
            DX = maximum length of path [usually 260]
            ES:DI buffer filled (ASCIZ, e.g. "FAT","NTFS","CDFS")
        CF set on error
            AX = error code
                7100h if function not supported
Notes:  for the file system name buffer, 32 bytes should be sufficient; that's
         what is used in some sample code by Walter Oney from Microsoft.
        this function accesses the disk the first time it is called
BUG:    this function returns DX=0000h for CD-ROM drives under Win95 SP1
SeeAlso: AX=714Eh,AX=7160h/CL=00h

Bitfields for long filename volume information flags:
Bit(s)  Description     (Table 01783)
 0      searches are case sensitive
 1      preserves case in directory entries
 2      uses Unicode characters in file and directory names
 3-13   reserved (0)
 14     supports DOS long filename functions
 15     volume is compressed
--------D-2171A1-----------------------------
INT 21 - Windows95 - LONG FILENAME - "FindClose" - TERMINATE DIRECTORY SEARCH
        AX = 71A1h
        BX = filefind handle (from AX=714Eh)
Return: CF clear if successful
        CF set on error
           AX = error code
                7100h if function not supported
Notes:  this function must be called after starting a search with AX=714Eh,
          to indicate that the search handle returned by that function will
          no longer be used
        this function is only available when IFSMgr is running, not under bare
          MS-DOS 7
SeeAlso: AH=4Eh,AX=714Eh,AX=714Fh
--------D-2171A2-----------------------------
INT 21 U - Windows95 - internal - LONG FILENAME - FIND NEXT MATCHING FILE
        AX = 71A2h
        BX = filefind handle (from AX=714Eh)
        SI = date/time format (see #01778)
        ES:DI -> buffer for FindData record (see #01779)
Return: CF clear if successful
            AX = 71A2h
            CX = Unicode conversion flags (see #01780)
        CF set on error
            AX = error code
                7100h if function not supported
Notes:  this function is only available when IFSMgr is running, not under bare
          MS-DOS 7; it is functionally identical to AX=714Fh
        documented as "for internal use by Windows 95 only"
--------D-2171A3-----------------------------
INT 21 U - Windows95 - internal
        AX = 71A3h
        ???
Return: ???
Note:   documented as "for internal use by Windows 95 only"
--------D-2171A4-----------------------------
INT 21 U - Windows95 - internal
        AX = 71A4h
        ???
Return: ???
Note:   documented as "for internal use by Windows 95 only"
--------D-2171A5-----------------------------
INT 21 U - Windows95 - internal
        AX = 71A5h
        ???
Return: ???
Note:   documented as "for internal use by Windows 95 only"
--------D-2171A6-----------------------------
INT 21 - Windows95 - LONG FILENAME - GET FILE INFO BY HANDLE
        AX = 71A6h
        BX = file handle
        DS:DX -> buffer for file information (see #01784)
        CF set
Return: CF clear if successful
            file information record filled
        CF set on error
            AX = error code
                7100h if function not supported
SeeAlso: AX=71A7h/BL=00h

Format of Windows95 file information:
Offset  Size    Description     (Table 01784)
 00h    DWORD   file attributes
 04h    QWORD   creation time (0 = unsupported)
 0Ch    QWORD   last access time (0 = unsupported)
 14h    QWORD   last write time
 1Ch    DWORD   volume serial number
 20h    DWORD   high 32 bits of file size
 24h    DWORD   low 32 bits of file size
 28h    DWORD   number of links to file
 2Ch    DWORD   unique file identifier (high 32 bits)
 30h    DWORD   unique file identifier (low 32 bits)
Note:   the file identifer and volume serial number together uniquely identify
          a file while it is open; the identifier may change when the system
          is restarted or the file is first opened
--------D-2171A7BL00-------------------------
INT 21 - Windows95 - LONG FILENAME - FILE TIME TO DOS TIME
        AX = 71A7h
        BL = 00h
        DS:SI -> QWORD file time
Return: CF clear if successful
            CX = DOS time (see #01665)
            DX = DOS date (see #01666)
            BH = hundredths (10-millisecond units past time in CX)
        CF set on error
            AX = error code
                7100h if function not supported
Desc:   convert Win95 64-bit file time (UTC) into DOS-style date and time
          (local timezone)
Note:   the conversion fails if the file time's value is outside the range
          01jan1980 and 31dec2107
SeeAlso: AX=71A6h,AX=71A7h/BL=01h
--------D-2171A7BL01-------------------------
INT 21 - Windows95 - LONG FILENAME - DOS TIME TO FILE TIME
        AX = 71A7h
        BL = 01h
        CX = DOS time (see #01665)
        DX = DOS date (see #01666)
        BH = hundredths (10-millisecond units past time in CX)
        ES:DI -> buffer for QWORD file time
Return: CF clear if successful
            ES:DI buffer filled
        CF set on error
            AX = error code
                7100h if function not supported
Desc:   convert DOS-style date and time (local timezone) into Win95 64-bit
          file time (UTC)
SeeAlso: AX=71A6h,AX=71A7h/BL=00h
--------D-2171A8-----------------------------
INT 21 - Windows95 - LONG FILENAME - GENERATE SHORT FILENAME
        AX = 71A8h
        DS:SI -> ASCIZ long filename (no path allowed!)
        ES:DI -> buffer for ASCIZ short filename
        DH = short name's format
            00h 11-char directory entry/FCB filename format
            01h DOS 8.3
        DL = character sets
            bits 7-4: short name's character set (see #01785)
            bits 3-0: long name's character set (see #01785)
Return: CF clear if successful
            ES:DI buffer filled
        CF set on error
            AX = error code
                7100h if function not supported
Note:   this function uses the same algorithm as the filesystem except that
          the returned name never has a numeric tail for disambiguation
SeeAlso: AX=7160h/CL=00h,AX=7160h/CL=02h,AX=71A7h/BL=00h

(Table 01785)
Values for Windows95 filename character set:
 00h    Windows ANSI
 01h    current OEM character set
 02h    Unicode
--------D-2171A9-----------------------------
INT 21 - Windows95 - LONG FILENAME - SERVER CREATE OR OPEN FILE
        AX = 71A9h
        BX = access mode and sharing flags (see #01782,also AX=6C00h)
        CX = attributes
        DX = action (see #01781)
        DS:SI -> ASCIZ filename
        DI = alias hint (number to append to short filename for disambiguation)
Return: CF clear if successful
            AX = global file handle
            CX = action taken
                0001h file opened
                0002h file created
                0003h file replaced
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   for use by real-mode servers only
SeeAlso: AX=6C00h,AX=716Ch
--------D-2171AABH00-------------------------
INT 21 - Windows95 - LONG FILENAME - CREATE SUBST
        AX = 71AAh
        BH = 00h
        BL = drive number (00h = default, 01h = A:, etc.)
        DS:DX -> ASCIZ pathname to associate with drive letter
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
SeeAlso: AX=71AAh/BH=01h,AX=71AAh/BH=02h,INT 2F/AX=1000h,#01643
--------D-2171AABH01-------------------------
INT 21 - Windows95 - LONG FILENAME - TERMINATE SUBST
        AX = 71AAh
        BH = 01h
        BL = drive number (01h = A:, etc.)
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   the specified drive number may not be 00h (default), and presumably not
          the current drive either
SeeAlso: AX=71AAh/BH=00h,AX=71AAh/BH=02h,INT 2F/AX=1000h,#01643
--------D-2171AABH02-------------------------
INT 21 - Windows95 - LONG FILENAME - QUERY SUBST
        AX = 71AAh
        BH = 02h
        BL = drive number (01h = A:, etc.)
        DS:DX -> buffer for ASCIZ pathname associated with drive letter
Return: CF clear if successful
            DS:DX buffer filled
        CF set on error
            AX = error code (see #01680)
                7100h if function not supported
Note:   the specified drive number may not be 00h (default drive)
SeeAlso: AX=71AAh/BH=00h,AX=71AAh/BH=01h,INT 2F/AX=1000h,#01643
--------D-2172-------------------------------
INT 21 - Windows95 beta - LFN-FindClose
        AH = 72h
        details not available
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680)
                7200h if function not supported (e.g. under bare MS-DOS 7)
Note:   this function was present in beta versions of Windows95, but is
          probably not present in the release version
SeeAlso: AX=71A1h
--------D-2173-------------------------------
INT 21 - MS-DOS 7 - DRIVE LOCKING AND FLUSHING
        AH = 73h
        DL = drive (0=current, 1=A:, etc.)
        CL = which flag to get or set
            00h drive flag???
            01h drive's dirty-buffers flag
        AL = subfunction
            00h get indicated flag
            01h set ???
                CH = new value for indicated flag
                    bit 1: ??? (CL=00h only)
                    bit 3: dirty buffers exist for selected drive (CL=01h only)
Return: CF clear if successful
            for AL=00h:
                AL = value of CL on entry
                for CL=00h: AH = new flag and 06h (i.e. bits 1 and 2 used)
                for CL=01h: AH = new flag and 08h (i.e. bit 3 used)
                (flag being taken from a table of bytes)
        CF set on error
            AX = error code (01h,0Fh,etc.) (see #01680)
                7300h if function not supported
Note:   these two subfunctions are available even when only the MS-DOS kernel
          is running
--------D-217302-----------------------------
INT 21 - Windows95 - FAT32 - "Get_ExtDPB" - GET EXTENDED DPB
        AX = 7302h
        DL = drive number (00h=default, 01h=A:, etc.)
        ES:DI -> buffer for returned data (see #01786)
        CX = length of buffer (003Fh for Windows95)
        SI = signature (undocumented, must be F1A6h to get device driver
              address and next-DBP pointer) (see #01787)
Return: CF clear if successful
            ES:DI buffer filled
        CF set on error
            AX = error code
               0018h bad buffer length
SeeAlso: AX=7303h,AX=7304h,AH=1Fh,AH=32h

Format of Get_ExtDPB data buffer:
Offset  Size    Description     (Table 01786)
 00h    WORD    (call) length of following data (003Dh)
 02h 61 BYTEs   (ret) drive parameter block (DPB) (see #01787)

Format of Extended Drive Parameter Block:
Offset  Size    Description     (Table 01787)
 00h 24 BYTEs   standard DOS 4+ DPB
 18h    BYTE    "dpb_flags" (undocumented)
                FFh force media check
 19h    DWORD   pointer to next DPB (see note)
 1Dh    WORD    cluster at which to start search for free space when writing,
                usually the last cluster allocated
 1Fh    WORD    number of free clusters on drive, FFFFh = unknown
 21h    WORD    high word of free cluster count
 23h    WORD    active FAT/mirroring
                bit 7: do not mirror active FAT to inactive FATs
                bits 6-4: reserved (0)
                bits 3-0: the 0-based FAT number of the active FAT
                    (only meaningful if mirroring disabled)
 25h    WORD    sector number of file system information sector, or
                  FFFFh for none (see also #01788)
 27h    WORD    sector number of backup boot sector, or FFFFh for none
 29h    DWORD   first sector number of the first cluster
 2Dh    DWORD   maximum cluster number
 31h    DWORD   number of sectors occupied by FAT
 35h    DWORD   cluster number of start of root directory
 39h    DWORD   cluster number at which to start searching for free space
Notes:  except for offset 18h, all of the first 33 bytes are identical to
          the standard DOS 4-6 DPB
        unless the proper value is given in SI on entry to "Get_ExtDBP", the
          next-DPB pointer and device driver address are set to 0000h:0000h
SeeAlso: #01786,#01395 at AH=32h,#01664

Format of File System Information structure:
Offset  Size    Description     (Table 01788)
 00h    DWORD   signature 61417272h
 04h    DWORD   number of free clusters (FFFFFFFFh if unknown)
 08h    DWORD   most recently allocated cluster
 0Ch 12 BYTEs   reserved
SeeAlso: #01787
--------D-217303-----------------------------
INT 21 - Windows95 - FAT32 - GET EXTENDED FREE SPACE ON DRIVE
        AX = 7303h
        DS:DX -> ASCIZ string for drive ("C:\" or "\\SERVER\Share")
        ES:DI -> buffer for extended free space structure (see #01789)
        CX = length of buffer for extended free space
Return: CF clear if successful
            ES:DI buffer filled
        CF set on error
            AX = error code
Notes:  on DOS versions which do not support the FAT32 calls, this function
          returns CF clear/AL=00h (which is the DOS v1+ method for reporting
          unimplemented functions)
        under DOS 7.x (i.e. "MSDOS Mode" under Windows95), the ASCIZ string
          pointed at by DS:DX *must* include the drive letter, or this function
          will return CF set/AX=0015h (invalid drive).  In a DOS box, omitting
          the drive letter (DS:DX -> "\") results in the free space for the
          current default drive, as expected
BUG:    this function returns a maximum of 2GB free space even on an FAT32
          partition larger than 2GB under some versions of Win95 and Win98,
          apparently by limiting the number of reported free clusters to no
          more than 64K -- but only in a DOS window if a TSR has hooked INT 21h
SeeAlso: AX=7302h,AX=7304h,AX=7305h,AH=36h

Format of extended free space structure:
Offset  Size    Description     (Table 01789)
 00h    WORD    (ret) size of returned structure
 02h    WORD    (call) structure version (0000h)
                (ret) actual structure version (0000h)
 04h    DWORD   number of sectors per cluster (with adjustment for compression)
 08h    DWORD   number of bytes per sector
 0Ch    DWORD   number of available clusters
 10h    DWORD   total number of clusters on the drive
 14h    DWORD   number of physical sectors available on the drive, without
                  adjustment for compression
 18h    DWORD   total number of physical sectors on the drive, without
                  adjustment for compression
 1Ch    DWORD   number of available allocation units, without adjustment
                  for compression
 20h    DWORD   total allocation units, without adjustment for compression
 24h  8 BYTEs   reserved
--------D-217304-----------------------------
INT 21 - Windows95 - FAT32 - Set DPB TO USE FOR FORMATTING
        AX = 7304h
        DL = drive number (00h=default, 01h=A:, etc.)
        CX = size of buffer (must be at least 18h)
        ES:DI -> buffer for Set_DPBforFormat structure (see #01790)
Return: CF clear if successful
            ES:DI buffer updated
        CF set on error
            AX = error code (18h = bad length in CX)
SeeAlso: AX=7302h,AX=7303h,AX=7305h

Format of Set_DPBforFormat structure:
Offset  Size    Description     (Table 01790)
 00h    WORD    (call) size
 02h    WORD    (call) structure version???  (0000h)
                (ret) ???
 04h    DWORD   (call) function number
                00h invalidate DPB counts
                01h rebuild DPB from BPB
                02h force media change (next access to drive rebuild DPB)
                03h get/set active FAT number and mirroring
                04h get/set root directory cluster number
---function 00h---
 08h    DWORD   new DPB free count (00000000h=no change, FFFFFFFFh=unknown)
 0Ch    DWORD   new DPB next-free (00000000h=no change, FFFFFFFFh=unknown)
 10h    DWORD   unused
 14h    DWORD   unused
---function 01h---
 08h    DWORD   unused???
 0Ch    DWORD   (call) -> BIOS Parameter Block from which to rebuild DPB
 10h    DWORD   unused
 14h    DWORD   unused
---function 02h---
 08h    DWORD   unused
 0Ch    DWORD   unused
 10h    DWORD   unused
 14h    DWORD   unused
---function 03h---
 08h    DWORD   (call) new active FAT/mirroring state, or FFFFFFFFh to get
                bits 3-0: the 0-based FAT number of the active FAT
                bits 6-4: reserved (0)
                bit 7: do not mirror active FAT to inactive FATs
 0Ch    DWORD   (ret) previous active FAT/mirroring state (as above)
 10h    DWORD   unused
 14h    DWORD   unused
---function 04h---
 08h    DWORD   (call) new root directory cluster number
                    FFFFFFFFh to get current
 0Ch    DWORD   (ret) previous root directory cluster number
 10h    DWORD   unused
 14h    DWORD   unused
--------D-217305CXFFFF-----------------------
INT 21 - Windows95 - FAT32 - EXTENDED ABSOLUTE DISK READ/WRITE
        AX = 7305h
        CX = FFFFh
        DL = drive number (01h=A:, etc.)
        SI = read/write mode flags (see #01791)
        DS:BX -> disk I/O packet (see #02548 at INT 25)
Return: CF clear if successful
        CF set on error
            AX = error code
Note:   one can not specify the default drive (DL=00h) for this function.
SeeAlso: AX=7302h,AX=7304h,INT 25,INT 26

Bitfields for Extended Absolute Disk Read/Write mode flags:
Bit(s)  Description     (Table 01791)
 0      direction (0=read, 1=write)
 12-1   reserved (0)
 14-13  write type (should be 00 on reads)
        00 unknown data
        01 FAT data
        10 directory data
        11 file data
 15     reserved (0)
--------V-217734-----------------------------
INT 21 U - SCROLLit v1.7 - INSTALLATION CHECK
        AX = 7734h
Return: DX = 3477h if installed
            AX = segment of resident code
Program: ScrollIt is a shareware backscroll utility by Bromfield Software
          Products
--------U-217761-----------------------------
INT 21 - WATCH.COM v3.2+ - INSTALLATION CHECK
        AX = 7761h ('wa')
Return: AX = 6177h
Note:   WATCH.COM is part of the "TSR" package by TurboPower Software
SeeAlso: INT 16/AX=7761h
--------D-2180-------------------------------
INT 21 - European MS-DOS 4.0 - "AEXEC" - EXECUTE PROGRAM IN BACKGROUND
        AH = 80h
        CX = mode
            0000h place child in zombie mode on exit to preserve exit code
            0001h discard child process and exit code on termination
        DS:DX -> ASCIZ full program name
        ES:BX -> parameter block (as for AX=4B00h)
Return: CF clear if successful
            AX = Command Subgroup ID (CSID)
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
Program: European MS-DOS 4.0 was written for Siemens in Germany and then used
          by several other European OEMs; its release falls between mainstream
          versions 3.2 and 3.3
Desc:   asynchronously execute a program, creating a new process for it
Notes:  this function is called by the DETACH command
        there is a system-wide limit of 32 processes
        the CSID is used to identify all processes that have been spawned by
          a given process, whether directly or indirectly
        programs to be run in the background must use the new executable format
          (see #01596 at AH=4Bh)
        background processes may only perform asynchronous (background) EXECs,
          either this function or AX=4B04h
        background processes may execute INT 11, INT 12, INT 21, INT 2A, and
          INT 2F at any time; they may execute INT 10 and INT 16 only while
          they have opened a popup screen via INT 2F/AX=1401h; no other
          interrupts may be executed from the background
        background processes may not use drive B: or overlay their code
          segments
        see AX=8700h for an installation check
        the "NE" new executable format made its first appearance in European
          MS-DOS 4.0
SeeAlso: AH=4Bh,AH=87h,INT 2F/AX=1400h"POPUP"
----------218080-----------------------------
INT 21 - PCW Weather Card interface - UNINSTALL PCW.COM AND FREE MEMORY
        AX = 8080h
Return: ???
SeeAlso: AX=7070h/BX=7070h
--------D-2181-------------------------------
INT 21 - European MS-DOS 4.0 - "FREEZE" - STOP A PROCESS
        AH = 81h
        BX = flag (00h freeze command subtree, 01h only specified process)
        CX = Process ID of head of command subtree
Return: CF clear if successful
        CF set on error
            AX = error code (no such process)
Desc:   temporarily suspend a process or a process and all of its children
Note:   if BX=0001h, this call will not return until the process is actually
          frozen, which may not be until after it unblocks from an I/O
          operation
SeeAlso: AH=82h,AH=89h,AX=8E00h,INT 15/AX=101Dh
--------D-2182-------------------------------
INT 21 - European MS-DOS 4.0 - "RESUME" - RESTART A PROCESS
        AH = 82h
        BX = flag (00h resume command subtree, 01h only specified process)
        CX = Process ID of head of command subtree
Return: CF clear if successful
        CF set on error
            AX = error code (no such process)
Desc:   restart a previously-suspended process or a process and all of its
          children
SeeAlso: AH=81h,INT 15/AX=101Eh
--------D-2183-------------------------------
INT 21 - European MS-DOS 4.0 - "PARTITION" - GET/SET FOREGROUND PARTITION SIZE
        AH = 83h
        AL = function
            00h get size
            01h set new size
                BX = new size in paragraphs
Return: CF clear if successful
            BX = current size (function 00h) or old size (function 01h)
        CF set on error
            AX = error code (01h,07h,0Dh)(see #01680 at AH=59h/BX=0000h)
Desc:   specify or determine how much memory may be allocated by the foreground
          process
Note:   if the partition size is set to 0000h, no partition management is done
          and all memory allocation is compatible with DOS 3.2.
        the partition size can be changed regardless of what use is being made
          of the changed memory; subsequent allocations will follow the
          partition rules (foreground processes may allocate only foreground
          memory; background processes allocate background memory first, then
          foreground memory)
SeeAlso: AH=48h,AH=4Ah
--------D-218400-----------------------------
INT 21 - European MS-DOS 4.0 - "CREATMEM" - CREATE A SHARED MEMORY AREA
        AX = 8400h
        BX = size in bytes (0000h = 65536)
        CX = flags
            bit 6: zero-initialize segment
        DS:DX -> ASCIZ name (must begin with "\SHAREMEM\")
Return: CF clear if successful
            AX = segment address of shared memory global object
        CF set on error
            AX = error code (06h,08h) (see #01680 at AH=59h/BX=0000h)
Desc:   create an area of memory which may be accessed by multiple processes
Notes:  shared memory objects are created as special files (thus the
          restriction on the name)
        on successful creation, the reference count is set to 1
SeeAlso: AX=8401h,AX=8402h,INT 15/AX=DE19h
--------D-218401-----------------------------
INT 21 - European MS-DOS 4.0 - "GETMEM" - OBTAIN ACCESS TO SHARED MEMORY AREA
        AX = 8401h
        CX = flags
            bit 7: writable segment (ignored by MS-DOS 4.0)
        DS:DX -> ASCIZ name (must begin with "\SHAREMEM\")
Return: CF clear if successful
            AX = segment address of shared memory global object
            CX = size in bytes
        CF set on error
            AX = error code (invalid name)
Desc:   get address of a previously-created area of memory which may be
          accessed by multiple processes
Note:   this call increments the reference count for the shared memory area
SeeAlso: AX=8400h,AX=8402h
--------D-218402-----------------------------
INT 21 - European MS-DOS 4.0 - "RELEASEMEM" - FREE SHARED MEMORY AREA
        AX = 8402h
        BX = handle (segment address of shared memory object)
Return: CF clear if successful
        CF set on error
            AX = error code (no such name)
Desc:   indicate that the specified area of shared memory will no longer be
          used by the caller
Note:   the reference count is decremented and the shared memory area is
          deallocated if the new reference count is zero
SeeAlso: AX=8400h,AX=8401h,INT 15/AX=DE19h
--------D-2185-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 85h
        ???
Return: ???
--------D-2186-------------------------------
INT 21 - European MS-DOS 4.0 - "SETFILETABLE" - INSTALL NEW FILE HANDLE TABLE
        AH = 86h
        BX = total number of file handles in new table
Return: CF clear if successful
        CF set on error
            AX = error code (06h,08h) (see #01680 at AH=59h/BX=0000h)
Desc:   adjust the size of the per-process open file table, thus raising or
          lowering the limit on the number of files the caller can open
          simultaneously
Notes:  any currently-open files are copied to the new table
        if the table is increased beyond the default 20 handles, only the
          first 20 will be inherited by child processes
        error 06h is returned if the requested number of handles exceeds
          system limits or would require closing currently-open files
SeeAlso: AH=26h,AH=67h
--------D-2187-------------------------------
INT 21 - European MS-DOS 4.0 - "GETPID" - GET PROCESS IDENTIFIER
        AH = 87h
Return: AX = PID
        BX = parent process's PID
        CX = Command Subgroup ID (CSID)
Program: European MS-DOS 4.0 was written for Siemens in Germany and then used
          by several other European OEMs; its release falls between mainstream
          versions 3.2 and 3.3
Desc:   determine an identifier by which to access the calling process
Notes:  called by MS C v5.1 getpid() function
        this function apparently must return AX=0001h for INT 21/AH=80h to
          succeed
        one possible check for European MS-DOS 4.0 is to issue this call with
          AL=00h and check whether AL is nonzero on return
SeeAlso: AH=30h,AH=62h,AH=80h
Index:  installation check;European MS-DOS 4.0
--------D-2188-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 88h
        ???
Return: ???
SeeAlso: AH=87h
--------D-2189-------------------------------
INT 21 - European MS-DOS 4.0 - SLEEP
        AH = 89h
        CX = time in milliseconds or 0000h to give up time slice
Return: CF clear if successful
            CX = 0000h
        CF set on error
            AX = error code (interrupted system call)
            CX = sleep time remaining
Desc:   suspend the calling process for the specified duration
Notes:  the sleep interval is rounded up to the next higher increment of the
          scheduler clock, and may be extended further if other processes are
          running
        this call may be interrupted by signals (see AH=8Dh)
        reportedly called by Microsoft C 4.0 startup code
        background processes have higher priority than the foreground process,
          and should thus periodically yield the CPU
SeeAlso: AH=81h,INT 15/AX=1000h,INT 2F/AX=1680h,INT 7A/BX=000Ah
--------D-218A-------------------------------
INT 21 - European MS-DOS 4.0 - "CWAIT" - WAIT FOR CHILD TO TERMINATE
        AH = 8Ah
        BL = range (00h command subtree, 01h any child)
        BH = suspend flag
            00h suspend if children exist but none are dead
            01h return if no dead children
        CX = Process ID of head of command subtree
Return: CF clear if successful
            AH = termination type (see #01792)
            AL = return code from child or aborting signal
            BX = PID of child (0000h if no dead children)
        CF set on error
            AX = error code (no child,interrupted system call)
Desc:   get return code from an asynchronously-executed child program,
          optionally waiting if no return code is available
SeeAlso: AH=4Bh,AH=4Dh,AH=80h,AH=8Dh

(Table 01792)
Values for termination type:
 00h    normal termination
 01h    aborted by Control-C
 02h    aborted by I/O error
 03h    terminate and stay resident
 04h    aborted by signal
 05h    aborted by program error
--------D-218B-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 8Bh
        ???
Return: ???
SeeAlso: AH=87h
--------D-218C-------------------------------
INT 21 - European MS-DOS 4.0 - SET SIGNAL HANDLER
        AH = 8Ch
        AL = signal number (see #01793)
        BL = action (see #01794)
        DS:DX -> signal handler (see #01795)
Return: CF clear if successful
            AL = previous action
            ES:BX -> previous signal handler
        CF set on error
            AX = error code (01h,invalid SigNumber or Action)
                  (see #01680 at AH=59h/BX=0000h)
Desc:   set the routine which will be invoked on a number of exceptional
          conditions
Note:   all signals will be sent to the most recently installed handler
SeeAlso: AH=8Dh

(Table 01793)
Values for European MS-DOS 4.0 signal number:
 01h    SIGINTR         Control-C or user defined interrupt key
 08h    SIGTERM         program termination
 09h    SIGPIPE         broken pipe
 0Dh    SIGUSER1        reserved for user definition
 0Eh    SIGUSER2        reserved for user definition

(Table 01794)
Values for signal action:
 00h    SIG_DFL         terminate process on receipt
 01h    SIG_IGN         ignore signal
 02h    SIG_GET         signal is accepted
 03h    SIG_ERR         sender gets error
 04h    SIG_ACK         acknowledge received signal and clear it, but don't
                          change current setting

(Table 01795)
Values signal handler is called with:
        AL = signal number (see #01793)
        AH = signal argument
Return: RETF, CF set: terminate process
        RETF, CF clear, ZF set: abort any interrupted system call with an error
        RETF, CF clear, ZF clear: restart any interrupted system call
        IRET: restart any interrupted system call
Note:   the signal handler may also perform a nonlocal GOTO by resetting the
          stack pointer and jumping; before doing so, it should dismiss the
          signal by calling this function with BL=04h
--------D-218D-------------------------------
INT 21 - European MS-DOS 4.0 - SEND SIGNAL
        AH = 8Dh
        AL = signal number (see #01793)
        BH = signal argument
        BL = action
            00h send to entire command subtree
            01h send only to specified process
        DX = Process ID
Return: CF clear if successful
        CF set on error
            AX = error code (01h,06h)(see #01680 at AH=59h/BX=0000h)
Desc:   invoke the exceptional-condition handler for the specified process
Note:   error 06h may be returned if one or more of the affected processes
          have an error handler for the signal
SeeAlso: AH=8Ch
--------D-218E00BH00-------------------------
INT 21 - European MS-DOS 4.0 - "SETPRI" - GET/SET PROCESS PRIORITY
        AX = 8E00h
        BH = 00h
        BL = action
            00h set priority for command subtree
            01h set priority for specified process only
        CX = Process ID
        DH = 00h
        DL = change in priority (00h to get priority)
Return: CF clear if successful
            DL = process priority
            DH destroyed
        CF set on error
            AX = error code (01h,no such process)(see #01680 at AH=59h)
Desc:   specify or determine the execution priority of the specified process
          or the process and all of its children
SeeAlso: AH=81h
--------D-218F-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 8Fh
        ???
Return: ???
SeeAlso: AH=87h
--------D-2190-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 90h
        ???
Return: ???
SeeAlso: AH=87h
----------2190--CXFFFF-----------------------
INT 21 U - PTS-DOS 6.51 & S/DOS 1.0 - DIRECT DISK READ
        AH = 90h
        CX = FFFFh (for partitions > 32 Mb)
            DS:BX -> request block (see #04094)
        CX = sector count (for partitions < 32 Mb)
            DX = starting sector number
            DS:BX -> buffer
Return: CF set on error
            AX error code
        CF clear on success
Notes:  This function is supported by Paragon Technology Systems S/DOS 1.0
          (and most probably by PTS-DOS 6.51, maybe even earlier).
        To access big partitions (> 32 Mb), the CX != FFFFh is not allowed and
          the buffer must always be used. Otherwise, the system will return
          error code 0207h.
        It is currently unknown if this function is also supported by
          PhysTechSoft PTS-DOS 6.60, 6.70, or 2000.
SeeAlso: INT 21/AH=91h"PTS",INT 25h,INT 26h

Format of PTS-DOS & S/DOS disk read/write request block:
Offset  Size    Description     (Table 04094)
 00h    DWORD   sector number
04h     WORD    counter
06h     DWORD   pointer to buffer
--------D-2191-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 91h
        ???
Return: ???
SeeAlso: AH=87h
----------2191--CXFFFF-----------------------
INT 21 U - PTS-DOS 6.51 & S/DOS 1.0 - DIRECT DISK WRITE
        AH = 91h
        CX = FFFFh (for partitions > 32 Mb)
            DS:BX -> request block (see #04094)
        CX = sector count (for partitions < 32 Mb)
            DX = starting sector number
            DS:BX -> buffer
Return: CF set on error
            AX error code
        CF clear on success
Note:   This function is supported by Paragon Technology Systems S/DOS 1.0
          (and most probably by PTS-DOS 6.51, maybe even earlier).
        To access big partitions (> 32 Mb), CX != FFFFh is not allowed and
          the buffer must always be used. Otherwise, the system will return
          error code 0207h.
        It is currently unknown if this function is also supported by
          PhysTechSoft PTS-DOS 6.60, 6.70, or 2000.
SeeAlso: INT 21/Ah=90h"PTS",INT 25h,INT 26h
--------D-2192-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 92h
        ???
Return: ???
SeeAlso: AH=87h
--------O-2192-------------------------------
INT 21 U - PTS-DOS 6.51 & S/DOS 1.0 - ABORT CURRENT PROCESS VIA INT 23
        AH = 92h
Desc:   This function is provided by Paragon Technology Systems S/DOS 1.0+
          (and most probably also by PTS-DOS 6.51+, maybe even earlier).
          It allows terminating programs via the CTRL+BREAK method.
Note:   It is currently unknown if this function is also supported by
          PhysTechSoft PTS-DOS 6.60, 6.70, or 2000.
SeeAlso: INT 23,INT 24
--------D-2193-------------------------------
INT 21 - European MS-DOS 4.0 - "PIPE" - CREATE A NEW PIPE
        AH = 93h
        CX = size in bytes
Return: CF clear if successful
            AX = read handle
            BX = write handle
        CF set on error
            AX = error code (08h) (see #01680 at AH=59h/BX=0000h)
Desc:   create a communications channel which may be used for interprocess
          data and command exchanges
SeeAlso: AH=3Ch,AH=3Fh"DOS",AH=40h"DOS",AH=84h
--------D-2194-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 94h
        ???
Return: ???
SeeAlso: AH=87h
--------D-2195-------------------------------
INT 21 - European MS-DOS 4.0 - HARD ERROR PROCESSING
        AH = 95h
        AL = new state
           00h enabled
           01h disabled, automatically fail hard errors
Return: AX = previous setting
Desc:   specify whether hard (critical) errors should automatically fail the
          system call or invoke an INT 24
SeeAlso: INT 24,INT 21/AH=59h/BX=0001h
--------D-2196-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 96h
        ???
Return: ???
--------D-2197-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 97h
        ???
Return: ???
--------D-2198-------------------------------
INT 21 U - European MS-DOS 4.0 - ???
        AH = 98h
        ???
Return: ???
--------D-2199-------------------------------
INT 21 u - European MS-DOS 4.0 - "PBLOCK" - BLOCK A PROCESS
        AH = 99h
        DS:BX -> memory location to block on
        CX = timeout in milliseconds
        DH = nonzero if interruptable
Return: CF clear if awakened by event
            AX = 0000h
        CF set if unusual wakeup
            ZF set if timeout, clear if interrupted by signal
            AX = nonzero
Desc:   suspend calling process until another process sends a "restart" signal
          or a timeout occurs
SeeAlso: AH=9Ah,INT 2F/AX=0802h
--------D-219A-------------------------------
INT 21 u - European MS-DOS 4.0 - "PRUN" - UNBLOCK A PROCESS
        AH = 9Ah
        DS:BX -> memory location processes may have blocked on
Return: AX = number of processes awakened
        ZF set if no processes awakened
Program: European MS-DOS 4.0 was written for Siemens in Germany and then used
          by several other European OEMs; its release falls between mainstream
          versions 3.2 and 3.3
Desc:   restart all processes waiting for the specified "restart" signal
SeeAlso: AH=99h,INT 2F/AX=0802h
--------I-21A0-------------------------------
INT 21 - Attachmate Extra! - GET 3270 DISPLAY STATE
        AH = A0h
Return: AL = display status (see #01796)
        BX = host window status (see #01797)
Program: Attachmate Extra! is a 3270 emulator by Attachmate Corporation
SeeAlso: AH=A1h

Bitfields for Attachmate Extra! display status:
Bit(s)  Description     (Table 01796)
 7      0=windowed, 1=enlarged
 6-3    current screen profile number 0-9
 2-0    active window number (0=PC, 1-4=host B-E, 5-6=notepad F-G)

Bitfields for host window status:
Bit(s)  Description     (Table 01797)
 15     reserved
 14     0=host E window installed, 1=not
 13     0=host E terminal on, 1=off
 12     0=host E window displayed, 1=not
 11     reserved
 10     0=host D window installed, 1=not
 9      0=host D terminal on, 1=off
 8      0=host D window displayed, 1=not
 7      reserved
 6      0=host C window installed, 1=not
 5      0=host C terminal on, 1=off
 4      0=host C window displayed, 1=not
 3      reserved
 2      0=host B window installed, 1=not
 1      0=host B terminal on, 1=off
 0      0=host B window displayed, 1=not
--------I-21A1-------------------------------
INT 21 - Attachmate Extra! - SET 3270 DISPLAY STATE
        AH = A1h
        AL = new display status byte (see #01796)
SeeAlso: AH=A0h,AH=A2h
--------I-21A2-------------------------------
INT 21 - Attachmate Extra! - SET HOST WINDOW STATE
        AH = A2h
        AL = new host window status byte (see #01798)
SeeAlso: AH=A1h

Bitfields for Attachmate Extra! host window status:
Bit(s)  Description     (Table 01798)
 7      0=power off, 1=power on
 6      0=not installed, 1=installed
 5-3    reserved
 2-0    window number 1-4=host B-E
--------I-21A3-------------------------------
INT 21 - Attachmate Extra! - SEND KEYSTROKES TO HOST WINDOW
        AH = A3h
        AL = window number (1-4=host B-E)
        CX = 0001h
        DS:BX -> keystroke buffer
        DL = zero if keystroke buffer contains host function code (see #01799),
              non-zero if keystroke buffer contains ASCII character
Return: CX = zero if character sent, non-zero if not
        BX incremented if CX=0

(Table 01799)
Values for Attachmate Extra! host function code:
 00h=reserved   10h=PF16        20h=Clear       30h=SysReq
 01h=PF1        11h=PF17        21h=Print       31h=ErInp
 02h=PF2        12h=PF18        22h=Left        32h=ErEof
 03h=PF3        13h=PF19        23h=Right       33h=Ident
 04h=PF4        14h=PF20        24h=Up          34h=Test
 05h=PF5        15h=PF21        25h=Down        35h=Reset
 06h=PF6        16h=PF22        26h=Home        36h=DevCncl
 07h=PF7        17h=PF23        27h=Fast Left   37h=Dup
 08h=PF8        18h=PF24        28h=Fast Right  38h=FldMark
 09h=PF9        19h=Alt on      29h=Bksp        39h=Enter
 0Ah=PF10       1Ah=Alt off     2Ah=Insert      3Ah=CrSel
 0Bh=PF11       1Bh=Shift on    2Bh=Delete
 0Ch=PF12       1Ch=Shift off   2Ch=Backtab
 0Dh=PF13       1Dh=PA1         2Dh=Tab
 0Eh=PF14       1Eh=PA2         2Eh=Newline
 0Fh=PF15       1Fh=PA3         2Fh=Attn
--------I-21A4-------------------------------
INT 21 - Attachmate Extra! - GET HOST WINDOW BUFFER ADDRESS
        AH = A4h
        AL = window number (1-4=host B-E)
Return: DS:BX -> 3270 display buffer
SeeAlso: AH=A5h,AH=B8h
--------I-21A5-------------------------------
INT 21 - Attachmate Extra! - GET HOST WINDOW CURSOR POSITION
        AH = A5h
        AL = window number (1-4=host B-E)
Return: BX = cursor position (80 * row + column, where 0:0 is upper left)
Note:   if the host window is configured with the Extended Attribute (EAB)
          feature, multiply the cursor position by 2 to obtain the byte offset
          into the display buffer
SeeAlso: AH=A4h
--------I-21AF-------------------------------
INT 21 - Attachmate Extra! - GET TRANSLATE TABLE ADDRESS
        AH = AFh
Return: DS:BX -> translate tables (see #01800)

Format of Attachmate Extra! translate tables:
Offset  Size    Description     (Table 01800)
 00h 256 BYTEs  ASCII to 3270 buffer code translate table
100h 256 BYTEs  3270 buffer code to ASCII translate table
200h 256 BYTEs  3270 buffer code to EBCDIC translate table
300h 256 BYTEs  EBCDIC to 3270 buffer code translate table
--------I-21B8-------------------------------
INT 21 - Attachmate Extra! - DISABLE HOST BUFFER UPDATES
        AH = B8h
        AL = window number (1-4=host B-E)
        DL = 01h
Notes:  only valid in CUT mode
        next AID keystroke (eg Enter) enables host buffer updates
SeeAlso: AH=A4h
--------d-21DC-------------------------------
INT 21 - PCMag PCMANAGE/DCOMPRES - TURN ON/OFF
        AH = DCh
        DX = state
            0000h turn on
            0001h turn off
SeeAlso: AX=FEDCh
--------T-21DF00DX534C-----------------------
INT 21 U - Software Carousel - INSTALLATION CHECK
        AX = DF00h
        DX = 534Ch ("SL")
        DI = 534Ch ("SL")
Return: AX = 00FFh if installed
            ???
Program: Software Carousel is a task switcher by SoftLogic Solutions, Inc.
--------T-21DF01-----------------------------
INT 21 - Software Carousel - SWITCH TO ANOTHER TASK
        AX = DF01h
        BL = task number (00h = next task)
Return: AL = status
            00h Carousel not running
            01h successful
            FFh unsucessful
                AH = error code (01h,02h) (see #01813)

(Table 01813)
Values for Software Carousel error code:
 00h    invalid subfunction in AL
 01h    invalid task number
 02h    tried to switch to task with no memory size
 03h    tried to kill program in partition with no program running
 04h    tried to change size of an active memory partition
 05h    invalid memory size
 06h    tried to send command to task with a pending previous command
--------T-21DF02-----------------------------
INT 21 - Software Carousel - KILL PROGRAM IN MEMORY PARTITION
        AX = DF02h
        BL = task number
Return: AL = status
            00h Carousel not running
            01h successful
            FFh unsucessful
                AH = error code (01h,03h) (see #01813)
--------T-21DF03-----------------------------
INT 21 - Software Carousel - GET PARTITION SIZE AND PROGRAM STATUS
        AX = DF03h
        BL = task number
Return: AL = status
            00h Carousel not running
            01h successful
                BL = partition state (00h no program running, 01h prog running)
                DX = partition size in KB
            FFh unsucessful
                AH = error code (01h) (see #01813)
SeeAlso: AX=DF05h
--------T-21DF04-----------------------------
INT 21 - Software Carousel - GET PARTITION NAME
        AX = DF04h
        BL = task number
Return: AL = status
            00h Carousel not running
            01h successful
                CX = length of name (00h if default partition name)
                ES:BX -> partition name (if CX nonzero)
            FFh unsucessful
                AH = error code (01h) (see #01813)
SeeAlso: AX=DF06h
--------T-21DF05-----------------------------
INT 21 - Software Carousel - CHANGE PARTITION SIZE
        AX = DF05h
        BL = task number
        DX = new size in KB
Return: AL = status
            00h Carousel not running
            01h successful
            FFh unsucessful
                AH = error code (01h,04h,05h) (see #01813)
        BX = minimum size allowed
        CX = maximum size available
SeeAlso: AX=DF03h
--------T-21DF06-----------------------------
INT 21 - Software Carousel - CHANGE PARTITION NAME
        AX = DF06h
        BL = task number
        CX = length of new name (00h to use default, max 18h)
        DS:SI -> new name
Return: AL = status
            00h Carousel not running
            01h successful
            FFh unsucessful
                AH = error code (01h) (see #01813)
SeeAlso: AX=DF04h
--------T-21DF07-----------------------------
INT 21 - Software Carousel - SEND COMMAND TO MEMORY SECTION
        AX = DF07h
        BL = task number
        CX = length of command (max 8 chars)
        DS:SI -> command line
Return: AL = status
            00h Carousel not running
            01h successful
            FFh unsucessful
                AH = error code (01h,06h) (see #01813)
Note:   the maximum length seems too small and may be a typo for 80 characters
--------T-21DF08-----------------------------
INT 21 - Software Carousel - SELECTIVELY ENABLE/DISABLE MENU AND SWITCHING
        AX = DF08h
        BL = new state of keyboard (00h disabled, 01h enabled)
Return: AL = status
            00h Carousel not running
            01h successful
Program: Software Carousel is a task switcher by SoftLogic Solutions, Inc.
Note:   when the keyboard is is disabled, the user may neither access the
          Carousel menu nor switch to another memory section
--------T-21DF09-----------------------------
INT 21 - Software Carousel - BOOT THE SYSTEM
        AX = DF09h
Return: AL = status
            00h Carousel not running
            FFh unsucessful
                AH = error code (01h,03h) (see #01813)
Note:   this function never returns if successful
--------T-21DF0A-----------------------------
INT 21 - Software Carousel - GET MEMORY SIZE/PARTITION NUMBER OF CURRENT TASK
        AX = DF0Ah
Return: AL = status
            00h Carousel not running
            01h successful
                BL = task number
                DX = memory size in KB
            FFh unsucessful
                AH = error code (01h,03h) (see #01813)
--------T-21DF0B-----------------------------
INT 21 - Software Carousel - SET TASK SWITCH CALLBACK
        AX = DF0Bh
        BH = interrupt number or 00h
        BL = function number to invoke on partition switch
        CL = function number to call when it is safe for resident programs
                to perform DOS calls
        DS:DX -> FAR function to call if BH=00h
Return: AL = status
            00h Carousel not running
            01h successful
            FFh unsucessful
                AH = error code (01h,03h) (see #01813)
Notes:  the specified interrupt or FAR function is called with AH set to the
          appropriate one of the values specified in BL and CL, and BL set to
          the new task number
        the function specified by CL will not be called until the notification
          is enabled with AX=DF0Ch
SeeAlso: AX=DF0Ch
--------T-21DF0C-----------------------------
INT 21 - Software Carousel - ENABLE DOS-CALL SAFETY NOTIFICATION
        AX = DF0Ch
Return: AL = status
            00h Carousel not running
            01h successful
Program: Software Carousel is a task switcher by SoftLogic Solutions, Inc.
SeeAlso: AX=DF0Bh
--------O-21E0-------------------------------
INT 21 - Digital Research DOS Plus - CALL BDOS
        AH = E0h
        CL = BDOS function number (see #04019 at INT E0"CP/M-86")
        other registers as appropriate for function
Return: as appropriate for function
SeeAlso: AX=4459h,INT E0"CP/M-86"
--------E-21E0-------------------------------
INT 21 - OS/286, OS/386 - INITIALIZE REAL PROCEDURE
        AH = E0h
        ???
Return: ???
SeeAlso: AH=E1h"OS/286"
--------T-21E0-------------------------------
INT 21 - DoubleDOS - MENU CONTROL
        AH = E0h
        AL = subfunction
            01h exchange tasks
            73h resume invisible job if suspended
            74h kill other job
            75h suspend invisible job
Note:   identical to AH=F0h
SeeAlso: AH=F0h"DoubleDOS"
--------E-21E1-------------------------------
INT 21 - OS/286, OS/386 - ISSUE REAL PROCEDURE CALL
        AH = E1h
        ???
Return: ???
Note:   protected mode only???
SeeAlso: AH=E0h"OS/286",AH=E2h"OS/286",AH=E3h"OS/286",AX=250Eh,INT 31/AX=0301h
--------T-21E1-------------------------------
INT 21 - DoubleDOS - CLEAR KEYBOARD BUFFER FOR CURRENT JOB
        AH = E1h
SeeAlso: AH=E2h"DoubleDOS",AH=E3h"DoubleDOS",AH=E8h"DoubleDOS"
SeeAlso: AH=F1h"DoubleDOS"
--------E-21E2-------------------------------
INT 21 - OS/286, OS/386 - SET REAL PROCEDURE SIGNAL HANDLER
        AH = E2h
        ???
Return: ???
SeeAlso: AH=E0h"OS/286",AH=E1h"OS/286",AH=E6h"OS/286"
--------!---Section--------------------------
