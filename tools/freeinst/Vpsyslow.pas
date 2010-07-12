// |---------------------------------------------------------|
// |                                                         |
// |     Virtual Pascal Runtime Library.  Version 2.1.       |
// |     System interface layer for all OSes                 |
// |     ----------------------------------------------------|
// |     Copyright (C) 1995-2003 vpascal.com                 |
// |                                                         |
// |---------------------------------------------------------|

{&OrgName+,Speed+,AlignCode+,AlignRec-,CDecl-,Far16-,Frame+,Delphi+}
{$X+,W-,I-,J+,H-,Delphi+,R-,S-,Q-,B-,T-,Use32+}

unit VPSysLow;

interface

uses
{$IFDEF OS2}    Os2Def, Os2Base; {$Undef KeyDll} {$ENDIF}
{$IFDEF LINUX}  Linux;                           {$ENDIF}
{$IFDEF WIN32}  Windows;                         {$ENDIF}
{$IFDEF DPMI32} Dpmi32df;

var
  Video_Adapter_Found : (mda_found, cga_found, ega_found, vga_found);
{$ENDIF}

type
  TQuad = Comp;
  PQuad = ^TQuad;
  TSemHandle = Longint;

const
{$IFDEF OS2}
  SemInfinite = sem_indefinite_wait;
  PathSeparator = ';';
  AllFilesMask  = '*';
{$ENDIF}
{$IFDEF WIN32}
  SemInfinite = INFINITE;
  PathSeparator = ';';
  AllFilesMask  = '*';
{$ENDIF}
{$IFDEF DPMI32}
  SemInfinite = -1; // not used
  PathSeparator = ';';
  AllFilesMask  = '*.*';
{$ENDIF}
{$IFDEF LINUX}
  SemInfinite = -1; // not used
  Exception_Maximum_Parameters = 4;
  PathSeparator = ':';
  AllFilesMask  = '*';
{$ENDIF}

{$IFDEF LINUX}
  sysmem_Read    = PROT_READ;
  sysmem_Write   = PROT_WRITE;
  sysmem_Execute = PROT_EXEC;
  sysmem_Guard   = 0; // Not supported
  sysmem_Default = PROT_READ or PROT_EXEC;
{$ELSE}
  sysmem_Read    = $01;
  sysmem_Write   = $02;
  sysmem_Execute = $04;
  sysmem_Guard   = $08;
  sysmem_Default = $05;
{$ENDIF}

const
  // SysFileOpen_Create flags
  // Flags: If the file already exists; set only one of these
  create_FailIfExists     = $0000;
  create_TruncateIfExists = $0001;

  // Flags: If the file does not exist; set only one of these
  open_FailIfNew          = $0000;  // ocFileOpen fails if no file
  open_CreateIfNew        = $0001;  // ocFileOpen creates file if no file
  open_TruncateIfExists   = $0002;  // ocFileOpen truncates existing file

// Required by the System unit
function SysFileStdIn: Longint;
function SysFileStdOut: Longint;
function SysFileStdErr: Longint;
function SysFileOpen(FileName: PChar; Mode: Longint; var Handle: Longint): Longint;
function SysFileCreate(FileName: PChar; Mode,Attr: Longint; var Handle: Longint): Longint;
function SysFileOpen_Create(Open: Boolean;FileName: PChar; Mode,Attr,Action: Longint; var Handle: Longint): Longint;
function SysFileCopy(_Old, _New: PChar; _Overwrite: Boolean): Boolean;
function SysFileSeek(Handle,Distance,Method: Longint; var Actual: Longint): Longint;
function SysFileRead(Handle: Longint; var Buffer; Count: Longint; var Actual: Longint): Longint;
function SysFileWrite(Handle: Longint; const Buffer; Count: Longint; var Actual: Longint): Longint;
function SysFileSetSize(Handle,NewSize: Longint): Longint;
function SysFileClose(Handle: Longint): Longint;
function SysFileFlushBuffers(Handle: Longint): Longint;
function SysFileDelete(FileName: PChar): Longint;
function SysFileMove(OldName,NewName: PChar): Longint;
function SysFileIsDevice(Handle: Longint): Longint;
function SysDirGetCurrent(Drive: Longint; Path: PChar): Longint;
function SysDirSetCurrent(Path: PChar): Longint;
function SysDirCreate(Path: PChar): Longint;
function SysDirDelete(Path: PChar): Longint;
function SysMemAvail: Longint;
function SysMemAlloc(Size,Flags: Longint; var MemPtr: Pointer): Longint;
function SysMemFree(MemPtr: Pointer): Longint;
function SysSysMsCount: Longint;
procedure SysSysWaitSem(var Sem: Longint);
procedure SysSysSelToFlat(var P: Pointer);
procedure SysSysFlatToSel(var P: Pointer);
function SysCtrlSelfAppType: Longint;
function SysCtrlCreateThread(Attrs: Pointer; StackSize: Longint; Func,Param: Pointer; Flags: Longint; var Tid: Longint): Longint;
function SysCtrlKillThread(Handle: Longint): Longint;
function SysCtrlSuspendThread(Handle: Longint): Longint;
function SysCtrlResumeThread(Handle: Longint): Longint;
function SysGetThreadId: Longint;
function SysGetProcessId: Longint;
procedure SysCtrlExitThread(ExitCode: Longint);
procedure SysCtrlExitProcess(ExitCode: Longint);
function SysCtrlGetModuleName(Handle: Longint; Buffer: PChar): Longint;
procedure SysCtrlEnterCritSec;
procedure SysCtrlLeaveCritSec;
function SysCtrlGetTlsMapMem: Pointer;
function SysCmdln: PChar;
function SysCmdlnCount: Longint;
procedure SysCmdlnParam(Index: Longint; var Param: ShortString);
function SysGetEnvironment: PChar;
procedure SysFreeEnvironment(_Env: PChar);

// Dos, WinDos, SysUtils

type
  TOSSearchRec = packed record
    Handle: Longint;
    NameLStr: Pointer;
    Attr: Byte;
    Time: Longint;
    Size: Longint;
    Name: ShortString;
    Filler: array[0..3] of Char;
{$IFDEF WIN32}
    ExcludeAttr: Longint;
    FindData:    TWin32FindData;
{$ENDIF}
{$IFDEF DPMI32}
    attr_must:byte;
    dos_dta:
      record
        Fill: array[1..21] of Byte;
        Attr: Byte;
        Time: Longint;
        Size: Longint;
        Name: array[0..12] of Char;
      end;
{$ENDIF}
{$IFDEF LINUX}
    FindDir:  array[0..255] of Char;
    FindName: ShortString;
    FindAttr: LongInt;
{$ENDIF}
  end;

  PLongint = ^Longint;  // Define here rather than using Use32 definition
  THandle = Longint;

function SysOsVersion: Longint;
procedure SysGetDateTime(Year,Month,Day,DayOfWeek,Hour,Minute,Second,MSec: PLongint);
procedure SysSetDateTime(Year,Month,Day,Hour,Minute,Second,MSec: PLongint);
function SysVerify(SetValue: Boolean; Value: Boolean): Boolean;
function SysDiskFree(Drive: Byte): Longint;
function SysDiskSize(Drive: Byte): Longint;
function SysDiskFreeLong(Drive: Byte): TQuad;
function SysDiskSizeLong(Drive: Byte): TQuad;
function SysGetFileAttr(FileName: PChar; var Attr: Longint): Longint;
function SysSetFileAttr(FileName: PChar; Attr: Longint): Longint;
function SysGetFileTime(Handle: Longint; var Time: Longint): Longint;
function SysSetFileTime(Handle: Longint; Time: Longint): Longint;
function SysFindFirst(Path: PChar; Attr: Longint; var F: TOSSearchRec; IsPChar: Boolean): Longint;
function SysFindNext(var F: TOSSearchRec; IsPChar: Boolean): Longint;
function SysFindClose(var F: TOSSearchRec): Longint;
function SysFileSearch(Dest,Name,List: PChar): PChar;
function SysFileExpand(Dest,Name: PChar): PChar;
function SysFileAsOS(FileName: PChar): Boolean;
function SysExecute(Path,CmdLine,Env: PChar; Async: Boolean; PID: PLongint; StdIn,StdOut,StdErr: Longint): Longint;
function SysExitCode: Longint;
function SysFileExists(const FileName: PChar): Boolean;

// Memory mapping functions.  The Alloc and Access functions return
// a handle or -1 (invalid).
function SysAllocSharedMemory(var _Base: Pointer; _Name: pChar; _Size: Longint): Longint;
function SysAccessSharedMemory(var _Base: Pointer; _Name: pChar): Longint;
procedure SysFreeSharedMemory(_Base: Pointer; _Handle: Longint);

// Semaphores

function SemCreateEvent(_Name: pChar; _Shared, _State: Boolean): TSemHandle;
function SemAccessEvent(_Name: pChar): TSemHandle;
function SemPostEvent(_Handle: TSemhandle): Boolean;
function SemResetEvent(_Handle: TSemhandle; var _PostCount: Longint): Boolean;
function SemWaitEvent(_Handle: TSemHandle; _TimeOut: Longint): Boolean;
procedure SemCloseEvent(_Handle: TSemHandle);

function SemCreateMutex(_Name: PChar; _Shared, _State: Boolean): TSemHandle;
function SemAccessMutex(_Name: PChar): TSemHandle;
function SemRequestMutex(_Handle: TSemHandle; _TimeOut: Longint): Boolean;
function SemReleaseMutex(_Handle: TSemHandle): Boolean;
procedure SemCloseMutex(_Handle: TSemHandle);

// Memory management

function SysMemInfo(_Base: Pointer; _Size: Longint; var _Flags: Longint): Boolean;
function SysSetMemProtection(_Base: Pointer; _Size: Longint; _Flags: Longint): Boolean;

// GUI

procedure SysMessageBox(_Msg, _Title: PChar; _Error: Boolean);

// VPUtils

type
  TDriveType = ( dtFloppy, dtHDFAT, dtHDHPFS, dtInvalid,
                 dtNovellNet, dtCDRom, dtLAN, dtHDNTFS, dtUnknown,
                 dtTVFS, dtHDExt2, dtHDJFS, dtHDFAT32, dtRAMFS, dtNDFS32);

function SysGetVolumeLabel(Drive: Char): ShortString;
function SysSetVolumeLabel(Drive: Char; _Label: ShortString): Boolean;
function SysGetForegroundProcessId: Longint;
function SysGetBootDrive: Char;
function SysGetDriveType(Drive: Char): TDriveType;
function SysGetVideoModeInfo( Var Cols, Rows, Colours : Word ): Boolean;
function SysSetVideoMode(Cols, Rows: Word): Boolean;
function SysGetVisibleLines( var Top, Bottom: Longint ): Boolean;

// Crt

function SysKeyPressed: boolean;
function SysReadKey: Char;
function SysPeekKey(Var Ch: Char): Boolean;
procedure SysFlushKeyBuf;
procedure SysGetCurPos(var X,Y: SmallWord);
procedure SysWrtCharStrAtt(CharStr: Pointer; Len,X,Y: SmallWord; var Attr: Byte);
function SysReadAttributesAt(x,y: SmallWord): Byte;
function SysReadCharAt(x,y: SmallWord): Char;
procedure SysScrollUp(X1,Y1,X2,Y2,Lines,Cell: SmallWord);
procedure SysScrollDn(X1,Y1,X2,Y2,Lines,Cell: SmallWord);
procedure SysBeepEx(Freq,Dur: LongInt);
{$IFDEF DPMI32}
procedure SysSound(freq:longint);
procedure SysNoSound;
{$ENDIF}

// TVision and Crt

type
  PSysPoint = ^TSysPoint;
  TSysPoint = packed record
    X,Y: SmallInt;
  end;

  PSysRect = ^TSysRect;
  TSysRect = packed record
    A,B: TSysPoint;
  end;

type
  TSysMouseEvent = packed record
    smeTime:    Longint;
    smePos:     TSysPoint;
    smeButtons: Byte;
  end;

  TSysKeyEvent = packed record
    skeKeyCode:    SmallWord;
    skeShiftState: Byte;
  end;

function  SysTVDetectMouse: Longint;
procedure SysTVInitMouse(var X,Y: Integer);
procedure SysTVDoneMouse(Close: Boolean);
procedure SysTVShowMouse;
procedure SysTVHideMouse;
procedure SysTVUpdateMouseWhere(var X,Y: Integer);
function SysTVGetMouseEvent(var Event: TSysMouseEvent): Boolean;
procedure SysTVKbdInit;
function SysTVGetKeyEvent(var Event: TSysKeyEvent): Boolean;
function SysTVPeekKeyEvent(var Event: TSysKeyEvent): Boolean;
function SysTVGetShiftState: Byte;
procedure SysTVSetCurPos(X,Y: Integer);
procedure SysTVSetCurType(Y1,Y2: Integer; Show: Boolean);
procedure SysTVGetCurType(var Y1,Y2: Integer; var Visible: Boolean);
procedure SysTVShowBuf(Pos,Size: Integer);
procedure SysTVClrScr;
function SysTVGetScrMode(_Size: PSysPoint; _Align: Boolean): Integer;
procedure SysTVSetScrMode(Mode: Integer);
function SysTVGetSrcBuf: Pointer;
procedure SysTVInitCursor;
procedure SysTvDoneCursor;
procedure SysCtrlSleep(Delay: Integer);
function SysGetValidDrives: Longint;

// Other

type
  TCtrlBreakHandler = function: Boolean;
  TCharCase = (ccLower, ccUpper, ccAnsiLower, ccAnsiUpper);

const
  CtrlBreakHandler: TCtrlBreakHandler = nil;
  TVVioHandle: Word = 0;

function SysGetCodePage: Longint;
procedure SysCtrlSetCBreakHandler;
function SysFileIncHandleCount(Count: Longint): Longint;
function SysGetSystemSettings: Longint;
function SysCompareStrings(s1, s2: PChar; l1, l2: Longint; IgnoreCase: Boolean): Longint;
procedure SysChangeCase(Source, Dest: PChar; Len: Longint; NewCase: TCharCase);
function SysLowerCase(s: PChar): PChar;
function SysUpperCase(s: PChar): PChar;

// IDE

procedure SysDisableHardErrors;
function SysKillProcess(Process: Longint): Longint;
function SysAllocSharedMem(Size: Longint; var MemPtr: Pointer): Longint;
function SysGiveSharedMem(MemPtr: Pointer): Longint;

function SysPipeCreate(var ReadHandle,WriteHandle: Longint; Size: Longint): Longint;
function SysPipePeek(Pipe: Longint; Buffer: Pointer; BufSize: Longint; var BytesRead: Longint; var IsClosing: Boolean): Longint;
function SysPipeClose(Pipe: Longint): Longint;

// Required by SysUtils unit
const
  open_access_ReadOnly          = $0000; { ---- ---- ---- -000 }
  open_access_WriteOnly         = $0001; { ---- ---- ---- -001 }
  open_access_ReadWrite         = $0002; { ---- ---- ---- -010 }
  open_share_DenyReadWrite      = $0010; { ---- ---- -001 ---- }
  open_share_DenyWrite          = $0020; { ---- ---- -010 ---- }
  open_share_DenyRead           = $0030; { ---- ---- -011 ---- }
  open_share_DenyNone           = $0040; { ---- ---- -100 ---- }

  xcpt_Signal_Ctrl_C =
    {$IFDEF OS2}   xcpt_Signal;               {$ENDIF}
    {$IFDEF WIN32} xcpt_Control_C_exit;       {$ENDIF}
    {$IFDEF DPMI32}xcpt_Ctrl_Break;           {$ENDIF}
    {$IFDEF LINUX} xcpt_Ctrl_Break;           {$ENDIF}

type
  TQuadRec = record
    Lo,Hi: Longint;
  end;

  POSExceptionRecord = ^TOSExceptionRecord;
  TOSExceptionRecord = record
    fExceptionNum: Longint;        { exception number }
    fHandlerFlags: Longint;
    fNestedExceptionRecord: POSExceptionRecord;
    fExceptionAddress: Pointer;
    fParameters: Longint;          { Size of Exception Specific Info }
    fExceptionInfo: array [0..exception_Maximum_Parameters-1] of Longint;
  end;

procedure SysGetCaseMap(TblLen: Longint; Tbl: PChar );
procedure SysGetWeightTable(TblLen: Longint; WeightTable: PChar);
function SysLoadResourceString(ID: Longint; Buffer: PChar; BufSize: Longint): PChar;
function SysFileExpandS(Name: ShortString): ShortString;
function SysGetSystemError(Code: Longint; Buffer: PChar; BufSize: Longint; var MsgLen: Longint): PChar;
function SysGetModuleName(var Address: Pointer; Buffer: PChar; BufSize: Longint): PChar;
function SysFileUNCExpand(Dest,Name: PChar): PChar;
procedure SysGetCurrencyFormat(CString: PChar; var CFormat, CNegFormat, CDecimals: Byte; var CThousandSep, CDecimalSep: Char);
procedure SysGetDateFormat(var DateSeparator: Char; ShortDateFormat,LongDateFormat: PChar);
procedure SysGetTimeFormat(var TimeSeparator: Char; TimeAMString,TimePMString,ShortTimeFormat,LongTimeFormat: PChar);
procedure SysDisplayConsoleError(PopupErrors: Boolean; Title, Msg: PChar);
procedure SysDisplayGUIError(Title, Msg: PChar);
function SysPlatformID: Longint;
function SysPlatformName: String;
function SysPlatformNameForId( _Id: Integer ): String;
procedure SysBeep;
procedure SysLowInitPreTLS;
procedure SysLowInitPostTLS;

// Clipboard interface

function SysClipCanPaste: Boolean;
function SysClipCopy(P: PChar; Size: Longint): Boolean;
function SysClipPaste(var Size: Integer): Pointer;

{$IFDEF DPMI32}
{$ENDIF}

{$IFDEF LINUX}
function SysConvertFileName(Dest, Source: PChar; DestFS, SourceFS: TFileSystem): PChar;
function SysIsValidFileName(FileName: PChar; FileSystem: TFileSystem): Boolean;
{$ENDIF}

{$IFDEF OS2}
// Routines used to safely call 16-bit OS/2 functions
function Invalid16Parm(const _p: Pointer; const _Length: Longint): Boolean;
function Fix_64k(const _Memory: Pointer; const _Length: Longint): pointer;
{$ENDIF}

function SysPathSep: Char;

implementation

{&OrgName-}

uses
  {$Ifdef Win32} {$Ifndef KeyDll}
  VpKbdW32,  // Statically linked default Win32 keyboard handler
  {$Endif} {$Endif}
  {$Ifdef DPMI32}
  Dpmi32, D32Res, // Dpmi support files
  {$Endif}
  {$IFDEF LINUX}
  LnxRes,
  {$ELSE}
  ExeHdr,
  {$ENDIF}
  Strings;

const
  // Max. Amount of TLS memory
  SharedMemSize = 32*1024;

type
  // This type *must* be in sync with System.TSharedMem or *everything* breaks
  PSharedMem = ^TSharedMem;
  TSharedMem = record
    TlsPerThread  : Pointer;        // Actual TLS
    MaxThreadCount: Longint;        // Max thread ID so far
    MaxThreadId   : Longint;        // Updated before MaxThreadCount
    TlsMemMgr     : TMemoryManager; // Memory Manager used by Tls Mgr
    HeapSemaphore : Longint;        // For synchronizing heap access
    HeapLockCount : Longint;
    HeapOwnerTid  : Longint;
  end;

  TDateTimeRec = record
    FTime,FDate: SmallWord;
  end;

procedure SysSysWaitSem(var Sem: Longint); {&USES None} {&FRAME-}
asm
      @@1:
        mov     eax,Sem
   lock bts     [eax].Longint,0
        jnc     @@RET
        push    31              // Wait for at least one timer slice
        Call    SysCtrlSleep    // and try to check again
        jmp     @@1
      @@RET:
end;

function SysFileExpandS(Name: ShortString): ShortString;
begin
  Name[Length(Name)+1] := #0;
  SysFileExpand(@Result[1], @Name[1]);
  SetLength(Result, strlen(@Result[1]));
end;

function SysDiskFree(Drive: Byte): Longint;
var
  Temp: TQuad;
begin
  Temp := SysDiskFreeLong(Drive);
  Result := TQuadRec(Temp).Lo;
  if Temp > MaxLongint then
    Result := MaxLongint;  // Handle overflow
end;

function SysDiskSize(Drive: Byte): Longint;
var
  Temp: TQuad;
begin
  Temp := SysDiskSizeLong(Drive);
  Result := TQuadRec(Temp).Lo;
  if Temp > MaxLongint then
    Result := MaxLongint;  // Handle overflow
end;

function SysPathSep: Char;
begin
{$IFDEF LINUX}
  if FileSystem = fsUnix then
    Result := '/'
  else
    Result := '\';
{$ELSE}
  Result := '\';
{$ENDIF}
end;

function SysPlatformNameForId( _Id: Integer ): String;
begin
  case _Id of
    -3: Result := 'Linux';
    -2: Result := 'DPMI';
    -1: Result := 'OS/2';
     0: Result := 'Win32s';
     1: Result := 'Win9x';
     2: Result := 'WinNT';
  else
    Result := 'Unknown';
  end;
end;

function SysPlatformName: String;
begin
  Result := SysPlatformNameForID( SysPlatformId );
end;

function SysFileExists(const FileName: PChar): Boolean;
var
  S: TOSSearchRec;
begin
  Result := SysFindFirst(FileName, $27, S, True) = 0;
  if Result then
    SysFindClose(S);
end;

// Include platform specific implementations

{$IFDEF OS2}
  {$I VpSysOs2.Pas}
{$ENDIF}

{$IFDEF WIN32}
  {$I VpSysW32.Pas}
{$ENDIF}

{$IFDEF DPMI32}
  {$I VpSysD32.Pas}
{$ENDIF}

{$IFDEF LINUX}
  {$I VpSysLnx.Pas}
{$ENDIF}

end.
