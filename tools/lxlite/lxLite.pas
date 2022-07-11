{&G3+}
{$ifndef fpc}
{$R lxlite.res}
{$R os2api.res}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-,Use32+}
{&M 262144}
{$else}
{$MinStackSize 262144}
{$Align 1}
{$mode objfpc}
{$asmmode intel}
{$Optimization STACKFRAME}
{$endif}
uses
 Dos, Crt, 
{$IFDEF OS2}
 os2def, {$ifndef fpc} os2base, {$else} doscalls, drivers, {$endif} 
{$ENDIF}
 exe386, os2exe, strOp, miscUtil,
 SysLib, Collect, Country, Strings, lxLite_Global, lxLite_Objects, vpsyslow;

label
 done;

Procedure LoadStub;
type
 pDosEXEheader = ^tDosEXEheader;
 tDosEXEheader = record
  ID        : SmallWord;
  PartPage  : SmallWord;
  PageCount : SmallWord;
  ReloCount : SmallWord;
  HeaderSize: SmallWord;
  MinAlloc  : SmallWord;
  MaxAlloc  : SmallWord;
  InitSS    : SmallWord;
  InitSP    : SmallWord;
  CheckSum  : SmallWord;
  InitIP    : SmallWord;
  InitCS    : SmallWord;
  RelTblOfs : SmallWord;
  Overlay   : SmallWord;
  dummy     : array[1..16] of SmallWord;
  ExtHdrOfs : Longint;
 end;
var
 F    : File;
 EH   : pDosEXEheader;
 P    : pByteArray;
 S,hS : Longint;
 stubPath: string;
 unixroot: string;
begin
 if (opt.tresholdStub <= 0) or (opt.stubName = '')
  then begin NewStubSz := 0; exit; end;
 //
 // 1.) we search the stub with the full name
 // 2.) we search the stub in the unixroot/usr/share/lxLite dir
 // 3.) we search the stub in the exe dir
 // 
 Assign(F, opt.stubName); Reset(F, 1);
 if ioResult <> 0
  then begin 
{$ifdef OS2}
   unixroot := GetEnv('UNIXROOT');
{$else}
   unixroot := '';
{$endif}
   if unixroot <> '' then begin
    stubPath := unixroot + '\usr\share\lxLite\'; 
    Assign(F, stubPath + opt.stubName); Reset(F, 1);
   end;
  end;
 if ioResult <> 0
  then begin Assign(F, SourcePath + opt.stubName); Reset(F, 1); end;
 if ioResult <> 0 then Stop(msgCantLoadStub, opt.stubName);
 newStubSz := FileSize(F);
 GetMem(newStub, newStubSz);
 BlockRead(F, newStub^, newStubSz);
 Close(F);
 if ioResult <> 0 then Stop(msgCantLoadStub, opt.stubName);
 EH := newStub;
 with EH^ do
  begin
   if (ID <> $4D5A) and (ID <> $5A4D) then Stop(msgInvalidStub, opt.stubName);
   if RelTblOfs < $40
    then begin
          hS := ($40 + ReloCount * 4 + 15) and $FFFFFFF0;
          S := hS + (PageCount * 512 - (512 - PartPage) - HeaderSize * 16);
          GetMem(P, S); FillChar(P^, S, 0);
          Move(newStub^, P^, RelTblOfs);
          pDosEXEheader(P)^.RelTblOfs := $40;
          pDosEXEheader(P)^.HeaderSize := hS shr 4;
          pDosEXEheader(P)^.PageCount := (S + 511) shr 9;
          pDosEXEheader(P)^.PartPage := S and 511;
          Move(pByteArray(newStub)^[RelTblOfs], P^[$40], ReloCount * 4);
          Move(pByteArray(newStub)^[HeaderSize * 16], P^[hS], S - hS);
          FreeMem(newStub, newStubSz);
          newStub := P; newStubSz := S;
         end;
  end;
end;

procedure FreeStub;
begin
 FreeMem(newStub, newStubSz);
 newStubSz := 0;
end;

Procedure ShowConfigList;
var
 I,J,
 xPos : Longint;
 pSC  : pStringCollection;
 pZS  : pZTstrCollection;
 S    : string;
begin
 SetColor($0E); Write(GetResourceString(msgListCfg)); NL;
 For I := 1 to cfgIDs^.Count do
  begin
   SetColor($07); Write('Ã[');
   SetColor($0A); Write(pString(cfgIDs^.At(pred(I)))^);
   SetColor($07); Write(']'); NL;
   pSC := pStringCollection(cfgOpts^.At(pred(I)));
   For J := 1 to pSC^.Count do
    begin
     SetColor($07); Write('Ã ');
     SetColor($02); Write(pString(pSC^.At(pred(J)))^); NL;
    end;
  end;
 SetColor($0E); Write(GetResourceString(msgListSel)); NL;
 For I := 1 to extra^.Count do
  begin
   pZS := pFileMatch(extra^.At(pred(I)))^.matchStrings;
   xPos := 1000;
   For J := 1 to pZS^.Count do
    begin
     S := strPas(pZS^.At(pred(J)));

     if xPos + length(S) > lo(WindMax)
      then begin
            if xPos <> 1000 then NL;
            SetColor($07);
            if xPos <> 1000 then Write('Ã ') else Write('Ã[');
            Write('/');
            SetColor($0A);
            xPos := 3;
           end
      else S := ':' + S;
     Write(S); Inc(xPos, length(S));
    end;
   SetColor($07); Write(']'); NL;
   pSC := extraOpts^.At(pred(I));
   For J := 1 to pSC^.Count do
    begin
     SetColor($07); Write('Ã ');
     SetColor($02); Write(pString(pSC^.At(pred(J)))^); NL;
    end;
  end;
end;

Function CheckError(ec : byte) : boolean;
begin
 if ec <> lxeOK
  then begin
        SetColor($0C);
        Write(GetResourceString(msgLXerror + ec));
        SetColor($0B); Writeln(#13'Ã');
        CheckError := TRUE;
       end
  else CheckError := FALSE;
end;

var
 prevProgressValue : Longint;

function showProgress(Current,Max : Longint) : boolean;
var
 S   : string;
 val : Longint;
begin
 if RedirOutput then exit;
 S := Strg('±', 20);
 val := Current * 20 div Max;
 if val <> prevProgressValue
  then begin
        FillChar(S[1], val, 'Û');
        SetColor($03);
        Write(S,']' + Strg(#8, length(S) + 2) + '[');
        prevProgressValue := val;
       end;
end;

function altChar(Ch : Char) : Char;
const
 altCh : array[16..143] of Char =
  ('Q','W','E','R','T','Y','U','I','O','P','[',']',' ',' ','A','S',
   'D','F','G','H','J','K','L',';','''','`',' ','\','Z','X','C','V',
   'B','N','M',',','.','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
   ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
   ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
   ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
   ' ',' ',' ',' ',' ',' ',' ',' ','1','2','3','4','5','6','7','8',
   '9','0','-','=',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ');
begin
 AltChar := altCh[byte(Ch)];
end;

Function Ask(idQ : Word; argQ : array of const; idA : Word; qNo : byte) : byte;
const
  auxAsk : array[askFirst..askLast] of Char = (#0, #0, #0, #0, #0, #0);
var
 Q,A : string;
 ch  : char;
 Alt : boolean;
 N   : Integer;
begin
 Q := FormatStr(idQ, argQ);
 A := GetResourceString(idA);
 if auxAsk[qNo] <> #0
  then ch := auxAsk[qNo]
  else ch := opt.AskStatus[qNo].Reply;
 N := First(upCase(ch), A);
 if N <> 0 then begin Ask := N; exit; end;
 SetColor($02);
 Write('À ', Q, ' ');
 repeat
  Alt := FALSE;
  ch := upCase(ReadKey);
  if ch = #0
   then begin Alt := TRUE; ch := altChar(ReadKey); end;
  if First(ch, A) <> 0
   then begin
         Ask := First(ch, A);
         if Alt then auxAsk[qNo] := ch;
         break;
        end;
 until FALSE;
 Writeln(Ch, #13'Ã');
end;

var
 askU : byte;

Function CheckUseCount(const fName : string) : boolean;
var
 F : File;
 I : Longint;
begin
 CheckUseCount := FALSE; askU := 0;
 I := FileMode; FileMode := open_access_ReadOnly or open_share_DenyReadWrite;
 Assign(F, fName);
 Reset(F, 1); Close(F); FileMode := I;
 if ioResult = 0 then exit;
 SetColor($0E);
 Writeln(FormatStr(msgModInUse, [fName]));
 CheckUseCount := TRUE;
 askU := Ask(msgModInUseAsk, [nil], msgModInUseRpl, askInUse);
 case askU of
  1 : ;
  2 : exit;
  3 : begin allDone := TRUE; exit; end;
 end;
{$IFDEF OS2}
 if not unlockModule(fName)
  then begin
        SetColor($0C);
        Writeln(FormatStr(msgModInUseCant, [fName]));
        exit;
       end;
{$ENDIF}
 CheckUseCount := FALSE;
end;

Procedure StoreData(const fName,fMask : string; var destF : string;
                    var Buff; BuffSize : Longint);
var
 Source,
 Mask,
 Target : array[0..255] of Char;
 F      : File;
 _d     : DirStr;
 _n     : NameStr;
 _e     : ExtStr;

begin
 if (fMask = '') or (@buff = nil) then Exit;
 fSplit(fName, _d, _n, _e);
 StrPcopy(Source, _n + _e);
 StrPcopy(Mask, fMask);
 if DosEditName(1, Source, Mask, Target, sizeOf(Target)) <> 0
  then Stop(msgBadFileMask, '');
 if StrComp(Source, Target) = 0 then Stop(msgEqualFilename, '');
 destF := _d + StrPas(Target);
 Assign(F, destF); Rewrite(F, 1);
 if ioResult <> 0 then Stop(msgCantWriteOut, '');
 BlockWrite(F, Buff, BuffSize);
 inOutRes := 0; Close(F); inOutRes := 0;
end;

Function CheckIfProcessed(const fName : string) : boolean;
var
 i : Integer;
 s : String;
begin
 s := lowStrg(fExpand(fName));
 i := pfNames^.IndexOf(@s);
 CheckIfProcessed := (i <> -1);
 if i = -1 then pfNames^.Insert(NewStr(s));
end;

Procedure ProcessFile(fName : string);
label
 SaveLX,locEx;
var
 _d       : DirStr;
 _n       : NameStr;
 _e       : ExtStr;
 bk,newbk,dbgOut,xtrOut,
 stbOut   : string;
 Short    : string[28];
 I,oldDbgInfoOfs,
 bkf,exT,_ss,_fs,
 rc,ss,fs : Longint;
 askD,askX,askN,
 askB     : Byte;
 isPacked : boolean;

Procedure TrackProcess;
begin
 SetColor($0B);
 if not RedirOutput
  then begin Write(#13); ClearToEOL; end;
 Short := Copy(_n + _e, 1, 28);
 Write(FormatStr(msgProcessing, [Short]));
end;

Procedure LogError(rc : Word);
begin
 if opt.Log and lcfUnsucc <> 0
  then begin
        bk := Cntry^.TimeStr(toStdTimeL);
        _d := GetResourceString(rc);
        Writeln(logFile, FormatStr(msgLogError, [bk, Short, _d]));
       end;
 // we also write all errors to stderr
 WriteError(GetResourceString(rc));
end;

const
 OptBackup  : boolean = FALSE;
var
 oldOpt     : Pointer;
 oldfNames  : pStringCollection;
 oldExclude : pZTstrCollection;
 oldLoadCFG : pStringCollection;
 oldStub    : pByteArray;
 oldStubSz  : Longint;

procedure PushOptions;
begin
 if OptBackup
  then Stop(msgRecursiveCfg, '');
 OptBackup := TRUE;
 GetMem(oldOpt, sizeOf(opt));
 Move(opt, oldOpt^, sizeOf(opt));
 New(oldfNames, Clone(fNames));
 New(oldExclude, Clone(exclude^.matchStrings));
 New(oldLoadCFG, Clone(loadCFG));

 GetMem(oldStub, newStubSz);
 Move(newStub^, oldStub^, newStubSz);
 oldStubSz := newStubSz;
end;

procedure PopOptions;
begin
 if not OptBackup then exit;
 OptBackup := FALSE;
 Move(oldOpt^, opt, sizeOf(opt));
 FreeMem(oldOpt, sizeOf(opt));
 Dispose(exclude^.matchStrings, Destroy);
 Dispose(fNames, Destroy);
 Dispose(loadCFG, Destroy);
 fNames := oldfNames;
 exclude^.matchStrings := oldExclude;
 loadCFG := oldLoadCFG;

 FreeStub;
 newStubSz := oldStubSz;
 newStub := oldStub;
end;

procedure CheckExtraOptions;
var
 I,J : Integer;
 S   : string;
 pSC : pStringCollection;
begin
 S := _n + _e;
 For I := 0 to pred(extra^.Count) do
  if pFileMatch(extra^.At(I))^.Matches(S)
   then begin
         PushOptions;
         pSC := extraOpts^.At(I);
         For J := 0 to pred(pSC^.Count) do
          begin
           S := pString(pSC^.At(J))^;
           Parser^.Parse(S);
          end;
         LoadStub;
         break;
        end;
end;

begin
 fSplit(fName, _d, _n, _e);
 if exclude^.Matches(_n + _e) then exit;
 if (opt.backupDir <> '')
  then begin
        bk := opt.backupDir;
        While (bk <> '') and (bk[1] in ['\','/']) do Delete(bk, 1, 1);
        While (bk <> '') and (bk[length(bk)] in ['\','/']) do Delete(bk, length(bk), 1);
        if (bk <> '') and (Pos(lowStrg(bk), lowStrg(_d)) <> 0)
         then exit;
       end;

 if not opt.DiscardXOpts then CheckExtraOptions;
 TrackProcess;
 askD := 0; askX := 0; askB := 0; askU := 0;
 dbgOut := ''; xtrOut := ''; stbOut := '';
 bkf := $8000;
 if opt.doUnpack
  then begin
        opt.Unpack := TRUE;
        opt.PackMode := opt.PackMode and not (pkfRunLength or pkfLempelZiv or pkfSixPack);
       end;

 exT := ntfLXmodule;
 rc := LX^.LoadLX(fName, opt.pageToEnlarge); { 2011-11-16 SHL  }
 if (rc = lxeIsNEformat) and (opt.NEloadMode and lneAlways <> 0)
  then begin
        rc := LX^.LoadNE(fName, opt.NEloadMode);
        exT := ntfNEmodule; bkf := bkf or bkfIfNE;
       end;
 if CheckError(rc)
  then begin LogError(msgLXerror + rc); Goto locEx; end;
 oldDbgInfoOfs := LX^.Header.lxDebugInfoOfs;
 with LX^ do
 begin
   if Header.lxDebugLen > 0
   then begin
      if (Header.lxDebugLen < opt.tresholdDbug) or (opt.Verbose and vfDebug <> 0)
      then begin
         Write(#13); ClearToEOL;
         SetColor($0E);
         Writeln(FormatStr(msgModDebugInfo, [Short, Header.lxDebugLen]));
      end;
     if (Header.lxDebugLen < opt.tresholdDbug) and (opt.FinalWrite and fwfWrite <> 0)
     then begin
        askD := Ask(msgModDebugAsk, [nil], msgModDebugRpl, askDbgInfo);
        if (opt.ForceOut and fofDebug <> 0) or (askD = 1)
         then StoreData(fName, opt.ddFileMask, dbgOut, DebugInfo^, Header.lxDebugLen);
        case askD of
          1 : if Header.lxDebugInfoOfs <> 0
               then begin
                     FreeMem(DebugInfo, Header.lxDebugLen);
                     Header.lxDebugInfoOfs := 0;
                     Header.lxDebugLen := 0;
                    end;
          3 : begin LogError(msgDbgSkip); Goto locEx; end;
          4 : begin LogError(msgDbgAbort); allDone := TRUE; Goto locEx; end;
        end;
        bkf := bkf or bkfIfDebug;
     end;
     TrackProcess;
   end;
  if OverlaySize > 0
   then begin
      if (OverlaySize < opt.tresholdXtra) or (opt.Verbose and vfExtra <> 0)
      then begin
         Write(#13); ClearToEOL;
         SetColor($0E);
         Writeln(FormatStr(msgModOverlay, [Short, OverlaySize]));
      end;
      if (OverlaySize < opt.tresholdXtra) and (opt.FinalWrite and fwfWrite <> 0)
      then begin
         askX := Ask(msgModOvrAsk, [nil], msgModOvrRpl, askExtraData);
         if (opt.ForceOut and fofXtra <> 0) or (askX = 1)
          then StoreData(fName, opt.xdFileMask, xtrOut, Overlay^, OverlaySize);
         case askX of
          1 : begin
               FreeMem(Overlay, OverlaySize);
               OverlaySize := 0;
              end;
          3 : begin LogError(msgOverlaySkip); Goto locEx; end;
          4 : begin LogError(msgOverlayAbort); allDone := TRUE; Goto locEx; end;
         end;
         bkf := bkf or bkfIfXtra;
      end;
      TrackProcess;
   end;
 end;

 if (opt.PageWriteOpt and 1)<>0 then
   LX^.WritePagesToFile(opt.PageRWStart,opt.PageRWEnd,opt.pageFileName,opt.PageWriteOpt)
 else
 if opt.PageRWStart>0 then
   LX^.ReadPagesFromFile(opt.PageRWStart,opt.PageRWEnd,opt.pageFileName,opt.PageReadPack);

 if opt.tresholdStub > 0 then I := newStubSz else I := -1;
 if (not opt.ForceRepack) and (askD <> 1) and (askX <> 1) and (not opt.doUnpack) and
    (LX^.isPacked(opt.Realign, I, opt.PackMode, opt.SaveMode, oldDbgInfoOfs, fs))
  then begin
        SetColor($0C); Write(GetResourceString(msgAlreadyProc));
        SetColor($0B); Writeln(#13'Ã');
        if opt.Log and lcfAlways = lcfAlways then LogError(msgAlreadyProc);
        Goto locEx;
       end;
 with LX^ do
  if (NResNameTbl^.Count > 0) and (opt.FinalWrite and fwfWrite <> 0)
   then begin
         rc := 0;
         For I := 0 to pred(NResNameTbl^.Count) do
          if pNameTblRec(NResNameTbl^.At(I))^.Ord <> 0
           then Inc(rc);
         if rc > 0
          then begin
                rc := 0;
                For I := 0 to pred(NResNameTbl^.Count) do
                 if pNameTblRec(NResNameTbl^.At(I))^.Ord <> 0
                  then Inc(rc, succ(length(pNameTblRec(NResNameTbl^.At(I))^.Name^)) + sizeOf(Word16));
                Write(#13); ClearToEOL;
                SetColor($0E);
                Writeln(FormatStr(msgNResTable, [Short, rc]));
                askN := Ask(msgNResAsk, [nil], msgNResRpl, askNResTable);
                case askN of
                 1 : For I := pred(NResNameTbl^.Count) downto 0 do
                      if pNameTblRec(NResNameTbl^.At(I))^.Ord <> 0
                       then NResNameTbl^.AtFree(I);
                 3 : begin LogError(msgNResSkip); Goto locEx; end;
                 4 : begin LogError(msgNResAbort); allDone := TRUE; Goto locEx; end;
                end;
                TrackProcess;
               {bkf := bkf or bkfIfXtra;}
               end;
        end;
 if (LX^.StubSize < opt.tresholdStub)
  then with LX^ do
        begin
         StoreData(fName, opt.sdFileMask, stbOut, Stub^, StubSize);
         FreeMem(Stub, StubSize);
         GetMem(Stub, NewStubSz);
         Move(NewStub^, Stub^, NewStubSz);
         StubSize := NewStubSz;
        end
  else if opt.ForceOut and fofStub <> 0
        then StoreData(fName, opt.sdFileMask, stbOut, LX^.Stub^, LX^.StubSize);
 ss := FileLength(fName);
 with LX^.Header do
  if (opt.NewTypeCond and (ntfLXmodule or ntfNEmodule) = 0) or
     (opt.NewTypeCond and exT <> 0)
   then begin
         case lxMFlags and lxModType of
          lxEXE   : exT := ntfExecutable;
          lxDLL   : exT := ntfLibrary;
          lxPMDLL : exT := ntfLibrary;
          lxPDD   : exT := ntfPhysDriver;
          lxVDD   : exT := ntfVirtDriver;
          else exT := 0;
         end;
         if opt.NewTypeCond and exT <> 0
          then lxMFlags := (lxMFlags and (not lxModType)) or opt.NewType;
        end;

 if opt.Unpack then LX^.Unpack(opt.AllowZTrunc);
 if opt.ApplyFixups then LX^.ApplyFixups(opt.ForceApply, opt.ApplyMask);

 if opt.Verbose <> 0
  then begin
        { LX^.packFixups(opt.PackMode); // This has weird effects on NE-modules and compression
                                        // Purpose of this call here is not clear
                                        // Maybe it should be executed before compression? }
        LX^.DisplayExeInfo;
       end;

{ Realignment must follow LX^.DisplayExeInfo
  but precede LX^.Pack for better compression }
 if opt.Realign <> NoRealign then LX^.Header.lxPageShift := opt.Realign;

 if (not opt.doUnpack) and (opt.PackMode and (pkfRunLength or pkfLempelZiv or pkfSixPack or pkfFixups) <> 0)
  then begin
        prevProgressValue := -1;
{$ifndef fpc}
        LX^.Pack(opt.PackMode, showProgress, opt.AllowZTrunc);
{$else}
        LX^.Pack(opt.PackMode, @showProgress, opt.AllowZTrunc);
{$endif}

       end;
 Write(#13); ClearToEOL;
 if (opt.FinalWrite = 0) then Goto locEx;
 if (opt.FinalWrite and fwfWrite <> 0)
  then begin
        if CheckUseCount(fName)
         then begin LogError(msgFileInUse); Goto locEx; end;
        bk := _d + _n + '.bak';
        if FileExist(bk)
         then begin
               SetColor($0E);
               Writeln(FormatStr(msgBackupExists, [bk]));
               askB := Ask(msgBackupAsk, [nil], msgBackupRpl, askOverBak);
               case askB of
                1 : FileErase(bk);
                2 : goto SaveLX;
                3 : begin LogError(msgBackupSkip); Goto locEx; end;
                4 : begin allDone := TRUE; Goto locEx; end;
               end;
              end;
        SetColor($0B); Write(FormatStr(msgBackingUp, [Short, bk]));
        if not FileRename(fName, bk)
         then begin
               SetColor($0C); Write(GetResourceString(msgBackupError));
               SetColor($0B); Writeln(#13'Ã');
               LogError(msgBackupError); Goto locEx;
              end;
        Write(#13); ClearToEOL;
       end;
SaveLX:
 if (opt.FinalWrite and fwfWrite <> 0)
  then begin
        SetColor($0B); Write(FormatStr(msgSaving, [Short]));
        rc := LX^.Save(fName, opt.SaveMode);
        if CheckError(rc)
         then begin
               LogError(msgLXerror + rc);
               if not FileCopy(bk, fName) then Stop(msgFatalIOerror, '');
               FileErase(bk);
               Goto locEx;
              end;
        if opt.Backup and bkf = 0
         then begin
               FileErase(bk);
               Write(#13); ClearToEOL;
               SetColor($0B); Write(FormatStr(msgDelBackup, [bk]));
              end
         else if opt.backupDir <> ''
               then begin
                     newbk := opt.backupDir;
                     if (First(':', newbk) <> 0) or
                        (newbk[1] in ['/', '\'])
                      then begin
                            _d := fExpand(_d); fs := 1;
                            While (fs <= length(_d)) and (_d[fs] <> ':') do Inc(fs);
                            if (fs <= length(_d)) then Delete(_d, 1, fs);
                            if (_d <> '') and (_d[1] in ['/','\'])
                             then Delete(_d, 1, 1);
                            newbk := newbk + _d;
                           end
                      else newbk := _d + newbk;
                     if not MakeDirTree(newbk)
                      then Stop(msgCantCreateDir, newbk);
                     newbk := newbk + _n + _e;
                     if not fileExist(newbk)
                      then if (not fileRename(bk, newbk)) and
                              ((not fileCopy(bk, newbk)) or (not fileErase(bk)))
                            then Stop(msgCantCopyBackup, bk)
                            else
                      else if (not fileErase(bk))
                            then Stop(msgCantCopyBackup, bk);
                     CheckIfProcessed(newbk);
                    end
               else CheckIfProcessed(bk);
        fs := FileLength(fName)
       end
  else begin
        if opt.tresholdStub > 0 then I := newStubSz else I := -1;
        LX^.isPacked(opt.Realign, I, opt.PackMode, opt.SaveMode, oldDbgInfoOfs, fs);
       end;
 Write(#13); ClearToEOL;
 SetColor($0B);

 _ss := ss; _fs := fs;
 if (_fs >= 2048*1024)
  then begin
        rc := succ(_fs div (2048*1024));
        _fs := _fs div rc;
        _ss := _ss div rc;
       end;
 _d := long2str(1000 - (_fs * 1000) div _ss);
 If (length(_d) < 2 + byte(_d[1] = '-'))
  then Insert('0.', _d, length(_d))
  else Insert('.', _d, length(_d));
 Writeln(FormatStr(msgCompRate, [Short, ss, fs, _d]));
 Inc(totalGain, ss - fs);

 if opt.Log and lcfSucc <> 0
  then begin
        bk := Cntry^.TimeStr(toStdTimeL);
        Writeln(logFile, FormatStr(msgLogOp, [bk, Short, ss, fs, _d]));
        case askD of
         1 : if dbgOut <> ''
              then Writeln(logFile, FormatStr(msgLogDebug, [dbgOut]))
              else Writeln(logFile, GetResourceString(msgLogDebugRmv));
         2 : Writeln(logFile, GetResourceString(msgLogDebugKept));
        end;
        case askX of
         1 : if xtrOut <> ''
              then Writeln(logFile, FormatStr(msgLogXtra, [xtrOut]))
              else Writeln(logFile, GetResourceString(msgLogXtraRmv));
         2 : Writeln(logFile, msgLogXtraKept);
        end;
        case askN of
         1 : {if xtrOut <> ''
              then Writeln(logFile, FormatStr(msgLogXtra, [xtrOut]))
              else }Writeln(logFile, GetResourceString(msgLogNResRmv));
         2 : Writeln(logFile, msgLogNResKept);
        end;
        case askB of
         1 : Writeln(logFile, GetResourceString(msgLogBackOverw));
         2 : Writeln(logFile, GetResourceString(msgLogBackSkip));
        end;
        case AskU of
         1 : Writeln(logFile, GetResourceString(msgLogModLocked));
        end;
        if stbOut <> ''
         then Writeln(logFile, FormatStr(msgLogStubOut, [stbOut]));
  end;

locEx:
 PopOptions;
end;

procedure LoadModuleDefs;
var
 I  : Integer;
 S  : string;
 MD : pModuleDef;
begin
 New(ModDef, Create(16, 16));
 For I := 1 to 99 do
  begin
   S := GetResourceString(I);
   if charCount('.', S) = 2
    then begin
          New(MD);
          MD^.ModuleName := UpStrg(Copy(S, 1, pred(First('.', S))));
          S := Copy(S, succ(First('.', S)), 255);
          MD^.defStart := decVal(S);
          Delete(S, 1, 1);
          MD^.defLength := decVal(S);
          ModDef^.Insert(MD);
         end;
  end;
end;

Procedure ProcessFiles(const fN : string; Level : Longint);
var
 sr : SearchRec;
 nf : Longint;
 dl : boolean;
 _d : DirStr;
 _n : NameStr;
begin
 _d := extractDir(fN);
 _n := extractName(fN);
 FindFirst(fN, Archive or Hidden or SysFile, sr);
 nf := 0; dl := FALSE;
 if (Dos.DosError <> 0) and (Level = 0) and (not opt.RecurSearch)
  then begin
        SetColor($0C);
        Writeln(FormatStr(msgCantFindFile, [fN]));
       end
  else
 While (Dos.DosError = 0) and (not allDone) do
  begin
   Inc(nf);
   if (length(_d) + length(sr.Name) <= 255) and
      (not CheckIfProcessed(_d + sr.Name))
    then begin
          if opt.Pause
           then case Ask(msgConfirmAsk, [sr.Name], msgConfirmRpl, askConfirm) of
                 2 : sr.Name := '';
                 3 : begin allDone := TRUE; break; end;
                end;
          if (sr.Name <> '')
           then begin
                 if (not dl) and (_d <> '')
                  then begin
                        dl := TRUE; SetColor($0A);
                        Writeln(FormatStr(msgCurDir, [_d]));
                        if (opt.Log <> 0)
                         then Writeln(logFile, FormatStr(msgLogCurDir, [Cntry^.TimeStr(toStdTimeL), _d]));
                       end;
                 ProcessFile(_d + sr.Name);
                end;
         end;
   FindNext(sr);
  end;
 FindClose(sr);
 if allDone or not opt.RecurSearch then Exit;
 if nf = 0
  then begin
        SetColor($0B); Write('À ', Short(_d, 77));
        ClearToEOL; Write(#13);
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

procedure InitLogFile;
var
 sD,sT : string[20];
begin
 if opt.logFileName = '' then opt.Log := 0;
 if opt.Log = 0 then exit;
 Assign(logFile, opt.logFileName);
 Append(logFile); if ioResult <> 0 then Rewrite(logFile);
 if ioResult <> 0 then Stop(msgCannotOpenLog, opt.logFileName);

 sD := Cntry^.DateStr(doStdDateL);
 sT := Cntry^.TimeStr(toStdTimeL);
 Writeln(logFile, FormatStr(msgLogStart, [sD, sT, Version]));
end;

Procedure MyExitProc;
begin
 if TextRec(logFile).Handle <> 0 then Close(logFile);
 FreeStub;
 if ModDef    <> nil then Dispose(ModDef, Destroy);
 if pfNames   <> nil then Dispose(pfNames, Destroy);
 if exclude   <> nil then Dispose(exclude, Destroy);
 if Parser    <> nil then Dispose(Parser, Destroy);
 if loadCFG   <> nil then Dispose(loadCFG, Destroy);
 if fNames    <> nil then Dispose(fNames, Destroy);
 if LX        <> nil then Dispose(LX, Destroy);
 if Cntry     <> nil then Dispose(Cntry, Destroy);
 if extraOpts <> nil then Dispose(extraOpts, Destroy);
 if extra     <> nil then Dispose(extra, Destroy);
 if cfgOpts   <> nil then Dispose(cfgOpts, Destroy);
 if cfgIDs    <> nil then Dispose(cfgIDs, Destroy);
 if exitCode >= 100
  then begin
        if WhereX > 1 then Writeln;
        SetColor($4F); ClearToEOL;
        Writeln(FormatStr(msgRuntime1, [exitCode, errorAddr, Version]));
        ClearToEOL;
        Writeln(GetResourceString(msgRuntime2));
       end;
 if not RedirOutput
  then begin
        Write(#13);
        SetColor($07); ClearToEOL;
       end;
{$ifndef fpc}
 OldExit;
{$endif}
 Halt(exitCode);
end;

var
 I : longint;

begin
 SetColor($0F);
{$ifndef fpc}
 @OldExit := ExitProc; ExitProc := @MyExitProc;
{$else}
 AddExitProc(@MyExitProc);
{$endif}
// HeapBlock := 64 * 1024; {for VP 1.10}

 New(cfgIDs, Create(16, 16));
 New(cfgOpts, Create(16, 16));
 New(extra, Create(16, 16));
 New(extraOpts, Create(16, 16));
 LoadConfig;

 New(Cntry, Create(cyDefault, cpDefault));
 if Cntry = nil then Stop(msgNoCountryInfo, '');
 New(LX, Create);
 New(fNames, Create(16, 16));
 New(loadCFG, Create(16, 16));
 New(Parser, Create);
 New(exclude, Create(''));
 New(pfNames, Create(16, 16));

 setConfig('default');
 Parser^.ParseCommandLine;
 PrintHeader;
{$IFDEF OS2}
 if opt.ForceIdle then DosSetPriority(Prtys_ProcessTree, Prtyc_IdleTime, 16, 0);
{$ENDIF}
 if opt.QueryCfgList then begin ShowConfigList; Goto Done; end;
 if (fNames^.Count = 0) and (not opt.ShowConfig) then Stop(1, '');
 LoadModuleDefs;
 LoadStub;
 InitLogFile;
 if opt.ShowConfig then ShowConfig;

 I := 0;
 While I < fNames^.Count do
  begin
   ProcessFiles(pString(fNames^.At(I))^, 0);
   if allDone then break else Inc(I);
  end;

 ClearToEOL;
 if totalGain <> 0
  then begin
        SetColor($03);
        Writeln(FormatStr(msgOverall, [totalGain]));
       end;
 if opt.Log <> 0
  then Writeln(logFile, FormatStr(msgLogOverall, [totalGain]));

done:
 SetColor($01);
 if not RedirOutput then Write(#13);
 Writeln(GetResourceString(msgDone));
end.
