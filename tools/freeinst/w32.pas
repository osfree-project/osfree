function SysGetBootDrive: Char;
begin
  Result := 'C';
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
  Result := dtInvalid;
  if GetVolumeInformation(Root, nil, 0, nil, MaxLength, FSFlags, FSName, sizeof(FSName)) then
    begin
      if StrComp(FSName, 'FAT32') = 0 then
        Result := dtHDFAT32
      else if StrLComp(FSName, 'FAT', 3) = 0 then
        Result := dtHDFAT
      else if StrComp(FSName, 'HPFS') = 0 then
        Result := dtHDHPFS
      else if StrComp(FSName, 'NTFS') = 0 then
        Result := dtHDNTFS
      else if StrLComp(FSName, 'CD', 2) = 0 then
        Result := dtCDROM
      else if StrComp(FSName, 'LAN') = 0 then
        Result := dtLan
      else if StrComp(FSName, 'NOVELL') = 0 then
        Result := dtNovellNet;
    end;

  if Result = dtInvalid then
    case GetDriveType(Root) of
      Drive_Fixed     : Result := dtHDFAT;
      Drive_Removable : Result := dtFloppy;
      Drive_CDRom     : Result := dtCDROM;
      Drive_Remote    : Result := dtLAN;
      0, 1            : Result := dtInvalid;
    else                Result := dtUnknown;
    end;
end;

function SysGetValidDrives: Longint;
begin
  Result := GetLogicalDrives;
end;
