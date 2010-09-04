{*************************************
 *  System-dependent implementation  *
 *  of low-level functions for OS/2  *
 *************************************}

unit Impl_W32;

interface

type
  Hfile  = Word;
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
  BIOSDISK_READ               : LongWord    = $0;
  BIOSDISK_WRITE              : LongWord    = $1;

Procedure MBR_Sector(Drivenum:Char; VAR MBRbuffer; IOcmd: Ulong);

Var
  MBRhandle     : Word;         // Filehandle
  DataLen       : ULong;        // Data length in bytes
  FH            : Integer;
  s3            : String[3];
  Drivenumber   : String[3];
  hdl           : HANDLE;

Begin
  hdl := CreateFile('\\.\PhysicalDrive' + Drivenum,
		    GENERIC_READ or GENERIC_WRITE,
		    FILE_SHARE_READ or FILE_SHARE_WRITE,
		    0, OPEN_EXISTING, 0, 0);

  if hdl = INVALID_HANDLE_VALUE then
  begin
    Writeln('CreateFile GetHandle error: return code = ', hdl);
    Halt(1);
  end;

  case IOcmd of
    BIOSDISK_READ:
    begin
      if ReadFile(hdl, MBRbuffer, 512, DataLen, nil) = 0 then
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
    if WriteFile(hdl, MBRbuffer, 512, DataLen, nil) = 0 then
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
  ulDataLen   : ULong;  // Data return buffer length
  rc          : ApiRet; // Return code
  Drive       : Char;

Begin
ulDataLen := sizeof(UShort);
// Request a count of the number of partitionable disks in the system
rc := DosPhysicalDisk(
  info_Count_Partitionable_Disks,
  @usNumDrives, // Pointer to returned data
  ulDataLen,    // Size of data buffer
  nil,          // No parameter for this function
  0);
if rc <> No_Error then
  Begin
  Writeln('DosPhysicalDisk error: return code = ', rc);
  Halt(1);
  End;
Writeln('OS/2 reports ',usNumDrives,' partitionable disk(s) available.');
Write('Input disknumber for MBR backup (1..',usNumDrives,'): ');
Readln(Drive);

MBR_Sector(drive,sector0,PDSK_READPHYSTRACK);
Writeln('Press Enter to continue...');
Readln;
End;

// Restore MBRsector from a file
Procedure Restore_MBR_sector;

Var
  usNumDrives : UShort; // Data return buffer
  ulDataLen   : ULong;  // Data return buffer length
  //ulDataLen2   : ULong; // Data return buffer length
  rc          : ApiRet; // Return code
  DevHandle1   : Ushort;
  Drive         : Char;
  Filename:     String;
  FH:   Integer;

Begin
ulDataLen := sizeof(UShort);
// Request a count of the number of partitionable disks in the system
rc := DosPhysicalDisk(
  info_Count_Partitionable_Disks,
  @usNumDrives, // Pointer to returned data
  ulDataLen,    // Size of data buffer
  nil,          // No parameter for this function
  0);
if rc <> No_Error then
begin
  Writeln('DosPhysicalDisk error: return code = ', rc);
  Halt(1);
end;
Writeln('OS/2 reports ',usNumDrives,' partitionable disk(s) available.');
Write('Input disknumber for MBR backup (1..',usNumDrives,'): ');
Readln(Drive);
ulDataLen := sizeof(DevHandle1);
//ulDataLen2 := 3;
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
  MBR_Sector(drive,sector0,PDSK_WritePHYSTRACK);
  End
 Else Writeln('Sorry, the file ',filename,' returned error ',-FH);
Writeln('Press Enter to continue...');
Readln;
End;

Procedure Read_Disk(devhandle: Hfile; VAR buf; buf_len: Ulong);
Var
  ulBytesRead   : ULONG;          // Number of bytes read by DosRead
  rc            : APIRET;         // Return code
  s3            : STRING[3];
  FH            : Integer;        // File handle for backup file

Begin
rc := DosRead (DevHandle,               // File Handle
               buf,                     // String to be read
               buf_len,                 // Length of string to be read
               ulBytesRead);            // Bytes actually read
If (rc <> NO_ERROR) Then
  Begin
  Writeln('DosRead error: return code = ', rc);
  Halt(1);
  End;

// Writeln('Read_Disk: ', ulBytesRead,' Bytes read.' );

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
  //ulLocal       : ULONG;        // File pointer position after DosSetFilePtr
  rc            : APIRET;       // Return code

Begin
rc := DosWrite (DevHandle,      // File handle
                buf,            // String to be written
                buf_len,        // Size of string to be written
                ulWrote);       // Bytes actually written
If (rc <> NO_ERROR) Then
  Begin
  Writeln('DosWrite error: return code = ', rc);
  Halt(1);
  End;
Writeln(ulWrote,' Bytes written to disk');
End;


Procedure Open_Disk(Drive: PChar; var DevHandle: Hfile);

Var
  rc          : ApiRet; // Return code
  Action      : ULong;  // Open action
  hdl         : LongInt;

Begin
// Opens the device to get a handle
//cbfile := 0;
//  DosOpen can be changed to DosOpenL if VP has been updated to support it
rc := DosOpen(
  Drive,                            // File path name
  Hdl,
  Action,                           // Action taken
  0,                                // File primary allocation
  file_Normal,                      // File attribute
  open_Action_Open_if_Exists,       // Open function type
//  open_Flags_NoInherit Or
  open_Share_DenyNone  Or
  open_Access_ReadWrite Or
  OPEN_FLAGS_DASD ,                 // Open mode of the file
  nil);                             // No extended attribute

DevHandle := Word(hdl);

If rc <> No_Error Then
  Begin
  Writeln('DosOpen error on drive ',drive,'  Errorcode = ',rc);
  Halt(1);
  End;
End;

Procedure Close_Disk(DevHandle: Hfile);

Var
  rc          : ApiRet; // Return code
  //Action      : ULong;  // Open action

Begin
rc := DosClose(DevHandle);
If rc <> No_Error Then
  Begin
  Writeln('DosClose ERROR. RC = ',rc);
  End;
End;


Procedure Lock_Disk(DevHandle: Hfile);

Var
  rc          : ApiRet;   // Return code
  //Action      : ULong;    // Open action
  //ParmRec     : packed record    // Input parameter record
  //  Command : ULong;      // specific to the call we make
  //  Addr0   : ULong;
  //  Bytes   : UShort;
  //  end;
  //ParmLen     : ULong;    // Parameter length in bytes
  //DataLen     : ULong;    // Data length in bytes
  lockbyte    : ULong;      //command and data parameter

Begin
// First open the device to get a handle

{$IFDEF FPC}
rc := DosDevIOCtl(
  DevHandle,                  // Handle to device
  ioctl_Disk,                 // Category of request
  dsk_LockDrive,              // Function being requested
  LockByte,                   // Input/Output parameter list
  1,                          // Maximum output parameter size
  LockByte,                   // Input:  size of parameter list
                              // Output: size of parameters returned
  LockByte,                   // Input/Output data area
  1,                          // Maximum output data size
  LockByte);                  // Input:  size of input data area
                              // Output: size of data returned
{$ELSE}
rc := DosDevIOCtl(
  DevHandle,                  // Handle to device
  ioctl_Disk,                 // Category of request
  dsk_LockDrive,              // Function being requested
  @LockByte,                  // Input/Output parameter list
  1,                          // Maximum output parameter size
  @LockByte,                  // Input:  size of parameter list
                              // Output: size of parameters returned
  @LockByte,                  // Input/Output data area
  1,                          // Maximum output data size
  @LockByte);                 // Input:  size of input data area
                              // Output: size of data returned
{$ENDIF}

If rc <> No_Error Then
  Begin
  Writeln('Drive lock error: return code = ', rc);
  // Halt(1);
  End
else
  Begin
  // Writeln('Drive is now locked !!!');
  End;
End;


Procedure Unlock_Disk(DevHandle: Hfile);

Var
  rc          : ApiRet; // Return code
  //Action      : ULong;  // Open action
  //ParmRec     : packed record  // Input parameter record
  //  Command : ULong;    // specific to the call we make
  //  Addr0   : ULong;
  //  Bytes   : UShort;
  //end;
  //ParmLen     : ULong;  // Parameter length in bytes
  //DataLen     : ULong;  // Data length in bytes
  lockbyte    : LongInt;   //command and data parameter

Begin

{$IFDEF FPC}
rc := DosDevIOCtl(
  DevHandle,                   // Handle to device
  ioctl_Disk,                  // Category of request
  dsk_UnlockDrive,             // Function being requested
  LockByte,                    // Input/Output parameter list
  1,                           // Maximum output parameter size
  LockByte,                    // Input:  size of parameter list
                               // Output: size of parameters returned
  LockByte,                    // Input/Output data area
  1,                           // Maximum output data size
  LockByte);                   // Input:  size of input data area
                               // Output: size of data returned
{$ELSE}
rc := DosDevIOCtl(
  DevHandle,                   // Handle to device
  ioctl_Disk,                  // Category of request
  dsk_UnlockDrive,             // Function being requested
  @LockByte,                   // Input/Output parameter list
  1,                           // Maximum output parameter size
  @LockByte,                   // Input:  size of parameter list
                               // Output: size of parameters returned
  @LockByte,                   // Input/Output data area
  1,                           // Maximum output data size
  @LockByte);                  // Input:  size of input data area
{$ENDIF}

If rc <> No_Error Then
  Begin
  Writeln('DosDevIOCtl (UNLOCK) error: return code = ', rc);
  //  Halt(1);
  End
Else
  Begin
  //  Writeln('Drive UNLOCKed successfully ');
  End;

End;

{$IFDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
