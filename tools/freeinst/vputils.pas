{ÛßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßÛ}
{Û                                                       Û}
{Û      Virtual Pascal Utilities Library v2.1            Û}
{Û      ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÛ}
{Û      Copyright (C) 1995-2000 vpascal.com              Û}
{Û                                                       Û}
{ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß}

Unit VPUtils;

Interface

{$X+,T-,R-}
//Delphi+,Use32+

{$IFDEF WIN32}  {$DEFINE WIN32_DPMI32_Linux} {$ENDIF}
{$IFDEF DPMI32} {$DEFINE WIN32_DPMI32_Linux} {$ENDIF}
{$IFDEF Linux}  {$DEFINE WIN32_DPMI32_Linux} {$ENDIF}

uses
  {$IFDEF OS2}    Os2Def, {Os2Base,} {$ENDIF}
  {$IFDEF WIN32}  Windows,         {$ENDIF}
  {$IFDEF DPMI32} Dpmi32,          {$ENDIF}
  VPSysLow, Strings;

{ --- System Information functions --- }

{ --- Disk related functions --- }

type
  DriveSet   = Set of 'A'..'Z';

{ Get the volume label of the specified drive letter }
function GetVolumeLabel( Drive : Char ) : String;
{ Search for fName in Current Dir, then in the PATH }
function FileExistsOnPath( FName : string; var FullName : string ) : Boolean;
{ Get the current boot drive letter }
function GetBootDrive : Char;
{ Get the format of a drive letter }
function GetDriveType( Ch: Char ) : TDriveType;
{ Get a list of all valid drive letters }
procedure GetValidDrives( var Drives : DriveSet );


Implementation

uses
  Dos;

//threadvar
//  SaveCursor : Word;     { Used for show/hide cursor }

{ Returns the volume label of the specified drive }
function GetVolumeLabel( Drive : Char ) : String;
begin
  Result := SysGetVolumeLabel(Drive);
end;

{ Search for fName in Current Dir, then PATH environment }
function FileExistsOnPath(FName : string; var FullName : string) : Boolean;
Var
  FNameZ  : array [0..259] of Char;
  Buffer  : Array [0..259] of Char;
  Path    : String;

begin
  FileExistsOnPath := False;

  Path := Dos.GetEnv('PATH')+#0;
  StrPCopy( FNameZ, FName );
  SysFileSearch(Buffer, FNameZ, @Path[1]);
  FullName := StrPas( Buffer );
  Result := Buffer[0] <> #0;
end;

{ Get the current boot drive letter }
function GetBootDrive : Char;
begin
  Result := SysGetBootDrive;
end;

{ Get the format of a drive letter }
function GetDriveType( Ch: Char ) : TDriveType;
begin
  Result := SysGetDriveType(Ch);
end;

{ Get a list of all valid drive letters }
procedure GetValidDrives( var Drives : DriveSet );
var
  DrivesWord : Longint absolute Drives;
begin
  DrivesWord := SysGetValidDrives shl 1;
end;

(*
function DirExists( _Path: String ): Boolean;
var
  DirInfo: SearchRec;
begin
  // Remove trailing backslash, if any
  if _Path[Length(_Path)] = SysPathSep then
    Delete( _Path, Length(_Path), 1 );
  // FindFirst: Allow any attribute; require Directory attribute
  FindFirst( _Path, $3F, DirInfo );
  Result := DosError = 0;
  if Result then
    FindClose(DirInfo);
end;
 *)

{$IFNDEF FPC}
begin
{$ENDIF}
end.


