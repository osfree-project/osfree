{&G3+}
{$ifndef fpc}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-}
{$M 262144}
{$else}
{$MinStackSize 262144}
{$Align 1}
{$Optimization STACKFRAME}
{$endif}
uses os2base, MiscUtil, SysLib, CmdLine, Collect,
     StrOp, Strings, Dos, lxlite_Global,
     os2def, VpUtils
{$ifdef fpc}
      , Crt
{$else}
      , MyCrt
{$endif};

const
 Recurse : boolean = FALSE;
 Pause   : boolean = FALSE;
 Verbose : boolean = FALSE;
var
 OldExit : Procedure;
 fNames  : pStringCollection;
 allDone : boolean;

{$IfDef OS2}
{$Cdecl+,Orgname+}
function DosReplaceModule(OldModName,NewModName,BackModName: PChar): ApiRet;
  external 'DOSCALLS' index 417;
{$Cdecl-,Orgname-}
{$EndIf OS2}

Procedure Stop(eCode : Byte);
begin
 case eCode of
  1,2 : begin
         if eCode = 2
          then begin
                TextAttr := $0C;
                Writeln('Ã Invalid switch - see help below for details');
               end;
         TextAttr := $07;
         Writeln('Ã Usage for executable unlock: unLock [FileMask( FileMask)] {[?|-]Options}');
         Writeln('Ã /P{+|-} Enable (+) or disable (-) pause before each file');
         Writeln('Ã /R{+|-} [R]ecursive (+) file search through subdirectories');
         Writeln('Ã /V{+|-} Verbose (show additional information)');
         Writeln('Ã /?,/H   Show this help screen');
         Writeln('Ã´Default: /P- /R- /V-');
         TextAttr := $08;
         Writeln('Ã´Example: unLock d:\*.exe d:\*.dll /r');
         TextAttr := $07;
         Writeln('|');
         Writeln('Ã Usage for IBM UDF unlock: unLock drive:');
         TextAttr := $08;
         Writeln('À´Example: unLock S:');
        end;
 end;
 Halt(eCode);
end;

Function ParmHandler(var S : string) : Byte;
var
 I : Longint;

Function Enabled : boolean;
begin
 Enabled := TRUE;
 if length(S) = 1
  then exit
  else
 if (S[2] in ['+','-'])
  then ParmHandler := 2
  else
 if (S[2] in [' ','/'])
  then exit
  else Stop(2);
 if S[2] = '-' then Enabled := FALSE;
end;

begin
 ParmHandler := 1;
 case upCase(S[1]) of
  '?',
  'H' : Stop(1);
  'P' : Pause := Enabled;
  'R' : Recurse := Enabled;
  'V' : Verbose := Enabled;
  else Stop(2);
 end;
end;

Function NameHandler(var S : string) : Byte;
var
 fN : string;
begin
 NameHandler := ParseName(S, 1, fN);
 if fN <> '' then fNames^.AtInsert(fNames^.Count, NewStr(fN));
end;

Procedure MyExitProc;
begin
 Write(#13);
 TextAttr := $07; ClearToEOL;
 OldExit;
end;

Function Ask(const Q,A : string) : byte;
var ch  : char;
begin
 TextAttr := $02;
 Write('À ', Q, ' ');
 repeat
  ch := upCase(ReadKey);
  if First(ch, A) <> 0
   then begin
         Ask := First(ch, A);
         break;
        end;
 until FALSE;
 Writeln(Ch,#13'Ã');
end;

Procedure ProcessFile(fName : string);
var
 F  : File;
 _n : string;
 sz : array[0..255] of Char absolute _n;

Procedure NotLocked;
begin
 if Verbose
  then begin Write(' not locked'); textAttr := $0B; Writeln(#13'Ã'); end
  else begin Write(#13); ClearToEOL; end;
end;

begin
 if length(fName) >= 255 then exit;
 _n := extractName(fName);
 textAttr := $0B;
 Write(#13); ClearToEOL;
 Write('À Processing file ', Copy(_n, 1, 28));
 FileMode := open_share_DenyReadWrite or open_access_ReadOnly;
 Assign(F, fName); Reset(F, 1);
 if ioResult = 0
  then begin
        Close(F); NotLocked;
        Exit;
       end;
 case DosReplaceModule(strPCopy(sz, fName), nil, nil) of
  0 : begin
       textAttr := $0A; Write(' unlocked');
       textAttr := $0B; Writeln(#13'Ã');
      end;
  2 : NotLocked;
 else begin
       textAttr := $0C; Write(' sharing violation');
       textAttr := $0B; Writeln(#13'Ã');
      end
 end;
end;

Procedure ProcessFiles(const fN : string; Level : Longint);
var
 sr : SearchRec;
 nf : Longint;
 _d : DirStr;
 _n : NameStr;
begin
 _d := extractDir(fN);
 _n := extractName(fN);
 FindFirst(fN, Archive or Hidden or SysFile, sr);
 nf := 0;
 if (DosError <> 0) and (Level = 0) and (not Recurse)
  then begin
        textAttr := $0C;
        Writeln('Ã Cannot find such files: ', fN);
       end
  else
 While (DosError = 0) and (not allDone) do
  begin
   Inc(nf);
   if (length(_d) + length(sr.Name) <= 255)
    then begin
          if Pause
           then case Ask('File ' + sr.Name + ': [P]rocess, [S]kip or [A]bort?', 'PSA') of
                 2 : sr.Name := '';
                 3 : begin allDone := TRUE; break; end;
                end;
          if (sr.Name <> '') then ProcessFile(_d + sr.Name);
         end;
   FindNext(sr);
  end;
 FindClose(sr);
 if allDone or not Recurse then Exit;
 if nf = 0
  then begin
        textAttr := $0B; Write('À ', Short(_d, 77));
        ClearToEOL; Write(#13);
       end;
 FindFirst(_d + '*', Archive or Hidden or SysFile or Directory, sr);
 While (dosError = 0) and (not allDone) do
  begin
   if (sr.Attr and Directory <> 0) and (sr.Name[1] <> '.') and
      (length(_d) + length(sr.Name) + length(_n) + 1 <= 255)
    then ProcessFiles(_d + sr.Name + '\' + _n, succ(Level));
   FindNext(sr);
  end;
 FindClose(sr);
end;

// write a string to StdErr
procedure WriteMessage(const Msg:String);
  begin
    DosPutMessage(SysFileStdErr,Length(Msg),@Msg[1]);
  end;

// retrieve, format and write a SYSxxxx message
procedure DispayMessage(const message_number:word;const fillin1:PChar);
  var
    cTable              :Word;
    Buf                 :string[80];
    cbMessage           :Word;

  begin
    if Assigned(fillin1) then cTable:=1 else cTable:=0;
    FillChar(Buf,SizeOf(Buf),0);
    DosGetMessage(@fillin1,cTable,@Buf[1],SizeOf(Buf)-1,message_number,'OSO001.MSG',cbMessage);
    SetLength(Buf,cbMessage);
    WriteMessage(Buf);
  end;

// emulate IBM UNLOCK.EXE, from the UDF package
// differences:
//   - object code size :)
//   - passes useful return code
function UnlockMedia(const drive:string):ApiRet;
  var
    drive_filename      :array[0..3] of char;
    drive_handle        :Word;
    rc                  :ApiRet;
    action              :Word;

    parameter           :
      packed record
        Length_Command_Information:byte;
        Drive_Unit      :byte;
      end;

    parameter_length    :Word;


  begin
    drive_handle:=High(drive_handle);

    if (Length(drive)<>2)
    or (not (UpCase(drive[1]) in ['A'..'Z']))
    or (drive[2]<>':')
     then
       begin
         DispayMessage(msg_Bad_Syntax,nil); // 1003
         Result:=error_Invalid_Parameter; // IBM Unlock sets zero!
         Exit;
       end;

    Os2Base.DosError(ferr_DisableHardErr+ferr_DisableException); // not restored..

    StrPCopy(drive_filename,drive);
    drive_filename[0]:=UpCase(drive_filename[0]);

    rc:=DosOpen(
          drive_filename,
          drive_handle,
          action,
          0,
          0,
          open_action_Open_If_Exists+open_action_Fail_If_New,
          open_flags_Dasd+open_flags_Fail_On_Error+open_flags_No_Locality+open_share_DenyNone+open_access_ReadOnly,
          nil);

    // unlock ignores rc??
    with parameter do
      begin
        Length_Command_Information:=0; // Unlock
        Drive_Unit:=Ord(drive_filename[0])-Ord('A');
      end;
    parameter_length:=SizeOf(parameter);

    if rc=no_Error then
      rc:=DosDevIOCtl(
            drive_handle,ioctl_Disk,dsk_UnlockEjectMedia,
            @parameter,parameter_length,@parameter_length,
            nil       ,0               ,nil);

    case rc of
      no_Error:                 //   0
        ;
      error_Access_Denied,      //   5
      error_Invalid_Drive,      //  15
      error_Not_Ready,          //  21
      error_Gen_Failure,        //  31
      error_Drive_Locked:       // 108
        DispayMessage(rc,@drive_filename);

    else
      WriteMessage('Error On Lock rc='+Int2Str(rc)+' ');
      DispayMessage(error_Bad_Command,@drive_filename); // command not recognized by device
    end;

    Result:=rc;

    if drive_handle<>High(drive_handle) then
      DosClose(drive_handle);

  end;

var
 I : Longint;

begin

 if  (ParamCount=1)
 and (Length(ParamStr(1))=Length('X:'))
 and (UpCase(ParamStr(1)[1]) in ['A'..'Z'])
 and (ParamStr(1)[2]=':') then
   begin
     Halt(UnlockMedia(ParamStr(1)));
   end;

 TextAttr := $0F;
 Writeln('Ú[ unLock ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ[ Version '+Version+' ]¿');
 Writeln('Ã Copyright 1996 by FRIENDS software Ä No rights reserved Ù');
 TextAttr := $07;
 @OldExit := ExitProc; ExitProc := @MyExitProc;
 New(fNames, Create(8, 8));
 ParseCommandLine(#0, ParmHandler, NameHandler);
 if (fNames^.Count = 0) then Stop(1);

 For I := 0 to pred(fNames^.Count) do
  begin
   ProcessFiles(pString(fNames^.At(I))^, 0);
   if allDone then break;
  end;

 TextAttr := $01; ClearToEOL;
 Writeln('À´Done');
end.
