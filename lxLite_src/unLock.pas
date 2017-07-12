{&AlignCode-,AlignData-,AlignRec-,G3+,Speed-,Frame-}
{$M 262144}
uses os2base, miscUtil, SysLib, CmdLine, Collect,
     strOp, Strings, Crt, Dos, lxlite_Global;
const
 Recurse : boolean = FALSE;
 Pause   : boolean = FALSE;
 Verbose : boolean = FALSE;
var
 OldExit : Procedure;
 fNames  : pStringCollection;
 allDone : boolean;

Procedure Stop(eCode : Byte);
begin
 case eCode of
  1,2 : begin
         if eCode = 2
          then begin
                TextAttr := $0C;
                Writeln('Ã Invalid switch - see help below for details');
               end;
         TextAttr := $07;
         Writeln('Ã Usage: unLock [FileMask( FileMask)] {[?|-]Options}');
         Writeln('Ã /P{+|-} Enable (+) or disable (-) pause before each file');
         Writeln('Ã /R{+|-} [R]ecursive (+) file search through subdirectories');
         Writeln('Ã /V{+|-} Verbose (show additional information)');
         Writeln('Ã /?,/H   Show this help screen');
         Writeln('Ã´Default: /P- /R- /V-');
         TextAttr := $08;
         Writeln('À´Example: unLock d:\*.exe d:\*.dll /r');
        end;
 end;
 Halt(eCode);
end;

Function ParmHandler(var S : string) : Byte;
var
 I : Longint;

Function Enabled : boolean;
begin
 Enabled := TRUE;
 if length(S) = 1
  then exit
  else
 if (S[2] in ['+','-'])
  then ParmHandler := 2
  else
 if (S[2] in [' ','/'])
  then exit
  else Stop(2);
 if S[2] = '-' then Enabled := FALSE;
end;

begin
 ParmHandler := 1;
 case upCase(S[1]) of
  '?',
  'H' : Stop(1);
  'P' : Pause := Enabled;
  'R' : Recurse := Enabled;
  'V' : Verbose := Enabled;
  else Stop(2);
 end;
end;

Function NameHandler(var S : string) : Byte;
var
 fN : string;
begin
 NameHandler := ParseName(S, 1, fN);
 if fN <> '' then fNames^.AtInsert(fNames^.Count, NewStr(fN));
end;

Procedure MyExitProc;
begin
 Write(#13);
 TextAttr := $07; ClrEOL;
 OldExit;
end;

Function Ask(const Q,A : string) : byte;
var ch  : char;
begin
 TextAttr := $02;
 Write('À ', Q, ' ');
 repeat
  ch := upCase(ReadKey);
  if First(ch, A) <> 0
   then begin
         Ask := First(ch, A);
         break;
        end;
 until FALSE;
 Writeln(Ch,#13'Ã');
end;

Procedure ProcessFile(fName : string);
var
 F  : File;
 _n : string;
 sz : array[0..255] of Char absolute _n;

Procedure NotLocked;
begin
 if Verbose
  then begin Write(' not locked'); textAttr := $0B; Writeln(#13'Ã'); end
  else begin Write(#13); ClrEOL; end;
end;

begin
 if length(fName) >= 255 then exit;
 _n := extractName(fName);
 textAttr := $0B;
 Write(#13); ClrEOL;
 Write('À Processing file ', Copy(_n, 1, 28));
 FileMode := open_share_DenyReadWrite or open_access_ReadOnly;
 Assign(F, fName); Reset(F, 1);
 if ioResult = 0
  then begin
        Close(F); NotLocked;
        Exit;
       end;
 case DosReplaceModule(strPCopy(sz, fName), nil, nil) of
  0 : begin
       textAttr := $0A; Write(' unlocked');
       textAttr := $0B; Writeln(#13'Ã');
      end;
  2 : NotLocked;
 else begin
       textAttr := $0C; Write(' sharing violation');
       textAttr := $0B; Writeln(#13'Ã');
      end
 end;
end;

Procedure ProcessFiles(const fN : string; Level : Longint);
var
 sr : SearchRec;
 nf : Longint;
 _d : DirStr;
 _n : NameStr;
begin
 _d := extractDir(fN);
 _n := extractName(fN);
 FindFirst(fN, Archive or Hidden or SysFile, sr);
 nf := 0;
 if (DosError <> 0) and (Level = 0) and (not Recurse)
  then begin
        textAttr := $0C;
        Writeln('Ã Cannot find such files: ', fN);
       end
  else
 While (DosError = 0) and (not allDone) do
  begin
   Inc(nf);
   if (length(_d) + length(sr.Name) <= 255)
    then begin
          if Pause
           then case Ask('File ' + sr.Name + ': [P]rocess, [S]kip or [A]bort?', 'PSA') of
                 2 : sr.Name := '';
                 3 : begin allDone := TRUE; break; end;
                end;
          if (sr.Name <> '') then ProcessFile(_d + sr.Name);
         end;
   FindNext(sr);
  end;
 FindClose(sr);
 if allDone or not Recurse then Exit;
 if nf = 0
  then begin
        textAttr := $0B; Write('À ', Short(_d, 77));
        ClrEOL; Write(#13);
       end;
 FindFirst(_d + '*', Archive or Hidden or SysFile or Directory, sr);
 While (dosError = 0) and (not allDone) do
  begin
   if (sr.Attr and Directory <> 0) and (sr.Name[1] <> '.') and
      (length(_d) + length(sr.Name) + length(_n) + 1 <= 255)
    then ProcessFiles(_d + sr.Name + '\' + _n, succ(Level));
   FindNext(sr);
  end;
 FindClose(sr);
end;

var
 I : Longint;

begin
 TextAttr := $0F;
 Writeln('Ú[ unLock ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ[ Version '+Version+' ]¿');
 Writeln('Ã Copyright 1996 by FRIENDS software Ä No rights reserved Ù');
 TextAttr := $07;
 @OldExit := ExitProc; ExitProc := @MyExitProc;
 New(fNames, Create(8, 8));
 ParseCommandLine(#0, ParmHandler, NameHandler);
 if (fNames^.Count = 0) then Stop(1);

 For I := 0 to pred(fNames^.Count) do
  begin
   ProcessFiles(pString(fNames^.At(I))^, 0);
   if allDone then break;
  end;

 TextAttr := $01; ClrEOL;
 Writeln('À´Done');
end.
