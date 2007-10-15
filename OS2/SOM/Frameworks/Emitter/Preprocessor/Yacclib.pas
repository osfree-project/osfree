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

unit Yacclib;

(* Yacc Library Unit for TP Yacc Version 3.0, 6-17-91 AG *)

interface

{$I dorb.inc}

const yymaxdepth = 1024;
  (* default stack size of parser *)

type YYSType = Integer;
  (* default value type, may be redefined in Yacc output file *)

var

yychar   : Integer; (* current lookahead character *)
yynerrs  : Integer; (* current number of syntax errors reported by the
                       parser *)
yydebug  : Boolean; (* set to true to enable debugging output of parser *)

procedure yyerror ( msg : String );
  (* error message printing routine used by the parser *)

procedure yyclearin;
  (* delete the current lookahead token *)

procedure yyaccept;
  (* trigger accept action of the parser; yyparse accepts returning 0, as if
     it reached end of input *)

procedure yyabort;
  (* like yyaccept, but causes parser to return with value 1, as if an
     unrecoverable syntax error had been encountered *)

procedure yyerrlab;
  (* causes error recovery to be started, as if a syntax error had been
     encountered *)

procedure yyerrok;
  (* when in error mode, resets the parser to its normal mode of
     operation *)

(* Flags used internally by the parser routine: *)

var

yyflag    : ( yyfnone, yyfaccept, yyfabort, yyferror );
yyerrflag : Integer;

implementation

uses
  Lexlib,SysUtils,{$IFDEF MSWINDOWS}Windows,{$ENDIF}{$IFDEF LINUX}Libc,{$ENDIF}parser;

procedure yyerror ( msg : String );
  var
    fulltext : string;
  begin
    fulltext := current_file +'('+inttostr(idl_line_no)+ '):'+msg;
{$IFDEF MSWINDOWS}
    FileWrite(GetStdHandle(STD_ERROR_HANDLE), PChar(fulltext)^, Length(fulltext));
{$ENDIF}
{$IFDEF LINUX}
    FileWrite(STDERR_FILENO, PChar(fulltext)^, Length(fulltext));
{$ENDIF}
  end(*yyerrmsg*);

procedure yyclearin;
  begin
    yychar := -1;
  end(*yyclearin*);

procedure yyaccept;
  begin
    yyflag := yyfaccept;
  end(*yyaccept*);

procedure yyabort;
  begin
    yyflag := yyfabort;
  end(*yyabort*);

procedure yyerrlab;
  begin
    yyflag := yyferror;
  end(*yyerrlab*);

procedure yyerrok;
  begin
    yyerrflag := 0;
  end(*yyerrork*);

end(*YaccLib*).
