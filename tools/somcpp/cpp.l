%{
// **********************************************************************
//
// Copyright (c) 2001 - 2002 MT Tools.
// Copyright (c) 2007 - 2008 osFree project.
//
// All Rights Reserved
//
// MT_DORB is based in part on the product DORB,
// written by Shadrin Victor
//
// **********************************************************************
unit cpp;
interface
uses
  Classes, SysUtils, Lexlib;

  function yylex: integer;

var
  includes : TStringList;
  defined  : TStringList;
  _enabled : array[0..100] of boolean;
  stack_pos : integer;
  disabled_at : integer;
  filename : string;

implementation

procedure include_file(const fname: string; local: boolean); forward;
function enabled : Boolean; forward;
procedure _define(val: string); forward;
procedure _undef(val: string); forward;
procedure _ifdef(val: string); forward;
procedure _if(val: string); forward;
procedure _ifndef(val: string); forward;
procedure _endif; forward;
procedure _else; forward;
procedure _print; forward;
procedure yyprintln(str: string); forward;
procedure PrintError(msg: string); forward;

%}

path       [ \-/\\A-Za-z0-9_\.€-Ÿ -ïÀ-ßà-ÿ]+

%start INCLUDE LOCAL FAR

%%

<INCLUDE>\"            start(LOCAL);
<INCLUDE>\<            start(FAR);
<LOCAL>{path}[^\n]*\n  begin
                         include_file(copy(yytext,1,pos('"',yytext)-1),true);
                         start(0);
                       end;
<FAR>{path}\>[^\n]*\n  begin
                         include_file(copy(yytext,1,pos('>',yytext)-1),false);
                         start(0);
                       end;
^[\t ]*"#define "[^\n]*\n    _define(trim(copy(trim(yytext),8,200)));
^[\t ]*"#undef "[^\n]*\n     _undef(trim(copy(trim(yytext),7,200)));
^[\t ]*"#ifdef "[^\n]*\n      _ifdef(trim(copy(trim(yytext),7,200)));
^[\t ]*"#if "[^\n]*\n           _if(trim(copy(trim(yytext),7,200)));
^[\t ]*"#ifndef "[^\n]*\n    _ifndef(trim(copy(trim(yytext),8,200)));
^[\t ]*"#endif"[^\n]*\n      _endif;
^[\t ]*"#else"[^\n]*\n       _else;
^[\t ]*"#include "           start(INCLUDE);
.                      _print;
%%

procedure yyprintln(str: string);
begin
  if str <> '' then
    Write(yyoutput, str);
  writeln(yyoutput);
end;

procedure PrintError(msg: string);
var
  fulltext : string;
begin
  fulltext := filename + '('+ inttostr(yylineno) + '):' + msg;
  writeln(fulltext);
  halt(1);
end;

function enabled: Boolean;
begin
  result := _enabled[stack_pos];
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
  _enabled[stack_pos] := (defined.values[val] = 'TRUE') and _enabled[stack_pos-1];
  yyprintln('');
end;

procedure _if(val: string);
begin
//  inc(stack_pos);
  writeln(val);
  readln;
//  _enabled[stack_pos] := (defined.values[val] = 'TRUE') and _enabled[stack_pos-1];
  yyprintln('');
end;



procedure _ifndef(val: string);
begin
  inc(stack_pos);
  _enabled[stack_pos] := (defined.values[val] = '') and _enabled[stack_pos-1];
  yyprintln('');
end;

procedure _else;
begin
  _enabled[stack_pos] := not _enabled[stack_pos];
  if not _enabled[stack_pos-1] then _enabled[stack_pos]:=false;
  yyprintln('');
end;

procedure _endif;
begin
  dec(stack_pos);
//  writeln(stack_pos);
  yyprintln('');
end;

procedure _print;
begin
  if enabled then
     put_char(yytext[1]);
end;

procedure include_file(const fname: string; local: boolean);
var
  i : longint;
  found: Boolean;
  s: string;
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

  function _ExtractFilePath(path: string): string;
  var
    I: Integer;
  begin
    I := LastDelimiter('\:/', FileName);
    Result := Copy(FileName, 1, I);
  end;

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
          real_file_name := _IncludeTrailingBackslash(_ExtractFilePath(filename))+fname;
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
  close(yyinput);
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
  Assign(yyinput, real_file_name);
  Reset(yyinput);
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
    filename := old_filename;
    assign(yyinput, old_filename);
    reset(yyinput);
    for i:=1 to old_yylineno do readln(yyinput, s);
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
    assign(yyoutput, '');
    rewrite(yyoutput);
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
