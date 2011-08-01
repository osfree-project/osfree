function SysGetBootDrive: Char;
begin
  SysGetBootDrive := 'C';
end;

function SysGetDriveType(Drive: Char): TDriveType;
const
  Root: Array[0..4] of char = 'C:\'#0;
var
  FSName: Array[0..255] of char;
  MaxLength: LongInt;
  FSFlags: LongInt;
begin
  Root[0] := Drive;
  SysGetDriveType := dtInvalid;
  if GetVolumeInformation(Root, nil, 0, nil, LongWord(MaxLength), LongWord(FSFlags), FSName, sizeof(FSName)) then
    begin
      if StrComp(FSName, 'FAT32') = 0 then
        SysGetDriveType := dtHDFAT32
      else if StrLComp(FSName, 'FAT', 3) = 0 then
        SysGetDriveType := dtHDFAT
      else if StrComp(FSName, 'HPFS') = 0 then
        SysGetDriveType := dtHDHPFS
      else if StrComp(FSName, 'NTFS') = 0 then
        SysGetDriveType := dtHDNTFS
      else if StrLComp(FSName, 'CD', 2) = 0 then
        SysGetDriveType := dtCDROM
      else if StrComp(FSName, 'LAN') = 0 then
        SysGetDriveType := dtLan
      else if StrComp(FSName, 'NOVELL') = 0 then
        SysGetDriveType := dtNovellNet;
    end;

  if SysGetDriveType = dtInvalid then
    case GetDriveType(Root) of
      Drive_Fixed     : SysGetDriveType := dtHDFAT;
      Drive_Removable : SysGetDriveType := dtFloppy;
      Drive_CDRom     : SysGetDriveType := dtCDROM;
      Drive_Remote    : SysGetDriveType := dtLAN;
      0, 1            : SysGetDriveType := dtInvalid;
    else                SysGetDriveType := dtUnknown;
    end;
end;

function SysGetValidDrives: Longint;
begin
  SysGetValidDrives := GetLogicalDrives;
end;
