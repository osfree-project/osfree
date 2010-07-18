{ÛßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßÛ}
{Û                                                       Û}
{Û      Virtual Pascal Utilities Library v2.1            Û}
{Û      ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÛ}
{Û      Copyright (C) 1995-2000 vpascal.com              Û}
{Û                                                       Û}
{ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß}

Unit Utl;

Interface

{$X+,T-,R-}
//Delphi+,Use32+

{$IFDEF WIN32}  {$DEFINE WIN32_DPMI32_Linux} {$ENDIF}
{$IFDEF DPMI32} {$DEFINE WIN32_DPMI32_Linux} {$ENDIF}
{$IFDEF Linux}  {$DEFINE WIN32_DPMI32_Linux} {$ENDIF}

uses
  {$IFDEF OS2}    Os2Def, {Os2Base,} {$ENDIF}
  {$IFDEF WIN32}  Windows,         {$ENDIF}
  {$IFDEF DPMI32} {Dpmi32,}          {$ENDIF}
  SysLow, Strings;

{ --- System Information functions --- }

{ --- Disk related functions --- }

type
  DriveSet   = Set of 'A'..'Z';

{ Get the current boot drive letter }
function GetBootDrive : Char;
{ Get the format of a drive letter }
function GetDriveType( Ch: Char ) : TDriveType;
{ Get a list of all valid drive letters }
procedure GetValidDrives( var Drives : DriveSet );


Implementation

uses
  Dos;

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
  DrivesWord : Longint; // absolute Drives;
  i: integer;
begin
  DrivesWord := SysGetValidDrives;
  Drives := [];
  for i := 0 to 31 do
    if ((DrivesWord shr i) and 1) = 1 then
      Drives := Drives + [chr(ord('A') + i)];
end;

{$IFDEF FPC}
{initialization}
begin
{$ENDIF}
end.


