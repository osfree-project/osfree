{

   @file helpmsg.pas

   @brief helpmsg command - show help for message or command

   (c) osFree Project 2003, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <prokushev@freemail.ru>
}

Program HelpMsg;

Uses
  DosCalls,
  Dos,
  SysUtils;

Var
  MsgFile: Array[0..$1fff] of Char;
  Buf: Array[0..$efff] of Char;
  MsgSize: Cardinal;
  RC: Cardinal;
  MsgNum: Cardinal;
  S: String;
  ID: String;
Begin
  // Default message
  MsgNum:=1491;
  MsgFile:='OSO001.MSG'#0;

  // Is it book or message?
  If ParamCount>0 then
  begin
    S:=ParamStr(1);
    // No, just default help message
    if S<>'/?' then
    begin
      // Yes, it is book
      If ((Length(S)<4) and (ParamCount=1)) or (ParamCount>1) then
      begin
        If ParamCount=1 then S:='CMDREF';
        Exec(GetEnv('COMSPEC'), '/C START /F VIEW '+S+' '+ParamStr(2));
        Exit;
      end;
    end;
    ID:=UpperCase(Copy(S, 1, 3));
    MsgNum:=StrToInt(Copy(S, 4, Length(S)-3));

    If ID='SYS' then
      MsgFile:='OSO001.MSG'#0
    else
      MsgFile:=ID+'.MSG'#0;

    RC:=DosGetMessage(nil, 0, Buf, SizeOf(Buf), MsgNum, MsgFile, msgsize);
    If RC<>0 then Exit;

    Buf[MsgSize]:=#0;
    WriteLn;
    WriteLn(Buf);
    exit;
  end;

  RC:=DosGetMessage(nil, 0, Buf, SizeOf(Buf), MsgNum, MsgFile, msgsize);
  Buf[MsgSize+1]:=#0;
  WriteLn(Buf);
End.

