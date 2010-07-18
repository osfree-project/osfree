// |---------------------------------------------------------|
// |                                                         |
// |     Virtual Pascal Runtime Library.  Version 2.1.       |
// |     System interface layer for all OSes                 |
// |     ----------------------------------------------------|
// |     Copyright (C) 1995-2003 vpascal.com                 |
// |                                                         |
// |---------------------------------------------------------|

{.$MODE ObjFPC}
{&OrgName+,Speed+,AlignCode+,AlignRec-,CDecl-,Far16-,Frame+,Delphi+}
{$X+,I-,H-,R-,S-,Q-,T-}
{$IFNDEF FPC}{$W-,J+.Delphi+,Use32+,B-}{$ENDIF}
unit SysLow;

interface

uses
{$IFDEF OS2}    Os2Def, Sysutils, {$ifdef FPC} Doscalls {$else} Os2Base {$endif}; {$Undef KeyDll} {$ENDIF}
{$IFDEF LINUX}  Linux;                           {$ENDIF}
{$IFDEF WIN32}  Windows;                         {$ENDIF}
{$IFDEF DPMI32} {$ifndef FPC} Dpmi32df, {$endif} Impl_D32;              {$ENDIF}

type
  TDriveType = (dtFloppy, dtHDFAT, dtHDHPFS, dtInvalid,
                dtNovellNet, dtCDRom, dtLAN, dtHDNTFS, dtUnknown,
                dtTVFS, dtHDExt2, dtHDJFS, dtHDFAT32, dtRAMFS, dtNDFS32);

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

function SysGetBootDrive: Char;
function SysGetDriveType(Drive: Char): TDriveType;
function SysGetValidDrives: Longint;


implementation

{&OrgName-}

uses
  {$Ifdef Win32} {$Ifndef KeyDll}
  VpKbdW32,
  {$Endif} {$Endif}
  {$Ifdef DPMI32}
  {$ifndef FPC} Dpmi32, D32Res, {$endif} // Dpmi support files
  {$Endif}
  {$IFDEF LINUX}
  LnxRes,
  {$ELSE}
  {$ENDIF}
  Strings;

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

{$IFDEF FPC}
{initialization}
begin
{$ENDIF}
end.
