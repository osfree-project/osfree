// |---------------------------------------------------------|
// |                                                         |
// |     Virtual Pascal Runtime Library.  Version 2.1.       |
// |     System Interface Layer for OS/2                     |
// |     ----------------------------------------------------|
// |     Copyright (C) 1995-2003 vpascal.com                 |
// |                                                         |
// |---------------------------------------------------------|

// PmWin and PmShApi function prototypes without Os2PmApi

// If CHECK_NO_PM is defined, the executable is bigger, the
// program temporarily uses more resources, but does not hang
// if PM-functions like SysClipCanPaste are called in an
// environment without Presentation Manager

// By default, we live with this overhead:
{$DEFINE CHECK_NO_PM}

type
  HIni                    = LHandle;
  tClipHack               = (clipInit, clipFailed, clipOk);
  tPMInit                 = (pmUntested, pmOK, pmFailed);

const
  HIni_UserProfile        = HIni(-1);
  hwnd_Desktop            = HWnd(1);
  mb_Ok                   = 0;
  mb_Information          = $0030;
  mb_CUACritical          = $0040;
  mb_Error                = mb_CUACritical;
  mb_Moveable             = $4000;
  wa_Error                = 2;
  cf_Text                 = 1;
  cfi_Pointer             = $0400;
  SIntl: PChar            = 'PM_National';

  // State constants for PM and clipboard hack
  PM_Initialised: tPMInit = pmUntested;
  PM_ClipboardHack: tClipHack = clipInit;

const
{&Cdecl+}
  PM_LoadString:            function(AB: Hab; Module: HModule; Id: ULong; MaxLen: Long; Buffer: PChar): Long = nil;
  PM_CreateMsgQueue:        function(AB: Hab; CMsg: Long): Hmq = nil;
  PM_Initialize:            function(Options: ULong): Hab = nil;
  PM_MessageBox:            function(Parent,Owner: HWnd; Text,Caption: PChar; IdWindow,Style: ULong): ULong = nil;
  PM_Alarm:                 function(Desktop: HWnd; rgfType: ULong): Bool = nil;
  PM_PrfQueryProfileString: function(Ini: HIni; App,Key,Default: PChar; Buffer: Pointer; cchBufferMax: ULong): ULong = nil;
  PM_PrfQueryProfileInt:    function(Ini: HIni; App,Key: PChar; Default: Long): Long = nil;
  PM_WinQueryClipbrdFmtInfo:function(AB: Hab; Fmt: ULong; var FmtInfo: ULong): Bool = nil;
  PM_WinOpenClipbrd:        function(AB: Hab): Bool = nil;
  PM_WinCloseClipbrd:       function(AB: Hab): Bool = nil;
  PM_WinSetClipbrdData:     function(AB: Hab; Data,Fmt,rgfFmtInfo: ULong): Bool = nil;
  PM_WinQueryClipbrdData:   function(AB: Hab; Fmt: ULong): ULong = nil;
{&Cdecl-}
  // Module handles
  dll_PMWIN:   HModule = 0;
  dll_PMSHAPI: HModule = 0;
  // Queue and Anchor block handles
  PM_MsgQueue: Hmq = 0;
  PM_Anchor: Hab = 0;

{ Initialise Win* and Prf* entry points, if Presentation Manager is available }

procedure FreePMModules;
begin
  // Free modules
  if dll_PMWIN <> 0 then
    DosFreeModule(dll_PMWIN);
  if dll_PMSHAPI <> 0 then
    DosFreeModule(dll_PMSHAPI);
end;

procedure InitPMModules;
{$IFDEF CHECK_NO_PM}
const
  {$Far16+}
  Dos16SMPresent  : function(var present:smallword):apiret16 = nil;
  {$Far16-}
{$ENDIF CHECK_NO_PM}
var
  FailedModule: array[0..259] of Char;
{$IFDEF CHECK_NO_PM}
  dll_DOSCALLS: HModule;
  sm_present: smallword;
{$ENDIF CHECK_NO_PM}
begin
  if PM_Initialised in [pmOK,pmFailed] then
    Exit;

  {$IFDEF CHECK_NO_PM}
  // Full-screen session under PM ?
  if (SysCtrlSelfAppType=0) then
    begin
      PM_Initialised:=pmFailed;
      if DosLoadModule(FailedModule, SizeOf(FailedModule), 'DOSCALLS', dll_DOSCALLS) = 0 then
        begin

          sm_present:=0;

          if DosQueryProcAddr(dll_DOSCALLS, 712, nil, @Dos16SMPresent)=no_Error then
            begin
              //if Dos16SMPresent(sm_present)=no_Error then
              //  if sm_present=1 then
              //    PM_Initialised:=pmOK;
              asm
                lea eax,sm_present
                push eax
                push [Dos16SMPresent]
                push 3 // B:0011: Addr
                call _Far16Pas

                cmp ax,no_Error
                jne @done

                cmp [sm_present], 1
                jne @done

                mov [PM_Initialised], pmOK
              @done:
              end;

            end;

          DosFreeModule(dll_DOSCALLS);
        end;

      end;

  // Do not hang on boot disk
  if PM_Initialised=pmFailed then
    Exit;
  {$ENDIF CHECK_NO_PM}

  if DosLoadModule(FailedModule, SizeOf(FailedModule), 'PMWIN', dll_PMWIN) = 0 then
  begin
    DosQueryProcAddr(dll_PMWIN, 781, nil, @PM_LoadString);
    DosQueryProcAddr(dll_PMWIN, 716, nil, @PM_CreateMsgQueue);
    DosQueryProcAddr(dll_PMWIN, 763, nil, @PM_Initialize);
    DosQueryProcAddr(dll_PMWIN, 789, nil, @PM_MessageBox);
    DosQueryProcAddr(dll_PMWIN, 701, nil, @PM_Alarm);
    DosQueryProcAddr(dll_PMWIN, 807, nil, @PM_WinQueryClipbrdFmtInfo);
    DosQueryProcAddr(dll_PMWIN, 793, nil, @PM_WinOpenClipbrd);
    DosQueryProcAddr(dll_PMWIN, 707, nil, @PM_WinCloseClipbrd);
    DosQueryProcAddr(dll_PMWIN, 854, nil, @PM_WinSetClipbrdData);
    DosQueryProcAddr(dll_PMWIN, 806, nil, @PM_WinQueryClipbrdData);
  end;
  if DosLoadModule(FailedModule, SizeOf(FailedModule), 'PMSHAPI', dll_PMSHAPI) = 0 then
  begin
    DosQueryProcAddr(dll_PMSHAPI, 115, nil, @PM_PrfQueryProfileString);
    DosQueryProcAddr(dll_PMSHAPI, 114, nil, @PM_PrfQueryProfileInt);
  end;
  PM_Initialised := pmOK;
  AddExitProc(FreePMModules);
end;

function WinLoadString(AB: Hab; Module: HModule; Id: ULong; MaxLen: Long; Buffer: PChar): Long;
var
  Stringtable_Resource: Pointer;
  Search: PChar;
begin
  InitPMModules;
  if Assigned(PM_LoadString) then
    Result := PM_LoadString(AB, Module, Id, MaxLen, Buffer)
  else
    begin (* try to do the work in bootdisk mode *)
      if DosGetResource(Module,rt_String,Id div 16+1,Stringtable_Resource)=0 then
        begin
          Search:=Stringtable_Resource;
          Inc(Longint(Search),SizeOf(SmallWord)); (* skip/ignore codepage *)
          Id := Id mod 16; (* string number in this package *)
          while Id > 0 do
            begin
              Dec(Id); (* skip one string: Length byte+Length *)
              Inc(Longint(Search) ,1+Ord(Search[0]) );
            end;
          StrLCopy(Buffer, @Search[1], MaxLen ); (* @Search[1] is #0 terminated *)
          DosFreeResource(Stringtable_Resource);
          Result := StrLen(Buffer);
        end
      else
        Result := 0;      // Return string length 0
    end;
end;

function WinCreateMsgQueue(AB: Hab; CMsg: Long): Hmq;
var
  TB: PTIB;
  PB: PPIB;
  org_Pib_ulType: uLong;
begin
  if PM_MsgQueue <> NULLHANDLE then
    Result := PM_MsgQueue
  else
    begin
      InitPMModules;
      if Assigned(PM_CreateMsgQueue) then
        begin
          Result := PM_CreateMsgQueue(AB, CMsg);
          if (Result = NULLHANDLE) and (AB <> 0) and
             (PM_Clipboardhack = clipInit) and IsConsole then
            begin
              // Attempt to force OS/2 into believing we're a PM app
              // so we can create a message queue
              PM_Clipboardhack := clipFailed;
              DosGetInfoBlocks(TB, PB);

              // Save program type and override it as PM
              org_Pib_ulType := PB^.Pib_ulType;
              PB^.Pib_ulType := 3;

              // Create queue and restore the program type
              Result := PM_CreateMsgQueue(AB, CMsg);
              PB^.Pib_ulType := org_Pib_ulType;
              if Result <> NULLHANDLE then
                PM_ClipboardHack := clipOK;
            end;
          PM_MsgQueue := Result;
        end
      else
        Result := $1051;  // pmErr_Not_in_a_XSession
    end;
end;

function WinInitialize(Options: ULong): Hab;
begin
  Result := PM_Anchor;
  if Result = 0 then
    begin
      InitPMModules;
      if Assigned(PM_Initialize) then
        begin
          Result := PM_Initialize(Options);
          PM_Anchor := Result;
        end
      else
        Result := 0;
    end;
end;

function WinMessageBox(Parent,Owner: HWnd; Text,Caption: PChar; IdWindow,Style: ULong): ULong;
begin
  InitPMModules;
  if Assigned(PM_MessageBox) then
    Result := PM_MessageBox(Parent, Owner, Text, Caption, IdWindow, Style)
  else
    Result := $FFFF;   // mbid_Error
end;

function WinQueryClipbrdFmtInfo(AB: Hab; Fmt: ULong; var FmtInfo: ULong): Bool;
begin
  InitPMModules;
  if Assigned(PM_WinQueryClipbrdFmtInfo) then
    Result := PM_WinQueryClipbrdFmtInfo(AB, Fmt, FmtInfo)
  else
    Result := False;
end;

function WinOpenClipbrd(AB: Hab): Bool;
begin
  InitPMModules;
  if Assigned(PM_WinOpenClipbrd) then
    Result := PM_WinOpenClipbrd(AB)
  else
    Result := False;
end;

function WinCloseClipbrd(AB: Hab): Bool;
begin
  InitPMModules;
  if Assigned(PM_WinCloseClipbrd) then
    Result := PM_WinCloseClipbrd(AB)
  else
    Result := False;
end;

function WinSetClipbrdData(AB: Hab; Data,Fmt,rgfFmtInfo: ULong): Bool;
begin
  InitPMModules;
  if Assigned(PM_WinSetClipbrdData) then
    Result := PM_WinSetClipbrdData(AB, Data, Fmt, rgfFmtInfo)
  else
    Result := False;
end;

function WinQueryClipbrdData(AB: Hab; Fmt: ULong): ULong;
begin
  InitPMModules;
  if Assigned(PM_WinQueryClipbrdData) then
    Result := PM_WinQueryClipbrdData(AB, Fmt)
  else
    Result := 0;
end;

function WinAlarm(Desktop: HWnd; rgfType: ULong): Bool;
begin
  InitPMModules;
  if Assigned(PM_Alarm) then
    Result := PM_Alarm(Desktop, rgfType)
  else
    Result := False;
end;

function PrfQueryProfileInt(Ini: HIni; App,Key: PChar; Default: Long): Long;
begin
  InitPMModules;
  if Assigned(PM_PrfQueryProfileInt) then
    Result := PM_PrfQueryProfileInt(Ini, App, Key, Default)
  else
    Result := Default;
end;

function PrfQueryProfileString(Ini: HIni; App,Key,Default: PChar; Buffer: Pointer; cchBufferMax: ULong): ULong;
begin
  InitPMModules;
  if Assigned(PM_PrfQueryProfileString) then
    Result := PM_PrfQueryProfileString(Ini, App, Key, Default, Buffer, cchBufferMax)
  else
    begin
      StrLCopy(Buffer, Default, cchBufferMax);
      Result := StrLen(Buffer) + 1;
    end;
end;

// Other non-Presentation Manager OS/2 functions

// Protect parameters of 16 bit functions to wrap around 64KB

function Invalid16Parm(const _p: Pointer; const _Length: Longint): Boolean;
begin
  Result := (Longint(_p) and $0000ffff) + _Length >= $00010000;
end;

function Fix_64k(const _Memory: Pointer; const _Length: Longint): pointer;
begin
  // Test if memory crosses segment boundary
  if Invalid16Parm(_Memory, _Length) then
    // It does: Choose address in next segment
    Fix_64k := Ptr((Ofs(_memory) and $ffff0000) + $00010000)
  else
    // It doesn't: return original pointer
    Fix_64k := _Memory;
end;


function SysFileStdIn: Longint;
begin
  Result := 0;
end;

function SysFileStdOut: Longint;
begin
  Result := 1;
end;

function SysFileStdErr: Longint;
begin
  Result := 2;
end;

function SysFileOpen_Create(Open: Boolean;FileName: PChar; Mode,Attr,Action: Longint; var Handle: Longint): Longint;
var
  APIFlags: Longint;
  ActionTaken: Longint;
begin
  APIFlags := 0;
  if Open then
    if Action and open_CreateIfNew <> 0 then
      APIFlags := open_action_create_if_new or open_action_open_if_exists
    else if Action and open_TruncateIfExists <> 0 then
      APIFlags := open_action_fail_if_new or open_action_replace_if_exists
    else
      APIFlags := open_action_open_if_exists or open_action_fail_if_new
  else
    if Action and create_TruncateIfExists <> 0 then
      APIFlags := open_action_create_if_new or open_action_replace_if_exists
    else
      APIFlags := open_action_create_if_new or open_action_fail_if_exists;

  if (Mode and $70) = 0 then
    Inc(Mode, open_share_DenyNone);
  Result := DosOpen(FileName, Handle, ActionTaken, 0, 0, APIFlags, Mode, nil);
end;

function SysFileOpen(FileName: PChar; Mode: Longint; var Handle: Longint): Longint;
var
  Action: Longint;
begin
  if (Mode and $70) = 0 then
    Inc(Mode, open_share_DenyNone);
  Result := DosOpen(FileName, Handle, Action, 0, 0, file_Open, Mode, nil);
end;

function SysFileCreate(FileName: PChar; Mode,Attr: Longint; var Handle: Longint): Longint;
var
  Action: Longint;
begin
  if (Mode and $70) = 0 then
    Inc(Mode, open_share_DenyNone);
  Result := DosOpen(FileName, Handle, Action, 0, Attr, file_Create+file_Truncate, Mode, nil);
end;

function SysFileCopy(_Old, _New: PChar; _Overwrite: Boolean): Boolean;
var
  Flag: Longint;
begin
  if _Overwrite then
    Flag := dcpy_existing
  else
    Flag := 0;
  Result := (DosCopy(_Old, _New, Flag)=No_Error);
end;

function SysFileSeek(Handle,Distance,Method: Longint; var Actual: Longint): Longint;
begin
  Result := DosSetFilePtr(Handle, Distance, Method, Actual);
end;

function SysFileRead(Handle: Longint; var Buffer; Count: Longint; var Actual: Longint): Longint;
begin
  Result := DosRead(Handle, Buffer, Count, Actual);
end;

function SysFileWrite(Handle: Longint; const Buffer; Count: Longint; var Actual: Longint): Longint;
begin
  Result := DosWrite(Handle, Buffer, Count, Actual);
end;

function SysFileSetSize(Handle,NewSize: Longint): Longint;
begin
  Result := DosSetFileSize(Handle, NewSize);
end;

function SysFileClose(Handle: Longint): Longint;
begin
  Result := 0;
  if (Handle > 2) or (Handle < 0) then
    Result := DosClose(Handle);
end;

function SysFileFlushBuffers(Handle: Longint): Longint;
begin
  Result := DosResetBuffer(Handle);
end;

function SysFileDelete(FileName: PChar): Longint;
begin
  Result := DosDelete(FileName);
end;

function SysFileMove(OldName,NewName: PChar): Longint;
begin
  Result := DosMove(OldName, NewName);
end;

function SysFileIsDevice(Handle: Longint): Longint;
var
  HandleType,Flags: Longint;
begin
  if DosQueryHType(Handle, HandleType, Flags) <> 0 then
    Result := 0
  else
    Result := HandleType; // 0=File, 1=Character device, 2=Pipe
end;

function SysDirGetCurrent(Drive: Longint; Path: PChar): Longint;
var
  P: PChar;
  X: Longint;
begin
  if Drive = 0 then
    DosQueryCurrentDisk(Drive, X);
  P := Path;
  P^ := Chr(Drive + (Ord('A') - 1));
  Inc(P);
  P^ := ':';
  Inc(P);
  P^ := '\';
  Inc(P);
  X := 260 - 4;         // 4: 'D:\', #0
  Result := DosQueryCurrentDir(Drive, P^, X);
end;

function SysDirSetCurrent(Path: PChar): Longint;
var
  P: PChar;
begin
  P := Path;
  Result := 0;
  if P^ <> #0 then
  begin
    if P[1] = ':' then
    begin
      Result := DosSetDefaultDisk((Ord(P^) and $DF) - (Ord('A') - 1));
      if Result <> 0 then
        Exit;
      Inc(P, 2);
      if P^ = #0 then         // "D:",0  ?
        Exit;                 // yes, exit
    end;
    Result := DosSetCurrentDir(P);
  end;
end;

function SysDirCreate(Path: PChar): Longint;
begin
  Result := DosCreateDir(Path, nil);
end;

function SysDirDelete(Path: PChar): Longint;
begin
  Result := DosDeleteDir(Path);
end;

// from vputils.pas
Function Min( a,b : Longint ) : Longint; inline;
  begin
    if a < b then
      Min := a
    else
      Min := b;
  end;

Function Max( a,b : Longint ) : Longint; inline;
  begin
    if a > b then
      Max := a
    else
      Max := b;
  end;

function SysMemAvail: Longint;
var
  meminfo:
    packed record
      TotPhysMem,
      TotResMem,
      TotAvailMem,
      MaxPrMem    :ULong;
    end;

begin
  // qsv_TotAvailMem is unreliable like:
  // qsv_TotAvailMem=-6MB, qsv_MaxPrMem=316MB
  DosQuerySysInfo(qsv_TotPhysMem, qsv_MaxPrMem, meminfo, SizeOf(meminfo));
  with meminfo do
    begin
      if MaxPrMem<0 then MaxPrMem:=High(Longint);
      // this formula does not thinks about swapfile memory, but
      // assumes that ther should be 0.5 MB available for it,
      // if address space (MaxPrMem) is availabe.
      Result:=Min(Max(TotPhysMem-TotResMem,512*1024),MaxPrMem);
    end;
end;

function SysMemAlloc(Size,Flags: Longint; var MemPtr: Pointer): Longint;
begin
  Result := DosAllocMem(MemPtr, Size, Flags);
end;

function SysMemFree(MemPtr: Pointer): Longint;
begin
  Result := DosFreeMem(MemPtr);
end;

function SysSysMsCount: Longint;
begin
  DosQuerySysInfo(qsv_Ms_Count, qsv_Ms_Count, Result, SizeOf(Result));
end;

procedure SysSysSelToFlat(var P: Pointer); {&USES ebx} {&FRAME-}
asm
        mov     ebx,P
        mov     eax,[ebx]
        Call    DosSelToFlat
        mov     [ebx],eax
end;

procedure SysSysFlatToSel(var P: Pointer); {&USES ebx} {&FRAME-}
asm
        mov     ebx,P
        mov     eax,[ebx]
        Call    DosFlatToSel
        mov     [ebx],eax
end;

function SysCtrlSelfAppType: Longint;
var
  TB: PTIB;
  PB: PPIB;
begin
  DosGetInfoBlocks(TB, PB);
  Result := PB^.Pib_ulType;
end;

function SysGetThreadId: Longint;
var
  TB: PTIB;
  PB: PPIB;
begin
  DosGetInfoBlocks(TB, PB);
  Result := TB^.tib_ordinal;
end;

function SysCtrlCreateThread(Attrs: Pointer; StackSize: Longint; Func,Param: Pointer; Flags: Longint; var Tid: Longint): Longint;
begin
  Result := DosCreateThread(Tid, FnThread(Func), Longint(Param), Flags, StackSize);
  if Result <> 0 then
    Tid := 0;
end;

function SysCtrlKillThread(Handle: Longint): Longint;
begin
  Result := DosKillThread(Handle);
end;

function SysCtrlSuspendThread(Handle: Longint): Longint;
begin
  Result := DosSuspendThread(Handle);
end;

function SysCtrlResumeThread(Handle: Longint): Longint;
begin
  Result := DosResumeThread(Handle);
end;

procedure SysCtrlExitThread(ExitCode: Longint);
begin
  DosExit(exit_Thread, ExitCode);
end;

procedure SysCtrlExitProcess(ExitCode: Longint);
begin
  DosExit(exit_Process, ExitCode);
end;

function SysCtrlGetModuleName(Handle: Longint; Buffer: PChar): Longint;
begin
  Result := DosQueryModuleName(0, 260, Buffer);
end;

procedure SysCtrlEnterCritSec;
begin
  DosEnterCritSec;
end;

procedure SysCtrlLeaveCritSec;
begin
  DosExitCritSec;
end;

function GetParamStr(P: PChar; var Param: String): PChar;
var
  Len: Longint;
begin
  Result := P;
  repeat
    while Result^ in [#1..' '] do
      Inc(Result);
    if PSmallWord(Result)^ = (Ord('"') shl 8 + Ord('"')) then
      Inc(Result, 2)
    else
      Break;
  until False;
  Len := 0;
  while Result^ > ' ' do
    if Result^ = '"' then
      begin
        Inc(Result);
        while not (Result^ in [#0,'"']) do
        begin
          Inc(Len);
          Param[Len] := Result^;
          Inc(Result);
        end;
        if Result^ <> #0 then
          Inc(Result);
      end
    else
      begin
        Inc(Len);
        Param[Len] := Result^;
        Inc(Result);
      end;
  Param[0] := Chr(Len);
end;

function SysCmdlnCount: Longint;
var
  P: PChar;
  S: String;
begin
  P := SysCmdln;
  Result := -1;
  repeat
    P := GetParamStr(P, S);
    if S = '' then
    begin
      if Result < 0 then
        Result := 0;
      Exit;
    end;
    Inc(Result);
    if Result = 0 then // Skip the first #0
      Inc(P);
  until False;
end;

procedure SysCmdlnParam(Index: Longint; var Param: ShortString);
var
  P: PChar;
  Len: Integer;
begin
  P := SysCmdln;
  if Index = 0 then
    begin
      Len := 0;
      Dec(P, 2);
      while P^ <> #0 do
      begin
        Dec(P);
        Inc(Len);
      end;
      SetString(Param, P + 1, Len);
    end
  else
    begin
      P := GetParamStr(P, Param);
      Inc(P);
      Dec(Index);
      repeat
        P := GetParamStr(P, Param);
        if (Index = 0) or (Param = '') then
          Exit;
        Dec(Index);
      until False;
    end;
end;

function SysCmdln: PChar;
var
  TB: PTIB;
  PB: PPIB;
begin
  DosGetInfoBlocks(TB, PB);
  Result := PB^.Pib_pchCmd;
end;

function SysGetProcessId: Longint;
var
  TB: PTIB;
  PB: PPIB;
begin
  DosGetInfoBlocks(TB, PB);
  Result := PB^.Pib_ulPid;
end;

function SysCtrlGetTlsMapMem: Pointer;
var
  TB: PTIB;
  PB: PPIB;
  SharedMemName: record
    L0: Longint;
    L1: Longint;
    L2: Longint;
    ID: array[0..11] of Char;
  end;
begin
  DosGetInfoBlocks(TB, PB);
  SharedMemName.L0 := Ord('\') + Ord('S') shl 8 + Ord('H') shl 16 + Ord('A') shl 24;
  SharedMemName.L1 := Ord('R') + Ord('E') shl 8 + Ord('M') shl 16 + Ord('E') shl 24;
  SharedMemName.L2 := Ord('M') + Ord('\') shl 8 + Ord('V') shl 16 + Ord('S') shl 24;
  Str(PB^.Pib_ulPid, SharedMemName.ID);
  if DosGetNamedSharedMem(Result, PChar(@SharedMemName), pag_Read + pag_Write) <> 0 then
  begin
    DosAllocSharedMem(Result, PChar(@SharedMemName), SharedMemSize, pag_Read+pag_Write+pag_Commit);
    FillChar(Result^, SharedMemSize, $FF);
    FillChar(Result^, SizeOf(TSharedMem), 0);
    // Set up pointers to functions to use when allocating memory
    System.GetMemoryManager( PSharedMem(Result)^.TlsMemMgr );
  end;
end;

function SysGetEnvironment: PChar;
var
  TB: PTIB;
  PB: PPIB;
begin
  DosGetInfoBlocks(TB, PB);
  Result := PB^.Pib_pchEnv;
end;

procedure SysFreeEnvironment(_Env: PChar);
begin
  // Nothing; the environment does not need freeing
end;

function SysOsVersion: Longint;
var
  Version: array [0..1] of Longint;
begin
  DosQuerySysInfo(qsv_Version_Major, qsv_Version_Minor, Version, SizeOf(Version));
  Result := Version[0] + Version[1] shl 8;
end;

function SysPlatformID: Longint;
begin
  Result := -1; // -1 = OS/2
end;

procedure SysGetDateTime(Year,Month,Day,DayOfWeek,Hour,Minute,Second,MSec: PLongint);
var
  DT: Os2Base.DateTime;
begin
  DosGetDateTime(DT);
  if Year <> nil then Year^ := DT.Year;
  if Month <> nil then Month^ := DT.Month;
  if Day <> nil then Day^ := DT.Day;
  if DayOfWeek <> nil then DayOfWeek^ := DT.WeekDay;
  if Hour <> nil then Hour^ := DT.Hours;
  if Minute <> nil then Minute^ := DT.Minutes;
  if Second <> nil then Second^ := DT.Seconds;
  if MSec <> nil then MSec^ := DT.Hundredths*10;
end;

procedure SysSetDateTime(Year,Month,Day,Hour,Minute,Second,MSec: PLongint);
var
  DT: Os2Base.DateTime;
begin
  DosGetDateTime(DT);
  if Year <> nil then DT.Year := Year^;
  if Month <> nil then DT.Month := Month^;
  if Day <> nil then DT.Day := Day^;
  if Hour <> nil then DT.Hours := Hour^;
  if Minute <> nil then DT.Minutes := Minute^;
  if Second <> nil then DT.Seconds := Second^;
  if MSec <> nil then DT.Hundredths := MSec^ div 10;
  DosSetDateTime(DT);
end;

function SysVerify(SetValue: Boolean; Value: Boolean): Boolean;
var
  Flag: LongBool;
begin
  if SetValue then
    Result := DosSetVerify(Value) = 0
  else
    begin
      DosQueryVerify(Flag);
      Result := Flag;
    end;
end;

function SysDiskFreeLong(Drive: Byte): TQuad;
var
  Info: FsAllocate;
begin
  if DosQueryFSInfo(Drive, fsil_Alloc, Info, SizeOf(Info)) = 0 then
    Result := 1.0 * Info.cUnitAvail * Info.cSectorUnit * Info.cbSector
  else
    Result := -1;
end;

function SysDiskSizeLong(Drive: Byte): TQuad;
var
  Info: FsAllocate;
begin
  if DosQueryFSInfo(Drive, fsil_Alloc, Info, SizeOf(Info)) = 0 then
    Result := 1.0 * Info.cUnit * Info.cSectorUnit * Info.cbSector
  else
    Result := -1;
end;

function SysGetFileAttr(FileName: PChar; var Attr: Longint): Longint;
var
  Info: FileStatus3;
begin
  Attr := 0;
  Result := DosQueryPathInfo(FileName, fil_Standard, Info, SizeOf(Info));
  if Result = 0 then
    Attr := Info.attrFile;
end;

function SysSetFileAttr(FileName: PChar; Attr: Longint): Longint;
var
  Info: FileStatus3;
begin
  Result := DosQueryPathInfo(FileName, fil_Standard, Info, SizeOf(Info));
  if Result = 0 then
  begin
    Info.attrFile := Attr;
    Result := DosSetPathInfo(FileName, fil_Standard, Info, SizeOf(Info), dspi_WrtThru);
  end;
end;

function SysGetFileTime(Handle: Longint; var Time: Longint): Longint;
var
  Info: FileStatus3;
  FDateTime: TDateTimeRec absolute Time;
begin
  Time := 0;
  Result := DosQueryFileInfo(Handle, fil_Standard, Info, SizeOf(Info));
  if Result = 0 then
    with FDateTime do
    begin
      FTime := Info.ftimeLastWrite;
      FDate := Info.fdateLastWrite;
    end
end;

function SysSetFileTime(Handle: Longint; Time: Longint): Longint;
var
  Info: FileStatus3;
  FDateTime: TDateTimeRec absolute Time;
begin
  Result := DosQueryFileInfo(Handle, fil_Standard, Info, SizeOf(Info));
  if Result = 0 then
    with FDateTime do
    begin
      Info.ftimeLastWrite := FTime;
      Info.fdateLastWrite := FDate;
      Result := DosSetFileInfo(Handle, fil_Standard, Info, SizeOf(Info));
    end;
end;

function SysFindFirst(Path: PChar; Attr: Longint; var F: TOSSearchRec; IsPChar: Boolean): Longint;
var
  Count: Longint;
  SR: FileFindBuf3;
  Path2: array[0..259] of char;
begin
  Attr := Attr and not $8; // No VolumeID under OS/2
  Count := 1;
  F.Handle := hdir_Create;
  Result := DosFindFirst(Path, F.Handle, Attr, SR, SizeOf(SR), Count, fil_Standard);

  // If a specific error occurs, and the call is to look for directories, and
  // the path is a UNC name, then retry
  if (Result = msg_Net_Dev_Type_Invalid) and
     (Hi(Attr) = $10) and
     (StrLen(Path) > Length('\\')) and
     (StrLComp(Path, '\\', Length('\\')) = 0) then
    begin
      DosFindClose(F.Handle);
      StrCat(StrCopy(Path2,Path), '\*.*');
      Result := DosFindFirst(Path2, F.Handle, Attr, SR, SizeOf(SR), Count, fil_Standard);
      if (Result = 0) and (Count <> 0) then
        Result := 0;
    end;

  if Result = 0 then
    with F,SR do
    begin
      Attr := attrFile;
      TDateTimeRec(Time).FTime := ftimeLastWrite;
      TDateTimeRec(Time).FDate := fdateLastWrite;
      Size := cbFile;
      if IsPChar then
        StrPCopy(PChar(@Name), achName)
      else
        Name := achName;
    end
  else
    F.Handle := hdir_Create;
end;

function SysFindNext(var F: TOSSearchRec; IsPChar: Boolean): Longint;
var
  Count: Longint;
  SR: FileFindBuf3;
begin
  Count := 1;
  Result := DosFindNext(F.Handle, SR, SizeOf(SR), Count);
  if Result = 0 then
    with F,SR do
    begin
      Attr := attrFile;
      TDateTimeRec(Time).FTime := ftimeLastWrite;
      TDateTimeRec(Time).FDate := fdateLastWrite;
      Size := cbFile;
      if IsPChar then
        StrPCopy(PChar(@Name), achName)
      else
        Name := achName;
    end;
end;

function SysFindClose(var F: TOSSearchRec): Longint;
begin
  if F.Handle = hdir_Create then
    Result := 0
  else
    Result := DosFindClose(F.Handle);
end;

// Check if file exists; if it does, update FileName parameter
// to include correct case of existing file
function SysFileAsOS(FileName: PChar): Boolean;
var
  SRec: TOSSearchRec;
  P: PChar;
begin
  Result := False;
  if SysFindFirst(FileName, $37, SRec, False) = 0 then
    begin
      if SRec.Name[1] <> #0 then
        begin
          // Replace '/' with '\'
          repeat
            P:= StrRScan(FileName, '/');
            if P = nil then Break;
            P[0] := '\';
          until False;

          // Replace filename part with data returned by OS
          P := StrRScan(FileName, '\');
          if P = nil then
            P := FileName
          else
            inc(P); // Point to first character of file name
          strPcopy(P, SRec.Name);
        end;
      SysFindClose(SRec );
      Result := True;
    end;
end;

function SysFileSearch(Dest,Name,List: PChar): PChar;
var
  Info: FileStatus3;
begin
  if (DosQueryPathInfo(Name, fil_Standard, Info, SizeOf(Info)) = 0)
    and ((Info.attrFile and file_Directory) = 0) then
    SysFileExpand(Dest, Name)
  else
    if DosSearchPath(dsp_ImpliedCur+dsp_IgnoreNetErr,List,Name,Dest,260) <> 0 then
      Dest[0] := #0;
  SysFileAsOS( Dest );
  Result := Dest;
end;

function SysFileExpand(Dest,Name: PChar): PChar;
var
  I,J,L: Integer;
  C: Char;
  CurDir: array[0..259] of Char;

  procedure AdjustPath;
  begin
    if (Dest[J-2] = '\') and (Dest[J-1] = '.') then
      Dec(J,2)
    else
      if (j>3) and (Dest[J-3] = '\') and (Dest[J-2] = '.') and (Dest[J-1] = '.') then
      begin
        Dec(J, 3);
        if Dest[J-1] <> ':' then
        repeat
          Dec(J);
        until Dest[J] = '\';
      end;
  end;

begin // SysFileExpand
  L := StrLen(Name);
  if (L >= Length('\\')) and (Name[0] = '\') and (Name[1] = '\') then
    StrCopy(Dest, Name)         // '\\SERVER\DIR'
  else
    if (L >= Length('X:')) and (Name[1] = ':') then
      begin                     // Path is already in form 'X:\Path' or 'X:/Path'
        if (L >= Length('X:\')) and (Name[2] in ['\','/']) then
          StrCopy(Dest, Name)
        else
          begin                 // Path is in form 'X:Path'
            SysDirGetCurrent(Ord(UpCase(Name[0])) - (Ord('A') - 1), CurDir);
            if StrLen(CurDir) > Length('X:\') then
              StrCat(CurDir, '\');
            StrLCat(StrCopy(Dest, CurDir), @Name[2], 259);
          end;
      end
    else
      begin                         // Path is without drive letter
        SysDirGetCurrent(0, CurDir);// Get default drive & directory
        if StrLen(CurDir) > Length('X:\') then
          StrCat(CurDir, '\');
        if Name[0] in ['\','/'] then
          StrLCopy(Dest, @CurDir[0], 2) // 'X:' only
        else
          StrCopy(Dest, CurDir);
        StrLCat(Dest, Name, 259);
      end;

  J := 0;
  for I := 0 to StrLen(Dest)-1 do
    begin
      C := Dest[I];
      if C = '/' then
        begin
          C := '\';
          Dest[I] := C;
        end;
      if C = '\' then AdjustPath;
      Dest[J] := C;
      Inc(J);
    end;
  AdjustPath;
  if Dest[J-1] = ':' then
  begin
    Dest[J] := '\';
    Inc(J);
  end;
  Dest[J] := #0;
  Result := Dest;
end;

threadvar
  ExecResult: ResultCodes;
  LastAsync:  Boolean;

function SysExecute(Path,CmdLine,Env: PChar; Async: Boolean; PID: PLongint; StdIn,StdOut,StdErr: Longint): Longint;
var
  P,Os2CmdLine: PChar;
  I,ExecFlags: Longint;
  FailedObj:  array [0..259] of Char;
  CmdLineBuf: array [0..1024*8-1] of Char;
  StdHandles: array[0..2] of Longint;
  NewHandles: array[0..2] of Longint;
  OldHandles: array[0..2] of Longint;
begin
  StdHandles[0] := StdIn;
  StdHandles[1] := StdOut;
  StdHandles[2] := StdErr;
  LastAsync := Async;
  ExecFlags := exec_Sync;
  if Async then
    ExecFlags := exec_AsyncResult;
  Os2CmdLine := CmdLineBuf;
  // Work around a bug in OS/2: Argument to DosExecPgm should not cross 64K boundary
  if ((Longint(Os2CmdLine) + 1024) and $FFFF) < 1024 then
    Inc(Os2CmdLine, 1024);
  P := StrECopy(Os2CmdLine, Path);      // 'Path'#0
  P := StrECopy(P+1, CmdLine);          // 'Path'#0'CommandLine'#0
  P[1] := #0;                           // 'Path'#0'CommandLine'#0#0
  for I := 0 to 2 do
    if StdHandles[I] <> -1 then
    begin
      OldHandles[I] := $FFFFFFFF;       // Save original StdIn to OldIn
      NewHandles[I] := I;
      DosDupHandle(NewHandles[I], OldHandles[I]);
      DosDupHandle(StdHandles[I], NewHandles[I]);
    end;
  Result := DosExecPgm(FailedObj, SizeOf(FailedObj), ExecFlags, Os2CmdLine,
    Env, ExecResult, Path);
  for I := 0 to 2 do
    if StdHandles[I] <> -1 then
    begin
      DosDupHandle(OldHandles[I], NewHandles[I]);
      SysFileClose(OldHandles[I]);
    end;
  if Async and (PID <> nil) then
    PID^ := ExecResult.codeTerminate;
end;

function SysExitCode: Longint;
var
  RetPid: Longint;
begin
  if LastAsync then
    DosWaitChild(dcwa_Process, dcww_Wait, ExecResult, RetPid, ExecResult.codeTerminate);
  Result := ExecResult.codeResult;
  if ExecResult.codeTerminate <> tc_Exit then
    Result := -1;
end;

type
  TCharCaseTable = array[0..255] of Char;
var
  UpperCaseTable: TCharCaseTable;
  LowerCaseTable: TCharCaseTable;
  AnsiUpperCaseTable: TCharCaseTable;
  AnsiLowerCaseTable: TCharCaseTable;
  WeightTable: TCharCaseTable;
const
  CaseTablesInitialized: Boolean = False;

procedure InitCaseTables;
var
  I,J: Integer;
begin
  for I := 0 to 255 do
  begin
    UpperCaseTable[I] := Chr(I);
    LowerCaseTable[I] := Chr(I);
    AnsiUpperCaseTable[I] := Chr(I);
    AnsiLowerCaseTable[I] := Chr(I);
    if I in [Ord('A')..Ord('Z')] then
      LowerCaseTable[I] := Chr(I + (Ord('a')-Ord('A')));
    if I in [Ord('a')..Ord('z')] then
      UpperCaseTable[I] := Chr(I - (Ord('a')-Ord('A')));
  end;
  SysGetCaseMap(SizeOf(AnsiUpperCaseTable), AnsiUpperCaseTable);
  for I := 255 downto 0 do
  begin
    J := Ord(AnsiUpperCaseTable[I]);
    if (J <> I) {and (AnsiLowerCaseTable[J] <> chr(J))} then
      AnsiLowerCaseTable[J] := Chr(I);
  end;
  SysGetWeightTable(SizeOf(WeightTable), WeightTable);
  CaseTablesInitialized := True;
end;

procedure ConvertCase(S1,S2: PChar; Count: Integer; var Table: TCharCaseTable); {&USES esi,edi} {&FRAME-}
asm
                cmp     CaseTablesInitialized,0
                jne     @@1
                Call    InitCaseTables
              @@1:
                xor     eax,eax
                mov     esi,S1
                mov     edi,S2
                mov     ecx,Count
                mov     edx,Table
                jecxz   @@3
              @@2:
                dec     ecx
                mov     al,[esi+ecx]
                mov     al,[edx+eax]
                mov     [edi+ecx],al
                jnz     @@2
              @@3:
end;

procedure SysChangeCase(Source, Dest: PChar; Len: Longint; NewCase: TCharCase);
begin
  case NewCase of
    ccLower:     ConvertCase(Source, Dest, Len, LowerCaseTable);
    ccUpper:     ConvertCase(Source, Dest, Len, UpperCaseTable);
    ccAnsiLower: ConvertCase(Source, Dest, Len, AnsiLowerCaseTable);
    ccAnsiUpper: ConvertCase(Source, Dest, Len, AnsiUpperCaseTable);
  end;
end;

function SysLowerCase(s: PChar): PChar;
begin
  ConvertCase(s, s, strlen(s), AnsiLowerCaseTable);
  Result := s;
end;

function SysUpperCase(s: PChar): PChar;
begin
  ConvertCase(s, s, strlen(s), AnsiUpperCaseTable);
  Result := s;
end;

function MemComp(P1,P2: Pointer; L1,L2: Integer; T1,T2: PChar): Integer; {&USES ebx,esi,edi,ebp} {&FRAME-}
asm
                cmp     CaseTablesInitialized,0
                jne     @@0
                Call    InitCaseTables
              @@0:
                mov     ecx,L1
                mov     eax,L2
                mov     esi,P1
                mov     edi,P2
                cmp     ecx,eax
                jbe     @@1
                mov     ecx,eax
              @@1:
                mov     ebx,T1
                mov     ebp,T2
                xor     eax,eax
                xor     edx,edx
                test    ecx,ecx
                jz      @@5
              @@2:
                mov     al,[esi]
                mov     dl,[edi]
                inc     esi
                inc     edi
                test    ebp,ebp
                mov     al,[ebx+eax]    // Table1
                mov     dl,[ebx+edx]
                jz      @@3
                mov     al,[ebp+eax]    // Table2
                mov     dl,[ebp+edx]
              @@3:
                cmp     al,dl
                jne     @@RET
                dec     ecx
                jnz     @@2
              @@5:
                mov     eax,L1
                mov     edx,L2
              @@RET:
                sub     eax,edx
end;

function SysCompareStrings(s1, s2: PChar; l1, l2: Longint; IgnoreCase: Boolean): Longint;
begin
  if IgnoreCase then
    Result := MemComp(s1, s2, l1, l2, @WeightTable, nil)
  else
    Result := MemComp(s1, s2, l1, l2, @AnsiUpperCaseTable, @WeightTable);
end;

procedure SysGetCaseMap(TblLen: Longint; Tbl: PChar );
var
  CC: CountryCode;
begin
  CC.Country := 0;  // Use default
  CC.CodePage := 0;
  DosMapCase(TblLen, CC, Tbl);
end;

procedure SysGetWeightTable(TblLen: Longint; WeightTable: PChar);
var
  CC: CountryCode;
  DataLen: Longint;
begin
  CC.Country := 0;  // Use default
  CC.CodePage := 0;
  DosQueryCollate(TblLen, CC, WeightTable, DataLen);
end;

function SysGetCodePage: Longint;
var
  Returned: Longint;
  CC: CountryCode;
  CI: CountryInfo;
begin
  Result := 0;
  DosQueryCp(SizeOf(Result), Result, Returned);
  if Result = 0 then
  begin
    CC.Country := 0;
    CC.CodePage := 0;
    DosQueryCtryInfo(SizeOf(CountryInfo), CC, CI, Returned);
    Result := CI.CodePage;
  end;
end;

var
  PrevXcptProc: Pointer = Ptr(-1);

function SignalHandler(Report:       PExceptionReportRecord;
                       Registration: PExceptionRegistrationRecord;
                       Context:      PContextRecord;
                       P:            Pointer): Longint; cdecl;
begin
  Result := xcpt_Continue_Search;
  if Report^.ExceptionNum = xcpt_Signal then
    case Report^.ExceptionInfo[0] of
      xcpt_Signal_Intr,xcpt_Signal_Break:
       if Assigned(CtrlBreakHandler) then
         if CtrlBreakHandler then
           Result := xcpt_Continue_Execution
    end;
  XcptProc := PrevXcptProc;
end;

procedure SysCtrlSetCBreakHandler;
var
  Times: Longint;
begin
  DosSetSignalExceptionFocus(True, Times);
  if PrevXcptProc=Ptr(-1) then
    begin
      PrevXcptProc := XcptProc;
      XcptProc := @SignalHandler;
    end;
end;

function SysFileIncHandleCount(Count: Longint): Longint;
var
  hDelta,hMax: Longint;
begin
  hDelta := Count;
  Result := DosSetRelMaxFH(hDelta, hMax);
end;

const
  CrtScanCode: Byte = 0;

function SysKeyPressed: Boolean;
var
  Key  : ^KbdKeyInfo;
  LKey : Array[1..2] of KbdKeyInfo;
begin
  Key := Fix_64k(@LKey, SizeOf(Key^));
  KbdPeek(Key^, 0);
  Result := (CrtScanCode <> 0) or ((Key^.fbStatus and kbdtrf_Final_Char_In) <> 0);
end;

procedure SysFlushKeyBuf;
begin
  CrtScanCode := 0;
end;

function SysPeekKey(Var Ch:Char):boolean;
Var
  ChData  : ^KbdKeyInfo;
  LChData : Array[1..2] of KbdKeyInfo;
begin
  ChData := Fix_64k(@LChData, SizeOf(ChData^));
  KbdPeek( ChData^, 0 );
  If ChData^.fbStatus and kbdtrf_Final_Char_In <> 0 then
    begin
      Ch := ChData^.ChChar;
      Result := True;
    end
  else
    Result := False;
end;

function SysReadKey: Char;
var
  Key  : ^KbdKeyInfo;
  LKey : Array[1..2] of KbdKeyInfo;
begin
  If CrtScanCode <> 0 then
    begin
      result:=Chr(CrtScanCode);
      CrtScanCode:=0;
    end
  else
    begin
      Key := Fix_64k(@LKey, SizeOf(Key^));
      KbdCharIn(Key^, io_Wait, 0);
      case Key^.chChar of
        #0: CrtScanCode := Key^.chScan;
        #$E0:           {   Up, Dn, Left Rt Ins Del Home End PgUp PgDn C-Home C-End C-PgUp C-PgDn C-Left C-Right C-Up C-Dn }
          if Key^.chScan in [$48,$50,$4B,$4D,$52,$53,$47, $4F,$49, $51, $77,   $75,  $84,   $76,   $73,   $74,    $8D, $91] then
          begin
            CrtScanCode := Key.chScan;
            Key^.chChar := #0;
          end;
      end;
      result:=Key^.chChar;
    end;
end;

procedure SysGetCurPos(var X, Y: SmallWord);
begin
  VioGetCurPos(Y, X, TVVioHandle);
end;

procedure SysSetCurPos(X,Y: SmallWord);
begin
  VioSetCurPos(Y, X, TVVioHandle);
end;

procedure SysWrtCharStrAtt(CharStr: Pointer; Len,X,Y: SmallWord; var Attr: Byte);
var
  pGood: Pointer;
  pTemp: Pointer;
begin
  if Invalid16Parm(CharStr, Len) then
    begin
      GetMem(pTemp, 2*Len);
      pGood := Fix_64k(pTemp, Len);
      Move(CharStr^, pGood^, Len);
      VioWrtCharStrAtt(pGood, Len, Y, X, Attr, TVVioHandle);
      FreeMem(pTemp);
    end
  else
    VioWrtCharStrAtt(CharStr, Len, Y, X, Attr, TVVioHandle);
end;

function SysReadAttributesAt(x,y: SmallWord): Byte;
var
  Cell, Size: SmallWord;
begin
  Size := Sizeof(Cell);
  VioReadCellStr(Cell, Size, y, x, 0);
  Result := Hi(Cell); // and $7f;
end;

function SysReadCharAt(x,y: SmallWord): Char;
var
  Cell, Size: SmallWord;
begin
  Size := Sizeof(Cell);
  if VioReadCellStr(Cell, Size, y, x, 0) = 0 then
    Result := chr(Lo(Cell))
  else
    Result := #0;
end;

procedure SysScrollUp(X1,Y1,X2,Y2,Lines,Cell: SmallWord);
begin
  VioScrollUp(Y1, X1, Y2, X2, Lines, Cell, TVVioHandle);
end;

procedure SysScrollDn(X1, Y1, X2, Y2, Lines, Cell: SmallWord );
begin
  VioScrollDn(Y1, X1, Y2, X2, Lines, Cell, TVVioHandle);
end;

const
  MouseHandle: SmallWord = $FFFF;
var
  ProtectArea: NoPtrRect;
  MouseEventMask: SmallWord;
  MouseMSec: Longint;
  ButtonCount: Longint;

function SysTVDetectMouse: Longint;
var
  MouLoc: PtrLoc;
  Buttons: SmallWord;
begin
  if MouOpen(nil, MouseHandle) = 0 then
    begin
      MouGetNumButtons(Buttons, MouseHandle);
      ButtonCount := Buttons;
{$IFNDEF NoMouseMove}
      MouLoc.Row := 0;
      MouLoc.Col := 0;
      MouSetPtrPos(MouLoc, MouseHandle);
{$ENDIF}
      Result := Buttons;
    end
  else
    Result := 0;
end;

procedure SysTVInitMouse(var X,Y: Integer);
var
  MouLoc: PtrLoc;
  EventMask: SmallWord;
begin
  if MouseHandle <> $FFFF then
  begin
    MouGetPtrPos(MouLoc, MouseHandle);
    X := MouLoc.Col;
    Y := MouLoc.Row;
    MouDrawPtr(MouseHandle);
    MouGetEventMask(MouseEventMask, MouseHandle);
    EventMask := $FFFF;
    MouSetEventMask(EventMask, MouseHandle);  // Select all events
  end;
end;

procedure SysTVDoneMouse(Close: Boolean);
begin
  if MouseHandle <> $FFFF then
  begin
    if Close then
      MouClose(MouseHandle)
    else
      begin
        SysTVHideMouse; // Restore events to original state
        MouSetEventMask(MouseEventMask, MouseHandle);
      end;
  end;
end;

procedure SysTVShowMouse;
begin
  if MouseHandle <> $FFFF then
    MouDrawPtr(MouseHandle);
end;

procedure SysTVHideMouse;
begin
  // Assume that ProtectArea does not wrap around segment boundary
  if MouseHandle <> $FFFF then
    MouRemovePtr(ProtectArea, MouseHandle);
end;

procedure SysTVUpdateMouseWhere(var X,Y: Integer);
var
  MouLoc: PtrLoc;
  MSec: Longint;
begin
  MSec := SysSysMsCount;
  if MSec - MouseMSec >= 5 then
  begin
    MouseMSec := MSec;
    MouGetPtrPos(MouLoc, MouseHandle);
    X := MouLoc.Col;
    Y := MouLoc.Row;
  end;
end;

function SysTVGetMouseEvent(var Event: TSysMouseEvent): Boolean;
var
  MouEvent  : ^MouEventInfo;
  MouQInfo  : ^MouQueInfo;
  LMouEvent : Array[1..2] of MouEventInfo;
  LMouQInfo : Array[1..2] of MouQueInfo;
const
  WaitFlag: SmallWord = mou_NoWait;
begin
  MouQInfo := Fix_64k(@LMouQInfo, SizeOf(MouQInfo^));

  MouGetNumQueEl(MouQInfo^, MouseHandle);
  if MouQinfo^.cEvents = 0 then
    Result := False
  else
    begin
      MouEvent := Fix_64k(@LMouEvent, SizeOf(MouEvent^));
      MouReadEventQue(MouEvent^, WaitFlag, MouseHandle);
      with Event do
        begin
          smeTime := MouEvent^.Time;
          MouseMSec := MouEvent^.Time;
          smeButtons := 0;
          if (MouEvent^.fs and (mouse_Motion_With_Bn1_Down or mouse_Bn1_Down)) <> 0 then
            Inc(smeButtons, $0001);
          if (MouEvent^.fs and (mouse_Motion_With_Bn2_Down or mouse_Bn2_Down)) <> 0 then
            Inc(smeButtons, $0002);
          smePos.X := MouEvent^.Col;
          smePos.Y := MouEvent^.Row;
        end;
      Result := True;
    end;
end;

procedure SysTVKbdInit;
var
  Key  : ^KbdInfo;
  LKey : Array[1..2] of KbdInfo;

begin
  Key := Fix_64k(@LKey, SizeOf(Key^));
  Key^.cb := SizeOf(KbdInfo);
  KbdGetStatus(Key^, 0);        { Disable ASCII & Enable raw (binary) mode}
  Key^.fsMask := (Key^.fsMask and (not keyboard_Ascii_Mode)) or keyboard_Binary_Mode;
  KbdSetStatus(Key^, 0);
end;

function SysTVGetPeekKeyEvent(var Event: TSysKeyEvent; _Peek: Boolean): Boolean;
var
  Key  : ^KbdKeyInfo;
  LKey : Array[1..2] of KbdKeyInfo;
begin
  Key := Fix_64k(@LKey, SizeOf(Key^));
  if _Peek then
    KbdPeek(Key^, 0)
  else
    KbdCharIn(Key^, io_NoWait, 0);
  if (Key^.fbStatus and kbdtrf_Final_Char_In) = 0 then
    Result := False
  else
    with Event do   // Key is ready
      begin
        skeKeyCode := Ord(Key^.chChar) + Key^.chScan shl 8;
        skeShiftState := Lo(Key^.fsState);
        Result := True;
      end;
end;

function SysTVGetKeyEvent(var Event: TSysKeyEvent): Boolean;
begin
  Result := SysTVGetPeekKeyEvent(Event, False);
end;

function SysTVPeekKeyEvent(var Event: TSysKeyEvent): Boolean;
begin
  Result := SysTVGetPeekKeyEvent(Event, True);
end;

function SysTVGetShiftState: Byte;
var
  Key  : ^KbdInfo;
  LKey : Array[1..2] of KbdInfo;

begin
  Key := Fix_64k(@LKey, SizeOf(Key^));
  Key^.cb := SizeOf(KbdInfo);
  KbdGetStatus(Key^, 0);
  Result := Lo(Key^.fsState);
end;

procedure SysTVSetCurPos(X,Y: Integer);
begin
  VioSetCurPos(Y, X, TVVioHandle);
end;

procedure SysTVSetCurType(Y1,Y2: Integer; Show: Boolean);
var
  CurData  : ^VioCursorInfo;
  LCurData : Array[1..2] of VioCursorInfo;
begin
  CurData := Fix_64k(@LCurData, SizeOf(CurData^));
  with CurData^ do
    begin
      yStart := Y1;
      cEnd   := Y2;
      cx := 1;
      if Show then
        attr := 0
      else
        begin
          attr := $FFFF;
          yStart := 0;
          cEnd := 1;
        end;
    end;
  VioSetCurType(CurData^, TVVioHandle);
end;

procedure SysTVGetCurType(var Y1,Y2: Integer; var Visible: Boolean);
var
  CurData  : ^VioCursorInfo;
  LCurData : Array[1..2] of VioCursorInfo;
begin
  CurData := Fix_64k(@LCurData, SizeOf(CurData^));
  VioGetCurType(CurData^, TVVioHandle);
  Visible := CurData^.attr <> $FFFF;
  Y1 := CurData^.yStart;
  Y2 := CurData^.cEnd;
end;

procedure SysTVShowBuf(Pos,Size: Integer);
begin
  VioShowBuf(Pos, Size, TVVioHandle);
end;

procedure SysTVClrScr;
const
  Cell: SmallWord = $0720;      // Space character, white on black
begin
  VioScrollUp(0, 0, 65535, 65535, 65535, Cell, TVVioHandle);
  SysTVSetCurPos(0, 0);
end;

procedure SetMouseArea(X,Y: Integer);
begin
  ProtectArea.Row := 0;
  ProtectArea.Col := 0;
  ProtectArea.cRow := Y - 1;
  ProtectArea.cCol := X - 1;
end;

function SysTVGetScrMode(_Size: PSysPoint; _Align: Boolean): Integer;
var
  VioMode  : ^VioModeInfo;
  LVioMode : Array[1..2] of VioModeInfo;
begin
  VioMode := Fix_64k(@LVioMode, SizeOf(VioMode^));
  VioMode^.cb := SizeOf(VioMode^);
  if VioGetMode(VioMode^, TVVioHandle) <> 0 then
    Result := $FF   // smNonStandard
  else
    begin
      with VioMode^ do
        begin
          if (fbType and vgmt_DisableBurst) = 0 then
            Result := 3   // smCO80
          else
            Result := 2;  // smBW80;
          if Color = 0 then
            Result := 7;  // smMono
          case Row of
            25: ;
            43,50: Inc(Result, $0100); // smFont8x8
            else   Result := $FF; // smNonStandard
          end;
          if (VioMode^.fbType and vgmt_Graphics) <> 0 then
            Result := 0;
        end;
    SetMouseArea(VioMode^.Col, VioMode^.Row);
    if _Size <> nil then
      with _Size^ do
        begin
          X := VioMode^.Col;
          Y := VioMode^.Row;
        end;
  end;
end;

procedure SysTVSetScrMode(Mode: Integer);
var
  BiosMode     : Byte;
  VioMode      : ^VioModeInfo;
  VideoConfig  : ^VioConfigInfo;
  LVioMode     : Array[1..2] of VioModeInfo;
  LVideoConfig : Array[1..2] of VioConfigInfo;
begin
  VioMode := Fix_64k(@LVioMode, SizeOf(VioMode^));
  VideoConfig := Fix_64k(@LVideoConfig, SizeOf(VideoConfig^));

  BiosMode := Lo(Mode);
  VideoConfig^.cb := SizeOf(VideoConfig^);
  VioGetConfig(0, VideoConfig^, TVVioHandle);

  with VioMode^ do
    begin
      // Indicate that we only filled important Entrys
      // the Video handler will find the best values itself
      cb := Ofs(HRes) - Ofs(cb);

      case Lo(Mode) of
        0, 2: fbType := vgmt_Other + vgmt_DisableBurst;
           7: fbType := 0;
      else
        fbType := vgmt_Other;
      end;

      if Lo(Mode) = 7 then
        Color := 0
      else
        Color := colors_16;         // Color

      Row := 25;

      if lo(Mode) < 2 then
        Col := 40
      else
        Col := 80;

      case VideoConfig^.Adapter of
        display_Monochrome..display_CGA: ; // only 25 Lines
        display_EGA:
          if hi(mode) = 1 then             // font 8x8
            Row := 43;                     // 350/8=43
      else // VGA
        if hi(mode) = 1 then               // font 8x8
          Row := 50;                       // 400/8=25
      end;
    end;

  SetMouseArea(VioMode^.Col, VioMode^.Row);
  VioSetMode(VioMode^, TVVioHandle);
end;

function SysTVGetSrcBuf: Pointer;
var
  BufSize: SmallWord;
begin
  VioGetBuf(Result, BufSize, TVVioHandle);
  SelToFlat(Result);
end;

procedure SysTVInitCursor;
var
  Font  : ^VioFontInfo;
  LFont : Array[1..2] of VioFontInfo;
begin
  Font := Fix_64k(@LFont, SizeOf(Font^));

  FillChar(Font^, SizeOf(Font^), 0);
  Font^.cb := SizeOf(VioFontInfo);
  Font^.rType := vgfi_GetCurFont;
  // Set underline cursor to avoid cursor shape problems
  if VioGetFont(Font^, TVVioHandle) = 0 then
    SysTVSetCurType(Font^.cyCell - 2, Font^.cyCell - 1, True);
end;

procedure SysTvDoneCursor;
begin
end;

procedure SysCtrlSleep(Delay: Integer);
begin
  DosSleep(Delay);
end;

function SysGetValidDrives: Longint;
var
  CurDrive: Longint;
begin
  if DosQueryCurrentDisk(CurDrive, Result) <> 0 then
    Result := 0;
end;

procedure SysDisableHardErrors;
begin
  DosError(ferr_DisableHardErr);
end;

function SysKillProcess(Process: Longint): Longint;
begin
  Result := DosKillProcess(dkp_ProcessTree, Process);
end;

// Copy _Name to _Os2Name, prefixing \SHAREMEM\ if necessary
procedure MakeSharedMemName( _Os2Name, _Name: pChar );
const
  Os2ShareMemPrefix: PChar = '\SHAREMEM\';
begin
  _Os2Name^ := #0;
  if StrLComp( _Name, Os2ShareMemPrefix, 10 ) <> 0 then
    StrCopy( _Os2Name, Os2ShareMemPrefix );
  StrCat( _Os2Name, _Name );
end;

function SysAllocSharedMemory(var _Base: Pointer; _Name: pChar; _Size: Longint): Longint;
var
  Name: Array[0..512] of char;
begin
  MakeSharedMemName( @Name, _Name );
  if DosAllocSharedMem(_Base, Name, _Size, obj_Giveable + pag_Read + pag_Write + pag_Commit) = 0 then
    Result := 0 // OS/2 does not use handles; 0 is its "handle"
  else
    Result := -1; // Error
end;

function SysAccessSharedMemory(var _Base: Pointer; _Name: pChar): Longint;
var
  Name: Array[0..512] of char;
begin
  MakeSharedMemName( @Name, _Name );
  if DosGetNamedSharedMem( _Base, _Name, obj_Giveable + pag_Read + pag_Write + pag_Commit ) = 0 then
    Result := 0
  else
    Result := -1; //Error
end;

procedure SysFreeSharedMemory(_Base: Pointer; _Handle: Longint);
begin
  DosFreeMem(_Base);
end;

function SysAllocSharedMem(Size: Longint; var MemPtr: Pointer): Longint;
begin
  Result := DosAllocSharedMem(MemPtr, nil, Size, obj_Giveable + pag_Read + pag_Write + pag_Commit);
end;

function SysGiveSharedMem(MemPtr: Pointer): Longint;
var
  PB: PPIB;
  TB: PTIB;
begin
  DosGetInfoBlocks(TB, PB);
  Result := DosGiveSharedMem(MemPtr, PB^.Pib_ulPPid, pag_Read + pag_Write);
end;

function SysPipeCreate(var ReadHandle,WriteHandle: Longint; Size: Longint): Longint;
var
  PipeName: array[0..259] of Char;
  Number: array[0..10] of Char;
begin
  StrCopy(PipeName, '\PIPE\');
  Str(SysSysMsCount, Number);
  StrCopy(@PipeName[6], Number);
  Result := DosCreateNPipe(PipeName, ReadHandle, np_Access_InBound, np_NoWait + 1, 0, Size, 0);
  DosConnectNPipe(ReadHandle);
  SysFileOpen(PipeName, $41, WriteHandle);
end;

function SysPipePeek(Pipe: Longint; Buffer: Pointer; BufSize: Longint; var BytesRead: Longint; var IsClosing: Boolean): Longint;
var
  State: Longint;
  Avail: AvailData;
begin
  Result := DosPeekNPipe(Pipe, Buffer^, BufSize, BytesRead, Avail, State);
  IsClosing := State = np_State_Closing;
end;

function SysPipeClose(Pipe: Longint): Longint;
begin
  Result := SysFileClose(Pipe);
end;

function SysLoadResourceString(ID: Longint; Buffer: PChar; BufSize: Longint): PChar;
begin
  Buffer[0] := #0;
  WinLoadString(0, 0, ID, BufSize, Buffer);
  Result:=Buffer;
end;

function SysFileUNCExpand(Dest,Name: PChar): PChar;
var
  P: PChar;
  Len: Longint;
  pfsqb: pfsqBuffer2;
  DevName: array[0..2] of Char;
  Drive: String;
begin
  SysFileExpand(Dest, Name);
  if (UpCase(Dest[0]) in ['A'..'Z']) and (Dest[1] = ':') and (Dest[2] = '\') then
  begin
    DevName[0] := Dest[0];
    DevName[1] := ':';
    DevName[2] := #0;
    Len := 1024;
    GetMem(pfsqb, Len);
    FillChar(pfsqb^, Len, 0);
    DosQueryFSAttach(DevName, 1, fsail_QueryName, pfsqb, Len);
    P := pfsqb^.szName;         // Points to name of entry
    P := P + StrLen(P) + 1;     // Points to name of FS (LAN, NETWARE, etc)
    P := P + StrLen(P) + 1;     // Points to UNC name
    if (P^ = '\') and ((P+1)^ = '\') then // Filter out results that do not start with '\\'
    begin
      Len := StrLen(P);
      StrCopy(@Dest[Len], @Dest[2]);
      Move(P^, Dest^, Len);
    end;
    FreeMem(pfsqb);
  end;
  Result := Dest;
end;

function SysGetSystemError(Code: Longint; Buffer: PChar; BufSize: Longint; var MsgLen: Longint): PChar;
begin
  Result := Buffer;
  if DosGetMessage(nil, 0, Buffer, BufSize-1, Code, 'OSO001.MSG', MsgLen) <> 0 then
    MsgLen := 0;
end;

function SysGetProfileStr(Section,Entry,Default,Dest: PChar): PChar;
begin
  Dest[0] := #0;
  PrfQueryProfileString(hini_UserProfile, Section, Entry, Default, Dest, 260);
  result:=Dest;
end;

function SysGetProfileChar(Section, Entry: PChar; Default: Char): Char;
var
  CDefault, Buffer: array[0..1] of Char;
begin
  CDefault[0] := Default;
  CDefault[1] := #0;
  PrfQueryProfileString(hini_UserProfile, Section, Entry, CDefault, @Buffer, SizeOf(Buffer));
  Result := Buffer[0];
end;

procedure SysGetCurrencyFormat(CString: PChar; var CFormat, CNegFormat, CDecimals: Byte; var CThousandSep, CDecimalSep: Char);
begin
  SysGetProfileStr(SIntl, 'sCurrency', '', CString);
  CFormat := PrfQueryProfileInt(hini_UserProfile, SIntl, 'iCurrency', 0);
  CNegFormat := 0;                   { N/A under PM }
  CThousandSep := SysGetProfileChar(SIntl, 'sThousand', ',');
  CDecimalSep := SysGetProfileChar(SIntl, 'sDecimal', '.');
  CDecimals := PrfQueryProfileInt(hini_UserProfile, SIntl, 'iDigits', 2);
end;

procedure SysGetDateFormat(var DateSeparator: Char; ShortDateFormat,LongDateFormat: PChar);
var
  Date: Integer;
const
  DateStr: array [0..2] of PChar =
    ( 'mm/dd/yy', 'dd/mm/yy', 'yy/mm/dd' );
  LongDateStr: array [0..2] of PChar =
    ('mmmm d, yyyy', 'dd mmmm yyyy', 'yyyy mmmm d');
begin
  DateSeparator := SysGetProfileChar(SIntl, 'sDate', '/');
  Date := PrfQueryProfileInt(hini_UserProfile, SIntl, 'iDate', 0);
  if Date > 2 then
    Date := 0;
  StrCopy(ShortDateFormat, DateStr[Date]);   // No exact equivalent under PM
  StrCopy(LongDateFormat, LongDateStr[Date]);
end;

procedure SysGetTimeFormat(var TimeSeparator: Char; TimeAMString,TimePMString,ShortTimeFormat,LongTimeFormat: PChar);
var
  TimePostfix: PChar;
const
  SIntl: PChar = 'PM_National';
begin
  TimeSeparator := SysGetProfileChar(SIntl, 'sTime', ':');
  SysGetProfileStr(SIntl, 's1159', 'am', TimeAMString);
  SysGetProfileStr(SIntl, 's2359', 'pm', TimePMString);
  if PrfQueryProfileInt(hini_UserProfile, SIntl, 'iLzero', 0) = 0 then
    begin
      StrCopy(ShortTimeFormat, 'h:mm');
      StrCopy(LongTimeFormat, 'h:mm:ss');
    end
  else
    begin
      StrCopy(ShortTimeFormat, 'hh:mm');
      StrCopy(LongTimeFormat, 'hh:mm:ss');
    end;
  TimePostfix := '';
  if PrfQueryProfileInt(hini_UserProfile, SIntl, 'iTime', 0) = 0 then
    TimePostfix := ' AMPM';
  StrCat(ShortTimeFormat, TimePostfix);
  StrCat(LongTimeFormat, TimePostfix);
end;

function SysGetModuleName(var Address: Pointer; Buffer: PChar; BufSize: Longint): PChar;
var
  ModuleName: array[0..259] of Char;
begin
  DosQueryModuleName(ModuleHandle, SizeOf(ModuleName), ModuleName);
  StrLCopy(Buffer, StrRScan(ModuleName, '\') + 1, BufSize - 1);
  Result := Buffer;
end;

procedure SysDisplayConsoleError(PopupErrors: Boolean; Title, Msg: PChar);
var
  PopupFlags : SmallWord;
  Info       : ^KbdKeyInfo;
  LInfo      : Array[1..2] of KbdKeyInfo;
  Count      : Longint;
  pTemp      : pChar;
  pGood      : pChar;
type
  WordRec = packed record
    Lo, Hi: Byte;
  end;
const
  Cell: SmallWord = $4F20;
begin
  if PopupErrors then
    begin
      PopupFlags := 1;
      VioPopup(PopupFlags, 0);
      VioScrollUp(0, 0, 127, 127, 127, Cell, 0);
      VioSetCurPos(12, 0, 0);
      Count:=StrLen(Msg);
      if Invalid16Parm(Msg, Count) then
        begin
          GetMem(pTemp, 2*Count);
          pGood := Fix_64k(pTemp, Count);
          Move(Msg^, pGood^, Count);
          VioWrtTTy(pGood, Count, 0);
          FreeMem(pTemp);
        end
      else
        VioWrtTTy(@Msg, Count, 0);
      VioWrtNAttr(WordRec(Cell).Hi, 2000, 0,0, 0);
      Info := Fix_64k(@LInfo, SizeOf(Info^));
      if KbdCharIn(Info^, io_Wait, 0) <> 0 then
        // Wait if kbd call fails.  It does so when exception is
        // caused by Ctrl-Brk or Ctrl-C.
        DosSleep(5000);
      VioEndPopUp(0);
    end
  else
    DosWrite(1, Msg^, StrLen(Msg), Count);
end;

procedure SysDisplayGUIError(Title, Msg: PChar);
begin
  WinCreateMsgQueue(WinInitialize(0), 0);
  WinMessageBox(hwnd_Desktop, hwnd_Desktop, Msg, Title, 0, mb_Error+mb_Moveable);
end;

procedure SysBeep;
begin
  WinAlarm(hwnd_Desktop, wa_Error);
end;

procedure SysBeepEx(Freq,Dur: Longint);
begin
  DosBeep(Freq, Dur);
end;

function SysGetVolumeLabel(Drive: Char): ShortString;
var
  rc          : Longint;
  DriveNumber : Word;
  Buf: Record
    SerialNum : Word;
    VolLabel  : String[12];
  end;

begin
  DriveNumber := Ord( UpCase(Drive) ) - Ord( 'A' ) + 1;

  rc := DosQueryFSInfo( DriveNumber, fsil_VolSer, Buf, Sizeof( Buf ));
  If rc = No_Error then
    Result := Buf.VolLabel
  else
    Result := '';
end;

function SysSetVolumeLabel(Drive: Char; _Label: ShortString): Boolean;
var
  DriveNumber : Word;
begin
  DriveNumber := Ord( Drive ) - Ord( 'A' ) + 1;
  _Label[Length(_Label)+1] := #0;
  Result := 0 = DosSetFSInfo(DriveNumber, fsil_VolSer, _Label, Length(_Label)+1);
end;

function SysGetForegroundProcessId: Longint;
Var
  Res : Word;
  rc  : Longint;

begin
  rc := DosQuerySysInfo( qsv_foreground_process, qsv_foreground_process,
                         Res, Sizeof( Res ));
  If rc = no_Error then
    Result := Res
  else
    Result := 0;
end;

function SysGetBootDrive: Char;
Var
  Res : Word;
  rc  : Longint;

begin
  rc := DosQuerySysInfo( qsv_boot_drive, qsv_boot_drive, Res, Sizeof( Res ));
  If rc = no_Error then
    Result := Chr( Res + ord('A') - 1 )
  else
    Result := #0;
end;

function SysGetDriveType(Drive: Char): TDriveType;
Var
  BufLen    : Word;
  FSQb      : pFSQBuffer2;
  DrvName   : String[3];
  Ordinal   : SmallWord;
  name      : pChar;
  rc        : Word;
  DiskSize  : Word;

begin
  Result := dtInvalid;
  BufLen := 100;
  GetMem( FSQb, BufLen );
  DrvName := Drive+':'#0;
  Ordinal := 0;
  rc := DosQueryFSAttach( @DrvName[1], Ordinal, fsail_QueryName, FSqb, BufLen );
  if rc = 0 then
    With FsqB^ do
    begin
      Name := szName + cbName + 1;
      If strComp( Name, 'FAT' ) = 0 then
        If Drive <= 'B' then
          Result := dtFloppy
        else
          Result := dtHDFAT
      else if strComp( Name, 'HPFS' ) = 0 then
        If Drive <= 'B' then
          Result := dtFloppy
        else
          Result := dtHDHPFS
      else If StrComp( Name, 'NETWARE' ) = 0 then
        Result := dtNovellNet
      else If StrComp( Name, 'CDFS' ) = 0 then
        Result := dtCDRom
      else If StrComp( Name, 'TVFS' ) = 0 then
        Result := dtTVFS
      else If StrComp( Name, 'ext2' ) = 0 then
        Result := dtHDExt2
      else If StrComp( Name, 'LAN' ) = 0 then
        Result := dtLAN
      else If StrComp( Name, 'JFS' ) = 0 then
        Result := dthdJFS
      else If StrComp( Name, 'FAT32' ) = 0 then
        Result := dthdFAT32
      else If StrComp( Name, 'RAMFS' ) = 0 then
        Result := dtRAMFS
      else If StrComp( Name, 'NDFS32' ) = 0 then
        Result := dtNDFS32;
    end;

  FreeMem( FSQb, 100 );
end;

function SysGetVideoModeInfo(Var Cols, Rows, Colours: Word ): Boolean;
Var
  vm  : ^VioModeInfo;
  Lvm : Array[1..2] of VioModeInfo;

begin
  vm := Fix_64k(@Lvm, SizeOf(vm^));
  vm.cb := Sizeof(vm^);
  VioGetMode( vm^, 0 );
  With vm^ do
    begin
      Rows := Row;
      Cols := Col;
      Colours := 1 shl Color;
    end;
  Result := True;
end;

function SysGetVisibleLines( var Top, Bottom: Longint ): Boolean;
var
  Cols, Rows, Colours: Word;
begin
  if SysGetVideoModeInfo( Cols, Rows, Colours ) then
  begin
    Result := True;
    Top := 1;
    Bottom := Rows;
  end
  else
    Result := False;
end;

function SysSetVideoMode(Cols, Rows: Word): Boolean;
Var
  vm  : ^VioModeInfo;
  Lvm : Array[1..2] of VioModeInfo;

begin
  vm := Fix_64k(@Lvm, SizeOf(vm^));
  vm^.cb := 8;          { Size of structure }
  VioGetMode(vm^, 0);
  vm^.fbType := 1;      { Text mode }
  vm^.Row := Rows;
  vm^.Col := Cols;
  vm^.Color := 4;       { 16 colors }
  Result := ( VioSetMode( vm^, 0 ) = 0 );
end;

function SemCreateEvent(_Name: pChar; _Shared, _State: Boolean): TSemHandle;
var
  rc: ApiRet;
  Attr: ULong;
  Buf: packed array[0..255] of char;
begin
  if _Shared then
    Attr := dc_Sem_Shared
  else
    Attr := 0;
  if (_Name<>nil) and (_Name^<>#0) then
    begin
      StrCat(StrCopy(@Buf,'\SEM32\'),_Name);
      rc := DosCreateEventSem( @Buf, Result, Attr, _State );
    end
  else
    rc := DosCreateEventSem( _Name, Result, Attr, _State );
  if rc<>No_Error then
    Result := -1;
end;

function SemAccessEvent(_Name: pChar): TSemHandle;
var
  Buf: packed array[0..255] of char;
  rc: ApiRet;
begin
  if (_Name<>nil) and (_Name^<>#0) then
    begin
      StrCat(StrCopy(@Buf,'\SEM32\'),_Name);
      Result := 0;
      rc := DosOpenEventSem( @Buf, Result );
      if rc<>No_Error then
        Result := -1;
    end
  else
    Result:=-1;
end;

function SemPostEvent(_Handle: TSemhandle): Boolean;
begin
  Result := DosPostEventSem( _Handle ) = 0;
end;

function SemResetEvent(_Handle: TSemhandle; var _PostCount: Longint): Boolean;
begin
  Result := DosResetEventSem( _Handle, _PostCount ) = 0;
end;

function SemWaitEvent(_Handle: TSemHandle; _TimeOut: Longint): Boolean;
var
  Dummy: Longint;
begin
  Result := DosWaitEventSem(_Handle, _TimeOut) = 0;
  DosResetEventSem( _Handle, Dummy );
end;

procedure SemCloseEvent(_Handle: TSemHandle);
begin
  DosCloseEventSem(_Handle);
end;

function SemCreateMutex(_Name: PChar; _Shared, _State: Boolean): TSemHandle;
var
  Flags: Longint;
  rc: ApiRet;
  Buf: packed array[0..255] of char;
begin
  Flags := 0;
  if _Shared then
    Flags := dc_sem_Shared;
  if (_Name<>nil) and (_Name^<>#0) then
    begin
      StrCat(StrCopy(@Buf,'\SEM32\'),_Name);
      rc := DosCreateMutexSem(@Buf, Result, Flags, _State);
    end
  else
    rc := DosCreateMutexSem(_Name, Result, Flags, _State);
  if rc<>No_Error then
    result := -1;
end;

function SemAccessMutex(_Name: PChar): TSemHandle;
var
  rc: ApiRet;
  Buf: packed array[0..255] of char;
begin
  Result := 0;
  if (_Name<>nil) and (_Name^<>#0) then
    begin
      StrCat(StrCopy(@Buf,'\SEM32\'),_Name);
      rc := DosOpenMutexSem(@Buf, Result);
    end
  else
    rc := DosOpenMutexSem(_Name, Result);
  if rc<>No_Error then
    Result := -1;
end;

function SemRequestMutex(_Handle: TSemHandle; _TimeOut: Longint): Boolean;
begin
  Result := DosRequestMutexSem(_Handle, _TimeOut) = 0;
end;

function SemReleaseMutex(_Handle: TSemHandle): Boolean;
begin
  Result := DosReleaseMutexSem( _Handle ) = 0;
end;

procedure SemCloseMutex(_Handle: TSemHandle);
begin
  DosCloseMutexSem(_Handle);
end;

function SysMemInfo(_Base: Pointer; _Size: Longint; var _Flags: Longint): Boolean;
begin
  Result := (DosQueryMem(_Base, _Size, _Flags)=No_Error);
end;

function SysSetMemProtection(_Base: Pointer; _Size: Longint; _Flags: Longint): Boolean;
begin
  result := (DosSetMem(_Base, _Size, _Flags)=No_Error);
end;

procedure SysMessageBox(_Msg, _Title: PChar; _Error: Boolean);
var
  Flag: Longint;
begin
  if _Error then
    Flag := mb_Error
  else
    Flag := mb_Information;
  WinMessageBox( hwnd_Desktop, hwnd_Desktop, _Msg, _Title, 0, Flag or mb_ok);
end;

function SysClipCanPaste: Boolean;
var
  Fmt: ULong;
begin
  WinCreateMsgQueue(WinInitialize(0), 0);
  // Console apps can only use the OS/2 clipboard if the "hack" works
  Result := (not IsConsole or (PM_Clipboardhack = clipOK)) and
    WinQueryClipBrdFmtInfo(WinInitialize(0), cf_Text, Fmt);
end;

function SysClipCopy(P: PChar; Size: Longint): Boolean;
var
  Q: pChar;
  Anchor: HAB;
begin
  Result := False;
  Anchor := WinInitialize(0);
  WinCreateMsgQueue(Anchor, 0);
  // Open PM clipboard
  if WinOpenClipBrd(Anchor) then
  begin
    // Allocate giveable block of memory
    DosAllocSharedMem(Pointer(Q), nil, Size+1, pag_Write+pag_Commit+obj_Giveable);
    if Q <> nil then
    begin
      // Copy clipboard data across
      Move(P^, Q^, Size);
      Q[Size]:=#0;
      // Insert data into clipboard
      Result := WinSetClipBrdData(Anchor, ULong(Q), cf_Text, cfi_Pointer);
    end;
    WinCloseClipBrd(Anchor);
  end;
end;

function SysClipPaste(var Size: Integer): Pointer;
var
  P: PChar;
  Anchor: HAB;
  Flags: Longint;
begin
  Result := nil;
  Anchor := WinInitialize(0);
  WinCreateMsgQueue(Anchor, 0);
  if WinOpenClipBrd(Anchor) then
  begin
    P := PChar(WinQueryClipBrdData(Anchor, cf_Text));
    if Assigned(P) then
      if SysMemInfo(P, 1, Flags) and (Flags and sysmem_read <> 0) then
        begin
          Size := StrLen(P) + 1;
          GetMem(Result, Size);
          Move(P^, Result^, Size);
        end;
    WinCloseClipBrd(Anchor);
  end;
end;

// Retrieve various system settings, bitmapped:
// 0: Enhanced keyboard installed

function SysGetSystemSettings: Longint;
Var
  rc    : ApiRet16;
  HwId  : ^KbdHwId;
  LHwId : Array[1..2] of KbdHwId;

begin
  Result := 0;

  // Determine if enhanced keyboard is available
  rc := KbdGetFocus(1, 0);
  If rc = No_Error then
    begin
      HwId := Fix_64k(@LHwId, SizeOf(HwId^));
      HwId^.cb := Sizeof( HwId^ );
      rc := KbdGetHwId( HwId^, 0 );
      If rc = No_Error then
        if ( HwId^.IdKbd = keyboard_Enhanced_101 ) or
           ( HwId^.IdKbd = keyboard_Enhanced_122 ) then
          Result := Result OR 1;

      rc := KbdFreeFocus( 0 );
    end;
end;

procedure SysLowInitPreTLS;
begin
  // Nothing
end;

procedure SysLowInitPostTLS;
begin
  // Nothing
end;

