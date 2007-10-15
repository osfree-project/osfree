
(* lexical analyzer template (TP Lex V3.0), V1.0 3-2-91 AG *)

(* global definitions: *)
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
unit cpp;
interface
uses
  Classes,{$IFDEF MSWINDOWS}Windows,{$ENDIF}SysUtils,Lexlib,Yacclib,codeutil;

  function yylex: integer;

var
  includes : TStringList;
  defined  : TStringList;
  _enabled : array[0..100] of boolean;
  stack_pos : integer;
  disabled_at : integer;

implementation

procedure include_file(const fname: string; local: boolean); forward;
function enabled : Boolean; forward;
procedure _define(val: string); forward;
procedure _undef(val: string); forward;
procedure _ifdef(val: string); forward;
procedure _ifndef(val: string); forward;
procedure _endif; forward;
procedure _else; forward;
procedure _print; forward;



const INCLUDE = 2;
const LOCAL = 4;
const FAR = 6;
const COMMENT = 8;


function yylex : Integer;

procedure yyaction ( yyruleno : Integer );
  (* local definitions: *)

begin
  (* actions: *)
  case yyruleno of
  1:
                       yyprintln('');
  2:
                       start(COMMENT);
  3:
                       ;
  4:
                       begin
                         yyprintln('');
                         start(0);
                       end;
  5:
                       start(0);
  6:
                       ;
  7:
                       yyprintln('');
  8:
                       start(LOCAL);
  9:
                       start(FAR);
  10:
                       begin
                         include_file(copy(yytext,1,pos('"',yytext)-1),true);
                         start(0);
                       end;
  11:
                       begin
                         include_file(copy(yytext,1,pos('>',yytext)-1),false);
                         start(0);
                       end;
  12:
                       _define(trim(copy(yytext,8,200)));
  13:
                       _undef(trim(copy(yytext,7,200)));
  14:
                       _ifdef(trim(copy(yytext,7,200)));
  15:
                       _ifndef(trim(copy(yytext,8,200)));
  16:
                       _endif;
  17:
                       _else;
  18:
                       start(INCLUDE);
  19:
                       _print;
  end;
end(*yyaction*);

(* DFA table: *)

type YYTRec = record
                cc : set of Char;
                s  : Integer;
              end;

const

yynmarks   = 40;
yynmatches = 40;
yyntrans   = 136;
yynstates  = 88;

yyk : array [1..yynmarks] of Integer = (
  { 0: }
  { 1: }
  { 2: }
  { 3: }
  { 4: }
  { 5: }
  { 6: }
  { 7: }
  { 8: }
  { 9: }
  { 10: }
  19,
  { 11: }
  19,
  { 12: }
  19,
  { 13: }
  8,
  19,
  { 14: }
  9,
  19,
  { 15: }
  19,
  { 16: }
  19,
  { 17: }
  19,
  { 18: }
  19,
  { 19: }
  6,
  19,
  { 20: }
  6,
  19,
  { 21: }
  6,
  19,
  { 22: }
  7,
  { 23: }
  6,
  19,
  { 24: }
  { 25: }
  2,
  { 26: }
  { 27: }
  { 28: }
  { 29: }
  { 30: }
  { 31: }
  2,
  { 32: }
  { 33: }
  { 34: }
  10,
  { 35: }
  { 36: }
  { 37: }
  { 38: }
  2,
  3,
  { 39: }
  5,
  { 40: }
  1,
  { 41: }
  { 42: }
  { 43: }
  { 44: }
  { 45: }
  { 46: }
  { 47: }
  { 48: }
  1,
  10,
  { 49: }
  { 50: }
  11,
  { 51: }
  4,
  { 52: }
  { 53: }
  { 54: }
  { 55: }
  { 56: }
  { 57: }
  { 58: }
  { 59: }
  1,
  11,
  { 60: }
  { 61: }
  { 62: }
  { 63: }
  { 64: }
  { 65: }
  { 66: }
  { 67: }
  { 68: }
  { 69: }
  { 70: }
  { 71: }
  { 72: }
  { 73: }
  17,
  { 74: }
  { 75: }
  { 76: }
  { 77: }
  { 78: }
  { 79: }
  16,
  { 80: }
  { 81: }
  13,
  { 82: }
  14,
  { 83: }
  { 84: }
  { 85: }
  12,
  { 86: }
  15,
  { 87: }
  18
);

yym : array [1..yynmatches] of Integer = (
{ 0: }
{ 1: }
{ 2: }
{ 3: }
{ 4: }
{ 5: }
{ 6: }
{ 7: }
{ 8: }
{ 9: }
{ 10: }
  19,
{ 11: }
  19,
{ 12: }
  19,
{ 13: }
  8,
  19,
{ 14: }
  9,
  19,
{ 15: }
  19,
{ 16: }
  19,
{ 17: }
  19,
{ 18: }
  19,
{ 19: }
  6,
  19,
{ 20: }
  6,
  19,
{ 21: }
  6,
  19,
{ 22: }
  7,
{ 23: }
  6,
  19,
{ 24: }
{ 25: }
  2,
{ 26: }
{ 27: }
{ 28: }
{ 29: }
{ 30: }
{ 31: }
  2,
{ 32: }
{ 33: }
{ 34: }
  10,
{ 35: }
{ 36: }
{ 37: }
{ 38: }
  2,
  3,
{ 39: }
  5,
{ 40: }
  1,
{ 41: }
{ 42: }
{ 43: }
{ 44: }
{ 45: }
{ 46: }
{ 47: }
{ 48: }
  1,
  10,
{ 49: }
{ 50: }
  11,
{ 51: }
  4,
{ 52: }
{ 53: }
{ 54: }
{ 55: }
{ 56: }
{ 57: }
{ 58: }
{ 59: }
  1,
  11,
{ 60: }
{ 61: }
{ 62: }
{ 63: }
{ 64: }
{ 65: }
{ 66: }
{ 67: }
{ 68: }
{ 69: }
{ 70: }
{ 71: }
{ 72: }
{ 73: }
  17,
{ 74: }
{ 75: }
{ 76: }
{ 77: }
{ 78: }
{ 79: }
  16,
{ 80: }
{ 81: }
  13,
{ 82: }
  14,
{ 83: }
{ 84: }
{ 85: }
  12,
{ 86: }
  15,
{ 87: }
  18
);

yyt : array [1..yyntrans] of YYTrec = (
{ 0: }
  ( cc: [ #1..#9,#11..'.','0'..#255 ]; s: 11),
  ( cc: [ '/' ]; s: 10),
{ 1: }
  ( cc: [ #1..#9,#11..'"','$'..'.','0'..#255 ]; s: 11),
  ( cc: [ '#' ]; s: 12),
  ( cc: [ '/' ]; s: 10),
{ 2: }
  ( cc: [ #1..#9,#11..'!','#'..'.','0'..';','='..#255 ]; s: 11),
  ( cc: [ '"' ]; s: 13),
  ( cc: [ '/' ]; s: 10),
  ( cc: [ '<' ]; s: 14),
{ 3: }
  ( cc: [ #1..#9,#11..'!','$'..'.','0'..';','='..#255 ]; s: 11),
  ( cc: [ '"' ]; s: 13),
  ( cc: [ '#' ]; s: 12),
  ( cc: [ '/' ]; s: 10),
  ( cc: [ '<' ]; s: 14),
{ 4: }
  ( cc: [ #1..#9,#11..#31,'!'..',',':'..'@','[',']','^',
            '`','{'..#127 ]; s: 11),
  ( cc: [ ' ','-','.','0'..'9','A'..'Z','\','_','a'..'z',
            #128..#255 ]; s: 16),
  ( cc: [ '/' ]; s: 15),
{ 5: }
  ( cc: [ #1..#9,#11..#31,'!','"','$'..',',':'..'@',
            '[',']','^','`','{'..#127 ]; s: 11),
  ( cc: [ ' ','-','.','0'..'9','A'..'Z','\','_','a'..'z',
            #128..#255 ]; s: 16),
  ( cc: [ '#' ]; s: 12),
  ( cc: [ '/' ]; s: 15),
{ 6: }
  ( cc: [ #1..#9,#11..#31,'!'..',',':'..'@','[',']','^',
            '`','{'..#127 ]; s: 11),
  ( cc: [ ' ','-','.','0'..'9','A'..'Z','\','_','a'..'z',
            #128..#255 ]; s: 18),
  ( cc: [ '/' ]; s: 17),
{ 7: }
  ( cc: [ #1..#9,#11..#31,'!','"','$'..',',':'..'@',
            '[',']','^','`','{'..#127 ]; s: 11),
  ( cc: [ ' ','-','.','0'..'9','A'..'Z','\','_','a'..'z',
            #128..#255 ]; s: 18),
  ( cc: [ '#' ]; s: 12),
  ( cc: [ '/' ]; s: 17),
{ 8: }
  ( cc: [ #1..#9,#11..')','+'..'.','0'..#255 ]; s: 21),
  ( cc: [ #10 ]; s: 22),
  ( cc: [ '*' ]; s: 20),
  ( cc: [ '/' ]; s: 19),
{ 9: }
  ( cc: [ #1..#9,#11..'"','$'..')','+'..'.','0'..#255 ]; s: 21),
  ( cc: [ #10 ]; s: 22),
  ( cc: [ '#' ]; s: 23),
  ( cc: [ '*' ]; s: 20),
  ( cc: [ '/' ]; s: 19),
{ 10: }
  ( cc: [ '*' ]; s: 25),
  ( cc: [ '/' ]; s: 24),
{ 11: }
{ 12: }
  ( cc: [ 'd' ]; s: 26),
  ( cc: [ 'e' ]; s: 29),
  ( cc: [ 'i' ]; s: 28),
  ( cc: [ 'u' ]; s: 27),
{ 13: }
{ 14: }
{ 15: }
  ( cc: [ #1..#9,#11..#31,'!'..')','+',',',':'..'@',
            '[',']','^','`','{'..#127 ]; s: 33),
  ( cc: [ #10 ]; s: 34),
  ( cc: [ ' ','-','.','0'..'9','A'..'Z','\','_','a'..'z',
            #128..#255 ]; s: 32),
  ( cc: [ '*' ]; s: 31),
  ( cc: [ '/' ]; s: 30),
{ 16: }
  ( cc: [ #1..#9,#11..#31,'!'..',',':'..'@','[',']','^',
            '`','{'..#127 ]; s: 33),
  ( cc: [ #10 ]; s: 34),
  ( cc: [ ' ','-'..'9','A'..'Z','\','_','a'..'z',#128..#255 ]; s: 32),
{ 17: }
  ( cc: [ ' ','-','.','0'..'9','A'..'Z','\','_','a'..'z',
            #128..#255 ]; s: 36),
  ( cc: [ '*' ]; s: 25),
  ( cc: [ '/' ]; s: 35),
  ( cc: [ '>' ]; s: 37),
{ 18: }
  ( cc: [ ' ','-'..'9','A'..'Z','\','_','a'..'z',#128..#255 ]; s: 36),
  ( cc: [ '>' ]; s: 37),
{ 19: }
  ( cc: [ '*' ]; s: 38),
  ( cc: [ '/' ]; s: 24),
{ 20: }
  ( cc: [ '/' ]; s: 39),
{ 21: }
{ 22: }
{ 23: }
  ( cc: [ 'd' ]; s: 26),
  ( cc: [ 'e' ]; s: 29),
  ( cc: [ 'i' ]; s: 28),
  ( cc: [ 'u' ]; s: 27),
{ 24: }
  ( cc: [ #1..#9,#11..#255 ]; s: 24),
  ( cc: [ #10 ]; s: 40),
{ 25: }
{ 26: }
  ( cc: [ 'e' ]; s: 41),
{ 27: }
  ( cc: [ 'n' ]; s: 42),
{ 28: }
  ( cc: [ 'f' ]; s: 43),
  ( cc: [ 'n' ]; s: 44),
{ 29: }
  ( cc: [ 'l' ]; s: 46),
  ( cc: [ 'n' ]; s: 45),
{ 30: }
  ( cc: [ #1..#9,#11..#31,'!'..',',':'..'@','[',']','^',
            '`','{'..#127 ]; s: 47),
  ( cc: [ #10 ]; s: 48),
  ( cc: [ ' ','-'..'9','A'..'Z','\','_','a'..'z',#128..#255 ]; s: 30),
{ 31: }
  ( cc: [ #1..#9,#11..#255 ]; s: 33),
  ( cc: [ #10 ]; s: 34),
{ 32: }
  ( cc: [ #1..#9,#11..#31,'!'..',',':'..'@','[',']','^',
            '`','{'..#127 ]; s: 33),
  ( cc: [ #10 ]; s: 34),
  ( cc: [ ' ','-'..'9','A'..'Z','\','_','a'..'z',#128..#255 ]; s: 32),
{ 33: }
  ( cc: [ #1..#9,#11..#255 ]; s: 33),
  ( cc: [ #10 ]; s: 34),
{ 34: }
{ 35: }
  ( cc: [ #1..#9,#11..#31,'!'..',',':'..'=','?','@',
            '[',']','^','`','{'..#127 ]; s: 24),
  ( cc: [ #10 ]; s: 40),
  ( cc: [ ' ','-'..'9','A'..'Z','\','_','a'..'z',#128..#255 ]; s: 35),
  ( cc: [ '>' ]; s: 49),
{ 36: }
  ( cc: [ ' ','-'..'9','A'..'Z','\','_','a'..'z',#128..#255 ]; s: 36),
  ( cc: [ '>' ]; s: 37),
{ 37: }
  ( cc: [ #1..#9,#11..#255 ]; s: 37),
  ( cc: [ #10 ]; s: 50),
{ 38: }
{ 39: }
  ( cc: [ #10 ]; s: 51),
{ 40: }
{ 41: }
  ( cc: [ 'f' ]; s: 52),
{ 42: }
  ( cc: [ 'd' ]; s: 53),
{ 43: }
  ( cc: [ 'd' ]; s: 54),
  ( cc: [ 'n' ]; s: 55),
{ 44: }
  ( cc: [ 'c' ]; s: 56),
{ 45: }
  ( cc: [ 'd' ]; s: 57),
{ 46: }
  ( cc: [ 's' ]; s: 58),
{ 47: }
  ( cc: [ #1..#9,#11..#255 ]; s: 47),
  ( cc: [ #10 ]; s: 48),
{ 48: }
{ 49: }
  ( cc: [ #1..#9,#11..#255 ]; s: 49),
  ( cc: [ #10 ]; s: 59),
{ 50: }
{ 51: }
{ 52: }
  ( cc: [ 'i' ]; s: 60),
{ 53: }
  ( cc: [ 'e' ]; s: 61),
{ 54: }
  ( cc: [ 'e' ]; s: 62),
{ 55: }
  ( cc: [ 'd' ]; s: 63),
{ 56: }
  ( cc: [ 'l' ]; s: 64),
{ 57: }
  ( cc: [ 'i' ]; s: 65),
{ 58: }
  ( cc: [ 'e' ]; s: 66),
{ 59: }
{ 60: }
  ( cc: [ 'n' ]; s: 67),
{ 61: }
  ( cc: [ 'f' ]; s: 68),
{ 62: }
  ( cc: [ 'f' ]; s: 69),
{ 63: }
  ( cc: [ 'e' ]; s: 70),
{ 64: }
  ( cc: [ 'u' ]; s: 71),
{ 65: }
  ( cc: [ 'f' ]; s: 72),
{ 66: }
  ( cc: [ #1..#9,#11..#255 ]; s: 66),
  ( cc: [ #10 ]; s: 73),
{ 67: }
  ( cc: [ 'e' ]; s: 74),
{ 68: }
  ( cc: [ ' ' ]; s: 75),
{ 69: }
  ( cc: [ ' ' ]; s: 76),
{ 70: }
  ( cc: [ 'f' ]; s: 77),
{ 71: }
  ( cc: [ 'd' ]; s: 78),
{ 72: }
  ( cc: [ #1..#9,#11..#255 ]; s: 72),
  ( cc: [ #10 ]; s: 79),
{ 73: }
{ 74: }
  ( cc: [ ' ' ]; s: 80),
{ 75: }
  ( cc: [ #1..#9,#11..#255 ]; s: 75),
  ( cc: [ #10 ]; s: 81),
{ 76: }
  ( cc: [ #1..#9,#11..#255 ]; s: 76),
  ( cc: [ #10 ]; s: 82),
{ 77: }
  ( cc: [ ' ' ]; s: 83),
{ 78: }
  ( cc: [ 'e' ]; s: 84),
{ 79: }
{ 80: }
  ( cc: [ #1..#9,#11..#255 ]; s: 80),
  ( cc: [ #10 ]; s: 85),
{ 81: }
{ 82: }
{ 83: }
  ( cc: [ #1..#9,#11..#255 ]; s: 83),
  ( cc: [ #10 ]; s: 86),
{ 84: }
  ( cc: [ ' ' ]; s: 87)
{ 85: }
{ 86: }
{ 87: }
);

yykl : array [0..yynstates-1] of Integer = (
{ 0: } 1,
{ 1: } 1,
{ 2: } 1,
{ 3: } 1,
{ 4: } 1,
{ 5: } 1,
{ 6: } 1,
{ 7: } 1,
{ 8: } 1,
{ 9: } 1,
{ 10: } 1,
{ 11: } 2,
{ 12: } 3,
{ 13: } 4,
{ 14: } 6,
{ 15: } 8,
{ 16: } 9,
{ 17: } 10,
{ 18: } 11,
{ 19: } 12,
{ 20: } 14,
{ 21: } 16,
{ 22: } 18,
{ 23: } 19,
{ 24: } 21,
{ 25: } 21,
{ 26: } 22,
{ 27: } 22,
{ 28: } 22,
{ 29: } 22,
{ 30: } 22,
{ 31: } 22,
{ 32: } 23,
{ 33: } 23,
{ 34: } 23,
{ 35: } 24,
{ 36: } 24,
{ 37: } 24,
{ 38: } 24,
{ 39: } 26,
{ 40: } 27,
{ 41: } 28,
{ 42: } 28,
{ 43: } 28,
{ 44: } 28,
{ 45: } 28,
{ 46: } 28,
{ 47: } 28,
{ 48: } 28,
{ 49: } 30,
{ 50: } 30,
{ 51: } 31,
{ 52: } 32,
{ 53: } 32,
{ 54: } 32,
{ 55: } 32,
{ 56: } 32,
{ 57: } 32,
{ 58: } 32,
{ 59: } 32,
{ 60: } 34,
{ 61: } 34,
{ 62: } 34,
{ 63: } 34,
{ 64: } 34,
{ 65: } 34,
{ 66: } 34,
{ 67: } 34,
{ 68: } 34,
{ 69: } 34,
{ 70: } 34,
{ 71: } 34,
{ 72: } 34,
{ 73: } 34,
{ 74: } 35,
{ 75: } 35,
{ 76: } 35,
{ 77: } 35,
{ 78: } 35,
{ 79: } 35,
{ 80: } 36,
{ 81: } 36,
{ 82: } 37,
{ 83: } 38,
{ 84: } 38,
{ 85: } 38,
{ 86: } 39,
{ 87: } 40
);

yykh : array [0..yynstates-1] of Integer = (
{ 0: } 0,
{ 1: } 0,
{ 2: } 0,
{ 3: } 0,
{ 4: } 0,
{ 5: } 0,
{ 6: } 0,
{ 7: } 0,
{ 8: } 0,
{ 9: } 0,
{ 10: } 1,
{ 11: } 2,
{ 12: } 3,
{ 13: } 5,
{ 14: } 7,
{ 15: } 8,
{ 16: } 9,
{ 17: } 10,
{ 18: } 11,
{ 19: } 13,
{ 20: } 15,
{ 21: } 17,
{ 22: } 18,
{ 23: } 20,
{ 24: } 20,
{ 25: } 21,
{ 26: } 21,
{ 27: } 21,
{ 28: } 21,
{ 29: } 21,
{ 30: } 21,
{ 31: } 22,
{ 32: } 22,
{ 33: } 22,
{ 34: } 23,
{ 35: } 23,
{ 36: } 23,
{ 37: } 23,
{ 38: } 25,
{ 39: } 26,
{ 40: } 27,
{ 41: } 27,
{ 42: } 27,
{ 43: } 27,
{ 44: } 27,
{ 45: } 27,
{ 46: } 27,
{ 47: } 27,
{ 48: } 29,
{ 49: } 29,
{ 50: } 30,
{ 51: } 31,
{ 52: } 31,
{ 53: } 31,
{ 54: } 31,
{ 55: } 31,
{ 56: } 31,
{ 57: } 31,
{ 58: } 31,
{ 59: } 33,
{ 60: } 33,
{ 61: } 33,
{ 62: } 33,
{ 63: } 33,
{ 64: } 33,
{ 65: } 33,
{ 66: } 33,
{ 67: } 33,
{ 68: } 33,
{ 69: } 33,
{ 70: } 33,
{ 71: } 33,
{ 72: } 33,
{ 73: } 34,
{ 74: } 34,
{ 75: } 34,
{ 76: } 34,
{ 77: } 34,
{ 78: } 34,
{ 79: } 35,
{ 80: } 35,
{ 81: } 36,
{ 82: } 37,
{ 83: } 37,
{ 84: } 37,
{ 85: } 38,
{ 86: } 39,
{ 87: } 40
);

yyml : array [0..yynstates-1] of Integer = (
{ 0: } 1,
{ 1: } 1,
{ 2: } 1,
{ 3: } 1,
{ 4: } 1,
{ 5: } 1,
{ 6: } 1,
{ 7: } 1,
{ 8: } 1,
{ 9: } 1,
{ 10: } 1,
{ 11: } 2,
{ 12: } 3,
{ 13: } 4,
{ 14: } 6,
{ 15: } 8,
{ 16: } 9,
{ 17: } 10,
{ 18: } 11,
{ 19: } 12,
{ 20: } 14,
{ 21: } 16,
{ 22: } 18,
{ 23: } 19,
{ 24: } 21,
{ 25: } 21,
{ 26: } 22,
{ 27: } 22,
{ 28: } 22,
{ 29: } 22,
{ 30: } 22,
{ 31: } 22,
{ 32: } 23,
{ 33: } 23,
{ 34: } 23,
{ 35: } 24,
{ 36: } 24,
{ 37: } 24,
{ 38: } 24,
{ 39: } 26,
{ 40: } 27,
{ 41: } 28,
{ 42: } 28,
{ 43: } 28,
{ 44: } 28,
{ 45: } 28,
{ 46: } 28,
{ 47: } 28,
{ 48: } 28,
{ 49: } 30,
{ 50: } 30,
{ 51: } 31,
{ 52: } 32,
{ 53: } 32,
{ 54: } 32,
{ 55: } 32,
{ 56: } 32,
{ 57: } 32,
{ 58: } 32,
{ 59: } 32,
{ 60: } 34,
{ 61: } 34,
{ 62: } 34,
{ 63: } 34,
{ 64: } 34,
{ 65: } 34,
{ 66: } 34,
{ 67: } 34,
{ 68: } 34,
{ 69: } 34,
{ 70: } 34,
{ 71: } 34,
{ 72: } 34,
{ 73: } 34,
{ 74: } 35,
{ 75: } 35,
{ 76: } 35,
{ 77: } 35,
{ 78: } 35,
{ 79: } 35,
{ 80: } 36,
{ 81: } 36,
{ 82: } 37,
{ 83: } 38,
{ 84: } 38,
{ 85: } 38,
{ 86: } 39,
{ 87: } 40
);

yymh : array [0..yynstates-1] of Integer = (
{ 0: } 0,
{ 1: } 0,
{ 2: } 0,
{ 3: } 0,
{ 4: } 0,
{ 5: } 0,
{ 6: } 0,
{ 7: } 0,
{ 8: } 0,
{ 9: } 0,
{ 10: } 1,
{ 11: } 2,
{ 12: } 3,
{ 13: } 5,
{ 14: } 7,
{ 15: } 8,
{ 16: } 9,
{ 17: } 10,
{ 18: } 11,
{ 19: } 13,
{ 20: } 15,
{ 21: } 17,
{ 22: } 18,
{ 23: } 20,
{ 24: } 20,
{ 25: } 21,
{ 26: } 21,
{ 27: } 21,
{ 28: } 21,
{ 29: } 21,
{ 30: } 21,
{ 31: } 22,
{ 32: } 22,
{ 33: } 22,
{ 34: } 23,
{ 35: } 23,
{ 36: } 23,
{ 37: } 23,
{ 38: } 25,
{ 39: } 26,
{ 40: } 27,
{ 41: } 27,
{ 42: } 27,
{ 43: } 27,
{ 44: } 27,
{ 45: } 27,
{ 46: } 27,
{ 47: } 27,
{ 48: } 29,
{ 49: } 29,
{ 50: } 30,
{ 51: } 31,
{ 52: } 31,
{ 53: } 31,
{ 54: } 31,
{ 55: } 31,
{ 56: } 31,
{ 57: } 31,
{ 58: } 31,
{ 59: } 33,
{ 60: } 33,
{ 61: } 33,
{ 62: } 33,
{ 63: } 33,
{ 64: } 33,
{ 65: } 33,
{ 66: } 33,
{ 67: } 33,
{ 68: } 33,
{ 69: } 33,
{ 70: } 33,
{ 71: } 33,
{ 72: } 33,
{ 73: } 34,
{ 74: } 34,
{ 75: } 34,
{ 76: } 34,
{ 77: } 34,
{ 78: } 34,
{ 79: } 35,
{ 80: } 35,
{ 81: } 36,
{ 82: } 37,
{ 83: } 37,
{ 84: } 37,
{ 85: } 38,
{ 86: } 39,
{ 87: } 40
);

yytl : array [0..yynstates-1] of Integer = (
{ 0: } 1,
{ 1: } 3,
{ 2: } 6,
{ 3: } 10,
{ 4: } 15,
{ 5: } 18,
{ 6: } 22,
{ 7: } 25,
{ 8: } 29,
{ 9: } 33,
{ 10: } 38,
{ 11: } 40,
{ 12: } 40,
{ 13: } 44,
{ 14: } 44,
{ 15: } 44,
{ 16: } 49,
{ 17: } 52,
{ 18: } 56,
{ 19: } 58,
{ 20: } 60,
{ 21: } 61,
{ 22: } 61,
{ 23: } 61,
{ 24: } 65,
{ 25: } 67,
{ 26: } 67,
{ 27: } 68,
{ 28: } 69,
{ 29: } 71,
{ 30: } 73,
{ 31: } 76,
{ 32: } 78,
{ 33: } 81,
{ 34: } 83,
{ 35: } 83,
{ 36: } 87,
{ 37: } 89,
{ 38: } 91,
{ 39: } 91,
{ 40: } 92,
{ 41: } 92,
{ 42: } 93,
{ 43: } 94,
{ 44: } 96,
{ 45: } 97,
{ 46: } 98,
{ 47: } 99,
{ 48: } 101,
{ 49: } 101,
{ 50: } 103,
{ 51: } 103,
{ 52: } 103,
{ 53: } 104,
{ 54: } 105,
{ 55: } 106,
{ 56: } 107,
{ 57: } 108,
{ 58: } 109,
{ 59: } 110,
{ 60: } 110,
{ 61: } 111,
{ 62: } 112,
{ 63: } 113,
{ 64: } 114,
{ 65: } 115,
{ 66: } 116,
{ 67: } 118,
{ 68: } 119,
{ 69: } 120,
{ 70: } 121,
{ 71: } 122,
{ 72: } 123,
{ 73: } 125,
{ 74: } 125,
{ 75: } 126,
{ 76: } 128,
{ 77: } 130,
{ 78: } 131,
{ 79: } 132,
{ 80: } 132,
{ 81: } 134,
{ 82: } 134,
{ 83: } 134,
{ 84: } 136,
{ 85: } 137,
{ 86: } 137,
{ 87: } 137
);

yyth : array [0..yynstates-1] of Integer = (
{ 0: } 2,
{ 1: } 5,
{ 2: } 9,
{ 3: } 14,
{ 4: } 17,
{ 5: } 21,
{ 6: } 24,
{ 7: } 28,
{ 8: } 32,
{ 9: } 37,
{ 10: } 39,
{ 11: } 39,
{ 12: } 43,
{ 13: } 43,
{ 14: } 43,
{ 15: } 48,
{ 16: } 51,
{ 17: } 55,
{ 18: } 57,
{ 19: } 59,
{ 20: } 60,
{ 21: } 60,
{ 22: } 60,
{ 23: } 64,
{ 24: } 66,
{ 25: } 66,
{ 26: } 67,
{ 27: } 68,
{ 28: } 70,
{ 29: } 72,
{ 30: } 75,
{ 31: } 77,
{ 32: } 80,
{ 33: } 82,
{ 34: } 82,
{ 35: } 86,
{ 36: } 88,
{ 37: } 90,
{ 38: } 90,
{ 39: } 91,
{ 40: } 91,
{ 41: } 92,
{ 42: } 93,
{ 43: } 95,
{ 44: } 96,
{ 45: } 97,
{ 46: } 98,
{ 47: } 100,
{ 48: } 100,
{ 49: } 102,
{ 50: } 102,
{ 51: } 102,
{ 52: } 103,
{ 53: } 104,
{ 54: } 105,
{ 55: } 106,
{ 56: } 107,
{ 57: } 108,
{ 58: } 109,
{ 59: } 109,
{ 60: } 110,
{ 61: } 111,
{ 62: } 112,
{ 63: } 113,
{ 64: } 114,
{ 65: } 115,
{ 66: } 117,
{ 67: } 118,
{ 68: } 119,
{ 69: } 120,
{ 70: } 121,
{ 71: } 122,
{ 72: } 124,
{ 73: } 124,
{ 74: } 125,
{ 75: } 127,
{ 76: } 129,
{ 77: } 130,
{ 78: } 131,
{ 79: } 131,
{ 80: } 133,
{ 81: } 133,
{ 82: } 133,
{ 83: } 135,
{ 84: } 136,
{ 85: } 136,
{ 86: } 136,
{ 87: } 136
);


var yyn : Integer;

label start, scan, action;

begin

start:

  (* initialize: *)

  yynew;

scan:

  (* mark positions and matches: *)

  for yyn := yykl[yystate] to     yykh[yystate] do yymark(yyk[yyn]);
  for yyn := yymh[yystate] downto yyml[yystate] do yymatch(yym[yyn]);

  if yytl[yystate]>yyth[yystate] then goto action; (* dead state *)

  (* get next character: *)

  yyscan;

  (* determine action: *)

  yyn := yytl[yystate];
  while (yyn<=yyth[yystate]) and not (yyactchar in yyt[yyn].cc) do inc(yyn);
  if yyn>yyth[yystate] then goto action;
    (* no transition on yyactchar in this state *)

  (* switch to new state: *)

  yystate := yyt[yyn].s;

  goto scan;

action:

  (* execute action: *)

  if yyfind(yyrule) then
    begin
      yyaction(yyrule);
      if yyreject then goto action;
    end
  else if not yydefault and yywrap then
    begin
      yyclear;
      return(0);
    end;

  if not yydone then goto start;

  yylex := yyretval;


end(*yylex*);



function enabled: Boolean;
begin
  result := (disabled_at = -1) and _enabled[stack_pos];
end;

procedure _define(val: string);
begin
  if not enabled then exit;
  defined.values[val] := 'TRUE';
  yyprintln('');
end;

procedure _undef(val: string);
begin
  if not enabled then exit;
  defined.values[val] := '';
  yyprintln('');
end;

procedure _ifdef(val: string);
begin
  inc(stack_pos);
  _enabled[stack_pos] := defined.values[val] = 'TRUE';
  if not _enabled[stack_pos] then disabled_at := stack_pos;
  yyprintln('');
end;

procedure _ifndef(val: string);
begin
  inc(stack_pos);
  _enabled[stack_pos] := defined.values[val] = '';
  if not _enabled[stack_pos] then disabled_at := stack_pos;
  yyprintln('');
end;

procedure _else;
begin
  _enabled[stack_pos] := not _enabled[stack_pos];
  if not _enabled[stack_pos] then
    disabled_at := stack_pos
  else if disabled_at = stack_pos then
    disabled_at := -1;
  yyprintln('');
end;

procedure _endif;
begin
  if disabled_at = stack_pos then
    disabled_at := -1;
  dec(stack_pos);
  yyprintln('');
end;

procedure _print;
begin
  if enabled then
     put_char(yytext[1]);
end;

procedure include_file(const fname: string; local: boolean);
var
  i : integer;
  found: Boolean;
  old_yyinput : TStream;
  old_yyline  : String;
  old_yylineno, old_yycolno : Integer;
  old_yytext  : ShortString;
  old_yyleng  : Byte;
  old_yystate    : Integer;
  old_yyactchar  : Char;
  old_yylastchar : Char;
  old_yyrule     : Integer;
  old_yyreject   : Boolean;
  old_yydone     : Boolean;
  old_yyretval   : Integer;
  old_filename,real_file_name : string;

{  function _ExtractFilePath(path: string): string;
  var
    I: Integer;
  begin
    I := LastDelimiter('\:/', FileName);
    Result := Copy(FileName, 1, I);
  end;}

  function _IncludeTrailingBackslash(path: string): string;
  begin
    result := path;
    if trim(path) = '' then exit;
    if (path[Length(path)] <> '/') and (path[Length(path)] <> '\') then
      result := path + '\'
  end;

begin
  if not enabled then
    begin
      yyprintln('');
      exit;
    end;
  real_file_name := fname;
  if not fileexists(fname) then
    begin
      if local then
        begin
          real_file_name := _IncludeTrailingBackslash(ExtractFilePath(filename))+fname;
          if not FileExists(real_file_name) then begin
            //PrintError('file not found "'+fname+'"');
          found := false;
          for i := 0 to pred(includes.count) do
            begin
              real_file_name := _IncludeTrailingBackslash(includes[i])+fname;
              if fileexists(real_file_name) then
                begin
                  found := true;
                  break;
                end;
            end;
          if not found then
            PrintError('file not found "'+fname+'"');
          end;
        end
      else
        begin
          found := false;
          for i := 0 to pred(includes.count) do
            begin
              real_file_name := _IncludeTrailingBackslash(includes[i])+fname;
              if fileexists(real_file_name) then
                begin
                  found := true;
                  break;
                end;
            end;
          if not found then
            PrintError('file not found "'+fname+'"')
        end;
    end;
  old_yyinput := yyinput;
  old_yyline  := yyline;
  old_yylineno := yylineno;
  old_yycolno := yycolno;
  old_yytext  := yytext;
  old_yyleng  := yyleng;
  old_yystate  := yystate;
  old_yyactchar  := yyactchar;
  old_yylastchar := yylastchar;
  old_yyrule     := yyrule;
  old_yyreject   := yyreject;
  old_yydone     := yydone;
  old_yyretval   := yyretval;
  old_filename := filename;
  yyinput := OpenFile(real_file_name);
  try
    filename := real_file_name;
    yylineno := 0;
    yycolno := 0;
    yystate := 0;
    yyclear;
    yyprintln('#line 1 "'+filename+'"');
    yylex;
  finally
    yyclear;
    //yyinput.Free;
    filename := old_filename;
    yyinput := old_yyinput;
    yyline  := old_yyline;
    yylineno := old_yylineno;
    yycolno := old_yycolno;
    yytext  := old_yytext;
    yyleng  := old_yyleng;
    yystate  := old_yystate;
    yyactchar  := old_yyactchar;
    yylastchar := old_yylastchar;
    yyrule     := old_yyrule;
    yyreject   := old_yyreject;
    yydone     := old_yydone;
    yyretval   := old_yyretval;
    yyprintln('#line '+inttostr(yylineno+1)+' "'+filename+'"');
    start(0);
  end;
end;

initialization
  includes := TStringList.Create;
  defined  := TStringList.Create;
  stack_pos := 0;
  _enabled[stack_pos] := true;
  disabled_at := -1;
finalization
  includes.free;
  defined.free;
end.
