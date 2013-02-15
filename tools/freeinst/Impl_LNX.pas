{*************************************
 *  System-dependent implementation  *
 *  of low-level functions for Win32 *
 *************************************}

{$IFDEF FPC}
{$LongStrings ON}
{$ENDIF}

unit Impl_LNX;

interface

type
  Hfile  = LongInt;
  ULong  = LongWord;
  UShort = Word;
{$I os2types.pas}
procedure Open_Disk(Drive: AnsiString; var DevHandle: Hfile);
procedure Read_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Write_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Close_Disk(DevHandle: Hfile);
procedure Lock_Disk(DevHandle: Hfile);
procedure Unlock_Disk(DevHandle: Hfile);

procedure Read_MBR_Sector(DriveNum: AnsiString; var MBRBuffer);
procedure Write_MBR_Sector(DriveNum: AnsiString; var MBRBuffer);
procedure Backup_MBR_Sector;
procedure Restore_MBR_Sector;

implementation

uses
  Common, Strings, SysUtils, Crt, Dos;

const
  BIOSDISK_READ               = $0;
  BIOSDISK_WRITE              = $1;

procedure GetDrives;
var
  line     : String;
  f        : Text;
begin
    // create a handle to the device
    assign(f, '/proc/partitions');
    
    {$I+}
    reset(f);
    {$I-}

    if IOResult <> 0 then
    begin
       writeln('Can''t open /proc/partitions');
       exit;
    end;
    while not eof(f) do
    begin
      readln(f, line);
      writeln(line);
    end;
end;

Procedure MBR_Sector(Drivenum:AnsiString; VAR MBRbuffer; IOcmd: Ulong);

Var
  FH            : Integer;
  s3            : String[3];
  hdl           : LongInt;
  DataLen       : LongWord;
  s             : String;

Begin
  s := '/dev/' + Drivenum;
  hdl := FileOpen(s, fmOpenRead OR fmOpenWrite OR fmShareDenyWrite);

  if hdl <= 0 then
  begin
    Writeln('FileOpen error: return code = ', hdl);
    Halt(1);
  end;

  case IOcmd of
    BIOSDISK_READ:
    begin
      DataLen := FileRead(hdl, MBRbuffer, 512);
      if DataLen <= 0 then
      begin
        Writeln('FileRead (Disk_I/O MBR sector) error');
        FileClose(hdl);
        Halt(1);
      end;
      i:=0;
      Repeat
        Str(i:3,s3);
        If pos(' ',s3) = 1 Then s3[1] := '0';
        If pos(' ',s3) = 2 Then s3[2] := '0';
        i:=succ(i);
        If I > 999 Then exit;
      Until NOT FileExists ('MBR_sect.'+s3);
      Writeln('Backup bootsector file = MBR_sect.',s3);
      FH := FileCreate( 'MBR_sect.'+s3);
      FileWrite( FH, Sector0, Sector0Len );
      FileClose( FH );
    end;
    BIOSDISK_WRITE:
    begin
      if DataLen <= 0 then
      begin
        Writeln('FileWrite (Disk_I/O MBR sector) error');
        FileClose(hdl);
        Halt(1);
      end;
    end;
  end;

  FileClose(hdl);
End;

procedure Read_MBR_Sector(DriveNum: AnsiString; var MBRBuffer);
begin
  MBR_Sector(DriveNum, MBRBuffer, BIOSDISK_READ)
end;

procedure Write_MBR_Sector(DriveNum: AnsiString; var MBRBuffer);
begin
  MBR_Sector(DriveNum, MBRBuffer, BIOSDISK_WRITE)
end;

// Backup MBR sector to a file
Procedure Backup_MBR_sector;

Var
  Drive      : AnsiString;
  Disk       : PChar;
Begin
  Writeln('Linux reports the following partitionable disk(s) available:');
  // Request the partitions list in the system
  GetDrives;

  Write('Input the boot device here: ');
  Write('/dev/'); Readln(Drive);
  Disk := PChar('/dev/' + Drive);

  Read_MBR_Sector(Disk, sector0);
  Writeln('Press Enter to continue...');
  Readln;
End;

// Restore MBRsector from a file
Procedure Restore_MBR_sector;

Var
  Disk        : PChar;
  Drive       : AnsiString;
  Filename    : String;
  FH          : Integer;
Begin
  Writeln('Linux reports the following partitionable disk(s) available:');
  GetDrives;
  Write('Input the boot device here: ');
  Write('/dev/'); Readln(Drive);
  Disk := PChar('/dev/' + Drive);
  
  Writeln('Enter name of the bootsectorfile to restore');
  Write('(Default is MBR_sect.000): ');
  Readln(filename);

  If filename = '' Then Filename := 'MBR_sect.000';
  FH := FileOpen( filename, fmOpenRead OR fmShareDenyNone);
  If FH > 0 Then
  Begin
    Writeln('Restoring ',filename, 'to bootsector');
    FileRead( FH, Sector0, Sector0Len );
    FileClose( FH );
    Write_MBR_Sector(Disk, sector0);
  End
  Else
    Writeln('Sorry, the file ',filename,' returned error ',-FH);
  Writeln('Press Enter to continue...');
  Readln;
End;

Procedure Read_Disk(devhandle: Hfile; VAR buf; buf_len: Ulong);
Var
  ulBytesRead   : ULONG;          // Number of bytes read by DosRead
  s3            : STRING[3];
  FH            : Integer;        // File handle for backup file

Begin
  ulBytesRead := FileRead(devhandle,               // File Handle
                 buf,                     // String to be read
                 buf_len);                // Length of string to be read
  If ulBytesRead <= 0 Then
  Begin
    Writeln('FileRead error!');
    Halt(1);
  End;

  // Write backup file of data read
  i := 0;
  Repeat
    Str(i:3,s3);
    If pos(' ',s3) = 1 Then s3[1] := '0';
    If pos(' ',s3) = 2 Then s3[2] := '0';
    i:=succ(i);
    If I > 999 Then exit;
  Until NOT FileExists ('Drive-'+drive1[1]+'.'+s3);
  Writeln('Backup bootsector file created:  Drive-',drive1[1],'.',s3);
  FH := FileCreate( 'Drive-'+drive1[1]+'.'+s3);
  FileWrite( FH, buf, ulBytesRead );
  FileClose( FH );
End;


Procedure Write_Disk(devhandle: Hfile; VAR buf; buf_len: Ulong);
Var
  ulWrote       : ULONG;        // Number of bytes written by DosWrite

Begin
  ulWrote := FileWrite(devhandle,        // File Handle
                 buf,                    // String to be read
                 buf_len);               // Length of string to be read
  If ulWrote <= 0 Then
  Begin
    Writeln('FileWrite error!');
    Halt(1);
  End;
  Writeln(ulWrote,' Bytes written to disk');
End;


Procedure Open_Disk(Drive: AnsiString; var DevHandle: Hfile);
Var
  hdl         : LongInt;

Begin
  // Opens the device to get a handle
  hdl := FileOpen(Drive, fmOpenRead or fmOpenWrite or fmShareDenyWrite);

  DevHandle := Hfile(hdl);

  If hdl <= 0 Then
  Begin
    Writeln('FileOpen error on drive ', drive);
    Halt(1);
  End;
End;

Procedure Close_Disk(DevHandle: Hfile);
Begin
  FileClose(DevHandle)
End;


Procedure Lock_Disk(DevHandle: Hfile);
Begin
End;


Procedure Unlock_Disk(DevHandle: Hfile);
Begin
End;

{$IFDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
