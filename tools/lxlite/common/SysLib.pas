{$A-,B-,D+,G-,I-,O-,P+,Q-,R-,S-,T-,V-,X+}
{&W-,G3+}
{$ifndef fpc}
{$E-,F-,L+,N-,Y+}{&AlignCode-,AlignData-,AlignRec-,Optimise+,OrgName-,Asm-,Cdecl-,Delphi+,Frame-,LocInfo+,SmartLink+,Speed-,Z-,ZD-,Use32+}
{$else}
{$asmmode intel}
{$mode delphi}
{$H-}
{$Align 1}
{$Optimization STACKFRAME}
{$ModeSwitch nestedprocvars}
{$endif}

Unit SysLib;

Interface uses Dos, miscUtil, Collect
{$ifdef win32}, windows{$endif}
{$IfDef OS2}, os2def{$IfnDef FPC}, os2base{$Else}, doscalls{$EndIf}{$EndIf};

{$IfDef OS2}
{$IfnDef FPC}
function DosReplaceModule(OldModName,NewModName,BackModName: PChar): ApiRet; cdecl; orgname;
{$EndIf}
{$else}
// replacement for use in Windows build
function DosEditName(MetaLevel: LongInt; Source,Edit: PChar; Target: PChar; cbTarget: LongInt): LongInt;
{$EndIf}

const
 fMaskDelim1 = ':'; {fileMask delimiter char}
 fMaskDelim2 = '/'; {fileMask delimiter char}

{$IfDef FPC}
const
 ENUMEA_LEVEL_NO_VALUE  = 1;
 ENUMEA_REFTYPE_PATH    = 1;

 FILE_READONLY  = $0001;
 FILE_HIDDEN    = $0002;
 FILE_SYSTEM    = $0004;

 DCPY_EXISTING  = 1;

 NULLHANDLE     = 0;

type
 HFILE  = longint;

type
 TID    = longint;
 APIRET = longint;
{$EndIf}

type
 pFileMatch = ^tFileMatch;
 tFileMatch = object(tObject)
  matchStrings : pZTstrCollection;
  constructor Create(const fMasks : string);
  procedure   AddMask(const fMasks : string);
  function    Matches(const fName : string) : boolean;
  destructor  Destroy; virtual;
 end;

{$ifDef OS2}
 pEAcollection = ^tEAcollection;
 tEAcollection = object(tCollection)
  constructor Fetch(const fName : string);
  function    Attach(const fName : string) : boolean;
  procedure   FreeItem(Item: Pointer); virtual;
 end;

{ Fast MUTEX semaphore type }
 tMutexSem = record
  Next  : Pointer;                { Next thread ID requesting ownership }
  Owner : TID;     { Current semaphore owner; bit 31 = semaphore in use }
  Count : Longint;                   { For recursive semaphore requests }
 end;
{$endIf}

 pCommandLineParser = ^tCommandLineParser;
 tCommandLineParser = object(tObject)
  procedure   Parse(var S : string);
  procedure   ParseCommandLine;
  function    GetWord(var ParmStr : string; StartChar : Word; var DestStr : string) : Word;
  function    GetOpt(const parmStr : string; StartChar : Word; const OptChars : string;
               OptFlags : array of Longint; var Option : Longint) : Word;
  function    ParmHandler(var ParmStr : string) : Word; virtual;
  function    NameHandler(var ParmStr : string) : Word; virtual;
  procedure   PreProcess(var ParmStr : string); virtual;
  procedure   PostProcess; virtual;
 end;

{Return TRUE if file exists; FALSE otherwise}
 Function fileExist(const fName : string) : Boolean;

{Try to rename file sName into dName. Returns TRUE if succesful}
 Function fileRename(const sName,dName : string) : Boolean;

{Try to erase file fName and returns TRUE if succesful}
 Function fileErase(const fName : string) : Boolean;

{Returns file length in bytes or -1 if no such file}
 Function fileLength(const fName : string) : Longint;

{Copy file srcName into dstName; return TRUE if o.k.}
 Function fileCopy(const sName,dName : string) : boolean;

{Create an entire directory tree structure}
 Function makeDirTree(const dirName : string) : boolean;

{Create an unique temporary filename by given filemask: replace '?' by}
{unique characters; if no path is given uses TEMP or TMP environment var}
 Function tempFileName(const fName : string) : string;

{Return a string containing executable`s source path including last '\' or '/'}
 Function SourcePath : string;

{An replacement for standard fSplit which handles correctly forvard slashes}
 procedure fSplit(const Path : PathStr; var Dir : DirStr; var Name : NameStr;
                  var Ext : ExtStr);

{$ifDef OS2 ---------}
{ Open an [F]ast [M]utex [S]emaphore }
 Function  fmsInit(var Sem : tMutexSem) : boolean;
{ Request a semaphore; wait until semaphore is available }
 Function  fmsRequest(var Sem : tMutexSem) : boolean;
{ Release semaphore; return TRUE if o.k.; FALSE if caller is not owner }
 Function  fmsRelease(var Sem : tMutexSem) : boolean;
{ Check if semaphore is owned; DO NOT RELY ON THIS! }
 Function  fmsCheck(var Sem : tMutexSem) : boolean;

{ Unlock a executable module if it is already in use }
 Function  unlockModule(const fName : string) : boolean;

{$endIf}
{ Return an string from resourse (from string table) }
 Function  GetResourceString(ID : Longint) : string;

Implementation uses StrOp, Streams, strings
{$ifdef fpc}
  , SysUtils
{$endif}
{$ifdef OS2}
{$ifdef virtualpascal}
  , vpsyslow
{$endif}
{$endif};

constructor tFileMatch.Create;
begin
 New(matchStrings, Create(4, 4));
 AddMask(fMasks);
end;

{$ifndef os2}
function MatchStr(Pat, Txt: string): Boolean;
var
  SubLen, ComPos, NextStar, SubPos: LongInt;
begin
  { First make sure that the pattern doesn't start with *, and always }
  { ends with *.  Change the text accordingly.                        }
  Pat := #0 + Pat + #0 + '*';
  Txt := #0 + Txt    + #0;

  Result := True;

  while (Pat <> '') and Result do
    begin
      // Look for the first *.  At least 1 character before this will be
      // a normal character, i.e. neither ? nor *
      NextStar := Pos('*', Pat);

      SubLen := NextStar - 1;

      // Ignore double-*
      while (NextStar < Length(Pat)) and (Pat[NextStar + 1] = '*') do
        Inc(NextStar);

      SubPos := 0;

      repeat
        Inc(SubPos);
        Result := True;
        ComPos := 0;
        while (ComPos < SubLen) and Result do
          begin
            if (Txt[SubPos + ComPos] <> Pat[ComPos + 1]) and
               (Pat[ComPos + 1] <> '?') then
              Result := False;

            Inc(ComPos);
          end;
      until (SubPos + SubLen > Length(Txt)) or Result;

      // When a match is found, cut a piece off the text and continue.
      if Result then
        begin
          Delete(Txt, 1, SubPos + SubLen - 1);
          Delete(Pat, 1, NextStar);
        end;
    end;
end;

function DosEditName(MetaLevel: LongInt; Source,Edit: PChar; Target: PChar; cbTarget: LongInt): LongInt;
var  rcLen, ii : longint;
     pSrc, pEd, pRes, pDelimit : PChar;
begin
    if MetaLevel<>1 then 
    begin 
       DosEditName := 87; // ERROR_INVALID_PARAMETER;
       exit;
    end;
    DosEditName := 123; // ERROR_INVALID_NAME;
    ii := 0;
    while Source[ii] <> #0 do
       if (Source[ii] = '\') or (Source[ii] = '/') then exit else inc(ii);
    while Edit[ii] <> #0 do
       if (Edit[ii] = '\') or (Edit[ii] = '/') then exit else inc(ii);

    pSrc  := Source;
    pEd   := Edit;
    pRes  := Target;
    rcLen := 0;
    while pEd[0] <> #0 do
    begin
        if rcLen<cbTarget then
        begin
            case pEd[0] of
            '*':begin
                    pDelimit := pEd;
                    inc(pDelimit);
                    while (rcLen < cbTarget) and (pSrc[0] <> #0) and
                        (upcase(pSrc[0]) <> upcase(pDelimit[0])) do
                    begin
                        if rcLen < cbTarget then
                        begin
                            pRes[0] := upcase(pSrc[0]);
                            inc(pRes); inc(pSrc); inc(rcLen);
                        end;
                    end;
                end;
            '?':begin
                    if (pSrc[0] <> '.') and (pSrc[0] <> #0) then
                    begin
                        if rcLen < cbTarget then
                        begin
                            pRes[0] := upcase(pSrc[0]);
                            inc(pRes); inc(pSrc); inc(rcLen);
                        end;
                    end;
                end;
            '.':begin
                    while (pSrc[0] <> '.') and (pSrc[0] <> #0) do inc(pSrc);
                    pRes[0] := '.';
                    inc(pRes);
                    inc(rcLen);
                    if pSrc[0] <> #0 then inc(pSrc);
                end;
            else begin
                    if (pSrc[0] <> '.') and (pSrc[0] <> #0) then inc(pSrc);
                    if rcLen <cbTarget then
                    begin
                        pRes[0] := upcase(pEd[0]);
                        inc(pRes);
                        inc(rcLen);
                    end;
                end;
            end;
            inc(pEd);
        end else
        begin
            DosEditName := 111; // ERROR_BUFFER_OVERFLOW;
            exit;
        end
    end;
    if rcLen < cbTarget then
    begin
        pRes[0] := #0;
        DosEditName := 0;
    end else
        DosEditName := 111; // ERROR_BUFFER_OVERFLOW;
end;
{$endif}

procedure tFileMatch.AddMask;
var
 I,oPos,
 Pos1,Pos2 : Word;
 iDone     : boolean;
 nP        : pChar;
begin
 oPos := 1; I := 1; iDone := FALSE;
 repeat
  Pos1 := ScanFwd(fMaskDelim1, fMasks, I);
  Pos2 := ScanFwd(fMaskDelim2, fMasks, I);
  if (Pos2 > 0)
   then if Pos1 = 0
         then Pos1 := Pos2
         else Pos1 := MinI(Pos1, Pos2);
  Inc(I);
  if (Pos1 = 0)
   then begin
         Pos1 := succ(length(fMasks));
         iDone := TRUE;
        end;
  if Pos1 > oPos
   then begin
         GetMem(nP, succ(Pos1 - oPos));
         strUpper(strPcopy(nP, copy(fMasks, oPos, Pos1 - oPos)));
         if matchStrings^.IndexOf(nP) >= 0
          then FreeMem(nP, succ(Pos1 - oPos))
          else matchStrings^.Insert(nP);
        end;
  oPos := succ(Pos1);
 until iDone;
end;

function tFileMatch.Matches;
var I      : Integer;
    Source,
    Target : array[0..255] of Char;
begin
 Matches := TRUE;
 StrUpper(StrPcopy(Source, fName));
 For I := 0 to pred(matchStrings^.Count) do
{$IFDEF OS2}
  if (DosEditName(1, Source, matchStrings^.At(I), Target, sizeOf(Target)) = 0) and
     (StrComp(Source, Target) = 0)
{$ELSE}
  if MatchStr(StrPas(matchStrings^.At(I)), StrPas(Source))
{$ENDIF}
        then exit;
 Matches := FALSE;
end;

destructor tFileMatch.Destroy;
begin
 Dispose(matchStrings, Destroy);
 inherited Destroy;
end;

{$ifdef OS2}
function DosReplaceModule(OldModName,NewModName,BackModName: PChar): ApiRet; external 'DOSCALLS' index 417;

constructor tEAcollection.Fetch;
const
 eaNameBfSz = 1024;
 secureSize = 256; {F$#%^k! Bug in DosEnumAttribute}
var
 fN         : array[0..255] of char;
 sV,oV,
 I,eaCn     : Longint;
 Buff       : pByteArray;
 eaN        : pStringCollection;
 pS         : pString;
 pEA,nEA    : pFea2;
{$ifndef fpc}
 eaBuf      : EAop2;
 fStat      : FileStatus4;
{$else}
 eaBuf      : TEAop2;
 fStat      : TFileStatus4;
{$endif}
procedure resFree;
begin
 if eaBuf.fpFEA2List <> nil
  then FreeMem(eaBuf.fpFEA2List, fStat.cbList);
 if eaN <> nil
  then Dispose(eaN, Destroy);
 if Buff <> nil then FreeMem(Buff, eaNameBfSz + secureSize);
end;

begin
 inherited Create(8, 8);
 GetMem(Buff, eaNameBfSz + secureSize);
 New(eaN, Create(8, 8));
 fillChar(fStat, sizeOf(fStat), 0);
 fillChar(eaBuf, sizeOf(eaBuf), 0);
 if (Buff = nil) or (eaN = nil)
  then begin resFree; Fail; end;
 StrPCopy(@fN, fName);
 sV := 1;
 repeat
  eaCn := -1; FillChar(Buff^, eaNameBfSz, 0); {F&^#$@%&k! Really not needed}
  if DosEnumAttribute(EnumEA_RefType_Path, @fN, sV, Buff^, eaNameBfSz, eaCn, EnumEA_Level_No_Value) <> 0
   then begin resFree; Fail; end;
  if eaCn = 0 then break;
  pEA := @Buff^;
  For I := 1 to eaCn do
   begin
    eaN^.Insert(NewStr(StrPas(@pEA^.szName)));
{$ifndef fpc}
    Inc(Longint(pEA), pEA^.oNextEntryOffset);
{$else}
    Inc(Longint(pEA), pEA^.NextEntry);
{$endif}
    Inc(sV);
   end;
 until FALSE;
{$ifndef fpc}
 if DosQueryPathInfo(@fN, Fil_QueryEAsize, fStat, sizeOf(fStat)) <> 0
{$else}
 if DosQueryPathInfo(@fN, Fil_QueryEAsize, @fStat, sizeOf(fStat)) <> 0
{$endif}
  then begin resFree; Fail; end;
 I := 0;
 GetMem(eaBuf.fpFEA2List, fStat.cbList);
 eaBuf.fpGEA2List := @Buff^;
 While I < eaN^.Count do
  begin
   sV := 4; oV := 4;
   repeat
    pS := eaN^.At(I);
    if sV + 4 + succ(length(pS^)) > pred(eaNameBfSz) then break;
    pLong(@Buff^[oV])^ := sV - oV;
    pLong(@Buff^[sV])^ := 0; oV := sV;
    Move(pS^, Buff^[sV + 4], succ(length(pS^)));
    Inc(sV, 4 + succ(length(pS^)));
    Buff^[sV] := 0; sV := (sV + 4) and $FFFFFFFC;
    Inc(I);
   until I >= eaN^.Count;
   pLong(@Buff^[0])^ := sV;
{$ifndef fpc}
   eaBuf.fpFEA2List^.cbList := fStat.cbList;
   if DosQueryPathInfo(@fN, Fil_QueryEAsFromList, eaBuf, sizeOf(eaBuf)) = 0
{$else}
   eaBuf.fpFEA2List^.ListLen := fStat.cbList;
   if DosQueryPathInfo(@fN, Fil_QueryEAsFromList, @eaBuf, sizeOf(eaBuf)) = 0
{$endif}
    then begin
          pEA := @eaBuf.fpFEA2List^.list;
{$ifndef fpc}
          While longint(pEA) - longint(@eaBuf.fpFEA2List^.list) <= eaBuf.fpFEA2List^.cbList do
{$else}
          While longint(pEA) - longint(@eaBuf.fpFEA2List^.list) <= eaBuf.fpFEA2List^.ListLen do
{$endif}
           begin
{$ifndef fpc}
            GetMem(nEA, sizeOf(Fea2) + pEA^.cbName + pEA^.cbValue);
            Move(pEA^, nEA^, sizeOf(Fea2) + pEA^.cbName + pEA^.cbValue);
{$else}
            GetMem(nEA, sizeOf(Fea2) + pEA^.NameLen + pEA^.Value);
            Move(pEA^, nEA^, sizeOf(Fea2) + pEA^.NameLen + pEA^.Value);
{$endif}
            Insert(nEA);
{$ifndef fpc}
            if pEA^.oNextEntryOffset = 0 then break;
            Inc(longint(pEA), pEA^.oNextEntryOffset);
{$else}
            if pEA^.NextEntry = 0 then break;
            Inc(longint(pEA), pEA^.NextEntry);
{$endif}
           end;
         end;
  end;
 resFree;
end;

Function tEAcollection.Attach;
label
 locEx;
const
 eaNameBfSz = 300;
var
 fN         : array[0..255] of Char;
 oldAttr,
 I,fT,maxEA : Longint;
 Buff,OneEA : pByteArray;
{$ifndef fpc}
 eaBuf      : EAop2;
 fInfo      : FileStatus3;
{$else}
 eaBuf      : TEAop2;
 fInfo      : TFileStatus3;
{$endif}
begin
 if (Count = 0) then begin Attach := TRUE; exit; end;
 Attach := FALSE;
 GetMem(Buff, eaNameBfSz);
 maxEA := 0;
 if (Buff = nil) then goto locEx;
{$ifndef fpc}
 if DosQueryPathInfo(StrPCopy(@fN, fName), fil_Standard, fInfo, SizeOf(fInfo)) <> 0
{$else}
 if DosQueryPathInfo(StrPCopy(@fN, fName), fil_Standard, @fInfo, SizeOf(fInfo)) <> 0
{$endif}
  then goto locEx;

{temporary remove hidden/readonly attributes}
 oldAttr := fInfo.attrFile;
 fInfo.attrFile := fInfo.attrFile and not (file_ReadOnly + file_System + file_Hidden);
{$ifndef fpc}
 DosSetPathInfo(@fN, fil_Standard, fInfo, SizeOf(fInfo), 0);
{$else}
 DosSetPathInfo(@fN, fil_Standard, @fInfo, SizeOf(fInfo), 0);
{$endif}
 fInfo.attrFile := oldAttr;

 For I := 0 to pred(Count) do
  with pFea2(At(I))^ do
{$ifndef fpc}
   if sizeOf(Fea2) + cbName + cbValue > maxEA
    then maxEA := sizeOf(Fea2) + cbName + cbValue;
{$else}
   if sizeOf(Fea2) + NameLen + Value > maxEA
    then maxEA := sizeOf(Fea2) + NameLen + Value;
{$endif}
 Inc(maxEA, 4);
 GetMem(oneEA, maxEA);
 pLong(oneEA)^ := maxEA;
 eaBuf.fpGEA2List := @Buff^;
 eaBuf.fpFEA2list := @oneEA^;
{$ifndef fpc}
 For I := 0 to pred(Count) do
  with pFea2(At(I))^ do
   begin
    oNextEntryOffset := 0;
    pLong(@Buff^[0])^ := 4 + 4 + 1 + 1 + cbName;
    pLong(@Buff^[4])^ := 0;
    Buff^[8] := cbName;
    Move(szName, Buff^[9], cbName);
    Buff^[9 + cbName] := 0;
    Move(oNextEntryOffset, oneEA^[4], sizeOf(Fea2) + cbName + cbValue);
    DosSetPathInfo(@fN, fil_QueryEAsize, eaBuf, sizeOf(eaBuf), 0);
   end;
 Attach := DosSetPathInfo(@fN, fil_Standard, fInfo, SizeOf(fInfo), 0) = 0;
{$else}
 For I := 0 to pred(Count) do
  with pFea2(At(I))^ do
   begin
    NextEntry := 0;
    pLong(@Buff^[0])^ := 4 + 4 + 1 + 1 + NameLen;
    pLong(@Buff^[4])^ := 0;
    Buff^[8] := NameLen;
    Move(szName, Buff^[9], NameLen);
    Buff^[9 + NameLen] := 0;
    Move(NextEntry, oneEA^[4], sizeOf(Fea2) + Namelen + Value);
    DosSetPathInfo(@fN, fil_QueryEAsize, @eaBuf, sizeOf(eaBuf), 0);
   end;
 Attach := DosSetPathInfo(@fN, fil_Standard, @fInfo, SizeOf(fInfo), 0) = 0;
{$endif}
locEx:
 FreeMem(oneEA, maxEA);
 if Buff <> nil then FreeMem(Buff, eaNameBfSz);
end;

procedure tEAcollection.FreeItem;
begin
 if Item <> nil
  then with pFea2(Item)^ do
{$ifndef fpc}
        FreeMem(Item, sizeOf(Fea2) + cbName + cbValue);
{$else}
        FreeMem(Item, sizeOf(Fea2) + NameLen + Value);
{$endif}
end;
{$endIf}

Function fileExist;
var
 sr : SearchRec;
begin
 Dos.FindFirst(fName, AnyFile, sr);
 fileExist := Dos.DosError = 0;
{ -- $ifDef virtualpascal}
 Dos.FindClose(sr);
{ -- $endIf}
end;

Function fileRename;
var F : File;
begin
 Assign(F, sName); Rename(F, dName);
 fileRename := ioResult = 0;
end;

Function fileErase;
var F : File;
begin
 Assign(F, FName); SetFAttr(F, Archive);
 Erase(F); fileErase := ioResult = 0;
end;

Function fileLength;
var F : File;
    I : Longint;
begin
 I := fileMode; fileMode := $40; { open_access_ReadOnly + open_share_DenyNone };
 Assign(F, fName); Reset(F, 1);
 fileMode := I;
 if ioResult <> 0
  then fileLength := -1
  else begin
        fileLength := fileSize(F);
        Close(F);
       end;
end;

Function fileCopy;
{$ifDef virtualpascal}
var
 sn,dn : pChar;
begin
 GetMem(sn, succ(length(sName)));
 GetMem(dn, succ(length(dName)));
 StrPCopy(sn, sName);
 StrPCopy(dn, dName);
{$ifdef WIN32}
 fileCopy := CopyFile(sn, dn, False);
{$else}
 fileCopy := False;
{$endif}
{$ifdef OS2}
 fileCopy := DosCopy(sn, dn, dcpy_Existing) = 0;
{$endif}
 FreeMem(sn, succ(length(sName)));
 FreeMem(dn, succ(length(dName)));
end;
{$else}
var
 ISS,OS : pFileStream;
 At    : Word;
 FT    : Longint;
begin
 fileCopy := FALSE;
 New(ISS, Create(sName, stmReadOnly));
 if (ISS = nil) or (ISS^.Error <> steOK)
  then begin
        if ISS <> nil then Dispose(ISS, Destroy);
        exit;
       end;
 New(OS, Create(dName, stmWriteOnly));
 FT := ISS^.GetTime; At := ISS^.GetAttr;
 if (OS = nil) or (OS^.Error <> steOK)
  then begin
        Dispose(ISS, Destroy);
        if OS <> nil then Dispose(OS, Destroy);
        exit;
       end;
 if ISS^.Size <> OS^.CopyFrom(ISS^, -1)
  then begin
        Dispose(OS, Erase);
        fileCopy := FALSE;
       end
  else begin
        OS^.SetAttr(At);
        OS^.SetTime(FT);
        Dispose(OS, Destroy);
        fileCopy := TRUE;
       end;
 Dispose(ISS, Destroy);
end;
{$endIf}

Function makeDirTree;
var
 L,SC : Integer;
 S    : string;
begin
 makeDirTree := FALSE;
 L := 0; SC := 0;
 While L <= length(dirName) do
  begin
   repeat
    Inc(L);
   until (L > length(dirName)) or (dirName[L] in ['/','\',':']);
   if (L <= length(dirName)) and (dirName[L] = ':') then SC := 2;
   if SC > 0 then begin Dec(SC); Continue; end;
   S := copy(dirName, 1, pred(L));
   while (S <> '') and (S[length(S)] in ['/','\']) do Dec(byte(S[0]));
   if (S <> '') and (not fileExist(S)) then mkDir(S);
   if ioResult <> 0 then exit;
  end;
 makeDirTree := TRUE;
end;

Function tempFileName;
var
 D,N,R : string;
 Count : Integer;
{$ifDef OS2}
 sz    : array[0..255] of Char;
 Action: Longint;
 Handle: hFile;
{$endif}
begin
 D := extractDir(fName);
 N := Copy(fName, succ(length(D)), 255);
 if D = '' then D := GetEnv('TEMP');
 if D = '' then D := GetEnv('TMP');
 if not (D[length(D)] in ['/', '\']) then D := D + '\';
 Count := 1000;
 repeat
  R := D + N; Dec(Count);
  While First('?', R) <> 0 do R[First('?', R)] := char(Random(10) + byte('0'));
{$ifDef OS2}
  if DosOpen(strPCopy(@sz, R), Handle, Action, 0, 0,
      open_action_Create_If_New + open_action_Fail_If_Exists,
      open_flags_Fail_On_Error + open_Share_DenyReadWrite +
      open_access_ReadOnly, nil) = 0
   then begin
         DosClose(Handle);
         break;
        end;
{$else}
  if (not fileExist(R)) then break;
{$endIf}
 until (Count = 0);
 if Count = 0
  then tempFileName := ''
  else tempFileName := R;
end;

{$ifDef fpc}
Function SourcePath;
var
  j    : Byte;
  S    : String;
begin
    S := ExtractFileDir(ParamStr(0));
    for j := 1 to byte(S[0]) do if S[j] = '\' then S[j] := DirectorySeparator;
    SourcePath :=  S + DirectorySeparator ;
end;
{$else}
{$ifdef WIN32}
Function SourcePath;
var len   :integer;
    ename :ShortString;
begin
  SysCmdlnParam(0,ename);
  len:=length(ename);
  while len>0 do
    if ename[len] in ['\','/'] then
    begin
      ename[0]:=char(len);
      break
    end else 
      dec(len);

  SourcePath := ename;
end;
{$else}
{$ifDef OS2}
Function SourcePath; assembler {&uses esi,edi};
asm             mov     edi,Environment
                mov     al,0
                mov     ecx,-1
@@cont:         repne   scasb
                scasb
                jnz     @@cont
                mov     esi,edi
                repne   scasb
@@searchSlash:  cmp     byte ptr [edi-1],':'
                je      @@done
                dec     edi
                cmp     edi,esi
                jbe     @@done
                cmp     byte ptr [edi],'/'
                je      @@done
                cmp     byte ptr [edi],'\'
                jne     @@searchSlash
@@done:         sub     edi,esi
                mov     eax,edi
                inc     eax
                mov     ecx,eax
                mov     edi,@result
                stosb
                rep     movsb
end;
{$else}
asm             push    ds
                mov     es,PrefixSeg
                mov     ds,es:[02Ch]
                push    ds
                pop     es
                mov     al,0
                mov     cx,-1
                xor     di,di
@@cont:         repne   scasb
                scasb
                jnz     @@cont
                scasw
                mov     si,di
                repne   scasb
@@searchSlash:  cmp     byte ptr [di-1],':'
                je      @@done
                dec     di
                cmp     di,si
                jbe     @@done
                cmp     byte ptr [di],'/'
                je      @@done
                cmp     byte ptr [di],'\'
                jne     @@searchSlash
@@done:         sub     di,si
                mov     ax,di
                inc     ax
                mov     cx,ax
                les     di,@result
                stosb
                rep     movsb
                pop     ds
end;
{$endIf}
{$endIf}
{$endIf}

procedure fSplit;
var
 I,J : Integer;
begin
 I := length(Path);
 While (I > 1) and (Path[I] <> '.') and (not (Path[I] in ['/','\',':'])) do Dec(I);
 if (I <= 1)
  then begin
        Dir := ''; Name := Path; Ext := '';
        exit;
       end;
 if (Path[I] = '.')
  then if (Path[I - 1] in ['/','\',':'])
        then begin
              Dir := Copy(Path, 1, I - 1);
              Name := Copy(Path, I, 255);
              Ext := '';
              exit;
             end
        else begin
              Ext := Copy(Path, I, 255); J := I;
              While (J > 1) and (not (Path[J] in ['/','\',':'])) do Dec(J);
              if (Path[J] in ['/','\',':'])
               then begin
                     Name := Copy(Path, J + 1, I - J - 1);
                     Dir := Copy(Path, 1, J);
                    end
               else begin
                     Name := Copy(Path, J, I - J);
                     Dir := '';
                    end;
              exit;
             end
  else begin
        Ext := '';
        Name := Copy(Path, I, 255);
        Dir := Copy(Path, 1, I - 1);
       end;
end;

{$ifDef OS2}
function fmsInit; assembler {&uses none};
asm             mov     ecx,Sem
           lock bts     [ecx].tMutexSem.Owner,31      {Lock semaphore updates}
                jnc     @@ok
                mov     al,0
                ret     4
@@ok:           xor     eax,eax
                mov     [ecx].tMutexSem.Next,eax
           lock xchg    [ecx].tMutexSem.Owner,eax
                mov     al,1
end;

function fmsRequest; assembler {&uses none};
asm             mov     eax,fs:[12]            {Get ^Thread Information Block}
                push    dword ptr [eax]                      {Owner : Longint}
                push    eax                                   {Next : Pointer}
@@testSem:      mov     ecx,Sem[4+4]                      {+4+4 since &frame-}
           lock bts     [ecx].tMutexSem.Owner,31
                jnc     @@semFree
                push    1          {There is no hurry since semaphore is busy}
                call    DosSleep                  {Go to sleep for a while...}
                pop     eax
                jmp     @@testSem

@@semFree:      mov     edx,[ecx].tMutexSem.Owner        {Get semaphore owner}
                btr     edx,31                     {Reset `semaphor busy` bit}
                cmp     edx,[esp+4]                     {Owner = current TID?}
                jne     @@notOur
                inc     [ecx].tMutexSem.Count
           lock btr     [ecx].tMutexSem.Owner,31           {Release semaphore}
                add     esp,4+4
                mov     al,1
                ret     4

@@notOur:       mov     eax,esp
                xchg    eax,[ecx].tMutexSem.Next
                test    edx,edx                                   {Owner = 0?}
                jz      @@notBusy
                mov     [esp],eax                              {Save ^nextTID}
           lock btr     [ecx].tMutexSem.Owner,31           {Release semaphore}
                push    dword ptr [esp+4]                            {Our TID}
                call    SuspendThread                     {Sleep until wakeup}
                add     esp,4+4
                mov     al,1
                ret     4

@@notBusy:      xchg    eax,[ecx].tMutexSem.Next
                inc     edx
                mov     [ecx].tMutexSem.Count,edx          {Request count = 1}
                pop     eax                                    {Skip ^nextTID}
                pop     eax
           lock xchg    [ecx].tMutexSem.Owner,eax {Set owner&unlock semaphore}
                mov     al,1
end;

function fmsRelease; assembler {&uses none};
asm
@@testSem:      mov     ecx,Sem
           lock bts     [ecx].tMutexSem.Owner,31      {Lock semaphore updates}
                jnc     @@semFree
                push    1
                call    DosSleep
                pop     eax
                jmp     @@testSem
@@semFree:      mov     eax,fs:[12]
                mov     eax,[eax]
                bts     eax,31              {Set bit 31 in EAX for comparison}
                cmp     eax,[ecx].tMutexSem.Owner
                je      @@isOur
           lock btr     [ecx].tMutexSem.Owner,31           {Release semaphore}
                mov     al,0
                ret     4

@@isOur:        dec     [ecx].tMutexSem.Count             {Request count = 1?}
                jz      @@scanChain
           lock btr     [ecx].tMutexSem.Owner,31           {Release semaphore}
                mov     al,1
                ret     4

@@scanChain:    mov     edx,eax
                mov     eax,ecx
                mov     ecx,[ecx].tMutexSem.Next                    {^nextTID}
                test    ecx,ecx
                jnz     @@scanChain
                mov     ecx,Sem
                cmp     eax,ecx
                je      @@onlyOwner                  {Thread is only in chain}
                mov     [edx].tMutexSem.Next,0      {Remove thread from chain}
                mov     [ecx].tMutexSem.Count,1       {Set request count to 1}
                mov     eax,[eax].tMutexSem.Owner
                push    eax
@@resumeIt:     push    eax                          {ResumeThread(TID = EAX)}
           lock xchg    [ecx].tMutexSem.Owner,eax{Make thread semaphore owner}
                call    ResumeThread                          {Wake up thread}
                cmp     eax,error_Not_Frozen
                jne     @@resumeOK
                push    1
                call    DosSleep
                pop     eax
                pop     eax
                jmp     @@resumeIt
@@resumeOK:     pop     eax
                mov     al,1
                ret     4

@@onlyOwner:    xor     eax,eax
           lock xchg    eax,[ecx].tMutexSem.Owner
                mov     al,1
end;

function fmsCheck; assembler {&uses none};
asm             mov     eax,Sem
                mov     eax,[eax].tMutexSem.Owner
                and     eax,7FFFFFFFh
                setz    al
end;

Function unlockModule;
var
 tmp : array[0..256] of Char;
begin
 unlockModule := DosReplaceModule(strPCopy(tmp, fName), nil, nil) = 0;
end;
{$endIf}

function GetResourceString;
{$ifdef os2}
var
 pS : pByte;
 I  : Integer;
 S  : string;
begin
 if DosGetResource(nullHandle, {$ifndef FPC}rt_String{$else}rtString{$endif}, ID div 16 + 1, Pointer(pS)) <> 0
  then begin
        GetResourceString := '';
        exit;
       end;
 Inc(pS, sizeOf(Word16)); {skip codepage}
 For I := 1 to ID and $0F do Inc(pS, succ(pS^));
 Move(pS^, S, pS^);
 Dec(byte(S[0]));
 DosFreeResource(pS);
 GetResourceString := S;
{$endif}
{$ifdef win32}

{$ifdef fpc}
function SysLoadResourceString(ID: Longint; Buffer: PChar; BufSize: Longint): PChar;
begin
  Buffer[0] := #0;
  LoadString(HInstance, ID, Buffer, BufSize);
  Result := Buffer;
end;
{$endif}

var
 buf: array [0..511] of char;
begin
 SysLoadResourceString(ID, buf, sizeof(buf));
 GetResourceString := StrPas(buf);
{$endif}
{$ifdef linux}
begin
 GetResourceString := ''; { !!!! }
{$endif}
end;


procedure tCommandLineParser.Parse;
begin
 PreProcess(S);
 While S <> '' do
  begin
   While (S <> '') and ((S[1] = ' ') or (S[1] = #9)) do
    Delete(S, 1, 1);
   if S <> ''
    then if (S[1] in ['/','-'])
          then begin
                Delete(S, 1, 1);
                if (S <> '') then Delete(S, 1, ParmHandler(S));
               end
          else Delete(S, 1, NameHandler(S));
  end;
 PostProcess;
end;

procedure tCommandLineParser.ParseCommandLine;
var
 ParmStr : string;
 CPtr    : pchar;
begin
{$ifdef win32}
 CPtr := CmdLine;
 if CPtr<>nil then 
 begin
    if (CPtr^='"') then
    begin
       inc(CPtr);
       CPtr:=StrScan(CPtr, '"');
    end else
       CPtr:=StrScan(CPtr, ' ');
    
    if CPtr<>nil then inc(CPtr);
 end;
 if CPtr=nil then ParmStr:='' else ParmStr:=StrPas(CPtr);
{$else}
{$ifdef dos}
 Move(mem[PrefixSeg:$80], ParmStr, succ(mem[PrefixSeg:$80]));
{$else}
 if CmdLine = nil
  then ParmStr := ''
  else ParmStr := StrPas(GetASCIIZptr(CmdLine^, 2));
{$endif}
{$endif}
 Parse(ParmStr);
end;

Function tCommandLineParser.GetWord;
var
 I,J : Word;
 fCh : Char;
begin
 I := StartChar;
 if (I <= length(ParmStr)) and (not (ParmStr[I] in [#9,' ','/','-']))
  then begin
        J := I;
        if ParmStr[I] = '"'
         then fCh := '"'
         else fCh := ' ';
        repeat
         Inc(I);
        until (I > length(ParmStr)) or (ParmStr[I] = fCh);
        if fCh = '"'
         then begin DestStr := Copy(ParmStr, succ(J), pred(I - J)); Inc(I); end
         else DestStr := Copy(ParmStr, J, I - J);
       end
  else DestStr := '';
 GetWord := I - StartChar;
end;

function tCommandLineParser.GetOpt;
var
 I,J,K : Longint;
 Ch    : Char;
 fCh   : boolean;
begin
 K := StartChar; I := 0;
 fCh := TRUE;
 repeat
  if StartChar <= length(parmStr)
   then Ch := UpCase(ParmStr[StartChar])
   else Ch := ' ';
  case Ch of
   '-' : begin
          if fCh then I := OptFlags[high(OptFlags)];
          Option := Option and (not I);
          I := 0;
         end;
   ':',
   '+' : begin
          if fCh then I := OptFlags[high(OptFlags)];
          Option := Option or I;
          I := 0;
          if Ch = ':' then break;
         end;
   else begin
         J := First(Ch, OptChars);
         if J = 0
          then begin
                if fCh then I := OptFlags[high(OptFlags)];
                Option := Option or I;
                break;
               end
          else I := I or OptFlags[pred(J) + low(OptFlags)];
        end;
  end;
  fCh := FALSE;
  Inc(StartChar);
 until FALSE;
 GetOpt := StartChar - K;
end;

function tCommandLineParser.ParmHandler;
var
 I : Integer;
begin
 I := 0; While (I < length(ParmStr)) and (ParmStr[succ(I)] <= ' ') do Inc(I);
 ParmHandler := I;
end;

function tCommandLineParser.NameHandler;
begin
 NameHandler := ParmHandler(ParmStr);
end;

procedure tCommandLineParser.PreProcess;
begin
end;

procedure tCommandLineParser.PostProcess;
begin
end;

end.
