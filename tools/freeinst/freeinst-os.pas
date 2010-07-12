
Program osFree_Install;

{&Linker
DESCRIPTION '@#osFree:0.0.1.16á#@##1## 11 may 2010 11:05:10ÿÿÿ  Asus SMP::en:us:1::@@  Installation app for FreeLDR'
}

Uses Os2def, Os2base, Strings, SysUtils, VPUtils, VpSysLow, Crt, Dos;


// Compiler settings
{$B-,H+,I+,J+,P+,Q+,R+,S+,T-,V-,W-,X+}
{&AlignCode+,AlignData+,AlignRec-,Cdecl-,Delphi+,Frame+}
{&Open32-,Optimise+,OrgName-,Speed+}
{$M 32768}



Type
  Str3 =        Array[0..2] Of Char;
  Str8 =        Array[0..7] Of Char;
  Str16 =       Array[0..15] Of Char;
  Possible_IFS = dtHDFAT..dtHDJFS ;

Const
  version       = 'osFree installer by osFree Project 2010';
//  Possible_IFS: ARRAY [1..4] of tDrivetype = (dtHDFAT,DThdfat32,dtHDHPFS,dtHDJFS);
  osFree: Str8  = '[osFree]' ;
  Sector0Len    = 512;
  BblockLen     = 8192;
//  8kLen         = 8192;
//  16kLen        = 16384;
  cdfs: Str16   = 'cdfs'#0#0#0#0#0#0#0#0#0#0#0#0;
  ntfs: Str16   = 'ntfs'#0#0#0#0#0#0#0#0#0#0#0#0;
  ext2: Str16   = 'ext2'#0#0#0#0#0#0#0#0#0#0#0#0;
  hpfs: Str16   = 'hpfs'#0#0#0#0#0#0#0#0#0#0#0#0;
  fat:  Str16   = 'fat'#0#0#0#0#0#0#0#0#0#0#0#0#0;
  jfs:  Str16   = 'jfs'#0#0#0#0#0#0#0#0#0#0#0#0#0;

Type
  Sector0Buf =  ARRAY [0..511] of Char;
  hdr =         Record                 // Total size 60 bytes
                jmp:            Word;
                FS_Len:         Word;
                Preldr_Len:     Word;
                data2:          Array[0..7] Of Byte;
                PartitionNr:    Byte;
                data3:          Array[0..28] Of Byte;
                FS:             Str16;
                Data4:          Array[0..BblockLen-61] Of Char;
                End;


  BblockBuf =   Array[0..Bblocklen-1] of Char;
  FAT32Buf =    Record
                // Bootsector (sector 0 = 512 bytes)
                jmp1:           Array[0..2] Of Byte;
                Oem_Id:         Str8;
                Bpb:            Array[0..78] Of Char;
                Boot_Code:      Array[0..410] Of Byte;
                FSD_LoadSeg:    Word;
                FSD_Entry:      Word;
                FSD_Len:        Byte;
                FSD_Addr:       Cardinal;
                Boot_End:       Word;
                Sector1:        Array[0..511] Of Byte;
                // ldr starts from sector 2
                jmp2:           Word;
                FS_Len:         Word;
                Preldr_Len:     Word;
                data2:          Array[0..7] Of Byte;
                PartitionNr:    Byte;
                data3:          Array[0..28] Of Byte;
                FS:             Str16;
                Data4:          Array[0..BblockLen-(61+1024)] Of Char;
                End;

  HPFSBuf =     Record
                // Bootsector (sector 0 = 512 bytes)
                jmp1:           Array[0..2] Of Byte;
                Oem_Id:         Str8;
                Bpb:            Array[0..50] Of Char;
                Boot_Code:      Array[0..438] Of Byte;
                FSD_LoadSeg:    Word;
                FSD_Entry:      Word;
                FSD_Len:        Byte;
                FSD_Addr:       Cardinal;
                Boot_End:       Word;
                // Sector1:        Array[0..511] Of Byte;
                // ldr starts from sector 1
                jmp2:           Word;
                FS_Len:         Word;
                Preldr_Len:     Word;
                data2:          Array[0..7] Of Byte;
                PartitionNr:    Byte;
                data3:          Array[0..28] Of Byte;
                FS:             Str16;
                Data4:          Array[0..BblockLen-(61+512)] Of Char;
                End;



Var
  Sector0:      Sector0Buf;
  BlockBuf:     Hdr;
  Bbuf:         BBlockbuf;
  F32Buf:       FAT32Buf;
  HPbuf:        HPFSbuf;
  i:            Integer;
  DevHandle:    Hfile;
  Drive1:       String;
  Drive2:       Array[0..2] of Char ;
  Drive:        PChar;
  DriveT:       tDrivetype;     // FS type on selected drive
  PartNr:       Byte;           // Partition number
  ch    :       Char;           // Menu selector
  ifsbuf:       Array[0..1000000] of Char;


{******************************************************************************************}


Procedure MBR_Sector(VAR Drivenum:Char; VAR MBRbuffer; IOcmd: Ulong);

Var
  rc            : ApiRet;       // Return code
  MBRhandle     : Word;         // Filehandle
  ulDataLen     : ULong;        // Data return buffer length
  ulDataLen2    : ULong;        // Data return buffer length
//  Action        : ULong;      // Open action
  ParmRec       : record        // Input parameter record
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
  Drivenumber   : String;

Begin
Drivenumber := Drivenum + ':' + #0;
ulDataLen := Sizeof(MBRHandle);
ulDataLen2 := 3;                //SizeOf(DriveNumber);

rc := DosPhysicalDisk(
  INFO_GETIOCTLHANDLE,
  @MBRHandle,           // Pointer to returned data
  ulDataLen,            // Size of data buffer
  Drivenumber,          // Drive number
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
rc := DosDevIOCtl(
  MBRHandle,            // Handle to device
  ioctl_PhysicalDisk,   // Category of request
  IOcmd,                // Function being requested
  @ParmRec,             // Input/Output parameter list
  ParmLen,              // Maximum output parameter size
  @ParmLen,             // Input:  size of parameter list
                        // Output: size of parameters returned
  @MBRbuffer,           // Input/Output data area
  Datalen,              // Maximum output data size
  @DataLen);            // Input:  size of input data area

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
  ulLocal       : ULONG;        // File pointer position after DosSetFilePtr
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

rc := DosFSCtl( nil , 0, uldatalen,
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
  wsd: Record
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

rc := DosDevIOCtl( DevHandle, IOCTL_FAT32, FAT32_WRITESECTOR,
                  @wsd, ulParamSize, @ulParamSize,
                  @buf, ulDataSize, @ulDataSize );

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
  CbFile      : Comp;

Begin
// Opens the device to get a handle
cbfile := 0;
//  DosOpen can be changed to DosOpenL if VP has been updated to support it
rc := DosOpen(
  Drive,                            // File path name
  DevHandle,                        // File handle
  Action,                           // Action taken
  0,                                // File primary allocation
  file_Normal,                      // File attribute
  open_Action_Open_if_Exists,       // Open function type
//  open_Flags_NoInherit Or
  open_Share_DenyNone  Or
  open_Access_ReadWrite Or
  OPEN_FLAGS_DASD ,                 // Open mode of the file
  nil);                             // No extended attribute

If rc <> No_Error Then
  Begin
  Writeln('DosOpen error on drive ',drive,'  Errorcode = ',rc);
  Halt(1);
  End;
End;

Procedure Close_Drive(VAR DevHandle: Hfile);

Var
  rc          : ApiRet; // Return code
  Action      : ULong;  // Open action

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
  Action      : ULong;    // Open action
  ParmRec     : record    // Input parameter record
    Command : ULong;      // specific to the call we make
    Addr0   : ULong;
    Bytes   : UShort;
    end;
  ParmLen     : ULong;    // Parameter length in bytes
  DataLen     : ULong;    // Data length in bytes
  lockbyte    : char;     //command and data parameter

Begin
// First open the device to get a handle

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
  Action      : ULong;  // Open action
  ParmRec     : record  // Input parameter record
    Command : ULong;    // specific to the call we make
    Addr0   : ULong;
    Bytes   : UShort;
  end;
  ParmLen     : ULong;  // Parameter length in bytes
  DataLen     : ULong;  // Data length in bytes
  lockbyte    : char;   //command and data parameter

Begin

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
                               // Output: size of data returned

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

// Rewrite start of preldr0 file acording to filesystem needs
Procedure Fix_Preldr0(DriveT:TdriveType);
Var
  FH:           Integer;
  Count:        Word;
  ldr0:         Record
                head:   Array[0..1] Of Byte;
                l1:     Word;
                l2:     Word;
                head2:  Array[0..7] Of Byte;
                PartNr: Byte;                   // NOT used anymore
                head3:  Array[0..28] Of Byte;
                FS:     str16;
                End;
  Length :      Word;

Begin
FillChar(ldr0, sizeof(ldr0),0);
FH := FileOpen( Drive1+'\boot\loader\preldr0.mdl', fmOpenReadWrite OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count := Fileread( FH, ldr0, 60 );
  length := FileSeek( FH, 0, 2 );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening preldr0.mdl');
  Halt(1);
  End;

With ldr0 DO
  Begin
  //  PartNr := PartNr1;
  l1 := 0;
  l2 := Length;

  Case DriveT Of
    dtHDFat,
    dtFloppy:   Begin
                l2 := 0;
                FS := fat;
                End;
    dtHDFat32:  FS := fat;
    dtHDHPFS:   FS := hpfs;
    dtHDJFS:    FS := jfs;
    End;
  End;
FileSeek( FH, 0, 0 );
FileWrite( FH, ldr0, 60 );
FileClose( FH );
End;


// Install MBR for FreeLDR

Procedure Install_MBR;
Type
  drvtype = record
    drvletter: char;
    PartNr: byte;
    partname: String[16];
    End;

VAR
  Drive:        Char;
  bootNr:       Byte;
  FH:           Integer;
  FreeMBR:      Sector0Buf;
  drvletterbuf: ARRAY [0..255] of Drvtype;
  line1:        String;
  File1:        Text;
  LetterNr:     Word;
  ReadLetters:  Boolean;
  Prim, Log, i: Word;


Begin
Write('Input disknumber for new MBR install (1''st disk is nr 1): ');
Readln(Drive);
ClrScr;
Writeln;
Writeln('  Your disk number ',drive,' has the following partitions available for booting: ');
exec('cmd.exe','/C lvm.exe /Query:All,'+Drive+' > lvmtemp.tmp');
assign(file1,'lvmtemp.tmp');
reset(file1);
readln(file1,line1);  // not used
letterNr := 1;
readletters := true;
Prim := 0;
log := 4;
While Not Eof( file1 ) Do
  Begin
  Readln (file1,line1);
  If line1 = ''
   Then
     Begin
     readletters := false;
     Readln(file1);
     Readln(file1);
     Readln(file1);
     End
   Else If readletters = true
    Then
     Begin
     If pos(':',line1) = 2 Then With drvletterbuf[letterNr] Do
       Begin
       drvletter := line1[1];
       PartName := copy(line1,4,16);
       letterNr := succ(letterNr);
       End;
     End
    Else
      Begin
      If pos('Primary',line1) = 35 Then
        Begin
        Prim := succ(Prim);
        For i := 1 to letterNr-1 Do With drvletterbuf[i] Do
          Begin
          If copy(line1,4,16) = PartName Then
            Begin
            PartNr := Prim;
            End
          End;
        End
      Else If pos('Logical',line1) = 35 Then
        Begin
        If Log = 3
         Then Prim := Succ (Prim);
        Log := Succ (log);
        For i := 1 to letterNr-1 Do With drvletterbuf[i] Do
          Begin
          If copy(line1,4,16) = PartName Then
            Begin
            PartNr := Log;
            End
          End;

        End;
      End;
  End; { while }
Writeln(' Nr  Drive  Partition Name');
For i := 1 To letterNr-1 Do With drvletterbuf[i] Do
  Begin
  Writeln(PartNr:3,'    ',drvletter,':   ',PartName);
  End;
Close(file1);
DeleteFile('lvmtemp.tmp');

Writeln;
Writeln('Primary partitions are numbered from 1-4    ');
Writeln('Logical partitions are numbered from 5-255');
Writeln;
Write('Which Partition number do you want to boot from (see numbers above) ? ');
Readln(BootNr);
MBR_Sector(drive,sector0,PDSK_READPHYSTRACK);
FH := FileOpen( drive1+'\boot\sectors\mbr.bin', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  FileRead( FH, FreeMBR, Sector0Len );
  FileClose( FH );
  StrMove( @FreeMBR[$1b8], @Sector0[$1b8], 72 );    //  Rewrite Partition Table and NTFS sig from HD
  FreeMBR[$1bc] := chr(BootNr);                     //  Insert partition bootnumber
  FreeMBR[$1bd] := chr($80);                        //  Insert disk boot number
  Sector0 := FreeMBR;
  MBR_Sector(drive,sector0,PDSK_WritePHYSTRACK);
  Writeln(#10,'Your MBR have been upgraded to FreeLDR , Press <Enter> to continue ');
  Readln;
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening MBR.bin');
  Halt(1);
  End;
End;


// Install FreeLDR on FAT (12/16) partition + floppy
Procedure Install_Fat;
Var
  FH          : Integer;
  FreeLdr0:     Sector0Buf;

Begin
Writeln('Going to install FreeLDR on ',drive1);
PartNr := 0;
Open_Drive(Drive,DevHandle);            // Get drivehandle
lock_drive(devhandle);
Read_Disk(devhandle,Sector0,Sector0Len);
unlock_drive(devhandle);
Close_Drive(DevHandle);
FH := FileOpen( drive1+'\boot\sectors\fatboot.bin', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  FileRead( FH, FreeLdr0, Sector0Len );
  FileClose( FH );

  // Set OEM ID
  StrMove( FreeLdr0+3, osFree, 8 );

  //  Rewrite BPB from HD
  StrMove( FreeLdr0+11, @Sector0[11], 51 );

  Sector0 := FreeLdr0;
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening FatBoot.bin');
  Halt(1);
  End;
Open_Drive(Drive,DevHandle);            // Get drivehandle
lock_drive(devhandle);
Write_Disk(devhandle,Sector0,Sector0Len);
unlock_drive(devhandle);
Close_Drive(DevHandle);
Fix_Preldr0(DriveT);
Writeln('Installation of osFree on partition ',drive1,' ended successfully.');
Write('Press <Enter> to continue... ');
Readln;
End;


// FAT32 install
//
Procedure Install_Fat32;
Type
  Free0 = Record
        jmp:            Array[0..2] Of Byte;
        Oem_Id:         Str8;
        Bpb:            Array[0..78] Of Char;
        Boot_Code:      Array[0..410] Of Byte;
        FSD_LoadSeg:    Word;
        FSD_Entry:      Word;
        FSD_Len:        Byte;
        FSD_Addr:       Cardinal;
        Boot_End:       Word;
        End;

Var
  FreeSect0 :           Free0;
  Fs0,bb,F32bb:         Pchar;
  FH:                   Integer;
  mini_hdr:             hdr;
  Count, Count1:        Word;

Begin
Writeln('Going to install FreeLDR on ',drive1);
Open_Drive(Drive,DevHandle);            // Get drivehandle
lock_drive(devhandle);
Read_Disk(devhandle,F32Buf,BblockLen);
unlock_drive(devhandle);
Close_Drive(DevHandle);
F32bb := @F32Buf;
FH := FileOpen( drive1+'\boot\sectors\bootsect_1.bin', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  FileRead( FH, FreeSect0, Sector0Len );
  FileClose( FH );
  With FreeSect0 Do
    Begin
    Oem_Id := osFree;
    //  Rewrite BPB from HD
    //  For FAT32:                 79 bytes
    StrMove( bpb, @F32Bb[11], 79);
    FSD_LoadSeg:=$0800;
    FSD_Entry:=0;
    FSD_Len:=(BblockLen - 1024) div 512;
    FSD_Addr:=2;
    End;
  StrMove (F32Bb, @FreeSect0, 512);
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening ',drive1,'\boot\sectors\BootSect_1.bin');
  Halt(1);
  End;
FH := FileOpen( drive1+'\boot\loader\preldr_mini.mdl', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count := FileRead( FH, F32bb[1024], Sizeof( Blockbuf ) );
  FileClose( FH );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening preldr_mini.mdl');
  Halt(1);
  End;
FH := FileOpen( drive1+'\boot\loader\fsd\fat.mdl', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count1 := FileRead( FH, F32bb[1024+count], Sizeof( Blockbuf ) - count );
  FileClose( FH );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening fat.mdl');
  Halt(1);
  End;

With F32Buf Do
  Begin
  FS_Len      := Count1;
  Preldr_Len  := Count;
  PartitionNr := 0;             // Previous = PartNr not used anymore
  FS := fat;
  End;

Open_Drive(Drive,DevHandle);    // Get drivehandle
lock_drive(devhandle);
Fat32FSctrl(Devhandle);
fat32WriteSector( DevHandle, 0, BBlockLen DIV 512, F32Buf );
unlock_drive(devhandle);
Close_Drive(DevHandle);
Fix_Preldr0(DriveT);
Writeln('Installation of osFree Bootsectors on FAT32 partition ',drive1,' ended successfully.');
Write('Press <Enter> to continue... ');
Readln;
End;



// HPFS install
// BootBlock max 16 sectors
Procedure Install_HPFS;
Const
    osFree: Str8  = 'IBM Free' ;

Type
  Free0 = Record
        jmp:            Array[0..2] Of Byte;
        Oem_Id:         Str8;
        Bpb:            Array[0..50] Of Char;
        Boot_Code:      Array[0..438] Of Byte;
        FSD_LoadSeg:    Word;
        FSD_Entry:      Word;
        FSD_Len:        Byte;
        FSD_Addr:       Cardinal;
        Boot_End:       Word;
        End;

Var
  FreeSect0 :           Free0;
  Fs0,bb,F32bb:         Pchar;
  FH:                   Integer;
  mini_hdr:             hdr;
  Count, Count1:        Word;

Begin
FillChar(HPbuf,SizeOf(HPbuf),0);
PartNr := 0;
Open_Drive(Drive,DevHandle);                   // Get drivehandle
lock_drive(devhandle);
Read_Disk(devhandle,HPbuf,BblockLen);          // Read 8k
unlock_drive(devhandle);
Close_Drive(DevHandle);
F32bb := @HPbuf;
FH := FileOpen( drive1+'\boot\sectors\bootsect_1.bin', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  FileRead( FH, FreeSect0, Sector0Len );
  FileClose( FH );
  With FreeSect0 Do
    Begin
    Oem_Id := osFree;                           // Insert new OEM ID, 'IBM is required as first 3 letters or mounting of IFS will fail
    StrMove( bpb, @F32Bb[11], 51);              // Copy bpb from HD, 51 bytes
    FSD_LoadSeg:=$0800;                         // FSD load address
    FSD_Entry:=0;                               // FSD entry point
    FSD_Len:=(BblockLen - 512) div 512;         // Length of boot block in sectors
    FSD_Addr:=1;                                // Startsector for boot block
    End;
  StrMove (F32Bb, @FreeSect0, 512);
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening BootSect_1.bin');
  Halt(1);
  End;
FH := FileOpen( drive1+'\boot\loader\preldr_mini.mdl', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count := FileRead( FH, F32bb[512], BblockLen - 512 );
  FileClose( FH );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening preldr_mini.mdl');
  Halt(1);
  End;
FH := FileOpen( drive1+'\boot\loader\fsd\hpfs.mdl', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count1 := FileRead( FH, F32bb[512+count], BblockLen - ( count + 512 ) );
  FileClose( FH );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening hpfs.mdl');
  Halt(1);
  End;

With HPbuf Do
  Begin
  FS_Len      := Count1;
  Preldr_Len  := Count;
  PartitionNr := PartNr;        // not used anymore
  FS := hpfs;
  End;

Open_Drive(Drive,DevHandle);            // Get drivehandle
lock_drive(devhandle);
Write_Disk(DevHandle,HPbuf,BBlockLen);
unlock_drive(devhandle);
Close_Drive(DevHandle);

Fix_Preldr0(DriveT);
Writeln('Installation of osFree bootsectors on partition ',drive1,' ended successfully.');
Write('Press <Enter> to continue... ');
Readln;
End;

// JFS version check is needed, because beta versions af JFS TRAPS
// when we write in DASD mode.
Function JFS_version_check:Boolean;
Var
  FH: Integer;
  xx,ii,l1,p1,p2: Longint;
  S1,jver: String[255];
  ifsPbuf:      Pchar;

Begin
result := False;
FH := FileOpen( GetBootDrive+':\os2\JFS.IFS', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  ifsPbuf := ifsbuf;
  XX:=FileRead( FH, ifsbuf, 1000000 );
  FileClose( FH );
  ii:= 0;
  Repeat
    S1 := StrPas(ifsPbuf+ii);
    p1 := pos('@#SSI:',S1);
    p2 := pos('@#IBM:',S1);
    If p1 > 0 Then
      Begin
      jver := copy(s1,p1+6,pos('#',copy(s1,p1+6,255))-1);
      If pos('pasha',s1) > 0 Then
        Begin
        Writeln('Version is from Pasha, it is OK ! ');
        Result := True;
        End
        Else
          Begin
          ClrScr;
          Writeln;
          Writeln('  ***********************   W A R N I N G  ****************************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********   You have a BETA version of JFS installed   ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********          Do NOT try to install osFree        ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********         on any of your JFS partitions,       ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********       as this will TRAP your system !!!!     ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********   Please ask Mensys for an updated version   ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********       of eCS JFS  or use IBM version !       ************');
          Writeln('  ***********                                              ************');
          Writeln('  ***********                                              ************');
          Writeln('  *********************************************************************');
          Writeln(#8#88);
          writeln;
          End;
      End;
    If p2 > 0 Then
      Begin
      jver := copy(s1,p2+6,pos('#',copy(s1,p2+6,255))-1);
      Writeln(s1);
      Writeln('IBM JFS version is: ',jver);
      Result := True;
      End;
    l1 := length(s1);
    If l1 = 0 Then l1 := 1;
    ii:=II+l1;
  Until (ii >=xx) or (p2 > 0) or (p1 > 0)
  End
  Else
   Writeln('Can''t find JFS.IFS');

END;


// JFS install
// BootBlock including bootsector max 16 sectors for now, but can be extended

Procedure Install_JFS;
Const
    osFree: Str8  = '[osFree]' ;

Type
  Free0 = Record
        jmp:            Array[0..2] Of Byte;
        Oem_Id:         Str8;
        Bpb:            Array[0..50] Of Char;
        Boot_Code:      Array[0..438] Of Byte;
        FSD_LoadSeg:    Word;
        FSD_Entry:      Word;
        FSD_Len:        Byte;
        FSD_Addr:       Cardinal;
        Boot_End:       Word;
        End;

Var
  FreeSect0 :           Free0;
  Fs0,bb,F32bb:               Pchar;
  FH:                   Integer;
  mini_hdr:             hdr;
  Count, Count1:        Word;

Begin
//JFS_version_check;
Open_Drive(Drive,DevHandle);                    // Get drivehandle
lock_drive(devhandle);
Read_Disk(devhandle,HPbuf,BblockLen);           // Read boot block
unlock_drive(devhandle);
Close_Drive(DevHandle);
F32bb := @HPbuf;
FH := FileOpen( drive1+'\boot\sectors\bootsect_1.bin', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  FileRead( FH, FreeSect0, Sector0Len );
  FileClose( FH );
  With FreeSect0 Do
    Begin
    Oem_Id := osFree;                           // Insert new OEM ID, 'IBM is required as first 3 letters or mounting of IFS will fail
    StrMove( bpb, @F32Bb[11], 51);              // Copy bpb from HD, 51 bytes
    FSD_LoadSeg:=$0800;                         // FSD load address
    FSD_Entry:=0;                               // FSD entry point
    FSD_Len:=(BblockLen - 512) div 512;         // Length of boot block in sectors
    FSD_Addr:=1;                                // Startsector for boot block
    End;
  FillChar(HPbuf,SizeOf(HPbuf),255);            // Wipe HPbug again
  StrMove (F32Bb, @FreeSect0, 512);
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening BootSect_1.bin');
  Halt(1);
  End;

// Prepare BootBlock
FH := FileOpen( drive1+'\boot\loader\preldr_mini.mdl', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count := FileRead( FH, F32bb[512], BblockLen - 512 );
  FileClose( FH );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening preldr_mini.mdl');
  Halt(1);
  End;
FH := FileOpen( drive1+'\boot\loader\fsd\jfs.mdl', fmOpenRead OR fmShareDenyNone);
If FH > 0 Then
  Begin
  Count1 := FileRead( FH, F32bb[512+count], BblockLen - ( count + 512 ) );
  FileClose( FH );
  End
 Else
  Begin
  Writeln('OS/2 Error ',-FH,' opening jfs.mdl');
  Halt(1);
  End;

With HPbuf Do
  Begin
  FS_Len      := Count1;
  Preldr_Len  := Count;
  PartitionNr := 0;             // WAS PartNr;
  FS := jfs;
  End;

Open_Drive(Drive,DevHandle);            // Get drivehandle
lock_drive(devhandle);
Write_Disk(DevHandle,HPbuf,BBlockLen);
unlock_drive(devhandle);
Close_Drive(DevHandle);

Fix_Preldr0(DriveT);
Writeln('FreeLDR installed successfully on JFS volumen ',drive1,' ');
Write('Press <Enter> to continue... ');
Readln;
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
  ulDataLen2   : ULong; // Data return buffer length
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
ulDataLen2 := 3;
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



{**** Boot Block operations ******}


Procedure Backup_BootBlock;
Begin
FillChar(Bbuf,SizeOf(Bbuf),0);
Open_Drive(Drive,DevHandle);
Read_Disk(DevHandle,Bbuf,BblockLen);
Writeln('Press Enter to continue...');
Readln;
Close_Drive(DevHandle);
End;

Procedure Restore_BootBlock;
Var
  Filename:     String;
  FH:   Integer;
  key:  Char;
  BufSize:      Integer;

Begin
Writeln(#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8,'Enter name of the bootblockfile to restore ');
Write('(Default is Drive-',drive1[1],'.000: ');
Readln(filename);
If filename = '' Then Filename := 'Drive-'+drive1[1]+'.000';

FH := FileOpen( filename, fmOpenRead OR fmShareDenyNone);
Writeln('Restoring ',filename, 'to Bootblock (sector 1)');
FillChar(Bbuf,SizeOf(Bbuf),0);

If FH > 0 Then
  Begin
  FileRead( FH, BBuf, BBlockLen );
  FileClose( FH );
  BufSize := BBlockLen;
  If DriveT = dtHDFAT Then
    Begin
    Writeln('You are trying to restore a BootBlock to a FAT partition. (8k)');
    Writeln('Normally you should only restore the BootSector (512 bytes)');
    Writeln('Press S to only restore bootSector or B to restore 8k bootBlock');
    key := upcase(readkey);
    If key <> 'B' Then BufSize := 512;
    End;
  Open_Drive(Drive,DevHandle);
  lock_drive(devhandle);
  If DriveT = dtHDFAT32 Then
    Begin
    Fat32FSctrl(Devhandle);
    fat32WriteSector( DevHandle, 0, BBlockLen DIV 512, BBuf );
    End
   Else Write_Disk(devhandle,BBuf,BufSize);
  Unlock_drive(devhandle);
  Close_Drive(DevHandle);
  End;
Writeln('Press Enter to continue...');
Readln;
End;


Procedure Header;
Begin
ClrScr;
Writeln('                     ',version);
Writeln('                     ---------------------------------------');
Writeln;
Writeln(' ***************************   W A R N I N G  *********************************');
writeln;
Writeln(' This is experimental software - that can *COMPLETELY* destroy your harddisk(s)');
Writeln;
Writeln(' Don''t use this software, unless you have a full system backup of all your HDs');
Writeln;
writeln(' ******************************************************************************');

Writeln;
Writeln('                   Partition ',drive1,' is selected for install');
Writeln;
End;


Procedure DriveInfo;
Var
 Drives: DriveSet;
 Drive3: Char;
 Desc: String;
Begin
// This program outputs a list of drives and their type
GetValidDrives( Drives );
Header;
Writeln( 'This system has the following drives available for install: ' );
Writeln;
for Drive3 := 'A' to 'Z' do
  if Drive3 in Drives then IF GetDriveType(drive3) IN [dtFloppy,dthdfat,dthdfat32,dthdhpfs,dthdjfs] Then
    begin
      case GetDriveType( Drive3 ) of
        dtFloppy    : Desc := 'Floppy drive';
        dtHDFAT     : Desc := 'Hard disk (FAT format)';
        dtHDHPFS    : Desc := 'Hard disk (HPFS format)';
        dtHDJFS     : Desc := 'Hard disk (JFS format)';
        dtHDFAT32   : Desc := 'Hard disk (FAT32 format)';
        dtHDExt2    : Desc := 'Hard disk (EXT2 format)';
        dtHDNTFS    : Desc := 'Hard disk (NTFS format)';
        dtNovellNet : Desc := 'Novell Netware drive';
        dtCDRom     : Desc := 'CD-ROM drive';
        dtLAN       : Desc := 'IBM network drive';
        dtTVFS      : Desc := 'TVFS virtual drive';
        dtRAMFS     : Desc := 'RAM drive';
        dtNDFS32    : Desc := 'Netdrive virtual drive';
        dtInvalid   : Desc := 'Invalid or unknown drive type';
      end;
      Writeln( Drive3,': '+Desc );
    end;
Writeln;
Write('Select Driveletter (A..Z) for install ');
Drive1 := upcase(Readkey) + ':' ;
Drive  := StrPCopy(Drive,drive1);
DriveT := GetDriveType(drive1[1]);
end;




{ ***********************************************************************************************************
  ********************************************       MAIN      **********************************************
  *********************************************************************************************************** }

Begin
Drive := Drive2;
{
If ParamCount > 0 Then
  Drive1 := ParamStr(1)  // Driveletter expected as first parameter on cmd line.
 Else
  Drive1 := copy(ParamStr(0),1,2);  // Get driveletter where this app is started from.
Drive := StrPCopy(Drive,drive1);

DriveT := GetDriveType(drive1[1]);
}
Header;
Gotoxy(1,12);
Write('                   Are you sure, you want to continue ? (Y/N) ');
If NOT (readkey in ['y','Y']) Then Halt(9);
DriveInfo;              // Ask user which drive to install to.

// SetVideoMode(80,40);
Repeat
  Header;
  Writeln(' What do you want to do ? ');
  Writeln;
  Writeln(' 1:  Install new MBR for FreeLDR ');
  Writeln(' 2:  Install FreeLDR on a partition');
//  Writeln(' 3:  Backup or Restore a MBR / BootSector / BootBlock');
  Writeln(' 3:  Backup MBR sector. ');
  Writeln(' 4:  Backup a BootBlock.');
  Writeln(' 5:  Restore MBR sector from backup file.');
  Writeln(' 6:  Restore a BootBlock from backup file.');
  Writeln(' 9:  Change partition to install, backup or restore to.');
  Writeln(' 0:  Exit');
  Writeln('');
  Write('(1,2,3,4,5,6,9,0) ');
  ch := ReadKey;
  Write(#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8#8);
  Case ch of
    '1': Install_MBR;
    '2': Begin
         Case DriveT Of
           dtFloppy    : Install_Fat;
           dtHDFAT     : Install_Fat;
           dtHDFAT32   : Install_Fat32;
           dtHDHPFS    : Install_HPFS;
           dtHDJFS     : Install_JFS;
//           dtHDNTFS  : Install_;  No Writeble IFS exists for OS/2
//           dtHDExt2  : Install_;  Wonder if it will work on OS/2 ???
           Else Begin
                Writeln('We do not yet support support the filesystem on your ',drive1,' partition');
                Writeln('Press <Enter> to continue');
                Readln;
                End;
           End;
         End;

    '3': Backup_MBR_sector;
    '4': BackUp_BootBlock;
    '5': Restore_MBR_Sector;
    '6': Restore_Bootblock;
    '9': Begin
         DriveInfo;
         End;
    '0': ;
    Else Writeln('Typo !!!  Exiting...');
    End;
  Delay(100);
Until NOT( ch IN ['1'..'9']);


End.

