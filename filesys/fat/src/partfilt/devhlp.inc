;       SCCSID = @(#)devhlp.inc 6.10 91/11/13
; ****************************************************************************
; *                                                                          *
; *                       IBM/Microsoft Confidential                         *
; *                                                                          *
; *                 Copyright (c) IBM Corporation  1987, 1990                *
; *                 Copyright (c) Microsoft Corp.  1987, 1990                *
; *                           All Rights Reserved                            *
; *                                                                          *
; ****************************************************************************

SUBTTL  DevHlp - Definitions for device driver helper functions


DevHlp_SchedClock       EQU      0      ;  0    Called each timer tick
DevHlp_DevDone          EQU      1      ;  1    Device I/O complete
DevHlp_Yield            EQU      2      ;  2    yield CPU if resched set
DevHlp_TCYield          EQU      3      ;  3    yield to time critical task

DevHlp_ProcBlock        EQU      4      ;  4    Block on event
DevHlp_ProcRun          EQU      5      ;  5    Unblock process

DevHlp_SemRequest       EQU      6      ;  6    claim a semaphore
DevHlp_SemClear         EQU      7      ;  7    release a semaphore
DevHlp_SemHandle        EQU      8      ;  8    obtain a semaphore handle

DevHlp_PushRequest      EQU      9      ;  9    Push the request
DevHlp_PullRequest      EQU     10      ;  A    Pull next request from Q
DevHlp_PullParticular   EQU     11      ;  B    Pull a specific request
DevHlp_SortRequest      EQU     12      ;  C    Push request in sorted order

DevHlp_AllocReqPacket   EQU     13      ;  D    allocate request packet
DevHlp_FreeReqPacket    EQU     14      ;  E    free request packet

DevHlp_QueueInit        EQU     15      ;  F    Init/Clear char queue
DevHlp_QueueFlush       EQU     16      ; 10    flush queue
DevHlp_QueueWrite       EQU     17      ; 11    Put a char in the queue
DevHlp_QueueRead        EQU     18      ; 12    Get a char from the queue

DevHlp_Lock             EQU     19      ; 13    Lock segment
DevHlp_Unlock           EQU     20      ; 14    Unlock segment

DevHlp_PhysToVirt       EQU     21      ; 15    convert physical address to virtual
DevHlp_VirtToPhys       EQU     22      ; 16    convert virtual address to physical
DevHlp_PhysToUVirt      EQU     23      ; 17    convert physical to LDT

DevHlp_AllocPhys        EQU     24      ; 18    allocate physical memory
DevHlp_FreePhys         EQU     25      ; 19    free physical memory

DevHlp_SetROMVector     EQU     26      ; 1A    set a ROM service routine vector
DevHlp_SetIRQ           EQU     27      ; 1B    set an IRQ interrupt
DevHlp_UnSetIRQ         EQU     28      ; 1C    unset an IRQ interrupt

DevHlp_SetTimer         EQU     29      ; 1D    set timer request handler
DevHlp_ResetTimer       EQU     30      ; 1E    unset timer request handler

DevHlp_MonitorCreate    EQU     31      ; 1F    create a monitor
DevHlp_Register         EQU     32      ; 20    install a monitor
DevHlp_DeRegister       EQU     33      ; 21    remove a monitor
DevHlp_MonWrite         EQU     34      ; 22    pass data records to monitor
DevHlp_MonFlush         EQU     35      ; 23    remove all data from stream

DevHlp_GetDOSVar        EQU     36      ; 24    Return pointer to DOS variable

; Dos variable indexes used in DevHlp_GetDOSVar
DHGETDOSV_SYSINFOSEG    EQU     1
DHGETDOSV_LOCINFOSEG    EQU     2
DHGETDOSV_VECTORSDF     EQU     4
DHGETDOSV_VECTORREBOOT  EQU     5
DHGETDOSV_VECTORMSATS   EQU     6
DHGETDOSV_INTERRUPTLEV  EQU     13      ; Interrupt level
DHGETDOSV_DEVICECLASSTABLE EQU     14   ; Device Class Table
DHGETDOSV_DMQSSEL       EQU     15      ; DMQS selector

DevHlp_SendEvent        EQU     37      ; 25    an event occurred
DevHlp_ROMCritSection   EQU     38      ; 26    ROM Critical Section
DevHlp_VerifyAccess     EQU     39      ; 27    Verify access to memory
DevHlp_RAS              EQU     40      ; 28    Put info in RAS trace buffer

DevHlp_ABIOSGetParms    EQU     41      ; 29    Get ABIOS Calling Parms
DevHlp_AttachDD         EQU     42      ; 2A    Attach to a device driver
DevHlp_InternalError    EQU     43      ; 2B    Signal an internal error
DevHlp_ModifyPriority   EQU     44      ; 2C    Undocumented (used by PM)
DevHlp_AllocGDTSelector EQU     45      ; 2D    Allocate GDT Selectors
DevHlp_PhysToGDTSelector EQU    46      ; 2E    Convert phys addr to GDT sel
DevHlp_RealToProt       EQU     47      ; 2F    Change from real to protected mode
DevHlp_ProtToReal       EQU     48      ; 30    Change from protected to real mode

DevHlp_EOI              EQU     49      ; 31    Send EOI to PIC
DevHlp_UnPhysToVirt     EQU     50      ; 32    mark completion of PhysToVirt
DevHlp_TickCount        EQU     51      ; 33    modify timer

DevHlp_GetLIDEntry      EQU     52      ; 34    Obtain Logical ID
DevHlp_FreeLIDEntry     EQU     53      ; 35    Release Logical ID
DevHlp_ABIOSCall        EQU     54      ; 36    Call ABIOS
DevHlp_ABIOSCommonEntry EQU     55      ; 37    Invoke Common Entry Point
DevHlp_GetDeviceBlock   EQU     56      ; 38    Get ABIOS Device Block
                                        ; 39    Reserved for Profiling Kernel
DevHlp_RegisterStackUsage EQU   58      ; 3A    Register for stack usage
DevHlp_LogEntry         EQU     59      ; 3B    Place data in log buffer

DevHlp_VideoPause       EQU     60      ; 3C Video pause on/off      - D607

DevHlp_Save_Message     EQU     61      ; 3D    Save msg in SysInit Message Table

DevHlp_SegRealloc       EQU     62      ; 3E    Realloc DD protect mode segment
DevHlp_PutWaitingQueue  EQU     63      ; 3F    Put I/O request on waiting queue
DevHlp_GetWaitingQueue  EQU     64      ; 40    Get I/O request from waiting queue
;DevHlp_PhysToSys        EQU     65      ; 41    Address conversion for the AOX
;DevHlp_PhysToSysHook    EQU     66      ; 42    Address conversion for the AOX
DevHlp_RegisterDeviceClass EQU     67      ; 43    Register DC entry point
;
; 32-Bit DevHelps start at 80 (50h)
;
DevHlp_RegisterPDD      EQU     80      ; 50    Register PDD entry point with
                                        ;       VDM manager for later PDD-VDD
                                        ;       communication
DevHlp_RegisterBeep     EQU     81      ; 51    register PTD beep service
                                        ;       entry point with kernel
DevHlp_Beep             EQU     82      ; 52    preempt beep service via PTD

DevHlp_FreeGDTSelector  EQU     83      ; 53    Free allocated GDT selector

DevHlp_PhysToGDTSel     EQU     84      ; 54    Convert Phys Addr to GDT sel
                                        ;       with given access
                                        ;   BUGBUG: TEMPORARY!!!

DevHlp_VMLock           EQU     85      ; 55    Lock linear address range

DevHlp_VMUnlock         EQU     86      ; 56    Unlock address range

DevHlp_VMAlloc          EQU     87      ; 56    Allocate memory

DevHlp_VMFree           EQU     88      ; 58    Free memory or mapping

DevHlp_VMProcessToGlobal EQU    89      ; 59    Create global mapping to process
                                        ;       memory

DevHlp_VMGlobalToProcess EQU    90      ; 5A    Create process mapping to global
                                        ;       memory

DevHlp_VirtToLin        EQU     91      ; 5B Convert virtual address to linear

DevHlp_LinToGDTSelector EQU     92      ; 5C Convert linear address to virtual

DevHlp_GetDescInfo      EQU     93      ; 5D Return descriptor information

DevHlp_LinToPageList    EQU     94      ; 5E build pagelist array from lin addr

DevHlp_PageListToLin    EQU     95      ; 5F map page list array to lin addr

DevHlp_PageListToGDTSelector EQU    96  ; 60 map page list array to GDT sel.

DevHlp_RegisterTmrDD    EQU     97      ; 61 Register TMR Device Driver.

DevHlp_RegisterPerfCtrs EQU     98      ; 62 Register device driver perf. ctrs (PVW).

DevHlp_AllocateCtxHook  EQU     99      ; 63 Allocate a context hook

DevHlp_FreeCtxHook      EQU     100     ; 64 Free a context hook

DevHlp_ArmCtxHook       EQU     101     ; 65 Arm a context hook

DevHlp_VMSetMem         EQU     102     ; 66H commit/decommit memory

DevHlp_OpenEventSem     EQU     103     ; 67H open an event semaphore

DevHlp_CloseEventSem    EQU     104     ; 68H close an event semaphore

DevHlp_PostEventSem     EQU     105     ; 69H post an event semaphore

DevHlp_ResetEventSem    EQU     106     ; 6AH reset an event semaphore

DevHlp_RegisterFreq     EQU     107     ; 6BH   register PTD freq service
                                        ;       entry point with kernel

DevHlp_DynamicAPI       EQU     108     ; 6CH add a dynamic API
;
; Request bit definitions used in DevHlp_DynamicAPI
DHDYNAM_16B_CALLGATE    EQU     1       ; 1 = 16 bit callgate, 0 = 32 bit
DHDYNAM_16B_APIRTN      EQU     2       ; 1 = 16 bit API rtn,  0 = 32 rtn

DevHlp_ProcRun2         EQU     109     ; 6DH  Unblock process via procrun2
;
DevHlp_CreateInt13VDM   EQU     110 ;6EH Create Int13 VDM (Internal Only) OEMINT13

;
DevHlp_RegisterKrnlExit EQU     111 ;6FH Used to capture Kernel Exits   F78693
DevHlp_PMPostEventSem   EQU     112     ; 70h  PM Post Event Semaphore

; DevHlp's 70-7C used by other OS/2 versions                              ;@SEC

DevHlp_KillProc         EQU     125 ;7DH Kill Proc                        ;@SEC
DevHlp_QSysState        EQU     126 ;7EH Query System State               ;@SEC

event_SM_Mou            equ     0       ; event was Session Manager-mouse@@
    event_MUF           equ     0       ; (OBSOLETE) event was muf key
event_CtrlBrk           equ     1       ; event was Ctrl-Break
event_CtrlC             equ     2       ; event was Ctrl-C
event_CtrlScrLk         equ     3       ; event was Ctrl-ScrollLock
event_CtrlPrtSc         equ     4       ; event was Ctrl-PrtSc
event_ShftPrtSc         equ     5       ; event was Shift-PrtSc
event_SM_Kbd            equ     6       ; event was Session Manager -keyboard@@
event_SM_CAD            equ     7       ; event was Ctl-Alt-Del
event_HOT_PLUG          equ     8       ; Keyboard Hot Plug/Reset   DCR1187
event_POWER             equ     9       ; Power suspend event #26087
event_POWEROFF          equ     10      ; Power off event           ;d153441
event_count             equ     11      ; Number of possible events ;d153441
;
; file system devhelps (2/19/96) SRD
DevHlp_OpenFile         EQU     127     ; 7FH  Ring-0 File system Write
DevHlp_CloseFile        EQU     128     ; 80H  Ring-0 File system Seek
DevHlp_ReadFile         EQU     129     ; 81H  Ring-0 File system Read
DevHlp_ReadFileAt       EQU     130     ; 82H  File system Read at (seek)

;       Character Queue structure
;
;       QueueInit must be called before any other queue manipulation
;       subroutine.  The Qsize field must be initialized before
;       calling QueueInit.

CharQueue STRUC
        Qsize   DW      ?               ; Size of queue in bytes
        Qchrout DW      ?               ; Index of next char out
        Qcount  DW      ?               ; Count of characters in the queue
        Qbase   DB      ?               ; Queue buffer
CharQueue ENDS

RASrouter struc
        pretrace_addr   dw      0
        preminor        dw      0
        posttrace_addr  dw      0
        postminor       dw      0
RASrouter ends


; Following are constants used in conversion of LDT pseudo-tiled addresses

SEL_FLATMASK    equ     01fff0000h
SEL_FLAT_SHIFT  equ     0dh
SEL_LDT_RPL3    equ     07h


SelToFlat  MACRO        ;; Converts LDTsel:offset to Flat linear address
;;
;;      This macro converts given LDT selector:offset pair to Flat linear
;;  address. No verification is done on parameters. It is the invoker's
;;  responsibility to assure that the selector is valid, not a call gate,
;;  and given offset is within descriptor's limit.
;;
;;      ENTRY   (AX) = selector
;;              (BX) = offset
;;      EXIT   (EAX) = Flat address

        shl     eax, SEL_FLAT_SHIFT
        and     eax, SEL_FLATMASK
        mov     ax, bx
ENDM



FlatToSel  MACRO        ;; Convert flat linear address to LDTsel:offset pair
;;
;;      This macro converts given flat address to selector:offset pair. No
;;  verification is done on the parameters. It is the invoker's responsibility
;;  to ensure that the flat address is in the LDT pseudo-tiled compatibility
;;  region, and that the selector returned is valid.
;;      Linear addresses < 20000000h (512 Meg) fall into the LDT compatibility
;;  region. All memory allocated in this region with selector mapping request
;;  will have valid selectors.
;;  WARNING: The returned selector will have RPL level 3. Selectors
;;  corresponding to IOPL segments should have their lowest bit turned off.
;;
;;      ENTRY  (EAX) = Flat address
;;      EXIT    (AX) = selector
;;              (BX) = offset

        mov     bx, ax
        shr     eax, SEL_FLAT_SHIFT
        or      ax, SEL_LDT_RPL3

ENDM
