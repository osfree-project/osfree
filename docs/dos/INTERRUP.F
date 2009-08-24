Copyright (c) 1989-1999,2000 Ralf Brown
--------G-20---------------------------------
INT 20 - COMTROL HOSTESS i/ISA DEBUGGER - INVOKE FIRMWARE DEBUGGER
        ???
Return: ???
SeeAlso: INT 21"COMTROL"
---Frontier Technologies' FTCVIP---
 09h    ???
        returns EAX=1
SeeAlso: #01318,#01320
----------20----Vx3048-----------------------
INT 20 P - Frontier Technologies ??? - VxD SERVICES
        VxD = 3048h
Note:   the desired VxD and service number are identified by the data
          immediately following the INT 20 instruction, as in:
                INT     20h
                DW      service number
                DW      VxD identifier
SeeAlso: INT 2F/AX=1684h"DEVICE API",INT 30"Windows",#01265,#01335

(Table 01335)
Values for ??? (VxD ID 3048h) service number:
 00h    get version???
 03h    ???
        called with 5 DWORDs on stack, returns EAX=???, ECX nonzero on error
 04h    ???
        called with DWORD on stack, returns EAX=???, ECX nonzero on error
 06h    ???
        called with DWORD on stack, returns EAX=???, ECX nonzero on error
 07h    ???
        called with 3 DWORDs on stack, returns EAX=???, ECX nonzero on error
 0Bh    ???
        called with 2 DWORDs on stack, returns EAX=???, ECX nonzero on error
 0Ch    ???
        called with 2 DWORDs on stack, returns EAX=???, ECX nonzero on error
 0Dh    ???
        called with 6 DWORDs on stack, returns EAX=???, ECX nonzero on error
 1Ah    ???
        called with 3 DWORDs on stack
SeeAlso: #01336
----------20----Vx3049-----------------------
INT 20 P - Frontier Technologies ??? - VxD SERVICES
        VxD = 3049h
Note:   the desired VxD and service number are identified by the data
          immediately following the INT 20 instruction, as in:
                INT     20h
                DW      service number
                DW      VxD identifier
SeeAlso: INT 2F/AX=1684h"DEVICE API",INT 30"Windows",#01265,#01336

(Table 01336)
Values for ??? (VxD ID 3049h) service number:
 00h    get version???
 1Dh    ???
        called with 2 DWORDs on stack; returns EAX=???, ECX nonzero on error
 1Eh    ???
        called with 6 DWORDs on stack; returns EAX=???, ECX nonzero on error
SeeAlso: #01335
--------G-21---------------------------------
INT 21 - COMTROL HOSTESS i/ISA DEBUGGER - GET SEGMENT FOR CONTROL PROGRAM USE
        ???
Return: AX = first segment available for control program use
SeeAlso: INT 20"COMTROL",INT 22"COMTROL"
--------K-210A00-----------------------------
INT 21 - WCED v1.6+ - INSTALLATION CHECK
        AX = 0A00h
        DS:DX -> 6-byte buffer whose first two bytes must be 00h
Return: buffer offset 02h-05h filled with "Wced" if installed
Program: WCED is a free command-line editor and history utility by Stuart
          Russell
SeeAlso: AH=FFh"CED"
--------v-210E--DLAD-------------------------
INT 21 U - Novell DOS 7 - SDRes v27.03 - INSTALLATION CHECK
        AH = 0Eh
        DL = ADh
Return: AL = BAh if installed
Program: SDRes is the resident portion of the Search&Destroy antiviral by
          Fifth Generation Systems, as bundled with Novell DOS 7
Note:   SDRes will terminate programs which test for the presence of viruses
          using interrupt-based installation calls, saying that the program
          may be infected
SeeAlso: AH=0Eh/DL=AEh,AH=0Eh/DL=AFh,AH=4Ah/BX=00B6h,INT 13/AX=A759h
--------v-210E--DLAE-------------------------
INT 21 U - Novell DOS 7 - SDRes v27.03 - CLEAR ??? FLAG
        AH = 0Eh
        DL = AEh
SeeAlso: AH=0Eh/DL=ADh,AH=0Eh/DL=AFh,INT 13/AX=A759h
--------v-210E--DLAF-------------------------
INT 21 U - Novell DOS 7 - SDRes v27.03 - SET ??? FLAG
        AH = 0Eh
        DL = AFh
SeeAlso: AH=0Eh/DL=ADh,AH=0Eh/DL=AEh,INT 13/AX=A759h
--------E-212501-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - RESET DOS EXTENDER DATA STRUCTURES
        AX = 2501h
        SS = application's original SS or DS (FlashTek X-32VM)
Return: CF clear if successful
        CF set on error
            caller is operating on X-32 stack (FlashTek X-32VM)
Notes:  Phar Lap uses INT 21/AH=25h as the entry point for all 386/DOS-Extender
          system calls.  Only available when directly using 386/DOS-Extender or
          a compatible DOS extender, or when using a product that was created
          using 386-DOS/Extender or a compatible
        this function is also supported by FlashTek X-32VM
SeeAlso: AH=30h"Phar Lap"
--------E-212502-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET PROTECTED-MODE INTERRUPT VECTOR
        AX = 2502h
        CL = interrupt number
Return: CF clear
        ES:EBX = CS:EIP of protected-mode interrupt handler
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2503h,AX=2504h,INT 31/AX=0204h
--------E-212503-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET REAL-MODE INTERRUPT VECTOR
        AX = 2503h
        CL = interrupt number
Return: CF clear
        EBX = CS:IP of real-mode interrupt handler
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2502h,AX=2504h,AH=35h,INT 31/AX=0200h
--------E-212504-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - SET PROTECTED-MODE INTERRUPT VECTOR
        AX = 2504h
        CL = interrupt number
        DS:EDX = CS:EIP of protected-mode interrupt handler
Return: CF clear
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2502h,AX=2505h,INT 31/AX=0205h
--------E-212505-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - SET REAL-MODE INTERRUPT VECTOR
        AX = 2505h
        CL = interrupt number
        EBX = CS:IP of real-mode interrupt handler
Return: CF clear
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2503h,AX=2504h,INT 31/AX=0201h
--------E-212506-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - SET INT TO ALWAYS GAIN CNTRL IN PR. MODE
        AX = 2506h
        CL = interrupt number
        DS:EDX = CS:EIP of protected-mode interrupt handler
Return: CF clear
Notes:  this function modifies both the real-mode low-memory interrupt
          vector table and the protected-mode Interrupt Descriptor Table (IDT)
        interrupts occurring in real mode are resignaled in protected mode
        this function is also supported by FlashTek X-32VM
--------E-212507-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - SET REAL- & PROTECTED-MODE INT VECTORS
        AX = 2507h
        CL = interrupt number
        DS:EDX = CS:EIP of protected-mode interrupt handler
        EBX = CS:IP of real-mode interrupt handler
Return: CF clear
Notes:  interrupts are disabled until both vectors have been modified
        this function is also supported by FlashTek X-32VM
SeeAlso: AX=2504h,AX=2505h
--------E-212508-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET SEGMENT LINEAR BASE ADDRESS
        AX = 2508h
        BX = segment selector
Return: CF clear if successful
            ECX = linear base address of segment
        CF set if invalid segment selector
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2509h
--------E-212509-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - CONVERT LINEAR TO PHYSICAL ADDRESS
        AX = 2509h
        EBX = linear address to convert
Return: CF clear if successful
            ECX = physical address (carry flag clear)
        CF set if linear address not mapped in page tables
SeeAlso: AX=2508h
--------E-212509-----------------------------
INT 21 P - FlashTek X-32VM - GET SYSTEM SEGMENTS AND SELECTORS
        AX = 2509h
Return: CF clear
        EAX high word = default DS
        AX = alias for 16-bit data segment
        BX = real mode code segment
        EDX high word = selector covering full 4GB address space
        DX = default SS
        ESI high word = PSP selector
        SI = environment selector
--------E-21250A-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - MAP PHYSICAL MEMORY AT END OF SEGMENT
        AX = 250Ah
        ES = segment selector in the Local Descriptor Table (LDT) of segment
             to modify
        EBX = physical base address of memory to map (multiple of 4K)
        ECX = number of physical 4K pages to map
Return: CF clear if successful
            EAX = 32-bit offset in segment of mapped memory
        CF set on error
            EAX = error code
                08h insufficient memory to create page tables
                09h invalid segment selector
SeeAlso: INT 31/AX=0800h
--------E-21250C-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET HARDWARE INTERRUPT VECTORS
        AX = 250Ch
Return: CF clear
        AL = base interrupt vector for IRQ0-IRQ7
        AH = base interrupt vector for IRQ8-IRQ15
        BL = interrupt vector for BIOS print screen function (Phar Lap only)
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: INT 31/AX=0400h,INT 67/AX=DE0Ah
--------E-21250D-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET REAL-MODE LINK INFORMATION
        AX = 250Dh
Return: CF clear
        EAX = CS:IP of real-mode callback procedure (see #01358) that will
                  call through from real mode to a protected-mode routine
        EBX = 32-bit real-mode address of intermode call data buffer
        ECX = size in bytes of intermode call data buffer
        ES:EDX = protected-mode address of intermode call data buffer
Notes:  this function is also supported by FlashTek X-32VM
        X-32VM guarantees the intermode buffer to be at least 4 KB
SeeAlso: AX=250Eh

(Table 01358)
Call Phar Lap real-mode callback with:
        STACK:  DWORD   offset to protected-mode code
                WORD    placeholder for protected-mode CS
                DWORD   pointer to selector structure (see #01359)
                        or 0000h:0000h for defaults
                var     parameters for protected-mode procedure
Return: via FAR return

Format of Phar Lap selector structure:
Offset  Size    Description     (Table 01359)
 00h    WORD    protected-mode GS selector
 02h    WORD    protected-mode FS selector
 04h    WORD    protected-mode ES selector
 06h    WORD    protected-mode DS selector
--------E-21250E-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - CALL REAL-MODE PROCEDURE
        AX = 250Eh
        EBX = CS:IP of real-mode procedure to call
        ECX = number of two-byte words to copy from protected-mode stack
              to real-mode stack
Return: CF clear if successful
            all segment registers unchanged
            all general registers contain values set by real-mode procedure
            all other flags set as they were left by real-mode procedure
            stack unchanged
        CF set on error
            EAX = error code
                01h not enough real-mode stack space
Note:   this function is also supported by FlashTek X-32VM; under X-32VM, the
          call will fail if ECX > 0000003Fh
SeeAlso: AX=250Dh,AX=2510h,AH=E1h"OS/286",INT 31/AX=0301h
--------E-21250F-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - CONVERT PROTECTED-MODE ADDRESS TO MS-DOS
        AX = 250Fh
        ES:EBX = 48-bit protected-mode address to convert
        ECX = 00000000h or length of data in bytes
Return: CF clear if successful (address < 1MB and contiguous)
            ECX = 32-bit real-mode MS-DOS address
        CF set on error (address >= 1MB or not contiguous)
            ECX = linear address
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2510h
--------E-212510-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - CALL REAL-MODE PROCEDURE, REGISTERS
        AX = 2510h
        EBX = CS:IP of real-mode procedure to call
        ECX = number of two-byte words to copy to protected-mode stack to
              real-mode stack
        DS:EDX -> pointer to parameter block (see #01360)
Return: CF clear if successful
            all segment registers unchanged,
            EDX unchanged
            all other general registers contain values set by real-mode proc
            all other flags are set as they were left by real-mode procedure
            real-mode register values are returned in the parameter block
        CF set on error
            EAX = error code
                01h not enough real-mode stack space
Note:   unlike most of the preceding 25xxh functions, this one is not
          supported by FlashTek X-32VM
SeeAlso: AX=250Eh,AX=250Fh

Format of Phar Lap real-mode call parameter block:
Offset  Size    Description     (Table 01360)
 00h    WORD    real-mode DS value
 02h    WORD    real-mode ES value
 04h    WORD    real-mode FS value
 06h    WORD    real-mode GS value
 08h    DWORD   real-mode EAX value
 0Ch    DWORD   real-mode EBX value
 10h    DWORD   real-mode ECX value
 14h    DWORD   real-mode EDX value
--------E-212511-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - ISSUE REAL-MODE INTERRUPT
        AX = 2511h
        DS:EDX -> parameter block (see #01361)
Return: all segment registers unchanged
        EDX unchanged
        all other registers contain values set by the real-mode int handler
        the flags are set as they were left by the real-mode interrupt handler
        real-mode register values are returned in the parameter block
Note:   this function is also supported by FlashTek X-32VM
SeeAlso: AX=2503h,AX=2505h,AX=250Eh,AH=E3h"OS/286",INT 31/AX=0300h

Format of Phar Lap real-mode interrupt parameter block:
Offset  Size    Description     (Table 01361)
 00h    WORD    interrupt number
 02h    WORD    real-mode DS value
 04h    WORD    real-mode ES value
 06h    WORD    real-mode FS value
 08h    WORD    real-mode GS value
 0Ah    DWORD   real-mode EAX value
 0Eh    DWORD   real-mode EDX value
Note: all other real-mode values set from protected-mode registers
--------E-212512-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - LOAD PROGRAM FOR DEBUGGING
        AX = 2512h
        DS:EDX -> pointer to ASCIZ program name
        ES:EBX -> pointer to parameter block (see #01363)
        ECX = size in bytes of LDT buffer
Return: CF clear if successful
            EAX = number of segment descriptors in LDT
        CF set on error
            EAX = error code (see #01362)
SeeAlso: AX=2517h

(Table 01362)
Values for Phar Lap error code:
 02h    file not found or path invalid
 05h    access denied
 08h    insufficient memory
 0Ah    environment invalid
 0Bh    invalid file format
 80h    LDT too small

Format of Phar Lap program load parameter block:
Offset  Size    Description     (Table 01363)
Input:
 00h    DWORD   32-bit offset of environment string
 04h    WORD    segment of environment string
 06h    DWORD   32-bit offset of command-tail string
 0Ah    WORD    segment of command-tail string
 0Ch    DWORD   32-bit offset of LDT buffer (size in ECX)
 10h    WORD    segment of LDT buffer
Output:
 12h    WORD    real-mode paragraph address of PSP (see also AH=26h)
 14h    WORD    real/protected mode flag
                0000h  real mode
                0001h  protected mode
 16h    DWORD   initial EIP value
 1Ah    WORD    initial CS value
 1Ch    DWORD   initial ESP value
 20h    WORD    initial SS value
 22h    WORD    initial DS value
 24h    WORD    initial ES value
 26h    WORD    initial FS value
 28h    WORD    initial GS value
--------E-212513-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - ALIAS SEGMENT DESCRIPTOR
        AX = 2513h
        BX = segment selector of descriptor in GDT or LDT
        CL = access-rights byte for alias descriptor
        CH = use-type bit (USE16 or USE32) for alias descriptor
Return: CF clear if successful
            AX = segment selector for created alias
        CF set on error
            EAX = error code
                08h insufficient memory (can't grow LDT)
                09h invalid segment selector in BX
--------E-212514-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - CHANGE SEGMENT ATTRIBUTES
        AX = 2514h
        BX = segment selector of descriptor in GDT or LDT
        CL = new access-rights byte
        CH = new use-type bit (USE16 or USE32)
Return: CF clear if successful
        CF set on error
            EAX = error code
                09h invalid selector in BX
SeeAlso: AX=2515h,INT 31/AX=0009h
--------E-212515-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET SEGMENT ATTRIBUTES
        AX = 2515h
        BX = segment selector of descriptor in GDT or LDT
Return: CF clear if successful
            CL = access-rights byte for segment
            CH = use-type bit (USE16 or USE32)
        ECX<16-31> destroyed
        CF set on error
            EAX = error code
                09h invalid segment selector in BX
SeeAlso: AX=2514h
--------E-212516-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v2.2+ - FREE ALL MEMORY OWNED BY LDT
        AX = 2516h
Return: CF clear
Note:   this function must be called from Ring 0 or the CS descriptor is freed
--------E-212517-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v2.1c+ - GET INFO ON DOS DATA BUFFER
        AX = 2517h
Return: CF clear
        ES:EBX -> data buffer (protected mode address)
        ECX -> data buffer (real mode address)
        EDX = size of data buffer in bytes
Note:   the data buffer's address changes after calls to AX=2512h and AX=252Ah
SeeAlso: AX=2512h,AX=252Ah,AX=2530h
--------E-212518-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender 2.1c+ - SPECIFY HANDLER FOR MOVED SEGMENTS
        AX = 2518h
        ES:EBX -> function to call when a segment is moved
Return: CF clear
        ES:EBX -> previous handler
--------E-212519-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - GET ADDITIONAL MEMORY ERROR INFO
        AX = 2519h
Return: CF clear
        EAX = error code
            0000h  no error
            0001h  out of physical memory
            0002h  out of swap space (unable to grow swap file)
            0003h  out of LDT entries and unable to grow LDT
            0004h  unable to change extended memory allocation mark
            FFFFFFFFh   paging disabled
Note:   VMM is the Virtual Memory Manager option
--------E-21251A-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - LOCK PAGES IN MEMORY
        AX = 251Ah
        EDX = number of 4k pages to lock
        if BL = 00h
            ECX = linear address of first page to lock
        if BL = 01h
            ES:ECX -> pointer to first page to lock
Return: CF clear if successful
        CF set on error
            EAX = error code
                08h insufficient memory
                09h invalid address range
SeeAlso: AX=251Bh,AX=EB06h,INT 31/AX=0600h
--------E-21251B-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - UNLOCK PAGES
        AX = 251Bh
        EDX = number of pages to unlock
        if BL = 00h
            ECX = linear address of first page to unlock
        if BL = 01h
            ES:ECX -> pointer to first page to unlock
Return: CF clear if successful
        CF set on error
            EAX = error code
                09h invalid address range
SeeAlso: AX=251Ah,AX=EB07h,INT 31/AX=0601h
--------E-21251C-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM v2.1c+ - FREE PHYSICAL MEMORY PAGES
        AX = 251Ch
        BH = preservation flag (00h preserve contents, 01h discard contents)
        EDX = number of pages to free
        BL = address type
            00h linear address
                ECX = linear address of first page to be freed
            01h pointer
                ES:ECX -> first page to be freed
Return: CF clear if successful
        CF set on error
            EAX = error code
                08h memory error, swap space full, no VMM or DPMI
                09h invalid address
--------E-21251D-----------------------------
INT 21 OP - Phar Lap 386/DOS-Extender VMM v2.1c - READ PAGE-TABLE ENTRY
        AX = 251Dh
        BL = address type
            00h linear address
                ECX = linear address of page table entry to read
            01h pointer
                ES:ECX -> page table entry to read
Return: CF clear if successful
            EAX = contents of page table entry
        CF set on error
            EAX = error code
                09h invalid address or NOPAGE option set
                78h invalid under DPMI
Note:   this function is obsolete; use AX=252Bh/BH=09h instead
SeeAlso: AX=251Eh,AX=252Bh/BH=09h,AX=EB00h,INT 31/AX=0506h
--------E-21251E-----------------------------
INT 21 OP - Phar Lap 386/DOS-Extender VMM v2.1c - WRITE PAGE-TABLE ENTRY
        AX = 251Eh
        BL = address type
            00h linear address
                ECX = linear address of page table entry to read
            01h pointer
                ES:ECX -> page table entry to read
        EDX = new value for page table entry
Return: CF clear if successful
        CF set on error
            EAX = error code
                09h invalid address or NOPAGE option set
                82h not compatible with DPMI
Note:   this call is obsolete; use AX=252Bh/BH=0Ah instead
SeeAlso: AX=251Dh,AX=252Bh/BH=0Ah,INT 31/AX=0507h
--------E-21251F-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - EXHANGE TWO PAGE-TABLE ENTRIES
        AX = 251Fh
        BL = address type
            00h linear address
                ECX = linear address of first page table entry
                EDX = linear address of second page table entry
            01h pointer
                ES:ECX -> first page table entry
                ES:EDX -> second page table entry
Return: CF clear if successful
        CF set on error
            EAX = error code
                09h invalid address or NOPAGE option set
                82h not compatible with DPMI
SeeAlso: AX=251Dh,AX=251Eh
--------E-212520-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - GET MEMORY STATISTICS
        AX = 2520h
        DS:EDX -> pointer to buffer at least 100 bytes in size (see #01364)
        BL = 0 (don't reset VM stats), 1 (reset VM stats)
Return: carry flag clear

Format of Phar Lap VM statistics buffer:
Offset  Size    Description     (Table 01364)
 00h    DWORD   VM status
                0001h VM subsystem is present
                0000h VM not present
 04h    DWORD   "nconvpg" number of conventional memory pages available
 08h    DWORD   "nbimpg" number of Compaq built-in memory pages available
 0Ch    DWORD   "nextpg" total number of extended memory pages
 10h    DWORD   "extlim" extender memory pages limit
 14h    DWORD   "aphyspg" number of physical memory pages allocated to appl
 18h    DWORD   "alockpg" number of locked pages owned by application
 1Ch    DWORD   "sysphyspg" number physical memory pages allocated to system
 20h    DWORD   "nfreepg" number of free physical pages; approx if EMS VCPI
 24h    DWORD   linear address of beginning of application address space
 28h    DWORD   linear address of end of application address space
 2Ch    DWORD   number of seconds since last time VM stats were reset
 30h    DWORD   number of page faults since last time
 34h    DWORD   number of pages written to swap file since last time
 38h    DWORD   number of reclaimed pages (page faults on swapped pages)
 3Ch    DWORD   number of virtual pages allocated to the application
 40h    DWORD   size in pages of swap file
 44h    DWORD   number of system pages allocated with EMS calls
 48h    DWORD   minimum number of conventional memory pages
 4Ch    DWORD   maximum size in pages to which swap file can be increased
 50h    DWORD   "vmflags"
                bit 0 = 1 if page fault in progress
---v4.0+ ---
 54h    DWORD   number of physical pages guaranteed to be free
 58h    DWORD   number of free physical pages currently available
 5Ch    DWORD   size in pages of largest free block of memory (including disk
                  swap space)
 60h    DWORD   reserved
--------E-212521-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - LIMIT PROGRAM'S EXTENDED MEM USAGE
        AX = 2521h
        EBX = max 4k pages of physical extended memory which program may use
Return: CF clear if successful
           EBX = maximum limit in pages
           ECX = minimum limit in pages
        CF set on error
            EAX = error code
                08h insufficient memory or -nopage switch used
SeeAlso: AX=2522h
--------E-212522-----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v2.2+ - SPECIFY ALTERNATE PAGE-FAULT HANDLR
        AX = 2522h
        ES:EBX -> alternate handler for page faults
Return: CF clear
        ES:EBX -> previous page-fault handler
SeeAlso: AX=2523h
--------E-212523-----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v2.2+ - SPECIFY OUT-OF-SWAP-SPACE HANDLER
        AX = 2523h
        ???
Return: ???
Note:   this function takes a DWORD pointer and a DWORD pointer to a DWORD
          pointer as arguments
SeeAlso: AX=2522h
--------E-212524-----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v2.2+ - INSTALL PAGE-REPLACEMENT HANDLERS
        AX = 2524h
        ???
Return: ???
Note:   this function takes three DWORD pointers and three DWORD pointers to
          DWORD pointers as arguments
--------E-212525-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - LIMIT PROGRAM'S CONVENT'L MEM USAGE
        AX = 2525h
        EBX = limit in 4k pages of physical conventional memory which program
              may use
Return: CF clear if successful
            EBX = maximum limit in pages
            ECX = minimum limit in pages
        CF set on error
            EAX = error code
                08h insufficient memory or -nopage switch used
SeeAlso: AX=2521h
--------E-212526-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET CONFIGURATION INFORMATION
        AX = 2526h
        ???
Return: ???
Notes:  details are not yet available
        this function takes a pointer to the configuration buffer (see #01365)
          and a poitner to a BYTE as arguments

Format of Phar Lap configuration buffer:
Offset  Size    Description     (Table 01365)
 00h    DWORD   flags 1 (see #01366)
 04h    DWORD   flags 2 (unused through v5.0)
 08h    DWORD   flags 3 (unused through v5.0)
 0Ch    DWORD   386|DOS-Extender major version
 10h    DWORD   386|DOS-Extender minor version
 14h    DWORD   first letter of text after minor version number in version str
 18h    DWORD   beta flag (00h normal release, 01h beta release)
 1Ch    DWORD   processor (3 = 386, 4 = 486)
 20h    DWORD   coprocessor (4 = none, 6 = 287, 7 = 387/486)
 24h    DWORD   Weitek coprocessor flag (0 = none, 1 = present)
 28h    DWORD   machine type (0 = IBM PC compatible, 1 = NEC 9800 series)
 2Ch    DWORD   machine class
                IBM: bus type (0=ISA, 1=MCA, 2=XT, 3=EISA)
                NEC: 0=normal mode, 1=high-res mode
 30h    DWORD   VCPI flag (0 = none, 1 = present)
 34h    DWORD   -WEITEK/-1167 switch (0 = AUTO, 1 = ON, 2 = OFF)
 38h    DWORD   -MINREAL setting
 3Ch    DWORD   -MAXREAL setting
 40h    DWORD   -MINIBUF setting
 44h    DWORD   -MAXIBUF setting
 48h    DWORD   size in bytes of DOS call data buffer
 4Ch    DWORD   number of interrupt stacks (-NISTACK)
 50h    DWORD   interrupt stack size (-ISTKSIZE)
 54h    DWORD   -REALBREAK setting
 58h    DWORD   -CALLBUFS
 5Ch    DWORD   -HWIVEC
 60h    DWORD   -PRIVEC
 64h    DWORD   -INTMAP
 68h    DWORD   -PRIMAP
 6Ch    DWORD   VCPI: master 8259 interrupt vector base (IRQ0 mapping)
 70h    DWORD   VCPI: slave 8259 interrupt vector base (IRQ8 mapping)
 74h    DWORD   BIOS print screen interrupt vector (0 if NEC)
 78h    DWORD   -EXTLOW setting
 7Ch    DWORD   -EXTHIGH setting
 80h    DWORD   lowest physical extended-memory address allocatable
 84h    DWORD   highest physical extended-memory address allocatable + 1
 88h    DWORD   special memory's physical base address (00000000h if none)
 8Ch    DWORD   special memory size in bytes (00000000h if none)
 90h    DWORD   -MAXVCPIMEM setting
 94h    DWORD   -VSCAN
 98h    DWORD   -SWAPCHK (0 = OFF, 1 = ON, 2 = FORCE, 3 = MAX)
 9Ch    DWORD   -CODESIZE setting
 A0h    DWORD   minimum swap file size (-MINSWFSIZE)
 A4h    DWORD   maximum swap fiel size (-MAXSWFSIZE)
 A8h    DWORD   page replacement policy (0 = LFU, 1 = NUR)
 ACh    DWORD   number of GDT entries (-NGDTENT)
 B0h    DWORD   number of LDT entries (-NLDTENT)
 B4h    DWORD   program's privilege level (0-3)
---386|DOS-Extender v3.0+ ---
 B8h    DWORD   -LOCKSTACK setting
 BCh    DWORD   -MAXEXTMEM
 C0h    DWORD   -MAXXMSMEM
 C4h    DWORD   -MAXPGMMEM
 C8h    DWORD   -DATATHRESHOLD
 CCh    DWORD   virtual memory manager flag (0 = not present, 1 = present)
 D0h    DWORD   Cyrix coprocessor flag (0 = no Cyrix EMC387, 1 = present)
 D4h    DWORD   -CYRIX setting (0 = AUTO, 1 = ON, 2 = OFF)
 D8h    DWORD   DPMI flag (0 = not present, 1 = present)
 DCh    DWORD   DPMI major version
 E0h    DWORD   DPMI minor version
 E4h    DWORD   DPMI capabilities flags (see #01367)
 E8h    DWORD   VCPI major version
 ECh    DWORD   VCPI minor version
 F0h    WORD    VCPI: IRQ0-7 physical base interrupt vector
 F2h    WORD    VCPI: IRQ8-15 physical base interrupt vector
 F4h    DWORD   XMS flag (0 = none, 1 = present)
 F8h    DWORD   XMS major version
 FCh    DWORD   XMS minor version
100h    WORD    application's CS selector
102h    WORD    application's DS selector
104h    WORD    application's PSP selector
106h    WORD    application's environment selector
108h    WORD    selector mapping entire first megabyte
10Ah    WORD    selector mapping text video memory
10Ch    WORD    selector mapping video memory (text for IBM, graphics for NEC)
10Eh    WORD    selector mapping Weitek address space, 0000h if not present
110h    WORD    selector mapping Cyrix EMC387 address space, 0000h if none
112h    WORD    reserved (0)
114h    DWORD   real-mode FAR entry point to call to switch to protected mode
                  with no saved context
118h    DWORD   size of LDT in bytes
---386|DOS-Extender v5.0+ ---
11Ch    DWORD   Windows flag (0 = not present, 1 = Windows present)
120h    DWORD   Windows major version
124h    DWORD   Windows minor version
128h    DWORD   Windows mode (0 = real/standard, 1 = enhanced)
12Ch    DWORD   OS/2 flag (0 = not present, 1 = OS/2 present)
130h    DWORD   OS/2 major version
134h    DWORD   OS/2 minor version
138h 50 DWORDs  reserved (0)

Bitfields for flags 1:
Bit(s)  Description     (Table 01366)
 0      -NOPAGE specified
 1      -A20 specified
 2      -VDISK specified
 3      -XT specified
 4      -AT specified
 5      -MCA specified
 6      -EISA specified
 7      -NORMRES specified (NEC only)
 8      -HIGHRES specified (NEC only)
 9      set if -SWFGROW1ST, clear if -NOSWFGROW1ST
 10     -NOVM specified
 11     -SAVEREGS specified
 12     unused (clear)
 13     -NOVCPI specified
 14     -NOMUL specified
 15     -NOBMCHK specified
 16     -NOSPCLMEM or -NOBIM
 17     -NOPGEXP specified
 18     -SWAPDEFDISK specified
---v3.0+ ---
 19     -SAVEINTS specified
 20     -NOLOAD specified
 21     -PAGELOG specified
 22     -OPENDENY specified
 23     -ERRATA17 specified
---v4.1+ ---
 24     -NESTDPMI specified
 25     -NONESTDPMI specified
 26     -NODPMI specified
 27     -NOPCDWEITEK specified
---v4.2+ ---
 28     -WININT21 specified

Bitfields for DPMI capabilities flags:
Bit(s)  Description     (Table 01367)
 0      paging supported
 1      physical device mapping
 2      conventional memory mapping
 3      exceptions restartable
--------E-212527-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - EN/DISABLE STATE SAVE ON INTERRUPTS
        AX = 2527h
        EBX = new status (00h disabled, 01h enabled)
Return: CF clear
        EBX = previous state save flag
SeeAlso: AX=2528h
--------E-212528-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - READ REGISTERS AFTER CTRL-C INT
        AX = 2528h
        DS:EBX -> buffer for registers (see #01368)
Return: CF clear if successful
            DS:EBX buffer filled
        CF set on error
            EAX = error code
                83h interrupt state save not enabled
                84h no active interrupt
SeeAlso: AX=2527h

Format of Phar Lap buffer for registers:
Offset  Size    Description     (Table 01368)
 00h  8 BYTEs   unused
 08h  4 DWORDs  EAX,EBX,ECX,EDX
 18h  4 DWORDs  ESI,EDI,EBP,ESP
 28h  6 WORDs   CS,DS,SS,ES,FS,GS
 34h    DWORD   EIP
 38h    DWORD   EFLAGS
--------E-212529-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - LOAD FLAT MODEL .EXP or .REX FILE
        AX = 2529h
        ???
Return: ES:EBX -> parameter block (see #01369)
        ???
Note:   details not available at this time
SeeAlso: AX=252Ah

Format of Phar Lap program load parameter block:
Offset  Size    Description     (Table 01369)
 00h    DWORD   initial EIP
 04h    WORD    initial CS
 06h    DWORD   initial ESP
 0Ah    WORD    initial SS
 0Ch  4 WORDs   initial DS, ES, FS, GS
 14h    DWORD   minimum size in bytes of program segment
 18h    DWORD   bytes of additional memory allocated
 1Ch    DWORD   flags
                bit 0: child linked with -UNPRIVILEGED
                ---v6.0+ ---
                bit 1: child is PE file instead of .EXP
                bit 2: loaded file is a DLL
                bits 3-31 reserved
---v6.0+ ---
 20h    DWORD   module handles (PE files only)
 24h  7 DWORDs  reserved (0)
--------E-21252A-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM - NEW LOAD PROGRAM FOR DEBUG
        AX = 252Ah
        DS:EDX -> ASCIZ program name
        ES:EBX -> parameter block (see #01363)
        ECX = size of LDT buffer in bytes
        ESI = bit flags
            bit 0: allow demand paging rather than loading entire program
            bit 1: demand page from swap file rather than from .EXP
Return: CF clear if successful
            EAX = VMM handle or FFFFFFFFh if none
            ECX = number of descriptors in LDT buffer
        CF set on error
            EAX = error code
                02h file error
                    EBX = file error code (see #01370)
                    ECX = DOS error code if EBX=1,2,3, or 8
                08h insufficient memory
                    EBX = memory error code (see #01371)
                80h LDT buffer too small
                87h called twice without intervening call to AX=2531h
SeeAlso: AX=2512h,AX=2517h,AX=2529h,AX=2531h

(Table 01370)
Values for Phar Lap file error code:
 01h    DOS open error
 02h    DOS seek error
 03h    DOS read error
 04h    not an .EXP or .REX file
 05h    invalid file format
 06h    -OFFSET is not a multiple of 64K
 07h    -NOPAGE incompatible with -REALBREAK/-OFFSET
 08h    DOS error loading .EXE file

(Table 01371)
Values for Phar Lap memory error code:
 01h    out of physical memory
 02h    out of swap space
 04h    unable to change extended memory allocation
 05h    -MAXPGMMEM exceeded
 06h    insufficient low memory to REALBREAK value
 07h    insufficient low memory for PSP and environment
--------E-21252BBH00-------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender - CREATE UNMAPPED PAGES
        AX = 252Bh
        BH = 00h
        ???
Return: ???
--------E-21252BBH01-------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender - CREATE ALLOCATED PAGES
        AX = 252Bh
        BH = 01h
        ???
Return: ???
--------E-21252BBH02-------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender - CREATE PHYSICAL DEVICE PAGES
        AX = 252Bh
        BH = 02h
        ???
Return: ???
--------E-21252BBH03-------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender - MAP DATA FILE
        AX = 252Bh
        BH = 03h
        ???
Return: ???
SeeAlso: AX=252Bh/BH=0Bh
--------E-21252BBH04-------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender - GET PAGE TYPES
        AX = 252Bh
        BH = 04h
        ???
Return: ???
Note:   returns one word per page
SeeAlso: #01372

Bitfields for Phar Lap page information:
Bit(s)  Description     (Table 01372)
 7      mapped to read/write data file
 6      mapped to read-only data file
 5      swapped to disk
 4      locked
 3-0    page type
        0 unmapped
        1 allocated
        2 physical device page
--------E-21252B-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - VIRTUAL MEMORY MANAGEMENT - PAGE LOCKING
        AX = 252Bh
        BH = function
            05h lock pages
            06h unlock pages
        BL = address type
            00h linear address
                ECX = linear start address of memory region
            01h segmented address
                ES:ECX -> start of memory region
        EDX = size of memory region in bytes
Return: CF clear if successful
        CF set on error
Note:   this function is also supported by FlashTek X-32VM; if X-32 is not
          using virtual memory, this function always succeeds
--------E-21252B-----------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender - FREE PHYSICAL PAGES
        AX = 252Bh
        BH = function (07h,08h)
        ???
Return: ???
--------E-21252BBH09-------------------------
INT 21 P - Phar Lap 386/DOS-Extender v4.1 - GET PAGETABLE ENTRY/PAGE TABLE INFO
        AX = 252Bh
        BH = 09h
        BL = subfunction
            00h get page table entry by linear address
                ECX = linear address for which to get page table entry
            01h get page table entry by logical address
                ES:ECX = address for which to get page table entry
Return: CF clear if successful
            EAX = page table entry
            EBX = additional page table information
        CF set on error
            EAX = error code
                0009h invalid address
                0082h running under DPMI
SeeAlso: AX=251Dh,AX=252Bh/BH=0Ah
--------E-21252BBH0A-------------------------
INT 21 P - Phar Lap 386/DOS-Extender v4.1 - SET PAGETABLE ENTRY/PAGE TABLE INFO
        AX = 252Bh
        BH = 0Ah
        BL = subfunction
            00h set page table entry for linear address
                ECX = linear address for which to get page table entry
            01h set page table entry for logical address
                ES:ECX = address for which to get page table entry
        ESI = page table entry
        EDI = additional page table information
Return: CF clear if successful
        CF set on error
            EAX = error code
                0009h invalid address
                0082h running under DPMI
SeeAlso: AX=252Bh/BH=09h
--------E-21252BBH0B-------------------------
INT 21 P - Phar Lap 386/DOS-Extender v4.1+ - MAP DATA FILE AT FILE OFFSET
        AX = 252Bh
        BH = 0Bh
        BL = subfunction
            00h by linear address
                ECX = linear address at which to map data file
            01h by logical address
                ES:ECX = logical address at which to map data file
        EDX = number of bytes to map
        DS:ESI -> mapping structure (see #01373)
        DS:EDI -> ASCIZ filename
Return: CF clear if successful
        CF set on error
            EAX = error code
                0002h file error
                    ECX = phase (01h opening file, 02h seeking, 03h reading)
                    EDX = error code returned by DOS
                0009h invalid address
                0081h invalid parameters or 386|VMM not present
                0086h all 386|VMM file handles already in use
SeeAlso: AX=252Bh/BH=03h,AX=252Bh/BH=09h

Format of Phar Lap mapping structure:
Offset  Size    Description     (Table 01373)
 00h    DWORD   starting file offset to be mapped
 04h    DWORD   DOS file access and sharing modes (see #01403 at INT 21/AH=3Dh)
--------E-21252C-----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v3.0 - ADD UNMAPPED PAGES AT END OF SEGMENT
        AX = 252Ch
        BX = segment selector
        ECX = number of 4K pages to add
Return: CF clear if successful
            EAX = offset in segment of beginning of unmapped pages
        CF set on error
            EAX = error code
                08h insufficent memory
                09h invalid selector
                82h not supported by current DPMI
--------E-21252D-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM v2.3+ - CLOSE VMM FILE HANDLE
        AX = 252Dh
        EBX = VMM file handle
Return: CF clear if successful
        CF set on error
            EAX = error code (81h invalid VMM handle)
--------E-21252E-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM v2.3+ - GET/SET VMM PARAMETERS
        AX = 252Eh
        CL = direction (00h get parameters, 01h set parameters)
        DS:EBX -> parameter buffer (see #01374)
Return: CF clear if successful
        CF set on error
            EAX = error code (81h bad parameter value)

Format of Phar Lap VMM parameter buffer:
Offset  Size    Description     (Table 01374)
 00h    DWORD   flags
                bit 0: page fault logging enabled
                ---v5.0+ ---
                bit 1: swap extender to disk during DOS EXEC call
                bit 2: don't zero allocated memory
 04h    DWORD   scan period for page aging, in milliseconds
 08h    DWORD   maximum size (in bytes) to check on each page scan
 0Ch 52 BYTEs   unused
--------E-21252F-----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v3.0 - WRITE RECORD TO VMM PAGE LOG FILE
        AX = 252Fh
        DS:EBX -> data to be written
        CX = size of data in bytes
Return: CF clear if successful
        CF set on error
            EAX = error code (85h no page log file or not 386/VMM)
--------E-212530-----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v2.3+ - SET SIZE OF BUFFER FOR DOS CALLS
        AX = 2530h
        ECX = size of data buffer in bytes (1024 to 65536)
Return: CF clear if successful
        CF set on error
            EAX = error code
                08h insufficient low memory
                81h invalid size
SeeAlso: AX=2517h
--------E-212531-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM v3.0 - READ/WRITE LDT DESCRIPTOR
        AX = 2531h
        BX = segment selector
        ECX = direction (00h read, 01h write)
        DS:EDX -> 8-byte buffer for descriptor contents
Return: CF clear if successful
        CF set on error
            EAX = error code
                81h invalid selector
                82h DPMI running, or not a code or data segment
--------E-212532-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - GET EXCEPTION HANDLER VECTOR
        AX = 2532h
        CL = exception number (00h-0Fh)
Return: CF clear if successful
            ES:EBX = CS:EIP of current exception handler
        CF set on error (CL > 0Fh)
Notes:  this call is also supported by the FlashTek X-32VM extender
        this function is incompatible with 386|VMM; use AX=2522h instead
SeeAlso: AX=2522h,AX=2533h
--------E-212533-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - SET EXCEPTION HANDLER VECTOR
        AX = 2533h
        CL = exception number (00h-0Fh)
        DS:EDX = CS:EIP of new exception handler
Return: CF clear if successful
        CF set on error (CL > 0Fh)
Notes:  this call is also supported by the FlashTek X-32VM extender
        this function is incompatible with 386|VMM; use AX=2522h instead
SeeAlso: AX=2522h,AX=2532h
--------E-212534-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v3.0+ - GET INTERRUPT FLAG
        AX = 2534h
Return: CF clear
        EAX = interrupt state (00h disabled, 01h enabled)
--------E-212535-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v3.0+ - READ/WRITE SYSTEM REGISTERS
        AX = 2535h
        EBX = direction (00h read registers, 01h write)
        DS:EDX -> system register record (see #01375)
Return: CF clear
Note:   this call is only available under MS Windows if PHARLAP.386 VDD is
          installed

Format of Phar Lap system register record:
Offset  Size    Description     (Table 01375)
 00h    DWORD   CR0
 04h  4 DWORDs  DR0,DR1,DR2,DR3
 14h  2 DWORDs  reserved
 1Ch  2 DWORDs  DR6,DR7
 24h  3 DWORDs  reserved
--------E-212536----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v3.0+ - MIN/MAX EXTENDED/CONV MEMORY USAGE
        AX = 2536h
        EBX = bit flags
            bit 0: modifying conventional memory rather than extended memory
            bit 1: setting maximum memory usage rather than minimum
        ECX = new limit in 4K pages
Return: CF clear if successful
            EAX = new limit
        CF set on error
            EAX = error code (08h memory error or -NOPAGE set)
            EBX = maximum limit in pages
            ECX = minimum limit in pages
--------E-212537----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v3.0 - ALLOCATE DOS MEMORY ABOVE DOS BUFFER
        AX = 2537h
        BX = number of paragraphs to allocate
Return: CF clear if successful
            AX = real-mode segment of allocated block
        CF set on error
            AX = error code
                07h MS-DOS memory chain corrupted
                08h insufficient low memory
            BX = size in paragraphs of largest free block
SeeAlso: AH=48h
--------E-212538----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v3.0 - READ PROTMODE REGS AFTER SFTWARE INT
        AX = 2538h
        DS:EBX -> buffer for registers (see #01368)
        ECX = register record to retrieve
            00h first interrupt state
            01h next interrupt state
                EDX = handle for current interrupt state
Return: CF clear if successful
            DS:EBX buffer filled
            EDX = handle of current interrupt state
            ESI = number of interrupt which occurred
        CF set on error
            EAX = error code
                81h invalid handle in EDX
                83h register saving not enabled
                84h no more interrupt states
SeeAlso: AX=2527h,AX=2528h
--------E-212539----------------------------
INT 21 P - Phar Lap 386/DOS-Ext VMM v3.0 - GET OFFSET OF .EXP FILE HEADER
        AX = 2539h
        BX = MS-DOS file handle for open file
Return: CF clear if successful
            EAX = offset of .EXP header in file
        CF set on error
            EAX = error code (02h file error)
            EBX = file error code
                02h DOS error seeking
                03h DOS error reading
                04h invalid file type
                05h invalid file format
            ECX = DOS error code if EBX=02h or 03h
        current file position in file modified
--------E-21253A----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v3.0+ - INSTALL MOD. SEG FAILURE HANDLER
        AX = 253Ah
        ES:EBX -> function to be called when INT 21/AH=4Ah is about to return
                an error
Return: CF clear
        ES:EBX -> previous handler
SeeAlso: AH=4Ah
--------E-21253B----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v3.0+ - JUMP TO REAL MODE CODE, NO CONTEXT
        AX = 253Bh
        DS:EBX -> buffer containing register contents (see #01368)
Return: never returns
SeeAlso: AX=2528h
--------E-21253C-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender VMM v3.0+ - SHRINK 386|VMM SWAP FILE
        AX = 253Ch
Return: CF clear
        EAX = old size of swap file in bytes
        EBX = new size of swap file in bytes
--------E-21253D-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender v4.0+ - READ/WRITE IDT DESCRIPTOR
        AX = 253Dh
        BL = interrupt number
        ECX = direction (0 = read, 1 = write)
        DS:EDX -> 8-byte buffer for descriptor
Return: CF clear if successful
            DS:EDX filled if reading
        CF set on error
            EAX = error code (0082h if running under DPMI)
Desc:   access hardware-level IDT rather than the internal 386/DOS-Extender
          shadow IDT
Notes:  this call will always fail under DPMI because it is not possible to
          access the IDT
        the descriptor is not checked when writing
        this call can normally be used only by programs running in ring 0
          because the processor does not allow an interrupt to be vectored to
          a less privileged ring
--------E-21253F-----------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender v6.0+ - ALLOCATE LDT DESCRIPTOR
        AX = 253Fh
        BX = LDT descriptor to allocate (0000h for any)
        ???
Return: ???
--------E-212540-----------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender v6.0+ - FORCE ALIAS OF SEGMENT
        AX = 2540h
        ???
Return: ???
--------E-212544-----------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender v6.0+ - FREE DLL
        AX = 2544h
        ???
Return: ???
--------E-212545-----------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender v6.0+ - GET/SET PROCEDURE ADDRESS
        AX = 2545h
        ECX = direction (00h get, 01h set)
        ???
Return: ???
--------E-212546-----------------------------
INT 21 Pu - Phar Lap 386/DOS-Extender v6.0+ - GET MODULE HANDLE
        AX = 2546h
        ???
Return: ???
--------E-2125C0-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - ALLOCATE MS-DOS MEMORY BLOCK
        AX = 25C0h
        BX = number of 16-byte paragraphs of MS-DOS memory requested
Return: CF clear if successful
            AX = real-mode paragraph address of memory
        CF set on error
            AX = error code
                07h MS-DOS memory control blocks destroyed
                08h insufficient memory
            BX = size in paragraphs of largest available memory block
SeeAlso: AX=25C1h,AX=25C2h
--------E-2125C1-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - RELEASE MS-DOS MEMORY BLOCK
        AX = 25C1h
        CX = real-mode paragraph address of memory block to free
Return: CF clear if successful
            EAX destroyed
        CF set on error
            AX = error code
                07h MS-DOS memory control blocks destroyed
                09h invalid memory block address in CX
SeeAlso: AX=25C0h,AX=25C2h
--------E-2125C2-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - MODIFY MS-DOS MEMORY BLOCK
        AX = 25C2h
        BX = new requested block size in paragraphs
        CX = real-mode paragraph address of memory block to modify
Return: CF clear if successful
            EAX destroyed
        CF set on error
            AX = error code
                07h MS-DOS memory control blocks destroyed
                08h insufficient memory
                09h invalid memory block address in CX
            BX = size in paragraphs of largest available memory block
SeeAlso: AX=25C0h,AX=25C1h
--------E-2125C3-----------------------------
INT 21 P - Phar Lap 386/DOS-Extender - EXECUTE PROGRAM
        AX = 25C3h
        ES:EBX -> pointer to parameter block (see #01377)
        DS:EDX -> pointer to ASCIZ program filename
Return: CF clear if successful
            all registers unchanged
        CF set on error
            EAX = error code (see #01376)

(Table 01376)
Values for Phar Lap error code:
 01h    function code in AL is invalid ???
 02h    file not found or path invalid
 05h    access denied
 08h    insufficient memory to load program
 0Ah    environment invalid
 0Bh    invalid file format

Format of parameter block:
Offset  Size    Description     (Table 01377)
 00h    DWORD   32-bit offset of environment string
 04h    WORD    segment selector of environment string
 06h    DWORD   32-bit offset of command-tail string
 0Ah    WORD    segment selector of command-tail string
--------E-212B--CX4149-----------------------
INT 21 - AI Architects - ??? - INSTALLATION CHECK
        AH = 2Bh
        CX = 4149h ('AI')
        DX = 413Fh ('A?')
Return: AL <> FFh if installed
Note:   Borland's TKERNEL makes this call
--------c-212B--CX4358-----------------------
INT 21 - Super PC-Kwik v3.20+ - INSTALLATION CHECK
        AH = 2Bh
        CX = 4358h ('CX')
Return: AL = FFh if PC-Kwik/PC-Cache not installed
        AL = 00h if installed
            CF clear
            CX = 6378h ('cx')
            BX = ???
            DX = version (DH = major version, DL = binary minor version)
Note:   PC Tools PC-Cache v5.x and Qualitas Qcache v4.00 are OEM versions of
          Super PC-Kwik, and thus support this call (PC-Cache 5.1 corresponds
          to PC-Kwik v3.20)
SeeAlso: INT 13/AH=A0h,INT 13/AH=B0h,INT 16/AX=FFA5h/CX=1111h
Index:  PC-Cache;installation check|Qualitas Qcache;installation check
Index:  installation check;PC-Cache 5.x|installation check;Qualitas Qcache
--------Q-212B--CX4445-----------------------
INT 21 - DESQview - INSTALLATION CHECK
        AH = 2Bh
        CX = 4445h ('DE')
        DX = 5351h ('SQ')
        AL = subfunction (DV v2.00+)
            01h get version
                Return: BX = version (BH = major, BL = minor)
                Note: early copies of v2.00 return 0002h
            02h get shadow buffer info, and start shadowing
                Return: BH = rows in shadow buffer
                        BL = columns in shadow buffer
                        DX = segment of shadow buffer
            04h get shadow buffer info
                Return: BH = rows in shadow buffer
                        BL = columns in shadow buffer
                        DX = segment of shadow buffer
            05h stop shadowing
Return: AL = FFh if DESQview not installed
Notes:  in DESQview v1.x, there were no subfunctions; this call only identified
          whether or not DESQview was loaded.  DESQview v2.52 performs function
          01h for all subfunction requests 0Ch and higher and appears to ignore
          all lower-numbered functions not listed here.
        DESQview versions 2.50-2.52 are part of DESQview/X v1.0x; version 2.53
          is part of DESQview/X v1.10; and version 2.63 is part of DESQview/X
          v2.00.
BUG:    subfunction 05h does not appear to work correctly in DESQview 2.52
SeeAlso: INT 10/AH=FEh,INT 10/AH=FFh,INT 15/AX=1024h,INT 15/AX=DE30h
--------U-212B--CX454C-----------------------
INT 21 - ELRES v1.1 - INSTALLATION CHECK
        AH = 2Bh
        CX = 454Ch ('EL')
        DX = 5253h ('RS')
Return: ES:BX -> ELRES history structure (see #01381)
        DX = DABEh (signature, DAve BEnnett)
Program: ELRES is an MS-DOS return code (errorlevel) recorder by David H.
          Bennett which stores recent errorlevel values, allows them to be
          retrieved for use in batch files, and can place them in an
          environment variable
SeeAlso: AH=4Bh"ELRES",AH=4Dh

Format of ELRES history structure:
Offset  Size    Description     (Table 01381)
 00h    WORD    number of return codes which can be stored by following buffer
 02h    WORD    current position in buffer (treated as a ring)
 04h  N BYTEs   ELRES buffer
--------l-212B00CX5643-----------------------
INT 21 - The Volkov Commander - GET POINTER TO LEFT PANEL DATA STRUCTURE
        AX = 2B00h
        CX = 5643h ('VC')
        DX = 4F4Dh ('OM')
Return: AL = 0
        ES:BX -> left panel data structure
Program: Volcov Commander is a shell for MS-DOS by Vsevolod V. Volkov
SeeAlso: AX=2B01h/CX=5643h,AX=2B02h/CX=5643h
--------l-212B01CX5643-----------------------
INT 21 - The Volkov Commander - GET POINTER TO RIGHT PANEL DATA STRUCTURE
        AX = 2B01h
        CX = 5643h ('VC')
        DX = 4F4Dh ('OM')
Return: AL = 0
        ES:BX -> right panel data structure
SeeAlso: AX=2B00h/CX=5643h,AX=2B02h/CX=5643h
--------l-212B02CX5643-----------------------
INT 21 - The Volkov Commander - GET POINTER TO GLOBAL VARIABLES
        AX = 2B02h
        CX = 5643h ('VC')
        DX = 4F4Dh ('OM')
Return: AL = 0
        ES:BX -> global variables
SeeAlso: AX=2B00h/CX=5643h,AX=2B01h/CX=5643h
--------m-212B01CX444D-----------------------
INT 21 - Quarterdeck DOS-UP.SYS v2.00 - INSTALLATION CHECK
        AX = 2B01h
        CX = 444Dh ('DM')
        DX = 4158h ('AX')
Return: AX = 0000h if installed
            BX = DOS-UP version (BH = minor, BL = major)
            CX = 4845h ('HE')
            DX = 5245h ('RE')
            ES = DOS-UP driver segment
--------T-212B01CX5441-----------------------
INT 21 - TAME v2.10+ - INSTALLATION CHECK
        AX = 2B01h
        CX = 5441h ('TA')
        DX = 4D45h ('ME')
---v2.60---
        BH = ???
            00h skip ???, else do
Return: AL = 02h if installed
        ES:DX -> data area in TAME-RES (see #01382,#01386,#01388)
Program: TAME is a shareware program by David G. Thomas which gives up CPU
          time to other partitions under a multitasker when the current
          partition's program incessantly polls the keyboard or system time

Format of TAME 2.10-2.20 data area:
Offset  Size    Description     (Table 01382)
 00h    BYTE    data structure minor version number (01h in TAME 2.20)
 01h    BYTE    data structure major version number (07h in TAME 2.20)
 02h    DWORD   number of task switches
 06h    DWORD   number of keyboard polls
 0Ah    DWORD   number of time polls
 0Eh    DWORD   number of times DESQview told program runs only in foreground
 12h    DWORD   original INT 10h
 16h    DWORD   original INT 14h
 1Ah    DWORD   original INT 15h
 1Eh    DWORD   original INT 16h
 22h    DWORD   original INT 17h
 26h    DWORD   original INT 21h
 2Ah    DWORD   original INT 28h
 2Eh    WORD    offset of TAME INT 10h handler
 30h    WORD    offset of TAME INT 14h handler
 32h    WORD    offset of TAME INT 15h handler
 34h    WORD    offset of TAME INT 16h handler
 36h    WORD    offset of TAME INT 17h handler
 38h    WORD    offset of TAME INT 21h handler
 3Ah    WORD    offset of TAME INT 28h handler
 3Ch    WORD    X in /max:X,Y or /freq:X,Y
 3Eh    WORD    Y in /max:X,Y or /freq:X,Y
 40h    WORD    number of polls remaining before next task switch
 42h    WORD    /KEYIDLE value
 44h    BYTE    interrupts already grabbed by TAME (see #01383)
 45h    BYTE    flags for interrupts which may be acted on (same bits as above)
 46h    BYTE    TAME enabled (01h) or disabled (00h)
 47h    BYTE    /TIMEPOLL (01h) or /NOTIMEPOLL (00h)
 48h    BYTE    /NOTIMER (01h) or /TIMER (00h)
 49h    BYTE    window or task number for this task
 4Ah    BYTE    multitasker type (see #01384)
 4Bh    BYTE    type of task switching selected
                bit 0: DESQview???
                bit 1: DoubleDOS???
                bit 2: TopView???
                bit 3: KeySwitch
                bit 4: HLT instruction
 4Ch    BYTE    ???
 4Dh    BYTE    flags
                bit 1: /FREQ instead of /MAX
 4Eh    BYTE    /FG: value
 4Fh    BYTE    task switches left until next FGONLY DESQview API call
 50h    BYTE    ???

Bitfields for interrupts already grabbed by TAME:
Bit(s)  Description     (Table 01383)
 0      INT 10h
 1      INT 14h
 2      INT 15h
 3      INT 16h
 4      INT 17h
 5      INT 21h
 6      INT 28h

(Table 01384)
Values for multitasker type:
 01h    DESQview
 02h    DoubleDOS
 03h    TopView
 04h    OmniView
 05h    VM/386

Bitfields for type of task switching selected:
Bit(s)  Description     (Table 01385)
 0      DESQview
 1      DoubleDOS
 2      TopView
 3      OmniView
 4      KeySwitch
 5      HLT instruction

Format of TAME 2.30 data area:
Offset  Size    Description     (Table 01386)
 00h    BYTE    data structure minor version number (02h in TAME 2.30)
 01h    BYTE    data structure major version number (0Ah in TAME 2.30)
 02h    DWORD   number of task switches
 06h    DWORD   number of keyboard polls
 0Ah    DWORD   number of time polls
 0Eh    DWORD   number of times DESQview told program runs only in foreground
 12h    DWORD   time of last /CLEAR or TAME-RES load
 16h    DWORD   time yielded
 1Ah    DWORD   time spent polling
 1Eh    DWORD   time spent waiting on key input with INT 16/AH=01h,11h
 22h    DWORD   original INT 10h
 26h    DWORD   original INT 14h
 2Ah    DWORD   original INT 15h
 2Eh    DWORD   original INT 16h
 32h    DWORD   original INT 17h
 36h    DWORD   original INT 21h
 3Ah    DWORD   original INT 28h
 3Eh    WORD    offset of TAME INT 10h handler
 40h    WORD    offset of TAME INT 14h handler
 42h    WORD    offset of TAME INT 15h handler
 44h    WORD    offset of TAME INT 16h handler
 46h    WORD    offset of TAME INT 17h handler
 48h    WORD    offset of TAME INT 21h handler
 4Ah    WORD    offset of TAME INT 28h handler
 4Ch    WORD    X in /max:X,Y or /freq:X,Y
 4Eh    WORD    Y in /max:X,Y or /freq:X,Y
 50h    WORD    number of polls remaining before next task switch
 52h    WORD    /KEYIDLE value
 54h    WORD    /FG: value
 56h    WORD    task switches left until next FGONLY DESQview API call
 58h    WORD    multitasker version
 5Ah    WORD    virtual screen segment
 5Ch    BYTE    interrupts already grabbed by TAME (see #01383)
 5Dh    BYTE    flags for interrupts which may be acted on (same bits as above)
 5Eh    BYTE    window or task number for this task
 5Fh    BYTE    multitasker type (see #01384)
 60h    BYTE    type of task switching selected (bit flags) (see #01385)
 61h    BYTE    watch_DOS
 62h    BYTE    action flags (see #01387)
 63h    BYTE    old status
 64h    WORD    signature DA34h

Bitfields for TAME action flags:
Bit(s)  Description     (Table 01387)
 0      TAME enabled
 1      /FREQ instead of /MAX (X and Y count fields are per tick)
 2      /TIMEPOLL
 3      /KEYPOLL
 4      inhibit timer
 5      enable status monitoring
SeeAlso: #01386,#01388

Format of TAME 2.60 data area:
Offset  Size    Description     (Table 01388)
 00h    BYTE    data structure minor version number (02h in TAME 2.60)
 01h    BYTE    data structure major version number (0Bh in TAME 2.60)
 02h    DWORD   number of task switches
 06h    DWORD   number of keyboard polls
 0Ah    DWORD   number of time polls
 0Eh    DWORD   number of times DESQview told program runs only in foreground
 12h    DWORD   time of last /CLEAR or TAME-RES load
 16h    DWORD   time yielded
 1Ah    DWORD   time spent polling
 1Eh    DWORD   time spent waiting on key input with INT 16/AH=01h,11h
 22h  4 BYTEs   ???
 26h    DWORD   original INT 10h
 2Ah    DWORD   original INT 14h
 2Eh    DWORD   original INT 15h
 32h    DWORD   original INT 16h
 36h    DWORD   original INT 17h
 3Ah    DWORD   original INT 21h
 3Eh    DWORD   original INT 28h
 42h    WORD    offset of TAME INT 10h handler
 44h    WORD    offset of TAME INT 14h handler
 46h    WORD    offset of TAME INT 15h handler
 48h    WORD    offset of TAME INT 16h handler
 4Ah    WORD    offset of TAME INT 17h handler
 4Ch    WORD    offset of TAME INT 21h handler
 4Eh    WORD    offset of TAME INT 28h handler
 50h    WORD    X in /max:X,Y or /freq:X,Y
 52h    WORD    Y in /max:X,Y or /freq:X,Y
 54h    WORD    number of polls remaining before next task switch
 56h    WORD    /KEYIDLE value
 58h  4 BYTEs   ???
 5Ch    WORD    X in /boost:X,Y
 5Eh    WORD    Y in /boost:X,Y
 60h    WORD    /FG: value
 62h    WORD    task switches remaining until next FGONLY DESQview API call
 64h    WORD    multitasker version ???
 66h    WORD    virtual screen segment
 68h    BYTE    interrupts already grabbed by TAME (see #01383)
 69h    BYTE    flags for interrupts which may be acted on (same bits as above)
 6Ah    BYTE    window or task number for this task
 6Bh    BYTE    multitasker type (see #01384)
 6Ch    BYTE    type of task switching selected (bit flags) (see #01385)
 6Dh    BYTE    watch_DOS
 6Eh    BYTE    action flags (see #01387)
 6Fh    BYTE    old status
 70h    WORD    signature DA34h
--------R-212B44BX4D41-----------------------
INT 21 - pcANYWHERE IV/LAN - INSTALLATION CHECK
        AX = 2B44h ('D')
        BX = 4D41h ('MA')
        CX = 7063h ('pc')
        DX = 4157h ('AW')
Return: AX = resident program
            4F4Bh ('OK') if large host resident
            6F6Bh ('ok') if small host resident
        CX:DX -> API entry point (see #01389)
SeeAlso: INT 16/AH=79h

(Table 01389)
Call pcANYWHERE API entry point with:
        AX = 0000h get pcANYWHERE IV version
            DS:SI -> BYTE buffer for host type code
            Return: AH = version number
                    AL = revision number
                    DS:DI buffer byte filled with
                        00h full-featured host
                        01h limited-feature LAN host
                        other API may not be supported
        AX = 0001h initialize operation
            DS:SI -> initialization request structure (see #01390)
            Return: AX = function status (see #01393)
        AX = 0002h get status
            Return: AH = current operating mode (see #01390)
                    AL = current connection status (see #01392)
        AX = 0003h suspend remote screen updates
            Return: AX = function status (see #01393)
        AX = 0004h resume screen updates
            Return: AX = function status (see #01393)
        AX = 0005h end current remote access session
            DS:SI -> termination request structure (see #01391)
            Return: AX = function status (see #01393)
        AX = 0006h remove pcANYWHERE IV from memory
            Return: AX = status (see #01393)
        AX = 8000h read data from communications channel
            DS:BX -> buffer
            CX = buffer size
            Return: AX >= number of characters read/available
                    AX < 0 on error
        AX = 8001h write data to communications channel
            DS:BX -> buffer
            CX = buffer size
            Return: AX >= number of characters written
                    AX < 0 on error
        AX = 8002h get connection status
            Return: AX = status
                        > 0000h if connection active
                        = 0000h if connection lost
                        < 0000h on error

Format of pcANYWHERE initialization request structure:
Offset  Size    Description     (Table 01390)
 00h    BYTE    operating mode
                00h wait for a call
                01h hot key activates
                02h incoming call activates
                03h initiate a call
 01h  3 BYTEs   user ID to append to config file names
 04h    WORD    DS-relative pointer to path for config files
 06h    WORD    DS-relative pointer to path for program files

Format of pcANYWHERE termination request structure:
Offset  Size    Description     (Table 01391)
 00h    BYTE    operating mode after termination
                00h wait for a call
                01h hot key activates
                02h incoming call activates
                80h use current mode
                FFh remove from memory

Bitfields for current connection status:
Bit(s)  Description     (Table 01392)
 0      a physical connection is active
 1      remove screen updating is active
 2      connection checking is active
 3      hot key detection is active
 4      background file transfer is active

(Table 01393)
Values for pcANYWHERE function status:
 0000h  function completed successfully
 FFD1h  unable to release interrupt vectors
 FFD2h  unable to release allocated memory
 FFF2h  unable to establish a connection when operating mode is
          "Initiate a call"
 FFF3h  modem configuration is invalid (corrupt config)
 FFF4h  modem initialization failed (no modem response)
 FFF5h  the communications device could not be initialized
 FFF6h  the host operator aborted the function
 FFF7h  the communications driver type specified in the configuration file is
          different than the one loaded when pcANYWHERE IV was started
 FFF9h  the configuration file is invalid
 FFFAh  the configuration file could not be found
 FFFBh  no session is active
 FFFCh  a remote access session is active
 FFFDh  the specified operating mode is invalid
--------l-212B--CX5643-----------------------
INT 21 - Volkov Commander - INSTALLATION CHECK
        AH = 2Bh
        CX = 5643h ('VC')
        DX = 4F4Dh ('OM')
        AL = function number
            00h get left window data address
            01h get right window data address
            02h get address of general variables
Return: AL = 00h if Volkov Commander installed
            AH = version code (27h for v4.00.039)
            ES:BX -> requested data
Program: Volkov Commander is a Norton Commander-like DOS shell
--------G-212B--CX6269-----------------------
INT 21 - WDTSR.COM - INSTALLATION CHECK
        AH = 2Bh
        CX = 6269h ('bi')
        DX = 742Dh ('t-')
Return: AL = FFh if not installed
        AL = 77h ('w') if WDTSR is installed
            CX = 6174h ('at')
            DX = 6368h ('ch')
            ES = resident code segment
            ES:DI -> identification and configuration data
Program: WDTSR is a driver for the bitWatch watchdog hardware by bit-design
          GmbH
SeeAlso: AH=2Bh/CX=6269h"bitFOSSI",INT 14/AH=14h"FOSSIL",INT 15/AH=C3h
--------S-212B--CX6269-----------------------
INT 21 - bitFOSS - INSTALLATION CHECK
        AH = 2Bh
        CX = 6269h ('bi')
        DX = 7446h ('tF')
Return: AL = FFh if not installed
        AL = 4Fh ('O') if bitFOSS is installed
            CX = 5353h ('SS')
            DX = 494Ch ('IL')
            ES = resident code segment
            ES:DI -> identification data
Program: bitFOSS is a revision 5 FOSSIL driver
SeeAlso: AH=2Bh/CX=6269h"bitFOSSI",INT 11/AH=BCh
--------S-212B--CX6269-----------------------
INT 21 - bitFOSSI - INSTALLATION CHECK
        AH = 2Bh
        CX = 6269h ('bi')
        DX = 7449h ('tI')
Return: AL = FFh if not installed
        AL = 53h ('S') if bitFOSSI is installed
            CX = 444Eh ('DN')
            DX = 2D46h ('-F')
            ES = resident code segment
            ES:DI -> identification data
Program: bitFOSSI is a revision 5 FOSSIL driver for ???'s ISDN board
SeeAlso: AH=2Bh/CX=6269h"bitFOSS",INT 11/AH=BCh
--------v-212C00CX534B-----------------------
INT 21 - SKUDO - INSTALLATION CHECK
        AX = 2C00h
        CX = 534Bh ('SK')
        DX = 5544h ('UD')
        BX = 4F21h ('O!')
Return: AX = 5349h ('SI') if installed
            CH = version number
            CL = subversion
Program: SKUDO is an antivirus TSR by Jordi Mas
--------T-212D01CX7820-----------------------
INT 21 - PC-Mix - INSTALLATION CHECK
        AX = 2D01h
        CX = 7820h ('X ')
        DX = 6D69h ('MI')
Return: AL = 00h if installed
--------E-2130-------------------------------
INT 21 - Phar Lap 386/DOS-Extender, Intel Code Builder - INSTALLATION CHECK
        AH = 30h
        EAX = 00003000h
        EBX = 50484152h ("PHAR")
Return: AL = major DOS version
        AH = minor DOS version
        EAX bits 31-16 = 4458h ('DX') if 386/DOS-extender installed
            BL = ASCII major version number
        EAX bits 31-16 = 4243h ('BC') if Intel Code Builder installed
            EDX = address of GDA
SeeAlso: AX=2501h,AX=FF00h,INT 2F/AX=F100h
--------T-213000BX1234-----------------------
INT 21 - CTask 2.0+ - INSTALLATION CHECK
        AX = 3000h
        BX = 1234h
        DS:DX -> 8-byte version string (DX < FFF0h) "CTask21",00h for v2.1-2.2
Return: AL = DOS major version
        AH = DOS minor version
        CX:BX -> Ctask global data block
Program: CTask is a multitasking kernel for C written by Thomas Wagner
Note:   if first eight bytes of returned data block equal eight bytes passed
          in, CTask is resident
--------O-213000BX3000-----------------------
INT 21 - PC-MOS/386 v3.0 - INSTALLATION CHECK/GET VERSION
        AX = 3000h
        BX = 3000h
        CX = DX = 3000h
Return: AX = PC-MOS version
Program: PC-MOS/386 is a multitasking/multiuser MS-DOS-compatible operating
          system by The Software Link, Inc.
SeeAlso: AH=30h,INT D4/AH=02h,INT D4/AH=10h
--------G-213022-----------------------------
INT 21 - StopPrg v2.0 - INSTALLATION CHECK
        AX = 3022h
Return: AX = DOS version (see AH=30h)
        CX = 1112h if StopPrg installed
            BX = segment of resident code
Program: StopPrg is a resident program aborter by MAK-TRAXON's Prophet
Note:   StopPrg may be temporarily disabled by storing 9090h in the word at
          0000h:04FEh
----------2130FFCX4445-----------------------
INT 21 - DESQ??? - INSTALLATION CHECK
        AX = 30FFh
        CX = 4445h ("DE")
        DX = 5351h ("SQ")
Return: BH = 05h if installed
        ???
Note:   called by DUBLDISK.COM v2.6; this function is not supported by
          DESQview, so it may be for DESQview's precursor DESQ.
SeeAlso: AX=4404h"DUBLDISK"
--------k-213341-----------------------------
INT 21 - Diet Disk v1.0 - INSTALLATION CHECK
        AX = 3341h
Return: DX = 1234h if installed
            CX = resident code segment
Program: Diet Disk is a public domain transparent data file compressor by
          Barry Nance
--------E-213501-----------------------------
INT 21 P - FlashTek X-32VM - ALLOCATE PROTECTED-MODE SELECTOR
        AX = 3501h
Return: CF clear if successful
            BX = new selector
        CF set on error (no more selectors available)
Note:   the new selector will be an expand-up read/write data selector with
          undefined base and limit
SeeAlso: AX=3502h,INT 31/AX=0000h
--------E-213502-----------------------------
INT 21 P - FlashTek X-32VM - DEALLOCATE PROTECTED-MODE SELECTOR
        AX = 3502h
        BX = selector
Return: CF clear if successful
        CF set on error (invalid selector)
Note:   only selectors allocated via AX=3501h should be deallocated
SeeAlso: AX=3501h,INT 31/AX=0001h
--------E-213503-----------------------------
INT 21 P - FlashTek X-32VM - SET SELECTOR BASE ADDRESS
        AX = 3503h
        BX = selector
        ECX = base address
Return: CF clear if successful
        CF set on error (invalid selector)
SeeAlso: AX=3504h,AX=3505h,INT 31/AX=0007h
--------E-213504-----------------------------
INT 21 P - FlashTek X-32VM - GET SELECTOR BASE ADDRESS
        AX = 3504h
        BX = selector
Return: CF clear if successful
            ECX = absolute base address of selector
        CF set on error (invalid selector)
SeeAlso: AX=3503h,INT 31/AX=0006h
--------E-213505-----------------------------
INT 21 P - FlashTek X-32VM - SET SELECTOR LIMIT
        AX = 3505h
        BX = selector
        ECX = desired limit
Return: CF clear if successful
            ECX = actual limit set
        CF set on error (no more selectors available)
Note:   the limit will be rounded down to nearest 4K boundary if the requested
          limit is greater than 1MB
SeeAlso: AX=3503h,INT 31/AX=0008h
--------E-21350A-----------------------------
INT 21 P - FlashTek X-32VM - PHYSICAL ADDRESS MAPPING
        AX = 350Ah
        EBX = absolute physical address
        ECX = size in bytes of area to map
Return: CF clear if successful
        CF set on error (insufficient memory or service refused by DPMI host)
Notes:  should not make repeated calls for the same physical address
        there is no provision for unmapping memory
--------E-21350B-----------------------------
INT 21 P - FlashTek X-32VM - UPDATE AND RETURN AVAILABLE FREE MEMORY
        AX = 350Bh
        DS = default selector for DS
Return: CF clear
        EAX = maximum amount of memory which can be allocated via AX=350Ch
SeeAlso: AX=350Ch
--------E-21350C-----------------------------
INT 21 P - FlashTek X-32VM - ALLOCATE A BLOCK OF MEMORY
        AX = 350Ch
        ECX = size of block in bytes
        DS = default DS
Return: CF clear if successful
            EAX = near pointer to new block
            EDX = new lowest legal value for stack
        CF set on error (requested size not multiple of 4K)
SeeAlso: AX=350Bh,AX=350Dh
--------E-21350D-----------------------------
INT 21 P - FlashTek X-32VM - RESERVE BLOCK OF MEMORY FOR 32-BIT STACK
        AX = 350Dh
        EBX = current ESP value
        ECX = size of block in bytes
        DS = default DS
Return: CF clear if successful
            EBX = new value for ESP
            EDX = suggested new limit for SS
        CF set on error
Note:   this function should only be called once during initialization
SeeAlso: AX=350Bh,AX=350Ch
--------k-2137A0BX6A6D-----------------------
INT 21 - XPACK v1.52+ - TSR INSTALLATION CHECK
        AX = 37A0h
        BX = 6A6Dh ('jm')
Return: AL = FFh if not present as TSR (default return value from DOS)
        AX = 0000h if installed as a TSR
            CX = 6A6Dh ('jm')
            DX = version ID (0152h)
Program: XPACK is a transparent file compressor/decompressor by JauMing Tseng
SeeAlso: AX=37A1h/BX=6A6Dh,AX=37A3h/BX=6A6Dh
--------k-2137A1BX6A6D-----------------------
INT 21 - XPACK v1.52+ - UNINSTALL
        AX = 37A1h
        BX = 6A6Dh ('jm')
Return: AX = status
            0000h successful
            FFFFh failed
Program: XPACK is a transparent file compressor/decompressor by JauMing Tseng
SeeAlso: AX=37A0h/BX=6A6Dh
--------k-2137A2BX6A6D-----------------------
INT 21 - XPACK v1.52+ - GET TSR STATUS
        AX = 37A2h
        BX = 6A6Dh ('jm')
Return: AX = 0000h (successful)
        DL = status (01h active, 00h disabled)
SeeAlso: AX=37A3h/BX=6A6Dh
--------k-2137A3BX6A6D-----------------------
INT 21 - XPACK v1.52+ - SET TSR STATUS
        AX = 37A3h
        BX = 6A6Dh ('jm')
        DL = status (01h active, 00h disabled)
Return: AX = 0000h (successful)
SeeAlso: AX=37A0h/BX=6A6Dh,AX=37A2h/BX=6A6Dh
----------2137A6BX6A6D-----------------------
INT 21 - XPACK v1.65 - GET TEMPORARY DIRECTORY NAME
        AX = 37A6h
        BX = 6A6Dh ('jm')
Return: AX = 0000h
        DS:DX -> name of temporary directory
Program: XPACK is a transparent file compressor/decompressor by JauMing Tseng
SeeAlso: AX=37A0h/BX=6A6Dh,AX=37A7h/BX=6A6Dh
----------2137A7BX6A6D-----------------------
INT 21 - XPACK v1.65 - SET TEMPORARY DIRECTORY NAME
        AX = 37A7h
        BX = 6A6Dh ('jm')
        DS:DX -> ASCIZ name of temporary directory (max 64 chars)
Return: AX = 0000h
Note:   the specified directory name must include a drive letter and end with
          a backslash (e.g. 'c:\dos\',0)
SeeAlso: AX=37A0h/BX=6A6Dh,AX=37A6h/BX=6A6Dh
--------k-2137D0BX899D-----------------------
INT 21 - DIET v1.43e - TSR INSTALLATION CHECK
        AX = 37D0h
        BX = 899Dh ('DI' + 'ET')
Return: AL = FFh if not present as TSR (default return value from DOS)
        AX = 0000h if installed as a TSR
            CX = 899Dh
            DX = version ID
Program: DIET is a transparent file compressor/decompressor by Teddy Matsumoto
SeeAlso: AX=37D1h,AX=37D2h,AX=37D4h,AX=37D6h,AX=37DFh,AX=4BF0h,AX=4BF1h
--------k-2137D1BX899D-----------------------
INT 21 - DIET v1.43e - GET DIET.EXE RESIDENT SEGMENT
        AX = 37D1h
        BX = 899Dh ('DI' + 'ET')
Return: AX = 0000h
        CX = code segment of TSR part of DIET.EXE
        DX = memory block segment of TSR DIET.EXE
                (0000h if installed as device driver)
SeeAlso: AX=37D0h,AX=37DFh
--------k-2137D2BX899D-----------------------
INT 21 - DIET v1.43e - GET TSR CONTROL FLAGS
        AX = 37D2h
        BX = 899Dh ('DI' + 'ET')
Return: AX = 0000h
        DL = control flag (00h active, else disabled)
        DH = skip flag (nonzero while TSR active)
SeeAlso: AX=37D0h,AX=37D3h,AX=37D4h
--------k-2137D3BX899D-----------------------
INT 21 - DIET v1.43e - SET TSR CONTROL FLAGS
        AX = 37D3h
        BX = 899Dh ('DI' + 'ET')
        DL = control flag (00h active, else disabled)
        DH = skip flag (00h)
Return: AX = 0000h
SeeAlso: AX=37D0h,AX=37D2h,AX=37D5h
--------k-2137D4BX899D-----------------------
INT 21 - DIET v1.43e - GET TSR OPTIONS
        AX = 37D4h
        BX = 899Dh ('DI' + 'ET')
Return: AX = 0000h
        DX = TSR options (see #01396)
SeeAlso: AX=37D0h,AX=37D2h,AX=37D5h

Bitfields for DIET TSR options:
Bit(s)  Description     (Table 01396)
 0      automated compression of DIETed file
 1      automated compression of newly-created file
 2      suppress DIET message
 3      display original file size
 4-15   reserved (0)
--------k-2137D5BX899D-----------------------
INT 21 - DIET v1.43e - SET TSR OPTIONS
        AX = 37D5h
        BX = 899Dh ('DI' + 'ET')
        DX = TSR options (see #01396)
Return: AX = 0000h
Program: DIET is a transparent file compressor/decompressor by Teddy Matsumoto
SeeAlso: AX=37D0h,AX=37D3h,AX=37D4h
--------k-2137D6BX899D-----------------------
INT 21 - DIET v1.43e - GET TEMPORARY DIRECTORY NAMES
        AX = 37D6h
        BX = 899Dh ('DI' + 'ET')
Return: AX = 0000h
        DS:DX -> name of temporary directory or 0000h:0000h for current dir
SeeAlso: AX=37D0h,AX=37D7h
--------k-2137D7BX899D-----------------------
INT 21 - DIET v1.43e - SET TEMPORARY DIRECTORY NAMES
        AX = 37D7h
        BX = 899Dh ('DI' + 'ET')
        DS:DX -> ASCIZ name of temporary directory (max 61 chars)
                0000h:0000h for current directory
Return: AX = 0000h
Note:   the specified directory name must include a drive letter and end with
          a backslash
SeeAlso: AX=37D0h,AX=37D6h
--------k-2137DCBX899D-----------------------
INT 21 - DIET v1.43e - SET ADDRESS OF EXTERNAL PROCEDURE
        AX = 37DCh
        BX = 899Dh ('DI' + 'ET')
        DS:DX -> external procedure (see #01397)
Return: AX = 0000h
Note:   the resident code will call the specified external procedure at the
          beginning of decompression and when compression is exited on failure
SeeAlso: AX=37DDh

(Table 01397)
Values DIET external procedure is called with:
        STACK:  WORD    class
                        FFFDh creation failed for unknown reasons
                        FFFEh creation failed due to lack of space
                        FFFFh file creation error
                        else file handle of DIETed file to be decompressed
                DWORD   -> compressed filename
                DWORD   -> decompressed or temporary filename
Return: SI,DI,BP,DS,ES must be preserved by external procedure
--------k-2137DDBX899D-----------------------
INT 21 - DIET v1.43e - RELEASE EXTERNAL PROCEDURE
        AX = 37DDh
        BX = 899Dh ('DI' + 'ET')
Program: DIET is a transparent file compressor/decompressor by Teddy Matsumoto
Note:   unlinks the external procedure specified by AX=37DCh
SeeAlso: AX=37DCh
--------k-2137DEBX899D-----------------------
INT 21 - DIET v1.43e - READ EMS STATUS
        AX = 37DEh
        BX = 899Dh ('DI' + 'ET')
Return: AX = 0000h
        CX = EMS status
            0000h not used
            0001h used as work area
            0002h used for code and as work area
        DX = EMM handle when EMS is in use
--------k-2137DFBX899D-----------------------
INT 21 - DIET v1.43e - UNINSTALL TSR
        AX = 37DFh
        BX = 899Dh ('DI' + 'ET')
Return: AX = status
            0000h successful
            00FFh failed
Program: DIET is a transparent file compressor/decompressor by Teddy Matsumoto
SeeAlso: AX=37D0h
Index:  uninstall;DIET
--------G-213F-------------------------------
INT 21 - Turbo Debug HARDWARE BREAKPOINTS - READ STATUS BLOCK
        AH = 3Fh
        BX = handle for character device "TDHDEBUG"
        CX = number of bytes to read
        DS:DX -> buffer for status block (see #01406)
Return: CF clear if successful
            AX = number of bytes actually read
        CF set on error
            AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h)
SeeAlso: AH=40h"Turbo Debug"

(Table 01405)
Values for status of Turbo Debugger command:
 00h    successful
 01h    invalid handle
 02h    no more breakpoints available
 03h    hardware does not support specified breakpoint type
 04h    previous command prevents execution
 05h    debugger hardware not found
 06h    hardware failure
 07h    invalid command
 08h    driver not initialized yet
 FEh    recursive entry (hardware breakpoint inside hw bp handler)

Format of Turbo Debugger status block:
Offset  Size    Description     (Table 01406)
 00h    BYTE    status of command (see #01405)
---status for command 01h---
 01h    WORD    device driver interface version number (currently 1)
 03h    WORD    device driver software version
 05h    BYTE    maximum simultaneous hardware breakpoints
 06h    BYTE    configuration bits (see #01407)
 07h    BYTE    supported breakpoint types (see #01408)
 08h    WORD    supported addressing match modes (see #01409)
 0Ah    WORD    supported data matches (see #01410)
 0Ch    BYTE    maximum data match length (01h, 02h, or 04h)
 0Dh    WORD    size of onboard memory (in KB)
 0Fh    WORD    maximum number of trace-back events
 11h    WORD    hardware breakpoint enable byte address segment (0000h if not
                  supported)
---status for command 04h---
 01h    BYTE    handle to use when referring to the just-set breakpoint

Bitfields for Turbo Debugger configuration bits:
Bit(s)  Description     (Table 01407)
 0      CPU and DMA accesses are distinct
 1      can detect DMA transfers
 2      supports data mask
 3      hardware pass counter on breakpoints
 4      can match on data as well as addresses

Bitfields for Turbo Debugger supported breakpoint types:
Bit(s)  Description     (Table 01408)
 0      memory read
 1      memory write
 2      memory read/write
 3      I/O read
 4      I/O write
 5      I/O read/write
 6      instruction fetch

Bitfields for Turbo Debugger supported addressing match modes:
Bit(s)  Description     (Table 01409)
 0      any address
 1      equal to test value
 2      not equal
 3      above test value
 4      below test value
 5      below or equal
 6      above or equal
 7      within range
 8      outside range

Bitfields for Turbo Debugger supported data matches:
Bit(s)  Description     (Table 01410)
 0      any data
 1      equal to test value
 2      not equal
 3      above test value
 4      below test value
 5      below or equal
 6      above or equal
 7      within range
 8      outside range
--------N-213F-------------------------------
INT 21 - WORKGRP.SYS - GET ENTRY POINT
        AH = 3Fh
        BX = file handle for device "NET$HLP$"
        CX = 0008h
        DS:DX -> buffer for entry point record (see #01485)
Return: CF clear if successful
            AX = number of bytes actually read (0 if at EOF before call)
        CF set on error
            AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h)
Program: WORKGRP.SYS is the portion of Microsoft's Workgroup Connection which
          permits communication with PCs running Windows for Workgroups or
          LAN Manager
SeeAlso: AX=4402h"WORKGRP.SYS",INT 2F/AX=9400h
----------213F-------------------------------
INT 21 U - IFSHLP.SYS - GET ENTRY POINT
        AH = 3Fh
        BX = file handle for device "IFS$HLP$"
        CX = 0008h (size of buffer in bytes)
        DS:DX -> buffer for entry point record (see #01415)
Return: CF clear if successful
            AX = number of bytes actually read (0 if at EOF before call)
        CF set on error
            AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h)
Program: IFSHLP.SYS is a support driver for Microsoft Windows for Workgroups
SeeAlso: AX=4402h"IFSHLP"

Format of IFSHLP.SYS entry point record:
Offset  Size    Description     (Table 01415)
 00h  4 BYTEs   (call) required signature if called via IOCTL
                        70h E9h 34h 37h for Windows 3.11
                        70h E9h 35h 37h for Windows 3.11
                (ret) signature 34h 37h 70h EFh (Windows 3.11)
                (ret) signature 35h 37h 70h EFh (Windows95)
 04h    DWORD   (ret) pointer to FAR call entry point (see #01416)

(Table 01416)
Call IFSHLP.SYS entry point with:
        STACK:  WORD    function number (00h-0Ch)
                        00h get ??? data
                        01h set interrupt intercepts (trap)
                        02h remove interrupt intercepts (untrap)
                        03h ??? LPT2
                        04h ??? LPT1
                        05h revector INT 2F to trap and remove trap for others
                        06h set ??? flag
                        07h clear ??? flag
                        08h get ??? flag word
                        ---Windows 3.11 only---
                        09h ???
                        0Ah ???
                        0Bh ???
                        0Ch get ???
---if function 00h---
Return: DX:AX -> ??? data (see #01417)
        BX destroyed
---if function 01h---
        STACK:  DWORD   new intercept (trap) address
Return: AX = status
            0000h successful
            0001h failed (already set)
                DX = 0000h
        BX destroyed
Note:   the trap handler is called with a function number in BX, and the
          original BX on top of the stack; a null handler must perform a
          POP BX and an IRET
---if function 02h---
Return: AX = status
            0000h successful
            0001h failed (not set)
        DX = 0000h
        BX destroyed
---if function 03h,04h---
        STACK:  WORD    ???
Return: AX = 0000h
        DX = 0000h
        BX destroyed
---if function 05h---
        ???
Return: BX destroyed
---if function 06h---
Return: AX = 0001h and DX = 0000h if already set
        AX,DX unchanged if successful
        BX destroyed
---if function 07h---
Return: AX = 0001h and DX = 0000h if not set
        AX,DX unchanged if successful
        BX destroyed
---if function 08h---
Return: DX = 0000h
        AX = flags
            bit 0: set/cleared by functions 06h and 07h
            bit 1: trap is currently set (refer to functions 01h/02h)
        BX destroyed
---if function 09h---
Return: AX = status
            0000h successful
            0001h failed (already called)
        BX destroyed
---if function 0Ah---
        STACK:  WORD    ???
        ???
Return: BX destroyed
---if function 0Bh---
Return: AX = status
            0000h successful
            0001h failed (not set)
        BX destroyed
---if function 0Ch---
Return: AX = 0000h
        ES:BX -> ??? data
---if function > 0Ch---
Return: AX = 0001h
        DX = 0000h
        BX destroyed

Format of IFSHLP ??? data:
Offset  Size    Description     (Table 01417)
 00h    DWORD   -> DOS Swappable Data Area (see #01687,#01689)
 02h    WORD    ??? offset in DOS data segment?
 04h    WORD    offset in DOS data segment of current-PSP WORD
 06h    WORD    ???
 08h    WORD    ???
 0Ah    WORD    ???
 0Ch    DWORD   -> DOS List of Lists (see #01627)
 10h    ???
SeeAlso: #01416
--------G-2140-------------------------------
INT 21 - Turbo Debug HARDWARE BREAKPOINTS - SEND CMD TO HARDWARE BRKPNT DRIVER
        AH = 40h
        BX = handle for character device "TDHDEBUG"
        CX = number of bytes to write
        DS:DX -> hardware breakpoint command (see #01418)
Return: CF clear if successful
            AX = number of bytes actually written
        CF set on error
            AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h)
Note:   results are retrieved by reading from the device
SeeAlso: AH=3Fh"Turbo Debug"

Format of Turbo Debugger hardware breakpoint commands:
Offset  Size    Description     (Table 01418)
 00h    BYTE    command code
                00h install interrupt vectors
                01h get hardware capabilities
                02h enable hardware breakpoints
                03h disable hardware breakpoints
                04h set hardware breakpoint
                05h clear hardware breakpoint
                06h set I/O base address and reset hardware
                07h restore interrupt vectors
---command code 00h---
 01h    DWORD   pointer to Turbo Debugger entry point to be jumped to on
                  hardware breakpoint; call with CPU state the same as on
                  the breakpoint except for pushing AX and placing an entry
                  code (FFh if breakout button or breakpoint handle) in AH
---command code 04h---
 01h    BYTE    breakpoint type
                00h memory read
                01h memory write
                02h memory read/write
                03h I/O read
                04h I/O write
                05h I/O read/write
                06h instruction fetch
 02h    BYTE    address matching mode (see #01419)
 03h    DWORD   32-bit linear low address
 07h    DWORD   32-bit linear high address
 0Bh    WORD    pass count
 0Dh    BYTE    data size (01h, 02h, or 04h)
 0Eh    BYTE    source of matched bus cycle (01h CPU, 02h DMA, 03h either)
 0Fh    BYTE    data-matching mode (see #01419)
 10h    DWORD   low data value
 14h    DWORD   high data value
 18h    DWORD   data mask specifying which bits of the data are tested
---command code 05h---
 01h    BYTE    handle of breakpoint to clear (breakpoint returned from command
                  04h)
---command code 06h---
 01h    WORD    base address of hardware debugger board

(Table 01419)
Values for Turbo Debugger address/data matching mode:
 00h    match any
 01h    equal to test value
 02h    different from test value
 03h    above test value
 04h    below test value
 05h    below or equal to test value
 06h    above or equal to test value
 07h    within inclusive range
 08h    outside specified range
--------O-214302-----------------------------
INT 21 - DR DOS 3.41+ internal - GET ACCESS RIGHTS
        AX = 4302h
        DS:DX -> ASCIZ pathname
Return: CF clear if successful
            CX = access rights (see #01421)
            AX = CX (DR DOS 5.0)
        CF set on error
            AX = error code
Desc:   Determine which operations the calling program may perform on a
          specified file without being required to provide a password.
Notes:  this protection scheme has been coordinated on all current Digital
          Research/Novell operating systems (DR DOS 3.41+, DRMDOS 5.x, and
          FlexOS 2+)
        this function is documented in DR DOS 6.0 and corresponds to the
          "Get/Set File Attributes" function, subfunction 2, documented in
          Concurrent DOS.
        only FlexOS actually uses the "execution" bits; DR DOS 3.41+ treats
          them as "read" bits.
        DR DOS 3.41-5.x only use bits 0-3.  Only DR DOS 6.0 using a
          DRMDOS 5.x security system allowing for users and groups uses bits
          4-11.
SeeAlso: AX=4303h,#01354 at INT 21/AH=11h

Bitfields for DR DOS file access rights:
Bit(s)  Description     (Table 01421)
 0      owner delete requires password
 1      owner execution requires password (FlexOS)
 2      owner write requires password
 3      owner read requires password
 4      group delete requires password
 5      group execution requires password (FlexOS)
 6      group write requires password
 7      group read requires password
 8      world delete requires password
 9      world execution requires password (FlexOS)
 10     world write requires password
 11     world read requires password
--------O-214303-----------------------------
INT 21 - DR DOS 3.41+ internal - SET ACCESS RIGHTS AND PASSWORD
        AX = 4303h
        CX = access rights
             bits 11-0: access rights (see #01421)
             bit 15: new password is to be set
        DS:DX -> ASCIZ pathname
        [DTA] = new password if CX bit 15 is set (blank-padded to 8 characters)
Return: CF clear if successful
        CF set on error
            AX = error code
Notes:  if the file is already protected, the old password must be added after
          the pathname, separated by a ";".  As with all password handling in
          DR DOS, the old password is processed by the kernel, not any
          applications or utilities making the call.
        this function is documented in DR DOS 6.0 and corresponds to the
          "Get/Set File Attributes" function, subfunction 3, documented in
          Concurrent DOS.
SeeAlso: AH=0Fh,AH=17h,AX=4302h"DR DOS",AX=4305h,AX=4454h
--------O-214304-----------------------------
INT 21 U - DR DOS 5.0-6.0 internal - GET ENCRYPTED PASSWORD
        AX = 4304h
        DS:DX -> ASCIZ filename
        ???
Return: CF clear if successful
            CX = AX = 0000h if no password assigned to file
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
Note:   this function is only supported by DR DOS 5.0 and 6.0 and DRMDOS 5.1
SeeAlso: AX=4303h,AX=4305h
--------O-214305-----------------------------
INT 21 U - DR DOS 5.0-6.0 internal - SET EXTENDED FILE ATTRIBUTES
        AX = 4305h
        DS:DX -> ASCIZ filename
        ???
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
Desc:   this function allows the extended attributes, and optionally the
          encrypted password, of a file to be set.
Note:   this function is only supported by DR DOS 5.0 and 6.0 and DRMDOS 5.1
SeeAlso: AX=4304h,AX=4311h
--------O-214306-----------------------------
INT 21 O - DR DOS 6.0 - GET FILE OWNER
        AX = 4306h
        DS:DX -> ASCIZ filename
Return: CF clear if successful
            AX = CX = value set with AX=4307h
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
Note:   no longer supported by Novell DOS 7 / OpenDOS
SeeAlso: AX=4307h
--------O-214307-----------------------------
INT 21 O - DR DOS 6.0 - SET FILE OWNER
        AX = 4307h
        CX = ??? (owner identification number?)
        DS:DX -> ASCIZ filename
Return: CF clear if successful
        CF set on error
            AX = error code (see #01680 at AH=59h/BX=0000h)
Note:   no longer supported by Novell DOS 7 / OpenDOS
SeeAlso: AX=4306h
--------O-214380-----------------------------
INT 21 - Novell DOS 7 - UNDELETE PENDING DELETE FILE
        AX = 4380h
        ???
Return: ???
SeeAlso: AH=41h,AX=4381h
--------O-214381-----------------------------
INT 21 - Novell DOS 7 - PURGE PENDING DELETE FILE
        AX = 4381h
        ???
Return: ???
SeeAlso: AH=41h,AX=4380h
--------O-214382-----------------------------
INT 21 - DR-DOS >7.03 - RESERVED
        AX = 4382h-438Fh
Notes:  These previously unused functions are reserved for future versions of
          DR-DOS (post 7.03).
        AX=4382h will be identical to the previous AX=4302h, which is now also
          used by MS-DOS 7+ for a different purpose.
SeeAlso: INT 21h/4302h
--------!---Section--------------------------
