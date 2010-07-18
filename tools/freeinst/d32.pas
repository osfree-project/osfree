function SysGetBootDrive: Char;
var
  regs : TRealRegs;
  res  : Word;

begin
  regs.realeax := $3305;
  SysRealIntr($21, regs);
  res := regs.realedx and $ff;
  result := chr(res + ord('A') - 1);
end;

function SysGetDriveType(Drive: Char): TDriveType;
begin
  if Drive <= 'B' then
    result := dtFloppy
  else
    result := dtHDFAT;
end;

function SysGetValidDrives: Longint;
var
  regs          : TRealRegs;
  res           : Longint;
  i, al         : Byte;

begin
  res := 0;
  regs.realeax := $3200;
  i := 0;
  repeat
    regs.realedx := i + 1;
    SysRealIntr($21, regs);
    al := regs.realeax and $ff;
    if al <> 0 then
      exit;
    res := res or (1 shl i);
    inc(i);
  until al = $ff;

  result := res;
end;
