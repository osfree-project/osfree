{&AlignCode-,AlignData-,AlignRec-,G3+,Speed-,Frame-,Delphi+,Use32+}
{$IFDEF OS2}
{&define OS_MAP_CASE}
{$ENDIF}
{$P+}
Unit strOp;

Interface

const
{keywords separator for 'Keyword', 'KeywordSpc', 'GetKeyword'}
     keywordSep = '';

type
     Str2       = String[2];
     Str4       = String[4];
     Str8       = String[8];
     Str10      = String[10];
     Str16      = String[16];
{$ifDef OS_MAP_CASE}
var
     uCaseMap,
     lCaseMap   : array[Char] of Char;
{$endIf OS_MAP_CASE}

{Convert a string to lower case}
 Procedure LowStr(var S : String);

{Convert a string to upper case}
 Procedure UpStr(var S : String);

{Return the lowercase string of argument}
 Function  LowStrg(S : String) : String;

{Return the uppercase string of argument}
 Function  UpStrg(S : String) : String;

{Return hexadecimal representation of a number}
 Function  Hex8(A : Longint) : Str8;
 Function  Hex4(A : Word) : Str4;
 Function  Hex2(A : Byte) : Str2;

{Return binary representation of a number}
 Function  Bin16(a : Word) : Str16;
 Function  Bin8(a : Byte) : Str8;

{Extract from a string a hexadecimal number. Cuts out the number from string}
 Function  HexVal(var s : String) : Longint;

{Same as HexVal but decimal. Example: DecVal('123 test') returns 123 and S = ' test' }
 Function  DecVal(var s : String) : Longint;

{FormatStr is the same as in Borland`s DRIVERS unit}
{$ifdef VER70} {$P-} {$endif}
 Procedure FormatStr(var Result: String; const Format: String; var Params);
{$ifdef VER70} {$P+} {$endif}

{Return decimal representation of A right-justified in N positions filled with Ch}
 Function  SStr(a : Longint; n : Byte; ch : Char) : String;

{Return left N characters from a string}
 Function Left(const S : string; N : Integer) : string;
{Return right N characters from a string}
 Function Right(const S : string; N : Integer) : string;
{Returns string S centered in a string of N chars Fill}
 Function Center(const S : string; N : Integer; Fill : Char) : string;

{Converts a longint to string (decimal)}
 Function  long2str(L : Longint) : String;

{Signed version of long2str: Returns '+###', '-###' or ' 0'}
 Function  signStr(L : Longint) : String;

{Remove starting spaces and tabs from a string}
 Procedure DelStartSpaces(var S : String);

{Remove trailing blanks and tabs from a string}
 Procedure DelTrailingSpaces(var S : String);

{Extract directory from a full file name}
 Function extractDir(const fName : string) : String;
{Extract filename from a full file name}
 Function extractName(const fName : string) : String;
{Extract last file extension from a full file name}
 Function extractExt(const fName : string) : String;

{Get string number No from a structure of type:}
{ dd length-of-entire-structure}
{ db 'string-number-one',0 }
{ db 'string-number-two',0 etc. }
 Function  GetASCIIZ(var Text; No : Longint) : String;
{The same but returns pChar to that string}
 Function  GetASCIIZptr(var Text; No : Longint) : pChar;
{Count ASCIIZ strings in above structure}
 Function  CountASCIIZ(var Text) : Longint;

{Strip off all blanks and tabs from a string}
 Function  StripBlanks(const S : String) : String;

{Return a string which represents percental relation of Val to Max with nFrac decimals}
 Function  StrPercent(Val,Max : Longint; NFrac : Byte) : String;

{Convert fixed-point float to string; Base is fixed-point unit (ONE);}
{nFrac is number of decimals. Example: StrFloat(8,16,3) = 0.500}
 Function  StrFloat(Val,Base : Longint; NFrac : Byte) : String;

{Return a roman number A = 0..2000}
 Function  RomanNumber(A : Word) : string;

{Return `short` version of pathName which fits into width W}
 Function  Short(const pathName : String; W : Byte) : String;

{Return a string containing Num characters Ch}
{&SAVES eax,ebx,edx,esi}
 Function  Strg(Ch : Char; Num : Integer) : String;

{Return position of first occurence of character Ch in string S}
{&SAVES ebx,esi}
 Function  First(ch : Char; const S : String) : Byte;

{Return position of last occurence of character Ch in string S}
{&SAVES ebx,esi}
 Function  Last(ch : Char; const S : String) : Byte;

{Return position of Nth occurence of character Ch in string S counting from head}
{&SAVES esi}
 Function  ScanFwd(ch : Char; const S : String; N : Byte) : Byte;

{Return position of Nth occurence of character Ch in string S counting from tail}
{&SAVES ebx,esi}
 Function  ScanBwd(ch : Char; const S : String; N : Byte) : Byte;

{Count occurences of character Ch in string S}
{&SAVES ebx,esi}
 Function  CharCount(ch : Char; const S : String) : Byte;

{Exclude from string S all characters Ch and return it}
{&SAVES edx}
 Function  Exclude(const S : String; Ch : Char) : String;

{Search for a keyword in string S and return its ordinal number.}
{Keyword definitions is an array of structure:}
{db 'keyword1keyword2keyword3[...]',0}
{If keyword is recognized it is cut out from input string}
{&SAVES none}
 Function  Keyword(var S : String; var Keyword) : Byte;

{The same as `Keyword` but after keyword must come a space or tab}
{(`Keyword` recognize even keywords in environments like 'keyword1thisis')}
{&SAVES none}
 Function  KeywordSpc(var S : String; var Keyword) : Byte;

{Return keyword number `No` from structure Keyword (same as above)}
{&SAVES edx}
 Function  GetKeyword(var Keyword; No : Word) : String;

{Return upper case of character C}
{&SAVES eax,ecx,edx,esi,edi}
 Function  UpCase(C : Char) : Char;

{Return lower case of character C}
{&SAVES eax,ecx,edx,esi,edi}
 Function  LowCase(C : Char) : Char;

{Return hexadecimal representation of lower nubble of A}
{&SAVES all}
 Function  HexChar(A : Byte) : Char;

Implementation uses miscUtil {$IfDef OS2}, os2base {$EndIf};

{&SAVES ebx,esi,edi}

Function HexChar; assembler;
asm             mov     al,a
                and     al,0Fh
                add     al,'0'
                cmp     al,58
                jc      @loc1
                add     al,7
@Loc1:
end;

Function Hex2;
begin
 Hex2 := HexChar(a shr 4) + HexChar(a);
end;

Function Hex4;
begin
 Hex4 := HexChar(Hi(a) shr 4) + HexChar(Hi(a))+
         HexChar(Lo(a) shr 4) + HexChar(Lo(a));
end;

Function Hex8;
begin
 Hex8 := Hex4(a shr 16) + Hex4(a);
end;

Function Bin16;
var s : Str16;
    i : Byte;
begin
 s := '';
 for i:=0 to 15 do
  begin
   asm rol a,1 end;
   s:=s + Char(48+(a and 1));
  end;
 Bin16 := s;
end;

Function Bin8;
var s : Str8;
    i : Byte;
begin
 s := '';
 for i := 0 to 7 do
  begin
   asm rol a,1 end;
   s := s + Char(48 + (a and 1));
  end;
 Bin8 := s;
end;

{$ifDef OS_MAP_CASE}
Function upCase; assembler;
asm             mov     al,&C
                lea     ebx,uCaseMap
                xlat
end;

Function lowCase; assembler;
asm             mov     al,&C
                lea     ebx,lCaseMap
                xlat
end;
{$else}
Function upCase; assembler;
asm             mov     al,&C
                cmp     al,'a'
                jb      @@ok
                cmp     al,'z'
                jbe     @@lo
                cmp     al,'†'
                jb      @@ok
                cmp     al,'Ø'
                jbe     @@lo
                cmp     al,'‡'
                jb      @@ok
                cmp     al,'Ô'
                ja      @@ok
                sub     al,80-32
@@lo:           sub     al,20h
@@ok:
end;

Function lowCase; assembler;
asm             mov     al,&C
                cmp     al,'A'
                jb      @@ok
                cmp     al,'Z'
                jbe     @@up
                cmp     al,'Ä'
                jb      @@ok
                cmp     al,'è'
                jbe     @@up
                cmp     al,'ê'
                jb      @@ok
                cmp     al,'ü'
                ja      @@ok
                add     al,80-32
@@up:           add     al,20h
@@ok:
end;
{$endIf}

procedure LowStr;
var i : byte;
begin
 for i:=1 to length(s) do s[i]:=LowCase(s[i]);
end;

Function LowStrg;
begin
 LowStr(s); LowStrg := s;
end;

Procedure UpStr;
var i : byte;
begin
 for i:=1 to length(s) do s[i]:=UpCase(s[i]);
end;

Function UpStrg;
begin
 UpStr(s); UpStrg := s;
end;

Function SStr;
var s : String;
    i : Byte;
begin
 Str(a:n,s);
 for i := 1 to n do if s[i] = ' ' then s[i] := ch else break;
 SStr := s;
end;

Function left;
begin
 left := Copy(S, 1, N);
end;

Function right;
begin
 right := Copy(S, succ(length(S) - N), N);
end;

Function Center(const S : string; N : Integer; Fill : Char) : string;
var
 tS    : string;
 l,f,c : Integer;
begin
 c := length(S); l := (N - c) div 2;
 if l < 1
  then begin f := -l+2; l := 1; end
  else f := 1;
 if f + c > N then c := succ(N - f);
 tS := Strg(Fill, N);
 Move(S[f], tS[l], c);
 Center := tS;
end;

{$ifDef use32}
{ A generalized string formatting routine. Given a string in Format     }
{ that includes format specifiers and a list of parameters in Params,   }
{ FormatStr produces a formatted output string in Result.               }
{ Format specifiers are of the form %[-][nnn]X, where                   }
{   % indicates the beginning of a format specifier                     }
{  [-] is an optional minus sign (-) indicating the parameter is to be  }
{      left-justified (by default, parameters are right-justified)      }
{ [nnn] is an optional, decimal-number width specifier in the range     }
{      0..255 (0 indicates no width specified, and non-zero means to    }
{      display in a field of nnn characters)                            }
{   X  is a format character:                                           }
{   's' means the parameter is a pointer to a string.                   }
{   'd' means the parameter is a Longint to be displayed in decimal.    }
{   'c' means the low byte of the parameter is a character.             }
{   'x' means the parameter is a Longint to be displayed in hexadecimal.}
{   '#' sets the parameter index to nnn.                                }
{$V+}
procedure FormatStr(var Result: String; const Format: String; var Params);
  assembler; {&USES ebx,esi,edi} {&FRAME+}
var ParOfs    : Longint;
    Buffer    : array [1..12] of Byte;
const
    HexDigits : array [0..15] of Char = '0123456789ABCDEF';

{ Convert next parameter to string              }
{ EXPECTS:      al    = Conversion character    }
{ RETURNS:      esi   = Pointer to string       }
{               ecx   = String length           }

procedure Convert; assembler; {$USES None} {$FRAME-}
asm
                mov     edx,eax
                mov     esi,Params
                lodsd
                mov     Params,esi
                xor     ecx,ecx
                lea     esi,Buffer[TYPE Buffer]
                and     dl,0DFh         { UpCase(ConversionChar) }
                cmp     dl,'C'
                je      @@ConvertChar
                cmp     dl,'S'
                je      @@ConvertStr
                cmp     dl,'D'
                je      @@ConvertDec
                cmp     dl,'X'
                jne     @@Done
{ ConvertHex }
@@1:            mov     edx,eax
                and     edx,0Fh
                mov     dl,HexDigits.Byte[edx]
                dec     esi
                inc     ecx
                mov     [esi],dl
                shr     eax,4
                jnz     @@1
                jmp     @@Done

@@ConvertDec:   push    esi
                mov     ebx,eax
                mov     ecx,10
                test    eax,eax
                jns     @@2
                neg     eax
@@2:            xor     edx,edx
                dec     esi
                div     ecx
                add     dl,'0'
                mov     [esi],dl
                test    eax,eax
                jnz     @@2
                pop     ecx
                sub     ecx,esi
                test    ebx,ebx
                jns     @@Done
                mov     al,'-'
@@ConvertChar:  inc     ecx
                dec     esi
                mov     [esi],al
                jmp     @@Done
@@ConvertStr:   test    eax,eax
                jz      @@Done
                mov     esi,eax
                lodsb
                mov     cl,al
@@Done:
end;

{ FormatStr body }
asm
                mov     eax,Params
                mov     ParOfs,eax
                xor     eax,eax
                mov     esi,Format
                mov     edi,Result
                inc     edi
                cld
                lodsb
                mov     ecx,eax
@@1:            jecxz   @@9
                lodsb
                dec     ecx
                cmp     al,'%'
                je      @@3
@@2:            stosb
                jmp     @@1
@@3:            jecxz   @@9
                lodsb
                dec     ecx
                cmp     al,'%'
                je      @@2             { bh = Justify (0:right, 1:left) }
                mov     ebx,' '         { bl = Filler character          }
                xor     edx,edx         { edx = Field width (0:no width) }
                cmp     al,'0'
                jne     @@4
                mov     bl,al
@@4:            cmp     al,'-'
                jne     @@5
                inc     bh
                jecxz   @@9
                lodsb
                dec     ecx
@@5:            cmp     al,'0'
                jb      @@6
                cmp     al,'9'
                ja      @@6
                sub     al,'0'
                xchg    eax,edx
                mov     ah,10
                mul     ah
                add     al,dl
                xchg    eax,edx
                jecxz   @@9
                lodsb
                dec     ecx
                jmp     @@5
@@6:            cmp     al,'#'
                jne     @@10
                shl     edx,2
                add     edx,ParOfs
                mov     Params,edx
                jmp     @@1
@@9:            mov     eax,Result
                mov     ecx,edi
                sub     ecx,eax
                dec     ecx
                mov     [eax],cl
                jmp     @@Done
@@10:           push    esi
                push    ecx
                push    edx
                push    ebx
                Call    Convert
                pop     ebx
                pop     edx
                test    edx,edx
                jz      @@12
                sub     edx,ecx
                jae     @@12
                test    bh,bh
                jnz     @@11
                sub     esi,edx
@@11:           add     ecx,edx
                xor     edx,edx
@@12:           test    bh,bh
                jz      @@13
                rep     movsb           { Copy formated parm (left-justified)}
@@13:           xchg    ecx,edx
                mov     al,bl
                rep     stosb           { Fill unused space }
                xchg    ecx,edx
                rep     movsb           { Copy formated parm (right-justified)}
                pop     ecx
                pop     esi
                jmp     @@1
@@Done:
end; {&FRAME-}

Function Strg; assembler;
asm             cld
                mov     edi,@result
                mov     ecx,Num
                cmp     cx,255
                jbe     @@lenOK
                xor     ecx,ecx
@@lenOK:        mov     al,cl
                stosb
                mov     al,&Ch
                mov     ah,al
                shr     ecx,1
                rep     stosw
                adc     cl,cl
                rep     stosb
end;

{&SAVES ebx,edx,esi,edi}
Function SetUpCase(var c : Char) : boolean; assembler;
asm             mov     ecx,&c
                mov     al,[ecx]
                cmp     al,'a'
                jb      @E1
                cmp     al,'z'
                ja      @E1
                and     al,0DFh
                mov     [ecx],al
@E1:            mov     ah,0
                cmp     al,'0'
                jb      @E3
                cmp     al,'F'
                ja      @E3
                cmp     al,'9'
                jbe     @E2
                cmp     al,'A'
                jb      @E3
@E2:            mov     ah,1
@E3:            mov     al,ah
end;
{&SAVES ebx,esi,edi}

{$else}

Function Strg; assembler;
asm             cld
                les     di,@result
                mov     cx,Num
                cmp     cx,255
                jbe     @@lenOK
                xor     cx,cx
@@lenOK:        mov     al,cl
                stosb
                mov     al,&Ch
                mov     ah,al
                shr     cx,1
                rep     stosw
                adc     cl,cl
                rep     stosb
end;

Function SetUpCase(var c : Char) : boolean; assembler;
asm             les     si,c
                mov     al,es:[si]
                cmp     al,'a'
                jb      @E1
                cmp     al,'z'
                ja      @E1
                and     al,$5F
                mov     es:[si],al
@E1:            mov     ah,0
                cmp     al,'0'
                jb      @E3
                cmp     al,'F'
                ja      @E3
                cmp     al,'9'
                jbe     @E2
                cmp     al,'A'
                jb      @E3
@E2:            mov     ah,1
@E3:            mov     al,ah
end;

{$endIf}

Function HexVal;
var i,j : Byte;
    k   : LongInt;
begin
 k:=0;i:=1;
 While SetUpCase(s[i]) and (i<=Length(s)) and (i<9) do
  begin
   j:=Byte(UpCase(s[i]))-48;if j>9 then Dec(j,7);
   k:=(k shl 4) or j;Inc(i);
  end;
 HexVal:=k;Delete(s,1,i-1);
end;

Function DecVal;
var i : Byte;
    k : LongInt;
    m : Boolean;
begin
 k := 0; i:=1;
 m := False;
 case s[1] of
  '-' : begin m := TRUE; Inc(i); end;
  '+' : Inc(i);
 end;
 While (i <= Length(s)) and (i < 11) and (UpCase(s[i]) in ['0'..'9']) do
  begin
   k := (k * 10) + (Byte(UpCase(s[i])) - 48);
   Inc(i);
  end;
 if m
  then DecVal := -k
  else DecVal := k;
 Delete(s, 1, i - 1);
end;

{$ifDef use32}
Function First; assembler;
asm             cld
                mov     edi,S
                movzx   ecx,[edi].byte
                mov     edx,ecx
                inc     edi
                mov     al,&ch
                jecxz   @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@locEx
@@OK:           sub     edx,ecx
                mov     al,dl
@@locEx:
end;

Function Last; assembler;
asm             std
                mov     edi,S
                movzx   ecx,[edi].byte
                mov     edx,ecx
                add     edi,ecx
                mov     al,&Ch
                jecxz   @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           mov     eax,edx
                sub     edx,ecx
                sub     eax,edx
                inc     al
@@LocEx:        cld
end;

Function ScanFwd; assembler;
asm             cld
                mov     edi,S
                mov     dh,N
                or      dh,dh
                je      @@NO
                movzx   ecx,[edi].byte
                mov     ebx,ecx
                inc     edi
                mov     al,&Ch
@@NS:           jecxz   @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           dec     dh
                jne     @@NS
                sub     ebx,ecx
                mov     al,bl
@@LocEx:
end;

Function ScanBwd; assembler;
asm             std
                mov     edi,S
                mov     dh,N
                or      dh,dh
                je      @@NO
                movzx   ecx,[edi].byte
                add     edi,ecx
                mov     al,&Ch
@@NS:           jecxz   @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           dec     dh
                jne     @@NS
                mov     eax,ecx
                inc     al
@@LocEx:        cld
end;

Function CharCount; assembler;
asm             cld
                mov     edi,S
                mov     dh,0
                movzx   ecx,[edi].byte
                mov     edx,ecx
                inc     edi
                mov     al,&Ch
@@next:         jecxz   @@done
                repne   scasb
                jne     @@done
                inc     dh
                jmp     @@next
@@done:         mov     al,dh
end;

Function Exclude; assembler;
asm             cld
                mov     esi,S
                mov     edi,@result
                inc     edi
                lodsb
                mov     cl,al
                mov     ebx,edi
                test    al,al
                je      @@done
                mov     ah,&ch
@@nextCh:       lodsb
                cmp     al,ah
                je      @@skip
                stosb
@@skip:         dec     cl
                jne     @@nextCh
@@done:         sub     edi,ebx
                mov     eax,edi
                mov     [ebx-1],al
end;

Function Keyword; assembler;
asm             cld
                mov     esi,S
                lodsb
                movzx   ecx,al
                mov     bx,100h
                mov     edi,Keyword
@@1:            push    esi
                push    ecx
@@2:            mov     al,[edi]
                inc     edi
                push    ebx
                push    eax
                call    UpCase
                pop     ebx
                or      al,al
                je      @@5
                mov     ah,al
                lodsb
                push    ebx
                push    eax
                call    upCase
                pop     ebx
                cmp     ah,keywordSep
                je      @@4
                inc     bl
                cmp     al,ah
                loope   @@2
                je      @@36
@@3:            mov     bl,0
                inc     bh
@@35:           mov     al,[edi]
                inc     edi
                or      al,al
                je      @@5
                cmp     al,keywordSep
                jne     @@35
                pop     ecx
                pop     esi
                jmp     @@1
@@36:           cmp     [edi].byte,keywordSep
                jne     @@3
@@4:            pop     ecx
                pop     esi
                mov     al,bh
                movzx   ebx,bl
                sub     [esi-1],bl
                sub     cl,bl
                mov     edi,esi
                add     esi,ebx
                rep     movsb
                jmp     @@6
@@5:            mov     al,0
                pop     ecx
                pop     esi
@@6:
end;

Function KeywordSpc; assembler;
asm             cld
                mov     esi,S
                lodsb
                movzx   ecx,al
                mov     bx,100h
                mov     edi,Keyword
@@1:            push    esi
                push    ecx
@@2:            mov     al,[edi]
                inc     edi
                push    ebx
                push    eax
                call    UpCase
                pop     ebx
                or      al,al
                je      @@5
                mov     ah,al
                lodsb
                push    ebx
                push    eax
                call    upCase
                pop     ebx
                cmp     ah,keywordSep
                je      @@36
                inc     bl
                cmp     al,ah
                loope   @@2
                je      @@4
@@34:           mov     bl,0
                inc     bh
@@35:           mov     al,[edi]
                inc     edi
                or      al,al
                je      @@5
                cmp     al,keywordSep
                jne     @@35
                pop     ecx
                pop     esi
                jmp     @@1
@@36:           dec     edi
                cmp     [esi-1].byte,' '
                ja      @@34
@@4:            cmp     [edi].byte,keywordSep
                jne     @@34
                pop     ecx
                pop     esi
                mov     al,bh
                movzx   ebx,bl
                sub     [esi-1],bl
                sub     cl,bl
                mov     edi,esi
                add     esi,ebx
                rep     movsb
                jmp     @@6
@@5:            mov     al,0
                pop     ecx
                pop     esi
@@6:
end;

Function GetKeyword; assembler;
asm             cld
                mov     esi,Keyword
                mov     edi,@result
                mov     ecx,No
@@nextWord:     dec     ecx
                jz      @@done
@@scan:         lodsb
                test    al,al
                jz      @@notFound
                cmp     al,keywordSep
                jne     @@scan
                jmp     @@nextWord
@@done:         mov     ah,0
                mov     ebx,edi
                inc     edi
@@copyWord:     lodsb
                cmp     al,keywordSep
                je      @@end
                stosb
                jmp     @@copyWord
@@end:          mov     al,ah
                mov     edi,ebx
@@notFound:     stosb
end;

{$else}
{ String formatting routines }
procedure FormatStr; external {FORMAT};
{$L FORMAT.OBJ}

Function First(ch : Char; const S : String) : Byte; assembler;
asm             cld
                les     di,S
                mov     cl,es:[di]
                mov     ch,0
                mov     bx,cx
                inc     di
                mov     al,&Ch
                jcxz    @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           sub     bx,cx
                mov     ax,bx
@@LocEx:
end;

Function Last(ch : Char; const S : String) : Byte; assembler;
asm             std
                les     di,S
                mov     cl,es:[di]
                mov     ch,0
                mov     bx,cx
                add     di,cx
                mov     al,&Ch
                jcxz    @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           mov     ax,bx
                sub     bx,cx
                sub     ax,bx
                inc     ax
@@LocEx:        cld
end;

Function ScanFwd(ch : Char; const S : String; N : Byte) : Byte; assembler;
asm             cld
                les     di,S
                mov     dh,N
                or      dh,dh
                je      @@NO
                mov     cl,es:[di]
                mov     ch,0
                mov     bx,cx
                inc     di
                mov     al,&Ch
@@NS:           jcxz    @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           dec     dh
                jne     @@NS
                sub     bx,cx
                mov     ax,bx
@@LocEx:
end;

Function ScanBwd(ch : Char; const S : String; N : Byte) : Byte; assembler;
asm             std
                les     di,S
                mov     dh,N
                or      dh,dh
                je      @@NO
                mov     cl,es:[di]
                mov     ch,0
                mov     bx,cx
                add     di,cx
                mov     al,&Ch
@@NS:           jcxz    @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,0
                jmp     @@LocEx
@@OK:           dec     dh
                jne     @@NS
                mov     ax,bx
                sub     bx,cx
                sub     ax,bx
                inc     ax
@@LocEx:        cld
end;

Function CharCount(ch : Char; const S : String) : Byte; assembler;
asm             cld
                les     di,S
                xor     dh,dh
                mov     cl,es:[di]
                mov     ch,0
                mov     bx,cx
                inc     di
                mov     al,&Ch
@@NS:           jcxz    @@NO
                repne   scasb
                je      @@OK
@@NO:           mov     al,dh
                jmp     @@LocEx
@@OK:           inc     dh
                jmp     @@NS
@@LocEx:
end;

Function Keyword(var S : String; var Keyword) : Byte; assembler;
asm             cld
                push    ds
                lds     si,S
                lodsb
                mov     cl,al
                mov     ch,0
                mov     bx,100h
                les     di,Keyword
@@1:            push    si
                push    cx
@@2:            mov     al,es:[di]
                inc     di
                call    @@loCase
                mov     ah,al
                lodsb
                call    @@loCase
                or      ah,ah
                je      @@5
                cmp     ah,keywordSep
                je      @@4
                inc     bl
                cmp     al,ah
                loope   @@2
                je      @@36
@@34:           mov     bl,0
                inc     bh
@@35:           mov     al,es:[di]
                inc     di
                or      al,al
                je      @@5
                cmp     al,keywordSep
                jne     @@35
                pop     cx
                pop     si
                jmp     @@1
@@36:           cmp     es:[di].byte,keywordSep
                jne     @@34
@@4:            pop     cx
                pop     si
                mov     al,bh
                sub     ds:[si-1],bl
                sub     cl,bl
                mov     bh,0
                mov     di,si
                add     si,bx
                push    ds
                pop     es
                rep     movsb
                jmp     @@6

@@loCase:       cmp     al,'A'
                jb      @@lcEx
                cmp     al,'Z'
                ja      @@lcEx
                or      al,20h
@@lcEx:         retn

@@5:            mov     al,0
                pop     cx
                pop     si
@@6:            pop     ds
end;

Function KeywordSpc(var S : String; var Keyword) : Byte; assembler;
asm             cld
                push    ds
                lds     si,S
                lodsb
                mov     cl,al
                mov     ch,0
                mov     bx,100h
                les     di,Keyword
@@1:            push    si
                push    cx
@@2:            mov     al,es:[di]
                inc     di
                call    @@loCase
                mov     ah,al
                lodsb
                call    @@loCase
                or      ah,ah
                je      @@5
                cmp     ah,keywordSep
                je      @@36
                inc     bl
                cmp     al,ah
                loope   @@2
                je      @@4
@@34:           mov     bl,0
                inc     bh
@@35:           mov     al,es:[di]
                inc     di
                or      al,al
                je      @@5
                cmp     al,keywordSep
                jne     @@35
                pop     cx
                pop     si
                jmp     @@1
@@36:           dec     di
                cmp     ds:[si-1].byte,' '
                ja      @@34
@@4:            cmp     es:[di].byte,keywordSep
                jne     @@34
                pop     cx
                pop     si
                mov     al,bh
                sub     ds:[si-1],bl
                sub     cl,bl
                mov     bh,0
                mov     di,si
                add     si,bx
                push    ds
                pop     es
                rep     movsb
                jmp     @@6

@@loCase:       cmp     al,'A'
                jb      @@lcEx
                cmp     al,'Z'
                ja      @@lcEx
                or      al,20h
@@lcEx:         retn

@@5:            mov     al,0
                pop     cx
                pop     si
@@6:            pop     ds
end;

Function GetKeyword; assembler;
asm             cld
                push    ds
                lds     si,Keyword
                les     di,@result
                mov     cx,No
@@nextWord:     dec     cx
                jz      @@done
@@scan:         lodsb
                test    al,al
                jz      @@notFound
                cmp     al,keywordSep
                jne     @@scan
                jmp     @@nextWord
@@done:         mov     ah,0
                mov     bx,di
                inc     di
@@copyWord:     lodsb
                cmp     al,keywordSep
                je      @@end
                inc     ah
                stosb
                jmp     @@copyWord
@@end:          mov     al,ah
                mov     di,bx
@@notFound:     stosb
                pop     ds
end;

Function Exclude; assembler;
asm             cld
                push    ds
                lds     si,S
                les     di,@result
                inc     di
                lodsb
                mov     cl,al
                mov     bx,di
                test    al,al
                je      @@done
                mov     ah,&ch
@@nextCh:       lodsb
                cmp     al,ah
                je      @@skip
                stosb
@@skip:         dec     cl
                jne     @@nextCh
@@done:         sub     di,bx
                mov     ax,di
                mov     es:[bx-1],al
                pop     ds
end;

{$endIf}

Procedure DelStartSpaces;
var I : Integer;
begin
 I := 1; While (I <= length(S)) and (S[I] in [' ',#9]) do Inc(I);
 Delete(S, 1, I - 1);
end;

Procedure DelTrailingSpaces;
begin
 While S[length(S)] in [' ', #9] do Dec(byte(S[0]));
end;

Function extractDir(const fName : string) : String;
var I : Byte;
begin
 I := length(fName);
 While (I > 0) and (not (fName[I] in ['/', '\', ':'])) do Dec(I);
 extractDir := Copy(fName, 1, I);
end;

Function extractName(const fName : string) : String;
var I : Byte;
begin
 I := length(fName);
 While (I > 0) and (not (fName[I] in ['/', '\', ':'])) do Dec(I);
 extractName := Copy(fName, I + byte(I > 0), 255);
end;

Function extractExt(const fName : string) : String;
var I : Byte;
begin
 I := length(fName);
 While (I > 0) and (not (fName[I] in ['.','/', '\', ':'])) do Dec(I);
 if (I > 0) and (fName[I] = '.')
  then extractExt := Copy(fName, I, 255)
  else extractExt := '';
end;

Function GetASCIIZ;
var I : Integer;
    P : pChar;
    S : String;
begin
 P := GetASCIIZptr(Text, No);
 S := '';
 While P^ <> #0 do begin S := S + P^; Inc(P); end;
 GetASCIIZ := S;
end;

Function GetASCIIZptr;
var I   : Integer;
    P,F : pChar;
begin
 P := @Text;
 F := P; Inc(F, pLong(F)^);
 Inc(P, sizeOf(Longint));
 For I := 2 to No do
  begin
   While (P < F) and (P^ <> #0) do Inc(P);
   if P >= F
    then begin P := nil; break; end
    else Inc(P);
  end;
 GetASCIIZptr := P;
end;

Function CountASCIIZ;
var C   : Longint;
    P,F : pChar;
begin
 P := @Text; C := 0;
 F := P; Inc(F, pLong(F)^);
 Inc(P, sizeOf(Longint));
 repeat
  While (P < F) and (P^ <> #0) do Inc(P);
  Inc(C);
  if P < F then Inc(P) else break;
 until FALSE;
 CountASCIIZ := C;
end;

Function StripBlanks(const S : String) : String;
var RS  : String;
    I,J : Integer;
begin
 J := 0;
 For I := 1 to length(S) do
  if not (S[I] in [' ',#9]) then begin Inc(J); RS[J] := S[I]; end;
 RS[0] := char(J);
 StripBlanks := RS;
end;

Function StrPercent;
var S : String;
    P : Longint;
    I : Integer;
begin
 P := 1; For I := 1 to NFrac + 2 do P := P * 10;
 S := SStr(longint(Val) * P div Max, NFrac + 1, '0');
 if NFrac > 0 then Insert('.', S, length(S) - NFrac + 1);
 StrPercent := S;
end;

Function StrFloat;
var S : String;
    P : Longint;
    I : Integer;
begin
 P := 1; For I := 1 to NFrac do P := P * 10;
 S := SStr(longint(Val) * P div Base, NFrac + 1, '0');
 if NFrac > 0 then Insert('.', S, length(S) - NFrac + 1);
 StrFloat := S;
end;

function RomanNumber(A : Word) : string; {0 < A < 2000}
var S : String[10];
begin
 if A >= 1000 then S := 'M' else S := '';
 A := A mod 1000;
 if A >= 100
    then case A div 100 of
          1..3 : S := S + Strg('C', A div 100);
          4    : S := S + 'CL';
          5..8 : S := S + 'L' + Strg('C', A div 100 - 5);
          9    : S := S + 'CM';
         end;
 A := A mod 100;
 if A >= 10
    then case A div 10 of
          1..3 : S := S + Strg('X', A div 10);
          4    : S := S + 'XL';
          5..8 : S := S + 'L' + Strg('X', A div 10 - 5);
          9    : S := S + 'XC';
         end;
 A := A mod 10;
 if A >= 1
    then case A of
          1..3 : S := S + Strg('I', A);
          4    : S := S + 'IV';
          5..8 : S := S + 'V' + Strg('I', A - 5);
          9    : S := S + 'IX';
         end;
 RomanNumber := S;
end;

Function Long2str(L : Longint) : String;
var A : String;
begin
 Str(L, A);
 Long2str := A;
end;

Function SignStr(L : Longint) : String;
var A : String;
begin
 Str(L, A);
 if L < 0
  then SignStr := A
  else
 if L > 0
  then SignStr := '+' + A
  else SignStr := ' ' + A;
end;

Function Short;
var
 sl,dl,
 i,j : integer;
 res : string;
begin
 if length(pathName) <= w
  then begin
        Short := pathName;
        exit;
       end;
 res := extractDir(pathName);
 dl := length(res); sl := 1;
 While (sl < length(res)) and (not (res[sl] in ['/','\'])) do Inc(sl);
 i := sl;
 res := pathName;
 repeat
  j := succ(i);
  While (j < dl) and (not (res[j] in ['/','\'])) do Inc(j);
  Delete(res, i, j-i);
 until (length(res) <= sl) or (length(res) + 4 <= w);
 Insert('...\', res, succ(i));
 Short := Copy(res, 1, w);
end;

{$ifDef OS_MAP_CASE}
var cc : CountryCode;
    I  : Longint;

begin
 FillChar(cc, SizeOf(cc), 0);
 For I := 0 to 255 do uCaseMap[char(I)] := char(I);
 lCaseMap := uCaseMap;
 if DosMapCase(256, cc, @uCaseMap) <> 0 then Halt(1);
 For I := 0 to 255 do
  if (uCaseMap[char(I)] <> char(I)) and (lCaseMap[uCaseMap[char(I)]] = uCaseMap[char(I)])
   then lCaseMap[uCaseMap[char(I)]] := char(I);
{$endIf}
end.

