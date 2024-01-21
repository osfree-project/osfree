{
 
 Make Message File Utility (MKMSGF) (C) 2002 Yuri Prokushev
                   
 This program is free software; you can redistribute it and/or modify it  
 under the terms of the GNU General Public License (GPL) as published by  
 the Free Software Foundation; either version 2 of the License, or (at    
 your option) any later version. This program is distributed in the hope  
 that it will be useful, but WITHOUT ANY WARRANTY; without even the       
 implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR          
 PURPOSE.                                                                 
 See the GNU General Public License for more details. You should have     
 received a copy of the GNU General Public License along with this        
 program; if not, write to the Free Software Foundation, Inc., 59 Temple  
 Place - Suite 330, Boston, MA 02111-1307, USA.                           
  
  $Log: mkmsgf.pas,v $
  Revision 1.2  2002/10/20 14:15:01  prokushev
  Cleaning

  
}

{
This work based on public domain program e_msgf by Veit Kannegieser and my own
research.
}
program mkmsgf;

{$MODE ObjFPC}

{$DESCRIPTION 'OS/2 *.MSG files compiler'}  

Uses
  MsgTools,
  GetOpts,
  SysUtils;
  
ResourceString
  msg_name='Make Message File Utility (MKMSGF)';
  msg_version='Version 0.0.1';
  msg_copyright='Copyright (C) 2002 Yuri Prokushev';
  msg_usage='MKMSGF <infile>[<.ext>] <outfile>[<.ext>] [<options>]'+LineEnding+
            'or'+LineEnding+
            'MKMSGF @controlfile'+LineEnding++LineEnding+
            'Options are:'+LineEnding+
            '  -d, --dbcsrange     DBCS range or country'+LineEnding+ 
            '  -p, --codepage      Code page'+LineEnding+
            '  -l, --language      Language id,sub id'+LineEnding+
            '  -v, --verbose       Verbose mode'+LineEnding+
            '  -h, -H, -?, --help  Help'+LineEnding+LineEnding+
            'You can use "/" instead of "-".'+LineEnding;
  msg_nofile='File %s not found';

Procedure Usage;
Begin
  WriteLn(msg_name);
  WriteLn(msg_version);
  WriteLn(msg_copyright);
  WriteLn(msg_usage);
End;
  

Var
  MSGFile: TMSGFile;

Procedure ParseCommandLine;
const
  options: Array[1..6] of TOption=
  (
  (Name: 'help'; Has_arg: No_Argument; Flag: nil; Value: 'h'),
  (Name: 'codepage'; Has_arg: Required_Argument; Flag: nil; Value: 'c'),
  (Name: 'dbcsrange'; Has_arg: Required_Argument; Flag: nil; Value: 'd'),
  (Name: 'language'; Has_arg: Required_Argument; Flag: nil; Value: 'l'),
  (Name: 'verbose'; Has_arg: No_Argument; Flag: nil; Value: 'v'),
  (Name: ''; Has_arg: No_Argument; Flag: nil; Value: #0)
  );
var  
  C: Char;
  optionindex: longint;
Begin
  If ParamCount=0 then
  begin
    Usage;
    Halt(1);
  end;
  
  OptSpecifier:=['/','-']; // Add dos-like option specifier
  
  repeat
    c:=getlongopts('?hHvVp:P:d:D:l:L:', @options[1], optionindex);
    if c=#0 then c:=options[optionindex].value;
    case c of
      'l', 'L': writeln('l');
      'p', 'P': writeln('p');
      'd', 'D': writeln('d');
      'v', 'V': writeln('v');
      '?', 'h', 'H': 
        begin
          Usage;
          c:=EndOfOptions;
        end;
      else
        begin
          WriteLn('Unknown option: ', c);
          c:=EndOfOptions;
        end;
    end;
  until c=EndOfOptions;
End;

Procedure ParseControlFile(ControlFileName: String);
Var
  F: Text;
  CommandLine: String;
Begin
  Delete(ControlFileName, 1, 1);
  If not FileExists(ControlFileName) then
  begin
    WriteLn(Format(msg_nofile, [ControlFileName]));
    Halt(1);
  end;
  Assign(F, ControlFileName);
  Reset(F);
  While not EOF(F) do
  begin
    ReadLn(F, CommandLine);
    ParseCommandLine;
  end;
  Close(F);
End;

Procedure Compile;
Begin
End;

begin
  If Copy(ParamStr(1), 1, 1)='@' then
    ParseControlFile(ParamStr(1))
  else begin
    ParseCommandLine;
    Compile;
  end;
end.
