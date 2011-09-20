{**************************************
 *  System-dependent implementation   *
 *  of low-level functions for DPMI32 *
 **************************************}

unit Impl_D32;

interface

type
  Hfile  = LongInt;
  ULong  = LongWord;
  UShort = Word;

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

function AbsRead(drive: AnsiString; lba, len, addr: LongInt) : LongInt;

implementation

{ FreePascal only :( As VirtualPascal does not support DPMI32 }

uses
  Common, Strings, SysUtils, Crt, Dos;

const
  BIOSDISK_READ               : LongWord    = $0;
  BIOSDISK_WRITE              : LongWord    = $1;

  carryflag = 1;

var
  filepos: LongWord; // global seek position

procedure getinoutres(def : word);
var
  regs : TRealRegs;
begin
  regs.realeax := $5900;
  regs.realebx := $0;
  sysrealintr($21,regs);
  InOutRes := lo(regs.realeax);
  case InOutRes of
   19 : InOutRes := 150;
   21 : InOutRes := 152;
   32 : InOutRes := 5;
  end;
  if InOutRes = 0 then
    InOutRes := Def;
end;

function tb_size : longint;
begin
  tb_size := go32_info_block.size_of_transfer_buffer;
end;

function tb_segment : longint;
begin
  tb_segment := go32_info_block.linear_address_of_transfer_buffer shr 4;
end;

function biosdisk(ahreg, drive,
                  coff, hoff, soff: LongInt;
                  var nsec: LongInt; var buf): LongInt;
var
  regs              : TRealRegs;
  bytes_transferred : LongWord;
  err               : LongWord;
begin
  regs.realeax := (nsec and $ff) or ((ahreg and $ff) shl 8);
  regs.realedx := (drive and $ff) or ((hoff and $ff) shl 8);
  regs.realecx := ((coff and $ff) shl 8) or (((coff shr 8) and 3) shl 6) or (soff and $3f);
  regs.reales  := tb_segment;
  regs.realebx := 0;

  if ahreg = BIOSDISK_WRITE + 2 then
    SysCopyToDOS(LongInt(@buf), lo(nsec << 9));

  SysRealIntr($13, regs);
  err := (regs.realeax shr 8) and $ff;

  if (err <> 0) or ((regs.realflags and CARRYFLAG) = CARRYFLAG) then
    begin
      bytes_transferred := 0;
      nsec := 0;

      GetInOutRes(lo(bytes_transferred));

      biosdisk := err;
      exit;
    end;
  bytes_transferred := nsec shl 9;

  if ahreg = BIOSDISK_READ + 2 then
    SysCopyFromDOS(LongInt(@buf), lo(bytes_transferred));

  biosdisk := err;
end;

function dosdisk_read(drive, sector, nsec, segment: LongInt;
                      var bytes_read: LongInt): LongInt;
var
  regs : TRealRegs;
begin
  regs.realeax := drive   and $ff;
  regs.realecx := nsec    and $ffff;
  regs.realedx := sector  and $ffff;
  regs.realds  := segment and $ffff;
  regs.realebx := 0;

  SysRealIntr($25, regs);

  bytes_read := nsec shl 9;

  if (regs.realflags and CARRYFLAG) = CARRYFLAG then
    begin
      dosdisk_read := 1;
      exit;
    end;

  dosdisk_read := 0;
end;


function dosdisk_write(drive, sector, nsec, segment: LongInt;
                       var bytes_written: LongInt): LongInt;
var
  regs : TRealRegs;
begin
  regs.realeax := drive   and $ff;
  regs.realecx := nsec    and $ffff;
  regs.realedx := sector  and $ffff;
  regs.realds  := segment and $ffff;
  regs.realebx := 0;

  SysRealIntr($26, regs);

  bytes_written := nsec shl 9;

  if (regs.realflags and CARRYFLAG) = CARRYFLAG then
    begin
      dosdisk_write := 1;
      exit;
    end;

  dosdisk_write := 0;
end;

function GetLastHardDisk: Byte;
begin
  GetLastHardDisk := Mem[$475];
end;

function AbsRead(drive: AnsiString; lba, len, addr: LongInt) : LongInt;
var
  size,
  readsize,
  bytes_read    : LongInt;
  err           : integer;
  drv           : LongInt;

begin
  drv := ord(upcase(PChar(drive)^)) - ord('A');
  readsize:=0;
  while len > 0 do
    begin
      if len > tb_size then
        size := tb_size
      else
        size := len;

      err := dosdisk_read(drv, lba, size shr 9, tb_segment, bytes_read);

      if err <> 0 then
        begin
          bytes_read := 0;

          GetInOutRes(lo(bytes_read));

          AbsRead := 0;
          exit;
        end;

       SysCopyFromDOS(addr + readsize, lo(bytes_read));

       inc(readsize,lo(bytes_read));
       dec(len,lo(bytes_read));
       { stop when not the specified size is read }
       if lo(bytes_read) < size then
         break;
    end;
  AbsRead := readsize;
end;

function AbsWrite(drive: AnsiString; lba, len, addr: LongInt) : LongInt;
var
  size,
  writesize       : LongInt;
  bytes_written   : LongInt;
  err             : integer;
  drv             : LongInt;

begin
  drv := ord(upcase(PChar(drive)^)) - ord('A');
  writesize := 0;
  while len > 0 do
    begin
      if len > tb_size then
        size := tb_size
      else
        size := len;

      SysCopyToDOS(addr + writesize, size);
      err := dosdisk_write(drv, lba, size shr 9, tb_segment, bytes_written);
      if err <> 0 then
        begin
          bytes_written := 0;

          GetInOutRes(lo(bytes_written));

          AbsWrite := writesize;
          exit;
        end;
      inc(writesize, lo(bytes_written));
      dec(len, lo(bytes_written));
      { stop when not the specified size is written }
      if lo(bytes_written) < size then
        break;
    end;
  AbsWrite := writesize;
end;

procedure Open_Disk(Drive: AnsiString; var DevHandle: Hfile);
begin
  DevHandle := ord(PChar(drive)^);
  filepos := 0;
end;

procedure Read_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
var
  rc            : LongInt;        // Return code
  s3            : String[3];
  FH            : integer;        // File handle for backup file
begin
  rc := AbsRead(chr(DevHandle),
                filepos shr 9,
                buf_len,
                LongInt(@buf));
  if rc = 0 then
    begin
      writeln('AbsRead error');
      halt(1);
    end;

  inc(filepos, rc); // increment seek position

  // Write backup file of data read
  i := 0;
  repeat
    str(i:3, s3);
    if pos(' ',s3) = 1 then s3[1] := '0';
    if pos(' ',s3) = 2 then s3[2] := '0';
    i := succ(i);
    if i > 999 then exit;
  until not FileExists ('Drive-' + drive1[1] + '.' + s3);
  writeln('Backup bootsector file created:  Drive-', drive1[1], '.', s3);
  FH := FileCreate('Drive-' + drive1[1] + '.' + s3);
  FileWrite(FH, buf, rc);
  FileClose(FH);
end;

procedure Write_Disk(devhandle: Hfile; VAR buf; buf_len: Ulong);
var
  rc            : LongInt;       // Return code

begin
  rc := AbsWrite(chr(DevHandle), // File handle
                 filepos shr 9,
                 buf_len,        // Size of string to be written
                 LongInt(@buf));       // Bytes actually written

  if rc = 0 then
    begin
      writeln('DosWrite error');
      halt(1);
    end;

  inc(filepos, rc);
  writeln(rc,' Bytes written to disk');
end;

procedure Close_Disk(DevHandle: Hfile);
begin
end;

procedure Lock_Disk(DevHandle: Hfile);
begin
end;

procedure Unlock_Disk(DevHandle: Hfile);
begin
end;

procedure MBR_Sector(DriveNum: AnsiString; var MBRBuffer; IOcmd: LongInt);
var
  FH            : Integer;
  s3            : String[3];
  drv           : Byte;
  rc, nsec      : LongInt;

begin
  drv := ord(PChar(DriveNum)^) - $31 + $80; // 1: means bios device $80

  nsec := 1;
  rc := biosdisk(2 + IOcmd, drv, 0, 0, 1, nsec, MBRBuffer);

  if rc <> 0 then
    begin
      writeln('biosdisk (Disk_I/O MBR sector) error: return code = ', rc);
      halt(1);
    end;

  i := 0;
  repeat
    str(i:3, s3);
    if pos(' ',s3) = 1 then s3[1] := '0';
    if pos(' ',s3) = 2 then s3[2] := '0';
    i := succ(i);
    if i > 999 then exit;
  until not FileExists ('MBR_sect.'+s3);
  writeln('Backup bootsector file = MBR_sect.',s3);
  FH := FileCreate( 'MBR_sect.'+s3);
  FileWrite(FH, Sector0, Sector0Len);
  FileClose(FH);
end;

procedure Read_MBR_Sector(DriveNum: AnsiString; var MBRBuffer);
begin
  MBR_Sector(DriveNum, MBRBuffer, BIOSDISK_READ)
end;

procedure Write_MBR_Sector(DriveNum: AnsiString; var MBRBuffer);
begin
  MBR_Sector(DriveNum, MBRBuffer, BIOSDISK_WRITE)
end;

// Backup MBR sector to a file
procedure Backup_MBR_sector;
var
  usNumDrives : Word;
  Drive       : Char;

begin
  usNumDrives := GetLastHardDisk;
  if usNumDrives = 0 then
    begin
      writeln('GetLastHardDisk error');
      halt(1);
    end;
  writeln('DOS reports ', usNumDrives, ' partitionable disk(s) available.');
  write('Input disknumber for MBR backup (1..', usNumDrives, '): ');
  readln(Drive);
  Read_MBR_Sector(Drive, sector0);
  writeln('Press Enter to continue...');
  readln;
end;

// Restore MBRsector from a file
procedure Restore_MBR_sector;
var
  usNumDrives : Word;
  Drive       : Char;
  Filename    : String;
  FH          : Integer;

begin
  usNumDrives := GetLastHardDisk;
  if usNumDrives = 0 then
    begin
      writeln('GetLastHardDisk error');
      halt(1);
    end;
  writeln('DOS reports ', usNumDrives, ' partitionable disk(s) available.');
  write('Input disknumber for MBR backup (1..', usNumDrives, '): ');
  readln(Drive);
  writeln('Enter name of the bootsectorfile to restore');
  write('(Default is MBR_sect.000): ');
  readln(filename);
  if filename = '' then filename := 'MBR_sect.000';
  FH := FileOpen(filename, fmOpenRead or fmShareDenyNone);
  if FH > 0 then
    begin
      writeln('Restoring ', filename, 'to bootsector');
      FileRead(FH, Sector0, Sector0Len);
      FileClose(FH);
      Write_MBR_Sector(Drive, sector0);
    end
  else
    writeln('Sorry, the file ', filename, ' returned error ', -FH);
  writeln('Press Enter to continue...');
  readln;
end;

{$IFDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
