(****************************************************************************)
(*  Title:       os2exe.pas                                                 *)
(*  Description: OS/2 executables handling object: supports loading of      *)
(*               both NE and LX files; supports writing of LX files only    *)
(*                                                                          *)
(*        Copyright (c) FRIENDS software, 1996   No Rights Reserved         *)
(****************************************************************************)
{&G3+,R-}
{$ifndef fpc}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-,Use32+}
{$else}
{$Align 1}
{$asmmode intel}
{$Optimization STACKFRAME}
{$endif}
Unit os2exe;

Interface uses exe286, exe386, miscUtil, SysLib, Collect, Streams;

const
{ tLX object error codes }
 lxeOK            = 0;
 lxeReadError     = 1;
 lxeWriteError    = 2;
 lxeBadFormat     = 3;
 lxeBadRevision   = 4;
 lxeBadOrdering   = 5;
 lxeInvalidCPU    = 6;
 lxeBadOS         = 7;
 lxeNotLoadable   = 8;        {Module is not loadable}
 lxeUnkEntBundle  = 9;        {Unknown entry bundle type}
 lxeUnkPageFlags  = 10;       {Unknown page flags}
 lxeInvalidPage   = 11;       {PageSize > 0 and Page is nil}
 lxeNoMemory      = 12;
 lxeInvalidStub   = 13;
 lxeEAreadError   = 14;
 lxeEAwriteError  = 15;
 lxeIsNEformat    = 16;       {File is in `new` exe format}
 lxeIsLXformat    = 17;       {File cannot be loaded via ^.LoadNE}
 lxeBadFixupTable = 18;       {Invalid record in fixup table encountered}
 lxeBoundApp      = 19;       {NE file will lose functionality (is bound)}
 lxeNoLongFnames  = 20;       {NE does not support long file names}
 lxeIncompatNEseg = 21;       {NE contains segments with incompatible flags}
 lxeBadSegment    = 22;       {NE contains an invalid segment definition}
 lxeResourcesInNE = 23;       {NE contains resources (see lneIgnoreRsrc)}

{ tLX.LoadNE flags definition }
 lneIgnoreBound   = $01;      { OK to convert bound executables }
 lneIgnoreLngName = $02;      { OK to convert apps not aware of long fnames }
 lneIgnoreRsrc    = $04;      { OK to convert apps with resources }
                              { (Dos16GetResource does not work for LX) }

{ tLX.Save flags definition }
 svfAlignFirstObj = $00000003;{First object alignment AND mask}
 svfFOalnShift    = $00000000;{Align 1st object on lxPageShift bound}
 svfFOalnNone     = $00000001;{Do not align 1st object at all}
 svfFOalnSector   = $00000002;{Align 1st object on sector bound}
 svfAlignEachObj  = $0000000C;{Other objects alignment AND mask}
 svfEOalnShift    = $00000000;{Align objects on lxPageShift bound}
 svfEOalnSector   = $00000008;{Align objects on sector bound}

 NoRealign        = $FF;      {No realignment is requested}

{ tLX.Pack flags definistion }
 pkfRunLengthLvl  = $00000003;{Run-length pack method mask}
 pkfRunLengthMin  = $00000000;{Find only 1-length repeated data}
 pkfRunLengthMid  = $00000001;{Find data patterns up to 16 chars length}
 pkfRunLengthMax  = $00000002;{Find ALL matching data (VERY SLOW!)}
 pkfFixupsLvl     = $00000030;{Fixups packing method mask}
 pkfFixupsVer2    = $00000000;{Pack fixups with an 2.x compatible method}
 pkfFixupsVer4    = $00000010;{Pack fixups with an 4.x (and 3.x?) compatible method}
 pkfFixupsMax     = $00000030;{Pack fixups at maximum possible level(slow!)}
 pkfRunLength     = $01000000;{Pack using run-length packing}
 pkfLempelZiv     = $02000000;{Pack using kinda Lempel-Ziv(WARP ONLY!)}
 pkfFixups        = $04000000;{Pack fixups}
 pkfSixPack       = $08000000;
type
 pFixupCollection = ^tFixupCollection;
 tFixupCollection = object(tCollection)
  procedure FreeItem(Item: Pointer); virtual;
  function  GetItem(var S : tStream) : Pointer; virtual;
  procedure PutItem(var S : tStream; Item : Pointer); virtual;
 end;

 pEntryPoint = ^tEntryPoint;
 tEntryPoint = record
  Ordinal : Word;                     { Bundle ordinal }
  BndType : Byte;                     { Bundle type }
  Obj     : Word16;                   { object number }
  Entry   : tLXentryPoint;
 end;
{ Collection of entry points }
 pEntryCollection = ^tEntryCollection;
 tEntryCollection = object(tCollection)
  procedure FreeItem(Item: Pointer); virtual;
 end;

 pNamedEntryCollection = ^tNamedEntryCollection;
 tNamedEntryCollection = object(tSortedCollection)
  procedure FreeItem(Item: Pointer); virtual;
  function  Compare(Key1, Key2 : Pointer) : Integer; virtual;
 end;

 pArrOfOT = ^tArrOfOT;
 tArrOfOT = array[1..999] of tObjTblRec;
 pArrOfOM = ^tArrOfOM;
 tArrOfOM = array[1..999] of tObjMapRec;
 pArrOfRS = ^tArrOfRS;
 tArrOfRS = array[1..999] of tResource;
 pArrOfMD = ^tArrOfMD;
 tArrOfMD = array[1..999] of tDirTabRec;
 tProgressFunc = function(Current,Max : Longint) : boolean;
 pLX = ^tLX;
 tLX = object(tObject)
  Stub        : pByteArray;
  StubSize    : Longint;
  TimeStamp   : Longint;
{$ifndef FPC}
  FileAttr    : Longint;
{$else}
  FileAttr    : Smallword;
{$endif}
  Header      : tLXheader;
  ObjTable    : pArrOfOT;
  ObjMap      : pArrOfOM;
  RsrcTable   : pArrOfRS;
  ResNameTbl  : pNamedEntryCollection;
  NResNameTbl : pNamedEntryCollection;
  EntryTbl    : pEntryCollection;
  ModDirTbl   : pArrOfMD;
  PerPageCRC  : pLongArray;
  FixRecSize  : pLongArray;
  FixRecTbl   : pPointerArray;
  ImpModTbl   : pStringCollection;
  ImpProcTbl  : pStringCollection;
  Pages       : pPointerArray;
  PageOrder   : pLongArray;
  DebugInfo   : pByteArray;
  Overlay     : pByteArray;
  OverlaySize : Longint;
{$IFDEF OS2}
  EA          : pEAcollection;
{$ENDIF}
  pageToEnlarge: LongInt ; { 2011-11-16 SHL added }
  constructor Create;
  procedure   Initialize; virtual;
  function    LoadLX(const fName : string; pageToEnlarge_ : LongInt) : Byte;    { 2011-11-16 SHL  }
  function    LoadNE(const fName : string; loadFlags : byte) : Byte;
  function    Save(const fName : string; saveFlags : Longint) : Byte;
  procedure   FreeModule;
 {Unpack a single page}
  function    UnpackPage(PageNo : Integer; AllowTrunc:boolean) : boolean;
  function    UnpackPageNoTouch(PageNo : Integer; var UnpPageSize:longint) : pointer;
  procedure   Unpack(AllowTrunc:boolean);
  procedure   Pack(packFlags : longint; Progress : tProgressFunc; AllowTrunc:boolean);
  function    BundleRecSize(BndType : Byte) : Longint;
  function    SetFixups(PageNo : Longint; Fixups : pFixupCollection) : boolean;
  function    FixupsSize(Fixups : pFixupCollection) : longint;
 {GetFixups needs unpacked page if v4.x chained fixups are used}
  function    GetFixups(PageNo : Longint; Fixups : pFixupCollection) : boolean;
 {PackFixups() will unpack all pages if pkfFixupsVer4 is used}
  procedure   PackFixups(packFlags : longint);
  procedure   ApplyFixups(ForceApply : boolean; ApplyMask : byte);
  procedure   DeletePage(PageNo : Longint);
  procedure   MinimizePage(PageNo : Longint);
  function    UsedPage(PageNo : Longint) : boolean;
  procedure   RemoveEmptyPages(AllowMinimize:boolean);
  function    isPacked(newAlign,newStubSize,packFlags,saveFlags,oldDbgOfs : longint;
               var NewSize : longint) : boolean;
  destructor  Destroy;virtual;
 end;

Implementation uses Dos, {$IFDEF OS2}{$ifndef fpc}os2base, vpsyslow, {$else}doscalls, {$endif}{$ENDIF} pack2;

{$ifndef DOS}
function MemAvail : longint;
begin
 MemAvail := High(longint);
end;
{$endif}

procedure tFixupCollection.FreeItem;
begin
 with pLXreloc(Item)^ do
  if (sType and nrChain <> 0) and (targetCount > 0)
   then FreeMem(targets, targetCount * sizeOf(Word16));
 Dispose(pLXreloc(Item));
end;

function tFixupCollection.GetItem(var S : tStream) : Pointer;
var
 Fx : pLXreloc;
begin
 New(Fx);
 S.Get(Fx^, sizeOf(tLXreloc));
 with pLXreloc(Fx)^ do
  if (sType and nrChain <> 0) and (targetCount > 0)
   then begin
         GetMem(targets, targetCount * sizeOf(Word16));
         S.Get(targets^, targetCount * sizeOf(Word16));
        end;
 GetItem := Fx;
end;

procedure tFixupCollection.PutItem(var S : tStream; Item : Pointer);
begin
 with pLXreloc(Item)^ do
  begin
   S.Put(Item^, sizeOf(tLXreloc));
   if (sType and nrChain <> 0) and (targetCount > 0)
    then S.Put(targets^, targetCount * sizeOf(Word16));
  end;
end;

procedure tEntryCollection.FreeItem(Item: Pointer);
begin
 Dispose(pEntryPoint(Item));
end;

procedure tNamedEntryCollection.FreeItem(Item: Pointer);
begin
 DisposeStr(pNameTblRec(Item)^.Name);
 Dispose(pNameTblRec(Item));
end;

function tNamedEntryCollection.Compare(Key1, Key2 : Pointer) : Integer;
begin
 if pNameTblRec(Key1)^.Ord > pNameTblRec(Key2)^.Ord
  then Compare := +1
  else
 if pNameTblRec(Key1)^.Ord < pNameTblRec(Key2)^.Ord
  then Compare := -1
  else Compare := 0;
end;

{*************************** Pack/Unpack procedures *************************}

Function UnpackMethod1(var srcData, destData; srcDataSize : Longint;
                       var dstDataSize : longint) : boolean;
var
 src     : tByteArray absolute srcData;
 dst     : tByteArray absolute destData;
 sOf,dOf : Longint;
 nI,cB   : Word16;

Function srcAvail(N : Longint) : boolean;
begin
 srcAvail := sOf + N <= srcDataSize;
end;

Function dstAvail(N : Longint) : boolean;
begin
 dstAvail := dOf + N <= dstDataSize;
end;

begin
 UnpackMethod1 := FALSE;
 sOf := 0; dOf := 0;
 repeat
  if not srcAvail(1) then break;
  if not srcAvail(2) then exit;
  nI := pWord16(@src[sOf])^; Inc(sOf, 2);
  if nI = 0 then break;
  if not srcAvail(2) then exit;
  cB := pWord16(@src[sOf])^; Inc(sOf, 2);
  if srcAvail(cB) and dstAvail(cB * nI)
   then if nI > 0
         then begin
               linearMove(src[sOf], dst[dOf], cB);
               linearMove(dst[dOf], dst[dOf + cB], cB * pred(nI));
               Inc(dOf, cB * nI);
              end
         else
   else exit;
  Inc(sOf, cB);
 until dOf >= dstDataSize;
 FillChar(dst[dOf], dstDataSize - dOf, 0);
 dstDataSize := dOf;
 UnpackMethod1 := TRUE;
end;

Function UnpackMethod2(var srcData, destData; srcDataSize : Longint;
                       var dstDataSize : Longint) : boolean;
var
 src   : tByteArray absolute srcData;
 dst   : tByteArray absolute destData;
 B1,B2 : Byte;
 sOf,dOf,
 bOf   : Longint;

Function srcAvail(N : Longint) : boolean;
begin
 srcAvail := sOf + N <= srcDataSize;
end;

Function dstAvail(N : Longint) : boolean;
begin
 dstAvail := dOf + N <= dstDataSize;
end;

begin
 UnpackMethod2 := FALSE;
 sOf := 0; dOf := 0;
 repeat
  if not srcAvail(1) then break;
  B1 := src[sOf];
  case B1 and 3 of
   0 : if B1 = 0
        then if srcAvail(2)
              then if src[succ(sOf)] = 0
                    then begin Inc(sOf, 2); break; end
                    else if srcAvail(3) and dstAvail(src[succ(sOf)])
                          then begin
                                FillChar(dst[dOf], src[succ(sOf)], src[sOf+2]);
                                Inc(sOf, 3); Inc(dOf, src[sOf-2]);
                               end
                          else exit
              else exit
        else if srcAvail(succ(B1 shr 2)) and dstAvail(B1 shr 2)
              then begin
                    linearMove(src[succ(sOf)], dst[dOf], B1 shr 2);
                    Inc(dOf, B1 shr 2);
                    Inc(sOf, succ(B1 shr 2));
                   end
              else exit;
   1 : begin
        if not srcAvail(2) then exit;
        bOf := pWord16(@src[sOf])^ shr 7;
        B2 := (B1 shr 4) and 7 + 3;
        B1 := (B1 shr 2) and 3;
        Inc(sOf, 2);
        if srcAvail(B1) and dstAvail(B1 + B2) and (dOf + B1 - bOf >= 0)
        then begin
              linearMove(src[sOf], dst[dOf], B1);
              Inc(dOf, B1); Inc(sOf, B1);
              linearMove(dst[dOf - bOf], dst[dOf], B2);
              Inc(dOf, B2);
             end
         else exit;
       end;
   2 : begin
        if not srcAvail(2) then exit;
        bOf := pWord16(@src[sOf])^ shr 4;
        B1 := (B1 shr 2) and 3 + 3;
        if dstAvail(B1) and (dOf - bOf >= 0)
         then begin
               linearMove(dst[dOf - bOf], dst[dOf], B1);
               Inc(dOf, B1); Inc(sOf, 2);
              end
         else exit;
       end;
   3 : begin
        if not srcAvail(3) then exit;
        B2 := (pWord16(@src[sOf])^ shr 6) and $3F;
        B1 := (src[sOf] shr 2) and $0F;
        bOf := pWord16(@src[succ(sOf)])^ shr 4;
        Inc(sOf, 3);
        if srcAvail(B1) and dstAvail(B1 + B2) and (dOf + B1 - bOf >= 0)
         then begin
               linearMove(src[sOf], dst[dOf], B1);
               Inc(dOf, B1); Inc(sOf, B1);
               linearMove(dst[dOf - bOf], dst[dOf], B2);
               Inc(dOf, B2);
              end
         else exit;
       end;
  end;
 until dOf >= dstDataSize;
 FillChar(dst[dOf], dstDataSize - dOf, 0);
 dstDataSize := dOf;
 UnpackMethod2 := TRUE;
end;

Function UnpackMethod3(var srcData, destData; srcDataSize : Longint;
                       var dstDataSize : Longint) : boolean;
begin
  dstDataSize:=pack2.Decompress(srcDataSize,addr(srcData),addr(destData));
  UnpackMethod3 := TRUE;
end;

function PackMethod1(var srcData,dstData; srcDataSize : longint;
                     var dstDataSize : Longint; packLevel : byte) : boolean;
var
 sOf,dOf,tOf,
 MatchOff,
 MatchCnt,
 MatchLen : Longint;
 src      : tByteArray absolute srcData;
 dst      : tByteArray absolute dstData;

{$ifdef fpc}

function Search : boolean;
var
  src, src0: longint;
  dst, dst0: longint;
  x, y, z:   longint;
  cnt:       byte;

begin
    src := longint(@srcData) + sOf;
    dst := longint(@srcData) + tOf;

    z := 0;

    cnt := packLevel;

    if cnt <> 255 then
    begin
        x := dst - src;
        if x > cnt then z := x - cnt;
    end;

    MatchOff := z;
    src := src + z;

    repeat
        src0 := src;
        dst0 := dst;

        z := srcDataSize - tOf;
        x := dst - src;

        if z >= x then
        begin
            y := z div x;
            cnt := x;

            repeat
                if memcmp(src, dst, cnt) = 0 then break;
                z := z - 1;
            until z <> 0;

            if z <> y then
            begin
                z := y - z + 1;
                y := x;
                x := x * z - 4;

                if x >= 0 then
                begin
                    if x > cnt then
                    begin
                        MatchCnt := z;
                        MatchLen := cnt;

                        src := src0;
                        dst := dst0;

                        Search := true;
                    end
                end
            end 
        end;

        src := src0;
        dst := dst0;

        src := src + 1;
        MatchOff := MatchOff + 1;

    until src >= dst;

    Search := false;
end;

{$else}

{&uses ebx,esi,edi}
{ Trick: In FRAME- state BP register is not altered so we can }
{ address external data via [bp+XX]; however we must address }
{ it via var[bp][-4] because compiler thinks that BP is modified }
function Search : boolean; assembler;
asm             cld
                mov     esi,srcData
                mov     edi,esi
                add     edi,tOf[-4] {!!! and so on !!!}
                add     esi,sOf[-4]
                xor     eax,eax
                movzx   ecx,packLevel
                cmp     cl,255
                je      @@setStart
                mov     ebx,edi
                sub     ebx,esi
                cmp     ebx,ecx
                jbe     @@setStart
                mov     eax,ebx
                sub     eax,ecx
@@setStart:     mov     MatchOff[-4],eax
                add     esi,eax
@@nextPatt:     push    esi
                push    edi
                mov     eax,srcDataSize
                sub     eax,tOf[-4]
                mov     ebx,edi
                sub     ebx,esi
                cmp     ebx,eax
                ja      @@noMatch
                xor     edx,edx
                div     ebx
                mov     edx,eax                 {EDX = EAX = max matches}
@@nextMatch:    mov     ecx,ebx                 {EBX = ECX = pattern length}
                repe    cmpsb
                jne     @@notEQ
                dec     eax
                jnz     @@nextMatch
@@notEQ:        cmp     eax,edx
                je      @@noMatch
                sub     eax,edx
                neg     eax
                inc     eax                     {EAX = number of actual matches}
                mov     edx,ebx
                db      $0F,$AF,$D8             {imul    ebx,eax}
                sub     ebx,2+2
                jc      @@noMatch
                cmp     ebx,edx
                jbe     @@noMatch
                mov     MatchCnt[-4],eax
                mov     MatchLen[-4],edx
                pop     esi
                pop     edi
                mov     al,1
                jmp     @@locEx
@@noMatch:      pop     edi
                pop     esi
                inc     esi
                inc     MatchOff[-4]
                cmp     esi,edi
                jb      @@nextPatt
                mov     al,0
@@locEx:
end;
{&uses none}

{$endif}

function dstAvail(N : Longint) : boolean;
begin
 dstAvail := dOf + N <= dstDataSize;
end;

function PutNonpackedData : boolean;
begin
 PutNonpackedData := TRUE;
 if MatchOff > 0
  then if dstAvail(2+2+MatchOff)
        then begin
              pWord16(@dst[dOf])^ := 1; Inc(dOf, 2);
              pWord16(@dst[dOf])^ := MatchOff; Inc(dOf, 2);
              Move(src[sOf], dst[dOf], MatchOff);
              Inc(dOf, MatchOff); Inc(sOf, MatchOff);
             end
        else PutNonpackedData := FALSE;
end;

begin
 PackMethod1 := FALSE;
 sOf := 0; dOf := 0;
 repeat
  tOf := succ(sOf);
  While tOf < srcDataSize do
   begin
    if Search
     then begin
           if (not PutNonpackedData) or
              (not dstAvail(2+2+MatchLen)) then exit;
           pWord16(@dst[dOf])^ := MatchCnt; Inc(dOf, 2);
           pWord16(@dst[dOf])^ := MatchLen; Inc(dOf, 2);
           linearMove(src[sOf], dst[dOf], MatchLen);
           Inc(sOf, MatchCnt * MatchLen); Inc(dOf, MatchLen);
           break;
          end
     else Inc(tOf);
   end;
 until tOf >= srcDataSize;
 MatchOff := srcDataSize - sOf;
 if (not PutNonpackedData) or (sOf <= dOf)
  then exit;
 if not dstAvail(2) then exit;
 pWord16(@dst[dOf])^ := 0; Inc(dOf, 2);
 if (dOf >= $FFC) { OS2KRNL limit !!! }
  then exit;
 dstDataSize := dOf;
 PackMethod1 := TRUE;
end;

function PackMethod2(var srcData,dstData; srcDataSize : longint; var dstDataSize : Longint) : boolean;
label skip,locEx;
var
 Chain       : pWord16Array;
 ChainHead   : pWord16Array;
 sOf,dOf,tOf,I,J,
 maxMatchLen,
 maxMatchPos : Longint;
 src         : tByteArray absolute srcData;
 dst         : tByteArray absolute dstData;

{$ifdef fpc}

function Search : boolean;
label maxLen, endOfChain;
var
    src:           pWord16Array;
    dst:           longint;
    x, a, b, cnt:  longint;
    ret:           boolean;

begin
    x := srcDataSize - tOf;
    ret := false;

    if x > 2 then
    begin
        src := pWord16Array(longint(@srcData) + tOf);
        dst := longint(src);

        a := (src^[word(tOf)] and $0FFF) shl 1;
        a := (a + longint(@ChainHead)) and maxMatchLen;

        repeat
            repeat
                repeat
                    if ChainHead^[a] = -1 then goto endOfChain;

                    a := Chain^[ChainHead^[a] shl 1];
                    dst := longint(srcData);
                    cnt := x;
                    if memcmp(src, dst, cnt) = 0 then goto maxLen;
                    cnt := x - cnt;
                    x := x - cnt;
                    cnt := cnt - 1;
                until cnt > maxMatchLen;

                dst := dst - longint(srcData);
                maxMatchlen := cnt;
                maxMatchPos := dst;
                b := tOf - 1;

            until b > dst;
        until cnt > 63;

        goto endOfChain;

maxLen:
        dst := dst - x - longint(srcData);
        maxMatchLen := x;
        maxMatchPos := dst;

endOfChain:
        ret := false;

        if maxMatchLen >= 3 then ret := true;
    end;

    Search := ret;
end;

{$else}

{&uses esi,edi,ebx}
function Search : boolean; assembler;
asm             cld
                mov     edx,srcDataSize
                sub     edx,tOf[-4]
                mov     al,0
                cmp     edx,2
                jbe     @@locEx
                mov     esi,srcData
                mov     edi,esi
                add     esi,tOf[-4]
                mov     ax,[esi]
                and     eax,0FFFh
                shl     eax,1
                add     eax,ChainHead[-4]
                and     maxMatchLen[-4],0

@@nextSearch:   push    esi
                movsx   edi,word ptr [eax]
                cmp     edi,-1
                je      @@endOfChain
                mov     eax,edi
                shl     eax,1
                add     eax,Chain[-4]
                add     edi,srcData
                mov     ecx,edx
                repe    cmpsb
                jz      @@maxLen
                pop     esi
                sub     ecx,edx
                neg     ecx
                sub     edi,ecx
                dec     ecx
                cmp     ecx,maxMatchLen[-4]
                jbe     @@nextSearch
                sub     edi,srcData
                mov     maxMatchLen[-4],ecx
                mov     maxMatchPos[-4],edi
                mov     ebx,tOf[-4]
                dec     ebx
                cmp     ebx,edi                 {Prefer RL encoding since it}
                jne     @@nextSearch            {packs longer strings}
                cmp     ecx,63                  {Strings up to 63 chars are always}
                jbe     @@nextSearch            {packed effectively enough}
                push    esi
                jmp     @@endOfChain

@@maxLen:       sub     edi,edx
                sub     edi,srcData
                mov     maxMatchLen[-4],edx
                mov     maxMatchPos[-4],edi

@@endOfChain:   mov     al,0
                cmp     maxMatchLen[-4],3
                jb      @@noMatch
                inc     al
@@noMatch:      pop     esi
@@locEx:
end;
{&uses none}

{$endif}

function dstAvail(N : Longint) : boolean;
begin
 dstAvail := dOf + N <= dstDataSize;
end;

procedure Register(sOf, Count : Longint);
var
 I : Longint;
begin
 While (Count > 0) and (sOf < pred(srcDataSize)) do
  begin
   I := pWord16(@src[sOf])^ and $FFF;
   Chain^[sOf] := ChainHead^[I];
   ChainHead^[I] := sOf;
   Inc(sOf); Dec(Count);
  end;
end;

procedure Deregister(sOf : Longint);
var
 I : Longint;
begin
 I := pWord16(@src[sOf])^ and $FFF;
 ChainHead^[I] := Chain^[sOf];
end;

begin
 PackMethod2 := FALSE;
 GetMem(Chain, srcDataSize * 2);
 GetMem(ChainHead, (1 shl 12) * 2);
 FillChar(ChainHead^, (1 shl 12) * 2, $FF);
 sOf := 0; dOf := 0;
 repeat
  tOf := sOf;
  while tOf < srcDataSize do
   if Search
    then begin
          if (maxMatchPos = pred(tOf))
           then begin
                 if tOf > sOf then
                  begin
                   Inc(maxMatchLen);
                   Dec(tOf); Deregister(tOf);
                  end;
                 if maxMatchLen = 3 then goto skip;
                 while sOf < tOf do
                  begin
                   I := MinL(tOf - sOf, 63);
                   if not dstAvail(succ(I)) then goto locEx;
                   dst[dOf] := I shl 2;
                   linearMove(src[sOf], dst[succ(dOf)], I);
                   Inc(sOf, I); Inc(dOf, succ(I));
                  end;
                 while maxMatchLen > 3 do
                  begin
                   if not dstAvail(3) then goto locEx;
                   I := MinL(maxMatchLen, 255);
                   dst[dOf] := 0;
                   dst[dOf+1] := I;
                   dst[dOf+2] := src[sOf];
                   Register(sOf, I);
                   Inc(sOf, I); Inc(dOf, 3);
                   Dec(maxMatchLen, I);
                  end;
                end
           else begin
                 if (tOf - maxMatchPos < 512) and (maxMatchLen <= 10)
                  then J := 3
                  else
                 if (maxMatchLen <= 6) then J := 0 else J := 15;
                 while (sOf < tOf - J) do
                  begin
                   I := MinL(tOf - sOf, 63);
                   if not dstAvail(succ(I)) then goto locEx;
                   dst[dOf] := I shl 2;
                   linearMove(src[sOf], dst[succ(dOf)], I);
                   Inc(sOf, I); Inc(dOf, succ(I));
                  end;
                 case byte(J) of
                  3  : begin
                        if not dstAvail(2 + tOf - sOf) then goto locEx;
                        pWord16(@dst[dOf])^ := 1 + (tOf - sOf) shl 2 + (maxMatchLen - 3) shl 4 + (tOf - maxMatchPos) shl 7;
                        linearMove(src[sOf], dst[dOf + 2], tOf - sOf);
                        Register(tOf, maxMatchLen);
                        Inc(dOf, 2 + tOf - sOf);
                        sOf := tOf + maxMatchLen;
                       end;
                  0  : begin
                        if not dstAvail(2) then goto locEx;
                        pWord16(@dst[dOf])^ := 2 + (maxMatchLen - 3) shl 2 + (tOf - maxMatchPos) shl 4;
                        Register(tOf, maxMatchLen);
                        Inc(dOf, 2);
                        sOf := tOf + maxMatchLen;
                       end;
                  15 : begin
                        if not dstAvail(3 + tOf - sOf) then goto locEx;
                        J := MinL(maxMatchLen, 63);
                        pWord16(@dst[dOf])^ := 3 + (tOf - sOf) shl 2 + (J shl 6) + (tOf - maxMatchPos) shl 12;
                        dst[dOf + 2] := (tOf - maxMatchPos) shr 4;
                        linearMove(src[sOf], dst[dOf + 3], tOf - sOf);
                        Register(tOf, J);
                        Inc(dOf, 3 + tOf - sOf);
                        sOf := tOf + J;
                       end;
                 end;
                end;
          break;
         end
    else begin
skip:     Register(tOf, 1);
          Inc(tOf);
         end;
 until tOf >= srcDataSize;
 if not dstAvail(srcDataSize - sOf + 2) then goto locEx;
 while sOf < srcDataSize do
  begin
   I := MinL(srcDataSize - sOf, 63);
   if not dstAvail(succ(I)) then goto locEx;
   dst[dOf] := I shl 2;
   linearMove(src[sOf], dst[succ(dOf)], I);
   Inc(sOf, I); Inc(dOf, succ(I));
  end;
 pWord16(@dst[dOf])^ := 0; Inc(dOf, 2);                 {Put end-of-page flag}
 if (dOf >= srcDataSize) or (dOf >= $FFC)                { OS2KRNL limit !!! }
  then goto locEx;
 PackMethod2 := TRUE;
 dstDataSize := dOf;
locEx:
 FreeMem(ChainHead, (1 shl 12) * 2);
 FreeMem(Chain, srcDataSize * 2);
end;

function PackMethod3(var srcData,dstData; srcDataSize : longint; var dstDataSize : Longint) : boolean;
var dst:pointer;
    len:longint;
begin
 GetMem(dst, srcDataSize * 3);
 len:=pack2.Compress(srcDataSize,@srcData,dst);
 PackMethod3:=len<$FFC;
 if len<$FFC then
 begin
   move(dst^,dstData,len);
   dstDataSize:=len;
 end;
 FreeMem(dst, srcDataSize * 3);
end;


{********************* LX executable object implementation ******************}

constructor tLX.Create;
begin
 Initialize;
end;

procedure tLX.Initialize;
begin
 Zero;
 Header.lxMagicID := lxMagic;
{Header.lxBOrder := lxLEBO;}
{Header.lxWOrder := lxLEWO;}
{Header.lxLevel := 0;}             {commented out since they`re already zeros}
 Header.lxCpu := lxCPU386;
 Header.lxOS := 1;
 Header.lxPageShift := 2;
 Header.lxPageSize := lx386PageSize;
end;

{* Two utility procedures for the QuickSort routine: *}
{* compare two pages and exchange two pages (below). *}
Function lxCmpPages(var Buff; N1,N2 : longint) : boolean;
var
 L1,L2 : Longint;
begin
 lxCmpPages := TRUE;
 with tLX(Buff) do
  begin
   with ObjMap^[PageOrder^[N1]] do
    case PageFlags of
     pgValid     : L1 := Header.lxDataPageOfs + PageDataOffset shl Header.lxPageShift;
     pgIterData,
     pgIterData2,
     pgIterData3 : L1 := Header.lxIterMapOfs + PageDataOffset shl Header.lxPageShift;
     pgInvalid,
     pgZeroed    : L1 := $7FFFFFFF;
    end;
   with ObjMap^[PageOrder^[N2]] do
    case PageFlags of
     pgValid     : L2 := Header.lxDataPageOfs + PageDataOffset shl Header.lxPageShift;
     pgIterData,
     pgIterData2,
     pgIterData3 : L2 := Header.lxIterMapOfs + PageDataOffset shl Header.lxPageShift;
     pgInvalid,
     pgZeroed    : L2 := $7FFFFFFF;
    end;
   if (L1 >= L2) or ((L1 = L2) and (N1 >= N2)) then exit;
  end;
 lxCmpPages := FALSE;
end;

Procedure lxXchgPages(var Buff; N1,N2 : longint);
begin
 with tLX(Buff) do
  XchgL(PageOrder^[N1], PageOrder^[N2]);
end;

function    tLX.LoadLX(const fName : string; pageToEnlarge_ : LongInt) : Byte;
label locEx;
var
 F       : File;
 fSz,lastData,
 I,J,L,M : Longint;
 S       : String;
 NTR     : pNameTblRec;
 ETR     : tEntryTblRec;
 EP      : pEntryPoint;
 Res     : Byte;
 tmpBuff : pByteArray;

Procedure UpdateLast;
var
 A : Longint;
begin
 A := FilePos(F);
 if (lastData < A)
  then if (A <= fSz)
        then lastData := A
        else lastData := fSz;
end;

begin
 freeModule;
 Res := lxeReadError;
 pageToEnlarge := pageToEnlarge_; { 2011-11-16 SHL }
 Assign(F, fName);
{$IFDEF OS2}
 New(EA, Fetch(fName));
 if EA = nil then begin Res := lxeEAreadError; GoTo locEx; end;
{$ENDIF}
 I := FileMode; FileMode := open_share_DenyWrite;
 GetFAttr(F, FileAttr); Reset(F, 1); FileMode := I;
 if inOutRes <> 0 then GoTo locEx;
 Res := lxeBadFormat;
 L := 0; lastData := 0;
 fSz := FileSize(F);
 GetFTime(F, TimeStamp);
 repeat
  if (fSz - FilePos(F)) < sizeOf(Header) then GoTo locEx;
  FillChar(Header, sizeOf(Header), 0);
  BlockRead(F, Header, sizeOf(Header));
  if inOutRes <> 0 then GoTo locEx;
  case Header.lxMagicID of
   lxMagic   : break;
   neMagic   : begin Res := lxeIsNEformat; GoTo locEx; end;
   exeMagic1,
   exeMagic2 : begin
                if pLongArray(@header)^[$0F] <= L then GoTo locEx;
                L := pLongArray(@header)^[$0F];
                if L > fSz - sizeOf(Header) then GoTo locEx;
                Seek(F, L); {Skip DOS stub}
               end;
   else GoTo locEx;
  end;
 until FALSE;
 if (Header.lxBOrder <> lxLEBO) or (Header.lxWOrder <> lxLEBO)
  then begin Res := lxeBadOrdering; GoTo locEx; end;
 if (Header.lxCPU < lxCPU286) or (Header.lxCPU > lxCPUP5)
  then begin Res := lxeInvalidCPU; GoTo locEx; end;
 if (Header.lxLevel <> 0)
  then begin Res := lxeBadRevision; GoTo locEx; end;
 if (Header.lxOS <> 1)  {Not for OS/2}
  then begin Res := lxeBadOS; GoTo locEx; end;
 if (Header.lxMFlags and lxNoLoad <> 0)
  then begin Res := lxeNotLoadable; GoTo locEx; end;
 if (Header.lxPageSize <> lx386PageSize)
  then begin Res := lxeBadFormat; GoTo locEx; end;

{ Read in DOS stub }
 stubSize := L; Seek(F, 0);
 GetMem(Stub, stubSize);
 BlockRead(F, Stub^, stubSize);
 updateLast;

{ Read Object Table }
 if (Header.lxObjTabOfs <> 0) and (Header.lxObjCnt <> 0) and (StubSize + Header.lxObjTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxObjTabOfs);
        GetMem(ObjTable, Header.lxObjCnt * sizeOf(tObjTblRec));
        BlockRead(F, ObjTable^, Header.lxObjCnt * sizeOf(tObjTblRec));
        updateLast;
        J := 0;
        For I := 1 to Header.lxObjCnt do
         with ObjTable^[I] do
          begin
           { 2011-11-16 SHL expand object containing /X:pagenum page to max size
             fixme to complain if not last page in object?
           }
           if (pageToEnlarge > 0) and (pageToEnlarge >= oPageMap) and (pageToEnlarge < oPageMap + oMapSize) then
             oSize := (oSize + (Header.lxPageSize - 1)) and not (Header.lxPageSize - 1);
           L := pred(oPageMap + oMapSize);
           if L > J then J := L;
          end;
        if Header.lxMPages > J  { Fix for some poorly-constructed executables }
         then Header.lxMPages := J;
       end
  else begin
        Header.lxObjTabOfs := 0;
        Header.lxObjCnt := 0;
       end;

{ Read Object Page Map Table }
 if (Header.lxObjTabOfs <> 0) and (Header.lxMpages <> 0) and (StubSize + Header.lxObjTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxObjMapOfs);
        GetMem(ObjMap, Header.lxMpages * sizeOf(tObjMapRec));
        BlockRead(F, ObjMap^, Header.lxMpages * sizeOf(tObjMapRec));
        updateLast;
       end
  else begin
        Header.lxObjMapOfs := 0;
        Header.lxMpages := 0;
       end;

 if (Header.lxRsrcTabOfs <> 0) and (Header.lxRsrcCnt <> 0) and (StubSize + Header.lxRsrcTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxRsrcTabOfs);
        GetMem(RsrcTable, Header.lxRsrcCnt * sizeOf(tResource));
        BlockRead(F, RsrcTable^, Header.lxRsrcCnt * sizeOf(tResource));
        updateLast;
       end
  else begin
        Header.lxRsrcTabOfs := 0;
        Header.lxRsrcCnt := 0;
       end;

 New(ResNameTbl, Create(16, 16));
 if (Header.lxResTabOfs <> 0) and (StubSize + Header.lxResTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxResTabOfs);
        repeat
         BlockRead(F, S, sizeOf(Byte));
         if (S='') or (FilePos(F)+length(S)+sizeOf(Word16) > fSz) then break;
         BlockRead(F, S[1], length(S));
         New(NTR);
         NTR^.Name := NewStr(S);
         BlockRead(F, NTR^.Ord, sizeOf(Word16));
         ResNameTbl^.Insert(NTR);
        until false;
        updateLast;
       end
  else Header.lxResTabOfs := 0;

 New(NResNameTbl, Create(16, 16));
 if (Header.lxNResTabOfs <> 0) and (Header.lxNResTabOfs < fSz)
  then begin
        Seek(F, Header.lxNResTabOfs);
        repeat
         BlockRead(F, S, sizeOf(Byte));
         if (S='') or (FilePos(F)+length(S)+sizeOf(Word16) - Header.lxNResTabOfs > Header.lxNResTabSize)
                   or (FilePos(F)+length(S)+sizeOf(Word16) > fSz) then break;
         BlockRead(F, S[1], length(S));
         New(NTR);
         NTR^.Name := NewStr(S);
         BlockRead(F, NTR^.Ord, sizeOf(Word16));
         NResNameTbl^.Insert(NTR);
        until false;
        updateLast;
       end
  else Header.lxNResTabOfs := 0;

 New(EntryTbl, Create(16, 16));
 if (Header.lxEntTabOfs <> 0) and (StubSize + Header.lxEntTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxEntTabOfs);
        M := 1;
        repeat
         ETR.Count := 0;
         BlockRead(F, ETR.Count, sizeOf(ETR.Count));
         if ETR.Count = 0 then break;
         BlockRead(F, ETR.BndType, sizeOf(ETR.BndType));
         L := BundleRecSize(ETR.BndType);
         if L = -1 then begin Res := lxeUnkEntBundle; GoTo locEx; end;
         if L <> 0 then BlockRead(F, ETR.Obj, sizeOf(ETR.Obj));
         if ETR.BndType = btEmpty
          then begin Inc(M, ETR.Count); Continue; end;
         J := ETR.Count * L;
         GetMem(tmpBuff, J);
         BlockRead(F, tmpBuff^, J);
         For I := 1 to ETR.Count do
          begin
           New(EP);
           EP^.Ordinal := M;
           EP^.BndType := ETR.BndType;
           EP^.Obj := ETR.Obj;
           FillChar(EP^.Entry, sizeOf(EP^.Entry), 0);
           Move(tmpBuff^[pred(I) * L], EP^.Entry, L);
           EntryTbl^.Insert(EP); Inc(M);
          end;
         FreeMem(tmpBuff, J);
        until inOutRes <> 0;
        updateLast;
       end
  else Header.lxEntTabOfs := 0;

 if (Header.lxDirTabOfs <> 0) and (Header.lxDirCnt <> 0) and (StubSize + Header.lxDirTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxDirTabOfs);
        GetMem(ModDirTbl, Header.lxDirCnt * sizeOf(tResource));
        BlockRead(F, ModDirTbl^, Header.lxDirCnt * sizeOf(tResource));
        updateLast;
       end
  else begin
        Header.lxDirTabOfs := 0;
        Header.lxDirCnt := 0;
       end;

 if (Header.lxPageSumOfs <> 0) and (StubSize + Header.lxPageSumOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxPageSumOfs);
        GetMem(PerPageCRC, Header.lxMpages * sizeOf(Longint));
        BlockRead(F, PerPageCRC^, Header.lxMpages * sizeOf(Longint));
        updateLast;
       end
  else Header.lxPageSumOfs := 0;

 if (Header.lxFPageTabOfs <> 0) and (StubSize + Header.lxFPageTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxFPageTabOfs);
        GetMem(FixRecSize, succ(Header.lxMpages) * sizeOf(Longint));
        BlockRead(F, FixRecSize^, succ(Header.lxMpages) * sizeOf(Longint));
        updateLast;
       end
  else begin
        Header.lxFPageTabOfs := 0;
        Res := lxeBadFixupTable;
        Goto locEx;
       end;

 For I := 1 to Header.lxMPages do
  if FixRecSize^[I] < FixRecSize^[pred(I)]
   then if FixRecSize^[I] = 0
         then FixRecSize^[I] := FixRecSize^[pred(I)]
         else begin
               FreeMem(FixRecSize, succ(Header.lxMpages) * sizeOf(Longint));
               FixRecSize := nil;
               Res := lxeBadFixupTable;
               Goto locEx;
              end;

 if (Header.lxFRecTabOfs <> 0) and (StubSize + Header.lxFRecTabOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxFRecTabOfs + FixRecSize^[0]);

      { convert fixup page offsets to sizes of individual fixups }
        For I := Header.lxMPages downto 1 do
         Dec(FixRecSize^[I], FixRecSize^[pred(I)]);
        GetMem(FixRecTbl, Header.lxMpages * sizeOf(Longint));
        Move(FixRecSize^[1], FixRecTbl^, Header.lxMpages * sizeOf(Longint));
        FreeMem(FixRecSize, succ(Header.lxMpages) * sizeOf(Longint));
        FixRecSize := Pointer(FixRecTbl);

        GetMem(FixRecTbl, Header.lxMpages * sizeOf(Longint));
        For I := 1 to Header.lxMpages do {Read fixups for each page}
         begin
          L := FixRecSize^[pred(I)];
          GetMem(FixRecTbl^[pred(I)], L);
          BlockRead(F, FixRecTbl^[pred(I)]^, L);
         end;
        updateLast;
       end
  else Header.lxFRecTabOfs := 0;                   {FixUp Record Table is absent}

 New(ImpModTbl, Create(16, 16));
 if (Header.lxImpModOfs <> 0) and (StubSize + Header.lxImpModOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxImpModOfs);
        For I := 1 to Header.lxImpModCnt do
         begin
          BlockRead(F, S, sizeOf(Byte));
          BlockRead(F, S[1], length(S));
          ImpModTbl^.AtInsert(ImpModTbl^.Count, NewStr(S));
         end;
        updateLast;
       end
  else Header.lxImpModOfs := 0;

 New(ImpProcTbl, Create(16, 16));
 if (Header.lxImpProcOfs <> 0) and (StubSize + Header.lxImpProcOfs < fSz)
  then begin
        Seek(F, StubSize + Header.lxImpProcOfs);
        I := Header.lxFPageTabOfs + Header.lxFixupSize - Header.lxImpProcOfs;
        While I > 0 do
         begin
          BlockRead(F, S, sizeOf(Byte));
          BlockRead(F, S[1], length(S));
          ImpProcTbl^.AtInsert(ImpProcTbl^.Count, NewStr(S));
          Dec(I, succ(length(S)));
         end;
        updateLast;
       end
  else Header.lxImpProcOfs := 0;

 If Header.lxIterMapOfs = 0 Then Header.lxIterMapOfs := Header.lxDataPageOfs;
 GetMem(Pages, Header.lxMpages * sizeOf(Pointer));
 FillChar(Pages^, Header.lxMpages * sizeOf(Pointer), 0);
 GetMem(PageOrder, Header.lxMpages * sizeOf(Longint));
 For I := 1 to Header.lxMpages do
  with ObjMap^[I] do
   begin
    PageOrder^[pred(I)] := I;
    case PageFlags of
     pgValid     : L := Header.lxDataPageOfs;
     pgIterData,
     pgIterData2,
     pgIterData3 : L := Header.lxIterMapOfs;
     pgInvalid,
     pgZeroed    : begin
                    PageDataOffset := 0;
                    L := -1;
                   end;
     else{pgRange} begin Res := lxeUnkPageFlags; GoTo locEx; end;
    end;
    if L <> -1
     then begin
           Inc(L, PageDataOffset shl Header.lxPageShift);
           if (L > fSz)
            then if UsedPage(I)
                  then goto locEx
                  else begin
                        PageSize := 0;
                        PageDataOffset := 0;
                        PageFlags := pgInvalid;
                       end
            else begin
                  Seek(F, L);
                  GetMem(Pages^[pred(I)], PageSize);
                  BlockRead(F, Pages^[pred(I)]^, PageSize);
                  updateLast;
                 end;
          end;
   end;
{ Now sort the pages in the order they come in the file }
 QuickSort(Self, 0, pred(Header.lxMpages), 0, lxCmpPages, lxXchgPages);

{ Fix for some cut-down executables (FASTECHO/2) }
 I := fSz - Header.lxDebugInfoOfs;
 if (I < Header.lxDebugLen)
  then begin
        Header.lxDebugLen := I;
        if Header.lxDebugLen = 0 then Header.lxDebugInfoOfs := 0;
       end;

 if (Header.lxDebugInfoOfs <> 0) and (Header.lxDebugInfoOfs < fSz)
  then begin
        Seek(F, Header.lxDebugInfoOfs);
        { 2012-12-01 SHL Avoid out of memory 203 }
        if (Header.lxDebugLen <= MemAvail - $10000) then begin
            GetMem(DebugInfo, Header.lxDebugLen);
            BlockRead(F, DebugInfo^, Header.lxDebugLen)
        end
        else begin
            Seek(F, Header.lxDebugInfoOfs + Header.lxDebugLen);
            Header.lxDebugInfoOfs := 0;
            Header.lxDebugLen := 0
        end;
        updateLast;
       end
  else begin
        Header.lxDebugInfoOfs := 0;
        Header.lxDebugLen := 0;
       end;

 if lastData < fSz
  then begin
        OverlaySize := fSz - lastData;
        GetMem(Overlay, OverlaySize);
        Seek(F, lastData);
        BlockRead(F, Overlay^, OverlaySize);
       end;

 if inOutRes <> 0 then GoTo locEx;

 Res := lxeOK;
locEx:
 if ioResult <> 0 then Res := lxeReadError;
 if Res <> lxeOK then freeModule;
 LoadLX := Res;
 Close(F); inOutRes := 0;
end;

{ Load file in `new` executable format and convert it on-the-fly into LX }
function tLX.LoadNE(const fName : string; loadFlags : byte) : Byte;
label
 locEx;
var
 F         : File;
 neHdr     : tNEheader;
 fSz,lastData,
 siz,buffPos,
 I,J,K,L,M : Longint;
 S         : String;
 NTR       : pNameTblRec;
 ETR       : tEntryTblRec;
 EP        : pEntryPoint;
 Res       : Byte;
 ImportP,
 ImportM   : pCollection;
 tmpBuff,
 Buff      : pByteArray;
 neSeg     : pNEseg;
 neRes     : tNEresource;
 ModRefTbl : pWord16Array;

Procedure UpdateLast;
var
 A : Longint;
begin
 A := FilePos(F);
 if (lastData < A)
  then if (A <= fSz)
        then lastData := A
        else lastData := fSz;
end;

function ConvertRelocations(ObjNo : Longint) : boolean;
var
 Count   : Word16;
 tmpB,
 tmpPtr  : pByteArray;
 I,J,K,L : Longint;
 Fixup   : array[0..15] of pFixupCollection;
 tmpF,
 tmpF1   : pLXreloc;
 pW      : pWord16;
 pB1,pB2 : pByte;

procedure AddTObjMod(O : Word16);
begin
 tmpF^.Flags := tmpF^.Flags or nr16objMod and (not nr8bitOrd);
 tmpF^.ObjMod := O;
end;

procedure locFree;
var
 I,J : Integer;
 P   : pByte;
begin
 FreeMem(tmpB, Count * 8);
 For I := 0 to 15 do
  if Fixup[I] <> nil
   then Dispose(Fixup[I], Destroy);
end;

function GetChain(Offs : Word) : Word;
var
 L : Word;
begin
 with ObjTable^[ObjNo], ObjMap^[oPageMap + J] do
  if Offs < pred(Header.lxPageSize)
   then if (pred(PageSize) < Offs)
         then L := 0
         else begin
               pW := @pByteArray(Pages^[pred(oPageMap + J)])^[Offs];
               if (pred(PageSize) = Offs)
                then begin
                      L := pByte(pW)^;
                      pByte(pW)^ := 0;
                     end
                else begin
                      L := pW^;
                      pW^ := 0;
                     end;
              end
   else begin
         if (pred(PageSize) < Offs)
          then L := 0
          else begin
                pB1 := @pByteArray(Pages^[pred(oPageMap + J)])^[Offs];
                L := pB1^; pB1^ := 0;
               end;
         if (ObjMap^[oPageMap + J + 1].PageSize > 0)
          then begin
                pB2 := @pByteArray(Pages^[oPageMap + J])^[0];
                L := L or Word(pB2^) * 256; pB2^ := 0;
               end;
        end;
 GetChain := L;
end;

begin
 ConvertRelocations := FALSE;
 Seek(F, neSeg^.Sector shl neHdr.neAlign + neSeg^.cbSeg);
 BlockRead(F, Count, sizeOf(Count));
 GetMem(tmpB, Count * 8);
 BlockRead(F, tmpB^, Count * 8);
 updateLast;
 For I := 0 to 15 do
  begin
   New(Fixup[I], Create(Count, 1));
   if Fixup[I] = nil then begin locFree; exit; end;
  end;
 TmpPtr := TmpB;
 For I := 1 to Count do
  begin
   if memScanFwd(TmpPtr^, 8, 0) < 8
    then with pNEreloc(TmpPtr)^ do
          begin
           if (sType and (not nerSType) <> 0) or
              (flags and (not (nerRTyp or nerAdd)) <> 0)
            then begin locFree; exit; end;
           New(tmpF);
           tmpF^.sType := sType or nrAlias;
           tmpF^.Flags := Flags;
           tmpF^.sOffs := sOff;
           case Flags and nerRTyp of
            nerRInt : if rel.segNo <> 255
                       then begin
                             AddTObjMod(rel.segNo);
                             tmpF^.Target.intRef := rel.Entry;
                            end
                       else begin
                             AddTObjMod(rel.Entry);
                             tmpF^.Flags := tmpF^.Flags and (not nerRTyp) or nrRent;
                            end;
            nerROrd : begin
                       AddTObjMod(rel.ModIndx);
                       tmpF^.Target.extRef.Ord := rel.Proc;
                      end;
            nerRNam : begin
                       K := 0;
                       For J := 1 to ImportM^.Count do
                        if Word16(ImportM^.At(pred(J))) = rel.ModIndx
                         then begin
                               AddTObjMod(J);
                               Inc(K); break;
                              end;
                       For J := 1 to ImportP^.Count do
                        if Word16(ImportP^.At(pred(J))) = rel.Proc
                         then begin
                               L := 0;
                               if J > 1
                                then repeat
                                      Dec(J);
                                      Inc(L, succ(length(pString(ImpProcTbl^.At(pred(J)))^)));
                                     until J <= 1;
                               tmpF^.Flags := tmpF^.Flags or nr32bitOff;
                               tmpF^.Target.extRef.Proc := L;
                               Inc(K); break;
                              end;
                       if K <> 2 then begin Dispose(tmpF); locFree; Exit; end;
                      end;
            nerROsf : begin
                    { Ignore OS fixups since they are not used in OS/2 - thanks Vallat! }
                       Dispose(tmpF);
                       tmpF := nil;
                      end;
           end;

         { handle fixup chains }
           if tmpF <> nil
            then repeat
                  case sType and nerSType of
                   nerSByte  : J := 1; { lo byte (8-bits)}
                   nerSSeg,            { 16-bit segment (16-bits) }
                   nerSoff   : J := 2; { 16-bit offset (16-bits) }
                   nerSPtr,            { 16:16 pointer (32-bits) }
                   nerOff32,           { 32-bit offset (32-bits) }
                   nerSOff32 : J := 4; { 32-bit self-relative offset (32-bits) }
                   nerPtr48  : J := 6; { 16:32 pointer (48-bits) }
                   else begin Dispose(tmpF); locFree; exit; end;
                  end;

                  K := pred(tmpF^.sOffs + J) div Header.lxPageSize;
                  J := tmpF^.sOffs div Header.lxPageSize;

                  Dec(tmpF^.sOffs, J * Header.lxPageSize);

                  if Flags and nerAdd <> 0 {Handle additive fixups}
                   then tmpF^.addFixup := GetChain(tmpF^.sOffs);

                  Fixup[J]^.Insert(tmpF);
                  if K <> J                        { If fixup crosses page boundary }
                   then begin
                         New(tmpF1);
                         Move(tmpF^, tmpF1^, sizeOf(tmpF1^));
                         Dec(tmpF1^.sOffs, Header.lxPageSize);
                         Fixup[K]^.Insert(tmpF1);
                        end;
                  if (sType and nerSType = nerSByte) or
                     (Flags and nerAdd <> 0)
                   then break;
                  L := GetChain(tmpF^.sOffs);
                  if L < $FFFF
                   then begin
                         New(tmpF1);
                         Move(tmpF^, tmpF1^, sizeOf(tmpF1^));
                         tmpF := tmpF1;
                         tmpF^.sOffs := L;
                        end
                   else break;
                 until FALSE;
          end;
   Inc(word(TmpPtr), 8);
  end;
 with ObjTable^[ObjNo] do
  For I := 0 to pred(oMapSize) do
   if not SetFixups(oPageMap + I, Fixup[I])
    then begin locFree; exit; end;
 locFree;
 ConvertRelocations := TRUE;
end;

begin
 freeModule;
 Res := lxeReadError;
 Assign(F, fName);
{$IFDEF OS2}
 New(EA, Fetch(fName));
 if EA = nil then begin Res := lxeEAreadError; GoTo locEx; end;
{$ENDIF}
 I := FileMode; FileMode := open_share_DenyWrite;
 GetFAttr(F, FileAttr); Reset(F, 1); FileMode := I;
 if inOutRes <> 0 then GoTo locEx;
 Res := lxeBadFormat;
 L := 0; lastData := 0;
 fSz := FileSize(F);
 GetFTime(F, TimeStamp);
 repeat
  FillChar(neHdr, sizeOf(neHdr), 0);
  BlockRead(F, neHdr, sizeOf(neHdr));
  if inOutRes <> 0 then GoTo locEx;
  case neHdr.neMagic of
   lxMagic   : begin Res := lxeIsLXformat; GoTo locEx; end;
   neMagic   : break;
   exeMagic1,
   exeMagic2 : begin
                if pLongArray(@neHdr)^[$0F] <= L then GoTo locEx;
                L := pLongArray(@neHdr)^[$0F];
                if L > fSz - sizeOf(neHdr) then GoTo locEx;
                Seek(F, L); {Skip DOS stub}
               end;
   else GoTo locEx;
  end;
 until FALSE;
 if (not (neHdr.neExeTyp in [neUnknown,neOS2]))  {Not for OS/2}
  then begin Res := lxeBadOS; GoTo locEx; end;
 if (loadFlags and lneIgnoreBound = 0) and (neHdr.neFlags and neBound <> 0)
  then begin Res := lxeBoundApp; GoTo locEx; end;
 if (loadFlags and lneIgnoreLngName = 0) and (neHdr.neFlagsOthers and neLongFileNames = 0)
  then begin Res := lxeNoLongFnames; GoTo locEx; end;

{ Read in DOS stub }
 stubSize := L; Seek(F, 0);
 GetMem(Stub, stubSize);
 BlockRead(F, Stub^, stubSize);
 updateLast;

{ Convert header }
 with Header,neHdr do
  begin
   if neFlags and neI386 <> 0
    then lxCpu := lxCPU386
    else lxCpu := lxCPU286;
   lxMflags := neFlags and (neNotP + neIerr + neAppTyp + nePPLI);
   lxObjCnt := neCSeg;
   lxPageSize := lx386PageSize;
   lxPageShift := neAlign;
   lxRsrcCnt := neCRes;
   lxStartObj := neCSIP shr 16;         { Object # for instruction pointer }
   lxEIP := Word16(neCSIP);
   lxStackObj := neSSSP shr 16;         { Object # for stack pointer }
   lxESP := Word16(neSSSP);
   lxAutoData := neAutoData;            { Object # for automatic data object }
   lxHeapSize := neHeap;                { Size of heap - for 16-bit apps }
   lxStackSize := neStack;
  end;

 if (neHdr.neSegTab <> 0) and (stubSize + neHdr.neSegTab < fSz)
  then begin
        GetMem(neSeg, neHdr.neCSeg * sizeOf(tNEseg));
        Seek(F, stubSize + neHdr.neSegTab);
        BlockRead(F, neSeg^, neHdr.neCSeg * sizeOf(tNEseg));
        buff := Pointer(neSeg);
        UpdateLast;
       end
  else if neHdr.neCSeg <> 0 then GoTo locEx;

{ Convert NE segments into LX pages }
{ First count how many pages we`ll got }
 For I := 1 to neHdr.neCSeg do
  begin
   if (neSeg^.Flags and nesType > nesData) or
      (neSeg^.Flags and nesGDT <> 0) or
      (neSeg^.Flags and nesHuge <> 0)
    then begin
          FreeMem(buff, neHdr.neCSeg * sizeOf(tNEseg));
          Res := lxeIncompatNEseg;
          GoTo locEx;
         end;
   L := neSeg^.MinAlloc; if L = 0 then L := $10000;
   if (Header.lxESP = 0) and (Header.lxStackSize <> 0) and
      (I = Header.lxStackObj)
    then Inc(L, Header.lxStackSize);
   with Header do Inc(lxMpages, pred(L + lxPageSize) div lxPageSize);
   Inc(longint(neSeg), sizeOf(tNEseg));
  end;

 with Header do
 begin
  GetMem(FixRecSize, lxMpages * sizeOf(Longint));
  FillChar(FixRecSize^, lxMpages * sizeOf(Longint), 0);
  GetMem(FixRecTbl,  lxMpages * sizeOf(Pointer));
  FillChar(FixRecTbl^, lxMpages * sizeOf(Pointer), 0);

  GetMem(ObjTable,  lxObjCnt * sizeOf(tObjTblRec));
  GetMem(Pages,     lxMpages * sizeOf(Pointer));
  GetMem(PageOrder, lxMpages * sizeOf(Longint));
  GetMem(ObjMap,    lxMpages * sizeOf(tObjMapRec));
 end;

{ Set page order to sequential }
 For I := 1 to Header.lxMpages do PageOrder^[pred(I)] := I;

{ Now split segments into 4k pages }
 Pointer(neSeg) := buff; J := 0;
 For I := 1 to neHdr.neCSeg do
  begin
   with ObjTable^[I] do
    begin
     oSize := neSeg^.MinAlloc; if oSize = 0 then oSize := $10000;
     oBase := I * $10000;
     oPageMap := succ(J);
     oReserved := 0;
     oFlags := objAlias16 + objRead;
     L := neSeg^.Flags;
     if L and nesType = nesCode
      then oFlags := oFlags + objExec
      else oFlags := oFlags + objWrite;
     if L and nesShared <> 0
      then oFlags := oFlags + objShared;
     if L and nesPreload <> 0
      then oFlags := oFlags + objPreload;
     if L and nesExRdOnly <> 0
      then if oFlags and objExec <> 0
            then oFlags := oFlags and (not objRead)
            else oFlags := oFlags and (not objWrite);
     {Relocations will be converted later (see far below)}
     if L and nesConform <> 0
      then oFlags := oFlags or objConform;
     if L and nesDPL <> nesDPL
      then oFlags := oFlags or objIOPL;
     if L and nesDiscard <> 0
      then oFlags := oFlags or objDiscard;
     if L and nes32bit <> 0
      then oFlags := oFlags or objBigDef;
     siz := neSeg^.cbSeg;
     if (siz = 0) and (neSeg^.Sector <> 0) then siz := $10000;
     L := oSize;
     if neSeg^.Flags and nesIter <> 0
      then M := oSize + siz {size of temporary buffer}
      else M := siz;
     if (Header.lxESP = 0) and (Header.lxStackSize <> 0) and
        (I = Header.lxStackObj)
      then begin { Increment object size by stack size }
            Inc(L, Header.lxStackSize);
            Inc(oSize, Header.lxStackSize);
            Header.lxStackSize := 0;
            Header.lxESP := L;
           end;
     Seek(F, neSeg^.Sector shl neHdr.neAlign);
     GetMem(tmpBuff, M);
     BlockRead(F, tmpBuff^[M - siz], siz);
     updateLast;
     if neSeg^.Flags and nesIter <> 0
      then begin
            K := M - siz;
            if not UnpackMethod1(tmpBuff^[M - siz], tmpBuff^, siz, K)
             then begin
                   FreeMem(tmpBuff, M);
                   Res := lxeBadSegment;
                   Goto locEx;
                  end;
            siz := K;
           end;
     if siz > L
      then begin
            FreeMem(tmpBuff, M);
            Res := lxeBadSegment;
            Goto locEx;
           end;
    end;
   buffPos := 0;
   while L > 0 do
    begin
     if siz >= Header.lxPageSize then K := Header.lxPageSize else K := siz;
     with ObjMap^[succ(J)] do
      begin
       PageSize := K;
       PageFlags := pgValid;
       PageDataOffset := (neSeg^.Sector shl neHdr.neAlign) shr Header.lxPageShift;
      end;
     GetMem(Pages^[J], K);
     Move(tmpBuff^[buffPos], Pages^[J]^, K);
     Dec(siz, K); Inc(buffPos, K);
     if L > Header.lxPageSize then Dec(L, Header.lxPageSize) else L := 0;
     Inc(J);
    end;
   FreeMem(tmpBuff, M);
   with ObjTable^[I] do
    oMapSize := succ(J - oPageMap);
   Inc(longint(neSeg), sizeOf(tNEseg));
  end;

{ Convert resident name table }
 New(ResNameTbl, Create(16, 16));
 if (neHdr.neResTab <> 0) and (stubSize + neHdr.neResTab < fSz)
  then begin
        Seek(F, stubSize + neHdr.neResTab);
        repeat
         BlockRead(F, S, sizeOf(Byte));
         if (S='') or (FilePos(F)+length(S)+sizeOf(Word16) > fSz) then break;
         BlockRead(F, S[1], length(S));
         New(NTR);
         NTR^.Name := NewStr(S);
         BlockRead(F, NTR^.Ord, sizeOf(Word16));
         ResNameTbl^.Insert(NTR);
        until false;
        updateLast;
       end;

{ Convert non-resident name table }
 New(NResNameTbl, Create(16, 16));
 if (neHdr.neNResTab <> 0) and (neHdr.neNResTab < fSz)
  then begin
        Seek(F, neHdr.neNResTab);
        repeat
         BlockRead(F, S, sizeOf(Byte));
         if (S='') or (FilePos(F)+length(S)+sizeOf(Word16) > fSz)
                   or (FilePos(F)+length(S)+sizeOf(Word16)-neHdr.neNResTab > neHdr.neCbNResTab) then break;
         BlockRead(F, S[1], length(S));
         New(NTR);
         NTR^.Name := NewStr(S);
         BlockRead(F, NTR^.Ord, sizeOf(Word16));
         NResNameTbl^.Insert(NTR);
        until false;
        updateLast;
       end;

{ Convert Entry Table }
 New(EntryTbl, Create(16, 16));
 if (neHdr.neCbEntTab <> 0) and (stubSize + neHdr.neEntTab < fSz)
  then begin
        Seek(F, stubSize + neHdr.neEntTab);
        L := neHdr.neCbEntTab; M := 1;
        repeat
         BlockRead(F, ETR.Count, sizeOf(ETR.Count));
         ETR.Obj := 0;
         BlockRead(F, ETR.Obj, 1);
         Dec(L, 1 + sizeOf(ETR.Count));
         if (ETR.Count = 0) or (L <= 0) then break;
         ETR.BndType := btEntry16;
         case ETR.Obj of
          $00 : Inc(M, ETR.Count);
          $FE : begin Res := lxeUnkEntBundle; Goto locEx; end;
          else begin
            if ETR.Obj = $FF
              then K := 6        { Bundle of moveable entries }
              else K := fixEnt16;
            J := ETR.Count * K;
            GetMem(tmpBuff, J);
            BlockRead(F, tmpBuff^, J);
            Dec(L, J);
            For I := 1 to ETR.Count do
            begin
             New(EP);
             with pNEentryBundle(@tmpBuff^[pred(I) * K])^, EP^ do
             begin
              Ordinal := M; Inc(M);
              if ETR.Obj = $FF
               then begin Obj := Ref.movSegNo; Entry.e16Ofs := Ref.movOfs; end
               else begin Obj := ETR.Obj;      Entry.e16Ofs := Ref.fixOfs; end;
              if (ObjTable^[Obj].oFlags and objIOPL)<>0 then BndType := btGate16
                                                        else BndType := btEntry16;
              Entry.e16Flags := Flags and lxExport; { No matter whether e16Flags, or eGate16Flags is used }
             end;
             EntryTbl^.Insert(EP);
            end;
            FreeMem(tmpBuff, J);
          end;
         end;
        until (inOutRes <> 0) or (L <= 0);
        updateLast;
       end;

{ Temporary read Module Reference Table }
 GetMem(ModRefTbl, neHdr.neCMod * sizeOf(Word16));
 if (neHdr.neModTab <> 0) and (stubSize + neHdr.neModTab < fSz)
  then begin
        Seek(F, stubSize + neHdr.neModTab);
        BlockRead(F, ModRefTbl^, neHdr.neCMod * sizeOf(Word16));
        updateLast;
       end;

{ Convert Imported Names table }
 New(ImpModTbl, Create(16, 16));
 New(ImpProcTbl, Create(16, 16));
 New(ImportP, Create(16, 16));
 New(ImportM, Create(16, 16));
 if (neHdr.neImpTab <> 0) and (stubSize + neHdr.neImpTab < fSz)
  then begin
        Seek(F, stubSize + neHdr.neImpTab);
        I := neHdr.neEntTab - neHdr.neImpTab; L := 0;
        While (inOutRes = 0) and (I > 0) do
         begin
          BlockRead(F, S, sizeOf(Byte));
          BlockRead(F, S[1], length(S));
          K := 0;
          For J := 1 to neHdr.neCMod do
           if ModRefTbl^[pred(J)] = L
            then begin K := J; break; end;
          if S <> ''
           then
          if K <> 0
           then begin
                 ImpModTbl^.AtInsert(ImpModTbl^.Count, NewStr(S));
                 ImportM^.Insert(Pointer(K));
                end
           else begin
                 ImpProcTbl^.AtInsert(ImpProcTbl^.Count, NewStr(S));
                 ImportP^.Insert(Pointer(L));
                end;
          Inc(L, succ(length(S)));
          Dec(I, succ(length(S)));
         end;
        updateLast;
       end;

{ Convert segment relocation info }
 Pointer(neSeg) := buff;
 For I := 1 to neHdr.neCSeg do
  begin
   if neSeg^.Flags and nesReloc <> 0
    then if not ConvertRelocations(I)
          then begin
                Res := lxeBadFixupTable;
                break;
               end;
   Inc(longint(neSeg), sizeOf(tNEseg));
  end;
 packFixups(pkfFixups + pkfFixupsVer2);

 ImportM^.DeleteAll; Dispose(ImportM, Destroy);
 ImportP^.DeleteAll; Dispose(ImportP, Destroy);
{ Free Module Reference Table }
 FreeMem(ModRefTbl, neHdr.neCMod * sizeOf(Word16));
{ Free segment table }
 FreeMem(buff, neHdr.neCSeg * sizeOf(tNEseg));
 if Res = lxeBadFixupTable Then Goto locEx                   { ConvertRelocations() failed }
                           Else Header.lxFRecTabOfs := 1;    { FixUp record present }

{ Convert resource table }
 if (neHdr.neRsrcTab <> 0) and (stubSize + neHdr.neRsrcTab < fSz)
  then begin
        if (neHdr.neCRes > 0) and (loadFlags and lneIgnoreRsrc = 0)
         then begin Res := lxeResourcesInNE; Goto locEx; end;
        GetMem(RsrcTable, neHdr.neCRes * sizeOf(tResource));
        Seek(F, stubSize + neHdr.neRsrcTab);
        For I := 1 to neHdr.neCRes do
         with RsrcTable^[I] do
          begin
           J := neHdr.neCseg - neHdr.neCRes + I;   {Number of resource object}
           BlockRead(F, neRes, sizeOf(neRes));
           resType := neRes.resType;
           resName := neRes.resID;
           resObj  := J;
           resOffs := 0;   {since resources are located in different segments}
           with ObjTable^[J] do
            begin
             resSize := oSize;
             oFlags := oFlags or objResource;
            end;
          end;
        updateLast;
       end;

 if lastData < fSz
  then begin
        OverlaySize := fSz - lastData;
        GetMem(Overlay, OverlaySize);
        Seek(F, lastData);
        BlockRead(F, Overlay^, OverlaySize);
        if pWord16(Overlay)^ = neDebugMagic
         then begin
               DebugInfo := Overlay;
               Header.lxDebugLen := OverlaySize;
               Overlay := nil; OverlaySize := 0;
              end;
       end;

 if inOutRes <> 0 then GoTo locEx;
 Res := lxeOK;
locEx:
 if ioResult <> 0 then Res := lxeReadError;
 if Res <> lxeOK then freeModule;
 LoadNE := Res;
 Close(F); inOutRes := 0;
end;

function tLX.Save(const fName : string; saveFlags : Longint) : Byte;
label locEx;
var
 F     : File;
 Res   : Byte;
 I,J,
 K,L   : Longint;
 pL    : pLong;
 NTR   : pNameTblRec;
 ETR   : tEntryTblRec;
 EP,NP : pEntryPoint;
 ZeroB : pByteArray;
 ZeroL : Longint;
 pS    : pString;
begin
{ The following fields in Header must be set up before Save: }
{ lxMpages      lxStartObj   lxEIP         lxStackObj
  lxESP         lxPageSize   lxPageShift   lxObjCnt
  lxRsrcCnt     lxDirCnt     lxAutoData }
 Header.lxFixupSum := 0;
 Header.lxLdrSum := 0;
 Header.lxNResSum := 0;
 {lxInstPreload := 0;*}
 {lxInstDemand := 0;*}
 {lxHeapSize := 0;*}
 if SaveFlags and svfAlignEachObj = svfEOalnSector
  then begin
        SaveFlags := (SaveFlags and not svfAlignFirstObj) or svfFOalnSector;
        if Header.lxPageShift < 9 then Header.lxPageShift := 9;
       end;
 if (SaveFlags and svfAlignFirstObj = svfFOalnSector) and (Header.lxPageShift < 9)
  then ZeroL := 512
  else ZeroL := 1 shl Header.lxPageShift;

{ Check LX flags }
 with Header do
  if ((lxMFlags and (lxLibInit or lxLibTerm) <> 0)) and
     ((lxStartObj = 0) or (lxStartObj >= lxObjCnt) or (lxEIP >= ObjTable^[lxStartObj].oSize))
   then lxMFlags := lxMFlags and (not (lxLibInit or lxLibTerm));

 GetMem(ZeroB, ZeroL);
 if ZeroB = nil then begin Res := lxeNoMemory; GoTo locEx; end;
 FillChar(ZeroB^, ZeroL, 0);

 Res := lxeOK; I := FileMode;
 FileMode := open_access_ReadWrite or open_share_DenyReadWrite;
 Assign(F, fName); SetFattr(F, 0); inOutRes := 0;
 Rewrite(F, 1); FileMode := I; if inOutRes <> 0 then Goto locEx;

{ Write stub to file. }
 if (StubSize <> 0) and ((Stub = nil) or (StubSize < $40))
  then begin Res := lxeInvalidStub; Goto locEx; end;
 if (Stub <> nil)
  then begin
        pLongArray(Stub)^[$0F] := StubSize;
        BlockWrite(F, Stub^, StubSize);
       end;

{ Temporary skip header }
 Seek(F, StubSize + sizeOf(Header));

{ Write Object Table }
 if ObjTable <> nil
  then begin
        Header.lxObjTabOfs := FilePos(F) - StubSize;
        BlockWrite(F, ObjTable^, Header.lxObjCnt * sizeOf(tObjTblRec));
       end
  else Header.lxObjTabOfs := 0;

{ Temporary skip Object Page Map Table }
 Seek(F, FilePos(F) + Header.lxMpages * sizeOf(tObjMapRec));

{ Write resource table }
 if RsrcTable <> nil
  then begin
        Header.lxRsrcTabOfs := FilePos(F) - StubSize;
        BlockWrite(F, RsrcTable^, Header.lxRsrcCnt * sizeOf(tResource));
       end
  else Header.lxRsrcTabOfs := 0;

{ Write resident name table }
 Header.lxResTabOfs := FilePos(F) - StubSize;
 For I := 1 to ResNameTbl^.Count do
  begin
   NTR := ResNameTbl^.At(pred(I));
   BlockWrite(F, NTR^.Name^, succ(length(NTR^.Name^)));
   BlockWrite(F, NTR^.Ord, sizeOf(Word16));
  end;
 I := 0; BlockWrite(F, I, sizeOf(Byte));

{ Write module entry table }
 Header.lxEntTabOfs := FilePos(F) - StubSize;
 I := 1;
 While I <= EntryTbl^.Count do
  begin
   J := I;
   EP := pEntryPoint(EntryTbl^.At(pred(I)));
   if I > 1
    then begin
          NP := pEntryPoint(EntryTbl^.At(I-2));
          K := pred(EP^.Ordinal - NP^.Ordinal);
         end
    else K := pred(EP^.Ordinal);
   While K > 0 do
    begin
     ETR.Count := MinL(K, 255);
     ETR.BndType := btEmpty;
     BlockWrite(F, ETR, sizeOf(ETR.Count) + sizeOf(ETR.BndType));
     Dec(K, ETR.Count);
    end;
   K := EP^.Ordinal;
   repeat
    Inc(J); Inc(K);
    if (J > EntryTbl^.Count) or (J - I >= 255)
     then break;
    NP := pEntryPoint(EntryTbl^.At(pred(J)));
   until (NP^.Ordinal <> K) or
         (EP^.BndType <> NP^.BndType) or
        ((EP^.BndType <> btEmpty) and
         (EP^.Obj <> NP^.Obj));
   K := BundleRecSize(EP^.BndType);
   ETR.Count := J - I;
   ETR.BndType := EP^.BndType;
   ETR.Obj := EP^.Obj;
   if ETR.BndType = btEmpty
    then BlockWrite(F, ETR, sizeOf(ETR.Count) + sizeOf(ETR.BndType))
    else BlockWrite(F, ETR, sizeOf(ETR));
   While I < J do
    begin
     BlockWrite(F, pEntryPoint(EntryTbl^.At(pred(I)))^.Entry, K);
     Inc(I);
    end;
  end;
 ETR.Count := 0; BlockWrite(F, ETR.Count, sizeOf(ETR.Count));

{ Write module directives table }
 if ModDirTbl <> nil
  then begin
        Header.lxDirTabOfs := FilePos(F) - StubSize;
        BlockWrite(F, ModDirTbl^, Header.lxDirCnt * sizeOf(tResource));
       end
  else Header.lxDirTabOfs := 0;

{ Write per-page checksum }
 if PerPageCRC <> nil
  then begin
        Header.lxPageSumOfs := FilePos(F) - StubSize;
        BlockWrite(F, PerPageCRC^, Header.lxMpages * sizeOf(Longint));
       end
  else Header.lxPageSumOfs := 0;

 Header.lxLdrSize := FilePos(F) - Header.lxObjTabOfs - StubSize;

{ Write page fixup table }
 L := FilePos(F);

 I := 0; BlockWrite(F, I, sizeOf(Longint));        {fixup offset for 1st page}
 For I := 1 to pred(Header.lxMpages) do             {convert sizes to offsets}
  Inc(FixRecSize^[I], FixRecSize^[pred(I)]);
 BlockWrite(F, FixRecSize^, Header.lxMpages * sizeOf(Longint));
 For I := pred(Header.lxMpages) downto 1 do    {convert back offsets to sizes}
  Dec(FixRecSize^[I], FixRecSize^[pred(I)]);

{ Write fixup record table if it present }
 If Header.lxFRecTabOfs<>0 Then
 begin
  Header.lxFRecTabOfs := FilePos(F) - StubSize;
  For I := 0 to pred(Header.lxMPages) do
   BlockWrite(F, FixRecTbl^[I]^, FixRecSize^[I]);
 end;

{ Write imported modules table }
 Header.lxImpModOfs := FilePos(F) - StubSize;
 Header.lxImpModCnt := ImpModTbl^.Count;
 For I := 1 to Header.lxImpModCnt do
  begin
   pS := ImpModTbl^.At(pred(I));
   if pS <> nil
    then BlockWrite(F, pS^, succ(length(pS^)))
    else BlockWrite(F, ZeroB^, 1);
  end;

{ Write imported procedures table }
 Header.lxImpProcOfs := FilePos(F) - StubSize;
 For I := 1 to ImpProcTbl^.Count do
  begin
   pS := ImpProcTbl^.At(pred(I));
   if pS <> nil
    then BlockWrite(F, pS^, succ(length(pS^)))
    else BlockWrite(F, ZeroB^, 1);
  end;

{ Calculate fixup section size }
 Header.lxFPageTabOfs := L - StubSize;
 Header.lxFixupSize := FilePos(F) - L;

{ Now write the data/code pages }
 L := FilePos(F);
 case SaveFlags and svfAlignFirstObj of
  svfFOalnNone   : I := L;
  svfFOalnShift  : I := (L + pred(1 shl Header.lxPageShift)) and
                        ($FFFFFFFF shl Header.lxPageShift);
  svfFOalnSector : I := (L + 511) and $FFFFFE00;
 end;
 BlockWrite(F, ZeroB^, I - L);

 Header.lxDataPageOfs := 0;
 Header.lxIterMapOfs := 0;
 Header.lxDataPageOfs := FilePos(F);
 For I := 1 to Header.lxMpages do
  begin
   K := PageOrder^[pred(I)];
   with ObjMap^[K] do
    begin
     case PageFlags of
      pgValid     : pL := @Header.lxDataPageOfs;
      pgIterData,
      pgIterData2,
      pgIterData3 : begin
                     Header.lxIterMapOfs := Header.lxDataPageOfs;
                     pL := @Header.lxIterMapOfs;
                    end;
      pgInvalid,
      pgZeroed    : pL := nil;
      else{pgRange} begin Res := lxeUnkPageFlags; GoTo locEx; end;
     end;
     if pL <> nil
      then begin
            if (Pages^[pred(K)] = nil) and (PageSize <> 0)
             then begin Res := lxeInvalidPage; GoTo locEx; end;
            J := FilePos(F);
            L := (J - pL^ + pred(1 shl Header.lxPageShift)) and
                 ($FFFFFFFF shl Header.lxPageShift);
            if pL^ + L > J then BlockWrite(F, ZeroB^, pL^ + L - J);
            PageDataOffset := L shr Header.lxPageShift;
            BlockWrite(F, Pages^[pred(K)]^, PageSize);
           end
      else PageDataOffset := 0;
    end;
  end;

{ And now write the non-resident names table }
 if NResNameTbl^.Count > 0
  then begin
        Header.lxNResTabOfs := FilePos(F);
        For I := 1 to NResNameTbl^.Count do
         begin
          NTR := NResNameTbl^.At(pred(I));
          BlockWrite(F, NTR^.Name^, succ(length(NTR^.Name^)));
          BlockWrite(F, NTR^.Ord, sizeOf(Word16));
         end;
        I := 0; BlockWrite(F, I, sizeOf(Byte));
        Header.lxNResTabSize := FilePos(F) - Header.lxNResTabOfs;
       end
  else begin
        Header.lxNResTabOfs := 0;
        Header.lxNResTabSize := 0;
       end;

 if Header.lxDebugInfoOfs <> 0
  then begin
        Header.lxDebugInfoOfs := FilePos(F);
        BlockWrite(F, DebugInfo^, Header.lxDebugLen);
       end
  else Header.lxDebugLen := 0;

 if OverlaySize <> 0
  then BlockWrite(F, Overlay^, OverlaySize);

 If Header.lxFRecTabOfs=0 Then
  Header.lxFRecTabOfs := FilePos(F) - StubSize;        {fake offset to absent fixup record table}

 Seek(F, StubSize + sizeOf(Header) + Header.lxObjCnt * sizeOf(tObjTblRec));
{ Now write Object Page Map Table }
 if ObjMap <> nil
  then begin
        Header.lxObjMapOfs := FilePos(F) - StubSize;
        BlockWrite(F, ObjMap^, Header.lxMpages * sizeOf(tObjMapRec));
       end
  else Header.lxObjMapOfs := 0;

{ Now seek to beginning and write the LX header }
 Seek(F, StubSize);
 BlockWrite(F, Header, sizeOf(Header));

locEx:
 if ZeroB <> nil then FreeMem(ZeroB, ZeroL);
 if ioResult <> 0 then Res := lxeWriteError;
 if TimeStamp <> 0 then SetFTime(F, TimeStamp);
 Save := Res;  Close(F); inOutRes := 0;
 if (Res = lxeOK) 
{$IFDEF OS2}
   and (not EA^.Attach(fName))
   then Save := lxeEAwriteError
   else 
{$ELSE}
   then
{$ENDIF}
     SetFattr(F, FileAttr);
end;

procedure tLX.freeModule;
var
 I   : Longint;
 NTR : pNameTblRec;
 EBR : pEntryTblRec;
begin
 if PageOrder <> nil
  then FreeMem(PageOrder, Header.lxMpages * sizeOf(Pointer));

 if Pages <> nil
  then begin
        For I := 1 to Header.lxMpages do
         if Pages^[pred(I)] <> nil
          then FreeMem(Pages^[pred(I)], ObjMap^[I].PageSize);
        FreeMem(Pages, Header.lxMpages * sizeOf(Pointer));
       end;

 if FixRecTbl <> nil
  then begin
        For I := 1 to Header.lxMPages do
         FreeMem(FixRecTbl^[pred(I)], FixRecSize^[pred(I)]);
        FreeMem(FixRecTbl, Header.lxMpages * sizeOf(Longint));
       end;

 if ImpProcTbl <> nil
  then Dispose(ImpProcTbl, Destroy);

 if ImpModTbl <> nil
  then Dispose(ImpModTbl, Destroy);

 if FixRecSize <> nil
  then FreeMem(FixRecSize, Header.lxMpages * sizeOf(Longint));

 if PerPageCRC <> nil
  then FreeMem(PerPageCRC, Header.lxMpages * sizeOf(Longint));

 if ModDirTbl <> nil
  then FreeMem(ModDirTbl, Header.lxDirCnt * sizeOf(tResource));

 if EntryTbl <> nil
  then Dispose(EntryTbl, Destroy);

 if NResNameTbl <> nil
  then Dispose(NResNameTbl, Destroy);

 if ResNameTbl <> nil
  then Dispose(ResNameTbl, Destroy);

 if RsrcTable <> nil
  then FreeMem(RsrcTable, Header.lxRsrcCnt * sizeOf(tResource));

 if ObjMap <> nil
  then FreeMem(ObjMap, Header.lxMpages * sizeOf(tObjMapRec));

 if ObjTable <> nil
  then FreeMem(ObjTable, Header.lxObjCnt * sizeOf(tObjTblRec));

 if stubSize <> 0
  then FreeMem(Stub, StubSize);

 if OverlaySize <> 0
  then FreeMem(Overlay, OverlaySize);
{$IFDEF OS2}
 if EA <> nil then Dispose(EA, Destroy);
{$ENDIF}
 Initialize;
end;

function tLX.BundleRecSize;
begin
 case BndType of
  btEmpty    : BundleRecSize := 0;
  btEntry16  : BundleRecSize := fixEnt16;
  btGate16   : BundleRecSize := gateEnt16;
  btEntry32  : BundleRecSize := fixEnt32;
  btEntryFwd : BundleRecSize := fwdEnt;
  else BundleRecSize := -1;
 end;
end;

function tLX.SetFixups;
var
 Fix    : pByteArray;
 I,FixAlloc,
 FixLen : Integer;
 FixPos,
 FixSz  : Longint;
 FixTbl : pByteArray;

procedure FreeFix;
begin
 FreeMem(Fix, FixAlloc);
 FixAlloc := 0;
end;

function PackFixup(Fixup : pLXreloc) : boolean;

procedure Put8(B : Byte);
begin
 pByte(@Fix^[FixLen])^ := B;
 Inc(FixLen, sizeOf(Byte));
end;

procedure Put16(W : Word16);
begin
 pWord16(@Fix^[FixLen])^ := W;
 Inc(FixLen, sizeOf(Word16));
end;

procedure Put32(L : Word32);
begin
 pWord32(@Fix^[FixLen])^ := L;
 Inc(FixLen, sizeOf(Word32));
end;

procedure PutIntRef;
begin
 with Fixup^ do
  if Target.intRef and $FFFF0000 = 0
   then begin
         Flags := Flags and (not nr32bitOff);
         Put16(Target.intRef);
        end
   else begin
         Flags := Flags or nr32bitOff;
         Put32(Target.intRef);
        end;
end;

procedure PutAddFixup;
begin
 with Fixup^ do
  if Flags and nrAdd <> 0
   then if AddFixup = 0
         then Flags := Flags and (not nrAdd)
         else
        if AddFixup and $FFFF0000 = 0
         then begin
               Flags := Flags and (not nr32bitAdd);
               Put16(AddFixup);
              end
         else begin
               Flags := Flags or nr32bitAdd;
               Put32(AddFixup);
              end;
end;

var
 I : Integer;
begin
 PackFixup := FALSE;
 if Fixup = nil then exit;
 with Fixup^ do
  begin
   I := 64;
   if sType and nrChain <> 0
    then Inc(I, targetCount * 2);
   if I > FixAlloc
    then begin
          FreeFix; FixAlloc := I;
          GetMem(Fix, FixAlloc);
         end;
   if Fix = nil then exit;
   FixLen := 0;
   Put8(sType);
   Put8(Flags);
   if sType and nrChain = 0
    then Put16(sOffs)
    else begin
          if targetCount > 255 then exit;
          Put8(targetCount);
         end;

   if (Flags and (nrAdd + nrRtype) = (nrAdd + nrRint))
    then begin
          if sType and nrSType = nrSSeg
           then Inc(ObjMod, AddFixup)
           else Inc(Target.intRef, AddFixup);
          Flags := Flags and (not nrAdd);
         end;

   if ObjMod and $FFFFFF00 = 0
    then begin
          Flags := Flags and (not nr16objMod);
          Put8(ObjMod);
         end
    else begin
          Flags := Flags or nr16objMod;
          Put16(ObjMod);
         end;

   case Flags and nrRtype of
    nrRint:
     begin
      if sType and nrSType <> nrSSeg
       then PutIntRef;
      PutAddFixup;
     end;
    nrRord:
     begin
      if Target.extRef.Ord and $FFFFFF00 = 0
       then begin
             Flags := Flags or nr8bitOrd and (not nr32bitOff);
             Put8(Target.extRef.Ord);
            end
       else
      if Target.extRef.Ord and $FFFF0000 = 0
       then begin
             Flags := Flags and (not nr8bitOrd) and (not nr32bitOff);
             Put16(Target.extRef.Ord);
            end
       else begin
             Flags := Flags and (not nr8bitOrd) or nr32bitOff;
             Put32(Target.extRef.Ord);
            end;
      PutAddFixup;
     end;
    nrRnam:
     begin
      if Target.extRef.Proc and $FFFF0000 = 0
       then begin
             Flags := Flags and (not nr32bitOff);
             Put16(Target.extRef.Proc);
            end
       else begin
             Flags := Flags or nr32bitOff;
             Put32(Target.extRef.Proc);
            end;
      PutAddFixup;
     end;
    nrRent:
     begin
      PutAddFixup;
     end;
   end;
   Fix^[1] := Flags; {Update flags}
   if sType and nrChain <> 0
    then For I := 1 to targetCount do Put16(targets^[pred(I)]);
  end;
 PackFixup := TRUE;
end;

begin
 SetFixups := FALSE;
 if (PageNo = 0) or (PageNo > Header.lxMPages) then exit;
{ Count overall fixup size }
 FixSz := 0; FixAlloc := 0; Fix := nil;
 For I := 1 to Fixups^.Count do
  if PackFixup(Fixups^.At(pred(I)))
   then Inc(FixSz, FixLen)
   else begin FreeFix; exit; end;
 GetMem(FixTbl, FixSz);
 if (FixSz <> 0) and (FixTbl = nil) then begin FreeFix; exit; end;
 FixPos := 0;
 For I := 1 to Fixups^.Count do
  begin
   PackFixup(Fixups^.At(pred(I)));
   Move(Fix^, FixTbl^[FixPos], FixLen);
   Inc(FixPos, FixLen);
  end;
 FreeFix;

 if FixRecSize^[pred(PageNo)] <> 0
  then FreeMem(FixRecTbl^[pred(PageNo)], FixRecSize^[pred(PageNo)]);
 FixRecSize^[pred(PageNo)] := FixSz;
 FixRecTbl^[pred(PageNo)] := FixTbl;

 SetFixups := TRUE;
end;

function tLX.FixupsSize;
var
 Fix    : pByteArray;
 I,FixAlloc,
 FixLen : Integer;
 FixPos,
 FixSz  : Longint;
 FixTbl : pByteArray;

function PackFixup(Fixup : pLXreloc) : boolean;

procedure PutIntRef;
begin
 with Fixup^ do
  if Target.intRef and $FFFF0000 = 0
   then Inc(FixLen, sizeOf(Word16))
   else Inc(FixLen, sizeOf(Word32));
end;

procedure PutAddFixup;
begin
 with Fixup^ do
  if (Flags and nrAdd <> 0) and (AddFixup <> 0)
   then if AddFixup and $FFFF0000 = 0
         then Inc(FixLen, sizeOf(Word16))
         else Inc(FixLen, sizeOf(Word32));
end;

var
 I : Integer;
begin
 PackFixup := FALSE;
 if Fixup = nil then exit;
 with Fixup^ do
  begin
   FixLen := sizeOf(Byte) * 2;
   if sType and nrChain = 0
    then Inc(FixLen, sizeOf(Word16))
    else Inc(FixLen, sizeOf(Byte));

   if ObjMod and $FFFFFF00 = 0
    then Inc(FixLen, sizeOf(Byte))
    else Inc(FixLen, sizeOf(Word16));

   case Flags and nrRtype of
    nrRint:
     begin
      if sType and nrSType <> nrSSeg
       then PutIntRef;
      PutAddFixup;
     end;
    nrRord:
     begin
      if Target.extRef.Ord and $FFFFFF00 = 0
       then Inc(FixLen, sizeOf(Byte))
       else
      if Target.extRef.Ord and $FFFF0000 = 0
       then Inc(FixLen, sizeOf(Word16))
       else Inc(FixLen, sizeOf(Word32));
      PutAddFixup;
     end;
    nrRnam:
     begin
      if Target.extRef.Proc and $FFFF0000 = 0
       then Inc(FixLen, sizeOf(Word16))
       else Inc(FixLen, sizeOf(Word32));
      PutAddFixup;
     end;
    nrRent:
     begin
      PutAddFixup;
     end;
   end;
   if sType and nrChain <> 0
    then Inc(FixLen, targetCount * sizeOf(Word16));
  end;
 PackFixup := TRUE;
end;

begin
 FixupsSize := 0;
{ Count overall fixup size }
 FixSz := 0;
 For I := 1 to Fixups^.Count do
  if PackFixup(Fixups^.At(pred(I)))
   then Inc(FixSz, FixLen)
   else exit;
 FixupsSize := FixSz;
end;

function tLX.GetFixups;
var
 FixTbl : pByteArray;
 FixSz  : Longint;
 newFix,
 Fix    : pLXreloc;
 ST,SF  : Byte;
 baseSet: boolean;
 Src,Base,
 I,fixCount,
 Next   : longint;
 Page   : pByteArray;

function Get8 : Byte;
begin
 Get8 := FixTbl^[0];
 Inc(Longint(FixTbl));
 Dec(FixSz);
end;

function Get16 : Word16;
begin
 Get16 := pWord16(@FixTbl^[0])^;
 Inc(Longint(FixTbl), 2);
 Dec(FixSz, 2);
end;

function Get32 : Word32;
begin
 Get32 := pWord32(@FixTbl^[0])^;
 Inc(Longint(FixTbl), 4);
 Dec(FixSz, 4);
end;

procedure GetIntRef;
begin
 with Fix^ do
  if Flags and nr32bitOff = 0
   then Target.intRef := Get16
   else Target.intRef := Get32;
end;

procedure GetAddFixup;
begin
 with Fix^ do
  if Flags and nrAdd <> 0
   then if Flags and nr32bitAdd = 0
         then AddFixup := Get16
         else AddFixup := Get32;
end;

begin
 GetFixups := FALSE;
 If Header.lxFRecTabOfs = 0 Then Exit;
 FixSz := FixRecSize^[pred(PageNo)];
 FixTbl := FixRecTbl^[pred(PageNo)];
 While FixSz > 0 do
  begin
   ST := Get8;
   SF := Get8;
   New(Fix);
   if Fix = nil then exit;
   Fix^.sType := ST;
   Fix^.Flags := SF;

   if ST and nrChain <> 0
    then Fix^.targetCount := Get8
    else Fix^.sOffs := Get16;

   if SF and nr16objMod = 0
    then Fix^.ObjMod := Get8
    else Fix^.ObjMod := Get16;

   case SF and nrRType of
    nrRInt : begin
              if ST and nrSType <> nrSSeg
               then GetIntRef;
              GetAddFixup;
             end;
    nrROrd : begin
              if SF and nr8bitOrd <> 0
               then Fix^.Target.extRef.Ord := Get8
               else
              if SF and nr32bitOff = 0
               then Fix^.Target.extRef.Ord := Get16
               else Fix^.Target.extRef.Ord := Get32;
              GetAddFixup;
             end;
    nrRNam : begin
              if SF and nr32bitOff = 0
               then Fix^.Target.extRef.Proc := Get16
               else Fix^.Target.extRef.Proc := Get32;
              GetAddFixup;
             end;
    nrRent : GetAddFixup;
    else begin Dispose(Fix); exit; end;
   end;

   if ST and nrChain <> 0
    then begin
          GetMem(Fix^.targets, Fix^.targetCount * sizeOf(Word16));
          Move(FixTbl^, Fix^.targets^, Fix^.targetCount * sizeOf(Word16));
          Inc(Longint(FixTbl), Fix^.targetCount * sizeOf(Word16));
          Dec(FixSz, Fix^.targetCount * sizeOf(Word16));
         end;

   if SF and nrNewChain <> 0
    then with ObjMap^[PageNo] do
          begin
           if ((Fix^.Flags and nrRType) <> nrRInt) or
              ((Fix^.sType and nrChain) <> 0)
            then exit;
           if PageFlags <> pgValid
            then UnpackPage(PageNo,true);
           if (PageFlags <> pgValid)
            then exit;
           Page := Pages^[pred(PageNo)];
           Src := Fix^.sOffs;
           baseSet := FALSE;
           fixCount := succ(Fixups^.Count);
           repeat
            Next := 0;
            Move(Page^[Src], Next, MinL(4, PageSize - Src));
            FillChar(Page^[Src], MinL(4, PageSize - Src), 0);
            if not baseSet
             then begin
                   Base := Fix^.Target.intRef - (Next and $FFFFF);
                   baseSet := TRUE;
                  end;
            For I := fixCount to Fixups^.Count do
             if pLXreloc(Fixups^.At(pred(I)))^.sOffs = Src
              then begin I := -1; break; end;
            if (I = -1) or (succ(Fixups^.Count - fixCount) > Header.lxPageSize shr 2)
             then break; {we have a loop or error here}
            New(newFix); newFix^ := Fix^;
            newFix^.sOffs := Src;
            newFix^.Target.intRef := Base + (Next and $FFFFF);
            newFix^.Flags := newFix^.Flags and (not nrNewChain);
            Fixups^.Insert(newFix);
            Src := Next shr 20;
           until Src > Header.lxPageSize - 4;
           Dispose(Fix);
          end
    else Fixups^.Insert(Fix);
  end;
 GetFixups := FixSz = 0;
end;

procedure tLX.PackFixups;
var
 pgTop,
 tmpPF,
 P  : Integer;
 aFx,
 Fx : pFixupCollection;
 nP : pByteArray;

procedure packVer2;
var
 I,J   : Integer;
 F1,F2 : pLXreloc;
 fixT  : pWord16array;
begin
 For I := Fx^.Count downto 2 do
  begin
   F2 := Fx^.At(pred(I));
   For J := pred(I) downto 1 do
    begin
     F1 := Fx^.At(pred(J));
     if (F1^.sType and (not nrChain) = F2^.sType and (not nrChain)) and
        (F1^.Flags = F2^.Flags) and (F1^.Flags and nrNewChain = 0) and
        (F1^.ObjMod = F2^.ObjMod)
      then begin
          { Check more deeply }
            case F1^.Flags and nrRType of
             nerRInt : if ((F1^.sType and nrSType <> nrSSeg) and
                           (F1^.Target.intRef <> F2^.Target.intRef)) or
                          ((F1^.Flags and nrAdd <> 0) and
                           (F1^.addFixup <> F2^.addFixup))
                        then Continue;
             nerROrd : if (F1^.Target.extRef.Ord <> F2^.Target.extRef.Ord) or
                          ((F1^.Flags and nrAdd <> 0) and
                           (F1^.addFixup <> F2^.addFixup))
                        then Continue;
             nerRNam : if (F1^.Target.extRef.Proc <> F2^.Target.extRef.Proc) or
                          ((F1^.Flags and nrAdd <> 0) and
                           (F1^.addFixup <> F2^.addFixup))
                        then Continue;
             nrRent  : if ((F1^.Flags and nrAdd <> 0) and
                           (F1^.addFixup <> F2^.addFixup))
                        then Continue;
            end;
          { join these fixups together }
            if F1^.sType and nrChain = 0
             then begin
                   F1^.targetCount := 1;
                   F1^.targets := @F1^.sOffs;
                  end;
            if F2^.sType and nrChain = 0
             then begin
                   F2^.targetCount := 1;
                   F2^.targets := @F2^.sOffs;
                  end;
            if F1^.targetCount + F2^.targetCount > 255 then Continue;
            GetMem(fixT, (F1^.targetCount + F2^.targetCount) * sizeOf(Word16));
            Move(F1^.targets^, fixT^, F1^.targetCount * sizeOf(Word16));
            Move(F2^.targets^, fixT^[F1^.targetCount], F2^.targetCount * sizeOf(Word16));
            if F1^.sType and nrChain <> 0
             then FreeMem(F1^.targets, F1^.targetCount * sizeOf(Word16));
            F1^.sType := F1^.sType or nrChain;
            Inc(F1^.targetCount, F2^.targetCount);
            F1^.targets := fixT;
            Fx^.AtFree(pred(I));
            break;
           end;
    end;
  end;
end;

procedure packVer4;
var
 I,J,K,L : Integer;
 sO      : Word16;
 F1,F2   : pLXreloc;
 FixMax,
 FixBase : pLongArray;

function GetL(Offs : Word16) : Longint;
begin
 if (Offs < pgTop)
  then GetL := pLong(@pByteArray(Pages^[pred(P)])^[Offs])^
  else GetL := -1;
end;

function SetL(Offs : Word16; Value : Longint) : boolean;
begin
 if (Offs < pgTop)
  then begin
        pLong(@pByteArray(Pages^[pred(P)])^[Offs])^ := Value;
        SetL := TRUE;
       end
  else SetL := FALSE;
end;

function Prepare(Fixup : pLXreloc) : boolean;
var
 K  : Integer;
 nF : pLXreloc;
 pW : pWord16Array;
begin
 Prepare := FALSE;
 if Fixup^.sType and nrChain = 0
  then begin
        Fixup^.targetCount := 1;
        Fixup^.targets := @Fixup^.sOffs;
       end;

 if Fixup^.Flags and nrNewChain = 0
  then begin
        sO := pred(Header.lxPageSize);
        For K := Fixup^.targetCount downto 1 do
         begin
          if not SetL(Fixup^.targets^[pred(K)], longint(sO) shl 20 + (Fixup^.Target.intRef - L))
           then begin
                 New(nF); Move(Fixup^, nF^, sizeOf(nF^));
                 nF^.sOffs := Fixup^.targets^[pred(K)];
                 nF^.sType := nF^.sType and (not nrChain);
                 Fx^.AtInsert(I, nF);
                 if Fixup^.targetCount > 1
                  then begin
                        GetMem(pW, pred(Fixup^.targetCount) * sizeOf(Word16));
                        Move(Fixup^.targets^, pW^, pred(K) * sizeOf(Word16));
                        Move(Fixup^.targets^[K], pW^[pred(K)], (Fixup^.targetCount - K) * sizeOf(Word16));
                        FreeMem(Fixup^.targets, Fixup^.targetCount * sizeOf(Word16));
                        Dec(Fixup^.targetCount); Fixup^.targets := pW;
                       end
                  else exit;
                end
           else sO := Fixup^.targets^[pred(K)];
         end;
        if Fixup^.sType and nrChain <> 0
         then FreeMem(Fixup^.targets, Fixup^.targetCount * sizeOf(Word16));
        Fixup^.sOffs := sO;
        Fixup^.sType := Fixup^.sType and (not nrChain);
        Fixup^.Flags := Fixup^.Flags or nrNewChain;
       end;
 Prepare := TRUE;
end;

begin
 with ObjMap^[P] do
  if (PageFlags <> pgValid) or (PageSize <> Header.lxPageSize)
   then exit;

 GetMem(FixBase, sizeOf(longint) * Header.lxObjCnt);
 if FixBase = nil then exit;
 GetMem(FixMax, sizeOf(longint) * Header.lxObjCnt);
 if FixMax = nil then begin FreeMem(FixBase, sizeOf(longint) * Header.lxObjCnt); exit; end;
 FillChar(FixBase^, sizeOf(longint) * Header.lxObjCnt, $7F);
 FillChar(FixMax^, sizeOf(longint) * Header.lxObjCnt, 0);
 For I := 1 to Fx^.Count do
  begin
   F2 := Fx^.At(pred(I));
   with F2^ do
    if (sType and nrSType in [nrOff32,nrSoff32]) and
       (Flags and nrRtype = nrRint)
     then begin
           K := Target.intRef;
          {F2^ _CANNOT_ point to a new-type fixup chain}
           if (sOffs < pgTop)
            then begin
                  if (K < FixBase^[pred(ObjMod)])
                   then FixBase^[pred(ObjMod)] := K;
                  if (K > FixMax^[pred(ObjMod)])
                   then FixMax^[pred(ObjMod)] := K;
                 end;
          end;
  end;
 For I := 0 to pred(Header.lxObjCnt) do
  if FixBase^[I] <> $7F7F7F7F
   then begin
         J := FixMax^[I] - FixBase^[I];
         K := $FFFFF - (J and $FFFFF);
         if FixBase^[I] > K
          then Dec(FixBase^[I], K)
          else FixBase^[I] := 0;
        end;
 FreeMem(FixMax, sizeOf(longint) * Header.lxObjCnt);

 For I := Fx^.Count downto 2 do
  begin
   F2 := Fx^.At(pred(I));
   if (not (F2^.sType and nrSType in [nrOff32,nrSoff32])) or
      (F2^.Flags and nrRtype <> nrRint)
    then Continue;
   if F2^.Flags and nrNewChain <> 0
    then L := F2^.Target.intRef - (GetL(F2^.sOffs) and $FFFFF)
    else L := FixBase^[pred(F2^.ObjMod)] + (F2^.Target.intRef and $FFF00000);
   if not Prepare(F2)
    then begin Fx^.AtFree(pred(I)); Continue; end;
   For J := pred(I) downto 1 do
    begin
     F1 := Fx^.At(pred(J));
     if (F1^.sType and nrSType = F2^.sType and nrSType) and
        (F1^.Flags and nrRtype = nrRint) and
        (F1^.ObjMod = F2^.ObjMod)
      then begin
            if F1^.Flags and nrNewChain <> 0
             then K := F1^.Target.intRef - (GetL(F1^.sOffs) and $FFFFF)
             else K := FixBase^[pred(F1^.ObjMod)] + (F1^.Target.intRef and $FFF00000);
            if (L <> K) or (L = $7F7F7F7F) then Continue;

            if not Prepare(F1)
             then begin Fx^.AtFree(pred(J)); Dec(I); Continue; end;

            sO := F1^.sOffs; K := -1;
            while sO <= Header.lxPageSize - 4 do
             begin K := sO; sO := GetL(sO) shr 20; end;

            if K <> -1
             then SetL(K, (longint(F2^.sOffs) shl 20) + (GetL(K) and $FFFFF))
             else Continue;
            F1^.Target.intRef := L + (GetL(F1^.sOffs) and $FFFFF);

            Fx^.AtFree(pred(I));
            break;
           end;
    end;
  end;

 For I := 1 to Fx^.Count do
  begin
   F2 := Fx^.At(pred(I));
   if (F2^.sType and nrSType in [nrOff32,nrSoff32]) and
      (F2^.Flags and (nrRtype + nrNewChain) = nrRint) and
      (F2^.sType and nrChain <> 0)
    then begin
          L := FixBase^[pred(F2^.ObjMod)] + (F2^.Target.intRef and $FFF00000);
          Prepare(F2);
         end;
   with F2^ do
    if (Flags and nrNewChain <> 0)
     then begin
           sO := sOffs; J := 0;
           While sO <= pgTop do begin sO := GetL(sO) shr 20; Inc(J); end;
           if J <= 1
            then begin
                  SetL(sOffs, 0);
                  Flags := Flags and (not nrNewChain);
                 end;
          end;
  end;
 FreeMem(FixBase, sizeOf(longint) * Header.lxObjCnt);
end;

procedure ClearFixedBytes;
var
 ofs,cnt,
 I,J,K : Integer;
 Fixup : pLXreloc;
begin
 if Header.lxMFlags and (lxNoIntFix + lxNoExtFix) <> 0
  then exit;
 For I := 1 to Fx^.Count do
  begin
   Fixup := Fx^.At(pred(I));
   if Fixup^.Flags and nrNewChain <> 0
    then Continue;
   if (Fixup^.sType and nrChain <> 0) and
      (Fixup^.targetCount = 1)
    then begin
          Fixup^.sOffs := Fixup^.targets^[0];
          FreeMem(Fixup^.targets, Fixup^.targetCount * sizeOf(Word16));
          Fixup^.sType := Fixup^.sType and (not nrChain);
         end;
   if ObjMap^[P].PageFlags <> pgValid
    then Continue;

   case Fixup^.sType and nrSType of
    nrSByte  : J := 1;                 { lo byte (8-bits)}
    nrSSeg,                            { 16-bit segment (16-bits) }
    nrSOff   : J := 2;                 { 16-bit offset (16-bits) }
    nrSPtr,                            { 16:16 pointer (32-bits) }
    nrOff32,                           { 32-bit offset (32-bits) }
    nrSoff32 : J := 4;                 { 32-bit self-relative offset (32-bits) }
    nrPtr48  : J := 6;                 { 16:32 pointer (48-bits) }
   end;
   if Fixup^.sType and nrChain = 0
    then begin
          Fixup^.targetCount := 1;
          Fixup^.targets := @Fixup^.sOffs;
         end;
   For K := 1 to Fixup^.targetCount do
    begin
     ofs := Fixup^.targets^[pred(K)]; cnt := J;
     if ofs > 32767 then Dec(ofs, 65536);
     if ofs < 0
      then begin Inc(cnt, ofs); ofs :=0; end;
     if ofs + cnt > Header.lxPageSize
      then Dec(cnt, ofs + cnt - Header.lxPageSize);
     if (cnt > 0) and (ofs < Header.lxPageSize)
      then FillChar(pByteArray(Pages^[pred(P)])^[ofs], cnt, 0);
    end;
  end;
end;

var
 ps1,ps2 : Integer;
 cPage   : pByteArray;

begin
 if packFlags and pkfFixups = 0 then exit;
 New(Fx, Create(16, 16));
 if packFlags and pkfFixupsLvl = pkfFixupsMax
  then GetMem(cPage, Header.lxPageSize);
 For P := 1 to Header.lxMPages do
  begin
   tmpPF := packFlags;
   if (Header.lxMFlags and (lxNoIntFix + lxNoExtFix) <> 0) or
      (ObjMap^[P].PageFlags = pgZeroed)
    then tmpPF := (tmpPF and (not pkfFixupsLvl)) or pkfFixupsVer2
    else with ObjMap^[P] do
          begin
           UnpackPage(P,true);
           if PageFlags <> pgValid then Continue;
           GetMem(nP, Header.lxPageSize);
           Move(Pages^[Pred(P)]^, nP^, PageSize);
           FreeMem(Pages^[Pred(P)], PageSize);
           if PageSize < Header.lxPageSize
            then FillChar(nP^[PageSize], Header.lxPageSize - PageSize, 0);
           Pages^[Pred(P)] := nP;
           PageSize := Header.lxPageSize;
          end;
   if GetFixups(P, Fx)
    then begin
          if tmpPF and pkfFixupsLvl >= pkfFixupsVer4
           then pgTop := Header.lxPageSize - MaxL(4, MemScanBwd(nP^, Header.lxPageSize, 0) + 4);

          if tmpPF and pkfFixupsLvl = pkfFixupsMax
           then begin
                 ClearFixedBytes;
                 New(aFx, Clone(Fx));
                 PackVer2;
                 ps1 := Header.lxPageSize;
                 if PackMethod2(Pages^[pred(P)]^, cPage^, Header.lxPageSize -
                     MemScanBwd(Pages^[pred(P)]^, Header.lxPageSize, 0), ps1)
                  then Inc(ps1, FixupsSize(Fx))
                  else ps1 := $7FFFFFFF;
                 XchgL(Fx, aFx);
                 PackVer4;
                 ps2 := Header.lxPageSize;
                 if PackMethod2(Pages^[pred(P)]^, cPage^, Header.lxPageSize -
                     MemScanBwd(Pages^[pred(P)]^, Header.lxPageSize, 0), ps2)
                  then Inc(ps2, FixupsSize(Fx))
                  else ps2 := $7FFFFFFF;
                 if ps1 <= ps2
                  then XchgL(Fx, aFx);
                 Dispose(aFx, Destroy);
                end
           else begin
                 if tmpPF and pkfFixupsLvl >= pkfFixupsVer4
                  then PackVer4;
                 if tmpPF and pkfFixupsLvl >= pkfFixupsVer2
                  then PackVer2;
                end;
          ClearFixedBytes;
          SetFixups(P, Fx);
         end;
   Fx^.FreeAll;
  end;
 if packFlags and pkfFixupsLvl = pkfFixupsMax
  then FreeMem(cPage, Header.lxPageSize);
 Dispose(Fx, Destroy);
end;

procedure tLX.ApplyFixups(ForceApply : boolean; ApplyMask : byte);
var
 Fx   : pFixupCollection;
 F    : pLXreloc;
 I,J,
 P,S  : Integer;
 A    : record case boolean of
         FALSE: (L : Longint; S : Word16);
         TRUE:  (B : array[0..5] of Byte);
        end;
 pOfs : pWord16;
 Chg  : boolean;
 tmpP : pByteArray;

begin
 { Applicable only to EXE modules OR /FB+ was specified }
 if (Header.lxMFlags and lxModType <> lxEXE) and not ForceApply then exit;
 New(Fx, Create(16, 16));
 For P := 1 to Header.lxMPages do
  begin
   Fx^.FreeAll;
   if not GetFixups(P, Fx) then Continue;
   Chg := FALSE;

   For I := Fx^.Count downto 1 do
    begin
     F := Fx^.At(pred(I));
     if F^.Flags and nrRtype = nrRint
      then begin
            if (F^.Flags and nrNewChain <> 0) or
               (F^.ObjMod = 0) or (F^.ObjMod > Header.lxObjCnt) or
               (F^.sType and nrSType in [nrSSeg, nrSPtr, nrPtr48, nrSoff32]) or
               (ObjTable^[F^.ObjMod].oBase = 0)  { Unassigned object address }
             then break;
            if (ObjMap^[P].PageFlags <> pgValid)
             then UnpackPage(P,true);
            if (ObjMap^[P].PageFlags <> pgValid)
             then break;
            if (ObjMap^[P].PageSize < Header.lxPageSize)
             then begin
                   GetMem(tmpP, Header.lxPageSize);
                   Move(Pages^[pred(P)]^, tmpP^, ObjMap^[P].PageSize);
                   FillChar(tmpP^[ObjMap^[P].PageSize], Header.lxPageSize - ObjMap^[P].PageSize, 0);
                   FreeMem(Pages^[pred(P)], ObjMap^[P].PageSize);
                   Pages^[pred(P)] := tmpP;
                   ObjMap^[P].PageSize := Header.lxPageSize;
                  end;
            A.L := ObjTable^[F^.ObjMod].oBase;
            if (A.L = 0)
             then break;   { todo: assign first free address instead of exit }
            if (F^.Flags and nrAdd <> 0)
             then Inc(A.L, F^.addFixup);
            Inc(A.L, F^.Target.intRef);
            case F^.sType and nrSType of
             nrSByte  : if ApplyMask and 1 <> 0 then S := 1 else break;
             nrSSeg   : break;{ CS is known only at runtime }
             nrSPtr   : break;{ CS is known only at runtime }
             nrSOff   : if ApplyMask and 2 <> 0 then S := 2 else break;
             nrPtr48  : break;{ CS is known only at runtime }
             nrOff32  : if ApplyMask and 4 <> 0 then S := 4 else break;
             nrSoff32 : break;{ Not supported (yet?) }
            end;
            if (F^.sType and nrChain <> 0)
             then begin
                   J := F^.targetCount;
                   pOfs := @F^.targets^;
                  end
             else begin
                   J := 1;
                   pOfs := @F^.sOffs;
                  end;
            While J > 0 do
             begin
              if pOfs^ < Header.lxPageSize
               then Move(A.L, pByteArray(Pages^[pred(P)])^[pOfs^],
                         minL(S, Header.lxPageSize - pOfs^))
               else
              if pOfs^ + S >= 0
               then Move(A.B[$10000 - pOfs^], Pages^[pred(P)]^, S - ($10000 - pOfs^));
              Inc(pOfs); Dec(J);
             end;
            Fx^.AtFree(pred(I)); Chg := TRUE;
           end;
    end;

   if Chg then SetFixups(P, Fx);
  end;
 Dispose(Fx, Destroy);
end;

function tLX.UnpackPage(PageNo : Integer; AllowTrunc:boolean) : boolean;
var
 J       : Integer;
 uD,pD   : pByteArray;
 UnpFunc : Function(var srcData, destData; srcDataSize : longint; var dstDataSize : Longint) : boolean;
begin
 UnpackPage := FALSE;
 with ObjMap^[PageNo] do
  begin
   case PageFlags of
    pgIterData  : @UnpFunc := @UnpackMethod1;
    pgIterData2 : @UnpFunc := @UnpackMethod2;
    pgIterData3 : @UnpFunc := @UnpackMethod3;
    pgValid     : @UnpFunc := nil;
    else exit;
   end;
   pD := Pages^[pred(PageNo)];
   if @UnpFunc <> nil
    then begin
          GetMem(uD, Header.lxPageSize); J := Header.lxPageSize;
          if UnpFunc(pD^, uD^, PageSize, J)
           then begin
                 FreeMem(pD, PageSize);
                 GetMem(pD, J);
                 Move(uD^, pD^, J);
                 PageSize := J;
                 PageFlags := pgValid;
                 Pages^[pred(PageNo)] := pD;
                end;
          FreeMem(uD, Header.lxPageSize);
         end;
   if AllowTrunc then
   begin
    J := PageSize;
    While (J > 0) and (pD^[pred(J)] = 0) do Dec(J);
    { 2011-11-16 SHL expand /X:pagenum page to max size }
    if (PageNo = pageToEnlarge) and (J < Header.lxPageSize) then begin
     GetMem(uD, Header.lxPageSize);
     Move(pD^, uD^, PageSize);
     FreeMem(pD, PageSize);
     pD := uD;
     Pages^[pred(PageNo)] := pD;
     PageSize := Header.lxPageSize;
     FillChar(pD^[pred(J)], Header.lxPageSize - J + 1, 0);
     J := Header.lxPageSize
    end;
    if J <> PageSize
     then begin
           GetMem(uD, J);
           Move(pD^, uD^, J);
           Pages^[pred(PageNo)] := uD;
           FreeMem(pD, PageSize);
           PageSize := J;
          end;
   end;
  end;
 UnpackPage := TRUE;
end;

function tLX.UnpackPageNoTouch(PageNo : Integer; var UnpPageSize:longint) : pointer;
var
 J       : Integer;
 uD,pD   : pByteArray;
 UnpFunc : Function(var srcData, destData; srcDataSize : longint; var dstDataSize : Longint) : boolean;
 rc      : boolean;
begin
 uD:= nil;
 with ObjMap^[PageNo] do
 begin
   case PageFlags of
    pgIterData  : @UnpFunc := @UnpackMethod1;
    pgIterData2 : @UnpFunc := @UnpackMethod2;
    pgIterData3 : @UnpFunc := @UnpackMethod3;
    else exit;
   end;
   pD := Pages^[pred(PageNo)];
   if @UnpFunc <> nil then
   begin
     GetMem(uD, Header.lxPageSize); J := Header.lxPageSize;
     rc := UnpFunc(pD^, uD^, PageSize, J);
     if not rc then
     begin
       FreeMem(uD, Header.lxPageSize);
       uD:=nil;
     end else
       UnpPageSize:=J;
   end;
 end;
 if uD=nil then UnpPageSize:=0;
 UnpackPageNoTouch := uD;
end;

procedure tLX.Unpack;
var
 I : Integer;
begin
 For I := 1 to Header.lxMpages do UnpackPage(I,AllowTrunc);
end;

procedure tLX.Pack;
const
    maxLen  : array[0..2] of Byte = (1, 16, 255);
var
 I,S1,S2,S3 : Longint;
 Bf1,Bf2,Bf3: Pointer;

Procedure SetPage(var oD : Pointer; nD : Pointer; var oS : Word16; nS : Longint);
begin
 FreeMem(oD, oS); oS := nS;
 GetMem(Pages^[pred(I)], nS);
 Move(nD^, oD^, nS);
end;

begin
{ Now pack fixup records }
 PackFixups(packFlags);
{ Remove empty pages }
 RemoveEmptyPages(AllowTrunc);
 if packFlags and (pkfRunLength or pkfLempelZiv or pkfSixPack) = 0 then exit;

 GetMem(Bf1, Header.lxPageSize);
 GetMem(Bf2, Header.lxPageSize);
 GetMem(Bf3, Header.lxPageSize);
 For I := 1 to Header.lxMPages do
  with ObjMap^[I] do
   if (PageFlags = pgValid) and (PageSize > 0)
    then begin
          if @Progress <> nil then Progress(pred(I), Header.lxMPages);
          S1 := Header.lxPageSize; S2 := Header.lxPageSize; S3 := Header.lxPageSize;
          if (packFlags and pkfRunLength = 0) or
             (not PackMethod1(Pages^[pred(I)]^, Bf1^, PageSize, S1, maxLen[packFlags and pkfRunLengthLvl]))
           then S1 := $7FFFFFFF;
          if (packFlags and pkfLempelZiv = 0) or
             (not PackMethod2(Pages^[pred(I)]^, Bf2^, PageSize, S2))
           then S2 := $7FFFFFFF;
          if (packFlags and pkfSixPack = 0) or (PageSize < 64) or
             (not PackMethod3(Pages^[pred(I)]^, Bf3^, PageSize, S3))
           then S3 := $7FFFFFFF;
          if (S3 < S2) and (S3 < S1) and (S3 < PageSize)
           then begin
                 PageFlags := pgIterData3;
                 SetPage(Pages^[pred(I)], Bf3, PageSize, S3);
                end
           else
          if (S1 < S2) and (S1 < PageSize) {RL-coding is effective enough?}
           then begin
                 PageFlags := pgIterData;
                 SetPage(Pages^[pred(I)], Bf1, PageSize, S1);
                end
           else
          if (S2 < Header.lxPageSize)                  {May be LZ77 done something?}
           then begin
                 PageFlags := pgIterData2;
                 SetPage(Pages^[pred(I)], Bf2, PageSize, S2);
                end;
         end;
 if @Progress <> nil then Progress(1, 1);
 FreeMem(Bf3, Header.lxPageSize);
 FreeMem(Bf2, Header.lxPageSize);
 FreeMem(Bf1, Header.lxPageSize);
end;

procedure tLX.DeletePage;

procedure Del(var P : pLongArray; Item,Count : Longint);
var
 N : pLongArray;
begin
 GetMem(N, pred(Count) * sizeOf(Longint));
 Move(P^, N^, pred(Item) * sizeOf(Longint));
 Move(P^[Item], N^[pred(Item)], (Count - Item) * sizeOf(Longint));
 FreeMem(P, Count * sizeOf(Longint));
 P := N;
end;

var
 I  : Longint;
 NF : pByteArray;
 NM : pArrOfOM;
begin
 if (PageNo > Header.lxMPages) or (PageNo = 0) then exit;
 FreeMem(Pages^[pred(PageNo)], ObjMap^[PageNo].PageSize);
 Del(pLongArray(Pages), PageNo, Header.lxMPages);
 if PerPageCRC <> nil
  then Del(PerPageCRC, PageNo, Header.lxMPages);

 GetMem(NM, pred(Header.lxMpages) * sizeOf(tObjMapRec));
 Move(ObjMap^, NM^, pred(PageNo) * sizeOf(tObjMapRec));
 Move(ObjMap^[succ(PageNo)], NM^[PageNo], (Header.lxMpages - PageNo) * sizeOf(tObjMapRec));
 FreeMem(ObjMap, Header.lxMpages * sizeOf(tObjMapRec));
 ObjMap := NM;

{ remove fixups for this page }
 FreeMem(FixRecTbl^[pred(PageNo)], FixRecSize^[pred(PageNo)]);
 Del(pLongArray(FixRecTbl), PageNo, Header.lxMPages);
 Del(FixRecSize, PageNo, Header.lxMPages);

 For I := pred(Header.lxMPages) downto 0 do
  if PageOrder^[I] = PageNo
   then Del(PageOrder, succ(I), Header.lxMPages)
   else
  if PageOrder^[I] > PageNo
   then Dec(PageOrder^[I]);

 For I := 1 to Header.lxObjCnt do
  with ObjTable^[I] do
   if PageNo >= oPageMap
    then if PageNo < oPageMap + oMapSize
          then Dec(oMapSize)
          else
    else Dec(oPageMap);

 Dec(Header.lxMPages);
end;

procedure tLX.MinimizePage;
var
 dOf : Longint;
 P   : pByteArray;
begin
 if (PageNo > Header.lxMPages) or (PageNo = 0) then exit;
 with ObjMap^[PageNo] do
  if PageFlags = pgValid
   then begin
         dOf := PageSize - MemScanBwd(Pages^[pred(PageNo)]^, PageSize, 0);
         dOf := (dOf + pred(1 shl Header.lxPageShift)) and
                ($FFFFFFFF shl Header.lxPageShift);
         if PageSize <> dOf
          then begin
                GetMem(P, dOf);
                Move(Pages^[pred(pageNo)]^, P^, MinL(dOf, PageSize));
                if dOf > PageSize
                 then FillChar(P^[PageSize], dOf - PageSize, 0);
                FreeMem(Pages^[pred(pageNo)], PageSize);
                Pages^[pred(pageNo)] := P;
                PageSize := dOf;
               end;
        end;
end;

function tLX.UsedPage;
var
 I : Longint;
begin
 For I := 1 to Header.lxObjCnt do
  with ObjTable^[I] do
   if (PageNo >= oPageMap) and (PageNo < oPageMap + oMapSize)
    then begin UsedPage := TRUE; exit; end;
 UsedPage := FALSE;
end;

procedure tLX.RemoveEmptyPages;
var
 I,J : Integer;
begin
{ Minimize space occupied by all pages }
 if AllowMinimize then
  For I := 1 to Header.lxMpages do MinimizePage(I);
{ Remove all absolutely empty pages at ends of objects }
 For I := 1 to Header.lxObjCnt do
  with ObjTable^[I] do
   For J := pred(oPageMap + oMapSize) downto oPageMap do
    with ObjMap^[J] do
     if ((PageFlags = pgValid) or (PageFlags = pgIterData) or (PageFlags = pgIterData2) or
      (PageFlags = pgIterData3)) and (PageSize = 0) and (FixRecSize^[pred(J)] = 0)
        then DeletePage(J)
        else break;
end;

function tLX.isPacked;
var
 i,j,k,l,
 f,cp  : Longint;
 pl    : pLong;
 NTR   : pNameTblRec;
 EP,NP : pEntryPoint;
 ps    : Byte;
begin
 isPacked := TRUE;
 if (newAlign <> NoRealign) and (newAlign <> header.lxPageShift) then isPacked := FALSE;
 if (newStubSize <> -1) and (newStubSize <> StubSize) then isPacked := FALSE;
 if newAlign <> NoRealign then ps := newAlign else ps := header.lxPageShift;

 cp := StubSize + sizeOf(Header);
{ Remove empty pages }
 RemoveEmptyPages(false);
{ Now pack fixup records }
{ PackFixups(packFlags); }

 if ObjTable <> nil
  then begin
        if Header.lxObjTabOfs <> cp - StubSize then isPacked := FALSE;
        Inc(cp, Header.lxObjCnt * sizeOf(tObjTblRec));
       end;

 if ObjMap <> nil
  then begin
        if Header.lxObjMapOfs <> cp - StubSize then isPacked := FALSE;
        Inc(cp, Header.lxMpages * sizeOf(tObjMapRec));
       end;

 if RsrcTable <> nil
  then begin
        if Header.lxRsrcTabOfs <> cp - StubSize then isPacked := FALSE;
        Inc(cp, Header.lxRsrcCnt * sizeOf(tResource));
       end;

 if (Header.lxResTabOfs <> 0) and (Header.lxResTabOfs <> cp - StubSize)
  then isPacked := FALSE;
 For I := 1 to ResNameTbl^.Count do
  begin
   NTR := ResNameTbl^.At(pred(I));
   Inc(cp, succ(length(NTR^.Name^)) + sizeOf(Word16));
  end;
 Inc(cp);

 if (Header.lxEntTabOfs <> 0) and (Header.lxEntTabOfs <> cp - StubSize)
  then isPacked := FALSE;

 I := 1;
 While I <= EntryTbl^.Count do
  begin
   J := I;
   EP := pEntryPoint(EntryTbl^.At(pred(I)));
   if I > 1
    then begin
          NP := pEntryPoint(EntryTbl^.At(I-2));
          K := pred(EP^.Ordinal - NP^.Ordinal);
         end
    else K := pred(EP^.Ordinal);
   While K > 0 do
    begin
     Inc(cp, 2);
     Dec(K, MinL(K, 255));
    end;
   K := EP^.Ordinal;
   repeat
    Inc(J); Inc(K);
    if (J > EntryTbl^.Count) or (J - I >= 255)
     then break;
    NP := pEntryPoint(EntryTbl^.At(pred(J)));
   until (NP^.Ordinal <> K) or
         (EP^.BndType <> NP^.BndType) or
        ((EP^.BndType <> btEmpty) and
         (EP^.Obj <> NP^.Obj));
   K := BundleRecSize(EP^.BndType);
   if EP^.BndType = btEmpty
    then Inc(cp, sizeOf(Byte) * 2)
    else Inc(cp, sizeOf(tEntryTblRec));
   Inc(cp, (J - I) * K);
   I := J;
  end;
 Inc(cp);

 if ModDirTbl <> nil
  then begin
        if Header.lxDirTabOfs <> cp - StubSize then isPacked := FALSE;
        Inc(cp, Header.lxDirCnt * sizeOf(tResource));
       end;

 if PerPageCRC <> nil
  then begin
        if Header.lxPageSumOfs <> cp - StubSize then isPacked := FALSE;
        Inc(cp, Header.lxMpages * sizeOf(Longint));
       end;

 if Header.lxLdrSize <> cp - Header.lxObjTabOfs - StubSize then isPacked := FALSE;

{ Write page fixup table }
 L := cp;

 if (Header.lxFPageTabOfs <> 0) and (Header.lxFPageTabOfs <> cp - StubSize)
  then isPacked := FALSE;
 Inc(cp, succ(Header.lxMpages) * sizeOf(Longint));

 if (Header.lxFRecTabOfs <> 0) and (Header.lxFRecTabOfs <> cp - StubSize)
  then isPacked := FALSE;
 For I := 1 to Header.lxMPages do
  Inc(cp, FixRecSize^[pred(I)]);

 if (Header.lxImpModOfs <> 0) and (Header.lxImpModOfs <> cp - StubSize)
  then isPacked := FALSE;
 For I := 1 to ImpModTbl^.Count do
  if ImpModTbl^.At(pred(I)) <> nil
   then Inc(cp, succ(length(pString(ImpModTbl^.At(pred(I)))^)))
   else Inc(cp);

 if (Header.lxImpProcOfs <> 0) and (Header.lxImpProcOfs <> cp - StubSize)
  then isPacked := FALSE;
 For I := 1 to ImpProcTbl^.Count do
  if ImpProcTbl^.At(pred(I)) <> nil
   then Inc(cp, succ(length(pString(ImpProcTbl^.At(pred(I)))^)))
   else Inc(cp);

 if Header.lxFixupSize <> cp - L then isPacked := FALSE;

 case SaveFlags and svfAlignFirstObj of
  svfFOalnNone   : ;
  svfFOalnShift  : cp := (cp + pred(1 shl ps)) and
                         ($FFFFFFFF shl ps);
  svfFOalnSector : cp := (cp + 511) and $FFFFFE00;
 end;
 if (Header.lxDataPageOfs <> 0) and (Header.lxDataPageOfs <> cp)
  then isPacked := FALSE;
 Header.lxDataPageOfs := cp;
 f := 0;
 For I := 1 to Header.lxMpages do
  begin
   K := PageOrder^[pred(I)];
   with ObjMap^[K] do
    begin
     case PageFlags of
      pgValid     : begin
                     pL := @Header.lxDataPageOfs;
                     if PageSize > 6 then f := f or 1;
                    end;
      pgIterData,
      pgIterData2,
      pgIterData3 : begin
                     if Header.lxIterMapOfs <> Header.lxDataPageOfs then isPacked := FALSE;
                     Header.lxIterMapOfs := Header.lxDataPageOfs;
                     pL := @Header.lxIterMapOfs;
                     case PageFlags of
                      pgIterData  : f := f or 2;
                      pgIterData2 : f := f or 4;
                      pgIterData3 : f := f or 8;
                     end;
                    end;
      pgInvalid,
      pgZeroed    : pL := nil;
      else isPacked := FALSE;
     end;
     if (PageSize > 0) and (pL <> nil)
      then begin
            if (Pages^[pred(K)] = nil) and (PageSize <> 0) then isPacked := FALSE;
            L := (cp - pL^ + pred(1 shl ps)) and ($FFFFFFFF shl ps);
            cp := pL^ + L;
            if PageDataOffset <> L shr ps then isPacked := FALSE;
            Inc(cp, PageSize);
           end;
    end;
  end;
 if (packFlags and pkfLempelZiv <> 0) and
    (f and 4 = 0) and
    (f and 1 <> 0)
  then isPacked := FALSE;
 if (packFlags and pkfRunLength <> 0) and
    (packFlags and pkfLempelZiv = 0) and
    (f and 2 = 0) and
    (f and 1 <> 0)
  then isPacked := FALSE;
 if (packFlags and pkfSixPack <> 0) and
    (f and 8 = 0) and
    (f and 1 <> 0)
  then isPacked := FALSE;

 if NResNameTbl^.Count > 0
  then begin
        if Header.lxNResTabOfs <> cp then isPacked := FALSE;
        For I := 1 to NResNameTbl^.Count do
         begin
          NTR := NResNameTbl^.At(pred(I));
          Inc(cp, succ(length(NTR^.Name^)) + sizeOf(Word16));
         end;
        Inc(cp);
        if Header.lxNResTabSize <> cp - Header.lxNResTabOfs then isPacked := FALSE;
       end;

 if (oldDbgOfs <> 0) or (Header.lxDebugInfoOfs <> 0)
  then if (Header.lxDebugInfoOfs <> cp) or (Header.lxDebugInfoOfs <> oldDbgOfs)
        then isPacked := FALSE
        else Inc(cp, Header.lxDebugLen);

 NewSize := cp;
end;

destructor tLX.Destroy;
begin
 freeModule;
end;

end.
