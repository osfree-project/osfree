// |---------------------------------------------------------|
// |                                                         |
// |     Virtual Pascal Runtime Library.  Version 2.1.       |
// |     System Interface Layer for OS/2                     |
// |     ----------------------------------------------------|
// |     Copyright (C) 1995-2003 vpascal.com                 |
// |                                                         |
// |---------------------------------------------------------|

// PmWin and PmShApi function prototypes without Os2PmApi

// If CHECK_NO_PM is defined, the executable is bigger, the
// program temporarily uses more resources, but does not hang
// if PM-functions like SysClipCanPaste are called in an
// environment without Presentation Manager

// By default, we live with this overhead:
{$DEFINE CHECK_NO_PM}

{$I os2types.pas}

function SysGetBootDrive: Char;
Var
  Res : Word;
  rc  : Longint;

begin
  rc := DosQuerySysInfo( qsv_boot_drive, qsv_boot_drive, Res, Sizeof( Res ));
  If rc = no_Error then
    Result := Chr( Res + ord('A') - 1 )
  else
    Result := #0;
end;

function SysGetDriveType(Drive: Char): TDriveType;
Var
  BufLen    : LongInt;
  FSQb      : PFSQBuffer2;
  DrvName   : String[3];
  Ordinal   : Word;
  aName     : pChar;
  rc        : Word;
  //DiskSize  : Word;

begin
  Result := dtInvalid;
  BufLen := 100;
  GetMem( FSQb, BufLen );
  DrvName := Drive+':'#0;
  Ordinal := 0;
  rc := DosQueryFSAttach( @DrvName[1], Ordinal, fsail_QueryName, FSqb^, BufLen );
  if rc = 0 then
    With FsqB^ do
    begin
      aName := @Name + NameLen + 1;
      If strComp( aName, 'FAT' ) = 0 then
        If Drive <= 'B' then
          Result := dtFloppy
        else
          Result := dtHDFAT
      else if strComp( aName, 'HPFS' ) = 0 then
        If Drive <= 'B' then
          Result := dtFloppy
        else
          Result := dtHDHPFS
      else If StrComp( aName, 'NETWARE' ) = 0 then
        Result := dtNovellNet
      else If StrComp( aName, 'CDFS' ) = 0 then
        Result := dtCDRom
      else If StrComp( aName, 'TVFS' ) = 0 then
        Result := dtTVFS
      else If StrComp( aName, 'ext2' ) = 0 then
        Result := dtHDExt2
      else If StrComp( aName, 'LAN' ) = 0 then
        Result := dtLAN
      else If StrComp( aName, 'JFS' ) = 0 then
        Result := dthdJFS
      else If StrComp( aName, 'FAT32' ) = 0 then
        Result := dthdFAT32
      else If StrComp( aName, 'RAMFS' ) = 0 then
        Result := dtRAMFS
      else If StrComp( aName, 'NDFS32' ) = 0 then
        Result := dtNDFS32;
    end;

  FreeMem( FSQb, 100 );
end;

function SysGetValidDrives: Longint;
var
  CurDrive, res: Longint;
begin
  DosQueryCurrentDisk(CurDrive, res);
  if res <> 0
    then result := res
    else result := 0;
end;

function SysOsVersion: Longint;
var
  Version: array [0..1] of Longint;
begin
  DosQuerySysInfo(qsv_Version_Major, qsv_Version_Minor, Version, SizeOf(Version));
  Result := Version[0] + Version[1] shl 8;
end;

function SysGetVolumeLabel(Drive: Char): ShortString;
var
  rc          : Longint;
  DriveNumber : Word;
  Buf: TFSInfo; (* Record
    SerialNum : Word;
    VolLabel  : String[12];
  end; *)

begin
  DriveNumber := Ord( UpCase(Drive) ) - Ord( 'A' ) + 1;

  rc := DosQueryFSInfo( DriveNumber, fsil_VolSer, Buf, Sizeof( Buf ));
  If rc = No_Error then
    Result := Buf.VolumeLabel
  else
    Result := '';
end;

function SysSetVolumeLabel(Drive: Char; _Label: ShortString): Boolean;
var
  buf: TFSInfo;
  DriveNumber : Word;
begin
  DriveNumber := Ord( Drive ) - Ord( 'A' ) + 1;
  _Label[Length(_Label)+1] := #0;
  buf.VolumeLabel := _Label; // vs
  Result := 0 = DosSetFSInfo(DriveNumber, fsil_VolSer, buf {,_Label -- vs}, Length(_Label)+1);
end;

function SysDirGetCurrent(Drive: Longint; Path: PChar): Longint;
var
  P: PChar;
  X: Longint;
begin
  if Drive = 0 then
    DosQueryCurrentDisk(Drive, X);
  P := Path;
  P^ := Chr(Drive + (Ord('A') - 1));
  Inc(P);
  P^ := ':';
  Inc(P);
  P^ := '\';
  Inc(P);
  X := 260 - 4;         // 4: 'D:\', #0
  Result := DosQueryCurrentDir(Drive, P^, X);
end;

function SysFileSearch(Dest,Name,List: PChar): PChar;
var
  Info: TFileStatus3;
begin
  if (DosQueryPathInfo(Name, fil_Standard, @Info, SizeOf(Info)) = 0)
    and ((Info.attrFile and file_Directory) = 0) then
    SysFileExpand(Dest, Name)
  else
    if DosSearchPath(dsp_ImpliedCur+dsp_IgnoreNetErr,List,Name,Dest,260) <> 0 then
      Dest[0] := #0;
  SysFileAsOS( Dest );
  Result := Dest;
end;

function SysFileExpand(Dest,Name: PChar): PChar;
var
  I,J,L: Integer;
  C: Char;
  CurDir: array[0..259] of Char;

  procedure AdjustPath;
  begin
    if (Dest[J-2] = '\') and (Dest[J-1] = '.') then
      Dec(J,2)
    else
      if (j>3) and (Dest[J-3] = '\') and (Dest[J-2] = '.') and (Dest[J-1] = '.') then
      begin
        Dec(J, 3);
        if Dest[J-1] <> ':' then
        repeat
          Dec(J);
        until Dest[J] = '\';
      end;
  end;

begin // SysFileExpand
  L := StrLen(Name);
  if (L >= Length('\\')) and (Name[0] = '\') and (Name[1] = '\') then
    StrCopy(Dest, Name)         // '\\SERVER\DIR'
  else
    if (L >= Length('X:')) and (Name[1] = ':') then
      begin                     // Path is already in form 'X:\Path' or 'X:/Path'
        if (L >= Length('X:\')) and (Name[2] in ['\','/']) then
          StrCopy(Dest, Name)
        else
          begin                 // Path is in form 'X:Path'
            SysDirGetCurrent(Ord(UpCase(Name[0])) - (Ord('A') - 1), CurDir);
            if StrLen(CurDir) > Length('X:\') then
              StrCat(CurDir, '\');
            StrLCat(StrCopy(Dest, CurDir), @Name[2], 259);
          end;
      end
    else
      begin                         // Path is without drive letter
        SysDirGetCurrent(0, CurDir);// Get default drive & directory
        if StrLen(CurDir) > Length('X:\') then
          StrCat(CurDir, '\');
        if Name[0] in ['\','/'] then
          StrLCopy(Dest, @CurDir[0], 2) // 'X:' only
        else
          StrCopy(Dest, CurDir);
        StrLCat(Dest, Name, 259);
      end;

  J := 0;
  for I := 0 to StrLen(Dest)-1 do
    begin
      C := Dest[I];
      if C = '/' then
        begin
          C := '\';
          Dest[I] := C;
        end;
      if C = '\' then AdjustPath;
      Dest[J] := C;
      Inc(J);
    end;
  AdjustPath;
  if Dest[J-1] = ':' then
  begin
    Dest[J] := '\';
    Inc(J);
  end;
  Dest[J] := #0;
  Result := Dest;
end;

function SysFindFirst(Path: PChar; Attr: Longint; var F: TOSSearchRec; IsPChar: Boolean): Longint;
var
  Count: Longint;
  SR: TFileFindBuf3;
  Path2: array[0..259] of char;
begin
  Attr := Attr and not $8; // No VolumeID under OS/2
  Count := 1;
  F.Handle := hdir_Create;
  Result := DosFindFirst(Path, F.Handle, Attr, @SR, SizeOf(SR), Count, fil_Standard);

  // If a specific error occurs, and the call is to look for directories, and
  // the path is a UNC name, then retry
  if (Result = msg_Net_Dev_Type_Invalid) and
     (Hi(Attr) = $10) and
     (StrLen(Path) > Length('\\')) and
     (StrLComp(Path, '\\', Length('\\')) = 0) then
    begin
      DosFindClose(F.Handle);
      StrCat(StrCopy(Path2,Path), '\*.*');
      Result := DosFindFirst(Path2, F.Handle, Attr, @SR, SizeOf(SR), Count, fil_Standard);
      if (Result = 0) and (Count <> 0) then
        Result := 0;
    end;

  if Result = 0 then
    with SR do
    begin
      F.Attr := attrFile;
      TDateTimeRec(F.Time).FTime := timeLastWrite;
      TDateTimeRec(F.Time).FDate := dateLastWrite;
      F.Size := FileSize;
      if IsPChar then
        StrPCopy(PChar(@F.Name), Name)
      else
        F.Name := Name;
    end
  else
    F.Handle := hdir_Create;
end;

function SysFindNext(var F: TOSSearchRec; IsPChar: Boolean): Longint;
var
  Count: Longint;
  SR: TFileFindBuf3;
begin
  Count := 1;
  Result := DosFindNext(F.Handle, @SR, SizeOf(SR), Count);
  if Result = 0 then
    with SR do
    begin
      F.Attr := attrFile;
      TDateTimeRec(F.Time).FTime := timeLastWrite;
      TDateTimeRec(F.Time).FDate := dateLastWrite;
      F.Size := FileSize;
      if IsPChar then
        StrPCopy(PChar(@F.Name), Name)
      else
        F.Name := Name;
    end;
end;

function SysFindClose(var F: TOSSearchRec): Longint;
begin
  if F.Handle = hdir_Create then
    Result := 0
  else
    Result := DosFindClose(F.Handle);
end;

function SysFileAsOS(FileName: PChar): Boolean;
var
  SRec: TOSSearchRec;
  P: PChar;
begin
  Result := False;
  if SysFindFirst(FileName, $37, SRec, False) = 0 then
    begin
      if SRec.Name[1] <> #0 then
        begin
          // Replace '/' with '\'
          repeat
            P:= StrRScan(FileName, '/');
            if P = nil then Break;
            P[0] := '\';
          until False;

          // Replace filename part with data returned by OS
          P := StrRScan(FileName, '\');
          if P = nil then
            P := FileName
          else
            inc(P); // Point to first character of file name
          strPcopy(P, SRec.Name);
        end;
      SysFindClose(SRec );
      Result := True;
    end;
end;
