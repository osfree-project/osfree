Copyright (c) 1989-1999,2000 Ralf Brown
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
--------U-216262SI1994-----------------------
INT 21 - ENVLOCK - INSTALLATION CHECK
        AX = 6262h
        SI = 1994h
Return: AX = 1994h if installed
            ES = ENVLOCK's resident segment
Notes:  to deactivate ENVLOCK, zero out the byte at ES:[0102h]
Program: ENVLOCK is a TSR by Alexander Yanovsky that forces other TSRs to
          deallocate their environment when they stay resident
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
----------218080-----------------------------
INT 21 - PCW Weather Card interface - UNINSTALL PCW.COM AND FREE MEMORY
        AX = 8080h
Return: ???
SeeAlso: AX=7070h/BX=7070h
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
--------O-2192-------------------------------
INT 21 U - PTS-DOS 6.51 & S/DOS 1.0 - ABORT CURRENT PROCESS VIA INT 23
        AH = 92h
Desc:   This function is provided by Paragon Technology Systems S/DOS 1.0+
          (and most probably also by PTS-DOS 6.51+, maybe even earlier).
          It allows terminating programs via the CTRL+BREAK method.
Note:   It is currently unknown if this function is also supported by
          PhysTechSoft PTS-DOS 6.60, 6.70, or 2000.
SeeAlso: INT 23,INT 24
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
