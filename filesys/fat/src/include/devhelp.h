/* DEVHELP.H - DevHelp header file for Watcom-based device drivers
*/

#ifndef DEVHELP_INCLUDED
#define DEVHELP_INCLUDED

#ifdef __WATCOMC__ 

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WATCOMC__
typedef VOID    FAR *PVOID;
typedef PVOID   FAR *PPVOID;
#endif


typedef ULONG   LIN;               // 32-Bit Linear Addess
typedef ULONG   __far *PLIN;       // 16:16 Ptr to a 32-Bit Linear Addess

typedef struct _PAGELIST {
  ULONG    PhysAddr;
  ULONG    Size;
} PAGELIST, __near *NPPAGELIST, __far *PPAGELIST;

extern ULONG Device_Help;
extern USHORT DGROUP;

#define DEVHELP_CALL \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "sub  ax,ax" \
   "error:" \
   value [ax]

USHORT DevHelp_ABIOSCall(USHORT Lid, NPBYTE ReqBlk, USHORT Entry_Type);
#pragma aux DevHelp_ABIOSCall = \
   "mov  dl,36h" \
   DEVHELP_CALL \
   parm caller [ax] [si] [dh] \
   modify exact [ax dl];

USHORT DevHelp_ABIOSCommonEntry(NPBYTE ReqBlk, USHORT Entry_Type);
#pragma aux DevHelp_ABIOSCommonEntry = \
   "mov  dl,37h" \
   DEVHELP_CALL \
   parm caller [si] [dh] \
   modify exact [ax dl];

USHORT DevHelp_AllocateCtxHook(NPFN HookHandler, PULONG HookHandle);
#pragma aux DevHelp_AllocateCtxHook = \
   ".386p" \
   "push bx" \
   "movzx   eax,ax" \
   "mov  ebx,-1" \
   "mov  dl,63h" \
   "call dword ptr ds:[Device_Help]" \
   "pop  bx" \
   "jc   error" \
   "mov  es:[bx],eax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [ax] [es bx] \
   modify exact [ax bx dl es];

USHORT DevHelp_AllocGDTSelector(PSEL Selectors, USHORT Count);
#pragma aux DevHelp_AllocGDTSelector = \
   "mov  dl,2Dh" \
   DEVHELP_CALL \
   parm caller [es di] [cx] \
   modify nomemory exact [ax dl];

#define MEMTYPE_ABOVE_1M   0
#define MEMTYPE_BELOW_1M   1

USHORT DevHelp_AllocPhys(ULONG lSize, USHORT MemType, PULONG PhysAddr);
#pragma aux DevHelp_AllocPhys = \
   "xchg ax,bx" \
   "mov  dl,18h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[di],bx" \
   "mov  es:[di+2],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [ax bx] [dh] [es di] \
   modify exact [ax bx dx];

#define WAIT_NOT_ALLOWED 0
#define WAIT_IS_ALLOWED  1

USHORT DevHelp_AllocReqPacket(USHORT WaitFlag, PBYTE __far *ReqPktAddr);
#pragma aux DevHelp_AllocReqPacket = \
   "mov  dl,0Dh", \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "push es" \
   "push bx" \
   "mov  bx,sp" \
   "les  bx,ss:[bx]" \
   "pop  es:[bx]" \
   "pop  es:[bx+2]" \
   "error:" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm caller [dh] [] \
   modify exact [ax bx dl es];

USHORT DevHelp_ArmCtxHook(ULONG HookData, ULONG HookHandle);
#pragma aux DevHelp_ArmCtxHook = \
   ".386p" \
   "mov  bx,sp" \
   "mov  dl,65h", \
   "mov  eax,ss:[bx]" \
   "mov  ecx,-1" \
   "mov  ebx,ss:[bx+4]" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax bx cx dl];

typedef struct _IDCTABLE {
  USHORT   Reserved[3];
  VOID     (__far *ProtIDCEntry) (VOID);
  USHORT   ProtIDC_DS;
} IDCTABLE, __near *NPIDCTABLE;

USHORT DevHelp_AttachDD(NPSZ DDName, NPBYTE DDTable);
#pragma aux DevHelp_AttachDD = \
   "mov  dl,2Ah" \
   "call dword ptr ds:[Device_Help]" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm caller [bx] [di] \
   modify exact [ax dl];

USHORT DevHelp_Beep(USHORT Frequency, USHORT DurationMS);
#pragma aux DevHelp_Beep = \
   "mov  dl,52h" \
   DEVHELP_CALL \
   parm caller nomemory [bx] [cx] \
   modify nomemory exact [ax dx];

USHORT DevHelp_CloseEventSem(ULONG hEvent);
#pragma aux DevHelp_CloseEventSem = \
   ".386p" \
   "movzx   esp,sp" \
   "mov  dl,68h" \
   "mov  eax,[esp]" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

USHORT DevHelp_DeRegister(USHORT MonitorPID, USHORT MonitorHandle, PUSHORT MonitorsLeft);
#pragma aux DevHelp_DeRegister = \
   "mov  dl,21h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[di],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [bx] [ax] [es di] \
   modify exact [ax dl];

USHORT DevHelp_DevDone(PBYTE ReqPktAddr);
#pragma aux DevHelp_DevDone = \
   "mov  dl,1" \
   DEVHELP_CALL \
   parm caller [es bx] \
   modify exact [ax dl];

#define DYNAPI_CALLGATE16       0x0001   /* 16:16 CallGate     */
#define DYNAPI_CALLGATE32       0x0000   /*  0:32 CallGate     */

#define DYNAPI_ROUTINE16        0x0002   /* 16:16 Routine Addr */
#define DYNAPI_ROUTINE32        0x0000   /*  0:32 Routine Addr */

USHORT DevHelp_DynamicAPI(PVOID  RoutineAddress, USHORT ParmCount, USHORT Flags, PSEL CallGateSel);
#pragma aux DevHelp_DynamicAPI = \
   "mov  dl,6Ch" \
   "xchg ax,bx" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[si],di" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller [ax bx] [cx] [dh] [es si] \
   modify exact [ax bx di dl];

USHORT DevHelp_EOI(USHORT IRQLevel);
#pragma aux DevHelp_EOI = \
   "mov  dl,31h" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [al] \
   modify nomemory exact [ax dl];

USHORT DevHelp_FreeCtxHook(ULONG HookHandle);
#pragma aux DevHelp_FreeCtxHook = \
   ".386p" \
   "movzx   esp,sp" \
   "mov  dl,64h", \
   "mov  eax,[esp]" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

USHORT DevHelp_FreeGDTSelector(SEL Selector);
#pragma aux DevHelp_FreeGDTSelector = \
   "mov  dl,53h" \
   DEVHELP_CALL \
   parm caller nomemory [ax] \
   modify nomemory exact [ax dl];

USHORT DevHelp_FreeLIDEntry(USHORT LIDNumber);
#pragma aux DevHelp_FreeLIDEntry = \
   "mov  dl,35h" \
   DEVHELP_CALL \
   parm caller nomemory [ax] \
   modify nomemory exact [ax dl];

USHORT DevHelp_FreePhys(ULONG PhysAddr);
#pragma aux DevHelp_FreePhys = \
   "xchg ax,bx" \
   "mov  dl,19h" \
   "call dword ptr ds:[Device_Help]" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm caller nomemory [ax bx] \
   modify exact [ax bx dl];

USHORT DevHelp_FreeReqPacket(PBYTE ReqPktAddr);
#pragma aux DevHelp_FreeReqPacket = \
   "mov  dl,0Eh", \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm caller [es bx] \
   modify exact [ax dl];

typedef struct _SELDESCINFO {
  UCHAR    Type;                        
  UCHAR    Granularity;                 
  LIN      BaseAddr;                    
  ULONG    Limit;                       
} SELDESCINFO, __far *PSELDESCINFO;  
                                        
typedef struct _GATEDESCINFO {         
  UCHAR    Type;                        
  UCHAR    ParmCount;                   
  SEL      Selector;                    
  USHORT   Reserved_1;                  
  ULONG    Offset;                      
} GATEDESCINFO, __far *PGATEDESCINFO;

USHORT DevHelp_GetDescInfo(SEL Selector, PBYTE SelInfo);
#pragma aux DevHelp_GetDescInfo = \
   ".386p" \
   "mov  dl,5Dh" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[bx],ax" \
   "mov  es:[bx+2],ecx" \
   "mov  es:[bx+6],edx" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [ax] [es bx] \
   modify exact [ax cx dx];

USHORT DevHelp_GetDeviceBlock(USHORT Lid, PPVOID DeviceBlockPtr);
#pragma aux DevHelp_GetDeviceBlock = \
   "mov  dl,38h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[si],dx" \
   "mov  es:[si+2],cx" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [ax] [es si] \
   modify exact [ax bx cx dx];

#define DHGETDOSV_SYSINFOSEG            1
#define DHGETDOSV_LOCINFOSEG            2
#define DHGETDOSV_VECTORSDF             4
#define DHGETDOSV_VECTORREBOOT          5
#define DHGETDOSV_YIELDFLAG             7                            /*@V76282*/
#define DHGETDOSV_TCYIELDFLAG           8                            /*@V76282*/
#define DHGETDOSV_DOSCODEPAGE           11                           /*@V76282*/
#define DHGETDOSV_INTERRUPTLEV          13
#define DHGETDOSV_DEVICECLASSTABLE      14                           /*@V76282*/
#define DHGETDOSV_DMQSSELECTOR          15                           /*@V76282*/
#define DHGETDOSV_APMINFO               16                           /*@V76282*/

USHORT DevHelp_GetDOSVar(USHORT VarNumber, USHORT VarMember, PPVOID KernalVar);
#pragma aux DevHelp_GetDOSVar = \
   "mov  dl,24h" \
   "call dword ptr ds:[Device_Help]" \
   "mov  es:[di],bx" \
   "mov  es:[di+2],ax" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [al] [cx] [es di] \
   modify exact [ax bx dl];

USHORT DevHelp_GetLIDEntry(USHORT DeviceType, SHORT LIDIndex, USHORT Type, PUSHORT LID);
#pragma aux DevHelp_GetLIDEntry = \
   "mov  dl,34h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[di],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [al] [bl] [dh] [es di] \
   modify exact [ax dl];

USHORT DevHelp_InternalError(PSZ MsgText, USHORT MsgLength );
#pragma aux DevHelp_InternalError aborts = \
   "push ds" \
   "push es" \
   "pop  ds" \
   "pop  es" \
   "mov  dl,2Bh" \
   "jmp dword ptr ds:[Device_Help]" \
   parm [es si] [di] \
   modify nomemory exact [];

USHORT DevHelp_LinToGDTSelector(SEL Selector, LIN LinearAddr, ULONG Size);
#pragma aux DevHelp_LinToGDTSelector = \
   ".386p" \
   "mov  bx,sp" \
   "mov  dl,5Ch" \
   "mov  ecx,ss:[bx+4]" \
   "mov  ebx,ss:[bx]" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [ax] [] \
   modify nomemory exact [ax bx cx dl];

#define LOCKTYPE_SHORT_ANYMEM    0x00
#define LOCKTYPE_LONG_ANYMEM     0x01
#define LOCKTYPE_LONG_HIGHMEM    0x03
#define LOCKTYPE_SHORT_VERIFY    0x04

USHORT DevHelp_Lock(SEL Segment, USHORT LockType, USHORT WaitFlag, PULONG LockHandle);
#pragma aux DevHelp_Lock = \
   "mov  dl,13h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[di],bx" \
   "mov  es:[di+2],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller [ax] [bh] [bl] [es di] \
   modify exact [ax dl];

USHORT DevHelp_MonFlush(USHORT MonitorHandle);
#pragma aux DevHelp_MonFlush = \
   "mov  dl,23h" \
   DEVHELP_CALL \
   parm caller [ax] \
   modify exact [ax dl];

USHORT DevHelp_MonitorCreate(USHORT MonitorHandle, PBYTE FinalBuffer, NPFN NotifyRtn, PUSHORT MonitorChainHandle);
#pragma aux DevHelp_MonitorCreate = \
   ".386p" \
   "mov  dl,1Fh" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  si,[esp]" \
   "mov  es,[esp+2]" \
   "mov  es:[si],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller [ax] [es si] [di] [] \
   modify exact [ax dl es si];

/* DevHelp_MonWrite has one change compared to the original specification.
   The DataRecord parameter was originally a far (16:16) pointer.  It has 
   been changed to a near pointer because DS must point to the default data
   segment and therefore the DataRecord parameter must be near.  This 
   allows the compiler to catch the error.
*/

USHORT DevHelp_MonWrite(USHORT MonitorHandle, NPBYTE DataRecord, USHORT Count, ULONG TimeStampMS, USHORT WaitFlag);
#pragma aux DevHelp_MonWrite = \
   "mov  dl,22h" \
   DEVHELP_CALL \
   parm caller [ax] [si] [cx] [di bx] [dh] \
   modify exact [ax dl];

USHORT DevHelp_OpenEventSem(ULONG hEvent);
#pragma aux DevHelp_OpenEventSem = \
   ".386p" \
   "mov  eax,[esp]" \
   "mov  dl,67h" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

#define GDTSEL_R3CODE           0x0000                               /*@V76282*/
#define GDTSEL_R3DATA           0x0001                               /*@V76282*/
#define GDTSEL_R2CODE           0x0003                               /*@V76282*/
#define GDTSEL_R2DATA           0x0004                               /*@V76282*/
#define GDTSEL_R0CODE           0x0005                               /*@V76282*/
#define GDTSEL_R0DATA           0x0006                               /*@V76282*/

/* GDTSEL_ADDR32 may be OR'd with above defines */                   /*@V76282*/
#define GDTSEL_ADDR32           0x0080                               /*@V76282*/

USHORT DevHelp_PageListToGDTSelector(SEL Selector, ULONG Size, USHORT Access, LIN pPageList);
#pragma aux DevHelp_PageListToGDTSelector = \
   ".386p" \
   "mov  ecx,[esp]" \
   "mov  dh,[esp+4]" \
   "mov  edi,[esp+6]" \
   "mov  dl,60h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller [] \
   modify nomemory exact [ax cx dx di];

USHORT DevHelp_PageListToLin(ULONG Size, LIN pPageList, PLIN LinearAddr);
#pragma aux DevHelp_PageListToLin = \
   ".386p" \
   "mov  ecx,[esp]" \
   "mov  edi,[esp+4]" \
   "mov  dl,5Fh" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  di,[esp+8]" \
   "mov  es:[di],eax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [] \
   modify exact [ax cx dx di];


USHORT DevHelp_LinToPageList( LIN    LinearAddr,
                                       ULONG  Size,
                                       LIN    pPageList,
                                       PULONG PageListCount );
#pragma aux DevHelp_LinToPageList = \
  ".386p" \
  "mov  eax,[esp]" \
  "mov  ecx,[esp+4]" \
  "mov  edi,[esp+8]" \
  "mov  dl, 5Eh" \
  "call dword ptr ds:[Device_Help]" \
  "jc   error" \
  "mov  [esp+8],edi" \
  "mov  di, [esp+12]" \
  "mov  ss:[di],eax" \
  "sub  eax,eax" \
  "error:" \
   value [ax] \
   parm caller [] \
   modify exact [ax cx di dx];

/* NOTE: After the DevHelp call, SI contains the modified selector. However, 
   the C interface has no provisions for returning this value to the caller.
   You can, however, use the _SI() inline function defined in include.h.
*/

USHORT DevHelp_PhysToGDTSel(ULONG PhysAddr, ULONG Count, SEL Selector, UCHAR Access);
#pragma aux DevHelp_PhysToGDTSel = \
   ".386p" \
   "push bp" \
   "mov  dl,54h" \
   "mov  bp,sp" \
   "mov  eax,[bp+2]" \
   "mov  ecx,[bp+6]" \
   "mov  si,[bp+10]" \
   "mov  dh,[bp+12]" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "sub  ax,ax" \
   "error:" \
   "pop  bp" \
   value [ax] \
   parm caller nomemory [] \
   modify nomemory exact [ax cx dx si];

USHORT DevHelp_PhysToGDTSelector(ULONG PhysAddr, USHORT Count, SEL Selector);
#pragma aux DevHelp_PhysToGDTSelector = \
   "xchg ax,bx" \
   "mov  dl,2Eh" \
   DEVHELP_CALL \
   parm nomemory [ax bx] [cx] [si] \
   modify nomemory exact [ax bx dl];

#define SELTYPE_R3CODE  0
#define SELTYPE_R3DATA  1
#define SELTYPE_FREE    2
#define SELTYPE_R2CODE  3
#define SELTYPE_R2DATA  4
#define SELTYPE_R3VIDEO 5

USHORT DevHelp_PhysToUVirt(ULONG PhysAddr, USHORT Length, USHORT flags, USHORT TagType, PVOID SelOffset);
#pragma aux DevHelp_PhysToUVirt = \
   "xchg ax,bx" \
   "mov  dl,17h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "push es" \
   "push bx" \
   "mov  bx,sp" \
   "les  bx,ss:[bx+4]" \
   "pop  es:[bx]" \
   "pop  es:[bx+2]" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [bx ax] [cx] [dh] [si] [] \
   modify exact [ax bx dl es];

USHORT DevHelp_PhysToVirt(ULONG PhysAddr, USHORT usLength, PVOID SelOffset, PUSHORT ModeFlag);
#pragma aux DevHelp_PhysToVirt = \
   "xchg ax,bx" \
   "mov  dx,15h" \
   "push ds" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "sub  ax,ax" \
   "mov  es:[di],si" \
   "mov  es:[di+2],ds" \
   "error:" \
   "pop  ds" \
   value [ax] \
   parm caller nomemory [bx ax] [cx] [es di] [] \
   modify exact [ax bx dx si];

USHORT DevHelp_PostEventSem(ULONG hEvent);
#pragma aux DevHelp_PostEventSem = \
   ".386p" \
   "mov  eax,[esp]" \
   "mov  dl,69h" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

#define WAIT_IS_INTERRUPTABLE      0
#define WAIT_IS_NOT_INTERRUPTABLE  1

#define WAIT_INTERRUPTED           0x8003
#define WAIT_TIMED_OUT             0x8001

USHORT DevHelp_ProcBlock(ULONG EventId, ULONG WaitTime, USHORT IntWaitFlag);
#pragma aux DevHelp_ProcBlock = \
   "mov  dl,4" \
   "xchg ax,bx" \
   "xchg cx,di" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  ax,0"       /* doesn't affect zero flag like sub ax,ax does */ \
   "error:" \
   value [ax] \
   parm caller nomemory [ax bx] [di cx] [dh] \
   modify nomemory exact [ax bx cx dl di];

USHORT DevHelp_ProcRun(ULONG EventId, PUSHORT AwakeCount);
#pragma aux DevHelp_ProcRun = \
   "mov  dl,5" \
   "xchg ax,bx" \
   "call dword ptr ds:[Device_Help]" \
   "mov  es:[si],ax" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [ax bx] [es si] \
   modify exact [ax bx dl];

USHORT DevHelp_PullParticular(NPBYTE Queue, PBYTE ReqPktAddr);
#pragma aux DevHelp_PullParticular= \
   "mov  dl,0Bh" \
   DEVHELP_CALL \
   parm [si] [es bx] \
   modify exact [ax dl];

USHORT DevHelp_PullRequest(NPBYTE Queue, PBYTE __far *ReqPktAddr);
#pragma aux DevHelp_PullRequest = \
   ".386p" \
   "push es" \
   "mov  dl,0Ah" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "movzx   esp,sp" \
   "push es" \
   "push bx" \
   "mov  bx,sp" \
   "les  bx,[esp]" \
   "pop  es:[bx]" \
   "pop  es:[bx+2]" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm [si] [] \
   modify exact [ax bx dl es];

USHORT DevHelp_PushRequest(NPBYTE Queue, PBYTE ReqPktddr);
#pragma aux DevHelp_PushRequest = \
   "mov  dl,09h" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm [si] [es bx] \
   modify exact [ax dl];

USHORT DevHelp_QueueFlush(NPBYTE Queue);
#pragma aux DevHelp_QueueFlush = \
   "mov  dl,10h" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm [bx] \
   modify exact [ax dl];

typedef struct _QUEUEHDR  {
  USHORT   QSize;
  USHORT   QChrOut;
  USHORT   QCount;
  BYTE     Queue[1];
} QUEUEHDR, __far *PQUEUEHDR;

USHORT DevHelp_QueueInit(NPBYTE Queue);
#pragma aux DevHelp_QueueInit = \
   "mov  dl,0Fh" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm [bx] \
   modify exact [ax dl];

USHORT DevHelp_QueueRead(NPBYTE Queue, PBYTE Char);
#pragma aux DevHelp_QueueRead = \
   "mov  dl,12h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[di],al" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm [bx] [es di] \
   modify exact [ax dl];

USHORT DevHelp_QueueWrite(NPBYTE Queue, UCHAR Char);
#pragma aux DevHelp_QueueWrite = \
   "mov  dl,11h" \
   DEVHELP_CALL \
   parm [bx] [al] \
   modify exact [ax dl];

USHORT DevHelp_RAS(USHORT Major, USHORT Minor, USHORT Size, PBYTE Data);
#pragma aux DevHelp_RAS = \
   "push ds" \
   "push es" \
   "pop  ds" \
   "pop  es" \
   "mov  dl,28h" \
   "call dword ptr ds:[Device_Help]" \
   "push es" \
   "pop  ds" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm [ax] [cx] [bx] [es si] \
   modify nomemory exact [ax dl es];

#define CHAIN_AT_TOP    0
#define CHAIN_AT_BOTTOM 1

USHORT DevHelp_Register(USHORT MonitorHandle, USHORT MonitorPID, PBYTE InputBuffer, NPBYTE OutputBuffer, USHORT ChainFlag);
#pragma aux DevHelp_Register = \
   "mov  dl,20h" \
   DEVHELP_CALL \
   parm caller nomemory [ax] [cx] [es si] [di] [dh] \
   modify nomemory exact [ax dl];

USHORT DevHelp_RegisterBeep(PFN BeepHandler);
#pragma aux DevHelp_RegisterBeep = \
   "mov  dl,51h" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [cx di] \
   modify nomemory exact [ax dl];

#define DEVICECLASS_ADDDM       1                                    /*@V74979*/
#define DEVICECLASS_MOUSE       2                                    /*@V74979*/

USHORT DevHelp_RegisterDeviceClass(NPSZ DeviceString, PFN DriverEP, USHORT DeviceFlags, USHORT DeviceClass, PUSHORT DeviceHandle);
#pragma aux DevHelp_RegisterDeviceClass = \
   ".386p" \
   "mov  dl,43h" \
   "xchg ax,bx" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  bx,[esp]" \
   "mov  es:[bx],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm [si] [ax bx] [di] [cx] [] \
   modify exact [ax bx dl es];

USHORT DevHelp_RegisterPDD(NPSZ PhysDevName, PFN HandlerRoutine);
#pragma aux DevHelp_RegisterPDD = \
   "mov  dl,50h" \
   DEVHELP_CALL \
   parm caller [si] [es di] \
   modify nomemory exact [ax dl];

typedef struct _STACKUSAGEDATA {
   USHORT Size;
   USHORT Flags;
   USHORT IRQLevel;
   USHORT CLIStack;
   USHORT STIStack;
   USHORT EOIStack;
   USHORT NestingLevel;
} STACKUSAGEDATA;

USHORT DevHelp_RegisterStackUsage(PVOID StackUsageData);
#pragma aux DevHelp_RegisterStackUsage = \
   "mov  dl,3Ah" \
   "call dword ptr ds:[Device_Help]" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm caller [bx] \
   modify nomemory exact [ax dl];

USHORT DevHelp_RegisterTmrDD(NPFN TimerEntry, PULONG TmrRollover, PULONG Tmr);
#pragma aux DevHelp_RegisterTmrDD = \
   ".386p" \
   "mov  dl,61h" \
   "call dword ptr ds:[Device_Help]" \
   "mov  ax,bx" \
   "les  bx,[esp]" \
   "mov  es:[bx],ax" \
   "mov  es:[bx+2],di" \
   "les  bx,[esp+4]" \
   "mov  es:[bx],cx" \
   "mov  es:[bx+2],di" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [di] [] \
   modify exact [ax bx cx di dl es];

USHORT DevHlp_ResetEventSem(ULONG hEvent, PULONG pNumPosts);
#pragma aux DevHelp_ResetEventSem = \
   ".386p" \
   "mov  eax,[esp]" \
   "mov  edi,[esp+4]" \
   "mov  dl,6Ah" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify exact [ax di dl];

USHORT DevHelp_ResetTimer(NPFN TimerHandler);
#pragma aux DevHelp_ResetTimer = \
   "mov  dl,1Eh" \
   DEVHELP_CALL \
   parm caller nomemory [ax] \
   modify nomemory exact [ax dl];

typedef struct _MSGTABLE {
  USHORT   MsgId;                       /* Message Id #                  */
  USHORT   cMsgStrings;                 /* # of (%) substitution strings */
  PSZ      MsgStrings[1];               /* Substitution string pointers  */
} MSGTABLE, __near *NPMSGTABLE;

USHORT DevHelp_Save_Message(NPBYTE MsgTable);
#pragma aux DevHelp_Save_Message = \
   "sub  bx,bx" \
   "mov  dl,3Dh" \
   DEVHELP_CALL \
   parm caller [si] \
   modify nomemory exact [ax bx dl];

USHORT DevHelp_SchedClock(PFN NEAR *SchedRoutineAddr);
#pragma aux DevHelp_SchedClock = \
   "mov  dl,0h" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm caller [ax] \
   modify nomemory exact [ax dl];

USHORT DevHelp_SemClear(ULONG SemHandle);
#pragma aux DevHelp_SemClear = \
   "xchg ax,bx" \
   "mov  dl,7h" \
   DEVHELP_CALL \
   parm nomemory [ax bx] \
   modify nomemory exact [ax bx dl];

#define SEMUSEFLAG_IN_USE       0
#define SEMUSEFLAG_NOT_IN_USE   1

USHORT DevHelp_SemHandle(ULONG SemKey, USHORT SemUseFlag, PULONG SemHandle);
#pragma aux DevHelp_SemHandle = \
   "xchg ax,bx" \
   "mov  dl,8h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "mov  es:[si],bx" \
   "mov  es:[si+2],ax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm nomemory [ax bx] [dh] [es si] \
   modify exact [ax bx dl];

USHORT DevHelp_SemRequest(ULONG SemHandle, ULONG SemTimeout);
#pragma aux DevHelp_SemRequest = \
   "xchg ax,bx" \
   "xchg di,cx" \
   "mov  dl,06h" \
   DEVHELP_CALL \
   parm nomemory [ax bx] [cx di] \
   modify nomemory exact [ax bx cx di dl];

#define EVENT_MOUSEHOTKEY   0
#define EVENT_CTRLBREAK     1
#define EVENT_CTRLC         2
#define EVENT_CTRLNUMLOCK   3
#define EVENT_CTRLPRTSC     4
#define EVENT_SHIFTPRTSC    5
#define EVENT_KBDHOTKEY     6
#define EVENT_KBDREBOOT     7

USHORT DevHelp_SendEvent(USHORT EventType, USHORT Parm);
#pragma aux DevHelp_SendEvent = \
   "mov  dl,25h" \
   "call dword ptr ds:[Device_Help]" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm nomemory [ah] [bx] \
   modify nomemory exact [ax dl];

USHORT DevHelp_SetIRQ(NPFN IRQHandler, USHORT IRQLevel, USHORT SharedFlag);
#pragma aux DevHelp_SetIRQ = \
   "mov  dl,1Bh" \
   DEVHELP_CALL \
   parm caller nomemory [ax] [bx] [dh] \
   modify nomemory exact [ax dl];

USHORT DevHelp_SetTimer(NPFN TimerHandler);
#pragma aux DevHelp_SetTimer = \
   "mov  dl,1Dh" \
   DEVHELP_CALL \
   parm caller nomemory [ax] \
   modify nomemory exact [ax dl];

USHORT DevHelp_SortRequest(NPBYTE Queue, PBYTE ReqPktAddr);
#pragma aux DevHelp_SortRequest = \
   "mov  dl,0Ch" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm [si] [es bx] \
   modify exact [ax dl];

USHORT DevHelp_TCYield(void);
#pragma aux DevHelp_TCYield = \
   "mov  dl,3" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

USHORT DevHelp_TickCount(NPFN TimerHandler, USHORT TickCount);
#pragma aux DevHelp_TickCount = \
   "mov  dl,33h" \
   DEVHELP_CALL \
   parm caller nomemory [ax] [bx] \
   modify nomemory exact [ax dl];

USHORT DevHelp_UnLock(ULONG LockHandle);
#pragma aux DevHelp_UnLock = \
   "xchg ax,bx" \
   "mov  dl,14h" \
   DEVHELP_CALL \
   parm nomemory [ax bx] \
   modify nomemory exact [ax bx dl];

USHORT DevHelp_UnSetIRQ(USHORT IRQLevel);
#pragma aux DevHelp_UnSetIRQ = \
   "mov  dl,1Ch" \
   DEVHELP_CALL \
   parm caller nomemory [bx] \
   modify nomemory exact [ax dl];

#define VERIFY_READONLY    0
#define VERIFY_READWRITE   1

USHORT DevHelp_VerifyAccess(SEL MemSelector, USHORT Length, USHORT MemOffset, UCHAR AccessFlag);
#pragma aux DevHelp_VerifyAccess = \
   "mov  dl,27h" \
   DEVHELP_CALL \
   parm caller nomemory [ax] [cx] [di] [dh] \
   modify nomemory exact [ax dl];

#define VIDEO_PAUSE_OFF            0
#define VIDEO_PAUSE_ON             1

USHORT DevHelp_VideoPause(USHORT OnOff);
#pragma aux DevHelp_VideoPause = \
   "mov  dl,3Ch" \
   "call dword ptr ds:[Device_Help]" \
   "mov  ax,0" \
   "sbb  ax,0" \
   value [ax] \
   parm nomemory [al] \
   modify nomemory exact [ax dl];

USHORT DevHelp_VirtToLin(SEL Selector, ULONG Offset, PLIN LinearAddr);
#pragma aux DevHelp_VirtToLin = \
   ".386p" \
   "movzx   esp,sp" \
   "mov  esi,[esp]" \
   "mov  dl,5bh" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  bx,[esp+4]" \
   "mov  es:[bx],eax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [ax] [] \
   modify exact [ax si es bx dl];

USHORT DevHelp_VirtToPhys(PVOID SelOffset, PULONG PhysAddr);
#pragma aux DevHelp_VirtToPhys = \
   "push ds" \
   "mov  dl,16h" \
   "push es" \
   "mov  si,ds" \
   "mov  es,si" \
   "mov  ds,bx" \
   "mov  si,ax" \
   "call dword ptr ds:[Device_Help]" \
   "pop  es" \
   "mov  es:[di],bx" \
   "mov  es:[di+2],ax" \
   "pop  ds" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [ax bx] [es di] \
   modify exact [ax bx dl es si];

#define VMDHA_16M               0x0001                               /*@V76282*/
#define VMDHA_FIXED             0x0002                               /*@V76282*/
#define VMDHA_SWAP              0x0004                               /*@V76282*/
#define VMDHA_CONTIG            0x0008                               /*@V76282*/
#define VMDHA_PHYS              0x0010                               /*@V76282*/
#define VMDHA_PROCESS           0x0020                               /*@V76282*/
#define VMDHA_SGSCONT           0x0040                               /*@V76282*/
#define VMDHA_RESERVE           0x0100                               /*@V76282*/
#define VMDHA_USEHIGHMEM        0x0800                               /*@V76282*/
 
USHORT DevHelp_VMAlloc(ULONG Flags, ULONG Size, ULONG PhysAddr, PLIN LinearAddr, PPVOID SelOffset);
#pragma aux DevHelp_VMAlloc = \
   ".386p" \
   "movzx   esp,sp" \
   "mov  eax,[esp]" \
   "mov  ecx,[esp+4]" \
   "mov  edi,[esp+8]" \
   "mov  dl,57h" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  di,[esp+12]" \
   "mov  es:[di],eax" \
   "les  di,[esp+16]" \
   "mov  es:[di],ecx" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [] \
   modify exact [ax cx di dl es];

USHORT DevHelp_VMFree(LIN LinearAddr);
#pragma aux DevHelp_VMFree = \
   ".386p" \
   "movzx   esp,sp" \
   "mov  dl,58h" \
   "mov  eax,[esp]" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

#define VMDHGP_WRITE            0x0001                               /*@V76282*/
#define VMDHGP_SELMAP           0x0002                               /*@V76282*/
#define VMDHGP_SGSCONTROL       0x0004                               /*@V76282*/
#define VMDHGP_4MEG             0x0008                               /*@V76282*/

USHORT DevHelp_VMGlobalToProcess(ULONG Flags, LIN LinearAddr, ULONG Length, PLIN ProcessLinearAddr);
#pragma aux DevHelp_VMGlobalToProcess = \
   ".386p" \
   "mov  dl,5Ah" \
   "mov  eax,[esp]" \
   "mov  ebx,[esp+4]" \
   "mov  ecx,[esp+8]" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  bx,[esp+12]" \
   "mov  es:[bx],eax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [] \
   modify exact [ax bx cx dl es];

#define VMDHL_NOBLOCK           0x0001
#define VMDHL_CONTIGUOUS        0x0002
#define VMDHL_16M               0x0004
#define VMDHL_WRITE             0x0008
#define VMDHL_LONG              0x0010
#define VMDHL_VERIFY            0x0020

USHORT DevHelp_VMLock(ULONG Flags, LIN LinearAddr, ULONG Length, LIN pPagelist, LIN pLockHandle, PULONG PageListCount);
#pragma aux DevHelp_VMLock = \
   ".386p" \
   "mov  dl,55h" \
   "mov  eax,[esp]" \
   "mov  ebx,[esp+4]" \
   "mov  ecx,[esp+8]" \
   "mov  edi,[esp+12]" \
   "mov  esi,[esp+16]" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  bx,[esp+20]" \
   "mov  es:[bx],eax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [] \
   modify exact [ax bx cx dl si di es];

#define VMDHPG_READONLY         0x0000                               /*@V76282*/
#define VMDHPG_WRITE            0x0001                               /*@V76282*/

USHORT DevHelp_VMProcessToGlobal(ULONG Flags, LIN LinearAddr, ULONG Length, PLIN GlobalLinearAddr);
#pragma aux DevHelp_ProcessToGlobal = \
   ".386p" \
   "mov  dl,59h" \
   "mov  eax,[esp]" \
   "mov  ebx,[esp+4]" \
   "mov  ecx,[esp+8]" \
   "call dword ptr ds:[Device_Help]" \
   "jc   error" \
   "les  bx,[esp+12]" \
   "mov  es:[bx],eax" \
   "sub  ax,ax" \
   "error:" \
   value [ax] \
   parm caller nomemory [] \
   modify exact [ax bx cx dl es];

#define VMDHS_DECOMMIT          0x0001                               /*@V76282*/
#define VMDHS_RESIDENT          0x0002                               /*@V76282*/
#define VMDHS_SWAP              0x0004                               /*@V76282*/

USHORT DevHelp_VMSetMem(LIN LinearAddr, ULONG Size, ULONG Flags);
#pragma aux DevHelp_VMSetMem = \
   ".386p" \
   "mov  dl,66h" \
   "mov  ebx,[esp]" \
   "mov  ecx,[esp+4]" \
   "mov  eax,[esp+8]" \
   DEVHELP_CALL \
   parm caller nomemory [] \
   modify nomemory exact [ax bx cx dl];

USHORT DevHelp_VMUnLock(LIN pLockHandle);
#pragma aux DevHelp_VMUnLock = \
   ".386p" \
   "mov  dl,56h" \
   "mov  esi,[esp]" \
   DEVHELP_CALL \
   parm caller [] \
   modify exact [ax si dl];

USHORT DevHelp_Yield(void);
#pragma aux DevHelp_Yield = \
   "mov  dl,2" \
   "call dword ptr ds:[Device_Help]" \
   "sub  ax,ax" \
   value [ax] \
   parm caller nomemory [] \
   modify nomemory exact [ax dl];

/*
** DevHlp Error Codes  (from ABERROR.INC)
*/

#define MSG_MEMORY_ALLOCATION_FAILED    0x00
#define ERROR_LID_ALREADY_OWNED         0x01
#define ERROR_LID_DOES_NOT_EXIST        0x02
#define ERROR_ABIOS_NOT_PRESENT         0x03
#define ERROR_NOT_YOUR_LID              0x04
#define ERROR_INVALID_ENTRY_POINT       0x05

#ifdef __cplusplus
}
#endif

#endif
#endif
