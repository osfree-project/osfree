{*************************************
 *  System-dependent implementation  *
 *  of low-level functions for Win32 *
 *************************************}

unit Impl_W32;

interface

type
  Hfile  = LongInt;
  ULong  = LongWord;
  UShort = Word;

procedure Open_Disk(Drive: PChar; var DevHandle: Hfile);
procedure Read_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Write_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Close_Disk(DevHandle: Hfile);
procedure Lock_Disk(DevHandle: Hfile);
procedure Unlock_Disk(DevHandle: Hfile);

procedure Read_MBR_Sector(DriveNum: char; var MBRBuffer);
procedure Write_MBR_Sector(DriveNum: char; var MBRBuffer);
procedure Backup_MBR_Sector;
procedure Restore_MBR_Sector;

implementation

uses
  Windows, Common, Strings, SysUtils, Crt, Dos;

const
  BIOSDISK_READ               = $0;
  BIOSDISK_WRITE              = $1;

  METHOD_BUFFERED             = $00000000;
  FILE_ANY_ACCESS             = $00000000;
  FILE_DEVICE_FILE_SYSTEM     = $00000009;

  FSCTL_LOCK_VOLUME           = (FILE_DEVICE_FILE_SYSTEM shl 16) or
                                (FILE_ANY_ACCESS shl 14) or 
                                ($6 shl 2) or METHOD_BUFFERED;
  FSCTL_UNLOCK_VOLUME         = (FILE_DEVICE_FILE_SYSTEM shl 16) or
                                (FILE_ANY_ACCESS shl 14) or 
                                ($7 shl 2) or METHOD_BUFFERED;

function GetNumDrives: Word;
var
  Drive    : char;
  hdl      : HANDLE;
  usDrives : Word;
begin
  usDrives := 0;
  for Drive := #$30 to #$37 do
  begin
    // create a handle to the device
    hdl     := CreateFileA(PChar('\\.\PhysicalDrive' + Drive),
                          GENERIC_READ or GENERIC_WRITE,
                          FILE_SHARE_READ or FILE_SHARE_WRITE,
                          nil,
                          OPEN_EXISTING,
                          0,
                          0);

    if hdl = INVALID_HANDLE_VALUE then
    begin
       // Not a good way to check because there could be cases
       // where a drive could be removed, for example a USB drive,
       // which might result in the \\.\PHYSICALDRIVE%d numbering
       // to have a gap
       break;
    end
    else
    begin
      inc(usDrives);
      CloseHandle(hdl);
    end;
  end;
  result := usDrives;
end;

Procedure MBR_Sector(Drivenum:Char; VAR MBRbuffer; IOcmd: Ulong);

Var
  FH            : Integer;
  s3            : String[3];
  hdl           : HANDLE;
  DataLen       : LongWord;

Begin
  hdl := CreateFile(Pchar('\\.\PhysicalDrive' + Drivenum),
                    GENERIC_READ or GENERIC_WRITE,
                    FILE_SHARE_READ or FILE_SHARE_WRITE,
                    nil, OPEN_EXISTING, 0, 0);

  if hdl = INVALID_HANDLE_VALUE then
  begin
    Writeln('CreateFile GetHandle error: return code = ', hdl);
    Halt(1);
  end;

  case IOcmd of
    BIOSDISK_READ:
    begin
      if ReadFile(hdl, MBRbuffer, 512, DataLen, nil) = false then
      begin
        Writeln('ReadFile (Disk_I/O MBR sector) error');
        CloseHandle(hdl);
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
    if WriteFile(hdl, MBRbuffer, 512, DataLen, nil) = false then
    begin
      Writeln('WriteFile (Disk_I/O MBR sector) error');
      CloseHandle(hdl);
      Halt(1);
    end;
  end;

  CloseHandle(hdl);
End;

procedure Read_MBR_Sector(DriveNum: char; var MBRBuffer);
begin
  MBR_Sector(DriveNum, MBRBuffer, BIOSDISK_READ)
end;

procedure Write_MBR_Sector(DriveNum: char; var MBRBuffer);
begin
  MBR_Sector(DriveNum, MBRBuffer, BIOSDISK_WRITE)
end;

// Backup MBR sector to a file
Procedure Backup_MBR_sector;

Var
  usNumDrives : UShort; // Data return buffer
  Drive       : Char;
Begin
  // Request a count of the number of partitionable disks in the system
  usNumDrives := GetNumDrives;

  Writeln('Windows reports ',usNumDrives,' partitionable disk(s) available.');
  Write('Input disknumber for MBR backup (1..',usNumDrives,'): ');
  Readln(Drive);

  Read_MBR_Sector(drive,sector0);
  Writeln('Press Enter to continue...');
  Readln;
End;

// Restore MBRsector from a file
Procedure Restore_MBR_sector;

Var
  usNumDrives : UShort; // Data return buffer
  Drive         : Char;
  Filename:     String;
  FH:   Integer;
Begin
  usNumDrives := GetNumDrives;

  Writeln('OS/2 reports ',usNumDrives,' partitionable disk(s) available.');
  Write('Input disknumber for MBR backup (1..',usNumDrives,'): ');
  Readln(Drive);
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
    Write_MBR_Sector(drive,sector0);
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
  rc            : LongBool;

Begin
  rc := ReadFile(devhandle,               // File Handle
                 buf,                     // String to be read
                 buf_len,                 // Length of string to be read
                 ulBytesRead,             // Bytes actually read
                 nil);
  If rc = false Then
  Begin
    Writeln('ReadFile error!');
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
  rc            : LongBool;     // Return code

Begin
  rc := WriteFile(devhandle,               // File Handle
                 buf,                     // String to be read
                 buf_len,                 // Length of string to be read
                 ulWrote,                 // Bytes actually read
                 nil);
  If rc = false Then
  Begin
    Writeln('WriteFile error!');
    Halt(1);
  End;
  Writeln(ulWrote,' Bytes written to disk');
End;


Procedure Open_Disk(Drive: PChar; var DevHandle: Hfile);
Var
  hdl         : HANDLE;

Begin
  // Opens the device to get a handle
  //cbfile := 0;
  //  DosOpen can be changed to DosOpenL if VP has been updated to support it
  hdl := CreateFile(PChar('\\.\' + Drive),
                    GENERIC_READ or GENERIC_WRITE,
                    FILE_SHARE_READ or FILE_SHARE_WRITE,
                    nil, OPEN_EXISTING, 0, 0);

  DevHandle := Word(hdl);

  If hdl = INVALID_HANDLE_VALUE Then
  Begin
    Writeln('CreateFile error on drive ', drive);
    Halt(1);
  End;
End;

Procedure Close_Disk(DevHandle: Hfile);
Begin
  CloseHandle(DevHandle)
End;


Procedure Lock_Disk(DevHandle: Hfile);
Var
  lpov  : LPOVERLAPPED;
  bytes : LongWord;
Begin
  if DeviceIoControl(DevHandle,
                     FSCTL_LOCK_VOLUME,
                     nil,
                     0,
                     nil,
                     0,
                     bytes,
                     @lpov) = false then
  begin
    writeln('Drive lock error!');
    halt(1);
  end;
End;


Procedure Unlock_Disk(DevHandle: Hfile);

Var
  lpov  : LPOVERLAPPED;
  bytes : LongWord;
Begin
  if DeviceIoControl(DevHandle,
                     FSCTL_UNLOCK_VOLUME,
                     nil,
                     0,
                     nil,
                     0,
                     bytes,
                     @lpov) = false then
  begin
    writeln('Drive lock error!');
    halt(1);
  end;
End;

{$IFDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
