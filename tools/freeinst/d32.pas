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
begin
  if Drive <= 'B' then
    SysGetDriveType := dtFloppy
  else
    SysGetDriveType := dtHDFAT;
end;

function SysGetValidDrives: Longint;
var
  regs          : TRealRegs;
  res           : Longint;
  i, al         : Byte;

begin
  res := 0;
  regs.realeax := $3200;
  for i := 0 to 25 do // a..z
    begin
      regs.realedx := i + 1;
      if i > 1 then
        begin
          SysRealIntr($21, regs);
          al := regs.realeax and $ff;
        end
      else
        al := 0; // floppies are always valid
      if al = 0 then
        res := (res or (1 shl i));
    end;
  SysGetValidDrives := res;
end;
