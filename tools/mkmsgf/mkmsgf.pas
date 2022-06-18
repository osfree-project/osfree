{

 Make Message File Utility (MKMSGF) Clone (C) 2002-2008 by Yuri Prokushev

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

{.$MODE ObjFPC}
{$H+}
{$DEFINE xDEBUG}

Uses
  SysUtils(*,
  Classes,
  XMLRead*);

const
  LineEnding=#10#13;
const
  msg_name: String ='Make Message File Utility (MKMSGF)';
  msg_version: String ='Version 0.1';
  msg_copyright: String ='Copyright (C) 2002-2008 osFree.org';
  msg_usage1: String ='MKMSGF <infile>[<.msf>] <outfile>[<.msg>] [<options>]'+LineEnding+
            'or'+LineEnding+
            'MKMSGF @controlfile'+LineEnding+LineEnding+
            'Options are:'+LineEnding+
            '  -d, --dbcsrange     DBCS range or country'+LineEnding+
            '  -p, --codepage      Code page'+LineEnding;
  msg_usage2: String ='  -l, --language      Language id,sub id'+LineEnding+
            '  -v, --verbose       Verbose mode'+LineEnding+
            '  -h, -H, -?, --help  Help'+LineEnding+LineEnding+
            'You can use "/" instead of "-".'+LineEnding;
  msg_nofile: String ='MKMSGF: File %s not found';
//  msg_vars: String ='global variables';
//  msg_strin: String ='strIn';
//  msg_strout: String ='strOut';
//  msg_not_msg_file: String ='Invalid magic - not a MSG file?';
//  msg_not_valid_version: String ='Unknown version of indexed message file.';
  msg_incorrect_format: String =': Incorrect source message file format';
  msg_incorrect_identifier: String =': Incorrect message identifier: ';
  msg_incorrect_switch: String ='Unsupported switch';
  msg_no_header: String =': Message header not found: ';
  msg_empty_line: String =': Unexpected empty line';

Const
  MSGFileHeaderMagic:array [1..8] of char =(#255,'M','K','M','S','G','F',#0);

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
    NextCountryInfo     : LongWord;            //
    Reserved2           : Array[1..5] of byte; // Must be 0 (zero)
  end;

(*
typedef struct file_head {              // message file header
        char      signature_h[8];       // signature mark
        char      component_id[len_COMP_ID];// component id
        unsigned  msg_count;            // number of messages
        unsigned  base_mid;             // base message number
        char      offset_type;          // double(0) or single word (1)
        unsigned  version;
        unsigned  header_length;
        char      cp_type;              // SBCS code page (0) or DBCS (1)
        int       code_page;            // code page number
        char      reserved[8];
        } head_t;
*)

(*

; **********************************************************************
;  Message file extended message file definition record
;  ----------------------------------------------------
EXTREC  STRUC                                   ;MSG FILE EXTENDED HEADER
        REC_CP_TYPE     DB      ?               ;1 = SBCS, 2 = DBCS
        REC_CNTY_ID     DW      ?               ;COUNTRY ID
        REC_LANG_ID     DW      ?               ;LANGUAGE FAMILY ID
        REC_LANG_VERID  DW      ?               ;LANGUAGE VERSION ID
        REC_CNT_CP      DW      ?               ;CODE PAGE COUNT
        REC_CP_ID       DW      16 dup (0)      ;CODE PAGE IDs
        REC_FILENAME    DB      ASCIIZ_LEN dup(?),0   ;FILE NAME
EXTREC  ENDS

*)

  TMultipageBlock=packed record
    BlockSize         : Word;
    BlocksCount       : Word;
  end;

  {Country Info block of message file}
  TMSGFileCountryInfo = packed record
    BytesPerChar      : Byte;                  // Bytes per char (1 - SBCS, 2 - DBCS)
    CountryID         : Word;                  // Country ID
    LanguageFamilyID  : Word;                  // Language family ID (As in CPI Reference)
    LanguageVersionID : Word;                  // Language version ID (As in CPI Reference)
    CodePagesNumber   : Word;                  // Number of country pages
    CodePages         : Array[1..16] of Word;  // Codepages list (Max 16)
    Filename          : Array[0..260] of Char; // Name of file
  end;

Var
  StrIn     : String;
  StrOut    : String;
//  StrIncDir : String;
//  CodePages : Word; //= 866
//  CP_type   : Boolean; // SBCS
  CmdLine: String;


Procedure Copyright;
Begin
  WriteLn(msg_name);
  WriteLn(msg_version);
  WriteLn(msg_copyright);
  WriteLn;
End;

Type
  CountryInfo=record
    Code: Array[1..3] of Char;
    FamilyID: Byte;
    VersionID: Byte;
    Language: String;
    Country: String;
  end;

(*
        Language ID:
        Code    Family  Sub     Language                Principal country
        ----    ------  ---     --------                -----------------
        ARA     1       2       Arabic                  Arab Countries
        BGR     2       1       Bulgarian               Bulgaria
        CAT     3       1       Catalan                 Spain
        CHT     4       1       Traditional Chinese     R.O.C.
        CHS     4       2       Simplified Chinese      P.R.C.
        CSY     5       1       Czech                   Czechoslovakia
        DAN     6       1       Danish                  Denmark
        DEU     7       1       German                  Germany
        DES     7       2       Swiss German            Switzerland
        EEL     8       1       Greek                   Greece
        ENU     9       1       US English              United States
        ENG     9       2       UK English              United Kingdom
        ESP     10      1       Castilian Spanish       Spain
        ESM     10      2       Mexican Spanish         Mexico
        FIN     11      1       Finnish                 Finland
        FRA     12      1       French                  France
        FRB     12      2       Belgian French          Belgium
        FRC     12      3       Canadian French         Canada
        FRS     12      4       Swiss French            Switzerland
        HEB     13      1       Hebrew                  Israel
        HUN     14      1       Hungarian               Hungary
        ISL     15      1       Icelandic               Iceland
        ITA     16      1       Italian                 Italy
        ITS     16      2       Swiss Italian           Switzerland
        JPN     17      1       Japanese                Japan
        KOR     18      1       Korean                  Korea
        NLD     19      1       Dutch                   Netherlands
        NLB     19      2       Belgian Dutch           Belgium
        NOR     20      1       Norwegian - Bokmal      Norway
        NON     20      2       Norwegian - Nynorsk     Norway
        PLK     21      1       Polish                  Poland
        PTB     22      1       Brazilian Portugues     Brazil
        PTG     22      2       Portuguese              Portugal
        RMS     23      1       Rhaeto-Romanic          Switzerland
        ROM     24      1       Romanian                Romania
        RUS     25      1       Russian                 Russian Federation
        SHL     26      1       Croato-Serbian (Lat     Yugoslavia
        SHC     26      2       Serbo-Croatian (Cyr     Yugoslavia
        SKY     27      1       Slovakian               Czechoslovakia
        SQI     28      1       Albanian                Albania
        SVE     29      1       Swedish                 Sweden
        THA     30      1       Thai                    Thailand
        TRK     31      1       Turkish                 Turkey
        URD     32      1       Urdu                    Pakistan
        BAH     33      1       Bahasa                  Indonesia
        SLO     34      1       Slovene                 Slovenia
*)

Procedure Usage;
Begin
  Write(msg_usage1);
  WriteLn(msg_usage2);
End;

Procedure ParseSwitch(var S: String);
{
-d, --dbcsrange
-p, --codepage
-l, --language
-v, --verbose
-h, -H, -?, --help
-q, --quiet		// Added in MKMSGF by Michael K Greene
-i			// Undocumentet IBM switch found by Michael K Greene
-a			// Undocumentet IBM switch found by Michael K Greene
-c			// Undocumentet IBM switch found by Michael K Greene
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
      'Q': begin
      end;
      'I': begin
      end;
      'A': begin
      end;
      'C': begin
      end;
      '?', 'H': begin
      end;
    else
      WriteLn(msg_incorrect_switch);
    end;
  end;
End;

Procedure ParseCommandLine(CmdLine: String);
Var
  S: String;
//  I: Byte;
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

    // Get source name
    StrIn:=Copy(S, 1, Pos(' ', S)-1);
    {$ifdef debug}
    WriteLn('InputFile=', StrIn);
    {$endif}
    Delete(S, 1, Pos(' ', S)-1);

    // Remove spaces
    If S<>'' then While S[1]=' ' do Delete(S, 1, 1);

    StrOut:=Copy(S, 1, Pos(' ', S)-1);
    {$ifdef debug}
    WriteLn('OutputFile=', StrOut);
    {$endif}
    Delete(S, 1, Pos(' ', S)-1);

    // Remove spaces
    If S<>'' then While S[1]=' ' do Delete(S, 1, 1);

    While S<>'' do
    begin
      {$ifdef debug}
      WriteLn(S);
      {$endif}

      If (S[1]='-') or (S[1]='/') then ParseSwitch(S);

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
  DWordArray=Array[0..0] of LongWord;
Var
  Header: TMSGFileHeader;
  CountryInfo: TMSGFileCountryInfo;
  FIn: Text;
  FOut: File;
  S: String;
  Number: Word;
  Line: Longint;
  Res: LongWord;
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
  Header.Identifier[1]:=S[1];
  Header.Identifier[2]:=S[2];
  Header.Identifier[3]:=S[3];

  S:='Let''s play the Yuri''s game';
  WriteLn('Reading messages from ', StrIn);
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
          If (S[1]=Header.Identifier[1]) and
             (S[2]=Header.Identifier[2]) and
             (S[3]=Header.Identifier[3]) then
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
//                WriteLn('Line: ', Line);
                Inc(Line);
                if Length(S)>7 then
                  If (S[1]=Header.Identifier[1]) and
                     (S[2]=Header.Identifier[2]) and
                     (S[3]=Header.Identifier[3]) then break;
                If Length(S)<>0 then
                begin
                  If Pos(';', S)<>1 then begin
                    Message:=Message+S+LineEnding; end;
                end else begin
                  Message:=Message+LineEnding;
                end;
                if EOF(Fin) then break;
              Until False;
            // Store message
            If Pos('%0', Message)>0 then
            begin
              Delete(Message, Pos('%0', Message), 2); // Delete %0 sign
              Delete(Message, Length(Message)-Length(LineEnding)+1, Length(LineEnding)); // Delete LineEnding
            end;
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
          WriteLn(Line, msg_no_header, s);
        end;
      end;
    end else begin
      WriteLn(Line, msg_empty_line);
    end;
  end;

  Close(FIn);

  // Number of messages
  Header.MessagesNumber:=MsgCount;
  Header.CountryInfo:=SizeOf(Header)+MsgCount*SizeOf(Cardinal);

  With Countryinfo do
  begin
    BytesPerChar:=1;                 // Bytes per char (1 - SBCS, 2 - DBCS)
    CountryID:=0;     // : Word;
    LanguageFamilyID:=7; //  : Word;                  // Language family ID (As in CPI Reference)
    LanguageVersionID:=1;// : Word;                  // Language version ID (As in CPI Reference)
    CodePagesNumber:=1;                  // Number of country pages
    CodePages[1]:=866;  // Codepages list (Max 16)
//    Filename:='oso001h.msg'#0); // Name of file
  end;

  WriteLn('Writing MSG-file to ', StrOut);
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
    Idx^[Number]:=Idx^[Number-1]+Longword(StrLen(MsgPos^.Message));
    MsgPos:=MsgPos^.Next;
    Inc(Number);
{$ifdef DEBUG}
writeln(Number);
{$endif}
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
    StrDispose(MsgPos^.Message);
    MsgPos:=MsgPos^.Next;
  end;

  // Free memory
  msgpos:=messages;
  While msgpos<>nil do
  begin
    messages:=msgpos;
    MsgPos:=MsgPos^.Next;
    FreeMem(messages, SizeOf(TMsgList));
  end;

  {$ifdef debug}
  WriteLn(Res);
  {$endif}
  Close(Fout);
  WriteLn('MSG-file created');
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
    CmdLine:=CommandLine;
    ParseCommandLine(CmdLine);
    Compile;
  end;
  Close(F);
End;

Var
  I: Longint;
begin
  Copyright;
  For I:=1 to ParamCount do
    If Pos(' ', ParamStr(I))>0 then
      CmdLine:=CmdLine+' "'+ParamStr(I)+'"'
    else
      CmdLine:=CmdLine+' '+ParamStr(I);
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

