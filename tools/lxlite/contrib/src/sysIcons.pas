{****************************************************************************}
{                                                                            }
{                       System Icons & Pointers editor                       }
{               Written by Andrew Zabolotny of FRIENDS software              }
{    No copyrights, no legal stuff, no credits, no greetings, no nothing.    }
{                                                                            }
{****************************************************************************}
{ Comments:This was my first program for OS/2 and I cannot guarantee nothing }
{          about it except it will try to do something... hope succesfully.  }
{          I seen a similar proggy... and I hasn`t been satisfied because    }
{          a) It always used reference to file                               }
{          b) It does not allow to edit pointer (using IconEdit, of course)  }
{          c) It was not mine :-)                                            }
{          ... and I wrote this one. Hope you like it.                       }
{****************************************************************************}
{$ifndef fpc}
{$frame-,speed-,smartlink+}
{$else}
{$Optimization STACKFRAME}
{$endif}
{$R sysIcons.res}
uses Dos, Strings, os2def, os2base, os2pmapi, MiscUtil, StrOp;

const
{* Dialog control IDs converted from sysicons.h *}
    idMAINWINDOW             = 100;
    idICONLIST               = 101;
    idICONNAME               = 102;
    idBUTCHANGE              = 103;
    idBUTUNDO                = 104;
    idBUTDEFAULT             = 105;
    idBUTQUIT                = 106;
    idSTOREDIRECT            = 107;
    idSTOREINDIRECT          = 108;
    idBUTEDIT                = 109;
    idICONBORDER             = 110;
    idBUTLOADSET             = 111;
    idENDEDIT                = 1000;

const
    nStdNames    = 19;
    cStdNames    : array[1..nStdNames] of pChar =
    ('Mouse arrow pointer',
     'Text cursor pointer',
     'Wait pointer',
     'Sizing window pointer',
     'Moving window pointer',
     'Resizing \ pointer',
     'Resizing / pointer',
     'Resizing - pointer',
     'Resizing | pointer',
     'Application icon',
     'Information icon',
     'Question icon',
     'Error icon',
     'Warning icon',
     'Illegal action icon',
     'Default file icon',
     'Default folder icon',
     'Multiple file icon',
     'Default program icon');
    cStdNums     : array[1..nStdNames] of Byte =
    (sptr_Arrow,
     sptr_Text,
     sptr_Wait,
     sptr_Size,
     sptr_Move,
     sptr_Sizenwse,
     sptr_Sizenesw,
     sptr_Sizewe,
     sptr_Sizens,
     sptr_AppIcon,
     sptr_IconInformation,
     sptr_IconQuestion,
     sptr_IconError,
     sptr_IconWarning,
     sptr_Illegal,
     sptr_File,
     sptr_Folder,
     sptr_Multfile,
     sptr_Program);
    fnTempFile   = 'si!tmp.ptr';
    pmSysPtrID   : pChar = 'PM_SysPointer';
    msgStored    : pChar = 'Stored in INI';
    msgUnknSM    : pChar = 'Unknown storage method';
    msgDefault   : pChar = 'Using default pointer';
    msgUnknown   : pChar = 'Unused pointer ID: 000';
    setKeyword   : pChar = 'arrowtextwaitsizemovesizenwsesizenesw'+
                           'sizewesizensapplicationinformationquestion'+
                           'errorwarningillegaldeffiledeffoldermultfile'+
                           'defprogram';
type
    pINIpointer  = ^tINIpointer;
    tINIpointer  = record
                    iconType : Longint;
                    iconData : Longint;
                    restData : array[0..0] of Byte;
                   end;

var appHAB       : HAB;
    appHMQ       : HMQ;
    newPointer   : boolean;
    oldPointer   : pINIpointer;
    oldPointerS  : ULong;
    oldPointerI  : IconInfo;
    editThreadID : uLong;
    tempIconName : String;
    tempFile     : boolean;
    editError    : boolean;

{* Use for editing a different thread... `cause we cannot *}
{* block main message queue even when editing pointer     *}
Function editThread(Arg : uLong) : uLong; cDecl;
label
    locEx;
var pS  : String;
    F   : File;
    MQ  : HMQ;
begin
 editError := TRUE;
 MQ := WinCreateMsgQueue(appHAB, 0);
 pS := fSearch('iconedit.exe', '.;'+GetEnv('PATH'));
 if pS = ''
  then begin
        WinMessageBox(hWnd_Desktop, hWnd_Desktop,
                      'Cannot find ICONEDIT.EXE'#13'in one of PATH directories',
                      'Error', 0, MB_ICONEXCLAMATION+MB_MOVEABLE+MB_ENTER);
        Goto locEx;
       end;
 if oldPointerS = 0
  then begin
        WinMessageBox(hWnd_Desktop, hWnd_Desktop,
                      'Cannot edit default system pointer'#13+
                      'Please load first another pointer'#13+
                      'instead of default',
                      'Error', 0, MB_ICONEXCLAMATION+MB_MOVEABLE+MB_ENTER);
        Goto locEx;
       end;

 if oldPointer^.iconType = 1
  then begin
        tempFile := FALSE;
        tempIconName := StrPas(@oldPointer^.iconData);
       end
  else begin
        tempFile := TRUE;
        tempIconName := fExpand(fnTempFile); Assign(F, tempIconName);
        if oldPointerI.cbIconData = 0
         then Erase(F)
         else begin
               Rewrite(F, 1);
               if ioResult <> 0
                then begin
                      WinMessageBox(hWnd_Desktop, hWnd_Desktop,
                                    'Cannot write temporary file',
                                    'Error', 0, MB_ICONEXCLAMATION+MB_MOVEABLE+MB_ENTER);
                      Exit;
                     end;
               BlockWrite(F, oldPointerI.pIconData^, oldPointerI.cbIconData);
               Close(F);
              end;
        inOutRes := 0;{* Clear ioResult in the case if Erase or Close failed*}
       end;
 Exec(pS, tempIconName);
 editError := FALSE;
locEx:
 WinPostQueueMsg(appHMQ, WM_COMMAND, idENDEDIT, 0);
 WinDestroyMsgQueue(MQ);
 DosExit(EXIT_THREAD, 0);
end;

Function DlgWindowProc(Window : hWnd; Msg : ULong; mp1,mp2 : MParam) : MResult; cDecl;

{* Ask user to choose a file using standard file dialog box *}
Function ChooseFile(fMask,fTitle : pChar) : String;
var fD : FileDlg;
    rC : UShort;
    _D : DirStr;
    _N : NameStr;
    _E : ExtStr;
begin
 FillChar(fD, sizeOf(fD), 0);
 fD.cbSize := sizeOf(fD);
 fD.fl := FDS_CENTER or FDS_OPEN_DIALOG;
 fD.pszTitle := fTitle;
 strCopy(fD.szFullFile, fMask);
 rC := WinFileDlg(hWnd_Desktop, Window, fD);

 if (rC <> 0) and (fD.lReturn = DID_OK)
  then begin
        fSplit(strPas(fD.szFullFile), _D, _N, _E);
        if (_D <> '') and (_D[length(_D)] = '\') then Dec(byte(_D[0]));
        ChDir(_D); ChooseFile := strPas(fD.szFullFile);
       end
  else ChooseFile := '';
end;

{* Return selected pointer ID *}
Function GetCurrentPointer : ULong;
begin
 GetCurrentPointer := cStdNums[succ(UShort(WinSendDlgItemMsg(Window, idICONLIST, LM_QUERYSELECTION, 0, 0)))];
end;

{* Return storage method - 0 = store as ICON_FILE; 1 = store as ICON_DATA *}
Function GetStorageMethod : byte;
begin
 GetStorageMethod := byte(WinSendDlgItemMsg(Window, idSTOREDIRECT, BM_QUERYCHECK, 0, 0));
end;

{* Display pointer in idICONBORDER window *}
Procedure ShowPointer;
var ps        : HPS;
    icon      : hPointer;
    Selection : ULong;
    iconName  : pChar;
    Rect      : Rectl;
    iconP     : pINIpointer;
    iconS     : uLong;
    tS        : String;
begin
 Selection := GetCurrentPointer;
 Icon := WinQuerySysPointer(hWnd_Desktop, Selection, FALSE);
 tS := long2str(Selection) + #0;
 if (Icon <> 0)
  then begin
        if (PrfQueryProfileSize(HINI_USERPROFILE, pmSysPtrID, @tS[1], iconS))
         then begin
               GetMem(iconP, iconS);
               if PrfQueryProfileData(HINI_USERPROFILE, pmSysPtrID, @tS[1], iconP, iconS)
                then case iconP^.iconType of
                      1 : iconName := @iconP^.IconData;
                      3 : iconName := msgStored;
                     else iconName := msgUnknSM;
                     end
                else begin
                      FreeMem(iconP, iconS);
                      iconS := 0;
                     end;
              end
         else iconName := msgDefault;
       end
  else begin
        iconName := msgUnknown; iconS := 0;
        tS := Strg('0', 4 - length(tS)) + tS;
        Move(tS[1], pByteArray(iconName)^[strLen(msgUnknown) - 3], 3);
       end;

{* Display icon *}
 ps := WinGetPS(WinWindowFromID(Window, idICONBORDER));
 Rect.xLeft := 2; Rect.xRight := 4+32;
 Rect.yBottom := 2; Rect.yTop := 4+32;
 WinFillRect(ps, Rect, sysclr_DialogBackground);
 if Icon <> 0 then WinDrawPointer(ps, 4, 4, Icon, DP_NORMAL);
 WinSetDlgItemText(Window, idICONNAME, iconName);
 WinReleasePS(ps);

{* Store icon in UNDO buffer *}
 if newPointer
    then begin
          FreeMem(oldPointerI.pIconData, oldPointerI.cbIconData);
          oldPointerI.cbIconData := 0;
          WinQuerySysPointerData(hWnd_Desktop, Selection, oldPointerI);
          GetMem(oldPointerI.pIconData, oldPointerI.cbIconData);
          WinQuerySysPointerData(hWnd_Desktop, Selection, oldPointerI);
          FreeMem(oldPointer, oldPointerS);
          oldPointer := iconP; oldPointerS := iconS;
          newPointer := FALSE;
         end
    else FreeMem(iconP, iconS);
end;

{* Reload pointer if changed *}
Procedure ResetPointer;
begin
 WinShowPointer(hWnd_Desktop, FALSE);
 WinShowPointer(hWnd_Desktop, TRUE);
 ShowPointer;
end;

{* Reset pointer to default *}
Procedure SetDefaultPointer;
begin
 WinSetSysPointerData(hWnd_Desktop, GetCurrentPointer, pIconInfo(nil));
 ResetPointer;
end;

{* Undo pointer image *}
Procedure DoUndo;
var Selection : USHORT;
    tempS     : String[4];
begin
 if oldPointerS = 0 then begin SetDefaultPointer; Exit; end;
 Selection := GetCurrentPointer;
 WinSetSysPointerData(hWnd_Desktop, Selection, @oldPointerI);
 Str(Selection, tempS); tempS := tempS + #0;
 PrfWriteProfileData(HINI_USERPROFILE, pmSysPtrID, @tempS[1], oldPointer, oldPointerS);
 ResetPointer;
end;

{* Load current icon from file *}
Function LoadCurrentIcon(const fName : String) : boolean;
label
    locEx;
var F     : File;
    Buff  : Pointer;
    iconS : ULong;
    tempS : String;
    ii    : IconInfo;
begin
 LoadCurrentIcon := FALSE;
 inOutRes := 0;
 Assign(F, fName); Reset(F, 1);
 if ioResult <> 0
  then begin
        tempS := Copy('Cannot open file'#13 + fName, 1, 254) + #0;
        WinMessageBox(hWnd_Desktop, Window,
                      @tempS[1], 'Error', 0, MB_ICONEXCLAMATION+MB_MOVEABLE+MB_ENTER);
        Exit;
       end;
{Always set ICON_DATA so changes will take effect immediately}
{even if <store file reference> is in effect}
 ii.cb := sizeOf(ii);
 ii.fFormat := ICON_DATA;
 iconS := fileSize(F); GetMem(Buff, iconS);
 BlockRead(F, Buff^, iconS);
 ii.cbIconData := iconS;
 ii.pIconData := Buff;
 WinSetSysPointerData(hWnd_Desktop, GetCurrentPointer, @ii);
 if GetStorageMethod = 0
  then begin
        FreeMem(Buff, iconS);
        ii.fFormat := ICON_FILE;
        iconS := length(fName) + 1; GetMem(Buff, iconS);
        StrPCopy(Buff, fName);
        ii.pszFileName := Buff;
       end;
 WinSetSysPointerData(hWnd_Desktop, GetCurrentPointer, @ii);
 FreeMem(Buff, iconS);
 LoadCurrentIcon := TRUE;
 ResetPointer;
locEx:
 Close(F);
 inOutRes := 0;
end;

{* <Change> button *}
Procedure DoChange;
var tS : String;
begin
 tS := ChooseFile('*.PTR', 'Choose pointer file');
 if tS = '' then Exit;
 LoadCurrentIcon(tS);
 ShowPointer;
end;

{* <Edit> button *}
Procedure DoEdit;
begin
 DosCreateThread(editThreadID, editThread, 0, 0, 8192);
 WinEnableWindow(WinWindowFromID(Window, idICONLIST), FALSE);
end;

{* <Load Set> button *}
Procedure DoLoadSet;
var tS        : String;
    T         : Text;
    KW        : SmallWord;
    Selection : ULong;
begin
 tS := ChooseFile('*.SET', 'Choose list file');
 if tS = '' then Exit;
 inOutRes := 0;
 Assign(T, tS); Reset(T);
 if ioResult <> 0
  then begin
        tS := Copy('Error reading set'#13 + tS, 1, 254) + #0;
        WinMessageBox(hWnd_Desktop, Window,
                      @tS[1], 'Error', 0, MB_ICONEXCLAMATION+MB_MOVEABLE+MB_ENTER);
        Exit;
       end;
 Selection := GetCurrentPointer;
 While (ioResult = 0) and (not seekEOF(T)) do
  begin
   Readln(T, tS);
   KW := First(';', tS);
   if KW > 0 then tS := copy(tS, 1, pred(KW));
   DelStartSpaces(tS);
   if tS = '' then Continue;
   KW := KeywordSpc(tS, setKeyword^);
   if KW = 0
    then begin
          tS := Copy('Unknown keyword in line'#13 + tS, 1, 254) + #0;
          WinMessageBox(hWnd_Desktop, Window,
                        @tS[1], 'Error', 0, MB_ICONEXCLAMATION+MB_MOVEABLE+MB_ENTER);
          break;
         end;
   DelStartSpaces(tS); Dec(KW);
   WinSendDlgItemMsg(Window, idICONLIST, LM_SELECTITEM,
                     MPARAM(KW), MPARAM(TRUE));
   if tS = ''
    then SetDefaultPointer
    else LoadCurrentIcon(fExpand(tS));
  end;
 Close(T); inOutRes := 0;
 For KW := 1 to nStdNames do
  if Selection = cStdNums[KW]
   then begin
         WinSendDlgItemMsg(Window, idICONLIST, LM_SELECTITEM,
                           MPARAM(Pred(KW)), MPARAM(TRUE));
         break;
        end;
end;

Procedure DoFinishEdit;
begin
 if not editError
  then begin
        newPointer := TRUE;
        if tempFile
         then WinCheckButton(Window, idSTOREDIRECT, 1);
        LoadCurrentIcon(tempIconName);
        if tempFile
         then begin
               tempIconName := tempIconName + #0;
               DosDelete(@tempIconName[1]);
              end;
       end;
 WinEnableWindow(WinWindowFromID(Window, idICONLIST), TRUE);
end;


var deskRect,
    applRect  : Rectl;
    i         : Integer;
begin
 DlgWindowProc := 0;
 case Msg of
  WM_INITDLG : begin
                WinQueryWindowRect(hWnd_Desktop, deskRect);
                WinQueryWindowRect(Window, applRect);
                ApplRect.xLeft := deskRect.xLeft +
                                  ((deskRect.xRight - deskRect.xLeft) -
                                   (applRect.xRight - applRect.xLeft)) div 2;
                ApplRect.yBottom := deskRect.yBottom +
                                  ((deskRect.yTop - deskRect.yBottom) -
                                   (applRect.yTop - applRect.yBottom)) div 2;
                WinSetWindowPos (Window, hWnd_TOP,
                                 applRect.xLeft, applRect.yBottom,
                                 0, 0, SWP_MOVE or SWP_SHOW);
                For i := 1 to nStdNames do
                    WinSendDlgItemMsg(Window, idICONLIST, LM_INSERTITEM,
                        MPFROM2SHORT(SmallWord(LIT_END), 0), MPARAM(cStdNames[i]));
                WinSendDlgItemMsg(Window, idICONLIST, LM_SELECTITEM,
                                  MPARAM(0), MPARAM(TRUE));
                WinCheckButton(Window, idSTOREDIRECT, 1);
               end;
  WM_CONTROL : case SmallWord(mp1) of
                idICONLIST   : case mp1 shr 16 of
                                LN_SELECT : begin newPointer := TRUE; ShowPointer; end;
                               end;
               end;
  WM_COMMAND : case SmallWord(mp1) of
                idBUTDEFAULT : SetDefaultPointer;
                idBUTCHANGE  : DoChange;
                idBUTUNDO    : DoUndo;
                idBUTEDIT    : DoEdit;
                idBUTLOADSET : DoLoadSet;
                idBUTQUIT    : WinDismissDlg(Window, DID_OK);
                idENDEDIT    : DoFinishEdit;
               end;
  else DlgWindowProc := WinDefDlgProc(Window, Msg, mp1, mp2);
 end;
end;

{* Dummy window procedure *}
Function mainWinProc(Window : hWnd; Msg : ULong; mp1,mp2 : MParam) : MResult; cdecl;
begin
 mainWinProc := 0;
 case Msg of
  WM_CREATE : begin
               WinDlgBox(hWnd_Desktop, hWnd_Desktop, DlgWindowProc, 0,
                         idMAINWINDOW, NIL);
               WinPostQueueMsg(appHMQ, WM_QUIT, 0, 0);
              end;
  else mainWinProc := WinDefWindowProc(Window, Msg, mp1, mp2);
 end;
end;

var frame,Client : hwnd;
    Msg          : qMsg;
    FrameFlags   : uLong;

begin
{* Init UNDO structure *}
 oldPointerI.cb := sizeOf(oldPointerI);
{* WinGetSysPointerData always return ICON_DATA *}
 oldPointerI.fFormat := ICON_DATA;
 appHAB := WinInitialize(0);
 appHMQ := WinCreateMsgQueue(appHAB, 0);
 if appHMQ = 0 then Halt(254);
{* Register a dummy class so program will appear in tasklist *}
 WinRegisterClass(appHAB, 'SysIcons', MainWinProc, cs_SizeRedraw, 0);
 Frame := WinCreateStdWindow(hWnd_Desktop, ws_Disabled, FrameFlags,
                             'SysIcons', nil, 0, 0, 0, @Client);
{* Do it *}
 if Frame <> 0
  then While (WinGetMsg(appHAB,Msg,0,0,0)) do WinDispatchMsg(appHAB,Msg);

{* Destroy anything that can be destroyed (AKA kill`em`all) *}
 WinDestroyWindow(Frame);
 WinDestroyMsgQueue(appHMQ);
 WinTerminate(appHAB);
end.
