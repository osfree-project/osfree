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
// See Readme.txt for contact information
//
// **********************************************************************
program somcpp;
uses
  SysUtils,
  Classes,
  cpp,
  LexLib;

var
  name,str,sstr : string;
  fname_defined: Boolean;
  params: TStrings;
  i,j,p : integer;
  ch : Char;

procedure print_usage;
begin
  writeln('usage: somcpp [<options>] [<idlfile>]');
  writeln('where <options> are: ');
  writeln('  -h, -help       : print this message');
  writeln('  -v, -version    : print version');
  writeln('  -i<path>        : path for include files');
  writeln('  -c              : ??? leave comments??? not known switch...');
  writeln('  -d<name>        : define xxx');
  Halt(1);
end;

begin
  params := TStringList.Create;
  fname_defined := false;
  DecimalSeparator := '.';
  for i := 1 to Paramcount do
    begin
      name := lowercase(paramstr(i));
      if (name[1] = '-') and (name[2] <> '\') and (name[2] <> '/') then
        begin
          if pos('-i',name) = 1 then // includes search paths
            begin
              str := copy(name,3,1000);
              p := 1;
              j := 1;
              while j < length(str) do
                begin
                  if str[j] = ';' then
                    begin
                      sstr := copy(str,p,j-1);
                      if sstr <> '' then includes.add(sstr);
                      p := j+1;
                    end;
                  inc(j);
                end;
              sstr := copy(str,p,j);
              if sstr <> '' then includes.add(sstr);
            end
          else if pos('-d',name) = 1 then // module list without prefix
            begin
              defined.values[name] := 'TRUE';
            end
          else if (name = '-h') or (name = '-help') then
            print_usage
          else if (name = '-c') then
            //print_usage
          else if (name = '-v') or (name = '-version') then
            begin
              WriteLn('Product name: osFree SOM Compiler Preprocessor (based on parts of the MTDORB for Delphi and Kylix)');
              WriteLn('Copyright:    (C) 2001 - 2004 by Millennium Group');
              WriteLn('Copyright:    (C) 2007 - 2008 by osFree');
              Halt(1);
            end
          else
            begin
              writeln('unknown parameter :'+name);
              print_usage;
            end;
        end
      else if not fileexists(paramstr(i)) then
        begin
          writeln('file not found :'+paramstr(i));
          print_usage
        end
      else
        begin
          if fname_defined then
            begin
              writeln('name already defined :'+name);
              print_usage;
            end;
          fname_defined := true;
          params.Insert(0,{name}paramstr(i)); // case sensitive
          filename := {name}paramstr(i); // case sensitive
        end;
    end;
  if not fname_defined then
    begin
      writeln('missing file name');
      print_usage;
    end;
  try
      assign(yyinput, filename);
        try
          writeln(yyoutput, '#line 1 "'+filename+'"');
          reset(yyinput);
          cpp.yylex;
        finally
        end;
  finally
  end;
end.
