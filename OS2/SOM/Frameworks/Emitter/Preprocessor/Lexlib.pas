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

{$I-}
{$H+}

unit Lexlib;

(* Standard Lex library unit for TP Lex Version 3.0.
   2-11-91 AG *)

interface

uses
  {$IFDEF MSWINDOWS}Windows,{$ENDIF}Classes, sysutils;

(* The Lex library unit supplies a collection of variables and routines
   needed by the lexical analyzer routine yylex and application programs
   using Lex-generated lexical analyzers. It also provides access to the
   input/output streams used by the lexical analyzer and the text of the
   matched string, and provides some utility functions which may be used
   in actions.

   This `standard' version of the LexLib unit is used to implement lexical
   analyzers which read from and write to MS-DOS files (using standard input
   and output, by default). It is suitable for many standard applications
   for lexical analyzers, such as text conversion tools or compilers.

   However, you may create your own version of the LexLib unit, tailored to
   your target applications. In particular, you may wish to provide another
   set of I/O functions, e.g., if you want to read from or write to memory
   instead to files, or want to use different file types. *)

(* Variables:

   The variable yytext contains the current match, yyleng its length.
   The variable yyline contains the current input line, and yylineno and
   yycolno denote the current input position (line, column). These values
   are often used in giving error diagnostics (however, they will only be
   meaningful if there is no rescanning across line ends).

   The variables yyinput and yyoutput are the text files which are used
   by the lexical analyzer. By default, they are assigned to standard
   input and output, but you may change these assignments to fit your
   target application (use the Turbo Pascal standard routines assign,
   reset, and rewrite for this purpose). *)

var

yyinput :  TStream;
yyoutput : TStream;
yyline            : String;      (* current input line *)
yylineno, yycolno : Integer;     (* current input position *)
yytext            : ShortString;      (* matched text (should be considered r/o) *)
yyleng            : Byte         (* length of matched text *)
  absolute yytext;
filename : string;

const
  CRLF : WORD = ($A shl 8) or $D;

procedure PrintError(msg: string);
procedure yyprintln(str: string);

(* I/O routines:

   The following routines get_char, unget_char and put_char are used to
   implement access to the input and output files. Since \n (newline) for
   Lex means line end, the I/O routines have to translate MS-DOS line ends
   (carriage-return/line-feed) into newline characters and vice versa. Input
   is buffered to allow rescanning text (via unput_char).

   The input buffer holds the text of the line to be scanned. When the input
   buffer empties, a new line is obtained from the input stream. Characters
   can be returned to the input buffer by calls to unget_char. At end-of-
   file a null character is returned.

   The input routines also keep track of the input position and set the
   yyline, yylineno, yycolno variables accordingly.

   Since the rest of the Lex library only depends on these three routines
   (there are no direct references to the yyinput and yyoutput files or
   to the input buffer), you can easily replace get_char, unget_char and
   put_char by another suitable set of routines, e.g. if you want to read
   from/write to memory, etc. *)

function get_char : Char;
  (* obtain one character from the input file (null character at end-of-
     file) *)

procedure unget_char ( c : Char );
  (* return one character to the input file to be reread in subsequent calls
     to get_char *)

procedure put_char ( c : Char );
  (* write one character to the output file *)

(* Utility routines: *)

procedure echo;
  (* echoes the current match to the output stream *)

procedure yymore;
  (* append the next match to the current one *)

procedure yyless ( n : Integer );
  (* truncate yytext to size n and return the remaining characters to the
     input stream *)

procedure reject;
  (* reject the current match and execute the next one *)

  (* reject does not actually cause the input to be rescanned; instead,
     internal state information is used to find the next match. Hence
     you should not try to modify the input stream or the yytext variable
     when rejecting a match. *)

procedure return ( n : Integer );
procedure returnc ( c : Char );
  (* sets the return value of yylex *)

procedure start ( state : Integer );
  (* puts the lexical analyzer in the given start state; state=0 denotes
     the default start state, other values are user-defined *)

(* yywrap:

   The yywrap function is called by yylex at end-of-file (unless you have
   specified a rule matching end-of-file). You may redefine this routine
   in your Lex program to do application-dependent processing at end of
   file. In particular, yywrap may arrange for more input and return false
   in which case the yylex routine resumes lexical analysis. *)

function yywrap : Boolean;
  (* The default yywrap routine supplied here closes input and output files
     and returns true (causing yylex to terminate). *)

(* The following are the internal data structures and routines used by the
   lexical analyzer routine yylex; they should not be used directly. *)

var

yystate    : Integer; (* current state of lexical analyzer *)
yyactchar  : Char;    (* current character *)
yylastchar : Char;    (* last matched character (#0 if none) *)
yyrule     : Integer; (* matched rule *)
yyreject   : Boolean; (* current match rejected? *)
yydone     : Boolean; (* yylex return value set? *)
yyretval   : Integer; (* yylex return value *)

procedure yynew;
  (* starts next match; initializes state information of the lexical
     analyzer *)

procedure yyscan;
  (* gets next character from the input stream and updates yytext and
     yyactchar accordingly *)

procedure yymark ( n : Integer );
  (* marks position for rule no. n *)

procedure yymatch ( n : Integer );
  (* declares a match for rule number n *)

function yyfind ( var n : Integer ) : Boolean;
  (* finds the last match and the corresponding marked position and adjusts
     the matched string accordingly; returns:
     - true if a rule has been matched, false otherwise
     - n: the number of the matched rule *)

function yydefault : Boolean;
  (* executes the default action (copy character); returns true unless
     at end-of-file *)

procedure yyclear;
  (* reinitializes state information after lexical analysis has been
     finished *)

implementation


procedure fatal ( msg : String );
  (* writes a fatal error message and halts program *)
var
  fulltext: string;
begin
  fulltext := filename + '(' + IntToStr(yylineno) + '):' + msg;
{$IFDEF MSWINDOWS}
  FileWrite(GetStdHandle(STD_ERROR_HANDLE), PChar(fulltext)^, Length(fulltext));
{$ENDIF}
{$IFDEF LINUX}
  FileWrite(STDERR_FILENO, PChar(fulltext)^, Length(fulltext));
{$ENDIF}
  halt(1);
end(*fatal*);

(* I/O routines: *)

const nl = #10;  (* newline character *)

const max_chars = 10000;

var

bufptr : Integer;
buf    : array [1..max_chars] of Char;

function get_char : Char;
  var i : Integer;
      str : array[0..1024] of char;
  begin
    if (bufptr=0) and (yyinput.Position < yyinput.Size) then
      begin
        //readln(yyinput, yyline);
        i := 0;
        yyline := '';
        FillChar(str,SizeOf(str),0);
        while (yyinput.Position < yyinput.Size) do
          begin
            yyinput.Read(str[i],1);
            if str[i] = #13 then
              begin
                yyinput.Read(str[i],1); // #10
                str[i] := #0;
                break;
              end
            else if str[i] = #10 then
              begin
                str[i] := #0;
                break;
              end;
            inc(i);
          end;
        yyline := StrPas(@str);
        inc(yylineno); yycolno := 1;
        buf[1] := nl;
        for i := 1 to length(yyline) do
          buf[i+1] := yyline[length(yyline)-i+1];
        inc(bufptr, length(yyline)+1);
      end;
    if bufptr>0 then
      begin
        get_char := buf[bufptr];
        dec(bufptr);
        inc(yycolno);
      end
    else
      get_char := #0;
  end(*get_char*);

procedure unget_char ( c : Char );
  begin
    if bufptr=max_chars then fatal('input buffer overflow');
    inc(bufptr);
    dec(yycolno);
    buf[bufptr] := c;
  end(*unget_char*);

procedure put_char ( c : Char );
  begin
    if c=#0 then
      { ignore }
    else if c=nl then
      yyoutput.Write(CRLF,2)
    else
      yyoutput.Write(c,1);
  end(*put_char*);

(* Variables:

   Some state information is maintained to keep track with calls to yymore,
   yyless, reject, start and yymatch/yymark, and to initialize state
   information used by the lexical analyzer.
   - yystext: contains the initial contents of the yytext variable; this
     will be the empty string, unless yymore is called which sets yystext
     to the current yytext
   - yysstate: start state of lexical analyzer (set to 0 during
     initialization, and modified in calls to the start routine)
   - yylstate: line state information (1 if at beginning of line, 0
     otherwise)
   - yystack: stack containing matched rules; yymatches contains the number of
     matches
   - yypos: for each rule the last marked position (yymark); zeroed when rule
     has already been considered
   - yysleng: copy of the original yyleng used to restore state information
     when reject is used *)

const

max_matches = 4096;
max_rules   = 512;

var

yystext            : String;
yysstate, yylstate : Integer;
yymatches          : Integer;
yystack            : array [1..max_matches] of Integer;
yypos              : array [1..max_rules] of Integer;
yysleng            : Byte;

(* Utilities: *)

procedure echo;
  var i : Integer;
  begin
    for i := 1 to yyleng do
      put_char(yytext[i])
  end(*echo*);

procedure yymore;
  begin
    yystext := yytext;
  end(*yymore*);

procedure yyless ( n : Integer );
{procedure yyless ( n : Integer );}
  var i : Integer;
  begin
    for i := yyleng downto n+1 do
      unget_char(yytext[i]);
    yyleng := n;
  end(*yyless*);

procedure reject;
  var i : Integer;
  begin
    yyreject := true;
    for i := yyleng+1 to yysleng do
      yytext := yytext+get_char;
    dec(yymatches);
  end(*reject*);

procedure return ( n : Integer );
  begin
    yyretval := n;
    yydone := true;
  end(*return*);

procedure returnc ( c : Char );
  begin
    yyretval := ord(c);
    yydone := true;
  end(*returnc*);

procedure start ( state : Integer );
  begin
    yysstate := state;
  end(*start*);

(* yywrap: *)

function yywrap : Boolean;
  begin
    //close(yyinput);
    yyinput.Position := 0;
    //close(yyoutput);
    yywrap := true;
  end(*yywrap*);

(* Internal routines: *)

procedure yynew;
  begin
    if yylastchar<>#0 then
      if yylastchar=nl then
        yylstate := 1
      else
        yylstate := 0;
    yystate := yysstate+yylstate;
    yytext  := yystext;
    yystext := '';
    yymatches := 0;
    yydone := false;
  end(*yynew*);

procedure yyscan;
  begin
    if yyleng=255 then
      fatal('yytext overflow');
    yyactchar := get_char;
    inc(yyleng);
    yytext[yyleng] := yyactchar;
  end(*yyscan*);

procedure yymark ( n : Integer );
  begin
    if n>max_rules then
      fatal('too many rules');
    yypos[n] := yyleng;
  end(*yymark*);

procedure yymatch ( n : Integer );
  begin
    inc(yymatches);
    if yymatches>max_matches then
      fatal('match stack overflow');
    yystack[yymatches] := n;
  end(*yymatch*);

function yyfind ( var n : Integer ) : Boolean;
  begin
    yyreject := false;
    while (yymatches>0) and (yypos[yystack[yymatches]]=0) do
      dec(yymatches);
    if yymatches>0 then
      begin
        yysleng := yyleng;
        n       := yystack[yymatches];
        yyless(yypos[n]);
        yypos[n] := 0;
        if yyleng>0 then
          yylastchar := yytext[yyleng]
        else
          yylastchar := #0;
        yyfind := true;
      end
    else
      begin
        yyless(0);
        yylastchar := #0;
        yyfind := false;
      end
  end(*yyfind*);

function yydefault : Boolean;
  begin
    yyreject := false;
    yyactchar := get_char;
    if yyactchar<>#0 then
      begin
        put_char(yyactchar);
        yydefault := true;
      end
    else
      begin
        yylstate := 1;
        yydefault := false;
      end;
    yylastchar := yyactchar;
  end(*yydefault*);

procedure yyclear;
  begin
    bufptr := 0;
    yysstate := 0;
    yylstate := 1;
    yylastchar := #0;
    yytext := '';
    yystext := '';
  end(*yyclear*);

procedure PrintError(msg: string);
var
  fulltext : string;
begin
  fulltext := filename + '('+ inttostr(yylineno) + '):' + msg;
  writeln(fulltext);
  halt(1);
end;

procedure yyprintln(str: string);
begin
  if str <> '' then
    yyoutput.Write(PChar(str)^,length(str));
  yyoutput.Write(CRLF,2);
end;

begin
  //assign(yyinput, '');
  //assign(yyoutput, '');
  //reset(yyinput);
  //rewrite(yyoutput);
  yylineno := 0;
  yyclear;
end(*LexLib*).
