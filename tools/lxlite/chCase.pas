{&G3+}
{$ifndef fpc}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-}
{$M 262144}
{$else}
{$Align 1}
{$Optimization STACKFRAME}
{$MinStackSize 262144}
{$endif}
uses  os2base, miscUtil, SysLib, CmdLine, Collect,
      strOp, Crt, Dos, lxlite_Global;
const
 Recurse   : boolean = FALSE;
 Pause     : boolean = FALSE;
 Verbose   : boolean = FALSE;
 AssumeYes : boolean = FALSE;

 cmBreak   = 0;
 cmLower   = 1;
 cmUpper   = 2;
 cmMixed   = 3;
 cmAsIs    = 4;

var
 OldExit   : Procedure;
 fNames    : pStringCollection;
 allDone   : boolean;
 fCaseMode,
 dCaseMode : array[1..64] of Byte;
 SepString : string[16];

Procedure Stop(eCode : Byte);
begin
 case eCode of
  1,2,3:begin
         case eCode of
          2 : begin
               TextAttr := $0C;
               Writeln('Ã Invalid switch - see help below for details');
              end;
         end;
         TextAttr := $07;
         Writeln('Ã Usage: chCase [FileMask( FileMask)] {[/|-]Options}');
         Writeln('Ã /C{F|D}(L|U|M|A)');
         Writeln('³  [C]ase conversion rules for [F]iles or [D]irectories');
         Writeln('³  (/C for both) [L]ower/[U]pper/[M]ixed/[A]s-is case');
         Writeln('Ã /P{+|-} Enable (+) or disable (-) pause before each file');
         Writeln('Ã /R{+|-} [R]ecursive (+) file search through subdirectories');
         Writeln('Ã /S{"%"} Define separator character(s)');
         Writeln('Ã /V{+|-} Verbose (show additional information)');
         Writeln('Ã /Y{+|-} Assume (+) affirmative replies on all queries');
         Writeln('Ã /?,/H   Show this help screen');
         Writeln('Ã´Default: /CFL /CDA /P- /R- /S"." /V+ /Y-');
         TextAttr := $08;
         Writeln('À´Example: chCase /cm d:\*.*.txt /r');
        end;
 end;
 Halt(eCode);
end;

Function ParmHandler(var S : string) : Byte;
var
 I,J : Longint;
 pB  : pByte;

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
  'C' : begin
         I := 2; J := 0;
         if length(S) > 1
          then case UpCase(S[2]) of
                'F' : J := 1;
                'D' : J := 2;
               end;
         if J <> 0 then Inc(I);
         if J = 1
          then pB := @fCaseMode
          else pB := @dCaseMode;
         while (I <= 64) and (I <= length(S)) do
          begin
           case upCase(S[I]) of
            'L' : pB^ := cmLower;
            'U' : pB^ := cmUpper;
            'M' : pB^ := cmMixed;
            'A' : pB^ := cmAsIs;
            else break;
           end;
           Inc(I); Inc(pB);
          end;
         pB^ := cmBreak;
         if J = 0 then fCaseMode := dCaseMode;
         ParmHandler := pred(I);
        end;
  'P' : Pause := Enabled;
  'R' : Recurse := Enabled;
  'V' : Verbose := Enabled;
  'Y' : AssumeYes := Enabled;
  'S' : ParmHandler := 1 + ParseName(S, 2, SepString);
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
var
 ch  : char;
begin
 if AssumeYes then begin Ask := 1; exit; end;
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

{Returns: 0 - file is not locked for write}
{         1 - file is locked and cannot be unlocked}
{         2 - file has been unlocked}
Function CheckUseCount(const fName : string) : byte;
var
 F : File;
 I : Longint;
begin
 CheckUseCount := 0;
 I := FileMode; FileMode := open_access_ReadOnly or open_share_DenyReadWrite;
 Assign(F, fName); Reset(F, 1); Close(F); FileMode := I;
 if ioResult = 0 then exit;
 textAttr := $0E;
 Writeln(#13'Ã The module ' + Copy(fName, 1, 40) + ' is used by another process');
 CheckUseCount := 1;
 case Ask('[R]eplace, [S]kip or [A]bort?', 'RSA') of
  1 : ;
  2 : exit;
  3 : begin allDone := TRUE; exit; end;
 end;
 if not unlockModule(fName)
  then begin
        textAttr := $0C;
        Writeln('Ã Cannot replace module ' + fName);
        exit;
       end;
 CheckUseCount := 2;
end;

Procedure ProcessFile(fName : string; Attr : Word);
var
 _d    : DirStr;
 _n    : NameStr;
 tS,dfn: String;
 I,cmp : Longint;
 pB    : pByteArray;
begin
 textAttr := $0B;
 Write(#13); ClrEOL;
 Write('À ', Short(fName, 77), #13);
 _d := extractDir(fName);
 _n := extractName(fName);
 I := 1; cmp := 0; dfn := '';
 if Attr and Directory <> 0
  then pB := @dCaseMode
  else pB := @fCaseMode;
 While I <= length(_n) do
  begin
   tS := '';
   While (i <= length(_n)) and (First(_n[i], SepString) = 0) do
    begin tS := tS + _n[i]; Inc(i); end;
   case pB^[cmp] of
    cmLower : lowStr(tS);
    cmUpper : upStr(tS);
    cmMixed : begin lowStr(tS); tS[1] := upCase(tS[1]); end;
   end;
   if (pB^[cmp] <> cmBreak) and (pB^[succ(cmp)] <> cmBreak)
    then Inc(cmp);
   dfn := dfn + tS;
   if i <= length(_n) then begin dfn := dfn + _n[i]; Inc(i); end;
  end;
 if _n = dfn then Exit;
 if (Attr and Directory = 0) and (CheckUseCount(fName) = 1) then exit;
 ClrEOL;
 textAttr := $0B; Write('À ', _n);
 textAttr := $0A; Write(' -> ');
 textAttr := $0E; Write(dfn);
 if FileRename(_d + _n, _d + dfn)
  then if Verbose
        then begin
              textAttr := $0A; Write(' ok'#13);
              textAttr := $0B; Writeln('Ã');
             end
        else begin Write(#13); ClrEOL; end
  else begin
        textAttr := $0C; Write(' error'#13);
        textAttr := $0B; Writeln('Ã');
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
 FindFirst(fN, Archive or Hidden or SysFile or Directory, sr);
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
   if (length(_d) + length(sr.Name) <= 255) and (sr.Name[1] <> '.')
    then begin
          if Pause
           then case Ask('File ' + sr.Name + ': [P]rocess, [S]kip or [A]bort?', 'PSA') of
                 2 : sr.Name := '';
                 3 : begin allDone := TRUE; break; end;
                end;
          if (sr.Name <> '') then ProcessFile(_d + sr.Name, sr.Attr);
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
 Writeln('Ú[ chCase ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ[ Version '+Version+' ]¿');
 Writeln('Ã Copyright 1996 by FRIENDS software Ä No rights reserved Ù');
 TextAttr := $07;
 @OldExit := ExitProc; ExitProc := @MyExitProc;
 New(fNames, Create(8, 8));
 fCaseMode[1] := cmLower; dCaseMode[1] := cmAsIs;
 SepString := '.';
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
