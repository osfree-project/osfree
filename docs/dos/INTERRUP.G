Copyright (c) 1989-1999,2000 Ralf Brown
--------N-214402-----------------------------
INT 21 - Network Driver Interface Specification (NDIS) 2.0.1 - PROTOCOL MANAGER
        AX = 4402h
        BX = file handle for device "PROTMAN$"
        CX = 000Eh (size of request block)
        DS:DX -> request block (see #01424,#01425,#01426,#01427,#01428,#01432,#01433)
Return: CF clear if successful
            AX = number of bytes actually read
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4402h"FTPSOFT"

Format of NDIS request block for GetProtocolManagerInfo:
Offset  Size    Description     (Table 01424)
 00h    WORD    01h
 02h    WORD    returned status (see #01434)
 04h    DWORD   returned pointer to structure representing parsed user config
 08h    DWORD   unused
 0Ch    WORD    returned BCD version of NDIS on which Protocol Manager is based
SeeAlso: #01425,#01426,#01427,#01428,#01429,#01430,#01431,#01432,#01433

Format of NDIS request block for RegisterModule:
Offset  Size    Description     (Table 01425)
 00h    WORD    02h
 02h    WORD    returned status (see #01434)
 04h    DWORD   pointer to module's common characteristics table (see #01435)
 08h    DWORD   pointer to list of modules to which the module is to be bound
 0Ch    WORD    unused
SeeAlso: #01424,#01426,#01427,#01428,#01429,#01430,#01431,#01432,#01433

Format of NDIS request block for BindAndStart:
Offset  Size    Description     (Table 01426)
 00h    WORD    03h
 02h    WORD    returned status (see #01434)
 04h    DWORD   caller's virtual address in FailingModules structure
 08h    DWORD   unused
 0Ch    WORD    unused
SeeAlso: #01424,#01425,#01427,#01428,#01429,#01430,#01431,#01432,#01433

Format of NDIS request block for GetProtocolManagerLinkage:
Offset  Size    Description     (Table 01427)
 00h    WORD    04h
 02h    WORD    returned status (see #01434)
 04h    DWORD   returned dispatch point
 08h    DWORD   unused
 0Ch    WORD    returned protocol manager DS
Note:   the dispatch point may be called as follows instead of using this IOCTL
        STACK: WORD  protocol manager DS
               DWORD pointer to request block
        Return: AX = returned status
                STACK popped
SeeAlso: #01424,#01425,#01426,#01428,#01429,#01430,#01431,#01432,#01433

Format of NDIS request block for GetProtocolIniPath:
Offset  Size    Description     (Table 01428)
 00h    WORD    05h
 02h    WORD    returned status (see #01434)
 04h    DWORD   pointer to a buffer for the ASCIZ pathname of PROTOCOL.INI
 08h    DWORD   unused
 0Ch    WORD    buffer length
SeeAlso: #01424,#01425,#01426,#01427,#01429,#01430,#01431,#01432,#01433

Format of NDIS request block for RegisterProtocolManagerInfo:
Offset  Size    Description     (Table 01429)
 00h    WORD    06h
 02h    WORD    returned status (see #01434)
 04h    DWORD   pointer to structure containing parsed user config file
 08h    DWORD   unused
 0Ch    WORD    length of structure
SeeAlso: #01424,#01425,#01426,#01427,#01428,#01430,#01431,#01432,#01433

Format of NDIS request block for InitAndRegister:
Offset  Size    Description     (Table 01430)
 00h    WORD    07h
 02h    WORD    returned status (see #01434)
 04h    DWORD   unused
 08h    DWORD   poitner to ASCIZ name of the module to be prebind initialized
 0Ch    WORD    unused
SeeAlso: #01424,#01425,#01426,#01427,#01428,#01429,#01431,#01432,#01433

Format of NDIS request block for UnbindAndStop:
Offset  Size    Description     (Table 01431)
 00h    WORD    08h
 02h    WORD    returned status (see #01434)
 04h    DWORD   failing modules as for BindAndStart
 08h    DWORD   if not 0000h:0000h, pointer to ASCIZ name of module to unbind
                if 0000h:0000h, terminate a set of previously dynamically
                  bound protocol modules
 0Ch    WORD    unused
SeeAlso: #01424,#01425,#01426,#01427,#01428,#01429,#01430,#01432,#01433

Format of NDIS request block for BindStatus:
Offset  Size    Description     (Table 01432)
 00h    WORD    09h
 02h    WORD    returned status (see #01434)
 04h    DWORD   must be 0000h:0000h
                on return, points to root tree
 08h    DWORD   0000h:0000h
 0Ch    WORD    unused under DOS
SeeAlso: #01424,#01425,#01426,#01427,#01428,#01429,#01430,#01431,#01433

Format of NDIS request block for RegisterStatus:
Offset  Size    Description     (Table 01433)
 00h    WORD    0Ah
 02h    WORD    returned status (0000h, 0008h, 002Ch) (see #01434)
 04h    DWORD   0000h:0000h
 08h    DWORD   pointer to 16-byte ASCIZ module name
 0Ch    WORD    0000h
Note:   not supported by the 10NET v5.0 PROTMAN$ driver
SeeAlso: #01424,#01425,#01426,#01427,#01428,#01429,#01430,#01431,#01432

(Table 01434)
Values for NDIS status code:
 0000h  success
 0001h  wait for release--protocol has retained control of the data buffer
 0002h  request queued
 0003h  frame not recognized
 0004h  frame rejected
 0005h  frame should be forwarded
 0006h  out of resource
 0007h  invalid parameter
 0008h  invalid function
 0009h  not supported
 000Ah  hardware error
 000Bh  transmit error
 000Ch  unrecognized destination
 000Dh  buffer too small
 0020h  already started
 0021h  binding incomplete
 0022h  driver not initialized
 0023h  hardware not found
 0024h  hardware failure
 0025h  configuration failure
 0026h  interrupt conflict
 0027h  MAC incompatible
 0028h  initialization failed
 0029h  no binding
 002Ah  network may be disconnected
 002Bh  incompatible OS version
 002Ch  already registered
 002Dh  path not found
 002Eh  insufficient memory
 002Fh  info not found
 00FFh  general failure
 F000h-FFFFh reserved for vendor-specific codes, treated as general failure

Format of NDIS common characteristics table:
Offset  Size    Description     (Table 01435)
 00h    WORD    size of table in bytes
 02h    BYTE    NDIS major version
 03h    BYTE    NDIS minor version
 04h    WORD    reserved
 06h    BYTE    module major version
 07h    BYTE    module minor version
 08h    DWORD   module function flag bits
                bit 0: binding at upper boundary supported
                bit 1: binding at lower boundary supported
                bit 2: dynamically bound
                bits 3-31 reserved, must be 0
 0Ch 16 BYTEs   ASCIZ module name
 1Ch    BYTE    upper boundary protocol level (see #01436)
 1Dh    BYTE    upper boundary interface type
                for MACs: 1 = MAC
                for data links and transports: to be defined
                for session: 1 = NCB
                any level: 0 = private (ISV-defined)
 1Eh    BYTE    lower boundary protocol level (see #01436)
 1Fh    BYTE    lower boundary interface type
                same as offset 1Dh
 20h    WORD    module ID filled in by protocol manager
 22h    WORD    module DS
 24h    DWORD   system request entry point
 28h    DWORD   pointer to service-specific characteristics (see #01438,#01440)
                0000h:0000h if none
 2Ch    DWORD   pointer to service-specific status, or 0000h:0000h if none
                  (see #01441)
 30h    DWORD   pointer to upper dispatch table (see #01437)
                0000h:0000h if none
 34h    DWORD   pointer to lower dispatch table (see #01437)
                0000h:0000h if none
 38h  2 DWORDs  reserved, must be 0
Note:   for compatibility with NDIS 1.x.x, a major version of 00h is
          interpreted as 01h

(Table 01436)
Values for NDIS boundary protocol level:
 00h    physical
 01h    Media Access Control
 02h    Data link
 03h    network
 04h    transport
 05h    session
 FFh    not specified

Format of NDIS dispatch table:
Offset  Size    Description     (Table 01437)
 00h    DWORD   -> common characteristics table (see #01435)
 04h  4 BYTEs   ???
 08h    DWORD   -> ??? function (called with 12 bytes of stack arguments)
 0Ch    DWORD   -> ??? function (called with 10 bytes of stack arguments)
 10h    DWORD   -> ??? function (called with 16 bytes of stack arguments)
 14h    DWORD   -> ??? function (called with 4 bytes of stack arguments)
 18h    DWORD   -> ??? function (called with 18 bytes of stack arguments)
 1Ch    DWORD   -> ??? function (called with 12 bytes of stack arguments)

Format of MAC Service-Specific Characteristics Table:
Offset  Size    Description     (Table 01438)
 00h    WORD    length of table in bytes
 02h 16 BYTEs   ASCIZ MAC type name, "802.3", "802.4", "802.5", "802.6", "DIX",
                  "DIX+802.3", "APPLETALK", "ARCNET", "FDDI", "SDLC", "BSC",
                  "HDLC", or "ISDN"
 12h    WORD    length of station addresses in bytes
 14h 16 BYTEs   permanent station address
 24h 16 BYTEs   current station address
 34h    DWORD   current functional adapter address (00000000h if none)
 38h    DWORD   pointer to multicast address list
 3Ch    DWORD   link speed in bits/sec
 40h    DWORD   service flags (see #01439)
 44h    WORD    maximum frame size which may be both sent and received
 46h    DWORD   total transmit buffer capacity in bytes
 4Ah    WORD    transmit buffer allocation block size in bytes
 4Ch    DWORD   total receive buffer capacity in bytes
 50h    WORD    receive buffer allocation block size in bytes
 52h  3 BYTEs   IEEE vendor code
 55h    BYTE    vendor adapter code
 56h    DWORD   pointer to ASCIZ vendor adapter description
 5Ah    WORD    IRQ used by adapter
 5Ch    WORD    transmit queue depth
 5Eh    WORD    maximum supported number of data blocks in buffer descriptors
 60h  N BYTEs   vendor-specific info
SeeAlso: #01440

Bitfields for service flags:
Bit(s)  Description     (Table 01439)
 0      supports broadcast
 1      supports multicast
 2      supports functional/group addressing
 3      supports promiscuous mode
 4      station address software settable
 5      statistics always current
 6      supports InitiateDiagnostics
 7      supports loopback
 8      MAC does primarily ReceiveChain indications instead of ReceiveLookahead
        indications
 9      supports IBM source routing
 10     supports MAC reset
 11     supports Open/Close adapter
 12     supports interrupt request
 13     supports source routing bridge
 14     supports GDT virtual addresses (OS/2 version)
 15     multiple TransferDatas allowed durign a single indication
 16     MAC normally sets FrameSize = 0 in ReceiveLookahead
 17-31  reserved, must be 0

Format of NetBIOS Service-Specific Characteristics Table:
Offset  Size    Description     (Table 01440)
 00h    WORD    length of table in bytes
 02h 16 BYTEs   ASCIZ type name of NetBIOS module
 12h    WORD    NetBIOS module code
 14h  N BYTEs   vendor-specific info
SeeAlso: #01438

Format of MAC Service-Specific Status Table:
Offset  Size    Description     (Table 01441)
 00h    WORD    length of table in bytes
 02h    DWORD   seconds since 0:00 1/1/70 when diagnostics last run
                (FFFFFFFFh = never)
 06h    DWORD   MAC status bits (see #01442)
 0Ah    WORD    current packet filter flags (see #01443)
 0Ch    DWORD   pointer to media-specific status table or 0000h:0000h
 10h    DWORD   seconds past 0:00 1/1/70 of last ClearStatistics
 14h    DWORD   total frames received (FFFFFFFFh = not counted)
 18h    DWORD   frames with CRC error (FFFFFFFFh = not counted)
 1Ch    DWORD   total bytes received (FFFFFFFFh = not counted)
 20h    DWORD   frames discarded--no buffer space (FFFFFFFFh = not counted)
 24h    DWORD   multicast frames received (FFFFFFFFh = not counted)
 28h    DWORD   broadcast frames received (FFFFFFFFh = not counted)
 2Ch    DWORD   frames with errors (FFFFFFFFh = not counted)
 30h    DWORD   overly large frames (FFFFFFFFh = not counted)
 34h    DWORD   frames less than minimum size (FFFFFFFFh = not counted)
 38h    DWORD   multicast bytes received (FFFFFFFFh = not counted)
 3Ch    DWORD   broadcast bytes received (FFFFFFFFh = not counted)
 40h    DWORD   frames discarded--hardware error (FFFFFFFFh = not counted)
 44h    DWORD   total frames transmitted (FFFFFFFFh = not counted)
 48h    DWORD   total bytes transmitted (FFFFFFFFh = not counted)
 4Ch    DWORD   multicast frames transmitted (FFFFFFFFh = not counted)
 50h    DWORD   broadcast frames transmitted (FFFFFFFFh = not counted)
 54h    DWORD   broadcast bytes transmitted (FFFFFFFFh = not counted)
 58h    DWORD   multicast bytes transmitted (FFFFFFFFh = not counted)
 5Ch    DWORD   frames not transmitted--timeout (FFFFFFFFh = not counted)
 60h    DWORD   frames not transmitted--hardware error (FFFFFFFFh = not countd)
 64h  N BYTEs   vendor-specific info

Bitfields for MAC status bits:
Bit(s)  Description     (Table 01442)
 0-2    operational status
        000 hardware not installed
        001 hardware failed startup diagnostics
        010 hardware configuration problem
        011 hardware fault
        100 operating marginally due to soft faults
        101 reserved
        110 reserved
        111 hardware fully operational
 3      MAC bound
 4      MAC open
 5      diagnostics in progress
 6-31   reserved

Bitfields for packet filter flags:
Bit(s)  Description     (Table 01443)
 0      directed/multicast or group/functional
 1      broadcast
 2      promiscuous
 3      all source routing
 4-15   reserved, must be zero
--------I-214402-----------------------------
INT 21 U - IBM SYSTEM 36/38 WORKSTATION EMULATION - VDI.SYS - GET ???
        AX = 4402h
        BX = handle for character device "GDMS"
        CX = number of bytes to read (>= 4)
        DS:DX -> buffer (see #01445)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX = number of bytes read

Format of System 36/38 emulator returned data:
Offset  Size    Description     (Table 01445)
 00h  4 BYTEs   ???
 04h    DWORD   pointer to ???
 08h  4 BYTEs   ???
--------m-214402-----------------------------
INT 21 U - LASTBYTE.SYS v1.19 - IOCTL - GET ??? TABLE
        AX = 4402h
        BX = handle for device "LA$TBYTE"
        CX = 0004h
        DS:DX -> DWORD to hold address of 39-byte table of ???
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX = number of bytes read
Program: LASTBYTE.SYS is part of the shareware "The Last Byte" memory
          management package by Key Software Products
SeeAlso: AX=4402h"HIGHUMM"
--------m-214402-----------------------------
INT 21 - HIGHUMM.SYS v1.17+ - IOCTL - GET API ADDRESS
        AX = 4402h
        BX = handle for device "KSP$UMM"
        CX = 0004h
        DS:DX -> DWORD to hold entry point (see #01446)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AX = number of bytes read
Program: HIGHUMM.SYS is part of the shareware "The Last Byte" memory
          management package by Key Software Products
SeeAlso: AX=4402h"LASTBYTE"

(Table 01446)
Call HIGHUMM.SYS entry point with:
        AH = 00h allocate UMB (same as XMS function 10h) (see INT 2F/AX=4310h)
            DX = size in paragraphs
            Return: BX = segment number (if successful)
                    DX = size of requested block/size of largest block
        AH = 01h deallocate UMB (same as XMS func 11h) (see INT 2F/AX=4310h)
            DX = segment number of UMB
        AH = 02h request a bank-switched memory block
            DX = size in paragraphs
            Return: BX = segment number (if successful)
                    DX = size of requested block/size of largest block
        AH = 03h release a bank-switched memory block
            DX = segment number
        AH = 04h transfer data to/from high memory
            DS:SI -> source
            ES:DI -> destination
            CX = length in bytes
            Note: enables bank-switched memory, does the copy, then disables
                bank-switched memory
        AH = 05h get a word from bank-switched memory
            ES:DI -> word to read
            Return: DX = word
        AH = 06h put a word to bank-switched memory
            ES:DI -> word to write
            DX = word
        AH = 07h put a byte to bank-switched memory
            ES:DI -> byte to write
            DL = byte
        AH = 08h enable bank-switched memory
            DS:SI -> 6-byte status save area
        AH = 09h disable bank-switched memory
            DS:SI -> 6-byte save area from enable call (AH=08h)
        AH = 0Ah assign name to UMB or high bank-switched block
            DX = segment number
            DS:SI -> 8-byte blank-padded name
        AH = 0Bh locate UMB block by name
            DS:SI -> 8-byte blank-padded name
            Return: BX = segment number (if successful)
                    DX = size of block
        AH = 0Ch locate bank-switched block by name
            DS:SI -> 8-byte blank-padded name
            Return: BX = segment number (if successful)
                    DX = size of block
Return: AX = status code
            0001h successful
            0000h failed
                BL = error code
                    80h not implemented
                    B0h insufficient memory, smaller block available
                    B1h insufficient memory, no blocks available
                    B2h invalid segment number
Note:   only functions 00h and 01h are always available; the remaining
          functions are only enabled if the proper commandline switch is given
--------c-214402-----------------------------
INT 21 - SMARTDRV.SYS v3.x only - IOCTL - GET CACHE STATUS
        AX = 4402h
        BX = file handle for device "SMARTAAR"
        CX = number of bytes to read (min 28h)
        DS:DX -> buffer for status record (see #01447)
Return: CF clear if successful
            AX = number of bytes actually read
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   NCache2 (from the Norton Utilities v8.0) attempts to support this
          interface, but does not do so correctly, often hanging the system;
          one should use the SmartDrive v4.x or NCache private interfaces
          (see INT 2F/AX=4A10h/BX=0000h,INT 2F/AX=FE00h/DI=4E55h)
SeeAlso: AX=4403h"SMARTDRV",INT 2F/AX=4A10h/BX=0000h

Format of SMARTDRV status record:
Offset  Size    Description     (Table 01447)
 00h    BYTE    write-through flag (always 01h)
 01h    BYTE    writes should be buffered (always 00h)
 02h    BYTE    cache enabled if 01h
 03h    BYTE    driver type (01h extended memory, 02h expanded)
 04h    WORD    clock ticks between cache flushes (currently unused)
 06h    BYTE    cache contains locked tracks if nonzero
 07h    BYTE    flush cache on INT 19 reboot if nonzero
 08h    BYTE    cache full track writes if nonzero
 09h    BYTE    double buffering (for VDS) state (00h off, 01h on, 02h dynamic)
 0Ah    DWORD   original INT 13 vector
 0Eh    BYTE    minor version number
 0Fh    BYTE    major version number
 10h    WORD    unused
 12h    WORD    sectors read                    \
 14h    WORD    sectors already in cache         > may be scaled rather than
 16h    WORD    sectors already in track buffer /  absolute counts
 18h    BYTE    cache hit rate in percent
 19h    BYTE    track buffer hit rate in percent
 1Ah    WORD    total tracks in cache
 1Ch    WORD    number of tracks in use
 1Eh    WORD    number of locked tracks
 20h    WORD    number of dirty tracks
 22h    WORD    current cache size in 16K pages
 24h    WORD    original (maximum) cache size in 16K pages
 26h    WORD    minimum cache size in 16K pages
 28h    DWORD   pointer to byte flag to increment for locking cache contents
--------d-214402-----------------------------
INT 21 - CD-ROM device driver - IOCTL INPUT
        AX = 4402h
        BX = file handle referencing character device for CD-ROM driver
        CX = number of bytes to read
        DS:DX -> control block (see #01449)
Return: CF clear if successful
            AX = number of bytes actually read
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  the data returned depends on the first byte of the control block
          (two bytes for functions 01h/07h/0Bh, see #01449); the remainder of
          the control block is filled by the driver
        some device drivers support several subunits (that is more than one
          drive) but it is not possible to distinguish between them with this
          function; use INT 2F/AX=1510h or INT 2F/AX=0802h instead
        under Windows95, the "mscd$$$$" device cannot be opened so you cannot
          obtain the handle required by this function: use INT 2F/AX=1510h
          or INT 2F/AX=0802h instead
SeeAlso: AX=4403h"CD-ROM",INT 2F/AX=0802h,INT 2F/AX=1510h

(Table 01448)
Values for CD-ROM data being requested:
 00h    device driver header address
 01h    drive head location
 02h    reserved
 03h    error statistics
 04h    audio channel info
 05h    raw drive bytes (uninterpreted and device-specific)
 06h    device status
 07h    sector size
 08h    volume size
 09h    media change status
 0Ah    audio disk info
 0Bh    audio track info
 0Ch    audio Q-Channel info
 0Dh    audio sub-channel info
 0Eh    UPC code
 0Fh    audio status info

Format of CD-ROM control block:
Offset  Size    Description     (Table 01449)
 00h    BYTE    data being requested (see #01448)
---function 00h---
 01h    DWORD   device driver header address (see also AH=52h,#01646)
---function 01h---
 01h    BYTE    (call) addressing mode
                00h HSG
                01h Red Book
 02h    DWORD   current location of drive's head
                logical sector number in HSG mode
                frame/second/minute/unused in Red Book mode
                (HSG sector = minute * 4500 + second * 75 + frame - 150)
---function 03h---
 01h  N BYTEs   undefined as of 5 Aug 88 specification
---function 04h---
 01h    BYTE    input channel (0-3) for output channel 0
 02h    BYTE    volume for output channel 0
 03h    BYTE    input channel (0-3) for output channel 1
 04h    BYTE    volume for output channel 1
 05h    BYTE    input channel (0-3) for output channel 2
 06h    BYTE    volume for output channel 2
 07h    BYTE    input channel (0-3) for output channel 3
 08h    BYTE    volume for output channel 3
Notes:  output channels 0 and 1 are left and right, 2 and 3 are left prime and
          right prime; a volume of 00h is off
        the default setting is for each input channel to be assigned to the
          same-numbered output channel at full (FFh) volume
---function 05h---
 01h    BYTE    number of bytes read
 02h 128 BYTEs  buffer for drive bytes
---function 06h---
 01h    DWORD   device parameters (see #01450)
---function 07h---
 01h    BYTE    (call) read mode
                00h cooked
                01h raw
 02h    WORD    (return) sector size in bytes
---function 08h---
 01h    DWORD   volume size in sectors
BUGS:   Aztech device driver v1.10 and v1.27 (at least) return the last sector
          number, i.e. total number of sectors - 1
        Windows95 returns the total number of sectors + 150 (see the Red Book
          to HSG conversion formula at function 01h to understand why this
          happens)
---function 09h---
 01h    BYTE    media change status
                00h don't know
                01h media unchanged
                FFh media has been changed
---function 0Ah---
 01h    BYTE    lowest audio track number
 02h    BYTE    highest audio track number
 03h    DWORD   start address of lead-out track (Red Book format)
--function 0Bh---
 01h    BYTE    (call) track number
 02h    DWORD   starting point of track (Red Book format)
 06h    BYTE    track control info
                bits 15,14,12: track type (notice: bits not contiguous!)
                        000 two audio channels, no pre-emphasis
                        001 two audio channels with pre-emphasis
                        010 data track
                        100 four audio channels, no pre-emphasis
                        101 four audio channels with pre-emphasis
                        other reserved
                bit 13: digital copy permitted
---function 0Ch---
 01h    BYTE    CONTROL and ADR byte (as received from drive)
 02h    BYTE    track number
 03h    BYTE    point or index
 04h    BYTE    minute  \
 05h    BYTE    second   > running time within track
 06h    BYTE    frame   /
 07h    BYTE    zero
 08h    BYTE    "AMIN" or "PMIN"     \
 09h    BYTE    "ASEC" or "PSEC"      > running time on disk
 0Ah    BYTE    "AFRAME" or "PFRAME" /
---function 0Dh---
 01h    DWORD   starting frame address (Red Book format)
 05h    DWORD   transfer address
 09h    DWORD   number of sectors to read
Note:   copies 96 bytes of sub-channel info per sector into buffer
---function 0Eh---
 01h    BYTE    CONTROL and ADR byte
 02h  7 BYTEs   UPC/EAN code (13 BCD digits,low-order nybble of last byte is 0)
 09h    BYTE    zero
 0Ah    BYTE    "AFRAME"
---function 0Fh---
 ??? documentation not yet available
 01h    WORD    pause status (0000h not paused, 0001h paused)
 03h    DWORD   audio play start address
 07h    DWORD   ??? audio play length or end address

Bitfields for CD-ROM device parameters:
Bit(s)  Description     (Table 01450)
 0      door open
 1      door unlocked
 2      supports raw reading in addition to cooked
 3      writable
 4      can play audio/video tracks
 5      supports interleaving
 6      reserved
 7      supports prefetch requests
 8      supports audio channel control
 9      supports Red Book addressing in addition to HSG
 10     audio is playing
 11     no disk in drive
 12     supports R-W subchannels
--------m-214402-----------------------------
INT 21 - Quarterdeck - QEMM-386 v5+ - GET API ENTRY POINT
        AX = 4402h
        BX = file handle for device "QEMM386$"
        CX = 0004h
        DS:DX -> DWORD buffer for API entry point
Return: CF clear if successful
            buffer filled (refer to INT 67/AH=3Fh for entry point parameters)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   Quarterdeck recently (June 1993) documented this function, but the
          documentation incorrectly states that it is only available for
          QEMM 6+
SeeAlso: AX=4402h"HOOKROM",INT 2F/AX=D201h/BX=5145h,INT 67/AH=3Fh
--------Q-214402-----------------------------
INT 21 U - Quarterdeck - HOOKROM.SYS - GET HOOKED VECTOR TABLE
        AX = 4402h
        BX = file handle for device "HOOKROM$"
        CX = 0004h
        DS:DX -> DWORD buffer for address of hooked vector table (see #01451)
Return: CF clear if successful
            DS:DX buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4402h/SF=01h

Format of HOOKROM.SYS hooked vector table entry:
Offset  Size    Description     (Table 01451)
 00h  5 BYTEs   FAR jump to actual interrupt handler
                (end of table if first byte is not EAh)
 05h    BYTE    interrupt vector number
--------d-214402-----------------------------
INT 21 - Advanced SCSI Programming Interface (ASPI) - INTERFACE
        AX = 4402h
        BX = file handle for device "SCSIMGR$"
        CX = 0004h or 0005h (refer to notes below)
        DS:DX -> buffer for result (see #01452), set to zeros before call
Return: CF clear if successful
            AX = 0004h or 0005h (refer to notes below)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  the variant of the call requesting five bytes is an UNDOCUMENTED
          extension supported by Adaptec's ASPI2DOS.SYS, ASPI4DOS.SYS, and
          ASPI7DOS.SYS; if made of a host manager which does not support the
          variant, only four bytes will be returned.  If the variant is
          supported, Adaptec's WINASPI.DLL assumes that the host manager is
          an "advanced" one which operates in either real or protected mode
          (and thus does not require a DPMI INT 31/AX=0301h call to be invoked
          from protected mode).  Support of the five-byte variant also appears
          to imply that an advanced ASPI host manager uses no temporary
          storage space except the SRB (see #01454) and the stack, and that it
          is fully reentrant.
        if called with a standard request for four bytes, even Adaptec's
          advanced drivers return only the requested four bytes containing the
          ASPI entry point address
        the function address is called with the address of a SCSI Request
          Block (see #01454) on the stack and the caller must clean up the
          stack
SeeAlso: AX=440Ch"ASPITAPE",INT 11/AH=FFh"WD7000"

Format of ASPI IOCTL result:
Offset  Size    Description     (Table 01452)
 00h    DWORD   function address
 04h    BYTE    number of SCSI host adapters supported by host manager

(Table 01453)
Values for ASPI request number:
 00h    "HA_INQ"        host adapter inquiry / extended host adapter inquiry
 01h    "GET_TYPE"      get device type
 02h    "EXEC_SIO"      execute SCSI I/O
 03h    "ABORT_SRB"     abort SRB
 04h    "RESET_DEV"     reset SCSI device
 05h    "SET_HAPRMS"    set host adapter parameters
 06h    get disk drive information
 7Fh    (Adaptec) get ASPI manager info?
Note:   request 7Fh is supported by all the Adaptec's DOS ASPI managers:
          ASPI2DOS.SYS (for AHA-152x), ASPI4DOS.SYS (for AHA-154x/164x),
          ASPI7DOS.SYS (for AIC-77xx), and ASPI8DOS.SYS (for AIC-78xx)

Format of SCSI Request Block (64 bytes):
Offset  Size    Description     (Table 01454)
 00h    BYTE    request number (see #01453)
 01h    BYTE    request status (see #01455)
 02h    BYTE    host adapter ID
 03h    BYTE    request flags (see #01456)
 04h    DWORD   reserved (0)
---request 00h---
 08h    BYTE    (ret) number of host adapters
 09h    BYTE    (ret) target adapter ID
 0Ah 16 BYTEs   (ret) SCSI manager ID
 1Ah 16 BYTEs   (ret) host adapter ID
 2Ah 16 BYTEs   (ret) host adapter-unique parameters (see #90010,#90011)
---extended request 00h---
 04h  2 BYTEs   (call) extended request signature 55h AAh
                (ret) if extended request supported, reply AAh 55h
 06h    WORD    (call) length of extended buffer at offset 3Ah
                (ret) number of bytes returned in extended buffer
 08h    BYTE    (ret) number of host adapters
 09h    BYTE    (ret) target adapter ID
 0Ah 16 BYTEs   (ret) SCSI manager ID
 1Ah 16 BYTEs   (ret) host adapter ID
 2Ah 16 BYTEs   (ret) host adapter-unique parameters
 3Ah  N BYTEs   extended buffer
                3Ah     WORD    features (see #01457)
                3Ch     WORD    maximum scatter/gather list length
                3Eh     DWORD   maximum SCIS data transfer size (0 = unlimited)
---request 01h---
 08h    BYTE    target ID
 09h    BYTE    logical unit number
 0Ah    BYTE    (ret) device type (see #01460)
---request 02h---
 08h    BYTE    target ID
 09h    BYTE    logical unit number
 0Ah    DWORD   data allocation length
                (ret) residual byte length (if supported and requested)
 0Eh    BYTE    sense allocation length
 0Fh    DWORD   data buffer pointer
 13h    DWORD   next request pointer (for linking)
 17h    BYTE    CDB length
 18h    BYTE    (ret) host adapter status (see #01454)
 19h    BYTE    (ret) target status (see #01459)
 1Ah    DWORD   post routine address (see #01466)
 1Eh    WORD    real mode Post DS
 20h    DWORD   SRB pointer
 24h    WORD    reserved
 26h    DWORD   SRB physical address
 2Ah 22 BYTEs   SCSIMGR$ workspace
 40h  N BYTEs   CCB, including sense data (20-24 bytes)
---request 03h---
 08h    DWORD   address of SRB to abort
---request 04h---
 08h    BYTE    target ID
 09h    BYTE    logical unit number
 0Ah 14 BYTEs   reserved
 18h    BYTE    (ret) host adapter status (see #01458)
 19h    BYTE    (ret) target status (see #01459)
 1Ah    DWORD   post routine address
 1Eh 34 BYTEs   workspace
---request 05h---
 08h 16 BYTEs   host adapter-unique parameters
---request 06h---
 08h    BYTE    target ID
 09h    BYTE    logical unit number
 0Ah    BYTE    disk drive flags (see #01461)
 0Bh    BYTE    INT 13h drive number
 0Ch    BYTE    preferred head number translation
 0Dh    BYTE    preferred sector size translation
 0Eh 10 BYTEs   reserved
---request 7Fh---
 08h    WORD    base I/O port address
 0Ah    BYTE    number of I/O ports used
 0Bh    BYTE    ??? (01h returned for AHA-152x)
 0Ch    BYTE    interrupt level
 0Dh    BYTE    DMA channel
 0Eh    BYTE    (ASPI7DOS.SYS) EISA slot  number
                (ASPI8DOS.SYS) PCI device number
 0Fh    BYTE    bits 7-1: reserved (0)
                bit 0: (AHA-152x) ???
 10h    DWORD   ASPI entry point address
 14h    DWORD   previous ASPI entry point address
 18h    WORD     offset to "ASPI request dispatcher" procedure
 1Ah    WORD    offset to "interrupt handler" procedure
 1Ch    WORD    offset to some procedure
 1Eh    WORD    offset to some procedure
 20h    WORD    offset to host adapter data
 22h    BYTE    ??? (apparently always 02h)
 23h    BYTE    reserved??? (0)
 24h    BYTE    (ASPI8DOS.SYS) PCI bus number?
 25h  11 BYTEs  reserved??? (0)
SeeAlso: #01462

(Table 01455)
Values for ASPI request status:
 00h    not done yet
 01h    completed successfully
 02h    aborted by host
 04h    SCSI I/O error
 80h    invalid
 81h    no adapter
 82h    no device attached
 else   status
SeeAlso: #01454

Bitfields for ASPI request flags:
Bit(s)  Description     (Table 01456)
 0      posting enabled
 1      linking enabled
 2      residual byte length reported in Data Allocation Length field
 3      transfer from SCSI target to host
 4      transfer from host to SCSI target
 5      scatter/gather
 7-6    reserved
Note:   no data is transferred if both bits 3 and 4 are set; if neither is
          set, the direction is determined by the SCSI command
SeeAlso: #01454

Bitfields for ASPI extended features:
Bit(s)  Description     (Table 01457)
 0      scatter/gather supported
 1      residual byte length reported
 2      Wide SCSI 16 host adapter
 3      Wide SCSI 32 host adapter
 15-4   reserved
SeeAlso: #01454

(Table 01458)
Values for host adapter status:
 00h    no error detected
 11h    select timeout
 12h    data overrun
 13h    bus error
 14h    bus failure

(Table 01459)
Values for target status:
 00h    no status
 02h    sense data stored in SRB
 08h    target busy
 18h    reservation error

(Table 01460)
Values for device type:
 00h    disk drive
 01h    tape drive (streamer)
 02h    printer
 03h    processor
 04h    WORM drive
 05h    CD-ROM drive
 06h    scanner
 07h    optical drive
 08h    autochanger
 09h    communications device

(Table 01461)
Values for disk drive flags:
 00h    no INT 13 access
 01h    INT 13 with DOS access
 02h    INT 13 without DOS access
 03h    invalid flags

Format of CCB:
Offset  Size    Description     (Table 01462)
 00h    BYTE    command code (see #01463)
 01h    BYTE    flags
                bits 4-0: vary by function
                bits 7-5: logical unit number
 02h    BYTE    "adr_1"
 03h    BYTE    "adr_0"
 04h    BYTE    length
 05h    BYTE    control
        ...
 06h/0Ah 14 BYTEs buffer for sense data (see #01464)
SeeAlso: #01454

(Table 01463)
Values for CCB command code:
 00h    test unit ready
 01h    rewind
 03h    request sense data
 05h    get block size limits
 08h    Group 0 read
 0Ah    Group 0 write
 10h    write file marks
 11h    SCSI Space (set position?)
 12h    SCSI Inquire
 15h    set mode information
 16h    reserve SCSI device
 17h    release SCSI device
 19h    erase
 1Ah    request mode information
 1Bh    load/unload media
 1Dh    request target self-check
 24h    set window parameters
 25h    get window parameters
 28h    Group 1 read
 2Ah    Group 1 write
 31h    document feeder control
 34h    get scan data status
---vendor-specific commands---
 D3h    get document feeder status
 D4h    set document feeder mode

Format of sense data:
Offset  Size    Description     (Table 01464)
 00h    BYTE    error code (bit 7 set if valid)
 01h    BYTE    segment number
 02h    BYTE    sense key
                bit 6: EOM
                bit 5: ILI
                bits 0-3: sense key (see #01465)
 03h  4 BYTEs   information bytes
 07h    BYTE    additional sense length (0Ah)
 08h  4 BYTEs   command-specific information
 0Ch    BYTE    additional sense code
 0Dh    BYTE    additional sense code qualifier
 0Eh    BYTE    field replaceable unit code
 0Fh  3 BYTEs   sense key specific bytes

(Table 01465)
Values for sense key:
 00h    no sense data
 02h    SCSI unit not ready
 03h    media error
 04h    unrecoverable hardware error
 05h    illegal parameter in CDB
 06h    target has been reset
 0Bh    target aborted command

(Table 01466)
Values ASPI post function is called with:
        STACK:  DWORD -> SRB (see #01454) which completed
        interrupts disabled
Return: EBP, EBX, ESI, EDI must be preserved
        interrupts disabled
Note:   the post function may issue any ASPI function except an abort; it
          should complete as quickly as possible

Format of ASPI2DOS.SYS v3.65 host adapter unique parameters:
Offset  Size    Description     (Table 90010)
 00h    WORD    reserved? (0)
 02h    WORD    reserved? (0)
 04h    WORD    base I/O port address
 06h    BYTE    interrupt level
 07h  9 BYTEs   reserved  (0)
SeeAlso: #90011,#01454

Format of ASPI4DOS.SYS v3.35 host adapter unique parameters:
Offset  Size    Description     (Table 90011)
 00h    WORD    offset to "ASPI request dispatcher" procedure
 02h    WORD    offset to "interrupt handler" procedure
 04h 12 BYTEs   reserved  (0)
SeeAlso: #90010,#01454
--------m-214402-----------------------------
INT 21 U - Qualitas 386MAX v6.00+ - IOCTL INPUT - GET STATE
        AX = 4402h
        BX = file handle for device "386MAX$$"
        CX = number of bytes to read
        DS:DX -> BYTE 03h followed by 386MAX state buffer (see #01467)
Return: CF clear if successful
            buffer at DS:DX+1 filled
            AX = number of bytes actually copied
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  if the value given in CX is less than the size of the state record
          (5Ah for v6.01, 66h for v7.00), only a partial state record will be
          returned
        the state is 40h bytes for 386MAX (actually ASTEMM) v2.20 ("386MAX$$"
          did not exist yet, use "QMMXXXX0" and then "EMMXXXX0" instead) and
          56h bytes for v5.11.
        to invoke 386MAX API functions, push DX onto the stack, load DX with
          the word at offset 25h in the returned state, load all other
          registers as needed for the desired function, and execute an
          OUT DX,AL or OUT DX,AX; DX will be set to the pushed value on return
          if it is not otherwise modified by the API function.  For safety,
          in case a function is not supported or 386MAX is not present, SP
          should be saved and restored around the API call.
        Windows 3.1 Standard mode, LAN Manager, and Windows for Workgroups all
          use the 386MAX API; LAN Manager and Windows for Workgroups reportedly
          make some calls incorrectly
SeeAlso: AX=4403h/SF=03h"386MAX",INT 67/AH=3Fh

Format of 386MAX v6.01+ state:
Offset  Size    Description     (Table 01467)
 -1     BYTE    (call) 03h
 00h  6 BYTEs   signature "386MAX"
 06h  4 BYTEs   version string "N;NN" or "N.NN" (i.e. "6;01" for v6.01)
                (';' by default; apparently changed to a period when 386MAX
                has linked high RAM into DOS's memory chain)
 0Ah    WORD    segment of low-memory portion of 386MAX.SYS
 0Ch  2 BYTEs   ???
 0Eh    WORD    segment of ??? memory block or 0000h
 10h    WORD    bit flags 1 (see #01468)
 12h    WORD    starting address of video memory in KB
 14h  2 BYTEs   ???
 16h    WORD    total high DOS memory in KB
 18h  2 BYTEs   ???
 1Ah    WORD    available shared memory in KB
 1Ch    WORD    KBytes extended memory used by 386MAX
 1Eh  2 BYTEs   ???
 20h    WORD    total extended memory in KB
 22h    WORD    IO port to write (OUT DX,AL) to invoke 386MAX INT 15 functions
 24h    WORD    IO port to write (OUT DX,AL) to invoke 386MAX API functions
                  (see #01481)
 26h    WORD    ??? (depends on DOS version)
 28h    WORD    size of ??? in paragraphs
 2Ah    DWORD   machine type (see #01476)
 2Eh    DWORD   -> first DOS memory control block
 32h    WORD    system configuration flags (see #01469)
 34h    WORD    debugging flags 1 (see #01470)
 36h    WORD    debugging flags 2 (see #01471)
 38h  2 BYTEs   ???
 3Ah    WORD    segment of first MCB in high memory chain
 3Ch    WORD    feature flags 1 (see #01473)
 3Eh    WORD    feature flags 2 (see #01474)
 40h    WORD    feature flags 3 (see #01475)
 42h    WORD    segment of first 386MAX control block??? (see #01477)
 44h    WORD    amount of memory to report available on INT 15/AH=88h
 46h  4 BYTEs   ???
 4Ah    WORD    number of K at start of address space swapped with fast
                  extended memory (SWAP= parameter)
 4Ch  2 BYTEs   ???
 4Eh    WORD    segment address of ???
 50h    WORD    debugging flags 3 (see #01472)
 52h    DWORD   old INT 21h
 56h    DWORD   pointer to 386MAX's EMS (INT 67h) handler
---386MAX v7.00---
 5Ah    DWORD   KB of extended memory managed by 386MAX
 5Eh    DWORD   bytes of extended memory (EXT= parameter)
 62h  4 BYTEs   ???

Bitfields for 386MAX bit flags 1:
Bit(s)  Description     (Table 01468)
 1      ???
 2      allow A20 to be enabled/disabled???
 3      ??? (cleared by calling INT 67 functions or starting MSWindows)
 4      high RAM present???
 5      386MAX in AUTO mode
 6      386MAX enabled
 7       386MAX is providing EMS services
 8      ??? (affects API function 08h)
 9      A20 gate closed (A20 disabled) (see INT 15/AX=2402h)
 10     Weitek support enabled
 11     ???
 12     ROMs not shadowed???
 13     QPMS has been used
 14     ???
 15     ???

Bitfields for 386MAX system configuration flags:
Bit(s)  Description     (Table 01469)
 1      ROM compressed???
 3      ???
 5      386MAX loaded into high memory
 6      Microchannel bus
 7      Weitek math coprocessor detected
 9      ??? (also generates INT 01 on ??? and INT 03 on ???)
 11     PC/XT (thus only single 8259 interrupt controller present, DMA only
                in 1st megabyte, etc)
 13     LMLTOP= specified
 14     enable A20 control???
 15     ???

Bitfields for 386MAX debugging flags 1:
Bit(s)  Description     (Table 01470)
 0      DEBUG=LED
 1      DEBUG=X67
 2      DEBUG=INV
 3      DEBUG=EMSPTED
 4      DEBUG=JMP
 5      DEBUG=CALL
 6      DEBUG=HLT
 7      DEBUG=PMR
 8      DEBUG=CR3
 9      DEBUG=CAPS or DEBUG=INT
 10     DEBUG=RC
 11     DEBUG=ROM
 12     DEBUG=XM
 13     DEBUG=SOR
 14     DEBUG=XR
 15     DEBUG=EMSERR (generate INT 01 on returning error from EMS call)

Bitfields for 386MAX debugging flags 2:
Bit(s)  Description     (Table 01471)
 0      DEBUG=ROMSWAP
 1      DEBUG=UNSHADOWROM
 2      DEBUG=COMPROM
 3      DEBUG=DPMIPHYS
 4      DEBUG=ALLROM
 5      DEBUG=VMS
 6      DEBUG=XMS (generate INT 01 on XMS calls)
 7      DEBUG=I06
 8      DEBUG=VCPI
 9      DEBUG=XDMA
 10     DEBUG=X09
 13     DEBUG=I67 (generate INT 01 on every INT 67 call)
 14     DEBUG=EVM (generate INT 01 on entering V86 mode)
 15     DEBUG=EMSSAVE or DEBUG=VDS

Bitfields for 386MAX debugging flags 3:
Bit(s)  Description     (Table 01472)
 10     DEBUG=EPM
 12     DEBUG=ABIOS
 13     DEBUG=XMSPTED
 14     DEBUG=TIME
 15     DEBUG=SCRUB

Bitfields for 386MAX feature flags 1:
Bit(s)  Description     (Table 01473)
 1      Weitek present
 2      no DPMI services
 3      NODMA
 4      TERSE
 5      NOROM
 6      NOPARITY
 8      NOFLEX (IGNOREFLEXFRAME)
 11     don't create UMBs
 12     don't backfill below video memory (NOLOW)
 13     FRAME= specified
 14     EXT= specified
 15     NOEMS, allow prior expanded memory manager to provide EMS

Bitfields for 386MAX feature flags 2:
Bit(s)  Description     (Table 01474)
 0      UNSHIFT specified (FORCEA20 disabled)
 1      NOXRAM
 2      NOSCSI specified
 3      SCREEN specified
 4      enabled EISADMA
 5      slow DMA
 6      RESETKEYB specified
 7      ???
 9      TOP384
 10     ???
 11     NOWARMBOOT
 12     USE= specified
 13     ROM= specified

Bitfields for 386MAX feature flags 3:
Bit(s)  Description     (Table 01475)
 0      Windows3 support enabled
 1      SHADOWROM
 2      don't compress ROM (NOCOMPROM)
 3      ??? (related to PRGREG=)
 4      ??? (related to PRGREG=)
 5      SHADOWRAM
 6      DOS4 specified
 7      NOLOADHIGH
 8      NOPULSE
 11     FORCEA20
 12     DMA buffer enabled
 13     NOSCRUB
 15     NOFRAME

Bitfields for 386MAX machine type:
Bit(s)  Description     (Table 01476)
 12     Amstrad
 13     Epson
 14     Zenith Data Systems
 15     "ASEM"
 16     NEC
 17     "HPRS" model codes 69h and 6Ah
 18     Dell
 19     "CA"
 20     ITT (Xtra Business Systems/Alcatel)
 21     Toshiba 5100
 22     Olivetti
 23     Quadram Quad386 (BIOS model FEh, submodel A6h)
 24     Tandy???
 25     AST 386
 26     INBOARD, ??? version
 27     INBOARD, ??? version
 28     INBOARD, ??? version
 29     "HPRS"
 30     Compaq 386
 31     JET386

Format of 386MAX control block:
Offset  Size    Description     (Table 01477)
 00h    WORD    segment of next block (FFFFh if last)
 02h    WORD    segment of previous block (FFFFh if first)
 04h 12 BYTEs   filename
 10h    WORD    resident size in paragraphs
 12h    WORD    environment size???
 14h    WORD    real prsent environment size + 1 (0000h if ENVSAVE used)
 16h  2 BYTEs   ???
 18h    DWORD   initial size or SIZE=n in 386LOAD commandline
 1Ch    DWORD   SIZE=-1 ???
 20h    DWORD   SIZE= ???
 24h    BYTE    PRGREG= if specified, else FFh
 25h    BYTE    ENVREG= if specified, else FFh
 26h    BYTE    FlexFrame (00h not present, 01h present)
 27h  3 BYTEs   ???
 2Ah    BYTE    GROUP= or 00h if not present
 2Bh    BYTE    ???
 2Ch    WORD    PSP

Format of 386MAX high memory info record:
Offset  Size    Description     (Table 01478)
 00h    WORD    segment address of memory region
 02h    WORD    size of memory region in paragraphs
 04h    BYTE    type or flags???
                00h if locked out
                02h if EMS page frame
                04h if high RAM
                42h if ROM
 05h    BYTE    ???

Format of 386MAX ROM shadowing record:
Offset  Size    Description     (Table 01479)
 00h    WORD    logical start segment of ROM??? (may be used by BlueMAX when it
                  squeezes together the ROMs to make room)
 02h    WORD    physical start segment of ROM
 04h  2 BYTEs   ???
 06h    WORD    size of shadowed ROM in paragraphs
 08h  2 BYTEs   ???
 0Ah    WORD    flags
                bit 15: shadowing enabled for this ROM???
                bit 14: ???
                bit 13: ???
                bit 12: ???
                bit 10: ???

(Table 01480)
Values for 386MAX memory type:
 00h    unused by EMS
 01h    DOS
 04h    page frame overlapping ROM???
 80h    high memory
 84h    page frame???
 87h    video ROM???
Note:   the type may be 00h (unused) if the 16K page is split among different
          uses (such as ROM and high RAM)

(Table 01481)
Call 386MAX API (via OUT DX,AL) with:
        STACK: WORD value for DX
        AH = 00h unused
                Return: AH = 84h (unsupported function)
        AH = 01h get high memory information
                ES:DI -> buffer for array of high memory info records
                        (see #01478)
                Return: CX = number of records placed in buffer
        AH = 02h get shadowed ROM info
                ES:DI -> buffer for array of ROM shadowing records (see #01479)
                Return: CX = number of records placed in buffer
        AH = 03h get 386MAX state
                ES:DI -> 90-byte buffer for state (see #01467)
                Return: AH = 00h (successful)
                        buffer filled
        AH = 04h get memory types???
                ES:DI -> buffer for memory type info (array of bytes, one per
                        16K page) (see #01480)
                Return: CX = number of bytes placed in buffer
        AH = 05h get page table entries
                AL = A20 control (00h enable A20 first, 01h leave unchanged)
                CX = buffer size in bytes (0000h = enough for all memory from
                        given start to end of memory managed by 386MAX)
                SI = first K to report (rounded down to 4K page)
                ES:DI -> buffer for returned page table entries
                Return: CX = number of bytes returned (four per 4K page)
                        ES:DI buffer filled
        AH = 06h get memory speed info
                ES:DI -> buffer for memory speed records (see #01482)
                Return: AH = 00h (successful)
                        CX = number of bytes placed in buffer
                Note:   this function can take over a second to execute
        AH = 07h map/unmap multiple handle pages
                DX = EMS handle (on stack)
                STACK: DWORD -> EMS mapping record
                Return: AH = status (00h,80h,83h,8Ah,8Bh)
                Format of EMS mapping record:
                Offset  Size    Description
                 00h    WORD    function
                                0000h use physical page numbers
                                0001h use segment addresses
                 02h    WORD    EMS handle
                 04h    WORD    number of mapping entries following
                 06h 2N WORDs   logical page number and physical page/segment
                                logical page FFFFh means unmap physical page
                SeeAlso: INT 67/AH=50h
        AH = 08h "EMM2_GOREAL" check whether possible to disable 386MAX
                AL = ??? (00h or nonzero)
                Return: AH = status (00h OK, A4h not possible at this time)
                Note:   if AL=00h, this function always returns success
        AH = 09h toggle Bit Flags 1 flags
                BX = bitmask of bit flags 1's flags to toggle (see #01468)
                Return: AH = 00h (successful)
                Note:   enables A20 first
        AH = 0Ah toggle Debugging Flags 1 flags
                BX = bitmask of Debugging Flags 1's bits to toggle (see #01470)
                Return: AH = 00h (successful)
                Notes:  enables A20 first
                        does ??? if bit 3 on after specified bits are toggled
        AH = 0Bh toggle Debugging Flags 2 flags
                BX = bitmask of Debugging Flags 2's bits to toggle (see #01471)
                Return: AH = 00h (successful)
                Note:   enables A20 first
        AH = 0Ch toggle feature flags 3
                BX = bitmask of feature flags 3's bits to toggle (see #01475)
                Return: AH = 00h (successful)
                Note:   enables A20 first
        AH = 0Dh specify 386MAX high-memory location
                BX = segment address of high-memory real-mode portion of 386MAX
                CX = current segment of real-mode stub???
                Return: AH = status (00h successful)
                        ???
        AH = 0Eh CRT controller register virtualization
                AL = subfunction
                    00h allow access to CRTC I/O ports 03B4h/03B5h, 03D4h/03D5h
                    01h trap accesses to CRTC I/O ports
        AH = 0Fh reboot system
                Return: never
        AH = 10h unused
                Return: AH = 84h (unsupported function)
        AH = 11h get high memory information
                ES:DI -> 96-byte buffer for high memory info
                Return: AH = 00h (successful)
                        ES:DI buffer filled
                Notes:  each byte in buffer contains bit flags for a 4K page in
                          the A000h-FFFFh region
                                bit 0: page is writeable
                                bit 1: physical address same as linear address
                                bit 2: EMS page frame
                                bit 6: page is part of the QPMS window
                        this function can take over a second to execute,
                          because it does a 128K read for each page in an
                          attempt to flush any RAM cache the system may have
        AH = 12h shadow RAM mapping
                AL = subfunction
                    00h unshadow ROMs (except page FFh if NOWARMBOOT set)
                    01h map shadow RAM into ROM regions???
                Return: AH = 00h (successful) if AL=00h or 01h
                        AH = 8Fh otherwise
        AH = 13h shadow RAM page protection
                AL = subfunction
                    00h set all shadowed ROM 4K pages to read-only
                    01h set all shadowed ROM 4K pages to read-write
                Return: AH = 00h (successful) if AL=00h or 01h
                        AH = 8Fh otherwise
        AH = 14h get Programmable Option Select info???
                ES:DI -> 54-byte buffer for POS data???
                Return: AH = 00h if successful
                        AH = A4h on error
                Note:   the buffer consists of nine 6-byte fields; the first
                          eight for slots 1-8, the last for the system board
        AH = 15h ???
                ???
                Return: ???
        AH = 16h get 386MAX memory usage screen
                ES:DI -> buffer for memory info display
                CX = size of buffer in bytes
                Return: ES:DI buffer filled with '$'-terminated string (if
                                large enough to hold entire usage screen)
                Note:   the screen is 0303h bytes in v7.00
        AH = 17h Windows 3 startup/termination
                AL = subfunction
                    00h Windows3 initializing
                        DX (on stack) = Windows startup flags
                        DI = Windows version number (major in upper byte)
                        ES:BX = 0000h:0000h
                        DS:SI = 0000h:0000h
                        Return: CX = 0000h if OK for Windows to load
                                   <> 0 if Windows should not load
                                ES:BX -> startup info structure
                                DS:SI -> Virtual86 mode enable/disable callback
                    01h Windows3 terminating
                        ES:BX -> ???
                        DX (on stack) = Windows exit flags
                        Return: ???
        AH = 18h QPMS (Qualitas Protected Memory Services)
                AL = subfunction
                    00h get QPMS configuration
                        Return: BX = starting segment of QPMS memory window
                                CX = number of 4K pages reserved for QPMS???
                                DX = number of 4K pages in QPMS window???
                    01h map QPMS memory page???
                        BX = 4K page number within memory reserved for QPMS???
                        CL = 4K page number within QPMS memory window???
                    02h mark all QPMS memory read-only
                    03h mark all QPMS memory read-write
                Return: AH = status (00h,8Ah,8Bh,8Fh)
        AH = 19h get linear address for physical address
                EDX = physical address (low word on stack)
                Return: AH = status
                            00h successful
                                EDX = linear address at which physical address
                                        may be accessed
                            8Bh physical address currently not addressable
                Note:   enables A20 first
        AH = 1Ah set page table entry
                EDX = new page table entry (low word on stack)
                ESI = linear address of page to map (bits 0-11 clear)
                Return: AH = status (00h,8Bh)
                Note:   enables A20 first
        AH = 1Bh get ???
                Return: AH = status
                        BX = ???
                        CX = number of ???
                        EDX = physical address of ???
        AH = 1Ch get original interrupt vector
                AL = interrupt vector (00h-7Fh)
                Return: AH = 00h (successful)
                        EDX = original vector before 386MAX loaded (segment in
                                high word, offset in low word)
                Note:   no range checking is performed; requests for INTs 80h-
                          FFh will return random values
        AH = 1Dh display string???
                SI = ???
                Return: AH = 00h (successful)
                        ???
                Note:   this function appears to be broken in v7.00
        AH = 1Eh get memory info
                ES:DI -> memory info (see #01483)
                Return: ???
        AH = 1Fh get DPMI host information
                Return: AX = 0000h if successful
                        BX = DPMI flags (see #03152 at INT 31/AX=0400h)
                        CL = CPU type (02h = 80286, 03h = 80386, etc.)
                        DX = DPMI ver supported (DH=major, DL=2-digit minor)
                        SI = ???
                        ES???:DI -> ???
                Note:   NOP if NODPMI switch specified
        AH = 20h (v7.00) get ???
            AL = index of ???
            Return: EDX = ??? for specified ???
        AH = 21h (v7.00) STACKS support
            AL = 00h get STACKS parameters
                Return: BX = ??? (0060h for v7.00)
                        CX = number of stacks for hardware interrupts
                        DX = size of each stack in bytes
                        SI = ??? (low and high bytes are separate values)
                        DI = ??? (low and high bytes are separate values)
                            low byte = logical page number set by subfn 02h
                        ES = ???
            AL = 01h set ??? "EMM2_DSTKS"
                EBX = ???
                ECX = ???
            AL = 02h set ???
                BL = logical page number for ??? (00h-03h)
                Return: AH = status (00h,8Ah)
        AH = 22h (v7.00) call ??? for every load module
            AL = which function to call
                00h call ???
                else call ????
            Return: AH = 00h
            Note: if AL=00h, calls the protected-mode function pointed at by
                the DWORD at offset 22h from the start of each module installed
                by a LOAD= directive; if AL<>00h, it calls the function
                pointed at by the DWORD at offset 28h of the load module
        AH = 23h (v7.00) ???
            AL = 00h set ???
                BL = ???
                Return: AH = 00h or unchanged (depending on ???)
            AL = 01h set ???
                BL = ???
                BH = ???
                CX = ???
                DX = ??? (on top of stack)
                Return: AH = status (00h if successful, 8Fh once table full)
                Note:   this call adds one entry to an internal table on each
                          call, until the table is full
            AL = 02h get ???
                CX = size of buffer
                ES:DI -> buffer for ??? (60 bytes total data)
                Return: CX = number of bytes actually returned
                Note:   returns the array storing the values set with AX=2301h
                Format of one entry in array:
                Offset  Size    Description
                 00h    BYTE    ??? (BL from subfn 01h)
                 01h    WORD    ??? (CX from subfn 01h)
                 03h    BYTE    ??? (BH from subfn 01h)
                 04h    WORD    ??? (DX from subfn 01h)
            AL = 03h set ??? name/path
                ES:DI -> buffer containing ASCIZ ???
            AL = 04h get ???
                ES:DI -> buffer for ASCIZ ???
            Note: the ASCIZ string for subfunctions 03h and 04h does not appear
                to be used by 386MAX, and may serve merely for communication
                between two other Qualitas programs
        AH = 24h (v7.00) high memory control
            AL = 00h get high memory state
                Return: BX = current state
                            00h high memory removed from DOS memory chain
                            01h high memory included in DOS memory chain
            AL = 01h set high memory state
                BX = new state
                    00h high memory removed from DOS memory chain
                    01h high memory included in DOS memory chain
                    else
                        Return: ??? (error, but return varies according to ???)
        AH = 25h (v7.00) remove high RAM from DOS memory chain
        AH = 26h (v7.00) ???
            BX = ???
            CX = ???
            SI = ???
            DI = ???
            Return: AH = status
                    BX = ???
                    CX = ???
        AH = 27h (v7.00) ???
            AL = 00h get ???
                Return: BX = number of paragraphs for ???
            AL = 01h ???
                BX = ???
                ES??? = ???
            AL = 02h ???
                ???
            AL = 03h ???
                CX = ???
                DX = ???
                ES??? = ???
                Return: ???
        AH = 28h (v7.00) get ???
                Return: AH = status (00h,8Fh) (see #03648 at INT 67/AH=40h)
                        if AH=00h,
                            CX = ???
                            DX = ???
        AH = 29h (v7.00) get ???
                Return: AX = ???
        AH = 40h-5Dh EMS services (see INT 67/AH=40h, etc.)
        AH = DEh VCPI services (see INT 67/AX=DE00h, etc.)
Return: AH = status (as for EMS INT 67 calls)
            00h successful
            80h internal error
            81h hardware malfunction
            83h invalid handle
            84h undefined function
            8Ah invalid logical page nuber
            8Bh illegal physical page number
            8Fh undefined subfunction
            A4h access denied
            etc.
        STACK popped (value placed in DX if no specific return value for DX)

Format of 386MAX memory speed record:
Offset  Size    Description     (Table 01482)
 00h    DWORD   page table entry for 4K page
 04h    WORD    number of microticks (840ns units) required for REP LODSD of
                  entire 4K page

Format of 386MAX memory info [array]:
Offset  Size    Description     (Table 01483)
 00h    DWORD   linear start address
 04h    DWORD   size in bytes
 08h    WORD    XMS handle (if next byte = 04h)
                ??? (if next byte = 05h)
                ??? (if next byte = 06h)
                ??? (if next byte = 13h)
                ??? (if next byte = 14h)
                ??? (if next byte = 15h)
                ??? (if next byte = 23h)
                ??? (if next byte = 24h)
                ??? (if next byte = 26h)
                else unused
 0Ah    BYTE    type
                00h = ???, 01h = VDISK,
                02h = INT 15h extended memory, 03h = ??? extended,
                04h = XMS handle's memory, 05h = ???, 06h = ???, 07h = ???,
                08h = ???, 09h = ???, 0Ah = ???, 0Bh = ???,
                11h = ???, 12h = ???, 14h = ???, 15h = ???,
                19h = ???, 1Ah = ???, 1Bh = ???,
                1Ch = ???, 1Dh = ???, 1Eh = ???, 1Fh = ???,
                20h = ???, 21h = ???, 23h = ???, 24h = ???,
                26h = ???
 0Bh    BYTE    ??? (00h for types 00h-03h, 07h-0Bh, 19h-21h;
                     80h for types 04h/13h-15h/23h-26h;
                        ??? for type 05h)
--------V-214402-----------------------------
INT 21 - PGS1600.DEV - IOCTL - GET CONFIGURATION INFO
        AX = 4402h
        BX = file handle for device "PGS1600$"
        CX = 0018h (size of buffer)
        DS:DX -> configuration buffer (see #01484)
Return: CF clear if successful
            buffer filled
            AX = number of bytes actually copied
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: PGS1600.DEV is a device driver for the Cornerstone Technology PG1600
          display adapter, which provides a 1600x1200 monochrome display as
          well as one of two emulations, MDA or CGA.
SeeAlso: AX=4403h"PGS1600"

Format of PGS1600.DEV configuration information:
Offset  Size    Description     (Table 01484)
 00h    WORD    version (high byte = major, low byte = minor)
 02h    WORD    board initialisation mode
 04h    WORD    board I/O address
                03D0h CGA emulation
                03B0h MDA emulation
                0390h no emulation
                0350h no emulation, alternate
 06h    WORD    emulation buffer segment
                B800h   CGA emulation
                B000h   MDA emulation
                0000h   no emulation
 08h    WORD    PG1600 graphics buffer segment
 0Ah    WORD    number of bytes between consecutive graphic rows
 0Ch    WORD    horizontal pixel size
 0Eh    WORD    vertical pixel size
 10h    WORD    horizontal dots per inch
 12h    WORD    vertical dots per inch
 14h    WORD    graphics buffer bits per pixel
 16h    WORD    monitor bits per pixel
--------N-214402-----------------------------
INT 21 - WORKGRP.SYS - GET API ENTRY POINT
        AX = 4402h
        BX = file handle for device "NET$HLP$"
        CX = 0008h
        DS:DX -> buffer for entry point record (see #01485)
Return: CF clear if successful
            AX = number of bytes actually read
        CF set on error
            AX = error code
Program: WORKGRP.SYS is the portion of Microsoft's Workgroup Connection which
          permits communication with PCs running Windows for Workgroups or
          LAN Manager
SeeAlso: AH=3Fh"WORKGRP.SYS"

Format of WORKGRP.SYS entry point record:
Offset  Size    Description     (Table 01485)
 00h    WORD    3633h  \ signature???
 02h    WORD    EF6Fh  /
 04h    DWORD   address of entry point (see #01486)
Note:   first four bytes of buffer must be 6Fh E9h 33h 36h on entry when using
          IOCTL rather than READ to get the entry point record

(Table 01486)
Call WORKGRP entry point with:
        STACK:  WORD    function number (0000h-0009h)
Return: STACK unchanged
SeeAlso: #01487,#01488,#01489,#01490,#01491,#01492,#01493,#01494

(Table 01487)
Call WORKGRP function 00h with:
        STACK:  WORD    0000h (function "get ???")
Return: DX:AX -> data table

(Table 01488)
Call WORKGRP function 01h with:
        STACK:  WORD    0001h (function "hook ???")
Return: STACK:  DWORD   pointer to ???
                WORD    0001h (function number)

(Table 01489)
Call WORKGRP function 02h with:
        STACK:  WORD    0002h (function "unhook ???")
        ???
Return: ???

(Table 01490)
Call WORKGRP function 03h with:
        STACK:  WORD    0003h (function "reenable printer port")
                WORD    LPT port number
Return: ???

(Table 01491)
Call WORKGRP function 04h with:
        STACK:  WORD    0004h (function "disable printer port")
                WORD    LPT port number
Return: ???

(Table 01492)
Call WORKGRP function 05h with:
        STACK:  WORD    0005h (function "???")
                ???
Return: ???

(Table 01493)
Call WORKGRP function 06h with:
        STACK:  WORD    0006h (function "???")
Return: STACK unchanged
        AX = 0000h
        DX = 0000h

(Table 01494)
Call WORKGRP functions 07h-09h with:
        STACK:  WORD    0007h-0009h (NOP functions)
Return: STACK unchanged
        AX = 0001h
        DX = 0000h
--------V-214402-----------------------------
INT 21 - Compaq AG1024.SYS - RGDI - GET DRIVER LOCATION
        AX = 4402h
        BX = file handle for device "$$$$RGDI"
        CX = 0006h (size of returned data)
        DS:DX -> location record (see #01498)
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: AG1024.SYS is a device driver for the Advanced Graphics 1024 adapter
SeeAlso: AX=4403h"RGDI"

Format of Compaq AG1024.SYS location record:
Offset  Size    Description     (Table 01498)
 00h    WORD    signature 55AAh
 02h    WORD    segment of ???
 04h    WORD    segment of device driver's code
--------n-214402-----------------------------
INT 21 U - PenDOS PENDEV.SYS - GET ENTRY POINTS
        AX = 4402h
        BX = file handle for device "$$PENDOS" or "$$PD_REG"
        CX = size of buffer (4 for $$PENDOS and a 4,8,12, or 16 for $$PD_REG)
        DS:DX -> buffer for entry point record (see #01500)
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: A limited version of PenDOS by Communication Intelligence Corporation,
          which provides pen capability to keyboard-based programs, is bundled
          with IBM DOS 6.1
SeeAlso: AX=4403h"PENDEV.SYS"

Format of PENDEV.SYS entry point record:
Offset  Size    Description     (Table 01500)
 00h    DWORD   -> array of jumps
 04h    WORD    offset of function to retrieve entry point (see #01501)
 06h  2 BYTEs   signature "Pe"
 08h    WORD    offset of function to set entry point (see #01502)
 0Ah  2 BYTEs   signature "nD"
 0Ch    WORD    offset of function to clear entry point (see #01503)
 0Eh    WORD    signature "OS"

(Table 01501)
Call PENDEV.SYS function to retrieve entry point with:
        AX = index of entry point (0-9)
Return: CF clear if successful
            DX:AX -> desired entry point
        CF set on error (AX out of range)

(Table 01502)
Call PENDEV.SYS function to set entry point with:
        AX = index of entry point (0-9)
        DX:SI -> new handler
Return: CF clear if successful
        CF set on error (AX out of range)

(Table 01503)
Call PENDEV.SYS function to clear entry point with:
        AX = index of entry point (0-9)
Return: CF clear if successful
        CF set on error (AX out of range)
Note:   resets the jump at the specified entry point to its default target,
          which simply returns
--------N-214402-----------------------------
INT 21 U - LAN Manager - TCPDRV.DOS - API
        AX = 4402h
        BX = file handle referencing device "TCPDRV$"
        CX = 0019h
        DS:DX -> buffer containing request block (see #01504)
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: TCPDRV.DOS is the low-level device driver supporting LAN Manager's
          TCP/IP protocol stack

Format of TCPDRV.DOS request block:
Offset  Size    Description     (Table 01504)
 00h    BYTE    (call) function number
                00h initialize ???
                06h get ???
                07h get ???
 01h    BYTE    (call) 00h
                (ret) error code if error, unchanged if successful
 02h    WORD    signature 4354h ('CT')
---function 00h---
 04h    DWORD   (call) pointer to ??? FAR function
                function is called with ES:BX -> device driver request used to
                  invoke this function
 08h  4 BYTEs   ???
 0Ch    DWORD   (call) pointer to ??? record, WORD at offset 22h is read
 10h    DWORD   (ret) -> ??? buffer if 0000h:0000h on call
---function 06h---
 04h  4 BYTEs   ???
 08h    DWORD   (ret) pointer to ???
---function 07h---
 04h    DWORD   (ret) pointer to ??? record
--------s-214402-----------------------------
INT 21 U - Creative Technology CTMMSYS.SYS v1.00.01 - API
        AX = 4402h
        BX = file handle for device "CTMMSYS$"
        CX = 0004h (size of data)
        DS:DX -> buffer for entry point (see #01507)
Return: CF clear if successful
            buffer updated
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: CTMMSYS.SYS is the Creative DOS Multimedia Architecture Integration
          driver
SeeAlso: AX=4402h"CTSB2",INT 80/BX=0000h"SBFM"

Format of CTMMSYS.SYS entry point record:
Offset  Size    Description     (Table 01507)
 00h    DWORD   (call) signature 4D6D7443h (ASCII "CtmM")
                (ret) pointer to CTMMSYS entry point (see #01508)
SeeAlso: #01509

(Table 01508)
Call CTMMSYS.SYS entry point with:
        AX = ???
        STACK:  WORD    ???
                WORD    ???
                WORD    ??? (0001h,0002h,0005h,0006h)
                WORD    ???
                DWORD   -> ???
                WORD    ???
                WORD    ???
Return: DX:AX = ??? or error code
                0000h:000Bh invalid value for ???
                0000h:000Fh API call already in progress
SeeAlso: #01507
--------s-214402-----------------------------
INT 21 U - Creative Technology CTSB2.SYS v1.01.01 - API
        AX = 4402h
        BX = file handle for device "CTSOUND0"
        CX = 0004h (size of data)
        DS:DX -> buffer for entry point (see #01509)
Return: CF clear if successful
            buffer updated
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: CTMMSYS.SYS is the Creative DOS Multimedia Architecture Integration
          driver
SeeAlso: AX=4402h"CTMMSYS",INT 80/BX=0000h"SBFM"

Format of CTSB2.SYS entry point record:
Offset  Size    Description     (Table 01509)
 00h    DWORD   (call) signature 4D6D7443h (ASCII "CtmM")
                (ret) pointer to CTSB2 entry point (see #01510)
SeeAlso: #01507

(Table 01510)
Call CTSB2.SYS entry point with:
        AX = ???
        STACK:  DWORD   -> ???
                WORD    function number
                        (0100h,0200h,0300h,0400h,0500h,0600h,0701h)
                DWORD   -> ???
                WORD    ???
                WORD    ???
Return: DX:AX = ??? or error code
                0000h:0004h invalid subfunction???
                0000h:000Bh invalid value for ???
                0000h:000Fh API call already in progress
SeeAlso: #01509
--------m-214402-----------------------------
INT 21 U - Novell DOS 7+ EMM386.EXE - GET STATE RECORD
        AX = 4402h
        BX = file handle for device "EMMXXXX0" or "EMMQXXX0"
        CX = 003Eh (size of state record)
        DS:DX -> buffer for state record (see #01511)
Return: CF clear if successful
            buffer filled (see #03603 at INT 67/AH=3Fh)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   This function is called by the Novell DOS 7+ MEM utility.

Format of Novell DOS 7 - DR-DOS 7.03 EMM386.EXE state record:
Offset  Size    Description     (Table 01511)
 00h    WORD    signature EDC0h
 02h 12 BYTEs   (ret) bitmap for include range
 0Eh 12 BYTEs   (ret) bitmap for auto range
 1Ah 12 BYTEs   (ret) bitmap for BIOS range
 26h 12 BYTEs   (ret) bitmap for ROM range
 32h 12 BYTEs   (ret) bitmap for map range
Note:   each bitmap covers the upper-memory address range A000h-FFFFh, where
          offset X bit B within the bitmap indicates the state of the 4K page
          starting at segment A000h + (8X + B) * 100h
--------m-214402SF00-------------------------
INT 21 U - Memory Managers - GET API ENTRY POINT
        AX = 4402h subfn 00h
        BX = file handle for device "EMMXXXX0"
        CX = 0006h (size of buffer in bytes)
        DS:DX -> buffer for API entry point record (see #01512)
                first byte must be 00h on entry
Return: CF clear if successful
            buffer filled (see #03603 at INT 67/AH=3Fh)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  this function is supported by Microsoft EMM386.EXE v4.45+ and
          CEMM v5.10+, and is intended for use by MS Windows as it starts up
        if no other program has hooked INT 67, an alternate installation
          check for CEMM is testing for the string
          "COMPAQ EXPANDED MEMORY MANAGER 386" at offset 14h in the INT 67
          handler's segment; if present, the word at offset 12h contains the
          offset of the API entry point
SeeAlso: AX=4402h/SF=01h,AX=4402h/SF=02h,AX=4402h"EMM386",INT 67/AH=3Fh

Format of memory manager API entry point record:
Offset  Size    Description     (Table 01512)
 00h    WORD    ??? (0022h for CEMM 5.11, 0025h for MS EMM386 v4.45)
 02h    DWORD   manager's private API entry point
                (see #01513,#03666 at INT 67/AX=FFA5h)

(Table 01513)
Call CEMM v5.10+ entry point with:
        AH = 00h get memory manager's state
            Return: AH = state
                        bit 0: turned OFF
                        bit 1: AUTO mode enabled
        AH = 01h set memory manager's state
            AL = new state (00h ON, 01h OFF, 02h AUTO)
            Return: CF clear if successful
                    CF set on error
        AH = 02h Weitek coprocessor support
            AL = subfunction
                00h get Weitek support state
                    Return: AL = status
                                bit 0: Weitek coprocessor is present
                                bit 1: Weitek support is enabled
                01h turn on Weitek support
                02h turn off Weitek support
            Return: CF clear if successful
                    CF set on error
                        AH = error code (01h invalid subfunc, 02h no Weitek)
        AH = 05h get statistics
            ???
        AH > 06h
            Return: CF set
                    AH = 01h (invalid function)
Notes:  AH=03h,04h,06h are NOPs which return CF clear, presumably for backwards
          compatibility with earlier versions of CEMM
        in v5.11, AH=05h merely prints an error message (using INT 21/AH=09h)
          stating that a different version of CEMM is installed and it is
          therefore not possible to display the statistics
--------m-214402SF01-------------------------
INT 21 U - Memory Managers - GET EMM IMPORT STRUCTURE ADDRESS
        AX = 4402h subfn 01h
        BX = file handle for device "EMMXXXX0"
        CX = 0006h (size of buffer in bytes)
        DS:DX -> buffer for EMM import structure record (see #01514)
                first byte must be 01h on entry
Return: CF clear if successful
            buffer filled (see also #03603 at INT 67/AH=3Fh)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  this function is supported by Microsoft EMM386.EXE v4.45+,
          QEMM-386 v6+, and CEMM v5.10+, and is intended for use by MS Windows
          as it starts up
        for QEMM-386, this call always returns an error if Windows3 support
          has been disabled with the NW3 switch
SeeAlso: AX=4402h/SF=00h,AX=4402h"EMM386",INT 2F/AX=D201h/BX=5145h
SeeAlso: INT 67/AH=3Fh

Format of EMM import structure record:
Offset  Size    Description     (Table 01514)
 00h    DWORD   physical address of EMM import structure (see #01515)
 04h    BYTE    EMM import structure major version
 05h    BYTE    EMM import structure minor version
Note:   vession 1.00 contains only EMS information (Windows 3.0+)
        version 1.10 contains UMB/XMS/HMA/EMS information (Windows 3.1)
        version 1.11 is version 1.10 plus memory manager maker/product name
SeeAlso: #03643

Format of Global EMM Import record:
Offset  Size    Description     (Table 01515)
 00h    BYTE    bit flags
                bit 2: ???
                bit 3: free EMM386 virtual HMA only if hma_page_table_paddr!=0
                bit 4: no UMB???
 01h    BYTE    reserved (0)
 02h    WORD    size of structure in bytes
 04h    WORD    structure version
 06h    DWORD   reserved
 0Ah 384 BYTEs  64 EMS frame status records (see #01516), one per 16K of
                  real-mode 1M address space
18Ah    BYTE    ??? (must be at least 3*number_of_EMS_frames+4)
18Bh    BYTE    number of UMB frame descriptors following
18Ch 4N DWORDs  UMB frame descriptors
                each is 4 DWORDs giving physical page numbers for the four
                  4K pages of a 16K EMS frame (00000000h if non-UMB page)
var     BYTE    number of EMS handle info records following
    16N BYTEs   EMS handle info records (see #01518)
---version 1.10+ ---
        DWORD   realmode INT 67 vector (used by Windows to set breakpoints)
        DWORD   physical address of HMA page table values
        BYTE    number of free page entries following
     2N DWORDs  free page entries
                each is:
                    DWORD       physical page number
                    DWORD       number of consecutive physical pages
        BYTE    number of XMS handle info records following
                00h if memory manager does not emulate XMS or has real mode
                  XMS code which can execute in the Windows environment
    12N BYTEs   XMS handle info records (see #01519)
        BYTE    number of free UMB info records following
     2N WORDs   free UMB info records
                each is:
                    WORD        real mode start segment
                    WORD        size in paragraphs
---version 1.11---
     20 BYTEs   blank-padded maker name
     20 BYTEs   blank-padded product name

Format of EMS frame status record:
Offset  Size    Description     (Table 01516)
 00h    BYTE    frame type (see #01517)
 01h    BYTE    owner handle (00h/FFh = none) from frame including UMB
                index to UMB frame descriptors
 02h    WORD    logical page for frame, 7FFFh if none, FFFFh if non-EMS frame
 04h    BYTE    EMS physical page number (FFh for non-EMS = don't care???)
 05h    BYTE    flags for non-EMS frames (00h for EMS frame)
                bits 0,1 for first 4K, bits 2,3 for second 4K, etc:
                  10: direct mapping (linear address = physical address)
                  01: UMB mapping

Bitfields for EMS frame type:
Bit(s)  Description     (Table 01517)
 0      EMS frame
 1      (if EMS frame) in standard 64K page frame
 2      first 4K of frame is UMB
 3      second 4K of frame is UMB
 4      third 4K of frame is UMB
 5      last 4K of frame is UMB

Format of EMS handle info record:
Offset  Size    Description     (Table 01518)
 00h    BYTE    handle number (00h = system handle)
 01h    BYTE    flags
                bit 0: normal handle rather than system handle
                bit 2: ??? (set by some EMS managers)
 02h  8 BYTEs   EMS handle's name
 0Ah    WORD    number of 16K pages for handle
 0Ch    DWORD   physical address of page table entries forming page map
Note:   all values should be zero for the system handle if no large frame
          support is present

Format of XMS handle info record:
Offset  Size    Description     (Table 01519)
 00h    WORD    handle
 02h    WORD    flags
                bit 0: handle usable by Windows
                        (already in use when Windows started if clear)
                bit 1: reserved (0)
 04h    DWORD   size in KB (may be zero, used only if flags bit 0 set)
 08h    DWORD   physical address (only if flags bit 0 set)
--------m-214402SF02-------------------------
INT 21 U - Memory Managers - GET MEMORY MANAGER VERSION
        AX = 4402h subfn 02h
        BX = file handle for device "EMMXXXX0"
        CX = 0002h (size of buffer in bytes)
        DS:DX -> buffer for memory manager version (see #01520)
                first byte must be 02h on entry
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   this function is supported by Microsoft EMM386.EXE v4.45+ and
          CEMM v5.10+, and is intended for use by MS Windows as it starts up
SeeAlso: AX=4402h/SF=00h,AX=4402h"EMM386",INT 67/AH=3Fh

Format of memory manager version:
Offset  Size    Description     (Table 01520)
 00h    BYTE    major version
 01h    BYTE    minor version (binary)
--------m-214402-----------------------------
INT 21 U - Microsoft EMM386.EXE v4.45 - GET MEMORY MANAGER INFORMATION
        AX = 4402h
        BX = file handle for device "EMMXXXX0"
        CX = size of buffer in bytes (varies, see #01521)
        DS:DX -> buffer for returned data (see #01521)
                first byte must be set on entry to indicate desired data
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  an error is returned if the number of bytes to be read does not match
          the number of bytes returned for the specified data item
        this function is part of the interface which allows MS Windows to
          cooperate with memory managers
SeeAlso: AX=4402h/SF=00h,AX=4402h/SF=01h,AX=4402h/SF=02h,INT 67/AX=FFA5h

Format of EMM386.EXE data buffer:
Offset  Size    Description     (Table 01521)
 00h    BYTE    (call) function
                03h get ???
                04h get ???
---function 03h---
 00h    WORD    ???
 02h    WORD    ???
---function 04h---
 00h    WORD    segment of UMB containing EMM386 code/data
 02h    WORD    number of paragraphs of EMM386 code/data in UMB
 04h    WORD    ???
----------214402-----------------------------
INT 21 U - IFSHLP.SYS - GET ENTRY POINT
        AX = 4402h
        BX = file handle for device "IFS$HLP$"
        CX = 0008h (size of buffer in bytes)
        DS:DX -> buffer for entry point record (see #01415 at AH=3Fh"IFSHLP")
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AH=3Fh"IFSHLP"
--------d-214402-----------------------------
INT 21 - ATA Software Programming Interface (ATASPI) - INTERFACE
        AX = 4402h
        BX = file handle for device "$ATAMGR$"
        CX = 0004h (size of buffer in bytes)
        DS:DX -> buffer for result (see #90000), set to zeros before call
Return: CF clear if successful
            AX = 0004h
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  ATASPI is an API to control IDE, EIDE, and ATAPI devices, proposed by
          Future Domain; there's at least one driver in existence which
          conforms to this specification: ATASPI16.SYS by Future Domain
        (see #90002) on the stack and the caller must clean up the stack
        Future Domain ATASPI16.SYS v2.2 performs ATASPI calls using INT 4Fh
          interface instead of this INT 21/4402h interface and itself supports
          both interfaces
SeeAlso: AX=4402h"ASPI",INT 4F/AX=0081h,INT 4F/AX=0082h,PORT 01F0h-01F7h"HDC1"

Format of ATASPI IOCTL result:
Offset  Size    Description     (Table 90000)
 00h    DWORD   ATASPI entry point address

(Table 90001)
Call ATASPI entry point with:
        STACK:  DWORD -> ATA Request Block (ARB) (see #90003)
Notes:  caller must clean up the stack
        the large-model C declaration is
          void (*entry)(ARB_Struct *);
SeeAlso: #90000

(Table 90002)
Values for ATASPI command code:
 00h    ATA controller inquiry
 01h    get ATA device type
 02h    execute ATA I/O
 03h    abort ATA request
 04h    reset ATA device
 05h    set ATA controller parameter
 06h    get ATA disk drive information
SeeAlso: #90003

Format of ATA Request Block (ARB):
Offset  Size    Description     (Table 90003)
 00h    BYTE    command code (see #90002)
 01h    BYTE    (ret) status (see #90004)
 02h    BYTE    ATA controller number
 03h    BYTE    request flags (see #90005)
 04h    DWORD   reserved (0)
---request 00h---
 08h    BYTE    (ret) total number of ATA controllers
                valid only if ATA controller number = 0FFh
 09h    BYTE    reserved (0)
 0Ah 16 BYTEs   (ret) ATA manager ID
 1Ah 16 BYTEs   (ret) ATA controller ID
 2Ah 16 BYTEs   (ret) controller unique parameters (see #90010)
---request 01h---
 08h    BYTE    device ID (00h = master, 01h = slave)
 09h    BYTE    reserved (0)
 0Ah    BYTE    (ret) peripheral device type (see #90007)
---request 02h---
 08h    BYTE    device ID
 09h    BYTE    reserved (0)
 0Ah    DWORD   data  transfer length
                (ret) residual byte length
 0Eh    BYTE    sense allocation length (N)
 0Fh    DWORD   data  buffer pointer
 13h    DWORD   reserved (0)
 17h    BYTE    ACB length (M)
                for Task File requests must be set to 07h, for ATAPI Packet
                  requests must match the number of bytes in the packet
 18h    BYTE    (ret) ATA controller status (see #90006)
 19h    BYTE    (ret) device status
                value of the error register, 00h if no error (see #P0512)
 1Ah    DWORD   post routine  address (see #90009)
 1Eh    WORD    data transfer block size (in bytes)
                number of data bytes to transfer per hardware interrupt for Task
                  File requests; number of data bytes host prefers to transfer
                  per hardware interrupt for ATAPI Packet commands
 20h 32 BYTEs   reserved for ATASPI workspace
 40h  M BYTEs   ATA/ATAPI command block (ACB)
                contains AT Task File Structure (see #90008) if bit 2 of the
                  request flags is set, ATAPI packet (see #03236,#03237,#03238)
                  if bit 2 is cleared
40h+M N BYTEs   sense allocation area
---request 03h---
 08h    DWORD   address of ARB to be aborted
---request 04h---
 08h    BYTE    device ID
 09h 15 BYTEs   reserved (0)
 18h    BYTE    (ret) ATA controller status (see #90006)
 19h    BYTE    (ret) device status
 1Ah    DWORD   post routine address (see #90009)
 1Eh 34 BYTEs   reserved for ATASPI workspace
---request 05h---
 08h 16 BYTEs   controller unique parameters (vendor unique)
---request 06h---
 08h    BYTE    device ID
 09h    BYTE    reserved (0)
 0Ah    BYTE    (ret) drive flags (see #01461)
 0Bh    BYTE    (ret) INT 13h drive
 0Ch    BYTE    (ret) preferred head   translation
 0Dh    BYTE    (ret) preferred sector translation
 0Eh 10 BYTEs   reserved (0)

(Table 90004)
Values for ATASPI request status:
 00h    request in progress
 01h    request completed without error
 02h    request aborted by host
 04h    request completed with error
 80h    invalid request
 81h    invalid ATA controller number
 82h    ATA device not installed
 83h    ATA controller/device busy
Note:   if ATASPI ever returns 83h as the status, it is the responsibility of
          the driver/applications to re-send the request at a later time
SeeAlso: #90003

Bitfields for request flags:
Bit(s)  Description     (Table 90005)
 7      reserved (0)
 6      "ByteXfer"
        =0 use word transfer mode
        =1 use byte transfer mode
 5      (ATAPI device) DSC unavailable action (DUA)
        =0 queue the request and service it when DSC bit is set
        =1 return with status 83h
 4-3    direction
        00 direction determined by device
        01 data in
        10 data out
        11 no data transfer
 2      request type
        =0 ATAPI Packet Command
        =1 AT Task File Structure
 1      reserved (0)
 0      "Post"
        =0 disable posting
        =1 enable  posting
SeeAlso: #90003

(Table 90006)
Values for ATA controller status:
 00h    no error
 11h    device not present
 12h    data overrun/underrun
SeeAlso: #90003

(Table 90007)
Values for peripheral device type:
 00h    direct-access device (e.g. magnetic disk)
 01h    tape device (QIC-121 SCSI Architectural Model)
 02h-04h reserved
 05h    CD-ROM device
 06h    reserved
 07h    optical memory device (e.g. some optical disks)
 08h-0Bh reserved
 0Ch    tape device (Cost Sensitive Architectural Model)
 0Dh-1Eh reserved
 1Fh    unknown or no device type
 80h    non-ATAPI device
SeeAlso: #90003

Format of AT Task File Structure:
Offset  Size    Description     (Table 90008)
 00h    features register
 01h    sector count register
 02h    cylinder LSB register
 04h    cylinder MSB register
 05h    device/head  register (see #P0513)
 06h    command register (see #P0515)
SeeAlso: #90005

(Table 90009)
Values ATASPI post function is called with:
        STACK:  DWORD -> ARB (see #90003) which completed
        interrupts disabled
Return: EBP, EBX, ESI, EDI must be preserved
        interrupts disabled
Notes:  the post function may issue any ATASPI function except an abort; it
          should complete as quickly as possible
        the large-model C declaration is
          void (*post)(ARB_Struct *);
SeeAlso: #90003

Format of Future Domain controller unique parameters:
Offset  Size    Description     (Table 90010)
 00h    WORD    controller features
 02h    WORD    controller main I/O port
 04h    WORD    controller alternate I/O port
 06h    BYTE    controller IRQ
 07h    BYTE    ??? (00h or 01h)
 08h    BYTE    ??? (00h or 01h)
 09h  7 BYTEs   reserved (0)
SeeAlso: #90003
----------214402-----------------------------
INT 21 - DRFAT32.SYS device driver - IOCTL INPUT
        AX = 4402h
        BX = file handle referencing character device for DRFAT32.SYS driver
            (e.g. "FAT32XXX" in the default configuration)
        CX = number of bytes to read
        DS:DX -> control block (see #04108)
Return: CF clear if successful
            AX = number of bytes actually read
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:   the data returned depends on the first byte of the control block
           the remainder of the control block is filled by the driver
         one DRFAT32.SYS device driver supports up to 8 subunits (that is up
           to 8 FAT32 partitions). By default, DRFAT32.SYS logs in all FAT32
           partitions it finds, however, by using command line options /C (CHS),
           /L (LBA), /H (Hidden) and /P:1..4 (Primary), /E:1..255 (Logical
           Drive), and /U:min[,max] (Unit 128..255) it can be directed to
           attach only to one or a specific range of partitions. Multiple
           DRFAT32.SYS drivers can be loaded in a system when using the
           /D:name option. To assign them to the redirector, the /D:name option
           must be repeated for each of the DRFAT32.SYS drivers in the
           DRFAT32.EXE command line.
SeeAlso: AX=4402h"CD-ROM",INT 2F/AX=0802h

Format of DRFAT32 control block:
Offset  Size    Description     (Table 04108)
 00h    BYTE    data being requested
                00h device driver header address
                09h media change status
                48h address of DRFAT32 geometry table
                else error
---function 00h---
 01h    DWORD   device driver header address (see also AH=52h,#01646)
---function 09h---
 01h    BYTE    media change status
                00h don't know
                01h media unchanged
                FFh media has been changed
---DRFAT32 function 48h---
 01h    DWORD   address of DRFAT32 geometry table (see #04109)

Format of DRFAT32 Geometry Table:
Offset  Size    Description     (Table 04109)
 00h 8*59 BYTEs eight DRFAT32 partition data tables (see #04110)
 1D8h   BYTE    DRFAT32 access flags (see #04111)
 1D9h   BYTE    DRFAT32 status flags (see #04112)
Note:   This is the table layout used by DRFAT32.SYS 1.00 which supports up
          to 8 sub-units. The layout of this table and its records may change,
          so the version indicator in the device driver's signature ("FAT320")
          and the size of the public data structure should be checked first.
SeeAlso: #04108

Format of DRFAT32 partition data table:
Offset  Size    Description     (Table 04110)
 00h    BYTE    size of following public data structure (9)
        --- public data (needed by the DRFAT32.EXE redirector) ---
 01h    WORD    bytes per sector
 03h    BYTE    sectors per cluster
 04h    WORD    reserved sectors
 06h    DWORD   root directory start cluster
        --- private data ---
 0Ah    DWORD   sectors per track
 0Eh    DWORD   root directory sector
 12h    DWORD   partition start sector
 16h    DWORD   highest partition sector allowed
                (only if range checking enabled)
 1Ah    DWORD   absolute cluster start sector
                (cluster start sector+partition start sector)
 1Eh    DWORD   total sectors per track (number of heads * sectors per track)
 22h    BYTE    drive unit (default 80h)
 23h    BYTE    temp buffer: CHS sectors to write (if verify enabled)
 24h    BYTE    INT 13h extension version
        --- CHS data ---
 25h    WORD    cylinder
 27h    WORD    head
 29h    WORD    sector
        --- LBA data ---
 2Bh 16 BYTEs disk address packet (see #00272 at INT 13/AH=42h)
SeeAlso: #04109

Bitfields for DRFAT32 access flags:
Bit(s)  Description     (Table 04111)
 7      using LBA addressing instead of CHS
 6      using INT 13h extensions (see also INT 13/AH=48h)
 5      multi-sector access allowed
 4      force media change indication on removable drives
 3      force verify after every write (/W)
 2      more than 1024 cylinders, less than 64 heads (/A)
 1      client-side asynchronous buffering allowed (/B), normally =0
 0      read-only access (/R)
SeeAlso: #04109,#04112

Bitfields for DRFAT32 status flags:
Bit(s)  Description     (Table 04112)
 7-2    reserved (0)
 1      Save Guard enabled (blocks any further writes after a fault)
 0      volume may have dirty sectors (set after write operations)
SeeAlso: #04109,#04111
--------c-214403-----------------------------
INT 21 - SMARTDRV.SYS v3.x only - IOCTL - CACHE CONTROL
        AX = 4403h
        BX = handle for device "SMARTAAR"
        CX = number of bytes to write
        DS:DX -> SMARTDRV control block (see #01523)
Return: CF clear if successful
            AX = number of bytes actually written
                0000h if control block too small for given command
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   NCache2 (from the Norton Utilities v8.0) attempts to support this
          interface, but does not do so correctly, often hanging the system;
          one should use the SmartDrive v4.x or NCache private interfaces
          (see INT 2F/AX=4A10h/BX=0000h,INT 2F/AX=FE00h/DI=4E55h)
SeeAlso: AX=4402h"SMARTDRV",INT 2F/AX=4A10h/BX=0000h

(Table 01522)
Values for SMARTDRV function code:
 00h    flush cache
 01h    flush and discard cache
 02h    disable caching (flushes and discards cache first)
 03h    enable caching
 04h    control write caching
 05h    set flushing tick count
 06h    lock cache contents
 07h    unlock cache contents
 08h    set flush-on-reboot flag
 09h    unused
 0Ah    control full-track caching
 0Bh    reduce cache size
 0Ch    increase cache size
 0Dh    set INT 13 chain address

Format of SMARTDRV control block:
Offset  Size    Description     (Table 01523)
 00h    BYTE    function code (see #01522)
---functions 00h-03h,06h,07h---
 no additional fields
---function 04h---
 01h    BYTE    write caching control action
                00h turn off write-through
                01h turn on write-through
                02h turn off write buffering (also flushes cache)
                03h turn on write buffering (also flushes cache)
---function 05h---
 01h    WORD    number of timer ticks between cache flushes
---function 08h---
 01h    BYTE    new flush-on-reboot flag (00h off, 01h on)
---function 0Ah---
 01h    BYTE    full-track writes are
                00h not cached
                01h cached
---functions 0Bh,0Ch---
 01h    WORD    number of 16K pages by which to increase/reduce cache size
---function 0Dh---
 01h    DWORD   new address to which to chain on INT 13
Note:   the previous address is not preserved
--------d-214403-----------------------------
INT 21 - CD-ROM device driver - IOCTL OUTPUT
        AX = 4403h
        BX = file handle referencing character device for CD-ROM driver
        CX = number of bytes to write
        DS:DX -> control block (see #01524)
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  some device drivers support several subunits (that is more than one
          drive) but it is not possible to distinguish between them with this
          function; use INT 2F/AX=1510h or INT 2F/AX=0802h instead
        under Windows95, the "mscd$$$$" device cannot be opened so you cannot
          obtain the handle required by this function: use INT 2F/AX=1510h
          or INT 2F/AX=0802h instead
SeeAlso: AX=4402h"CD-ROM",INT 2F/AX=0802h,INT 2F/AX=1510h

Format of CR-ROM control block:
Offset  Size    Description     (Table 01524)
 00h    BYTE    function code
                00h eject disk
                01h lock/unlock door
                02h reset drive
                03h control audio channel
                04h write device control string
                05h close tray
---functions 00h,02h,05h---
 no further fields
---function 01h---
 01h    BYTE    lock function
                00h unlock door
                01h lock door
---function 03h---
 01h    BYTE    input channel (0-3) for output channel 0
 02h    BYTE    volume for output channel 0
 03h    BYTE    input channel (0-3) for output channel 1
 04h    BYTE    volume for output channel 1
 05h    BYTE    input channel (0-3) for output channel 2
 06h    BYTE    volume for output channel 2
 07h    BYTE    input channel (0-3) for output channel 3
 08h    BYTE    volume for output channel 3
Note:   output channels 0 and 1 are left and right, 2 and 3 are left prime and
          right prime; a volume of 00h is off
---function 04h---
 01h  N BYTEs   bytes to send directly to the CD-ROM drive without
                  interpretation
--------d-214403-----------------------------
INT 21 - Brian Antoine Seagate ST-01 SCSI.SYS - IOCTL - EXECUTE COMMANDS
        AX = 4403h
        BX = handle for device "SCSITAPE"
        CX = number of bytes to write
        DS:DX -> SCSITAPE control block (see #01525)
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4405h"ST-01",INT 78/AH=10h

Format of SCSITAPE control block:
Offset  Size    Description     (Table 01525)
 00h    WORD    command type
                'F' Format (argument 1 = interleave, argument 2 = format type)
                'E' Erase
                'R' Rewind
                'L' Load
                'N' No Load
                'S' Space (argument 1 = count, argument 2 = type)
                'M' File Mark (argument 1 = count)
                'A' Reassign
 02h    WORD    argument 1
 04h    WORD    argument 2
 06h    WORD    segment of command buffer
 08h    WORD    offset of command buffer
 0Ah    WORD    length of command buffer
--------E-214403-----------------------------
INT 21 U - AI Architects - OS/x86??? - API
        AX = 4403h
        BX = handle for device "AIA_OS"
        CX = number of bytes to write (ignored)
        DS:DX -> 12-byte buffer (see #01526), first byte is command:
                  81h installation check
                  82h get API entry point
                  84h uninstall
Return: CF clear if successful
            AX = number of bytes actually written
            DS:DX buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Ch,0Dh) (see #01680 at AH=59h)
Notes:  these functions are only available if the DOS extender was loaded as a
          device driver in CONFIG.SYS
        called by TKERNEL (a licensed version of AI Architects/Ergo OS/x86)
SeeAlso: INT 2F/AX=FBA1h/BX=0081h,INT 2F/AX=FBA1h/BX=0082h
Index:  installation check;OS/x86|entry point;OS/x86|uninstall;OS/x86

Format of buffer on return:
Offset  Size    Description     (Table 01526)
 00h  4 BYTEs   signature "IABH"
---if function 81h---
 (no additional fields)
---if function 82h---
 04h    DWORD   pointer to API entry point (see INT 2F/AX=FBA1h/BX=0082h)
---if function 84h---
 04h    WORD    success indicator
 06h    WORD    segment of ???
 08h    WORD    segment of ??? memory block to free if nonzero
 0Ah    WORD    segment of ??? memory block to free if nonzero
--------V-214403-----------------------------
INT 21 - PGS1600.DEV - IOCTL - SET CONFIGURATION???
        AX = 4403h
        BX = file handle for device "PGS1600$"
        CX = 0018h (size of buffer)
        DS:DX -> configuration buffer (see #01484 at AX=4402h"PGS1600")
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: PGS1600.DEV is a device driver for the Cornerstone Technology PG1600
          display adapter, which provides a 1600x1200 monochrome display as
          well as one of two emulations, MDA or CGA.
SeeAlso: AX=4402h"PGS1600"
--------N-214403-----------------------------
INT 21 - PC/TCP IPCUST.SYS - RESET CONFIGURATION DATA READ POINTER
        AX = 4403h
        BX = file handle referencing device "$IPCUST"
        CX, DS:DX ignored
Return: CF clear if successful
            AX destroyed
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  there are a total of 378h bytes of configuration data for IPCUST.SYS
          version 2.05.  If less than the entire data is read or written,
          the next read/write continues where the previous one ended; this
          call and AX=4402h both reset the location at which the next
          operation starts to zero
        v2.1+ uses a new configuration method, but allows the installation
          of IPCUST.SYS for backward compatibility with other software which
          must read the PC/TCP configuration
SeeAlso: AH=3Fh"IPCUST",AH=40h"IPCUST",AX=4402h"IPCUST"
--------V-214403-----------------------------
INT 21 - Compaq AG1024.SYS - CPQ_MGES - IOCTL OUTPUT
        AX = 4403h
        BX = file handle referencing device "CPQ_MGES"
        DS:DX -> request packet (see #01527)
        CX ignored
Return: CF clear if successful
            AX destroyed
            data buffer filled (if applicable)
            first word of request packet set to number of bytes of data
              available (amount returned is smaller of this and requested
              amount)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: AG1024.SYS is a device driver for the Advanced Graphics 1024 adapter
SeeAlso: AX=4403h"RGDI"

Format of Compaq AG1024.SYS request packet:
Offset  Size    Description     (Table 01527)
 00h    WORD    function
                0000h get ??? (26h bytes)
                0001h get ??? (36h bytes)
                0002h set ??? (same as returned by function 0001h)
                0003h get ??? (6 bytes)
                0004h get ???
                0005h get ???
                0006h get ??? (10h bytes)
                0007h set ??? (same as returned by function 0006h)
---functions 00h-03h,06h,07h---
 02h    WORD    size of data buffer
 04h    DWORD   -> buffer for function's data
 ---functions 04h,05h---
 02h    WORD    ???
 04h    WORD    size of data buffer
 06h    DWORD   -> buffer to receive data
--------V-214403-----------------------------
INT 21 - Compaq AG1024.SYS - RGDI - IOCTL OUTPUT
        AX = 4403h
        BX = file handle referencing device "$$$$RGDI"
        DS:DX -> request packet (see #01528)
        CX ignored
Return: CF clear if successful
            AX destroyed
            data buffer filled (if applicable)
            first word of request packet set to number of bytes of data
              available (amount returned is smaller of this and requested
              amount)
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: AG1024.SYS is a device driver for the Advanced Graphics 1024 adapter
SeeAlso: AX=4402h"RGDI",AX=4403h"CPQ_MGES"

Format of Compaq AG1024.SYS request packet:
Offset  Size    Description     (Table 01528)
 00h    WORD    function
                0000h get entry points
                0001h get ???
 02h    DWORD   address of buffer for returned data
--------m-214403SF01-------------------------
INT 21 U - Qualitas 386MAX v6.01+ - TURN 386MAX OFF
        AX = 4403h subfn 01h
        BX = handle for device "386MAX$$"
        DS:DX -> BYTE 01h
        CX ignored
Return: DS:DX -> BYTE status (00h = successful)
        CF clear if successful
            AX destroyed
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   this function will fail if any EMS or UMBs are in use
SeeAlso: AX=4403h/SF=02h,AX=4403h/SF=03h,AX=4403h/SF=05h
--------m-214403SF02-------------------------
INT 21 U - Qualitas 386MAX v6.01+ - TURN 386MAX ON
        AX = 4403h subfn 02h
        BX = handle for device "386MAX$$"
        DS:DX -> BYTE 02h
        CX ignored
Return: DS:DX -> BYTE status (00h = successful)
        CF clear if successful
            AX destroyed
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4403h/SF=01h,AX=4403h/SF=03h,AX=4403h/SF=05h
--------m-214403SF03-------------------------
INT 21 U - Qualitas 386MAX v6.01+ - SET STATE
        AX = 4403h subfn 03h
        BX = handle for device "386MAX$$"
        CX = number of bytes to copy (up to size of state buffer)
        DS:DX -> BYTE 03h followed by state buffer
                  (see #01467 at AX=4402h"386MAX")
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   the first byte of the buffer must be either 01h, 02h, or 03h
          (specifying the version of the state record) and the buffer must
          contain CX bytes AFTER the initial byte
SeeAlso: AX=4402h"386MAX"
--------m-214403SF05-------------------------
INT 21 U - Qualitas 386MAX v7.00+ - LIMIT AUTOMATIC ACTIVATION TO STD EMS CALLS
        AX = 4403h subfn 05h
        BX = handle for device "386MAX$$"
        DS:DX -> BYTE 05h
        CX ignored
Return: CF clear if successful
            AX destroyed
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Desc:   specifies that 386MAX will only shift from AUTO to ON mode on standard
          EMS calls INT 67/AH=40h-5Dh
SeeAlso: AX=4403h/SF=02h,AX=4403h/SF=03h,AX=4403h/SF=06h
--------m-214403SF06-------------------------
INT 21 U - Qualitas 386MAX v7.00+ - ALLOW AUTOMATIC ACTIVATION ON ANY INT 67
        AX = 4403h subfn 06h
        BX = handle for device "386MAX$$"
        DS:DX -> BYTE 06h
        CX ignored
Return: CF clear if successful
            AX destroyed
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Desc:   specified that 386MAX should shift from AUTO to ON mode on any INT 67
          call other than INT 67/AH=3Fh
SeeAlso: AX=4403h/SF=01h,AX=4403h/SF=03h,AX=4403h/SF=05h
--------n-214403-----------------------------
INT 21 U - PenDOS PENDEV.SYS - ???
        AX = 4403h
        BX = file handle for device "$$PENDOS" or "$$PD_REG"
        CX = size of buffer
        DS:DX -> buffer containing ???
Return: CF clear if successful
            buffer filled
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: A limited version of PenDOS by Communication Intelligence Corporation,
          which provides pen capability to keyboard-based programs, is bundled
          with IBM DOS 6.1
Note:   this call sets the WORD at offset 1Ah into the device driver request
          header used to call the driver to 0000h.
SeeAlso: AX=4402h"PENDEV.SYS"
----------214403-----------------------------
INT 21 - DR DOS $IDLE$ - IOCTL - DYNAMIC IDLE DETECTION CONTROL
        AX = 4403h
        BX = file handle referencing character device $IDLE$
        CX = number of bytes to write (0004h)
        DS:DX -> $IDLE$ IOCTL structure (see #04113)
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Notes:  When the DR DOS IBMBIO.COM detects the presence of the $IDLE$ device
          during startup, it will retrieve the address of the idle state
          data area via INT 21/AX=4458h and pass it along to the $IDLE$ device
          driver by calling this function.

Format of DR DOS $IDLE$ IOCTL structure:
Offset  Size    Description     (Table 04113)
 00h    DWORD   -> idle state data area (see Table !!! at INT 21/AX=4458h)
--------k-214404-----------------------------
INT 21 - Stacker - GET DEVICE DRIVER ADDRESS AND SET VOLUME NUMBER
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc.)
        CX = 0004h
        DS:DX -> DWORD buffer to receive device driver address
Return: buffer filled with pointer into Stacker device driver (see #02550):
          driver + 19h if Stacker Anywhere controls this drive
          driver + 1Ah if Stacker controls this drive
          unchanged else
Notes:  in addition to returning the address of the Stacker device driver,
          this call also sets the volume number at offset 58h in the device
          driver (see #02550 at INT 25/AX=CDCDh)
        Stacker Anywhere does not link its built-in device driver into
          the standard device driver chain, but it can be found via CDS/DPB
        this call can be used as an installation check for all versions of
          Stacker and Stacker Anywhere to avoid the INT 25 call
SeeAlso: AX=4408h,AX=440Eh,AH=52h,INT 25/AX=CDCDh
--------k-214404-----------------------------
INT 21 - Stacker - GET STACVOL FILE SECTORS
        AX = 4404h
        BL = drive number (0 is current drive)
        CX = byte count (i.e., 200h = 1 sector)
        DS:DX -> buffer (see #01529)
Return: Data Buffer contains the number of sectors requested from the
          STACVOL physical file for the drive specified.

Format of Stacker buffer:
Offset  Size    Description     (Table 01529)
 00h    WORD    01CDh
 02h    WORD    sector count
 04h    DWORD   number of starting sector
 08h    DWORD   far pointer to Data Buffer
--------k-214404-----------------------------
INT 21 - DUBLDISK.SYS v2.6 - GET INFO
        AX = 4404h
        BL = drive number of DoubleDisk drive (00h = default, 01h = A:, etc.)
        CX = number of bytes (000Ah-0014h, call ignored otherwise)
        DS:DX -> data record (see #01530)
Return: CF clear if successful
            AX = number of bytes read
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: DUBLDISK.SYS is the device driver portion of DoubleDisk, a disk
          expander by Vertisoft Systems, Inc.
InstallCheck:   for v2.6, scan memory for the signature "FAT 2.6  byte:",
          which is immediately followed by a data table (see #01531)
BUG:    VOPT (a disk optimizer by Golden Bow Software) assumes that any driver
          which returns the "correct" number of bytes when the first word of
          the buffer for the data record contains the signature value 4444h is
          DoubleDisk; a workaround is for the non-DoubleDisk driver to return
          no data if the signature is present
SeeAlso: AX=440Dh"DOS 3.2+"
Index:  installation check;DUBLDISK.SYS

Format of DUBLDISK data record:
Offset  Size    Description     (Table 01530)
 00h    WORD    (call) signature 4444h
 02h    BYTE    (call) function
                        00h ???
                        01h ???
---function 00h---
 02h    BYTE    (ret) ???
 03h    BYTE    (ret) ???
---function 01h---
 02h    WORD    (ret) 4444h
 04h    WORD    allocation unit size???
 06h    WORD    ???
 08h    WORD    ???
 0Ah    BYTE    ???

Format of DUBLDISK signature data table:
Offset  Size    Description     (Table 01531)
 00h  5 BYTEs   ???
 05h    BYTE    first drive number
 06h    BYTE    number of drives
 07h    ???
--------k-214404-----------------------------
INT 21 - DBLSPACE.BIN - IOCTL - FLUSH OR INVALIDATE INTERNAL CACHES
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = 000Ah (size of DSPACKET structure)
        DS:DX -> DSPACKET structure (see #01532)
Return: CF clear if IOCTL successful -- check DSPACKET for actual status
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4405h"DBLSPACE",INT 2F/AX=4A11h/BX=0000h

Format of DoubleSpace DSPACKET structure:
Offset  Size    Description     (Table 01532)
 00h    WORD    signature 444Dh ("DM")
 02h    BYTE    command code
                46h ('F') flush internal caches
                49h ('I') flush and invalidate internal caches
 03h    WORD    result code
                (ret) 4F4Bh ("OK") if successful, else unchanged
 05h  5 BYTEs   padding
--------k-214404-----------------------------
INT 21 - DBLSPACE.BIN v6.2 - IOCTL - GET ??? FOR SPECIFIED DRIVE
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = size of DSPACKET structure (ignored in DOS 6.2)
        DS:DX -> DSPACKET structure (see #01533)
Return: CF clear if IOCTL successful -- check DSPACKET for actual status
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4405h"DBLSPACE",INT 2F/AX=4A11h/BX=0000h

Format of DoubleSpace DSPACKET structure:
Offset  Size    Description     (Table 01533)
 00h    WORD    signature 444Dh ("DM")
 02h    BYTE    command code
                53h ('S') get ??? for specified drive
 03h    WORD    result code
                (ret) 4F4Bh ("OK") if successful, else unchanged
 05h    DWORD   (ret) pointer to 96-byte ??? data
 09h    DWORD   (ret) pointer to ??? data
 0Dh  3 BYTEs   reserved
--------k-214404-----------------------------
INT 21 U - DoubleTools v1.0 - GET ???
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = 0006h
        DS:DX -> DoubleTools structure (see #01534)
Return: CF clear if IOCTL successful -- check DSPACKET for actual status
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   the Transporter device driver has the signature 55h 61h 50h 74h
          ("DaPt") twelve bytes after it beginning

Format of DoubleTools structure:
Offset  Size    Description     (Table 01534)
 00h    WORD    (call) signature 55h AAh
                (ret) signature 78h 70h ("xp")
 02h    WORD    (call) signature 52h 16h
                (ret) ???
 04h    WORD    (ret) segment of driver's DS (same as its PSP)
--------d-214404-----------------------------
INT 21 U - xDISK v3.31 - CONFIGURE
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = 0047h (length of version string)
        DS:DX -> 79-byte buffer for version string and ???
        DS:0081h = commandline containing new switches for driver
Return: CF clear if successful
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: xDISK is a shareware resizeable EMS RAMdisk by FM de Monasterio
SeeAlso: AX=4405h"xDISK",INT 2F/AH=DDh/BX=7844h"xDISK"
--------c-214404-----------------------------
INT 21 - COMBI-disk v1.13 - GET DATA RECORD
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = 0032h (length of data packet)
        DS:DX -> buffer for data packet (see #01535)
Return: CF clear if successful
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: COMBI-disk is a shareware combination cache and RAMdisk sharing a
          single pool of memory by Vadim V. Vlasov
InstallCheck:   scan the valid drives for one which returns a correctly-sized
          data packet with the proper signature in the first field
SeeAlso: AX=4405h"COMBI"
Index:  installation check;COMBI-disk

Format of COMBI-disk data packet:
Offset  Size    Description     (Table 01535)
 00h  6 BYTEs   ASCIZ signature "COMBI"
 06h    WORD    version (high byte = major, low = decimal minor version)
 08h    BYTE    current options
 09h    BYTE    sectors per allocation block
 0Ah    WORD    maximum buffer in KB
 0Ch    WORD    current buffer in KB (less than max if XMS memory being lent)
 0Eh    WORD    total number of allocation blocks
 10h    WORD    current number of allocation blocks
 12h    WORD    number of blocks being used by RAM disk
 14h    WORD    number of blocks being used by cache or unused
 16h    WORD    number of dirty cache blocks
 18h    WORD    number of blocks which could not be written out due to errors
 1Ah    WORD    total number of read requests
 1Ch    WORD    total number of sectors read
 1Eh    WORD    number of BIOS read requests (cache misses)
 20h    WORD    number of sectors read via BIOS (cache misses)
 22h    WORD    total number of write requests
 24h    WORD    total number of sectors written
 26h    WORD    number of BIOS write requests
 28h    WORD    number of sectors written via BIOS
 2Ah    WORD    number of RAM disk read requests
 2Ch    WORD    number of sectors read from RAM disk
 2Eh    WORD    number of RAM disk write requests
 30h    WORD    number of sectors written to RAM disk
--------d-214404-----------------------------
INT 21 - SFS v1.00 - GET CONFIGURATION INFORMATION
        AX = 4404h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = 001Ah (length of data packet)
        DS:DX -> buffer for data packet (see #01536)
Return: CF clear if successful
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: SFS (Secure FileSystem) is a shareware encrypting disk driver by
          Peter C. Gutmann
SeeAlso: AX=4405h"SFS"

Format of SFS data packet:
Offset  Size    Description     (Table 01536)
 00h  4 BYTEs   signature string "SFS1"
 04h    WORD    SFS unit number (0-based)
 06h    WORD    drive on which the SFS volume is mounted
 08h    DWORD   sector offset of logical volume from start of physical volume
                0 if logical volume = physical volume
 0Ch    WORD    flag: 00h = no disk mounted, 01h = disk mounted
 0Eh    WORD    flag: 00h read/write, 01h read-only
 10h    WORD    quick-unmount hotkey
                (high byte = shift state, low = scan code) (see #00006)
 12h    WORD    auto-unmount time in minutes, or 0000h if already expired, or
                FFFFh if not set
 14h    WORD    time in minutes remaining before auto-unmount
 16h    WORD    internal driver check code
                0000h no error
                0001h driver consistency check failed
                0002h unit consistency check failed
 18h    WORD    disk access mode
                0000h BIOS
                0001h IDE direct access
                0002h SCSI direct access
--------d-214405-----------------------------
INT 21 - Brian Antoine Seagate ST-01 SCSI.SYS - IOCTL - EXECUTE COMMANDS
        AX = 4405h
        BX = drive number (00h = default, 01h = A:, etc)
        CX = number of bytes to write
        DS:DX -> SCSIDISK control block (see also #01525 at AX=4403h"ST-01")
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4403h"ST-01"
--------k-214405-----------------------------
INT 21 U - DBLSPACE.BIN - IOCTL - FLUSH OR INVALIDATE INTERNAL CACHES
        AX = 4405h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = 000Ah (size of DSPACKET structure)
        DS:DX -> DSPACKET structure (see #01537)
Return: CF clear if IOCTL successful -- check DSPACKET for actual status
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   this call is identical to the documented AX=4404h
SeeAlso: AX=4404h"DBLSPACE",INT 2F/AX=4A11h/BX=0000h

Format of DoubleSpace DSPACKET structure:
Offset  Size    Description     (Table 01537)
 00h    WORD    signature 444Dh ("DM")
 02h    BYTE    command code
                46h ('F') flush internal caches
                49h ('I') flush and invalidate internal caches
 03h    WORD    result code
                (ret) 4F4Bh ("OK") if successful, else unchanged
 05h  5 BYTEs   padding
--------d-214405-----------------------------
INT 21 U - xDISK v3.31 - ???
        AX = 4405h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = number of bytes to write
        DS:DX -> buffer containing version string
        ???
Return: CF clear if successful
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: xDISK is a shareware resizeable EMS RAMdisk by FM de Monasterio
SeeAlso: AX=4404h"xDISK",INT 2F/AH=DDh/BX=7844h"xDISK"
--------d-214405-----------------------------
INT 21 - COMBI-disk v1.13 - CONTROL COMBI-disk
        AX = 4405h
        BL = drive number (00h = default, 01h = A:, etc) for RAM disk
        CX = number of bytes to write
        DS:DX -> buffer containing command packet (see #01539)
Return: CF clear if successful
            AX = number of bytes actually transferred
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AX=4404h"COMBI"

(Table 01538)
Values for COMBI-disk command code:
 80h    flush cache
 81h    change options byte
 82h    shrink memory
 83h    expand memory
 84h    get dirty block information
 85h    reset write errors
 86h    reset counters

Format of COMBI-disk command packet:
Offset  Size    Description     (Table 01539)
 00h    WORD    version
 02h    BYTE    command code (see #01538)
---command code 80h---
 no additional fields
---command code 81h---
 03h    BYTE    new options byte (see #01540)
---command code 82h---
 03h    WORD    number of KB to release
---command code 83h---
 03h    WORD    number of KB to expand
---command code 84h---
 03h    DWORD   -> buffer for block info (see #01541)
---command code 85h---
 03h    DWORD   block ID
---command code 86h---
 03h    BYTE    which counters to reset
                bit 0: hard disk read counts
                bit 1: hard disk write counts
                bit 2: RAM disk read/write counts
Note:   multiple commands may be placed in a single packet by stringing
          together as many command/argument pairs as desired

Bitfields for COMBI-disk options byte:
Bit(s)  Description     (Table 01540)
 0      cache off
 1      cache frozen
 2      write caching enabled
 3      delayed writing disabled
 5      fix memory allocation (no XMS lending)
 6      no 'sector not found' error

Format of COMBI-disk block info:
Offset  Size    Description     (Table 01541)
 00h    DWORD   block ID
 04h    BYTE    bitmask of valid sectors in block
 05h    BYTE    bitmask of dirty sectors in block
 06h    BYTE    last error returned by BIOS
 07h    BYTE    number of errors
--------d-214405-----------------------------
INT 21 - SFS v1.00 - DRIVER CONTROL
        AX = 4405h
        BL = drive number (00h = default, 01h = A:, etc)
        CX = number of bytes to write
        DS:DX -> data to write (see #01543)
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Program: SFS (Secure FileSystem) is a shareware encrypting disk driver by
          Peter C. Gutmann
SeeAlso: AX=4404h"SFS"

(Table 01542)
Values for SFS control function:
 00h    "PACKET_SET_DISKINFO"    set disk parameters
 01h    "PACKET_SET_KEYINFO"     set keying information
 02h    "PACKET_SET_READONLY"    select read/write or read-only
 03h    "PACKET_SET_DRIVENO"     set drive number to mount
 04h    "PACKET_SET_MOUNTSTATUS" set mount status
 05h    "PACKET_SET_UNMOUNT"     set/clear quick-unmount hotkey
 06h    "PACKET_SET_TIMEOUT"     set/clear auto-unmount timeout

Format of SFS control data packet:
Offset  Size    Description     (Table 01543)
 00h    WORD    signature 4330h ('C0')
 02h    WORD    function (see #01542)
---function 00h---
 04h    WORD    sector size in bytes
 06h    BYTE    sectors per cluster
 07h    WORD    number of boot sectors
 09h    BYTE    number of copies of FAT
 0Ah    WORD    size of root directory in entries
 0Ch    WORD    number of sectors on disk, 16-bit
 0Eh    BYTE    media descriptor byte
 0Fh    WORD    sectors per FAT
 11h    WORD    sectors per track
 13h    WORD    number of heads
 15h    DWORD   number of hidden sectors
 19h    DWORD   number of sectors on disk, 32-bit
---function 01h---
 04h 20 BYTEs   master IV for encrypted disk
 18h 64 BYTEs   NDC/SHS keying information
---function 02h---
 04h    WORD    read-only state: 00h read-only, 01h read/write
---function 03h---
 04h    WORD    drive number (see #01544)
 06h    DWORD   sector offset of logical volume from start of physical volume
                0 if logical volume = physical volume
---function 04h---
 04h    WORD    mount status (00h unmounted, 01h mounted)
---function 05h---
 04h    WORD    hotkey (high byte = shift state, low byte = scan code or 00h)
                (see #00006)
                0000h to disable hotkey
---function 06h---
 04h    WORD    timeout in minutes before automatic unmount
                0000h to disable auto-unmount
Notes:  the data for function 00h corresponds to a DOS BPB (see AH=53h)
        functions 00h, 01h, and 03h automatically unmount the encrypted drive
        unmounting a drive with function 04h also destroys the encryption
          information in the driver and forces all dirty buffers to be flushed

Bitfields for SFS drive number:
Bit(s)  Description     (Table 01544)
 15-12  drive access mode
        0000 BIOS access
        0001 direct IDE access
        0010 direct SCSI access
---if BIOS access---
 11-8   unused (0)
 7-0    BIOS drive number
---if IDE access---
 11-8   unused (0)
 7-0    IDE drive number
---if SCSI access---
 11-8   SCSI host number
 7-4    SCSI target ID
 3-0    SCSI logical unit number
----------214405-----------------------------
INT 21 U - SUPERSTOR - IOCTL - INSTALL CHECK
        AX = 4405h
        BL = drive number (00h = default, 01h = A:, etc)
        (BH = 0???)
        CX = 000Ch (size of SuperStor packet structure)
        DS:DX -> SuperStor packet structure (see #04114)
Return: CF clear if IOCTL successful -- check SuperStor packet for actual
              status
            AX = number of bytes actually transferred???
        CF set on error
            AX = error code (01h,05h,06h,0Dh) (see #01680 at AH=59h/BX=0000h)
Note:   This function is called with CX = 0008h for command 06h by NWCACHE
          although the Request Packet can be larger.  WINSWAP.SYS calls it
          with 000Ch - also for command 06h.

Format of SuperStor packet:
Offset  Size    Description     (Table 04114)
 00h    WORD    (call) product signature AA55h
                (ret) result code, 0000h if successful
 02h    BYTE    (call) SuperStor product ID (01h)
 03h    BYTE    (call) SuperStor command parameter:
                06h = get compressed drive structure / drive parameters
                07h = return the compressed length of a file
                08h = return real drive ID for swapped drive
                09h = validate the contents of the cluster
                0Bh = checks whether deleted cluster is free
                0Ch = reallocate previously deleted cluster
                0Dh = read absolute sector from the drive
                0Eh = write absolute sector to the drive
                0Fh = instruct the driver to rebuild tables
                10h = flush any cached Replacement Block Table (RBT) sectors
                      to the drive
---command 06h---
 04h    DWORD   (ret) pointer to SuperStor CVF's compressed unit structure
                      (see #04115)
 08h    DWORD   (ret) pointer to internal status structure
---command 07h---
 04h    DWORD   (ret) size of file in bytes
 08h    WORD    (ret) first cluster number
---command 08h---
 04h    BYTE    (ret) swapped drive (0-based)
---commands 09h,0Bh,0Ch---
 04h    WORD    (call) cluster
---commands 0Dh,0Eh---
 04h    DWORD   (call) first logical sector to transfer
 08h    WORD    (call) number of logical sectors to transfer
 0Ah    DWORD   (call) pointer to buffer
---command 0Fh---
 04h    WORD    (call) temporary buffer area
---command 10h---
 no additional parameters

Format of SuperStor CVF compressed unit structure:
Offset  Size    Description     (Table 04115)
 00h 31 BYTEs   BIOS parameter block for compressed drive (see #01663)
 1Fh  5 BYTEs   reserved for future expansion of the BPB
 24h 33 BYTEs   DOS 4-6 style drive parameter block for compressed drive
                  (see #01395)
 45h  2 BYTEs   reserved for future expansion of the DPB
 47h    WORD    first sector of relocation table (RBT)
 49h    WORD    first sector of file allocation table
 4Bh    WORD    first sector of the data area
 4Dh    BYTE    sec2byteshift
 4Eh  2 BYTEs   ???
                Note:   Reportedly these two bytes did not exist in ADDSTOR's
                          specification, so the whole structure would have
                          been documented differently.
 50h    WORD    sectors per cluster
 52h    BYTE    sector shift
 53h    WORD    compression ratio
 55h    DWORD   number of physical sectors in the host file
 59h    WORD    OS version
 5Bh    BYTE    SuperStor level
 5Ch    BYTE    unknown???
 5Dh    BYTE    flags
                bits 7-1: ???
                bit 0: drive is mounted
 5Eh    BYTE    unknown???
 5Fh    DWORD   pointer to CVF's underlaying host DCB
                this should be the physical DPB for this unit
                    ->  +00h BYTE host unit DOS unit number
                        +01h BYTE CVF's driver host subunit number
Note:   As this SuperStor compressed unit structure's structure is located
          within the segment the SuperStor driver has occupied, the segment
          value of its address can be used as an installation check, since it
          contains the signature "ADDSTOR" at offset 20h in the SuperStor
          driver.  This is used by Novell DOS 7 WINSWAP.SYS to check for the
          presence of SuperStor.
SeeAlso: #04114
Index:  installation check;SuperStor
--------d-21440C-----------------------------
INT 21 - Greg Shenaut ASPITAPE.SYS - INTERFACE
        AX = 440Ch
        BX = device handle
        CH = category code
            07h tape (ASPITAPE.SYS)
        CL = function
            01h "mtop" - perform tape operation
            02h "mtget" - get tape status
            03h ignore end-of-tape errors
            04h enable end-of-tape errors
        DS:DX -> parameter block (see #01554,#01555)
Return: CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            DS:DX -> data block
Notes:  This device driver is a simple DOS interface to the Adaptec Advanced
          SCSI Programming Interface (ASPI).  It provides the following device
          names as access to the SCSI tape, 'RMTx' (rewind on close) and
          'NRMTx' (NO rewind on close) where x can go from 0 to 3.  There may
          also be the following names 'MTx' and 'NMTx' which default to 1024
          byte blocks.  The names may also have a '$' appended to try and make
          them unique from file names of 'RMT0' etc.
        once opend these devices must be put into RAW mode
SeeAlso: AX=4402h"ASPI"

Format of ASPITAPE.SYS mtop parameter block:
Offset  Size    Description     (Table 01554)
 00h    WORD    operation code
                00h "MTWEOF" - write an end-of-file record
                01h "MTFSF" - forward space file
                02h "MTBSF" - backward space file
                03h "MTFSR" - forward space record
                04h "MTBSR" - backward space record
                05h "MTREW" - rewind
                06h "MTOFFL" - rewind and unload
                07h "MTNOP" - perform TEST UNIT READY
 02h    DWORD   repetition count

Format of ASPITAPE.SYS mtget parameter block:
Offset  Size    Description     (Table 01555)
 00h    BYTE    ASPI host ID
 01h    BYTE    SCSI target ID
 02h    BYTE    SCSI logical unit number
 03h    BYTE    device parameters
                bit 0: drive must use fixed-block read and write
                bit 7: drive is an ASPI device
 04h    BYTE    current device state (see #01556)
 05h    BYTE    unit number within driver
 06h    WORD    fixed block blocksize
 08h    BYTE    last SCSI status
 09h    BYTE    last SCSI sense key
 0Ah    WORD    last SCSI opcode (packed) (see #01557)
 0Ch    WORD    residual bytes from SCSI opcode

Bitfields for ASPITAPE.SYS current device state:
Bit(s)  Description     (Table 01556)
 0      device currently opened in buffered mode
 1      drive currently opened in nonbuffered mode
 2      rewind drive on last close
 3      drive has been written on
 4      drive has been read from
 5      next read will return 0 bytes
 6      EOM will resemble EOF
 7      drive may be busy rewinding

Bitfields for SCSI opcode:
Bit(s)  Description     (Table 01557)
 0-7    SCSI operation (SCSI packet byte 0)
 8-10   SCSI flags (SCSI packet byte 1)
 11-12  ASPI "Direction Bits" (ASPI SRB byte 3)
--------d-214410BXFFFF-----------------------
INT 21 U - NewSpace - ENABLE DRIVER
        AX = 4410h
        BX = FFFFh
Program: NewSpace is a TSR by Isogon Corporation which automatically compresses
          all files as they are written and decompresses them as they are read
Note:   compressed files are not accessible unless the driver is enabled
SeeAlso: AX=4411h/BX=FFFFh
--------d-214411BXFFFF-----------------------
INT 21 U - NewSpace - DISABLE DRIVER
        AX = 4411h
        BX = FFFFh
Program: NewSpace is a TSR by Isogon Corporation which automatically compresses
          all files as they are written and decompresses them as they are read
Note:   compressed files are not accessible unless the driver is enabled
SeeAlso: AX=4410h/BX=FFFFh
--------O-214412-----------------------------
INT 21 O - DR DOS 5.0-6.0 - DETERMINE DOS TYPE
        AX = 4412h
        CF set
Return: CF set if not DR DOS
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if DR DOS
            DX = AX = version code (see #01580)
Note:   this obsolete call, which is no longer supported in Novell DOS 7, is
          identical to AX=4452h
SeeAlso: AX=4452h
--------d-214412BXFFFF-----------------------
INT 21 U - NewSpace - INSTALLATION CHECK???
        AX = 4412h
        BX = FFFFh
Return: AX = PSP segment of NewRes (resident driver for NewSpace)
        BX:DX -> ???
        CX = ???
SeeAlso: AX=4411h/BX=FFFFh
--------d-214413BXFFFF-----------------------
INT 21 U - NewSpace - GET ???
        AX = 4413h
        BX = FFFFh
Return: AX = code segment of NewRes (resident driver for NewSpace)
        BX = offset of ???
SeeAlso: AX=4412h/BX=FFFFh
--------O-214414-----------------------------
INT 21 OU - DR DOS 5.0-6.0 - SET GLOBAL PASSWORD
        AX = 4414h
        DS:DX -> password string (blank-padded to 8 characters)
Desc:   Specify the master password for accessing files.
Note:   this obsolete call, which is no longer supported in Novell DOS 7, is
          identical to AX=4454h
SeeAlso: AX=4454h
--------d-214414BXFFFF-----------------------
INT 21 U - NewSpace - DEBUGGING DUMP
        AX = 4414h
        BX = FFFFh
Return: debugging dump written to X:\NEWSPACE.SMP
SeeAlso: AX=4413h/BX=FFFFh,AX=44FFh/BX=FFFFh
--------O-214416-----------------------------
INT 21 OU - DR DOS 5.0-6.0 - HISTORY BUFFER, SHARE, AND HILOAD CONTROL
        AX = 4416h to 4418h
Note:   these obsolete subfunctions (which are no longer supported in Novell
          DOS 7) are identical to AX=4456h through 4458h
SeeAlso: AX=4456h,AX=4457h,AX=4458h
--------O-214451-----------------------------
INT 21 - Concurrent DOS v3.2+ - INSTALLATION CHECK
        AX = 4451h
Return: CF set if not Concurrent DOS
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if successful
            AH = single-user/multiuser nature (see #01581)
                10h single-user
                    AL = operating system version ID (see #01580)
                14h multiuser
                    AL = operating system version ID (see #01579)
Notes:  as of Concurrent DOS/XM 5.0 (possibly earlier), the version is stored
          in the environment variable VER and the OS type in variable OS
        use this function if you are looking for multiuser capabilities,
          AX=4452h for single-user
        this function should never return the single-user values; however, an
          application should be prepared to accept single-user values, i.e. it
          should check the returned AH and AL separately
        CCI Multiuser DOS 7.22 returns AX=DX=1466h.
        DR DOS 3.31+ error returns with AX=0001h
SeeAlso: AX=4452h,AX=4459h

(Table 01579)
Values for Digital Research operating system version ID:
 32h    Concurrent PC DOS 3.2
 41h    Concurrent DOS 4.1
 50h    Concurrent DOS/XM 5.0 or Concurrent DOS/386 1.1
 60h    Concurrent DOS/XM 6.0 or Concurrent DOS/386 2.0
 62h    Concurrent DOS/XM 6.2 or Concurrent DOS/386 3.0
 66h    DR Multiuser DOS 5.1, CCT Multiuser DOS 7.x
 67h    Concurrent DOS 5.1
SeeAlso: #01581,#04023
--------O-214452-----------------------------
INT 21 - DR DOS 3.41+ - DETERMINE DOS TYPE/GET DR DOS VERSION
        AX = 4452h ("DR")
        CF set
Return: CF set if not DR DOS
            AX = error code (see #01680 at AH=59h/BX=0000h)
        CF clear if DR DOS
            AX = version code
            AH = single-user/multiuser nature (see #01581)
                10h single-user
                    AL = operating system version ID (see #01580)
                14h multiuser
                    AL = operating system version ID (see #01579)
            DX modified (refer to note below)
Notes:  the DR DOS version is stored in the environment variable VER
        use this function if looking for single-user capabilities, AX=4451h
          if looking for multiuser; this call should never return multiuser
          values
        in DR DOS 3.41-6.0, DX=AX on return; for Novell DOS 7, DH=AH but DL=00h
          (reportedly, DH=00h when booting NWDOS7 from installation disks)
        Novell DOS 7 returns error code 0001h if SETVER x.255 is in effect for
          the calling program, or SETVER /G x.255 is in effect
        although based on DR DOS, CCI Multiuser DOS 7.xx,
          IMS Multiuser DOS 7.x, and IMS REAL/32 7.50+ do not support this call
        With OpenDOS 7.02 BETA 2 and DR-OpenDOS 7.02+, the install check
          in most tools has been changed to run with both, AH=10h and AH=14h
          to support possible future "client side" multiuser releases of
          DR-DOS which may return 14h instead of 10h. Tools should also strip
          off the CP/Net bit before checking the kernel version.
          Often used version checks like >= 1070h are invalid, therefore,
          hence the AH and AL must be checked seperately.
        In DR DOS 3.31-6.0, and DR PalmDOS, DX=AX on return.
        For Novell DOS 7 - DR DOS 7.03 the DX value returned by this function
          is the same as the DX value returned by INT 21/AX=3306h, and
          represents the DOS revision (bits 7-0, currently always DL=00h) and
          the version flags (bits 15-8, with bit 12 = DOS in HMA, bit 11 = DOS
          in ROM) of the "patch_version" field in the PCM_HEADER structure in
          the IBMDOS.COM file. The version flags, however, are updated at
          runtime to reflect the actual system status, resulting, for example,
          in DH=10h for DOS in HMA, and DH=0 when booting Novell DOS 7 from
          installation disks.
        The operating system version ID represents the BDOS (Basic Disk
          Operating System) kernel version (of the DRBDOS.SYS aka IBMDOS.COM
          file), and the two nibbles can actually be read as CP/M version
          and sub-version, that is DR-DOS 7.03 (version code 1073h) is
          actually CP/M-86 version 7.3.
        Due to lost original patch sources from the backups, Caldera OpenDOS
          7.01 merely represented Novell DOS 7 Update 10 with minor changes.
        For Caldera DR-OpenDOS 7.02, all the Novell DOS 7 patches up to
          including Update 15.2 have been re-incorporated into the system
          from other sources, while the missing patches for IBMBIO.COM were
          independently re-implemented by Matthias Paul in 07/1997-10/1997.
        Novell DOS 7 (since 1993-11-08), OpenDOS 7.01 and OpenDOS 7.02 BETA
          IBMDOS.COM return error code 0001h if SETVER x.255 is in effect
          for the calling program, or SETVER /G x.255 is in effect.
        Since Caldera DR-OpenDOS 7.02 the SHARE 2.05+ (1998-01-05) driver
          uses INT 21/AX=65A2h as an *additional* BDOS version check: If the
          "FUCASE char" function is functional on a DR-DOS BDOS kernel 72h+,
          SHARE assumes it is running on an OpenDOS 7.02 BETA 1 (73h) kernel
          (or later) even if the BDOS version returned by INT 21/AX=4452h would
          indicate an earlier issue of the kernel. Some 3rd party drivers
          (like HPFS_DOS.EXE) were hardwired to detect Novell DOS 7 only,
          and so the kernel version had to be patched back on such systems
          to allow such drivers to work properly. The BDOS version and DOS
          revision are stored in PCM_HEADER of the IBMDOS.COM file, see
          table XXXX below.
        However, this is no longer necessary with the introduction of the
          DR-DOS 7.02+ IBMDOS.COM (since 1998-01-10) and SETVER 1.01+
          (since 1998-01-12) because they allow to fake BDOS versions as well
          as faking DOS versions. In /X mode, a set sub-version of y = 100..127
          will be used as BDOS version instead (64h..7Fh), while the DOS
          revision will be used to report the DOS sub-version instead. For
          example, given a DOS revision of 0, SETVER 6.114 would be the proper
          value to fake Novell DOS 7 (114=72h) on a DR-DOS 7.02+ system,
          reporting a DOS version of "IBM" 6.00. Sub-versions 128..255 will
          completely disable this BDOS version check, and report a DOS sub-
          version of 0..127. This is to work around problems with possibly
          hostile programs, that try to detect DR-DOS to not run on this OS.
        Additional version check for 1072h kernels: At API level, there
          is no known way to distinguish Novell DOS 7 from OpenDOS 7.01,
          however, an IBMBIO.COM/IBMDOS.COM file date since 1997 and the
          existance of an environment variable %OS%=OPENDOS usually indicates
          an OpenDOS system (but not the other way around; some OpenDOS
          BETAs still used %OS%=NWDOS!).
        Additional version checks for 1073h kernels: A functional test of
          INT 21/AX=65A2h (see above for SHARE) can be used to distinguish
          OpenDOS 7.02 BETA 1 from the later (OpenDOS 7.02 BETA 2+ and)
          DR-OpenDOS 7.02 and DR-DOS 7.02 releases, which are both the
          same, except for the name change and minor bug fixes. Testing
          for the INT 21/AX=6601h/BX=0000h bug can be used to differentiate the
          original release of DR-DOS 7.02 from later 7.02 updates and
          DR-DOS 7.03.
        The DR DOS version is stored in the environment variable VER.
SeeAlso: AX=4412h,AX=4451h,AX=4459h

(Table 01580)
Values for Digital Research operating system version ID:
 41h    DOS Plus 1.2
 60h    DOS Plus 2.0 or 2.1
 63h    DR DOS 3.41
 64h    DR DOS 3.42
 65h    DR DOS 5.0
 67h    DR DOS 6.0
 70h    PalmDOS
 71h    DR DOS 6.0 March 1993 "business update"
 72h    Novell DOS 7.0
 73h    OpenDOS 7.02
SeeAlso: #01579,#01581,#04088,#04023

(Table 04088)
Values for Digital Research operating system ID codes (full AX return value):
 ??41h  - DOS Plus 1.2
 1060h  - DOS Plus 2.0 or 2.1 (1988-03-09???)
          (this was basically a Concurrent CP/M-86 with built-in DOS emulation
           developed between 1983-1986)
 1060h  - DR DOS 3.31 (OEM) (1988-04-27???, 06/1988)
        - DR DOS 3.32 (OEM) (1988-08-17)
        - DR DOS 3.33 (OEM) (1988-09-01)
        - DR DOS 3.34 (OEM)
        - DR DOS 3.35 (OEM) (1988-10-21)
          for example: at least an issue for "Prism System 232" (10/1988)
          and a French issue existed.
 106?h  - DR DOS 3.40 BETA 1 (1988-11-11)
        - DR DOS 3.40
          (This is the first issue for sure known to be ROMmable, although
          DR DOS should have been ROMmable right from the start (1987).)
 1063h  - DR DOS 3.41 various OEM and retail releases (06/1989-07/1989)
        - DR DOS 3.41T
 1064h  - DR DOS 3.42
          (It is not clear, if this issue was actually released, since Lineo
          recently (fall 1999) released some historical kernels, and one of
          them is sailing under the name DR DOS 5.0 "Leopard" BETA 1 and has
          copyright strings from 1990, while it still identifies itself as
          being DR DOS 3.41... Unfortunately the uploaded archive is broken,
          hence it is not possible to find out easily. Maybe DR DOS 3.42 was
          nothing more than this early "Leopard" BETA???)
 1065h  - DR DOS ??? (1990-02-01)
        - DR DOS 5.0 "Leopard" BETA 2 (1990-03-16),
        - DR DOS 5.0 "Leopard" BETA 2B
          (This was the first issue to use the new IBMBIO.COM/IBMDOS.COM
          names instead of DRBIOS.SYS/DRBDOS.SYS.)
        - DR DOS 5.0 "Leopard" BETA 3
          (This issue still uses separate boot sectors for floppies and
          harddisks. The boot sector OEM label is still "DIGITAL".)
        - DR DOS 5.0 release (06/1990, 08/1990)
          (This issue has a combined boot sector for both, floppies and
          harddisks. The boot sector OEM label has changed to "IBM  3.3".
          DR DOS 5.0 was the first DOS to introduce high-load capabilities.)
 1066h  - DR DOS ??? (1990-08-09)
        - DR DOS 6.0 "Buxton" ALPHA (02/1991-03/1991)
 1067h  - DR DOS ??? (1991-03-15)
 106?h  - DR DOS 6.0 "Buxton" BETA 2 (??/1991)
 106?h  - DR DOS 6.0 "Buxton" BETA 3 (05/1991, 1991-06-27, 1991-06-28)
 1067h  - DR DOS 6.0 release (05/1991, 08/1991)
        - DR DOS 6.0 BDOS patch "PAT304" English (1991-11-29, XDIR /C: 9D8Ch)
          fix for "\\" in cmdline by Quick Basic 4.5/MS PDS 7.1 on Lantastic 4.0
        - DR DOS 6.0 BDOS patch "PAT303" English (1991-12-03, XDIR /C: 66B0h)
          This patch makes INT 21/AH=4Eh (Search First) compatible with MS-DOS
          when the Volume Label attribute is set. Previously under DR DOS the
          volume label was only searched for if bit 3 was the only bit set in
          the attribute whereas MS-DOS searches for the volume label if bit 3
          is set, irrespective of any other bits in the attribute.
        - DR DOS 6.0 BDOS patch "PAT306" English (1991-12-11, XDIR /C: 8DE5h)
          This patch fixes a problem with OPTASM which would give error 8
          if there were not enough handles available. This patch forces the
          system to check for available handles before it opens a file.
        - DR DOS 6.0 BDOS patch "PAT312" English (1992-01-07, XDIR /C: A0C6h)
          This patch fixes a problem with INT 21/AH=26h (create PSP) the
          segment from which the PSP was copied was incorrect so the the PSP
          was not filled correctly and did not contain the command tail.
        - DR DOS 6.0 BDOS patch "PAT314" English (1992-01-10, XDIR /C: C964h)
          This patch modifies INT 21/AX=33xxh, the Ctrl Break handler to
          support undocumented MS-DOS function INT 21/AX=3302h.
        - DR DOS 6.0 BIOS patch "PAT315" English (1992-01-10, XDIR /C: DBAAh)
          This patch fixes a problem where, when booting from a Bernoulli
          drive, any DEVICE= statement in CONFIG.SYS failed if it was placed
          after the device driver RCD.SYS v7.x and DOSOAD.SYS v1.x
        - DR DOS 6.0 BDOS patch "PAT317" English (1992-01-27, XDIR /C: B701h)
          This patch fixes a problem where attempting to close a changed file
          on a write protected disk seems to work after the first retry.
          This was caused because when the retry is attempted the file has been
          marked as not changed, so the attempt to write to disk is not made.
        - DR DOS 6.0 BDOS patch "PAT321" English (1992-02-19, XDIR /C: 947Bh)
          This patch fixes a problem where the error codes returned from
          INT 25h and INT 26h for "drive not ready" and "write protect"
          errors were incorrect.
        - DR DOS 6.0 BDOS patch "PAT323" English (1992-02-20, XDIR /C: E1B0h)
          This patch fixes a problem where, if the first command entered after
          booting the machine is a single character, any use of the command
          line recall keys will only recall the first command.
        - DR DOS 6.0 ??? German (1992-02-21)
        - DR DOS 6.0 update (02/1992)
        - DR DOS 6.0 BDOS patch "PAT326" English (1992-03-05, XDIR /C: 8EF2h)
          This is an optional patch to prevent DRDOS from padding the
          environment of EXEPACKed applications or utilities.
        - DR DOS 6.0 ??? German (1992-03-27)
        - DR DOS 6.0 BDOS patch "PAT334" English (1992-03-27, XDIR /C: 2AFEh)
          This patch fixes a problem where selecting (R)etry after hitting ^P
          while the printer is offline caused the system to hang.
        - DR DOS 6.0 "Windows 3.1 update, April 1992" "dr6win" (03/1992,
          1992-04-07). This public update only includes patches addressing
          full Windows 3.1 compatiblity. There should have been a full
          "business update" for registered users, shipping a little bit later.
        - DR DOS 6.0 ??? English (1992-04-07)
        - DR DOS 6.0 BDOS patch "pat354" English (1992-07-28, XDIR /C: 3398h)
          This patch for Beame and Whiteside Networks: On opening a duplicate
          file handle which describes a device, a device open call is made to
          inform the device driver that a new file handle has been opened.
          This patch is current and up to date as of 1992-11-10.
        - DR DOS 6.0 update (07/1992)
        - DR DOS 6.0 BDOS patch "PAT301" English (1992-10-28, XDIR /C: 959Bh)
          This patch fixes a problem with apps that opens a file, with Share
          loaded, but then cannot delete the file until the file is closed.
          This causes "Money Manager" to fail.
        - DR DOS 6.0 update (11/1992)
        Note: There is no known method to distinguish these different kernels
               at runtime, except for - maybe - checksumming the IBMBIO.COM/
               IBMDOS.COM files.
              The listed patches only address a single problem, while the
               listed updates are full rebuilds, also including all previous
               fixes.
 1070h  - multitasking "Vladivar" kernel architecture and introduction of
          DOS-like structures (1991-07-26, 08/1991)
        - DR DOS ??? (1991-09-26)
        - DR PalmDOS "Merlin" BETA 1-2
        - DR PalmDOS "Merlin" BETA 3 (1992-06-23)
        - DR PalmDOS "Merlin" Evaluation Release (1992-08-25)
        - DR PalmDOS Release Candidates 1-2 (1992-08-26)
        - DR PalmDOS/NetWare PalmDOS 1.0 (06/1992-11/1992)
          (DR PalmDOS was the first DR DOS re-incarnation that supported
          a genuine CDS, but still pretended to be DOS 3.31. Much of the
          former CP/M stuff was stripped out to reduce the code size to meet
          early Palmtop PC requirements. It introduced the patented dynamic
          idle detection method (by Roger Gross), a special issue of the
          TASKMAX program switcher (MINIMAX) to support Personal Information
          Modules (PIM) plug-in executables, supported Flash/ROM disk,
          XIP "Execute in Place" applications, and came with PCMCIA Card
          Services and Service Stack (SS) partially written by Ian Cullimore
          (ex-Poquet Computer Corporation founder).
        - DR DOS "Panther" BETA 1 (1992-10-16)
          (This issue already supported DPMS, had the "Vladivar" pre-emptive
          multitasker, DPMS-SuperStor disk compression, Multiuser-security
          (World/Group/Owner), and much improved multi-windowing ViewMAX/3 GUI
          which looked alot more like Windows than GEM. It pretended to be
          PC DOS 5. However, this issue was never released and later partially
          merged into the Novell DOS 7 project (without Multiuser-security and
          ViewMAX/3). In 1999, Caldera Thin Clients released ViewMAX/3 under
          GPL, see http://www.gemworld.com.)
 1070h??? - DR DOS "StarTrek" (STDOS) (07/1992-05/1993)
          (A never released, though reportedly fully functional result of
          a Novell and Apple team-up utilizing the DR DOS "Vladivar" multi-
          tasker to run Apple's Intel-port of MacOS 7.1 on an issue of DR DOS
          "Panther", even emulating HFS and Mac resources on DOS FAT file-
          systems.)
 1071h  - DR DOS ??? (1992-11-26)
        - DR DOS 6.0 "business update" "DRDOS493" English for
          Windows for Workgroups 3.1 (1993-03-19)
        - DR DOS 6.0 patch "PATDR6" English (1993-03-19) for NetWare
        - DR DOS 6.0 patch "PAT394" English (1993-09-17)
        - Apparently also some issues of DR DOS "Panther"???
 1072h  - Novell DOS 7/PNW "Smirnoff" BETA 1 (??/1993, probably 1993-04-26)
        - Novell DOS 7 "Panther" BETA 2 (08/1993)
          (This "Panther" issue is much different from the early 10/1992
          "Panther" BETA 1.)
        - Novell DOS 7 "Panther/Smirnoff" BETA 2 (08/1993)
        - Novell DOS 7 "Panther/Smirnoff" BETA 3 (09/1993)
          (This issue does not have workarounds for Windows 3.1 "AARD" code)
        - Novell DOS 7 "Panther/Smirnoff" BETA 4 (10/1993)
        - Novell DOS 7 code freeze 1993-12-03???)
        - Novell DOS 7 release (12/1993-04/1994)
          (also known as NWDOS or ND7, sometimes erroneously called NDOS)
        - Novell DOS 7 German release (1994-02-22)
          (This issue is known to have workarounds for Windows 3.1 "AARD" code)
        - Novell DOS 7 updates 1-3 (???/1994)
        - Novell DOS 7 update 4 (1994-05-31)
        - Novell DOS 7 updates 5-9 (???/1994)
        - Novell DOS 7 update 10 (12/1994)
        - Novell DOS 7 update 11 (01/1995)
        - Novell DOS 7 update 12 (04/1995)
        - Novell DOS 7 update 13 (05/1995)
        - Novell DOS 7 update 14 (08-09/1995)
        - Novell DOS 7 update 15 (12/1995)
        - Novell DOS 7 update 15.2 (01/1996)
          Note: The Novell DOS 7 updates 4-15.2 are full rebuilds, not patched
           binaries.
        - Caldera OpenDOS 7.01 BETA (02/1997)
          (basically representing Novell DOS 7 update 10 with minor
          changes here and there)
        - Caldera OpenDOS 7.01 (02-03/1997)
        - Caldera OpenDOS 7.01 M.R.S. (05/1997)
          (release of the kernel source code)
        - Matthias Paul's enhanced OpenDOS 7.02 ALPHA 1-4 kernels (07-11/1997)
 1073h  - Caldera OpenDOS 7.02 BETA 1 (09/1997)
          (incorporating most the Novell DOS 7 update 15.2 changes,
          except for IBMBIO.COM changes, plus a few other enhancements)
        - Caldera OpenDOS 7.02 BETA 2 (11/1997)
          (now also incorporating Matthias Paul's ALPHA 4 enhancements,
          the Novell DOS update 15.2 IBMBIO.COM patches have been
          re-implemented)
        - Caldera OpenDOS 7.02 BETA 2A (12/1997)
          (now incorporating all Novell DOS 7 update 15.2 changes)
        - Caldera DR-OpenDOS 7.02 (1997-12-2x)
          (now for the first time the product name is written with a hyphen!!!)
        - Caldera DR-DOS 7.02 internal build (02/1998)
        - Caldera DR-DOS 7.02 release (03/1998)
        - Caldera DR-DOS 7.02A release (06/1998)
        - DR-DOS 7.02 Update 1 (08/1998)
        - DR-DOS 7.02 Update 2 (09/1998)
        - Caldera (Thin Clients) DR-DOS 7.03 BETA (1998-12-03),
          sometimes referred to as "DR-DOS 7.03 BETA 3"
        - Caldera (Thin Clients) DR-DOS 7.03 release (1998-12-24, 1999-01-06)
        - Lineo DR-DOS 7.03 release (1999-06-07, 1999-09-09)
          (this issue has no functional changes compared to the 1999-01-06
          issue of DR-DOS 7.03)
SeeAlso: #01580,#01579,#01581,#04023

Bitfields for CP/M type indicator:
Bit(s)  Description     (Table 01581)
 7-4    CPU type
        0000 = 8080
        0001 = 8086
 3-0    OS type
        0000 = CP/M
        0001 = MP/M
        0002 = CP/Net
        0004 = multiuser
Notes:  Usually 10h (single-user) or 14h (multi-user).
        While earlier releases of the system utilities performed a test for
          a version code of (>)= (10)xxh, this was changed with
          DR-OpenDOS 7.02+ (now taking a possible multi-user version into
          account): Most utilities now test for AH being either 10h or 14h,
          and a BDOS version >=xxh to allow them to properly run on possible
          future multi-user releases of DR-DOS. Before doing this test, some
          of them strip off the CP/Net bit.
SeeAlso: #01580
--------O-214454-----------------------------
INT 21 U - DR DOS 3.41+ - SET GLOBAL PASSWORD
        AX = 4454h
        DS:DX -> password string (blank-padded to 8 characters)
Desc:   Specify the master password for accessing files.
SeeAlso: AX=4303h,AX=4414h
--------O-214456-----------------------------
INT 21 U - DR DOS 5.0+ - HISTORY BUFFER CONTROL
        AX = 4456h
        DL = control flags (see #01582)
Return: AL = previous value of state flags (see #01583)
Note:   DR DOS 6.0 only checks bit 0 and ignores the rest of DL
SeeAlso: #01584

Bitfields for control flags:
Bit(s)  Description     (Table 01582)
 0      whose buffer: 0=application, 1=COMMAND.COM
---Novell DOS 7---
 1      toggle HISTORY usage
 2      toggle INSERT state
Note:   only one bit at a time may be used

Bitfields for state flags:
Bit(s)  Description     (Table 01583)
 0      HISTORY buffer enabled
 1      INSERT enabled
 2-5    unused
 7      whose buffer: 0=application, 1=COMMAND.COM
--------O-214457-----------------------------
INT 21 U - DR DOS 5.0-6.0 - SHARE/HILOAD CONTROL
        AX = 4457h
        DH = subfunction
            00h enable/disable SHARE
                DL = 00h disable
                   = 01h enable
                   else Return: AX = ???
            01h get HILOAD status
                Return: AX = status
                            0000h off
                            0001h on
            02h set HILOAD status
                DL = new state (00h off, 01h on)
                Return: AX = ???
            other
                Return: AX = ???
Note:   This was seen called by COMMAND.COM of DR DOS 6.0; it does not seem
          to be supported by Novell DOS 7
SeeAlso: AX=4457h/DX=FFFFh
--------O-214457DXFFFF-----------------------
INT 21 OU - DR DOS 6.0 - GET SHARE STATUS
        AX = 4457h
        DX = FFFFh
Return: AX = SHARE status
Note:   not supported by Novell DOS 7
SeeAlso: INT 2F/AX=1000h
--------O-214458-----------------------------
INT 21 U - DR DOS 5.0+ internal - GET POINTER TO INTERNAL VARIABLE TABLE
        AX = 4458h
Return: ES:BX -> internal variable table (see #01584,#01585)
        AX = ??? (0B50h for DR DOS 5.0, 0A56h for DR DOS 6.0, 0FE4h for
                  Novell DOS 7)
SeeAlso: AX=4452h

Format of DR DOS 5.0-6.0 internal variable table:
Offset  Size    Description     (Table 01584)
 00h    WORD    ???
 02h    WORD    segment of ???
 04h    WORD    offset within DOS data segment of history control structure
                  for COMMAND.COM history buffer (see #01586)
 06h    WORD    offset within DOS data segment of history control structure
                  for application history buffer (see #01586)
 08h    BYTE    initial history state flags (see #01583)
 09h  2 BYTEs   ???
 0Bh    WORD    KB of extended memory at startup
 0Dh    BYTE    number of far jump entry points
 0Eh    WORD    segment containing far jumps to DR DOS entry points (see #01587)
 10h    WORD    (only if kernel loaded in HMA) offset in HMA of first free HMA
                  memory block (see #01588) or 0000h if none; segment is FFFFh
 12h    WORD    pointer to segment of environment variables set in CONFIG,
                  or 0000h if already used
---DR DOS 6.0---
 14h    WORD    (only if kernel loaded in HMA) offset in HMA of first used HMA
                memory block (see #01588) or 0000h if none; segment is FFFFh
 16h  8 BYTEs   ???
 1Eh    WORD    offset in DOS data segment of full COUNTRY.SYS filename
 20h  8 BYTEs   ???
 28h    WORD    offset in DOS data segment of SHARE hook table
 2Ah  2 BYTEs   ???
 2Ch    WORD    offset in DOS data segment of far pointer to INT 2F/AX=1000h
                  handler
Notes:  the segment used for the DR DOS 6.0 CONFIG environment variables
          (excluding COMSPEC, VER and OS) is only useful for programs/drivers
          called from CONFIG.SYS. The word is set to zero later when the area
          is copied to the COMMAND.COM environment space.  This allows
          CONFIG.SYS to pass information to AUTOEXEC.BAT.
        the Novell DOS 7 KEYB uses offsets 10h,14h, and 2Ch in the same was
          as for DR DOS 6.0, so it is likely that the entire table is the same

Format of Novell DOS 7 internal variable table:
Offset  Size    Description     (Table 01585)
 00h    ???
 1Eh    WORD    offset of COUNTRY.SYS filename
 42h 16 DWORDs  pointers to ??? entry points
        ???

Format of history control structure:
Offset  Size    Description     (Table 01586)
 00h    WORD    segment of buffer
 02h    WORD    size of buffer in bytes
 04h    WORD    ???

Format of kernel entry jump table for DR DOS 5.0-6.0:
Offset  Size    Description     (Table 01587)
 00h  5 BYTEs   far jump to kernel entry point for CP/M CALL 5
 05h  5 BYTEs   far jump to kernel entry point for INT 20
 0Ah  5 BYTEs   far jump to kernel entry point for INT 21
 0Fh  5 BYTEs   far jump to kernel entry point for INT 22 (RETF)
 14h  5 BYTEs   far jump to kernel entry point for INT 23 (RETF)
 19h  5 BYTEs   far jump to kernel entry point for INT 24
 1Eh  5 BYTEs   far jump to kernel entry point for INT 25
 23h  5 BYTEs   far jump to kernel entry point for INT 26
 28h  5 BYTEs   far jump to kernel entry point for INT 27
 2Dh  5 BYTEs   far jump to kernel entry point for INT 28
 32h  5 BYTEs   far jump to kernel entry point for INT 2A (IRET)
 37h  5 BYTEs   far jump to kernel entry point for INT 2B (IRET)
 3Ch  5 BYTEs   far jump to kernel entry point for INT 2C (IRET)
 41h  5 BYTEs   far jump to kernel entry point for INT 2D (IRET)
 46h  5 BYTEs   far jump to kernel entry point for INT 2E (IRET)
 4Bh  5 BYTEs   far jump to kernel entry point for INT 2F
Notes:  all of these entry points are indirected through this jump table
          to allow the kernel to be relocated into high memory while leaving
          the actual entry addresses in low memory for maximum compatibility
        some of these entry points (22h,23h,24h,2Eh,2Fh) are replaced as soon
          as COMMAND.COM is loaded, and return immediately to the caller, some
          returning an error code (the original handler for INT 2F returns
          AL=03h [fail]).

Format of HMA Memory Block (DR DOS 6.0 kernel loaded in HMA):
Offset  Size    Description     (Table 01588)
 00h    WORD    offset of next HMA Memory Block (0000h if last block)
 02h    WORD    size of this block in bytes (at least 10h)
 04h    BYTE    type of HMA Memory Block (interpreted by MEM)
                00h system
                01h KEYB
                02h NLSFUNC
                03h SHARE
                04h TaskMAX
                05h COMMAND
 05h    var     TSR (or system) code and data. DR DOS TSR's, such as KEYB,
                  hooks interrupts using segment FFFEh instead FFFFh.
--------O-214459-----------------------------
INT 21 - DR Multiuser DOS 5.0 - API
        AX = 4459h
        CL = function (see #04019 at INT E0"CP/M-86")
        DS,DX = parameters
Notes:  DR DOS 5.0 and Novell DOS 7 return CF set and AX=0001h
        this API is also available on INT E0
SeeAlso: AX=4452h,INT E0"CP/M-86"
--------O-21445A-----------------------------
INT 21 U - Concurrent DOS etc. - USER GROUP SUPPORT
        AX = 445Ah
        CX = operation type
            00h get default file access rights
            01h set default file access rights
            02h get user/group
            03h get security version
            04h set security key (needs key)
            05h set user/group (needs key)
        DX = data for operation if set
        BX = key (if required)
Return: AX = result
Note:   This function was introduced on 1990/06/04 for CDOS.  It has been
          supported by DR PalmDOS and DR DOS "Panther" (BDOS 1070h),
          but it is not supported by Novell DOS 7 - DR-DOS 7.03.
--------N-2144E0-----------------------------
INT 21 U - Sun PC-NFS - API???
        AX = 44E0h
        DS:DX -> ???
        SS:BP -> stack frame (see #01589)
Return: ???
Note:   this function is also supported by Beame&Whiteside's BWPCNFS shim; the
          description presented here was derived from that shim

Format of PC-NFS stack frame:
Offset  Size    Description     (Table 01589)
 00h    WORD    -> previous stack frame
 02h    DWORD   return address
--------d-2144FFBXFFFF-----------------------
INT 21 U - NewSpace - ???
        AX = 44FFh
        BX = FFFFh
        DX = ???
Program: NewSpace is a TSR by Isogon Corporation which automatically compresses
          all files as they are written and decompresses them as they are read
SeeAlso: AX=4414h/BX=FFFFh
--------U-214B-------------------------------
INT 21 - ELRES v1.0 only - INSTALLATION CHECK
        AH = 4Bh
        DS:DX = 0000h:0000h
Return: ES:BX -> ELRES history structure (see #01381 at AH=2Bh/CX=454Ch)
        DX = DABEh (signature, DAve BEnnett)
Program: ELRES is an MS-DOS return code (errorlevel) recorder by David H.
          Bennett
SeeAlso: AH=2Bh/CX=454Ch
----------214B18DX0010-----------------------
INT 21 U - FBOOT v2.13 - PERFORM FAST BOOTSTRAP
        AX = 4B18h
        DX = 0010h
        BX = disk selector
            0000h boot from floppy
            0080h boot from hard disk
Return: never if FastBoot installed
Program: CyberWare FastBoot allows fast warm boots by skipping CMOS
          checking, ROM scan, RAM & peripheral components test
InstallCheck:   search for a character device driver called "FBOOT$$$"
--------O-214B80-----------------------------
INT 21 - DR DOS v3.31+ - RUN ALREADY-LOADED KERNEL FILE
        AX = 4B80h
        DS:DX -> ASCIZ name of program to EXEC
        ES = segment of PSP for kernel file
Return: only if call failed
Note:   DR DOS uses this call after an AX=4B01h to load the kernel file into
          memory and patch the program's parent-PSP field to point at itself
SeeAlso: AH=4Bh"EXEC"
--------k-214BF0-----------------------------
INT 21 - DIET v1.10+ (Overlay Mode) - INSTALLATION CHECK
        AX = 4BF0h
Return: CF clear if installed
            AX = 899Dh
Program: DIET is an executable-compression program by Teddy Matsumoto
SeeAlso: AX=37D0h,AX=4BF1h
--------k-214BF1-----------------------------
INT 21 - DIET v1.10+ (Overlay Mode) - EXPAND PROGRAM???
        AX = 4BF1h
Return: ???
SeeAlso: AX=37D0h,AX=4BF0h
--------m-214C57-----------------------------
INT 21 - Headroom - ???
        AX = 4C57h
        DS:DX -> target address
Note:   jumps to target address instead of terminating program
SeeAlso: AX=5758h
--------f-214E-------------------------------
INT 21 - WILDUNIX.COM internal - INSTALLATION CHECK
        AH = 4Eh
        DS:DX = 0000h:0000h
Return: AH = 99h if installed
Program: WILDUNIX.COM is a resident Unix-style wildcard expander by Steve
          Hosgood and Terry Barnaby
--------O-215702-----------------------------
INT 21 - OS/2 v1.1+ Family API - DosQFileInfo
        AX = 5702h
        BX = file handle
        CX = size of buffer for information
        DX = level of information
            0001h standard file information (see #01672)
            0002h Query EA Size (see #01672)
            0003h Query EAs from List (see #01673)
            0004h Query All EAs (see #01673)
        ES:DI -> buffer for information (see #01672,#01673)
Return: CF clear if successful
        CF set on error
            AX = error code
SeeAlso: AX=5702h/BX=FFFFh,AX=5703h"OS/2",AH=6Dh"OS/2"

Format of OS/2 DosQFileInfo:
Offset  Size    Description     (Table 01672)
 00h    WORD    creation date
 02h    WORD    creation time
 04h    WORD    last access date
 06h    WORD    last access time
 08h    WORD    last write date
 0Ah    WORD    last write time
 0Ch    DWORD   file size in bytes
 10h    DWORD   allocated space in bytes
 14h    WORD    file attributes
---level 2 only---
 16h    DWORD   size of Extended Attributes in byte
SeeAlso: #01673,#01676

Format of OS/2 DosQFileInfo, EAOP structure:
Offset  Size    Description     (Table 01673)
 00h    DWORD   pointer to general EA list (see #01674)
 04h    DWORD   pointer to buffer for full EA list, with length field set
 08h    DWORD   (ret) error
Note:   for info level 3, the first pointer must contain the address of a
          list of the Extended Attributes to be retrieved; for info level 4,
          it should be 0000h:0000h
SeeAlso: #01672,#01676

Format of OS/2 DosQFileInfo, General EA List:
Offset  Size    Description     (Table 01674)
 00h    DWORD   (call) total size of list in bytes (including this field)
                (ret) number of bytes actually used (including this field)
 04h    var     Extended Attribute entries (see #01675) [packed arrray]

Format of OS/2 DosQFileInfo, General EA entry:
Offset  Size    Description     (Table 01675)
 00h    BYTE    length of Extended Attribute name (excluding terminating NUL)
 01h  N BYTEs   EA name
        BYTE    00h
SeeAlso: #01674

Format of OS/2 DosQFileInfo, Full EA List:
Offset  Size    Description     (Table 01676)
 00h    DWORD   (call) total size of list in bytes (including this field)
                (ret) number of bytes actually used (including this field)
 04h    var     Extended Attribute data (see #01677) [packed array]
SeeAlso: #01672,#01673

Format of OS/2 Extended Attribute data (struct FEA):
Offset  Size    Description     (Table 01677)
 00h    BYTE    flags
                bit 7: critical EA
 01h    BYTE    length of Extended Attribute name (excluding terminating NUL)
 02h    WORD    length of Extended Attribute value
 04h  N BYTEs   EA name
        BYTE    00h
      M BYTEs   EA value
--------O-215702BXFFFF-----------------------
INT 21 - OS/2 v1.1+ Compatibility Box Family API - DosQPathInfo
        AX = 5702h
        BX = FFFFh
        CX = size of buffer for information
        DX = level of information (0002h)
        DS:SI -> filename
        ES:DI -> buffer for FAPI path information (see #01678)
Return: CF clear if successful
            AL = 00h
        CF set on error
            AX = error code
SeeAlso: AX=5702h"OS/2",AX=5703h/BX=FFFFh

Format of FAPI path information:
Offset  Size    Description     (Table 01678)
 00h 22 BYTEs   ???
 16h    DWORD   extended attribute size (none present if less than 5)
--------O-215703-----------------------------
INT 21 - OS/2 v1.1+ Family API - DosSetFileInfo
        AX = 5703h
        BX = file handle
        CX = size of information buffer
        DX = level of information
        ES:DI -> information buffer
Return: CF clear if successful
        CF set on error
            AX = error code
SeeAlso: AX=5702h"OS/2",AX=5703h/BX=FFFFh
--------O-215703BXFFFF-----------------------
INT 21 - OS/2 v1.1+ Family API - DosSetPathInfo
        AX = 5703h
        BX = FFFFh
        CX = size of information buffer
        DX = level of information
        DS:SI -> filename
        ES:DI -> information buffer
Return: CF clear if successful
        CF set on error
            AX = error code
SeeAlso: AX=5702h/BX=FFFFh,AX=5703h"OS/2"
--------U-215757BX5757-----------------------
INT 21 U - IBM Genie - Resident Manager - INSTALLATION CHECK
        AX = 5757h
        BX = 5757h
Return: AX = 0000h if installed
            BX = ???
            DX = ???
            DS:SI -> list of 27 DWORD entry point addresses
Program: IBM Genie is a set of utility TSRs by Helix Software
Note:   other functions possible if BX <> 5757h, but details not yet available
----------215758-----------------------------
INT 21 U - Headroom - API
        AX = 5758h
        BL = function
            00h ???
            01h get Headroom location
                Return: CF clear if installed
                            AX = PSP segment of Headroom TSR
                            BX = paragraphs of memory used by Headroom
                        CF set if not (normal DOS return)
                Note:   this function is also used as an installation check
            02h get INT 21 handler
                Return: CF clear
                        ES:BX -> Headroom's INT 21 handler
                Note: also sets unknown flag
            03h launch application???
                DS:SI -> 233-byte application record
                Return: ???
            04h ???
                ???
                Return: CF clear
            05h get swap directory
                Return: CF clear
                        DX:AX -> ASCIZ swap directory name
            06h ???
                DX = ???
                Return: CF clear
            07h ???
            08h ???
            09h get current application
                Return: BX = application number
            0Ah ???
                DX = application number
                DS:SI = ???
                Return: ???
            0Bh ???
            0Ch ???
                DX = application number
                ???
                Return: ???
            0Dh ???
                DX = application number
                ???
                Return: ???
            0Eh get ???
                Return: CF clear
                        AX = ???
            0Fh set ??? flag
            10h clear ??? flag
            11h find application by name
                DS:SI -> ASCIZ application name
                Return: CF clear
                        AX = application number or FFFFh if not loaded
            12h ???
                DX = application number
                Return: CF clear
                        ???
            13h ???
                Return: CF clear
            14h ???
                same as function 13h
            15h set ???
                DX = ???
            16h get ???
                Return: AX = ??? set by function 15h
            17h get ???
                Return: BX = ???
                        CX = ??? (may be pointer in BX:CX)
            18h BUG: branches incorrectly due to fencepost error
Program: Headroom is a TSR/task switcher by Helix Software
SeeAlso: AX=4C57h,AX=5757h,INT 2F/AX=5758h
--------!---Section--------------------------
