
%{
// **********************************************************************
//
// Copyright (c) 2001 - 2002 MT Tools.
//
// All Rights Reserved
//
// MT_DORB is based in part on the product DORB,
// written by Shadrin Victor
//
// See Readme.txt for contact information
//
// **********************************************************************

const
  _n = chr(10); // \n
  _t = chr(9); // \t
  _v = chr(11); // \v
  _b = chr(8); // \b
  _r = chr(13); // \r
  _f = chr(12); // \f
  _a = chr(7); // \a
  _bslash = chr(92); // \\
  _sq = chr(39); // \'
  _dq = chr(34); // \"

function ascii_to_longlong( base: integer; s: PChar) : longint;
var
  c: char;
begin
  result := 0;
  while s^ <> chr(0) do
    begin
      c := s^;
      s := PChar(integer(s)+1);
      if  c >= 'a' then
        c := chr(ord(c) - (ord('a') - ord('A')));
      c := chr(ord(c) - ord('0'));
      if ord(c) > 9 then
        c := chr(ord(c) - (ord('A') - ord('0') - 10));
      result := result * base + ord(c);
  end
end;

function translate_char( const  s: string): char;
var
  c: char;
begin
  if  s[1] <> '\' then
    begin
      result := s[1];
      exit;
    end;
  c := s[2];
  case c of
  'n': result := _n;
  't': result := _t;
  'v': result := _v;
  'b': result := _b;
  'r': result := _r;
  'f': result := _f;
  'a': result := _a;
  '\': result := _bslash;
  '''': result := _sq;
  '"': result := _dq;
  'x','X' : result := chr(ascii_to_longlong( 16, PChar(integer(PChar(s))+2)));
  else
    // Gotta be an octal
    result := chr(ascii_to_longlong( 8, PChar(s)));
  end
end;

function translate_wchar(const s: string): widechar;
begin
  Result := WideChar(s[1]);
end;

%}

Digits                  [0-9]+
Oct_Digit               [0-7]
Hex_Digit               [a-fA-F0-9]
Int_Literal		[1-9][0-9]*
Oct_Literal		0{Oct_Digit}*
Hex_Literal		(0x|0X){Hex_Digit}*
Esc_Sequence1           \\[ntvbrfa\\\?\'\"]
Esc_Sequence2           \\{Oct_Digit}{1,3}
Esc_Sequence3           \\(x|X){Hex_Digit}{1,2}
Esc_Sequence4           \\(u|U){Hex_Digit}{1,4}
Esc_Sequence            ({Esc_Sequence1}|{Esc_Sequence2}|{Esc_Sequence3})
Char                    ([^\n\t\"\'\\]|{Esc_Sequence})
WChar                   ({Char}|{Esc_Sequence4})
Char_Literal            \'({Char}|\")\'
WChar_Literal           L'({WChar}|\")'
String_Literal		\"({Char}|\')*\"
WString_Literal		L\"({WChar}|\')*\"
Float_Literal1		{Digits}"."{Digits}?(e|E)('+'|'-')?{Digits}
Float_Literal2		{Digits}(e|E)('+'|'-')?{Digits}
Float_Literal3          {Digits}"."{Digits}
Float_Literal4          {Digits}"."
Float_Literal5		"."{Digits}
Float_Literal6		"."{Digits}(e|E)('+'|'-')?{Digits}
Fixed_Literal1          {Digits}(d|D)
Fixed_Literal2          {Digits}"."(d|D)
Fixed_Literal3          "."{Digits}(d|D)
Fixed_Literal4          {Digits}"."{Digits}(d|D)
CORBA_Identifier	[a-zA-Z_][a-zA-Z0-9_]*
%%

[ \t\r]			;
[\n]			inc(idl_line_no);
"//"[^\n]*		;
"#pragma"[^\n]*\n       begin
                          yylval.yy_str := yytext;
                          inc(idl_line_no);
                          return(T_PRAGMA);
                        end;
^"#line "[^\n]*\n       begin
                          preprocessor_directive( yytext );
			  yylval.yy_str := yytext;
                          return(T_INCLUDE);
                        end;
^"#"[^\n]*\n            ;
"{"			return(T_LEFT_CURLY_BRACKET);
"}"			return(T_RIGHT_CURLY_BRACKET);
"["		 	return(T_LEFT_SQUARE_BRACKET);
"]"			return(T_RIGHT_SQUARE_BRACKET);
"("			return(T_LEFT_PARANTHESIS);
")"			return(T_RIGHT_PARANTHESIS);
":"			return(T_COLON);
","			return(T_COMMA);
";"			return(T_SEMICOLON);
"="			return(T_EQUAL);
">>"			return(T_SHIFTRIGHT);
"<<"			return(T_SHIFTLEFT);
"+"			return(T_PLUS_SIGN);
"-"			return(T_MINUS_SIGN);
"*"			return(T_ASTERISK);
"/"			return(T_SOLIDUS);
"%"			return(T_PERCENT_SIGN);
"~"			return(T_TILDE);
"|"			return(T_VERTICAL_LINE);
"^"			return(T_CIRCUMFLEX);
"&"			return(T_AMPERSAND);
"<"			return(T_LESS_THAN_SIGN);
">"			return(T_GREATER_THAN_SIGN);

import			return(T_IMPORT);
const			return(T_CONST);
typedef			return(T_TYPEDEF);
float			return(T_FLOAT);
double			return(T_DOUBLE);
char			return(T_CHAR);
wchar			return(T_WCHAR);
fixed                   return(T_FIXED);
boolean			return(T_BOOLEAN);
string			return(T_STRING);
wstring			return(T_WSTRING);
void			return(T_VOID);
unsigned		return(T_UNSIGNED);
long 			return(T_LONG);
short			return(T_SHORT);
FALSE			return(T_FALSE);
TRUE			return(T_TRUE);
struct			return(T_STRUCT);
union			return(T_UNION);
switch			return(T_SWITCH);
case			return(T_CASE);
default			return(T_DEFAULT);
enum			return(T_ENUM);
in			return(T_IN);
out			return(T_OUT);
interface		return(T_INTERFACE);
abstract		return(T_ABSTRACT);
valuetype		return(T_VALUETYPE);
truncatable		return(T_TRUNCATABLE);
supports		return(T_SUPPORTS);
custom			return(T_CUSTOM);
public			return(T_PUBLIC);
private			return(T_PRIVATE);
factory			return(T_FACTORY);
native			return(T_NATIVE);
ValueBase		return(T_VALUEBASE);
"::"			return(T_SCOPE);
module			return(T_MODULE);
octet			return(T_OCTET);
any			return(T_ANY);
sequence		return(T_SEQUENCE);
readonly		return(T_READONLY);
attribute		return(T_ATTRIBUTE);
exception		return(T_EXCEPTION);
oneway			return(T_ONEWAY);
inout			return(T_INOUT);
raises			return(T_RAISES);
context			return(T_CONTEXT);
Object                  return(T_OBJECT);
typeId			return(T_TYPEID);
typePrefix		return(T_TYPEPREFIX);
local			return(T_LOCAL);
{CORBA_Identifier}	begin
                          if yytext[1] = '_' then
			    yylval.yyident := copy(yytext,2,200)
			  else
			    yylval.yyident := yytext;
			  return(T_IDENTIFIER);
                        end;
{Float_Literal1}	|
{Float_Literal2}	|
{Float_Literal3}	|
{Float_Literal4}	|
{Float_Literal5}	|
{Float_Literal6}	begin
                          if (DecimalSeparator <> '.') and (pos('.',yytext) > 0) then
                            yytext[pos('.',yytext)] := DecimalSeparator
                          else if (DecimalSeparator <> ',') and (pos(',',yytext) > 0) then
                            yytext[pos(',',yytext)] := DecimalSeparator;
                          yylval.yy_float := strtofloat(yytext);
			  return(T_FLOATING_PT_LITERAL);
                        end;
{Fixed_Literal1}	|
{Fixed_Literal2}	|
{Fixed_Literal3}	|
{Fixed_Literal4}	begin
                          if (DecimalSeparator <> '.') and (pos('.',yytext) > 0) then
                            yytext[pos('.',yytext)] := DecimalSeparator
                          else if (DecimalSeparator <> ',') and (pos(',',yytext) > 0) then
                            yytext[pos(',',yytext)] := DecimalSeparator;
                          yylval.yy_float := strtofloat(yytext);
			  return(T_FLOATING_PT_LITERAL);
                        end;
{Int_Literal}		begin
                          yylval.yy_int := strtoint(yytext );
			  return(T_INTEGER_LITERAL);
                        end;
{Oct_Literal}		begin
                          yylval.yy_int := ascii_to_longlong(8,PChar(AnsiString(yytext)));
			  return(T_INTEGER_LITERAL);
                        end;
{Hex_Literal}		begin
                          yylval.yy_int := strtoint(yytext);
			  return(T_INTEGER_LITERAL);
                        end;
{Char_Literal}		begin
                          yytext := copy(yytext,2,length(yytext)-2);
                          if yytext[1] = '\' then
                            yylval.yy_char := translate_char(yytext)
                          else
                            yylval.yy_char := yytext[1];
			  return(T_CHARACTER_LITERAL);
                        end;
{WChar_Literal}		begin
                          yytext := copy(yytext,3,length(yytext)-3);
                          if yytext[1] = '\' then
                            yylval.yy_wchar := translate_wchar(yytext)
                          else
                            yylval.yy_wchar := WideChar(yytext[1]);
			  return(T_WCHARACTER_LITERAL);
			end;
{String_Literal}	begin
                          yylval.yy_str := copy(yytext,2,length(yytext)-2);
			  return(T_STRING_LITERAL);
                        end;
{WString_Literal}	begin
                          yylval.yy_wstr := copy(yytext,3,length(yytext)-3);
			  return(T_WSTRING_LITERAL);
			end;
.                       return(T_UNKNOWN);

%%