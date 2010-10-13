unit Impl_LNX;

interface
type
  //Hfile  = Word;
  Hfile  = LongWord;
  ULong  = LongWord;
  UShort = Word;

procedure Open_Disk(Drive: PChar; var DevHandle: Hfile);
procedure Read_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Write_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Close_Disk(DevHandle: Hfile);
procedure Lock_Disk(DevHandle: Hfile);
procedure Unlock_Disk(DevHandle: Hfile);

procedure Read_MBR_Sector(DriveNum: Char; var MBRBuffer);
procedure Write_MBR_Sector(DriveNum: Char; var MBRBuffer);
procedure Backup_MBR_Sector;
procedure Restore_MBR_Sector;

implementation

procedure Open_Disk(Drive: PChar; var DevHandle: Hfile);
begin
    Writeln('Open_Disk() is not implemented.');
end;

procedure Read_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
begin
    Writeln(' Read_Disk() is not implemented.');
end;

procedure Write_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
begin
    Writeln(' Write_Disk() is not implemented.');
end;

procedure Close_Disk(DevHandle: Hfile);
begin
    Writeln(' Close_Disk() is not implemented.');
end;

procedure Lock_Disk(DevHandle: Hfile);
begin
    Writeln(' Lock_Disk() is not implemented.');
end;

procedure Unlock_Disk(DevHandle: Hfile);
begin
    Writeln(' () is not implemented.');
end;


procedure Read_MBR_Sector(DriveNum: Char; var MBRBuffer);
begin
    Writeln(' Unlock_Disk() is not implemented.');
end;

procedure Write_MBR_Sector(DriveNum: Char; var MBRBuffer);
begin
    Writeln(' Write_MBR_Sector() is not implemented.');
end;

procedure Backup_MBR_Sector;
begin
    Writeln(' Backup_MBR_Sector() is not implemented.');
end;

procedure Restore_MBR_Sector;
begin
    Writeln(' Restore_MBR_Sector() is not implemented.');
end;

end.
