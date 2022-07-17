{&G3+}
{$ifndef fpc}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-,Use32+}
{$else}
{$Align 1}
{$mode objfpc}
{$H-}
{$Optimization STACKFRAME}
{$endif}
Unit lxLite_Global;

Interface uses exe286, exe386, os2exe, Collect, SysLib, Country
{$ifdef win32}
  , windows
{$endif};
{$ifdef fpc}
var
 StdIn,                        { Old standard input }
 StdOut: Text;                 { and output streams }

const
 RedirOutput: boolean = False; {True if stdOut is redirected}
 RedirInput: boolean = False;  {True if stdIn is redirected}
{$endif}

const
 Version          : string[6] = '1.3.9';

{ Message Identifiers }
 msgProgHeader1   = 100;
 msgProgHeader2   = 101;
 msgProgHeader3   = 102;
 msgProgHeader4   = 103;
 msgProgHeader5   = 104;
 msgProgHeader6   = 105;
 msgDone          = 106;
 msgMore          = 107;
 msgInvalidSwitch = 108;
 msgAborted       = 109;
 msgRuntime1      = 110;
 msgRuntime2      = 111;

 msgCantLoadStub  = 151;
 msgFatalIOerror  = 152;
 msgInvalidStub   = 153;
 msgCannotLoadCFG = 154;
 msgCannotOpenLog = 155;
 msgNoCountryInfo = 156;
 msgBadFileMask   = 157;
 msgEqualFilename = 158;
 msgCantWriteOut  = 159;
 msgRecursiveCfg  = 160;
 msgCantCreateDir = 161;
 msgCantCopyBackup= 162;
 msgCfgLoadFailed = 163;
 msgCantWriteBin  = 164;
 msgCantReadBin   = 165;

 msgEmpty         = 192;

 msgPageFlags     = 193;

 msgEntryPoints   = 200;

 msgCPU           = 206;

 msgNameTableHdr  = 224;
 msgImpTableHdr   = 225;
 msgExported      = 226;
 msgSharedData    = 227;
 msgParamCount    = 228;
 msgProcOffs      = 229;
 msgApplication   = 230;
 msgFullScreen    = 231;
 msgWindowed      = 232;
 msgPMapplication = 233;
 msgUnknownType   = 234;
 msgDLL           = 235;
 msgProtDLL       = 236;
 msgPDD           = 237;
 msgVDD           = 238;
 msgPerProcInit   = 239;
 msgPerProcTerm   = 240;
 msgUnknownModType= 241;
 msgNoIntFixups   = 242;
 msgNoExtFixups   = 243;
 msgNotLoadable   = 244;
 msgModuleType    = 250;
 msgReqCPU        = 251;
 msgReqOS         = 252;
 msgPageSize      = 253;
 msgObjects       = 254;
 msgObjTableOfs   = 255;
 msgMapTableOfs   = 256;
 msgNResTableOfs  = 257;
 msgImpProcOfs    = 258;
 msgPageDataOfs   = 259;
 msgPageFixOfs    = 260;
 msgFixupSize     = 261;
 msgResidentSize  = 262;
 msgImpTableOfs   = 263;
 msgImpEntries    = 264;
 msgStartEIP      = 265;
 msgAutoData      = 266;
 msgStackSize     = 267;
 msgModuleName    = 268;
 msgDescription   = 269;
 msgObjectTable   = 270;
 msgObjTableHdr   = 271;
 msgPageTableHdr  = 272;
 msgResTableHdr   = 273;
 msgNResTableHdr  = 274;
 msgImportTable   = 275;
 msgImpProcHdr    = 276;
 msgEntryTable    = 277;
 msgEntryTblHdr   = 278;
 msgRelocTable    = 279;
 msgRelocTblHdr   = 280;
 msgFixByte       = 281;
 msgFixSelector   = 282;
 msgFixPtr16      = 283;
 msgFixOfs16      = 284;
 msgFixPtr48      = 285;
 msgFixOfs32      = 286;
 msgFixRelOfs32   = 287;
 msgFixOf         = 288;
 msgFixObject     = 289;
 msgFixOffset     = 290;
 msgFixImport     = 291;
 msgFixModEntry   = 292;

 msgConfigHeader  = 300;
 msgShowInfo      = 301;
 msgForceIdle     = 302;
 msgUnpack        = 303;
 msgBackup        = 304;
 msgPause         = 305;
 msgReplaceStub   = 306;
 msgRemoveStub    = 307;
 msgAlignFO       = 308;
 msgAlignFONone   = 309;
 msgAlignFOShift  = 310;
 msgAlignFOSect   = 311;
 msgAlignObj      = 312;
 msgAlignObjShift = 313;
 msgAlignObjSect  = 314;
 msgSetPageShift  = 315;
 msgDontChange    = 316;
 msgRunLength     = 317;
 msgRLoff         = 318;
 msgRLmin         = 319;
 msgRLmed         = 320;
 msgRLmax         = 321;
 msgLempelZiv     = 322;
 msgExcludedFiles = 323;
 msgFixupsPack    = 324;
 msgFXoff         = 325;
 msgFXv2          = 326;
 msgFXv4          = 327;
 msgFXmax         = 328;
 msgSixPack       = 329;
 msgPageM3        = 330;
 msgWritingPage   = 331;
 msgReadingPage   = 332;

 msgLXerror       = 350;

 msgModInUse      = 400;
 msgModInUseAsk   = 401;
 msgModInUseRpl   = 402;
 msgModInUseCant  = 403;
 msgProcessing    = 410;
 msgModDebugInfo  = 411;
 msgModDebugAsk   = 412;
 msgModDebugRpl   = 413;
 msgAlreadyProc   = 414;
 msgModOverlay    = 415;
 msgModOvrAsk     = 416;
 msgModOvrRpl     = 417;
 msgBackupExists  = 418;
 msgBackupAsk     = 419;
 msgBackupRpl     = 420;
 msgBackingUp     = 421;
 msgBackupError   = 422;
 msgSaving        = 423;
 msgCompRate      = 424;
 msgOverall       = 425;
 msgCurDir        = 426;
 msgNResTable     = 427;
 msgNResAsk       = 428;
 msgNResRpl       = 429;

 msgLogStart      = 430;
 msgLogOp         = 431;
 msgLogDebug      = 432;
 msgLogDebugRmv   = 433;
 msgLogDebugKept  = 434;
 msgLogXtra       = 435;
 msgLogXtraRmv    = 436;
 msgLogXtraKept   = 437;
 msgLogBackOverw  = 438;
 msgLogBackSkip   = 439;
 msgLogModLocked  = 440;
 msgLogStubOut    = 441;
 msgLogError      = 442;
 msgLogCurDir     = 443;
 msgLogOverall    = 444;
 msgLogNResRmv    = 445;
 msgLogNResKept   = 446;
 msgDelBackup     = 447;

 msgCantFindFile  = 450;
 msgConfirmAsk    = 451;
 msgConfirmRpl    = 452;

 msgDbgSkip       = 460;
 msgDbgAbort      = 461;
 msgOverlaySkip   = 462;
 msgOverlayAbort  = 463;
 msgFileInUse     = 464;
 msgBackupSkip    = 465;
 msgBackupAbort   = 466;
 msgNResSkip      = 467;
 msgNResAbort     = 468;

 msgListCfg       = 480;
 msgListSel       = 481;

 msgHelpFirst     = 500;
 msgHelpLast      = 616;

{-Backup flags-}
 bkfIfDebug       = $0001;
 bkfIfXtra        = $0002;
 bkfIfNE          = $0004;
 bkfAlways        = $FFFF;

{-Log condition flags-}
 lcfSucc          = $0001;
 lcfUnsucc        = $0002;
 lcfAlways        = $FFFF;

{-NEloadMode flags (except that in OS2EXE.PAS)-}
 lneAlways        = $FFFF;

{-FinalWrite flags-}
 fwfWrite         = $0001;
 fwfSimulate      = $0002;
 fwfAlways        = $FFFF;

{-NewType conditions-}
 ntfExecutable    = $0001;
 ntfLibrary       = $0002;
 ntfPhysDriver    = $0004;
 ntfVirtDriver    = $0008;
 ntfNEmodule      = $0010;
 ntfLXmodule      = $0020;
 ntfAlways        = $FFFF;

{-ForceOut bit flags-}
 fofDebug         = $0001;
 fofXtra          = $0002;
 fofStub          = $0004;
 fofAlways        = $0008;
 fofAnything      = $FFFF;

{-Confirmation query subsystem constants-}
 askInUse         = 1;
 askExtraData     = 2;
 askOverBak       = 3;
 askConfirm       = 4;
 askDbgInfo       = 5;
 askNResTable     = 6;
 askFirst         = askInUse;
 askLast          = askNResTable;

{-Verbose Information flags-}
 vfHeaderL0       = $00000001;
 vfHeaderL1       = $00000002;
 vfHeaderL2       = $00000004;
 vfHeaderL3       = $00000008;
 vfObjects        = $00000010;
 vfPageMap        = $00000030;
 vfResName        = $00000100;
 vfNResName       = $00000200;
 vfImpName        = $00000400;
 vfImpProc        = $00000800;
 vfEntTable       = $00001000;
 vfFixups         = $00010000;
 vfExtra          = $00100000;
 vfDebug          = $00200000;
 vfAnything       = $FFFFFFFF;

 cfgFname         = 'lxLite.cfg';
 logFname         = 'lxLite.log';

{-Configuration parameters-}
 Opt              : record
  Verbose      : Longint;
  NewType      : Longint;
  NewTypeCond  : Longint;               {ntfXXXX flags}
  doUnpack     : boolean;               {Unpack LX and save unpacked}
  pageToEnlarge: LongInt;               {page to enlarge to maximum size when unpacking 2011-11-16 SHL}
  Unpack       : boolean;               {Unpack LX before packing}
  Backup       : longint;
  Pause        : boolean;
  ApplyFixups  : boolean;
  ForceApply   : boolean;
  ApplyMask    : byte;
  SaveMode     : longint;
  PackMode     : longint;
  NEloadMode   : longint;
  Realign      : longint;
  ForceOut     : longint;
  Log          : longint;
  FinalWrite   : longint;
  PageReadPack : longint;
  PageWriteOpt : longint;
  PageRWStart  : longint;
  PageRWEnd    : longint;
  ForceRepack  : boolean;
  ForceIdle    : boolean;
  ShowConfig   : boolean;
  RecurSearch  : boolean;
  QueryCfgList : boolean;
  DiscardXOpts : boolean;
  ColoredOutput: boolean;
  UseStdOut    : boolean;
  AllowZTrunc  : boolean;
  tresholdStub : Longint;
  tresholdXtra : Longint;
  tresholdDbug : Longint;
  stubName     : string;
  logFileName  : string;
  pageFileName : string;
  backupDir    : string;
  xdFileMask   : string;
  ddFileMask   : string;
  sdFileMask   : string;
  AskStatus    : array[askFirst..askLast] of record
                  ID    : char; {The /Y# character}
                  Reply : char; {What to answer}
                 end;
 end =
 (Verbose      : 0;
  NewType      : 0;
  NewTypeCond  : 0;                     {ntfXXXX flags}
  doUnpack     : FALSE;                 {Unpack LX and save unpacked}
  pageToEnlarge: 0;                     {page to enlarge to maximum size when unpacking}
  Unpack       : TRUE;                  {Unpack LX before packing}
  Backup       : 0;
  Pause        : FALSE;
  ApplyFixups  : TRUE;
  ForceApply   : FALSE;
  ApplyMask    : 7;
  SaveMode     : svfFOalnNone + svfEOalnShift;
  PackMode     : pkfLempelZiv + pkfFixups + pkfFixupsVer2;
  NEloadMode   : 0;
  Realign      : NoRealign;
  ForceOut     : 0;
  Log          : lcfSucc;
  FinalWrite   : fwfWrite;
  PageReadPack : 0;
  PageWriteOpt : 0;
  PageRWStart  : -1;
  PageRWEnd    : -1;
  ForceRepack  : FALSE;
  ForceIdle    : TRUE;
  ShowConfig   : FALSE;
  RecurSearch  : FALSE;
  QueryCfgList : FALSE;
  DiscardXOpts : FALSE;
  ColoredOutput: TRUE;
  UseStdOut    : FALSE;
  AllowZTrunc  : TRUE;
  tresholdStub : 0;
  tresholdXtra : 1024;
  tresholdDbug : $7FFFFFFF;
  stubName     : '';
  logFileName  : '';
  pageFileName : '';
  backupDir    : '';
  xdFileMask   : '';
  ddFileMask   : '';
  sdFileMask   : '';
  AskStatus    : ((ID : 'U'; Reply : #0),  {module in Use}
                  (ID : 'X'; Reply : #0),  {eXtra data}
                  (ID : 'B'; Reply : #0),  {overwrite Backup}
                  (ID : 'C'; Reply : #0),  {Confirmation}
                  (ID : 'D'; Reply : #0),  {Debug info}
                  (ID : 'N'; Reply : #0))  {Non-resident name table}
 );

type
 pModuleDef = ^tModuleDef;
 tModuleDef = record
  ModuleName : string;
  defStart,
  defLength  : Longint;
 end;
 pModuleCollection = ^tModuleCollection;
 tModuleCollection = object(tStringCollection)
  procedure FreeItem(Item: Pointer); virtual;
 end;

var
 fNames,
 loadCFG   : pStringCollection;
 exclude   : pFileMatch;
 newStub   : Pointer;
 newStubSz : Longint;

procedure SetColor(Color : Byte);
procedure ClearToEOL;

Implementation uses StrOp
{$ifdef os2}
{$ifdef fpc}
  , doscalls
{$else}
  , os2base
{$endif}
{$endif}
{$ifdef fpc}
      , Crt
{$else}
      , MyCrt
{$endif};

procedure SetColor(Color : Byte);
const
 ColorTranslate : array[0..15] of Byte =
 (0, 7, 7, 7, 7, 7, 7, 7, 8, 7, 15, 15, 15, 15, 15, 15);
var
 B,F : Byte;
begin
 if opt.ColoredOutput
  then textAttr := Color
  else begin
        B := Color shr 4;
        F := Color and $0F;
        textAttr := ColorTranslate[B] * 16 + ColorTranslate[F];
       end;
end;

procedure ClearToEOL;
var
 Spaces : Integer;
begin
 if not RedirOutput
  then if opt.UseStdOut
        then begin
              Spaces := succ(lo(WindMax)) - WhereX;
              Write(strg(' ', Spaces), strg(#8, Spaces));
             end
        else ClrEOL;
end;

procedure tModuleCollection.FreeItem(Item: Pointer);
begin
 Dispose(pModuleDef(Item));
end;

Procedure AssignConToCrt;
var hType,hAttr : Longint;
begin
 Move(Input, StdIn, sizeOf(StdIn));
 Move(Output, StdOut, sizeOf(StdOut));
{$IFDEF LINUX}
 if isatty(0) = 0 then RedirInput  := True;
 if isatty(1) = 0 then RedirOutput := True;
{$ENDIF}
{$IFDEF WIN32}
 hAttr := GetFileType(GetStdHandle(STD_INPUT_HANDLE));
 if hAttr <> FILE_TYPE_CHAR then RedirInput := True;
 hAttr := GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
 if hAttr <> FILE_TYPE_CHAR then RedirOutput := True;
{$ENDIF}
{$IFDEF OS2}
 DosQueryHType(0, hType, hAttr);
 if (hType and 3 = 1) and (hAttr and 1 <> 0)
  then begin
{$ifndef fpc}
        AssignCrt(Input);
        Reset(Input);
{$endif}
       end
  else RedirInput := True;
 DosQueryHType(1, hType, hAttr);
 if (hType and 3 = 1) and (hAttr and 2 <> 0)
  then begin
{$ifndef fpc}
        AssignCrt(Output);
        ReWrite(Output);
{$endif}
       end
  else RedirOutput := True;
{$ENDIF}
{$IFDEF nonexxx}
  AssignCrt(Input);  Reset(Input);
  AssignCrt(Output); ReWrite(Output);
{$ENDIF}
end;

begin
  AssignConToCrt;
end.
