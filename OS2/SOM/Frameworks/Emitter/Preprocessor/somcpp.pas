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
program somcpp;
uses
  SysUtils,
//  ir_int,
//  orb_int,
//  orb,
//  ir,
//  version_int,
//  orbtypes,
  Classes,
//  idl in 'idl.pas',
//  parser in 'parser.pas',
//  codebase in 'codebase.pas',
//  scanner in 'scanner.pas',
//  codeunit in 'codeunit.pas',
  codeutil in 'codeutil.pas',
  cpp in 'cpp.pas',
//  idldb in 'idldb.pas',
  YaccLib in 'Yacclib.pas',
  LexLib in 'Lexlib.pas';

var
  name,str,sstr : string;
  preprocess,fname_defined: Boolean;
  omit_preprocess : Boolean;
//  con: IRepository;
  params: TStrings;
  i,j,p : integer;
//  db: TDB;
  ch : Char;

procedure print_usage;
begin
{
        somcpp -D__OS2__  -I. -IC:\_work\SVN\wpamptr_new\idl -IC:\_work\SVN\wpam
ptr_new\os2tk45cut\idl -IC:\_work\SVN\wpamptr_new\os2tk45cut\som\idl -IC:\_work\
SVN\wpamptr_new\os2tk45cut\som\include -D__SOMIDL_VERSION_1__  -D__SOMIDL__  -C
 ..\IDL\wpamptr.idl > C:\var\temp\7ed00000.CTN
}


  writeln('usage: idltopas [<options>] [<idlfile>]');
  writeln('where <options> are: ');
  writeln('  -h, -help       : print this message');
  writeln('  -v, -version    : print version');
  writeln('  -poa            : generate code for POA');
  writeln('  -i<path>        : path for include files');
  writeln('  -p              : preprocess to stdout');
  writeln('  -c            : ???');
  writeln('  -np             : don`t preprocess');
  writeln('  -impl           : generate example implementation');
  writeln('  -notabs         : generate servants and skeletons with implementation');
  writeln('  -without-prefix : generate types names without prefix');
  writeln('  -d<name>        : define xxx');
  writeln('  -guid           : store GUID in ini file');
  writeln('  -ns             : namespaces in file name');
  Halt(1);
end;

begin
  preprocess := true;
  omit_preprocess := false;
  params := TStringList.Create;
  fname_defined := false;
  DecimalSeparator := '.';
  for i := 1 to Paramcount do
    begin
      name := lowercase(paramstr(i));
      if (name[1] = '-') and (name[2] <> '\') and (name[2] <> '/') then
        begin
          if name = '-p' then
            preprocess := true
          else if name = '-np' then
            omit_preprocess := true
          else if (name = '-poa') or (name = '-impl') or (name = '-any') or (name = '-notabs') or (name = '-without-prefix') then
            params.add(name)
          else if pos('-i',name) = 1 then // includes search paths
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
//              defined.values[name] := 'TRUE';
            end
          else if (name = '-h') or (name = '-help') then
            print_usage
          else if (name = '-c') then
            //print_usage
          else if (name = '-v') or (name = '-version') then
            begin
              WriteLn('Product name: Idl to pascal compiler (part of the MTDORB for Delphi and Kylix)');
              WriteLn('Version:      0.5');
              WriteLn('Copyright:    (C) 2001 - 2004 by Millennium Group');
              Halt(1);
            end
          else if (name = '-guid') then
            begin
//              StoreGUID := True;
            end
          else if (name = '-ns') then
            begin
//              NameSpaces := True;
              params.add(name);
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
  if params.IndexOf('-poa') = -1 then
    params.Add('-poa');
  if not fname_defined then
    begin
      writeln('missing file name');
      print_usage;
    end;
  defined.values['__SOMIDL__'] := 'TRUE';
  yyoutput := TMemoryStream.Create;
  try
    //yyinput := TFileStream.Create(filename,fmOpenRead);
    yyinput := OpenFile(filename);
        try
          yyprintln('#line 1 "'+filename+'"');
          yyinput.position := 0;
          cpp.yylex;
        finally
          yyinput.free;
          yyinput := nil;
        end;
        yyoutput.Position := 0;
        while yyoutput.Position < yyoutput.Size do
        begin
          yyoutput.Read(ch,1);
          write(ch);
        end;
  finally
    yyoutput.free;
    yyinput.free;
  end;
end.
