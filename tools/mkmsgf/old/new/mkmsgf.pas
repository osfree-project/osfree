{

 Make Message File Utility (MKMSGF) Clone (C) 2002-2004 by Yuri Prokushev

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

}

{
This work based on public domain program e_msgf by Veit Kannegieser and my own
research.
}

program mkmsgf;

{$MODE ObjFPC}
{$H+}
{.$DEFINE DEBUG}

Uses
  SysUtils,
  Classes,
  XMLRead;

ResourceString
  msg_name='Make Message File Utility (MKMSGF)';
  msg_version='Version 0.1';
  msg_copyright='Copyright (C) 2002-2004 by Yuri Prokushev (prokushev@freemail.ru)'+LineEnding+
                'MKMSGF is a product of eCo Software (http://ecomstation.ru/ecosoft)';
  msg_usage='MKMSGF <infile>[<.msf>] <outfile>[<.msg>] [<options>]'+LineEnding+
            'or'+LineEnding+
            'MKMSGF @controlfile'+LineEnding+LineEnding+
            'Options are:'+LineEnding+
            '  -d, --dbcsrange     DBCS range or country'+LineEnding+
            '  -p, --codepage      Code page'+LineEnding+
            '  -l, --language      Language id,sub id'+LineEnding+
            '  -v, --verbose       Verbose mode'+LineEnding+
            '  -h, -H, -?, --help  Help'+LineEnding+LineEnding+
            'You can use "/" instead of "-".'+LineEnding;
  msg_nofile='MKMSGF: File %s not found';
  msg_vars='global variables';
  msg_strin='strIn';
  msg_strout='strOut';
  msg_not_msg_file='Invalid magic - not a MSG file?';
  msg_not_valid_version='Unknown version of indexed message file.';
  msg_incorrect_format=': Incorrect source message file format';
  msg_incorrect_identifier=': Incorrect message identifier: ';

Const
  MSGFileHeaderMagic:array [1..8] of char =(#255,'M','K','M','S','G','F',#0);

Type
  IDString=Array[1..3] of Char;

Type
  TIndexTable=Array[0..0] of word;

Type
  {Header of message file}
  TMsgFileHeader = packed record
    Magic               : Array[1..8] of Char; // Magic word
    Identifier          : Array[1..3] of Char; // Identifier (SYS, DOS, NET, etc.)
    MessagesNumber      : Word;                // Number of messages
    FirstMessageNumber  : Word;                // Number of the first message
    Offsets16bit        : Boolean;             // Index table is 16-bit offsets for small file (<64k)
    Version             : Word;                // File version 2 - New Version 0 - Old Version
    IndexTableOffset    : Word;                // Offset of index table
    CountryInfo         : Word;                // Offset of country info block
    NextCountryInfo     : DWord;               //
    Reserved2           : Array[1..5] of byte; // Must be 0 (zero)
  end;

  TMultipageBlock=packed record
    BlockSize         : Word;
    BlocksCount       : Word;
  end;

  {Country Info block of message file}
  TMSGFileCountryInfo = packed record
    BytesPerChar      : Byte;                  // Bytes per char (1 - SBCS, 2 - DBCS)
    Reserved          : Array[0..1] of byte;   // Not known
    LanguageFamilyID  : Word;                  // Language family ID (As in CPI Reference)
    LanguageVersionID : Word;                  // Language version ID (As in CPI Reference)
    CodePagesNumber   : Word;                  // Number of country pages
    CodePages         : Array[1..16] of Word;  // Codepages list (Max 16)
    Filename          : Array[0..260] of Char; // Name of file
  end;

  TMSGFile=class
  private
    FMessages: TStringList;
    FDBCS: Boolean;
    FLanguage: Word;
    FSubLanguage: Word;
    FCodePages: Array[1..16] of Word;
    FIdentifier: Array[1..3] of Char;
  protected
    function GetMsg(Index: Integer): AnsiString; virtual;
    procedure PutMsg(Index: Integer; const S: AnsiString); virtual;
    function GetMessagesCount: Longint; virtual;

  public
    constructor Create;
    constructor Create(Stream: TStream);
    constructor Create(FileName: String);
    destructor Destroy; override;

    procedure Store(Stream: TStream);
    procedure Store(FileName: String);

    property Messages[Index: Integer]: AnsiString read GetMsg write PutMsg;
    property Identifier: IDString read FIdentifier write FIdentifier;
    property Count: Longint read GetMessagesCount;
    property DBCS: Boolean read FDBCS write FDBCS;
  end;

  EMSGFileError = class(Exception);

constructor TMSGFile.Create;
begin
  inherited Create;

  FMessages:=TStringList.Create;
  FMessages.Sorted:=False;
end;

constructor TMSGFile.Create(Stream: TStream);
Var
  Header: TMSGFileHeader;
  CountryInfo: TMSGFileCountryInfo;
  PIndexTable: ^TIndexTable;
  IndexTableSize: DWord;
  Message: AnsiString;
  I: Word;
  Len: Longint;
begin
  Self.Create;

  Stream.Read(Header, SizeOf(Header));

  If Header.Magic<>MSGFileHeaderMagic then raise EMSGFileError.Create(msg_not_msg_file);
  If not (Header.Version in [0, 2]) then raise EMSGFileError.Create(msg_not_valid_version);

  FIdentifier:=Header.Identifier;

  If Header.Version=0 then Header.IndexTableOffset:=SizeOf(Header);

  Stream.Position:=Header.IndexTableOffset;

  IndexTableSize:=Header.MessagesNumber*2*(1+Byte(not Header.Offsets16bit));

  GetMem(PIndexTable, IndexTableSize);
  Stream.Read(PIndexTable^, IndexTableSize);

  For I:=1 to Header.FirstMessageNumber do
  begin
    FMessages.BeginUpdate;
    FMessages.Add('?'#13#10);
    FMessages.EndUpdate
  end;
  For I:=1 to Header.MessagesNumber do
  begin
    If Header.Offsets16bit then
    begin
      Stream.Position:=Word(PIndexTable^[Pred(I)]);
      If I=Header.MessagesNumber then
        If Header.NextCountryInfo=0 then
          Len:=Stream.Size-Word(PIndexTable^[Pred(I)])
        else
          Len:=Header.NextCountryInfo-Word(PIndexTable^[Pred(I)])
      else
        Len:=PIndexTable^[I]-PIndexTable^[Pred(I)];
    end else begin
      Stream.Position:=DWord(PIndexTable^[Pred(I)]);
      If I=Header.MessagesNumber then
        If Header.NextCountryInfo=0 then
          Len:=Stream.Size-DWord(PIndexTable^[Pred(I)])
        else
          Len:=Header.NextCountryInfo-DWord(PIndexTable^[Pred(I)])
      else
        Len:=PIndexTable^[I]-PIndexTable^[Pred(I)];
    end;

    SetLength(Message, Len);
    Stream.Read(Message[1], Len);

    If Header.Version=0 then Message:='P'+Message;

    If Message[Len]<>#10 then Message:=Message+'%0'#13#10;
    FMessages.BeginUpdate;
    FMessages.Add(Message);
    FMessages.EndUpdate;
  End;

  FreeMem(PIndexTable, IndexTableSize);

  If Header.Version>0 then
  begin
    Stream.Position:=Header.CountryInfo;
    Stream.Read(CountryInfo, SizeOf(CountryInfo));

    With CountryInfo do
    begin
      FDBCS:=(BytesPerChar=2);
      FLanguage:=LanguageFamilyID;
      FSubLanguage:=LanguageVersionID;
      For I:=1 to CodePagesNumber do FCodePages[I]:=CodePages[I];
    end;

  end;

(*

        If Header.NextCountryInfo<>0 then begin
        Seek(MsgFile, Header.NextCountryInfo);
        BlockRead(MsgFile, MultiBlock, SizeOf(MultiBlock));


        for j:=1 to MultiBlock.BlocksCount do
        begin
          BlockRead(MsgFile, Block, SizeOf(Block));
          WriteLn(SourceFile, '; Linkedfile: ', Block.Filename);
        end;
      end;
    end; *)

end;

constructor TMSGFile.Create(FileName: String);
var
  S: TStream;
begin
  S:=TFileStream.Create(Filename, fmOpenRead);
  try
    Self.Create(S);
  finally
    S.Free;
  end;
end;

destructor TMSGFile.Destroy;
begin
  FMessages.Destroy;
end;

function TMSGFile.GetMsg(Index: Integer): AnsiString;
begin
  Result:=FMessages.Strings[Index];
end;

procedure TMSGFile.PutMsg(Index: Integer; const S: AnsiString);
begin
  FMessages.Strings[Index]:=S;
end;

Function TMSGFile.GetMessagesCount: Longint;
Begin
  Result:=FMessages.Count;
End;

Procedure TMSGFile.Store(Stream: TStream);
Begin
End;

Procedure TMSGFile.Store(FileName: String);
var
  S: TStream;
begin
  S:=TFileStream.Create(Filename, fmOpenWrite);
  try
    Self.Create(S);
  finally
    S.Free;
  end;
End;

Var
  StrIn     : String;
  StrOut    : String;
  StrIncDir : String;
  CodePages : Word; //= 866
  CP_type   : Boolean; // SBCS


Procedure Usage;
Begin
  WriteLn(msg_name);
  WriteLn(msg_version);
  WriteLn(msg_copyright);
  WriteLn;
  WriteLn(msg_usage);
End;

Procedure ParseSwitch(var S: String);
{
-d, --dbcsrange
-p, --codepage
-l, --language
-v, --verbose
-h, -H, -?, --help
}
Var
  DOSSwitch: Boolean;
Begin
  If (S[1]='/') or ((S[1]='-') and (S[2]<>'-')) then DOSSwitch:=True else DOSSwitch:=False;
  Delete(S, 1, 1);
  If S[1]='-' then Delete(S, 1, 1);
  If DOSSwitch then
  begin
    Case UpCase(S[1]) of
      'D': begin
      end;
      'P': begin
      end;
      'L': begin
      end;
      'V': begin
      end;
      '?', 'H': begin
      end;
    else
      WriteLn('Unsupported switch');
    end;
  end;
End;

Procedure ParseCommandLine(CmdLine: String);
Var
  S: String;
  I: Byte;
Begin
  {$ifdef debug}
  WriteLn(CmdLine);
  {$endif}
  StrIn:='';
  StrOut:='';
  S:=CmdLine;
  If S<>'' then
  begin
    // Remove leading spaces
    While S[1]=' ' do Delete(S, 1, 1);

    While S<>'' do
    begin
      {$ifdef debug}
      WriteLn(S);
      {$endif}

      If (S[1]='-') or (S[1]='/') then
        ParseSwitch(S)
      else
      begin
        If StrIn='' then
        begin
          StrIn:=Copy(S, 1, Pos(' ', S)-1);
          Delete(S, 1, Pos(' ', S)-1);
          // Remove spaces
          If S<>'' then While S[1]=' ' do Delete(S, 1, 1);
          S:=' '+S;

          {$ifdef debug}
          WriteLn(S);
          {$endif}

      end else if StrOut='' then
        begin
          StrOut:=Copy(S, 1, Pos(' ', S)-1);
          Delete(S, 1, Pos(' ', S)-1);
          // Remove spaces
          If S<>'' then While S[1]=' ' do Delete(S, 1, 1);
          S:=' '+S;

          {$ifdef debug}
          WriteLn(S);
          {$endif}

        end;
      end;
      If S<>'' then Delete(S, 1, 1);
    end;
  end;
End;

Type
  PMsgList=^TMsgList;
  TMsgList=record
    Message: PChar;
    Next: PMsgList;
  end;

Procedure Compile;
Type
  DWordArray=Array[0..0] of Cardinal;
Var
  Header: TMSGFileHeader;
  CountryInfo: TMSGFileCountryInfo;
  FIn: Text;
  FOut: File;
  S: String;
  Number: Word;
  Line: Longint;
  Res: Word;
  Message: String;
  Messages: PMsgList;
  MsgPos: PMsgList;
  MsgCount: Word;
  Idx: ^DWordArray;
Begin
  {$ifdef debug}
  WriteLn(StrIn);
  WriteLn(StrOut);
  {$endif}

  Assign(FIn, StrIn);
  Reset(FIn);

  Messages:=nil;
  MsgPos:=Messages;
  Line:=0;

  // Set magic signature
  Header.Magic:=MSGFileHeaderMagic;
  // We support only 32-bit index table
  Header.Offsets16bit:=False;
  // We support only version 2 msgfiles
  Header.Version:=2;
  // CountryInfo
  Header.IndexTableOffset:=SizeOf(Header);
  Header.NextCountryInfo:=0;
  Header.Reserved2[1]:=0;
  Header.Reserved2[2]:=0;
  Header.Reserved2[3]:=0;
  Header.Reserved2[4]:=0;
  Header.Reserved2[5]:=0;

  // Skip comments
  repeat
    Readln(Fin, S);
    Inc(Line);
    If Trim(S)='' then S:=';'; // Heh... Can be just empty line. Skip too.
  until S[1]<>';';

  MsgCount:=0;

  // Is it really ID?
  If Length(S)>3 then WriteLn(Line, msg_incorrect_format);

  // Set ID
  Header.Identifier:=S;

  S:='Let''s play the Yuri''s game';
  // Read messages
  While not EOF(Fin) do
  begin
    If S='Let''s play the Yuri''s game' then
    begin
      ReadLn(FIn, S);
      Inc(Line);
    end;
    If Length(S)<>0 then
    begin
      // Is it not comment?
      If S[1]<>';' then
      begin
        // Check for correct message header (ex: SYS0000)
        If length(S)>7 then
        begin
          If copy(S ,1 ,3)=Header.Identifier then
          begin
            Number:=StrToInt(Copy(S, 4, 4));
            // Number of the first message
            if MsgPos=nil then Header.FirstMessageNumber:=Number;
            Delete(S, 1, 3+4);
            Delete(S, 2, 2);
            Message:=S+LineEnding;

            // Read message
            If not EOF(Fin) then
              Repeat
                ReadLn(FIn, S);
                Inc(Line);
                if Length(S)>7 then
                  if copy(S, 1, 3)=Header.Identifier then break;
                If Length(S)<>0 then
                begin
                  If S[1]<>';' then
                    Message:=Message+S+LineEnding;
                end else begin
                  Message:=Message+LineEnding;
                end;
                if EOF(Fin) then break;
              Until False;
            // Store message
            If Pos('%0', Message)>0 then Delete(Message, Length(Message)-1, 2);
            If MsgPos=nil then
            begin
              GetMem(Messages, SizeOf(TMsgList));
              MsgPos:=Messages;
              MsgPos^.Next:=nil;
            end else begin
              GetMem(MsgPos^.Next, SizeOf(TMsgList));
              MsgPos^.Next^.Next:=nil;
              MsgPos:=MsgPos^.Next;
            end;
            Inc(MsgCount);
            Message:=Message+#0;
            MsgPos^.Message:=StrNew(@Message[1]);

          end else begin
            WriteLn(Line, msg_incorrect_identifier, s);
          end;
        end else begin
          WriteLn(Line, ': Message header not found: ', s);
        end;
      end;
    end else begin
      WriteLn(Line, ': Unexpected empty line');
    end;
  end;

  Close(FIn);

  // Number of messages
  Header.MessagesNumber:=MsgCount;
  Header.CountryInfo:=SizeOf(Header)+MsgCount*SizeOf(Cardinal);

  With Countryinfo do
  begin
    BytesPerChar:=1;                 // Bytes per char (1 - SBCS, 2 - DBCS)
    Reserved[0]:=0;
    Reserved[1]:=0;   // Not known
    LanguageFamilyID:=7; //  : Word;                  // Language family ID (As in CPI Reference)
    LanguageVersionID:=1;// : Word;                  // Language version ID (As in CPI Reference)
    CodePagesNumber:=1;                  // Number of country pages
    CodePages[1]:=866;  // Codepages list (Max 16)
    Filename:='oso001h.msg'#0; // Name of file
  end;

  // Write MSG-file
  Assign(Fout, StrOut);
  Rewrite(Fout, 1);
  BlockWrite(Fout, Header, SizeOf(Header), Res);

  GetMem(Idx, MsgCount*SizeOf(Cardinal));
  {$R-}
  Idx^[0]:=SizeOf(Header)+SizeOf(CountryInfo)+MsgCount*SizeOf(Cardinal);
  Number:=1;
  msgpos:=messages;
  While msgpos<>nil do
  begin
    Idx^[Number]:=Idx^[Number-1]+StrLen(MsgPos^.Message);
    MsgPos:=MsgPos^.Next;
    Inc(Number);
  end;
  {$R+}
  BlockWrite(Fout, Idx^, MsgCount*SizeOf(Cardinal), Res);
  FreeMem(Idx, MsgCount*SizeOf(Cardinal));

  BlockWrite(Fout, CountryInfo, SizeOf(CountryInfo), Res);

  {$ifdef debug}
  WriteLn(SizeOf(TMsgFileHeader), Res);
  {$endif}
  msgpos:=messages;
  While msgpos<>nil do
  begin
    BlockWrite(Fout, MsgPos^.Message^, StrLen(MsgPos^.Message), Res);
    MsgPos:=MsgPos^.Next;
  end;
  {$ifdef debug}
  WriteLn(Res);
  {$endif}
  Close(Fout);
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
    CommandLine:=CommandLine+#0;
    CmdLine:=StrNew(@CommandLine[1]);
    ParseCommandLine(CmdLine);
    Compile;
  end;
  Close(F);
End;

{$ifdef VER1_0}
Var
  I: Byte;
  CmdLine: String;
{$endif}
begin
  {$ifdef VER1_0}
  For I:=1 to ParamCount do
    If Pos(' ', ParamStr(I))>0 then
      CmdLine:=CmdLine+' "'+ParamStr(I)+'"'
    else
      CmdLine:=CmdLine+' '+ParamStr(I);
  {$endif}
  If CmdLine='' then
  begin
    Usage;
    Exit;
  end;
  If Copy(ParamStr(1), 1, 1)='@' then
    ParseControlFile(ParamStr(1))
  else begin
    ParseCommandLine(CmdLine);
    Compile;
  end;
end.

{
  $Log: mkmsgf.pas,v $
  Revision 1.2  2002/10/20 14:15:01  prokushev
  Cleaning
}
