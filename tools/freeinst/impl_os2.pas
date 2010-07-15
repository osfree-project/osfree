unit Impl_OS2;

interface

{$I os2types.pas}

Procedure MBR_Sector(VAR Drivenum:Char; VAR MBRbuffer; IOcmd: Ulong);
Procedure Backup_MBR_sector;
Procedure Restore_MBR_sector;

Procedure Read_Disk(VAR devhandle: Hfile; VAR buf; buf_len: Ulong);
Procedure Write_Disk(VAR devhandle: Hfile; VAR buf; buf_len: Ulong);

Procedure Fat32FSctrl(DevHandle: Hfile);
Procedure Fat32WriteSector( DevHandle: hfile; ulSector: ULONG; nSectors: USHORT; VAR buf );

Procedure Open_Drive(Drive: PChar; VAR DevHandle: Hfile);
Procedure Close_Drive(VAR DevHandle: Hfile);
Procedure Lock_Drive(VAR DevHandle: Hfile);
Procedure Unlock_Drive(DevHandle: Hfile);

implementation

uses
   Os2def, Common,
{$IFDEF FPC}
   Utl, SysLow, Doscalls,
{$ELSE}
   VpUtils, VpSysLow, Os2base,
{$ENDIF}
   Strings, SysUtils, Crt, Dos;

Procedure MBR_Sector(VAR Drivenum:Char; VAR MBRbuffer; IOcmd: Ulong);

Var
  rc            : ApiRet;       // Return code
  MBRhandle     : Word;         // Filehandle
  ulDataLen     : ULong;        // Data return buffer length
  ulDataLen2    : ULong;        // Data return buffer length
//  Action        : ULong;      // Open action
  ParmRec       : packed record // Input parameter record
    Command     : Byte;         // specific to the call we make
    Head        : UShort;
    Cyl         : UShort;
    First       : UShort;
    num         : UShort;
    sec0        : UShort;
    size0       : UShort;
    End;
  ParmLen       : ULong;        // Parameter length in bytes
  DataLen       : ULong;        // Data length in bytes
  FH            : Integer;
  s3            : String[3];
  Drivenumber   : String[3];

Begin
Drivenumber := Drivenum + ':' + #0;
ulDataLen  := 2; //Sizeof(MBRHandle);
ulDataLen2 := 3; //SizeOf(DriveNumber);

rc := DosPhysicalDisk(
  INFO_GETIOCTLHANDLE,
  @MBRHandle,           // Pointer to returned data
  ulDataLen,            // Size of data buffer
  @Drivenumber[1],      // Drive number
  ulDataLen2);          // Size of drive number string

if rc <> No_Error then
begin
  Writeln('DosPhysicalDisk GetHandle error: return code = ', rc);
  Halt(1);
end;

// Length of input parameters
ParmLen := sizeof(ParmRec);

// Set up i/o parameters for bootsector
With ParmRec Do
begin
  Command := 1;           // Must be 1
  Head    := 0;
  Cyl     := 0;
  First   := 0;           // First sector number to get
  Num     := 1;           // Number of sectors to get
  Sec0    := 1;           // Number of first sector on cylinder
  Size0   := sector0Len;  // size of the sector
End;

// Read/Write 512 bytes
DataLen := Sector0Len;

{$IFDEF FPC}
rc := DosDevIOCtl(
  MBRHandle,            // Handle to device
  ioctl_PhysicalDisk,   // Category of request
  IOcmd,                // Function being requested
  ParmRec,              // Input/Output parameter list
  ParmLen,              // Maximum output parameter size
  ParmLen,              // Input:  size of parameter list
                        // Output: size of parameters returned
  MBRbuffer,            // Input/Output data area
  DataLen,              // Maximum output data size
  DataLen);             // Input:  size of input data area
{$ELSE}
rc := DosDevIOCtl(
  MBRHandle,            // Handle to device
  ioctl_PhysicalDisk,   // Category of request
  IOcmd,                // Function being requested
  @ParmRec,             // Input/Output parameter list
  ParmLen,              // Maximum output parameter size
  @ParmLen,             // Input:  size of parameter list
                        // Output: size of parameters returned
  @MBRbuffer,           // Input/Output data area
  DataLen,              // Maximum output data size
  @DataLen);            // Input:  size of input data area
{$ENDIF}

If rc <> No_Error Then
  Begin
  Writeln('DosDevIOCtl (Disk_I/O MBR sector) error: return code = ', rc);
  Halt(1);
  End;

IF IOcmd = PDSK_READPHYSTRACK Then
  Begin
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
  End;

rc := DosPhysicalDisk(
  INFO_FREEIOCTLHANDLE,
  nil,          // Pointer to returned data
  0,            // Size of data buffer
  @MBRHandle,
  ulDataLen);

if rc <> No_Error then
  begin
  Writeln('DosPhysicalDisk FreeHandle error: return code = ', rc);
  //Halt(1);
  Writeln('Press ENTER to continue');
  Readln;
  end;
end;

Procedure Read_Disk(VAR devhandle: Hfile; VAR buf; buf_len: Ulong);
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


Procedure Write_Disk(VAR devhandle: Hfile; VAR buf; buf_len: Ulong);
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


// Fat32FSctl is needed to work around bugs in FAT32.IFS still not fixed in driver  v0.99.13
Procedure Fat32FSctrl(DevHandle: Hfile);

Const
  ulDeadFace: ULONG  = $DEADFACE;
  FAT32_SECTORIO =     $9014;

Var
  ulDataLen   : ULong;                        // Input and output data size
  rc          : ApiRet;                       // Return code
  ulParamSize : Ulong;


Begin
ulDataLen := 0;
ulParamSize := sizeof( ulDeadFace );

rc := DosFSCtl( nil , 0, {$IFNDEF FPC} PULONG(ulDataLen), {$ELSE} ulDataLen, {$ENDIF}
               @ulDeadFace, ulParamSize, ulParamSize,
               FAT32_SECTORIO,
               nil,
               devhandle,
               FSCTL_HANDLE );

if rc <> No_Error then
  begin
  Writeln('DosFSCtl error: return code = ', rc);
  Halt(1);
  end;
End;

Procedure fat32WriteSector( DevHandle: hfile; ulSector: ULONG; nSectors: USHORT; VAR buf );

Const
 IOCTL_FAT32       =  IOCTL_GENERAL;
 FAT32_READSECTOR  =  $FD ;
 FAT32_WRITESECTOR =  $FF;

Var
  wsd: Packed Record
       ulSector:  ULONG;
       nSectors:  USHORT;
       End;

  ulParamSize:  ULONG ;
  ulDataSize:   ULONG ;
  rc:           ULONG ;

Begin
wsd.ulSector := ulSector;
wsd.nSectors := nSectors;

ulParamSize := sizeof( wsd );
ulDataSize := wsd.nSectors * 512;

{$IFDEF FPC}
rc := DosDevIOCtl( DevHandle, IOCTL_FAT32, FAT32_WRITESECTOR,
                  wsd, ulParamSize, ulParamSize,
                  buf, ulDataSize, ulDataSize );
{$ELSE}
rc := DosDevIOCtl( DevHandle, IOCTL_FAT32, FAT32_WRITESECTOR,
                  @wsd, ulParamSize, @ulParamSize,
                  @buf, ulDataSize, @ulDataSize );
{$ENDIF}

If rc <> No_Error Then
  Begin
  Writeln('DosDevIOCtl() : FAT32_WRITESECTOR failed, return code = ', rc);
  Halt(1);
  End;

End;

Procedure Open_Drive(Drive: PChar; VAR DevHandle: Hfile);

Var
  rc          : ApiRet; // Return code
  Action      : ULong;  // Open action
  //CbFile      : Comp;
  hdl         : LongInt;

Begin
// Opens the device to get a handle
//cbfile := 0;
//  DosOpen can be changed to DosOpenL if VP has been updated to support it
rc := DosOpen(
  Drive,                            // File path name
  hdl,                        // File handle
  Action,                           // Action taken
  0,                                // File primary allocation
  file_Normal,                      // File attribute
  open_Action_Open_if_Exists,       // Open function type
//  open_Flags_NoInherit Or
  open_Share_DenyNone  Or
  open_Access_ReadWrite Or
  OPEN_FLAGS_DASD ,                 // Open mode of the file
  nil);                             // No extended attribute

  DevHandle := hdl;

If rc <> No_Error Then
  Begin
  Writeln('DosOpen error on drive ',drive,'  Errorcode = ',rc);
  Halt(1);
  End;
End;

Procedure Close_Drive(VAR DevHandle: Hfile);

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


Procedure lock_Drive(VAR DevHandle: Hfile);

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


Procedure Unlock_Drive(DevHandle: Hfile);

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

{$IFNDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
