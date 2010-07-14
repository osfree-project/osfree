// |---------------------------------------------------------|
// |                                                         |
// |     Virtual Pascal Runtime Library.  Version 2.1.       |
// |     System interface layer for all OSes                 |
// |     ----------------------------------------------------|
// |     Copyright (C) 1995-2003 vpascal.com                 |
// |                                                         |
// |---------------------------------------------------------|

//{.$MODE ObjFPC}
{&OrgName+,Speed+,AlignCode+,AlignRec-,CDecl-,Far16-,Frame+,Delphi+}
{$X+,I-,H-,R-,S-,Q-,T-}
//W-,J+.Delphi+,Use32+,B-
unit SysLow;

interface

uses
{$IFDEF OS2}    Os2Def, {Os2Base} Sysutils, Doscalls; {$Undef KeyDll} {$ENDIF}
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

  TDriveType = (dtFloppy, dtHDFAT, dtHDHPFS, dtInvalid,
                dtNovellNet, dtCDRom, dtLAN, dtHDNTFS, dtUnknown,
                dtTVFS, dtHDExt2, dtHDJFS, dtHDFAT32, dtRAMFS, dtNDFS32);

const
{$IFDEF OS2}
  SemInfinite = sem_indefinite_wait;
  Exception_Maximum_Parameters = 4;
  PathSeparator = ';';
  AllFilesMask  = '*';
{$ENDIF}
{$IFDEF WIN32}
  SemInfinite = INFINITE;
  Exception_Maximum_Parameters = 15;
  PathSeparator = ';';
  AllFilesMask  = '*';
{$ENDIF}
{$IFDEF DPMI32}
  SemInfinite = -1; // not used
  Exception_Maximum_Parameters = 4;
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

function SysGetVolumeLabel(Drive: Char): ShortString;
function SysSetVolumeLabel(Drive: Char; _Label: ShortString): Boolean;

function SysGetBootDrive: Char;
function SysGetDriveType(Drive: Char): TDriveType;
function SysGetValidDrives: Longint;

function SysOsVersion: Longint;
function SysDirGetCurrent(Drive: Longint; Path: PChar): Longint;
function SysFileSearch(Dest,Name,List: PChar): PChar;
function SysFindFirst(Path: PChar; Attr: Longint; var F: TOSSearchRec; IsPChar: Boolean): Longint;
function SysFindNext(var F: TOSSearchRec; IsPChar: Boolean): Longint;
function SysFindClose(var F: TOSSearchRec): Longint;
function SysFileExpand(Dest,Name: PChar): PChar;
function SysFileAsOS(FileName: PChar): Boolean;
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
  //ExeHdr,
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
    FTime,FDate: Word;
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

// Include platform specific implementations

{$IFDEF OS2}
  {$I Os2.Pas}
{$ENDIF}

{$IFDEF WIN32}
  {$I W32.Pas}
{$ENDIF}

{$IFDEF DPMI32}
  {$I D32.Pas}
{$ENDIF}

{$IFDEF LINUX}
  {$I Lnx.Pas}
{$ENDIF}

{initialization}
{$IFNDEF FPC}
begin
{$ENDIF}
end.
