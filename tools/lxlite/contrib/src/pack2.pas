unit pack2;

interface

{$ifndef fpc}
uses use32;
{$endif}

Function Compress  (Length:longint;Source,Destin:PByte):longint;

Function Decompress(Length:longint;Source,Destin:PByte):longint;

implementation
CONST
  TEXTSEARCH =  768;   { Max strings to search in text file - smaller -> Faster compression}
  BINSEARCH  =  192;   { Max strings to search in binary file }
  TEXTNEXT   =   64;   { Max search at next character in text file - Smaller -> better compression }
  BINNEXT    =   16;   { Max search at next character in binary file }
  MAXFREQ    = 2000;   { Max frequency count before table reset }
  MINCOPY    =    3;   { Shortest string COPYING length }
  MAXCOPY    =   66;   { Longest string COPYING length }
  SHORTRANGE =    3;   { Max distance range for shortest length COPYING }
  COPYRANGES =    {6}7;   { Number of string COPYING distance bit ranges @@@}
  CopyBits : Array[0..PRED(COPYRANGES)] OF INTEGER = (4,6,8,10,12,14,14); { Distance bits }
  CODESPERRANGE = (MAXCOPY - MINCOPY + 1);

  NUL = -1;                    { End of linked list marker }
  HASHSIZE = 32768;            { Number of entries in hash table}(*Don`t change*)
  HASHMASK = (HASHSIZE - 1);   { Mask for hash key wrap }

{ Adaptive Huffman variables }
  TERMINATE = 256;             { EOF code }
  FIRSTCODE = 257;             { First code for COPYING lengths }
  MAXCHAR = (FIRSTCODE+COPYRANGES*CODESPERRANGE-1); {704}
  SUCCMAX = (MAXCHAR+1);                            {705}
  TWICEMAX = (2*MAXCHAR+1);                        {1407}
  ROOT = 1;
  MAXBUF = 4096;

{** Bit packing routines **}


TYPE
  Copy_Type = Array[0..PRED(CopyRanges)] OF Integer;


CONST
  {CopyMin : Copy_Type = (0,16,80,336,1360,5456);
  CopyMax : Copy_Type = (15,79,335,1359,5455,21839);
  MaxDistance : Integer = 21839;
  MaxSize = 21839 + MAXCOPY;}   { @@@ }
  CopyMin : Copy_Type = (0,16,80,320,1024,4096,16384);
  CopyMax : Copy_Type = (15,79,319,1023,4095,16383,32768-Maxcopy);
  MaxDistance : LongInt = 32768-Maxcopy;
  MaxSize = 32768;   { @@@ }


TYPE
  HashType    = packed Array[0..PRED(HashSize)] OF longint;
  Hash_Ptr    = ^HashType;
  ListType    = packed Array[0..MaxSize] OF longint;
  List_Ptr    = ^ListType;
  Buffer_Type = Array[0..MaxSize] OF byte;  { Convenient typecast. }
  Buffer_Ptr  = ^Buffer_Type;
  HTree_Type  = packed Array[0..MaxChar] OF longint;
  THTree_Type = packed Array[0..TwiceMax] OF longint;
  WDBufType   = Array[0..PRED(MAXBUF*4)] OF longint;
  WDBufPtr    = ^WDBufType;


VAR
  Distance,Insrt   :longint;
  DictFile, Binary :longint;
  Head, Tail       :Hash_Ptr;             { Hash table          }
  Next, Prev       :List_Ptr;             { Doubly linked lists }
  Buffer           :Buffer_Ptr;           { Text buffer         }
  LeftC, RightC    :HTree_Type;           { Huffman tree        }
  Parent,Freq      :THTree_Type;
  CurBuf           :PByte;
  Input_Bit_Count  :longint;
  Input_Bit_Buffer :longint;
  Output_Bit_Count :longint;
  Output_Bit_Buffer:longint;



{***************** Compression & Decompression *****************}

{ Initialize data for compression or decompression }

Procedure initialize;
var i:longint;
begin
 { Initialize Huffman frequency tree }
 for i:=2 to TWICEMAX do begin Parent[i]:=i shr 1;Freq[i]:=1 end;
 for i:=1 to MAXCHAR  do begin LeftC[i] :=i shl 1;RightC[i]:=i shl 1+1 end;
END;


{********************* Compression Routines ***********************}
{procedure SaveBuffer;
begin
 PLongint(OutBuf)^:=Output_Bit_Buffer;
 inc(l(OutBuf),4);Output_Bit_Count := 0;
end;}


{ Write multibit code to output file }
(*{&SAVES EBX,ESI,EDI}
Procedure Output_Code(Code,Bits:l);assembler;
asm
 mov  edx,Output_Bit_Buffer
 mov  ecx,Bits
 inc  ecx
@loop:
 dec  ecx
 jz   @quit
 rcr  Code,1
 rcl  edx,1
 inc  OutPut_Bit_Count
 cmp  OutPut_Bit_Count,32
 jc   @loop
 mov  eax,OutBuf
 mov  [eax],edx
 add  eax,4
 mov  OutBuf,eax
 mov  Output_Bit_Count,0
 jmp  @loop
@quit:
 mov  Output_Bit_Buffer,edx
end;
*)

{ Flush any remaining bits to output file before closing file }
Procedure Flush_Bits;
begin
 if Output_Bit_Count>0 then
 begin
  Output_Bit_Buffer:=Output_Bit_Buffer shl (32-Output_Bit_Count);
  PLongint(CurBuf)^:=Output_Bit_Buffer;
  inc(longint(CurBuf),4);Output_Bit_Count:= 0;
 end;
END;


{ Update frequency counts from leaf to root }
Procedure Update_Freq(A,B : Integer);
begin
 repeat
  Freq[Parent[A]]:=Freq[A]+Freq[B];
  A:=Parent[A];
  if A<>ROOT then
   if LeftC[Parent[A]]=A then B:=RightC[Parent[A]] else B := LeftC[Parent[A]];
 until A=ROOT;
 { Periodically scale frequencies down by half to avoid overflow }
 { This also provides some local adaption and better compression }
 if Freq[ROOT]=MAXFREQ then for A:=1 to TWICEMAX do Freq[A]:=Freq[A] shr 1;
end;


{ Update Huffman model for each character code }
Procedure Update_Model(Code:longint);
var A,B,C,Ua,Uua:longint;
begin
 A:=Code + SUCCMAX;
 inc(Freq[A]);
 if Parent[A]<>ROOT then
 begin
  ua := Parent[a];
  if LeftC[ua]=a then update_freq(a,RightC[ua]) else update_freq(a,LeftC[ua]);
  repeat
   uua := Parent[ua];
   if LeftC[uua]=ua then b:=RightC[uua] else b:=LeftC[uua];
   { IF high Freq lower in tree, swap nodes }
   if Freq[a]>Freq[b] then
   begin
    Parent[b]:=Ua;
    Parent[a]:=Uua;
    if LeftC[Uua]=ua then RightC[Uua]:=A else LeftC[Uua]:=A;
    if LeftC[ua] =a then begin LeftC[Ua]:=B;C:=RightC[ua] end else
     begin RightC[Ua]:=B;C := LeftC[Ua] end;
    A:=B;
    Update_Freq(B,C);
   end;
   A:=Parent[A];
   Ua:=Parent[A];
  until Ua=ROOT;
 end;
end;


var Stack : Array[0..49] of longint;
{ Compress a character code to output stream }
Procedure _Compress(code: Integer);
var a,_sp:longint;
begin
 _Sp := 0; A:=Code + SUCCMAX;
 repeat
  Stack[_Sp]:=longint(RightC[Parent[A]]=A);inc(_Sp);A:=Parent[A];
 until A=ROOT;
 asm {&saves ebx,esi}
  {$IFNDEF VIRTUALPASCAL}
   push edi
  {$ENDIF}
   mov  ecx,_sp
   mov  edi,offset Stack
   mov  edx,Output_Bit_Buffer
@loop:
   dec  ecx
   js   @quit
   mov  eax,[edi+ecx*4]
   rcr  eax,1
   rcl  edx,1
   inc  OutPut_Bit_Count
   cmp  OutPut_Bit_Count,32
   jc   @loop
   mov  eax,CurBuf
   mov  [eax],edx
   add  eax,4
   mov  CurBuf,eax
   mov  Output_Bit_Count,0
   jmp  @loop
@quit:
   mov  Output_Bit_Buffer,edx
  {$IFNDEF VIRTUALPASCAL}
   pop  edi
  {$ENDIF}
 end;
 Update_Model(Code);
END;



{** Hash table linked list string search routines **}

{ Add node to head of list }
{&SAVES EBX}
Procedure Add_Node(N:longint);pascal;assembler;
asm
       {$IFNDEF VIRTUALPASCAL}
                push    edi
                push    esi
       {$ENDIF}
              @@0:
{ Key := (Buffer^[N] xor (Buffer^[(N+1) and $7FFF] SHL 4)) xor
                    (Buffer^[(N+2) and $7FFF] SHL 8) AND HASHMASK;}
                MOV     ECX,Buffer
                MOV     edi,N
                MOVZX   ESI,BYTE PTR [edi+ECX]
                MOV     EAX,edi
                INC     EAX
                mov     edx,7FFFh
                AND     EAX,edx
                MOVZX   EAX,BYTE PTR [EAX+ECX]
                SHL     EAX,4
                XOR     ESI,EAX
                MOV     EAX,edi
                inc     eax
                inc     eax
                and     eax,edx
                movzx   eax,byte ptr [eax+ecx]
                shl     eax,8
                and     eax,edx
                xor     esi,eax
{ IF (Head^[Key] = NUL) THEN}
                mov     ecx,Head
                CMP     DWORD PTR [ESI*4+ecx],-1
                JNE     @@2
{ BEGIN
   Tail^[Key] := N;}
                mov     eax,Tail
                mov     [esi*4+eax],edi
{  Next^[N] := NUL;}
                MOV     eax,Next
                MOV     DWORD PTR [edi*4+eax],-1
{ END
  ELSE}
                JMP     @@3
              @@2:
{ BEGIN
   Next^[N] := Head^[Key];}
                mov     eax,[esi*4+ecx]
                mov     edx,Next
                mov     [edi*4+edx],eax
{  Prev^[Head^[Key]] := N;}
                mov     eax,Prev
                MOV     edx,[esi*4+ecx]
                MOV     [edx*4+eax],edi
{ END;}
              @@3:
{ Head^[Key] := N;}
                MOV     [ESI*4+ECX],edi
{ Prev^[N] := NUL;}
                MOV     ECX,Prev
                MOV     DWORD PTR [edi*4+ECX],-1
  {$IFNDEF VIRTUALPASCAL}
                pop     esi
                pop     edi
  {$ENDIF}
end;




{ Delete node from tail of list }
Procedure Delete_Node(N:longint);
var  Key:longint;
begin
{ Define hash key function using MINCOPY characters of string prefix }
{  Key := (Buffer^[N] xor (Buffer^[(N+1) mod MaxSize] shl 4)) xor
                   (Buffer^[(N+2) mod Maxsize] shl 8) and HASHMASK;}
 Key:=(Buffer^[N] xor (Buffer^[(N+1) and $7FFF] shl 4)) xor
      (Buffer^[(N+2) and $7FFF] shl 8) and HASHMASK;
 if Head^[Key]=Tail^[Key] then Head^[Key]:=NUL else
 begin
  Next^[Prev^[Tail^[Key]]]:=NUL;
  Tail^[Key]:=Prev^[Tail^[Key]];
 end;
end;


{ Find longest string matching lookahead buffer string }
{&SAVES NONE}
Function Match(N,Depth:longint):longint;pascal;assembler;
var   Index, Dist,Len,Best,Count:longint;
asm
       {$IFNDEF VIRTUALPASCAL}
                push    ebx
                push    edi
                push    esi
       {$ENDIF}
                xor     eax,eax
                mov     Best,eax
                mov     Count,eax
{ IF (N = MaxSize) THEN }
                cmp     N,8000h
                JNE     @@2
              @@1:
{ N := 0; }
                mov     N,eax
              @@2:
{ Key := (Buffer^[N] XOR (Buffer^[(N+1) and $7FFF] SHL 4)) XOR
                    (Buffer^[(N+2) and $7FFF] SHL 8) AND HASHMASK; }
                mov     ecx,Buffer
                mov     edx,N
                mov     al,[edx+ecx]
                inc     edx
                mov     esi,7FFFh
                and     edx,esi
                xor     ebx,ebx
                mov     bl,[edx+ecx]
                shl     ebx,4
                xor     eax,ebx
                inc     edx
                and     edx,esi
                xor     ebx,ebx
                mov     bl,[edx+ecx]
                shl     ebx,8
                xor     eax,ebx
                and     eax,esi
{ Index := Head^[Key]; }
                mov     edx,Head
                mov     ESI,[eax*4+edx]
{ WHILE (Index <> NUL) DO }
              @@3:
                nop
                mov     Index,esi
                inc     esi
                jz      @@27
{ BEGIN
   INC(Count);}
                inc     Count
{  IF (Count > Depth) THEN Goto 1; }
                mov     eax,Depth
                dec     esi
                cmp     eax,Count
                jc      @@27
{  IF (Buffer^[(N+Best) and $7FFF] = Buffer^[(Index+Best) and $7FFF]) THEN }
                mov     eax,Best
                mov     ebx,eax
                mov     ecx,Buffer
                mov     edi,7FFFh
                add     eax,N
                and     eax,edi
                add     ebx,esi
                MOV     DL,[eax+ECX]
                and     ebx,edi
                xchg    edi,esi
                CMP     DL,[ebx+ECX]
                JNE     @@26
              @@7:
{  BEGIN
{   J := Index;}
                mov     edx,edi
{   Len := 0;}
                xor     edi,edi
{   I := N;}
                mov     ebx,N
{   WHILE (Buffer^[I] = Buffer^[J]) AND (Len<MAXCOPY) AND ((J<>N) AND (I<>Insrt)) DO}
              @@8:
                and     ebx,esi
                and     edx,esi
                mov     al,[ebx+ecx]
                cmp     al,[edx+ecx]
                JNE     @@17

                cmp     edx,N
                JE      @@17
                inc     edx

                cmp     ebx,Insrt
                JE      @@17
                inc     ebx

                CMP     EDI,40h
                JGE     @@17
{   BEGIN
     INC(Len);}
                inc     edi
{    INC(I);}
{    IF (I = MaxSize) THEN}
{    I := 0;}
{    INC(J);}
{    IF (J = MaxSize) THEN}
{    J := 0;}
{   END;}
                JMP     @@8
              @@17:
{   Dist := N - Index;}
                MOV     ECX,N
                SUB     ECX,Index
{   IF (Dist < 0) THEN}
{    Dist := Dist + MaxSize;}
                and     ecx,esi
                mov     ebx,offset CopyMax
{   Dist := Dist - Len;}
                SUB     ECX,EDI
{   IF (DictFile AND Dist > CopyMax[0]) THEN Goto 1;}
                MOV     EAX,DictFile
                AND     EAX,ECX
                CMP     EAX,[EBX]
                JLE     @@21
              @@20:
                JMP     @@27
              @@21:
{   IF (Len > Best) AND (Dist <= MaxDistance) THEN}
                CMP     EDI,Best
                JLE     @@26
              @@22:
                CMP     ECX,MaxDistance
                JG      @@26
              @@23:
{   BEGIN
     IF (Len > MINCOPY) OR (Dist <= CopyMax[SHORTRANGE+Binary]) THEN}
                mov     eax,3
                cmp     edi,eax
                JG      @@25
              @@24:
                add     eax,Binary
                CMP     ECX,[EBX+EAX*4]
                JG      @@26
              @@25:
{    BEGIN
      Best := Len;}
                MOV     Best,EDI
{     Distance := Dist;}
                MOV     Distance,ECX
{    END;
    END;
   END;}

              @@26:
                mov     esi,Index
{  Index := Next^[Index];}
                mov     eax,Next
                mov     esi,[esi*4+eax]
{ END;}
                jmp     @@3
{ 1: Match := Best;}
              @@27:
                mov     eax,Best
  {$IFNDEF VIRTUALPASCAL}
                pop     esi
                pop     edi
                pop     ebx
  {$ENDIF}
end;


(** Finite Window compression routines **)
const
  IDLE = 0;      { Not processing a COPYING }
  COPYING = 1;   { Currently processing COPYING }

{ Check first buffer for ordered dictionary file }
{ Better compression using short distance copies }
(*Procedure Dictionary;
var i,j,k,count :l;
begin
 i:=0;j:=0;count:=0;
 { Count matching chars at start of adjacent lines }
 inc(j);
 while (J < MINCOPY+MAXCOPY) do
 begin
  if (Buffer^[j-1]=10) then
  begin
   k:=j;
   while (Buffer^[i] = Buffer^[k]) do begin inc(i);inc(k);inc(count) end;
   i:=j;
  end;
  inc(j);
 end;
 { IF matching line prefixes > 25% assume dictionary }
 if (count>(MINCOPY+MAXCOPY) shr 2) then dictfile:=1;
end;*)



Function Compress(Length:longint;Source,Destin:PByte):longint;
var   c,i,n,Addpos,Len  :longint;
      Full,State,Nextlen:longint;
begin
 Dictfile := 0;Binary := 0;Input_Bit_Count := 0;Input_Bit_Buffer := 0;
 Output_Bit_Count := 0;Output_Bit_Buffer := 0;Addpos:=0;Len:=0;Full:=0;
 State:=IDLE;c:=0;CurBuf:=Destin;n:=MINCOPY;
 initialize;
 New(Head);New(Tail);New(Next);New(Prev);New(Buffer);
 { Initialize hash table to empty }
 for i:=0 to pred(HASHSIZE) do Head^[I]:=NUL;

 { Compress first few characters using Huffman }
 for i:=0 to pred(MINCOPY) do
 begin
  C:=Source^;inc(longint(Source));dec(length);
  if Length=0 then
  begin
   _Compress(TERMINATE);
   Flush_bits;
   Dispose(Head);Dispose(Tail);Dispose(Next);Dispose(Prev);Dispose(buffer);
   Exit
  end;
  _Compress(C);
  Buffer^[i] := C;
 end;

 { Preload next few characters into lookahead buffer }
(*
      Insrt := MINCOPY;
      FOR I := 0 To PRED(MAXCOPY) DO
       begin
        C := Source^;dec(length);inc(l(Source));
        IF length=0 THEN break;
        Buffer^[Insrt] := C;
        INC(Insrt);
        IF (C > 127) THEN Binary := 1;     { Binary file ?}
       end;*)

 asm {&SAVES EBX}
  {$IFNDEF VIRTUALPASCAL}
     push edi
     push esi
  {$ENDIF}
     mov  ecx,MAXCOPY
     mov  esi,Source
     mov  edi,Buffer
     mov  edx,MINCOPY
@Sloop:
     lodsb
     dec  Length
     jz   @Quit
     mov  [edi+edx],al
     inc  edx
     or   al,al
     jns  @loop
     mov  Binary,1
@loop:
     dec  ecx
     jnz  @Sloop
@quit:
     mov  Source,esi
     mov  Insrt,edx
  {$IFNDEF VIRTUALPASCAL}
     pop  esi
     pop  edi
  {$ENDIF}
 end;

// Dictionary;  { Check for dictionary file }

 while n<>Insrt do
 begin
  { Check compression to insure really a dictionary file }
     { IF (Boolean(dictfile) AND ((Bytes_In MOD MAXCOPY) = 0)) THEN
        IF (Bytes_In/Bytes_Out < 2) THEN
          Dictfile := 0; }    { Oops, not a dictionary file ! }

  { Update nodes in hash table lists }
  if boolean(Full) then Delete_Node(Insrt);
  Add_node(Addpos);

  { IF doing COPYING, process character, ELSE check for new COPYING }
  if State=COPYING then begin dec(Len);if len=1 then State:=IDLE end else
  begin
   { Get match length at next character and current char }
   if boolean(binary) then
   begin
    Nextlen := Match(N+1,BINNEXT);
    Len := Match(N,BINSEARCH);
   end else
   begin
    Nextlen := Match(N+1,TEXTNEXT);
    Len := Match(N,TEXTSEARCH);
   end;
   { IF long enough and no better match at next char, start COPYING }
   if (Len >= MINCOPY) and (len >= NextLen) then
   begin
    State := COPYING;
    { Look up minimum bits to encode distance }
    for i:=0 to pred(COPYRANGES) do
    begin
     if distance<=CopyMax[i] then
     begin
      _Compress(FIRSTCODE-MINCOPY+Len+I*CODESPERRANGE);
      asm {&SAVES EBX}
    {$IFNDEF VIRTUALPASCAL}
        push  edi
        push  esi
    {$ENDIF} 
        mov   esi,I
        mov   ecx,DWORD PTR CopyBits[ESI*4]
        inc   ecx
        mov   edi,Distance
        sub   edi,dword ptr CopyMin[ESI*4]
        mov   edx,Output_Bit_Buffer
@loop:
        dec   ecx
        jz    @quit
        rcr   edi,1
        rcl   edx,1
        inc   OutPut_Bit_Count
        cmp   OutPut_Bit_Count,32
        jc    @loop
        mov   eax,CurBuf
        mov   [eax],edx
        add   eax,4
        mov   CurBuf,eax
        mov   Output_Bit_Count,0
        jmp   @loop
@quit:
        mov   Output_Bit_Buffer,edx
    {$IFNDEF VIRTUALPASCAL}
        pop   esi
        pop   edi
    {$ENDIF} 
      end;
      break;
     end
    end;
   end else _Compress(Buffer^[N]);
  end;

  { Advance buffer pointers }
  asm {&SAVES EBX,ECX,EDX,ESI,EDI}
   mov   eax,7FFFh
   inc   N
   and   N,eax
   inc   AddPos
   and   AddPos,eax
  end;
(*  inc(N);if N=32768 then N:= 0;
  inc(Addpos);if Addpos = 32768 then Addpos := 0; *)

  { Add next input character to buffer }
  if Length>0 then
  begin
   Buffer^[Insrt]:=Source^;inc(longint(Source));dec(Length);Inc(Insrt);
   if Insrt=MaxSize then begin Insrt:=0;Full:=1 end;
  end else Full:=0;
 end;

 { Output EOF code and free memory }
 _compress(TERMINATE);
 Flush_Bits;
 Dispose(Head);Dispose(Tail);Dispose(Next);Dispose(Prev);Dispose(buffer);
 Compress:=longint(CurBuf)-longint(Destin);
END;





{********************* Decompression Routines ********************}







{ Read multibit code from input file }
(*Function Input_Code(Bits:Integer): l;
VAR
  I: l;
BEGIN
  result := 0;
  FOR I := 0 TO Bits-1 DO
    BEGIN
      IF (Input_Bit_Count = 0) THEN
        BEGIN
          Input_Bit_Buffer := Source^;inc(l(Source));
          Input_Bit_Count := 31;
        END
      else dec(Input_Bit_Count);
      IF Input_Bit_Buffer < 0 THEN result := result OR (1 shl i);
      Input_Bit_Buffer :=  Input_Bit_Buffer SHL 1;
    END;
END;*)




(*
{ Uncompress a character code from input stream }
Function Uncompress: l;
VAR Res:Longint;
BEGIN
 Result:=1;
 repeat
  if Input_Bit_Count = 0 then
  begin
   Input_Bit_Buffer := CurBuf^;inc(l(CurBuf));
   Input_Bit_Count := 31;
  end else dec(Input_Bit_Count);
  if Input_Bit_Buffer<0 then Result:=RightC[Result] else Result:=LeftC[Result];
  Input_Bit_Buffer := Input_Bit_Buffer SHL 1;
 until Result>MAXCHAR;
 dec(Result,SUCCMAX);
 Update_Model(Result);
END; *)


Function Decompress(Length:longint;Source,Destin:PByte):longint;
label fuck;
VAR
  I, J, Dist, Len, Index, K, T : longint;
  N, C : longint;
BEGIN
 Input_Bit_Count:=0;Input_Bit_Buffer:=0;n:=0;CurBuf:=Destin;
 initialize;
 New(Buffer);
 goto fuck;
 while C<>TERMINATE do
 begin
  if C<256 then
  begin     { Single literal character ? }
   CurBuf^:=C;inc(longint(CurBuf));
   Buffer^[N] := C;inc(N);if N = MaxSize then N := 0;
  end else
  begin     { ELSE string copy length/distance codes }
   T := C - FIRSTCODE;
   Index := (T) div CODESPERRANGE;
   Len := T + MINCOPY - Index*CODESPERRANGE;
//   Dist := Input_Code(CopyBits[Index]) + Len + CopyMin[Index];
   Dist:=0;
   for i:= 0 to CopyBits[Index]-1 do
   begin
    if Input_Bit_Count=0 then
    begin
     Input_Bit_Buffer:=PLongint(Source)^;inc(longint(Source),4);Input_Bit_Count:=31
    end else dec(Input_Bit_Count);
    if Input_Bit_Buffer<0 then Dist:=Dist or (1 shl i);
    Input_Bit_Buffer :=  Input_Bit_Buffer shl 1;
   end;
   inc(Dist,Len + CopyMin[Index]);
   J := N;
   K := N - Dist;
   if (K < 0) then INC(K,MaxSize);
   for i := 0 to PRED(Len) do
   begin
    CurBuf^:=Buffer^[K];inc(longint(CurBuf));
    Buffer^[J] := Buffer^[K];
    inc(J);if (J = Maxsize) then J := 0;
    inc(K);if (K = Maxsize) then K := 0;
   end;
   inc(N,Len);
   if N >= Maxsize then dec(N,MaxSize);
  end;
fuck:
  C:=1;  {--- bit input / but ^&*%$^&*$^&%$^&% ---}
  repeat
   if Input_Bit_Count = 0 then
   begin
    Input_Bit_Buffer:=PLongint(Source)^;inc(longint(Source),4);Input_Bit_Count:=31
   end else dec(Input_Bit_Count);
   if Input_Bit_Buffer<0 then C:=RightC[C] else C:=LeftC[C];
   Input_Bit_Buffer := Input_Bit_Buffer shl 1;
  until C>MAXCHAR;
  dec(C,SUCCMAX);
  Update_Model(C);
 end;
 Dispose(buffer);
 Decompress:=longint(CurBuf)-longint(Destin);
END;



END.




