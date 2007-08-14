/* $Id: syslevel.cmd,v 1.2 2004/08/16 04:51:16 prokushev Exp $ */

Call ShowSysFile Arg(1)
return

ShowSysFile: Procedure
  filename=arg(1)
  f=charin(filename, 1, chars(filename));
  Major=trunc(c2d(substr(f, 41, 1))/16);
  Minor=(c2d(substr(f, 41, 1))-Major*16)*10 + c2d(bitand(substr(f, 42, 1), d2c(15)));
  CurCSD=substr(f, 45, 7)
  OldCSD=substr(f, 53, 7)
  Name=substr(f, 61, 1)
  CompID=substr(f, 141, 9)
  RevByte=c2d(substr(f, 150, 1));
  Type=substr(f, 151, 1)
  Say Filename
  Say Major
  Say Minor
  Say RevByte
  Say CurCSD
  Say OldCSD
  Say CompID
  Say Type
return

/*
C:\IBMGSK\SYSLEVEL.GSK
                      OS/2 Global Security Runtime for SSL Protocol (56/128/168-
bit Encryption)
Version 3.30     Component ID 5639F8902
Type 0P
Current CSD level: WR08701
Prior   CSD level: WR08701

*/
/*

procedure ShowSysFiles;
var
  Cnt, i: integer;
  f : File;
  Buf: Array[0..$200] of byte;
  bytes: Longint;
  pCurCSD : pChar;
  pOldCSD : pChar;
  pName   : pChar;
  pCompID : pChar;
  RevByte : Byte;
  pType   : pChar;
  Major   : Byte;
  Minor   : Byte;

            if ( Buf[0] = $FF ) and ( Buf[1] = $FF ) and
               ( 'SYSLEVEL' = strpas(@Buf[2]) ) then
              begin
                Inc( Cnt );
                Writeln( SysF^[i] );

                gotoxy( 20, WhereY );
                Writeln( pName );
                Write( Format( 'Version %d.%2.2d', [Major, Minor] ) );
                if RevByte <> 0 then
                  Write( '.',RevByte );
                Writeln( '     Component ID ',pCompID );
                if pType^ <> #0 then
                  Writeln( 'Type ',pType );
                Writeln( 'Current CSD level: ',pCurCSD );
                Writeln( 'Prior   CSD level: ',pOldCSD );
                Writeln;
/*
*/

$Log: syslevel.cmd,v $
Revision 1.2  2004/08/16 04:51:16  prokushev
* Fixes from my local tree. Build tools.

Revision 1.1.1.1  2003/10/04 08:24:22  prokushev
Import


*/
/*

const
  dirs: Longint = 0;
  SysFiles: Longint = 0;
  MaxStack = 500;

type
  StringList = Array[1..MaxStack] of String;
  pStringList = ^StringList;

var
  SysF: pStringList;
  y: Integer;

procedure ScanPath( Path : String );
var
  i, stop: Longint;
  s: SearchRec;
  DirStack: pStringList;

begin
  inc( Dirs );
  if Dirs mod 6 = 0 then
    begin
      gotoxy( 5, y );
      Write(dirs:6,'  ');
      if length(Path) <= 60 then
        Write( Path )
      else
        Write( copy( Path, Length(Path)-60, 60 ), '...' );
      clreol;
    end;

  FindFirst( Path+'\syslevel.*', AnyFile,  S );
  while DosError = 0 do
    begin
      Inc( SysFiles );
      gotoxy( 5, y+1 );Write('Files: ', SysFiles:3);
      SysF^[SysFiles] := path+'\'+s.Name;
      FindNext( s );
    end;
  FindClose( s );

  Stop := 0;
  DirStack := nil;
  FindFirst( Path+'\*', Must_Have_Directory, S );
  while ( DosError = 0 ) and ( Stop < MaxStack ) do
    begin
      if ( s.Name <> '.' ) and ( s.Name <> '..' ) then
        begin
          if DirStack = nil then
            New( DirStack );

          Inc( Stop );
          DirStack^[Stop] := s.Name;
        end;
      FindNext( s );
    end;
  FindClose( s );

  for i := 1 to Stop do
    ScanPath( Path+'\'+DirStack^[i] );

  if DirStack <> nil then
    Dispose( DirStack );
end;


function GetLocalDrives: String;
  // Get a list of all local hard disk drive letters
var
  Drive: Char;

begin
  Result := '';
  For Drive := 'C' to 'Z' do
    If GetDriveType( Drive ) in [ dtHDFAT, dtHDHPFS ] then
      Result := Result + Drive;
end;

var
  s: String;
  i: integer;

begin
 if WhereY+8>Hi(WindMax) then
   ClrScr;
 Writeln('SysLevel v2.1     (C) 1996-2000 vpascal.com' );

  s := GetLocalDrives;

  Writeln;
  Writeln( 'Scanning drives "',s,'" for SYSLEVEL.*' );
  Writeln;

  try
    try
      New( SysF );
      y := WhereY;
      for i := 1 to Length(s) do
        ScanPath( s[i]+':' );

      ShowSysFiles;
    finally
      Dispose( SysF );
    end;
  except
    on e:Exception do
      begin
        Writeln;
        Writeln( 'Exception: ',E.Message );
        Writeln( 'SysLvl terminated.' );
      end;
  end;
end.

*/