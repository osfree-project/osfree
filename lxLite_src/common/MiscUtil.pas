{$A-,B-,D+,E-,F-,G+,I-,L+,N-,O-,P-,Q-,R-,S-,T-,V-,X+}
{&AlignCode-,AlignData-,AlignRec-,G3+,Speed-,Frame-,Use32+}
Unit miscUtil;

Interface

const
{$ifDef OS2}
 tickerFreq             = 100;  {ticker frequence in 100000/Freq (1000)}
{$else}
 tickerFreq             = 5494; { = 100000/18.2 }
{$endIf}

type
  SmallWord  = System.Word;
  SmallInt   = System.Integer;
  Word16     = System.Word;
  Integer16  = System.Integer;
  Integer32  = System.Longint;
  Word32     = System.Longint;
{$IFDEF USE32}
  Integer    = System.Longint;
  Word       = System.Longint;
const
  MaxInt     = MaxLongint;
{$ENDIF}

type
 tByteArray      = array[0..65500] of Byte;
 pByteArray      = ^tByteArray;
 tWordArray      = array[0..32700] of Word;
 pWordArray      = ^tWordArray;
 tSmallWordArray = array[0..32700] of SmallWord;
 pSmallWordArray = ^tSmallWordArray;
 pWord16Array    = ^tWord16Array;
 tWord16Array    = array[0..32700] of Word16;
 tLongArray      = array[0..16380] of Longint;
 pLongArray      = ^tLongArray;
 tPointerArray   = array[0..16380] of Pointer;
 pPointerArray   = ^tPointerArray;
 tCharArray      = array[0..65500] of Char;
 pCharArray      = ^tCharArray;
 pByte           = ^Byte;
 pWord           = ^Word;
 pSmallWord      = ^SmallWord;
 pWord16         = ^Word16;
 pSmallInt       = ^SmallInt;
 pInteger16      = ^Integer16;
 pWord32         = ^Word32;
 pInteger        = ^Integer;
 pLong           = ^Longint;
 pInteger32      = ^Integer32;
 pString         = ^String;

 { Compare function for QuickSort. Return TRUE if Buff[N1] is 'greater    }
 { or _EQUAL_' than Buff[N2]. If it will return simply 'less' or 'greater'}
 { the function will cause a protection failure in the best case. If you  }
 { use the QuickSort with CmpFunc, you don`t need to specify element size }
 tCmpFunc        = Function(var Buff; N1,N2 : Word) : boolean;
 { Procedure which exchanges element N1 with N2 in Buff for QuickSort     }
 { Note that N1 and N2 in both cases are zero-based                       }
 tXchgProc       = Procedure(var Buff; N1,N2 : Word);

 { tObject base object, stolen :-) from TurboVision }
 pObject = ^tObject;
 tObject = object
  Constructor Create;
  Procedure   Zero;
  Procedure   Free; virtual;
  Destructor  Destroy; virtual;
{$ifDef OS2}
 {Multi-thread support: detach an object.function(Parm : Pointer) : longint}
  function    Detach(ObjFunc, Parm : Pointer; StackSize : Word; Ready : boolean) : Longint;
{$endIf}
 end;

{ Dynamic string [de]allocation routines }
 Function  NewStr(const S : String) : pString;
 Procedure DisposeStr(P : PString);
{ Return interval in minutes between two events }
 Function  TimeInterval(sYear,sMonth,sDay,sHour,sMin,
                        fYear,fMonth,fDay,fHour,fMin : Word) : Longint;

{ Quick sort routine. Element size must be one of 1,2,4 if cmpFunc is }
{ not provided (is nil) - the array will be sorted in usual way (i.e. }
{ from minimal to maximal value. If you use cmpFunc it`s also in your }
{ responsability to supply a valid xchgProc pointer.                  }
{ NOTE THAT BYTES, WORDS AND DWORDS WILL BE TREATED AS UNSIGNED VALUE }
{$ifDef use32}
 Procedure QuickSort(var Buff; First,Last : Word; ElementSize : Byte;
                     cmpFunc : tCmpFunc; xchgProc : tXchgProc);
{$endIf}

{ Bit-manipulation routines }
{&saves ebx,edx,esi,edi}
 Function  bitTest(var bitArray; BitNo : Word) : boolean;
 Procedure bitSet(var bitArray; BitNo : Word);
 Procedure bitReset(var bitArray; BitNo : Word);

{Bit scan forward/reverse functions; return 255 if no set bits in A}
{&saves all}
 Function  BitSF(A : Longint) : Byte;
 Function  BitSR(A : Longint) : Byte;

{ Min/max functions }
{&saves all}
 Function  minL(A,B : Longint) : Longint;
{$ifDef VirtualPascal}
 Inline; begin if A <= B then minL := A else minL := B; end;
{$endIf}
 Function  maxL(A,B : Longint) : Longint;
{$ifDef VirtualPascal}
 Inline; begin if A >= B then maxL := A else maxL := B; end;
{$endIf}
 Function  minI(A,B : Integer) : Integer;
{$ifDef VirtualPascal}
 Inline; begin if A <= B then minI := A else minI := B; end;
{$endIf}
 Function  maxI(A,B : Integer) : Integer;
{$ifDef VirtualPascal}
 Inline; begin if A >= B then maxI := A else maxI := B; end;
{$endIf}

{ The same as System.Move but address always increments (i.e. CLD)   }
{ Use it ONLY when there is no possibility of overlapping A & B      }
{ or you intentionaly want to overlap them (i.e. in un/packers)      }
{ NOTE: IT DOES NOT OPTIMIZE MOVES BY DWORDS/WORDS: DONT CHANGE THIS }
{&saves eax,ebx,edx,esi,edi}
 Procedure linearMove(var A,B; Size : Word);

{ Exchange bytes, words, double words and vars of any size }
{&saves ebx,esi,edi}
 Procedure XchgB(var A,B);
 Procedure XchgW(var A,B);
 Procedure XchgL(var A,B);
 Procedure Xchg(var A,B; Size : Word);

{ Compare two strings; return 0 if equal; positive if A>B and negative if A<B }
{&saves ebx,edx,esi,edi}
 Function  MemCmp(var A,B; Size : Word) : Shortint;

{ Search buffer Buff of length BuffLen for a Target of given length TargetLen }
{&saves ebx,esi,edi}
 Function  Search(var Buff; BuffLen : Word; var Target; TargetLen : Word) : Word;

{ Return number of "Value" bytes beginning from Buff to Buff+BuffLen }
{&saves ebx,esi,edi}
 Function MemScanFwd(var Buff; BuffLen : Word; Value : byte) : Word;
{ Return number of "Value" bytes beginning from Buff+BuffLen down to Buff }
{&saves ebx,esi,edi}
 Function MemScanBwd(var Buff; BuffLen : Word; Value : byte) : Word;

{$ifDef use32}
{&saves ebx,esi,edi}
 Function bTicker : Byte;
 Function wTicker : Word;
 Function lTicker : Longint;
{$else use32}
 Function bTicker : Byte;
 InLine( $8E/$06/seg0040/   { mov   es,seg0040   }
         $26/$A0/$6C/$00);  { mov   ax,es:[6Ch] }
 Function wTicker : Word;
 InLine( $8E/$06/seg0040/   { mov   es,seg0040   }
         $26/$A1/$6C/$00);  { mov   ax,es:[6Ch] }
 Function lTicker : Longint;
 InLine( $8E/$06/seg0040/   { mov   es,seg0040   }
         $26/$A1/$6C/$00/   { mov   ax,es:[6Ch] }
         $26/$8B/$16/$6E/$00);{mov  dx,es:[6Eh] }
{$endIf use32}

{ Trick: call a embedded (ONLY level 2!) procedure }
{&saves ebx,esi,edi}
 Function level2call(Proc,Info : Pointer) : boolean;

Implementation {$ifDef os2} uses os2base; {$endIf}

{±±±±±±±±±±±±±±±±±±±±±±±±±±±± High-level functions ±±±±±±±±±±±±±±±±±±±±±±±±±±}
constructor tObject.Create;
begin
 Zero;
end;

procedure tObject.Zero;
type Image = record
      Link : Word;
      Data : record end;
     end;
begin
 FillChar(Image(Self).Data, SizeOf(Self) - SizeOf(TObject), 0);
end;

{$ifDef OS2}
type
     pThreadParmBlock = ^tThreadParmBlock;
     tThreadParmBlock = record
      Func,Self,Parm : Pointer;
     end;

function stubThread(ParmBlock : Pointer) : Longint; assembler;
{&FRAME-} {&USES none} {&SAVES ebx,esi,edi}
asm             mov    eax,ParmBlock
                push   [eax].tThreadParmBlock.Parm
                push   [eax].tThreadParmBlock.Self
                push   [eax].tThreadParmBlock.Func
                push   eax
                push   type tThreadParmBlock
                call   _MemFree
                pop    eax
                call   eax
end;

function tObject.Detach;
var
 tpb : pThreadParmBlock;
 tid : Longint;
begin
 New(tpb);
 tpb^.Func := ObjFunc;
 tpb^.Self := @Self;
 tpb^.Parm := Parm;
 tid := 0;
 BeginThread(nil, StackSize, stubThread, tpb,
  create_Suspended * byte(not Ready) + stack_Sparse, tid);
 Detach := tid;
end;
{$endIf}

procedure tObject.Free;
begin
end;

destructor tObject.Destroy;
begin
 Free;
end;

const EmptyStr : string[1] = '';

Function NewStr;
var p : PString;
begin
 if S = ''
  then p := @EmptyStr                { to allow length() call on empty strings }
  else begin
        GetMem(p, succ(length(S)));
        p^ := S;
       end;
 NewStr := p;
end;

Procedure DisposeStr;
begin
 if p<>@EmptyStr then FreeMem(p, succ(integer(length(p^))));
 p := nil;
end;

Function TimeInterval(sYear,sMonth,sDay,sHour,sMin,
                      fYear,fMonth,fDay,fHour,fMin : Word) : Longint;
const
    daysInMonth      : array[1..12] of Byte =
    (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
var cY,cM,cD,cH,cMin : Word;
    tI               : Longint;

procedure IncMonth;
begin
 Inc(cM);
 if cM > 12 then begin cM := 1; Inc(cY); end;
end;

procedure IncDay;
begin
 Inc(cD);
 if cD > daysInMonth[cM] + byte((cM = 2) and (cY and 3 = 0))
  then begin cD := 1; IncMonth; end;
end;

procedure IncHour;
begin
 Inc(cH);
 if cH > 23 then begin cH := 0; IncDay; end;
end;

procedure IncMin;
begin
 Inc(cMin);
 if cMin > 59 then begin cMin := 0; IncHour; end;
end;

begin
 cY := sYear; cM := sMonth; cD := sDay;
 cH := sHour; cMin := sMin; tI := 0;
 While (cY <> fYear) or (cM <> fMonth) or (cD <> fDay) or
       (cH <> fHour) or (cMin <> fMin) do
  begin
   if (cY <> fYear) and (cM = 1) and (cD = 1) and (cH = 0) and (cMin = 0)
    then begin
          Inc(tI, longint(365 + byte(cY and 3 = 0)) * 1440);
          Inc(cY);
         end
    else
   if ((cY <> fYear) or (cM <> fMonth)) and (cD = 1) and (cH = 0) and (cMin = 0)
    then begin
          Inc(tI, longint(daysInMonth[cM] + byte((cM = 2) and (cY and 3 = 0))) * 1440);
          IncMonth;
         end
    else
   if ((cY <> fYear) or (cM <> fMonth) or (cD <> fDay)) and (cH = 0) and (cMin = 0)
    then begin
          Inc(tI, 1440); IncDay;
         end
    else
   if ((cY <> fYear) or (cM <> fMonth) or (cD <> fDay) or (cH <> fHour)) and (cMin = 0)
    then begin
          Inc(tI, 60); IncHour;
         end
    else begin
          Inc(tI); IncMin;
         end;
  end;
 TimeInterval := tI;
end;

{±±±±±±±±±±±±±±±±±±±±±±± Compiler-specific functions ±±±±±±±±±±±±±±±±±±±±±±±}
{$ifDef use32}

Function lTicker : Longint;
var L : Longint;
begin
 DosQuerySysInfo(qsv_Ms_Count, qsv_Ms_Count, L, SizeOf(L));
 lTicker := L;
end;

Function wTicker : Word;
begin
 wTicker := lTicker and $FFFF;
end;

Function bTicker : Byte;
begin
 bTicker := lTicker and $FF;
end;


Procedure QuickSort;

function CmpInt(var Buff; N1,N2:Word):boolean;
begin
  case ElementSize of
  1: cmpInt := tByteArray(Buff)[N1] >= tByteArray(Buff)[N2];
  2: cmpInt := tWord16Array(Buff)[N1] >= tWord16Array(Buff)[N2];
  4: cmpInt := tWordArray(Buff)[N1] >= tWordArray(Buff)[N2];
  end;
end;

procedure XchgInt(var Buff; N1,N2:Word);
begin
  case ElementSize of
  1: xchgB(tByteArray(Buff)[N1],tByteArray(Buff)[N2]);
  2: xchgW(tWord16Array(Buff)[N1],tWord16Array(Buff)[N2]);
  4: xchgL(tWordArray(Buff)[N1],tWordArray(Buff)[N2]);
  end;
end;

var i,med,c:Word;

procedure QuickSort2(First,Last:Word);
var j:Word;
begin
    c := succ(Last-First);
    if c<=1 then Exit;
    med := First + Random(c);
    i := First; j := Last;
    while true do
    begin
        while (i<j) and CmpFunc(Buff,j,med) do dec(j);
        while (i<j) and not CmpFunc(Buff,i,med) do inc(i);
        if i>=j then break;
        { swapping [i] and [j] values; update med }
        if med=i then med:=j else if med=j then med:=i;
        XchgProc(Buff,i,j);
    end;
    QuickSort2(First,j);
    QuickSort2(j+1,Last);
end;


begin
    if @CmpFunc=nil then
        case ElementSize of
          1,2,4:  // CmpFunc := CmpInt;
          asm
            mov eax, offset CmpInt
            mov CmpFunc, eax
          end;
          else Exit;
        end;

    if @XchgProc=nil then
        case ElementSize of
          1,2,4: // XchgProc := xchgInt;
          asm
            mov eax, offset XchgInt
            mov XchgProc, eax
          end;
          else Exit;
        end;

    QuickSort2(First,Last);
end;


{±±±±±±±±±±±±±±±±±±±±±±±±±±±± Low-level functions ±±±±±±±±±±±±±±±±±±±±±±±±±±±}
Function bitTest; assembler;
asm             mov     eax,bitArray
                mov     ecx,bitNo
                bt      [eax],ecx
                setc    al
end;

Procedure bitSet; assembler;
asm             mov     eax,bitArray
                mov     ecx,bitNo
                bts     [eax],ecx
end;

Procedure bitReset; assembler;
asm             mov     eax,bitArray
                mov     ecx,bitNo
                btr     [eax],ecx
end;

Procedure XchgB(var a,b); assembler;
asm             mov     ecx,A
                mov     edx,B
                mov     al,[ecx]
                xchg    al,[edx]
                mov     [ecx],al
end;

Procedure XchgW(var a,b); assembler;
asm             mov     ecx,A
                mov     edx,B
                mov     ax,[ecx]
                xchg    ax,[edx]
                mov     [ecx],ax
end;

Procedure XchgL; assembler;
asm             mov     ecx,A
                mov     edx,B
                mov     eax,[ecx]
                xchg    eax,[edx]
                mov     [ecx],eax
end;

{&uses ebx}
Procedure Xchg(var A,B; Size : Word); assembler;
asm             mov     ebx,A
                mov     edx,B
                mov     ecx,Size
@@next:         mov     al,[ebx]
                xchg    al,[edx]
                mov     [ebx],al
                inc     ebx
                inc     edx
                loop    @@next
end;

{&uses esi,edi}
Procedure linearMove; assembler;
asm             cld
                mov     esi,A
                mov     edi,B
                mov     ecx,Size
                rep     movsb
end;

{&uses none}
Function BitSF; assembler;
asm             mov     eax,A
                bsf     eax,eax
                jnz     @@ok
                dec     al
@@ok:
end;

Function BitSR; assembler;
asm             mov     eax,A
                bsr     eax,eax
                jnz     @@ok
                dec     al
@@ok:
end;

{&uses esi,edi}
Function MemCmp; assembler;
asm             cld
                mov    esi,A
                mov    edi,B
                mov    ecx,Size
                repe   cmpsb
                lahf
                mov    al,ah
                and    al,0C0h         {Dirty trick :)}
                xor    al,040h
end;

{&uses esi,edi}
Function Search; assembler;
asm             cld
                mov     edi,Buff
                mov     ecx,BuffLen
                sub     ecx,TargetLen
                jbe     @@notFound
                inc     ecx
                mov     esi,Target
                mov     al,[esi]
@@Scan:         repne   scasb
                jne     @@notFound
                push    ecx
                push    esi
                push    edi
                dec     edi
                mov     ecx,TargetLen[4+4+4]
                repe    cmpsb
                pop     edi
                pop     esi
                pop     ecx
                jne     @@Scan
                dec     edi
                sub     edi,Buff
                mov     eax,edi
                jmp     @@locEx
@@notFound:     mov     eax,-1
@@locEx:
end;

{&uses none}
function level2call(Proc,Info : Pointer) : boolean; assembler;
asm             push    ebp
                mov     ebp,[ebp]
                push    Info[4]
                call    Proc[8]
                pop     ebp
end;

{&uses edi}
Function MemScanFwd; assembler;
asm             cld
                mov     edi,Buff
                mov     ecx,BuffLen
                mov     al,Value
                repe    scasb
                setne   al
                movzx   eax,al
                sub     edi,eax
                sub     edi,Buff
                mov     eax,edi
end;

{&uses edi}
Function MemScanBwd; assembler;
asm             std
                mov     edi,Buff
                mov     ecx,BuffLen
                jecxz   @@zeroLen
                lea     edi,[edi+ecx-1]
                mov     al,Value
                repe    scasb
                setnz   al
                movzx   eax,al
                add     edi,eax
                inc     edi
@@zeroLen:      sub     edi,Buff
                mov     eax,BuffLen
                sub     eax,edi
                cld
end;
{&uses none}
{$else €€€€€€€€€€€ Implementation specific for Borland Pascal 7.0 €€€€€€€€€€€}

Procedure bitSet; assembler;
asm             les     di,bitArray
                mov     bx,bitNo
                cmp     test8086,2
                jb      @@8086
                db      $26,$0F,$AB,$1D {bts es:[di],bx}
                jmp     @@locEx

@@8086:         mov     cl,bl
                shr     bx,1
                shr     bx,1
                shr     bx,1
                and     cl,7
                mov     al,00000001b
                rol     al,cl
                or      es:[bx+di],al
@@locEx:
end;

Procedure bitReset; assembler;
asm             les     di,bitArray
                mov     bx,bitNo
                cmp     test8086,2
                jb      @@8086
                db      $26,$0F,$B3,$1D {btr es:[di],bx}
                jmp     @@locEx

@@8086:         mov     cl,bl
                shr     bx,1
                shr     bx,1
                shr     bx,1
                and     cl,7
                mov     al,11111110b
                rol     al,cl
                and     es:[bx+di],al
@@locEx:
end;

Function bitTest; assembler;
asm             les     di,bitArray
                mov     bx,bitNo
                cmp     test8086,2
                jb      @@8086
                db      $26,$0F,$A3,$1D {bt es:[di],bx}
                db      $0F,$92,$C0     {setc al}
                jmp     @@locEx

@@8086:         mov     cl,bl
                shr     bx,1
                shr     bx,1
                shr     bx,1
                and     cl,7
                mov     al,00000001b
                rol     al,cl
                test    es:[bx+di],al
                mov     al,0
                jz      @@locEx
                inc     al
@@locEx:
end;

Function bitSF; assembler;
asm      db $66;mov     ax,A.word
         db $66,$0F,$BC,$C0 {bsf eax,eax}
                jnz     @@ok
                dec     al
@@ok:
end;

Function bitSR; assembler;
asm      db $66;mov     ax,A.word
         db $66,$0F,$BD,$C0 {bsr eax,eax}
                jnz     @@ok
                dec     al
@@ok:
end;

Function MemCmp; assembler;
asm             cld
                push    ds
                lds     si,A
                les     di,B
                mov     cx,Size
                repe    cmpsb
                pop     ds
                lahf
                mov     al,ah
                and     al,0C0h {Dirty trick :)}
                xor     al,040h
end;

Function Search; assembler;
asm             cld
                push    ds
                les     di,Buff
                mov     cx,BuffLen
                sub     cx,TargetLen
                jbe     @@NotFound
                inc     cx
                lds     si,Target
                mov     al,[si]
@@Scan:         repne   scasb
                jne     @@NotFound
                push    cx
                push    si
                push    di
                dec     di
                mov     cx,TargetLen
                repe    cmpsb
                pop     di
                pop     si
                pop     cx
                jne     @@Scan
                dec     di
                sub     di,Buff.word
                mov     ax,di
                jmp     @@locEx
@@NotFound:     mov     ax,-1
@@locEx:        pop     ds
end;

Procedure XchgB(var a,b); assembler;
asm             push    ds
                les     di,a
                lds     si,b
                mov     al,es:[di]
                xchg    al,ds:[si]
                cld
                stosb
                pop     ds
end;

Procedure XchgW(var a,b); assembler;
asm             push    ds
                les     di,a
                lds     si,b
                mov     ax,es:[di]
                xchg    ax,ds:[si]
                cld
                stosw
                pop     ds
end;

Procedure XchgL(var a,b); assembler;
asm             push    ds
                les     di,a
                lds     si,b
                mov     ax,es:[di]
                xchg    ax,ds:[si]
                cld
                stosw
                mov     ax,es:[di]
                xchg    ax,ds:[si+2]
                stosw
                pop     ds
end;

Procedure Xchg(var A,B; Size : Word); assembler;
asm             push    ds
                lds     si,A
                les     di,B
                mov     cx,Size
@@1:            mov     al,ds:[si]
                xchg    es:[di],al
                mov     ds:[si],al
                inc     si
                inc     di
                loop    @@1
                pop     ds
end;

Procedure linearMove; assembler;
asm             cld
                push    ds
                lds     si,A
                les     di,B
                mov     cx,Size
                rep     movsb
                pop     ds
end;

Function  minL(A,B : Longint) : Longint;
begin
 if A <= B then minL := A else minL := B;
end;

Function  maxL(A,B : Longint) : Longint;
begin
 if A >= B then maxL := A else maxL := B;
end;

Function  minI;
begin
 if A <= B then minI := A else minI := B;
end;

Function  maxI;
begin
 if A >= B then maxI := A else maxI := B;
end;

function level2call(Proc,Info : Pointer) : boolean; assembler;
asm             push    bp
                push    Info[2].word
                push    Info[0].word
                mov     bx,[bp]
                push    ss:[bx].word
                call    Proc
                pop     bp
end;

Function MemScanFwd; assembler;
asm             cld
                les     di,Buff
                mov     cx,BuffLen
                mov     al,Value
                repe    scasb
                je      @@1
                dec     di
@@1:            sub     di,word ptr Buff
                mov     ax,di
end;

Function MemScanBwd; assembler;
asm             std
                les     di,Buff
                mov     cx,BuffLen
                add     di,cx
                dec     di
                mov     al,Value
                repe    scasb
                je      @@1
                inc     di
@@1:            inc     di
                sub     di,word ptr Buff
                mov     ax,BuffLen
                sub     ax,di
                cld
end;
{$endIf €€€€€€€€€€€€€€€€€€€ end of compiler - specific  section €€€€€€€€€€€€€}

end.

