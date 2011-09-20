function SysGetBootDrive: Char;
var
  regs : TRealRegs;
  res  : Word;

begin
  regs.realeax := $3305;
  SysRealIntr($21, regs);
  res := regs.realedx and $ff;
  SysGetBootDrive := chr(res + ord('A') - 1);
end;

function SysGetDriveType(Drive: Char): TDriveType;
var
  buf    : array[0..$1ff] of char;
  drv    : array[0..1] of char;
  rc     : integer;
begin
  if Drive <= 'B' then
    SysGetDriveType := dtFloppy;

  drv[0] := Drive; drv[1] := #0;

  rc := AbsRead(AnsiString(@drv), 0, 512, LongInt(@buf));

  if rc = 0 then
    SysGetDriveType := dtInvalid;

  if (buf[$3] = 'N') and (buf[$4] = 'T') and (buf[$5] = 'F') and (buf[$6] = 'S') then
    SysGetDriveType := dtHDNTFS
  else if (buf[$36] = 'H') and (buf[$37] = 'P') and (buf[$38] = 'F') and (buf[$39] = 'S') then
    SysGetDriveType := dtHDHPFS
  else if (buf[$36] = 'J') and (buf[$37] = 'F') and (buf[$38] = 'S') then
    SysGetDriveType := dtHDJFS
  else if (buf[$52] = 'F') and (buf[$53] = 'A') and (buf[$54] = 'T') and
     (buf[$55] = '3') and (buf[$56] = '2') then
    SysGetDriveType := dtHDFAT32
  else if (buf[$36] = 'F') and (buf[$37] = 'A') and (buf[$38] = 'T') then
    SysGetDriveType := dtHDFAT
  else
    SysGetDriveType := dtUnknown
end;

function tb_segment : longint;
begin
  tb_segment := go32_info_block.linear_address_of_transfer_buffer shr 4;
end;

function SysGetValidDrives: Longint;
var
  buf           : array[0..41]  of char;
  regs          : TRealRegs;
  res           : Longint;
  i             : Byte;
begin
  res := 0;
  
  regs.realds  := tb_segment;
  regs.reales  := tb_segment;

  // drive specification
  buf[0] := '@'; // a letter before 'A'
  buf[1] := ':';
  for i := 0 to 25 do
    begin
      regs.realesi := 0; // drive offet
      regs.realedi := 2; // FCB offset
      inc(buf[0]);
      writeln('0');
      SysCopyToDos(LongInt(@buf), sizeof(buf));
      writeln('1');
      regs.realeax := $2906;
      writeln('2');
      SysRealIntr($21, regs);
      writeln('3'); readln;

      if (regs.realeax and $ff) = $ff then
        continue;

      res := res or (1 shl i);
    end;

  SysGetValidDrives := res;
end;
