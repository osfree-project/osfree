{&G3+}
{$ifndef fpc}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-}
{$M 262144}
{$else}
{$MinStackSize 262144}
{$Align 1}
{$Optimization STACKFRAME}
{$endif}
uses  Dos, os2base, MiscUtil, SysLib, StrOp, Collect,
      CmdLine, lxlite_Global, Crtx;

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
 EA        : pEAcollection;

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
         Writeln('Ã Usage: noEA [FileMask( FileMask)] {[/|-]Options}');
         Writeln('Ã /R{+|-} [R]ecursive (+) file search through subdirectories');
         Writeln('Ã /P{+|-} Enable (+) or disable (-) pause before each file');
         Writeln('Ã /V{+|-} Verbose (show EAs instead of removing them)');
         Writeln('Ã /Y{+|-} assume (+) on all queries first available responce');
         Writeln('Ã /?,/H   Show this help screen');
         Writeln('Ã´Default: /P- /R- /V- /Y-');
         TextAttr := $08;
         Writeln('À´Example: noEA * /r /v');
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
  'Y' : AssumeYes := Enabled;
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
 i,j : integer;
const Action : array[1..4] of string[6] = ('rocess', 'kip', 'bort', 'eplace');
      ActAbbr: string[4] = 'PSAR';
begin
 if AssumeYes then begin Ask := 1; exit; end;
 TextAttr := $02;
 Write('À ');
 for i:=1 to Length(Q) do
   If Q[i] >= ' ' Then Write(Q[i]) Else TextAttr:=Ord(Q[i]);

 for i := 1 to Length(A) do
 begin
   j := First(A[i],ActAbbr);
   write('[');
   TextAttr := $06; write(A[i]);
   TextAttr := $02; write(']',Action[j]);
   if i = Length(A) then write('? ') else
     if i = pred(Length(A)) then write(' or ') else write(', ');
 end; 

 repeat
  ch := upCase(ReadKey);
  j := First(ch, A);
 until j <> 0;
 Ask := j;
 TextAttr := $06; write(A[j],Action[First(ch,ActAbbr)]);
 TextAttr := $02; Writeln(#13'Ã');
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
 Writeln(#13'Ã The module "' + Copy(fName, 1, 40) + '" is used by another process');
 CheckUseCount := 1;
 case Ask('', 'RSA') of
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

Procedure ShowEAs;
var
 I : Longint;
 S : String;
begin
 textAttr := $0E; Write(' EA list:');
 textAttr := $0B; Write(#13'Ã');
 For I := 0 to pred(EA^.Count) do
  with pFea2(EA^.At(I))^ do
   begin
    Move(szName, S[1], cbName);
    S[0] := char(cbName); if length(S) > 60 then S[0] := #60;
    textAttr := $0B; Write(#13#10'Ã ');
    textAttr := $02; Write(S, ' (');
    textAttr := $0F; Write(cbValue, ' bytes');
    textAttr := $02; Write(')');
   end
end;

Procedure ProcessFile(fName : string; Attr : Word);
var
 _d : DirStr;
 _n : NameStr;
 _e : ExtStr;
 I  : Longint;
 P  : pFea2;

Procedure TrackProcess;
begin
 textAttr := $0B; ClrEOL; Write('À Processing ', Copy(_n, 1, 32), ' ...');
end;

begin
 fSplit(fName, _d, _n, _e);
 _n := _n + _e;
 TrackProcess;
 New(EA, Fetch(fName));
 if EA <> nil
  then begin
        if (EA^.Count = 0)
         then begin textAttr := $03; Write(' no EAs'); end
         else if Verbose
               then ShowEAs
               else begin
                     For I := 0 to pred(EA^.Count) do
                      with pFea2(EA^.At(I))^ do
                       begin
                        GetMem(P, sizeOf(Fea2) + cbName);
                        Move(oNextEntryOffset, P^, sizeOf(Fea2) + cbName);
                        P^.cbValue := 0;
                        EA^.AtFree(I);
                        EA^.AtInsert(I, P);
                       end;
                     if Attr and Directory = 0
                      then case CheckUseCount(fName) of
                            1 : Exit;
                            2 : TrackProcess;
                           end;
                     if EA^.Attach(fName)
                      then begin textAttr := $0A; Write(' ok'); end
                      else begin textAttr := $0C; Write(' sharing violation'); end;
                    end;
        textAttr := $0B;
        if (Verbose and (EA^.Count > 0)) then Writeln(#13'Ã') else Write(#13);
        Dispose(EA, Destroy);
       end
  else begin
        textAttr := $0C; Write(' error');
        textAttr := $0B; Writeln(#13'Ã');
       end;
end;

Procedure ProcessFiles(const fN : string; Level : Longint);
var
 sr : SearchRec;
 nf : Longint;
 _d : DirStr;
 _n : NameStr;
const FileDir : array[false..true] of string[9] = ('File','Directory');
begin
 _d := extractDir(fN);
 _n := extractName(fN);
 FindFirst(fN, Archive or Hidden or SysFile or Directory, sr);
 nf := 0;
 if (Dos.DosError <> 0) and (Level = 0) and (not Recurse)
  then begin
        textAttr := $0C;
        Writeln('Ã Cannot find such files: ', fN);
       end
  else
 While (Dos.DosError = 0) and (not allDone) do
  begin
   Inc(nf);
   if (length(_d) + length(sr.Name) <= 255) and (sr.Name <> '.') and (sr.Name <> '..')
    then begin
          if Pause
           then case Ask(FileDir[sr.Attr and Directory<>0] + ' "'#7 + sr.Name + #2'" : ', 'PSA') of
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
 While (Dos.DosError = 0) and (not allDone) do
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
 Writeln('Ú[ noEA ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂ[ Version '+Version+' ]¿');
 Writeln('Ã Copyright 1996 by FRIENDS software    Ã    No rights    ´');
 Writeln('Ã Copyright 2001 by Max Alekseyev       À     reserved    Ù');
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
