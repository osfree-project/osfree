{ÛßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßÛ}
{Û                                                       Û}
{Û      Virtual Pascal Runtime Library.  Version 1.0.    Û}
{Û      CRT Interface unit for OS/2                      Û}
{Û      ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÛ}
{Û      Copyright (C) 1995 B&M&T Corporation             Û}
{Û      ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÛ}
{Û      Written by Vitaly Miryanov                       Û}
{Û                                                       Û}
{ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß}

{$S-,R-,Q-,I-}

{$ifndef fpc}{$Cdecl-,OrgName-,AlignRec-,Use32+}{$endif}

unit Crt;

interface

const

{ CRT modes }

  BW40          = 0;            { 40x25 B/W on Color Adapter   }
  CO40          = 1;            { 40x25 Color on Color Adapter }
  BW80          = 2;            { 80x25 B/W on Color Adapter   }
  CO80          = 3;            { 80x25 Color on Color Adapter }
  Mono          = 7;            { 80x25 on Monochrome Adapter  }
  Font8x8       = 256;          { Add-in for 8x8 font          }

{ Foreground and background color constants }

  Black         = 0;
  Blue          = 1;
  Green         = 2;
  Cyan          = 3;
  Red           = 4;
  Magenta       = 5;
  Brown         = 6;
  LightGray     = 7;

{ Foreground color constants }

  DarkGray      = 8;
  LightBlue     = 9;
  LightGreen    = 10;
  LightCyan     = 11;
  LightRed      = 12;
  LightMagenta  = 13;
  Yellow        = 14;
  White         = 15;

{ Add-in for blinking }

  Blink         = 128;

{ Interface variables }

const
  CheckBreak: Boolean = True;   { Enable Ctrl-Break      }
  CheckEOF: Boolean = False;    { Allow Ctrl-Z for EOF?  }
  TextAttr: Byte = LightGray;   { Current text attribute }
  RedirOutput: boolean = False; {True if stdOut is redirected}
  RedirInput: boolean = False;  {True if stdIn is redirected}

var
  LastMode: Word;               { Current text mode }
  WindMin: Word;                { Window upper left coordinates }
  WindMax: Word;                { Window lower right coordinates }
  StdIn,                        { Old standard input }
  StdOut: Text;                 { and output streams }

{ The following interface variables are not used (for compatibility only) }

const
  DirectVideo: Boolean = False; { Enable direct video addressing }
  CheckSnow: Boolean = True;    { Enable snow filtering }

{ Interface procedures }

procedure AssignCrt(var F: Text);
function KeyPressed: Boolean;
function ReadKey: Char;
procedure TextMode(Mode: Integer);
procedure Window(X1,Y1,X2,Y2: Byte);
procedure GotoXY(X,Y: Byte);
function WhereX: Byte;
function WhereY: Byte;
procedure ClrScr;
procedure ClrEol;
procedure InsLine;
procedure DelLine;
procedure TextColor(Color: Byte);
procedure TextBackground(Color: Byte);
procedure LowVideo;
procedure HighVideo;
procedure NormVideo;
procedure Delay(MS: Longint);

{ The following procedures are not implemented

procedure Sound(Hz: Word);
procedure NoSound;

use new procedure PlaySound instead

}

procedure PlaySound(Freq,Duration: Longint);

implementation

uses Dos, Os2Def, {$IFDEF FPC} DosCalls, KbdCalls, VioCalls; {$ELSE} Os2Base; {$ENDIF}

{ Private variables }

{$IFDEF FPC}
type SmallWord = Word;
{$ENDIF}

var
  VioMode: VioModeInfo;
  NormAttr: Byte;
  DelayCount: Longint;
{$IFNDEF FPC}
  PrevXcptProc: Pointer;
{$ENDIF}

const
  ScanCode: Byte = 0;

  QSV_MAX_PATH_LENGTH       =  1;
  QSV_MAX_TEXT_SESSIONS     =  2;
  QSV_MAX_PM_SESSIONS       =  3;
  QSV_MAX_VDM_SESSIONS      =  4;
  QSV_BOOT_DRIVE            =  5;
  QSV_DYN_PRI_VARIATION     =  6;
  QSV_MAX_WAIT              =  7;
  QSV_MIN_SLICE             =  8;
  QSV_MAX_SLICE             =  9;
  QSV_PAGE_SIZE             = 10;
  QSV_VERSION_MAJOR         = 11;
  QSV_VERSION_MINOR         = 12;
  QSV_VERSION_REVISION      = 13;
  QSV_MS_COUNT              = 14;
  QSV_TIME_LOW              = 15;
  QSV_TIME_HIGH             = 16;
  QSV_TOTPHYSMEM            = 17;
  QSV_TOTRESMEM             = 18;
  QSV_TOTAVAILMEM           = 19;
  QSV_MAXPRMEM              = 20;
  QSV_MAXSHMEM              = 21;
  QSV_TIMER_INTERVAL        = 22;
  QSV_MAX_COMP_LENGTH       = 23;
  QSV_FOREGROUND_FS_SESSION = 24;
  QSV_FOREGROUND_PROCESS    = 25;
  QSV_NUMPROCESSORS         = 26;
  QSV_MAXHPRMEM             = 27;
  QSV_MAXHSHMEM             = 28;
  QSV_MAXPROCESSES          = 29;
  QSV_VIRTUALADDRESSLIMIT   = 30;
  QSV_INT10ENABLED          = 31;
  QSV_MAX                   = QSV_INT10ENABLED;

{ Determines if a key has been pressed on the keyboard and returns True }
{ if a key has been pressed                                             }

function KeyPressed: Boolean;
var
  Key: KbdKeyInfo;
begin
  KbdPeek(Key,0);
  KeyPressed := (ScanCode <> 0) or ((Key.fbStatus and kbdtrf_Final_Char_In) <> 0);
end;

{ Reads a character from the keyboard and returns a character or an     }
{ extended scan code.                                                   }

function ReadKey: Char;
var
  Key: KbdKeyInfo;
begin
 if RedirInput
  then Read(Input, Key.chChar)
  else begin
        If ScanCode <> 0
         then begin
               Key.chChar := Chr(ScanCode);
               ScanCode := 0;
              end
         else begin
               KbdCharIn(Key,io_Wait,0);
               case Key.chChar of
                #0:
                 ScanCode := Key.chScan;
                #$E0:           {   Up, Dn, Left Rt Ins Del Home End PgUp PgDn C-Home C-End C-PgUp C-PgDn C-Left C-Right C-Up C-Dn }
                 if Key.chScan in [$48,$50,$4B,$4D,$52,$53,$47, $4F,$49, $51, $77,   $75,  $84,   $76,   $73,   $74,    $8D, $91]
                  then begin
                        ScanCode := Key.chScan;
                        Key.chChar := #0;
                       end;
               end;
              end;
       end;
 ReadKey := Key.chChar;
end;

{ Reads normal character attribute }

procedure ReadNormAttr;
var
  Cell,Size: Word; {SmallWord;}
begin
  Size := 2;
  VioReadCellStr(Cell, Size, WhereY-1, WhereX-1, 0);
  NormAttr := Hi(Cell) and $7F;
  NormVideo;
end;

{ Setups window coordinates }

procedure SetWindowPos;
begin
  WindMin := 0;
  WindMax := VioMode.Col - 1 + (VioMode.Row - 1) shl 8;
end;

{ Stores current video mode in LastMode }

procedure GetLastMode;
begin
  VioMode.cb := SizeOf(VioMode);
  VioGetMode(VioMode, 0);
  with VioMode do
  begin
    if Col = 40 then LastMode := BW40 else LastMode := BW80;
    if (fbType and vgmt_DisableBurst) = 0 then
      if LastMode = BW40 then LastMode := CO40 else LastMode := CO80;
    if Color = 0 then LastMode := Mono;
    if Row > 25 then Inc(LastMode,Font8x8);
  end;
end;

{ Selects a specific text mode. The valid text modes are:               }
{   BW40: 40x25 Black and white                                         }
{   CO40  40x25 Color                                                   }
{   BW80  80x25 Black and white                                         }
{   CO80  80x25 Color                                                   }
{   Mono  80x25 Black and white                                         }
{   Font8x8 (Add-in) 43-/50-line mode                                   }

procedure TextMode(Mode: Integer);
var BiosMode: Byte; Cell: SmallWord; VideoConfig: VioConfigInfo;
begin
  GetLastMode;
  TextAttr := LightGray;
  BiosMode := Lo(Mode);
  VideoConfig.cb := SizeOf(VideoConfig);
  VioGetConfig(0, VideoConfig, 0);
  with VioMode do
  begin
    cb := SizeOf(VioMode);
    fbType := vgmt_Other;
    Color := colors_16;         { Color }
    Row := 25;                  { 80x25 }
    Col := 80;
    VRes := 400;
    HRes := 720;
    case BiosMode of            { 40x25 }
      BW40,CO40:
        begin
          Col := 40; HRes := 360;
        end;
    end;
    if (Mode and Font8x8) <> 0 then
    case VideoConfig.Adapter of { 80x43 }
      display_Monochrome..display_CGA: ;
      display_EGA:
        begin
          Row := 43; VRes := 350; HRes := 640;
        end;
      else                      { 80x50 }
        begin
          Row := 50; VRes := 400; HRes := 720;
        end;
    end;
    case BiosMode of            { Black and white }
      BW40,BW80: fbType := vgmt_Other + vgmt_DisableBurst;
      Mono:
        begin                   { Monochrome }
          HRes := 720; VRes := 350; Color := 0; fbType := 0;
        end;
    end;
  end;
  VioSetMode(VioMode, 0);
  VioGetMode(VioMode, 0);
  NormVideo;
  SetWindowPos;
  Cell := Ord(' ') + TextAttr shl 8;    { Clear entire screen }
  VioScrollUp(0,0,65535,65535,65535,Cell,0);
end;

{ Defines a text window on the screen.                                  }

procedure Window(X1,Y1,X2,Y2: Byte);
begin
  if (X1 <= X2) and (Y1 <= Y2) then
  begin
    Dec(X1);
    Dec(Y1);
    if (X1 >= 0) and (Y1 >= 0) then
    begin
      Dec(X2);
      Dec(Y2);
      if (X2 < VioMode.Col) and (Y2 < VioMode.Row) then
      begin
        WindMin := X1 + Y1 shl 8;
        WindMax := X2 + Y2 shl 8;
        GotoXY(1,1);
      end;
    end;
  end;
end;

{ Moves the cursor to the given coordinates within the screen.          }

procedure GotoXY(X,Y: Byte);
var
  X1,Y1: Word;
begin
  if (X > 0) and (Y > 0) then
  begin
    X1 := X - 1 + Lo(WindMin);
    Y1 := Y - 1 + Hi(WindMin);
    if (X1 <= Lo(WindMax)) and (Y1 <= Hi(WindMax)) then VioSetCurPos(Y1,X1,0);
  end;
end;

{ Returns the X coordinate of the current cursor location.              }

function WhereX: Byte;
var
  X,Y: SmallWord;
begin
  VioGetCurPos(Y,X,0);
  WhereX := X - Lo(WindMin) + 1;
end;

{ Returns the Y coordinate of the current cursor location.              }

function WhereY: Byte;
var
  X,Y: SmallWord;
begin
  VioGetCurPos(Y,X,0);
  WhereY := Y - Hi(WindMin) + 1;
end;

{ Clears the screen and returns the cursor to the upper-left corner.    }

procedure ClrScr;
var
  Cell: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  VioScrollUp(Hi(WindMin),Lo(WindMin),Hi(WindMax),Lo(WindMax),Hi(WindMax)-Hi(WindMin)+1,Cell,0);
  GotoXY(1,1);
end;

{ Clears all characters from the cursor position to the end of the line }
{ without moving the cursor.                                            }

procedure ClrEol;
var
  Cell,X,Y: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  VioGetCurPos(Y,X,0);
  VioScrollUp(Y,X,Y,Lo(WindMax),1,Cell,0);
end;

{ Inserts an empty line at the cursor position.                         }

procedure InsLine;
var
  Cell,X,Y: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  VioGetCurPos(Y,X,0);
  VioScrollDn(Y,Lo(WindMin),Hi(WindMax),Lo(WindMax),1,Cell,0);
end;

{ Deletes the line containing the cursor.                               }

procedure DelLine;
var
  Cell,X,Y: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  VioGetCurPos(Y,X,0);
  VioScrollUp(Y,Lo(WindMin),Hi(WindMax),Lo(WindMax),1,Cell,0);
end;

{ Selects the foreground character color.                               }

procedure TextColor(Color: Byte);
begin
  if Color > White then Color := (Color and $0F) or $80;
  TextAttr := (TextAttr and $70) or Color;
end;

{ Selects the background color.                                         }

procedure TextBackground(Color: Byte);
begin
  TextAttr := (TextAttr and $8F) or ((Color and $07) shl 4);
end;

{ Selects low intensity characters.                                     }

procedure LowVideo;
begin
  TextAttr := TextAttr and $F7;
end;

{ Selects normal intensity characters.                                  }

procedure NormVideo;
begin
  TextAttr := NormAttr;
end;

{ Selects high-intensity characters.                                    }

procedure HighVideo;
begin
  TextAttr := TextAttr or $08;
end;

{ Waits for next timer tick or delays 1ms }

function DelayLoop(Count: Longint; var StartValue: ULong): Longint;
var
  Value: ULong;
begin
  repeat
    DosQuerySysInfo(qsv_Ms_Count,qsv_Ms_Count,Value,SizeOf(Value));
    Dec(Count);
  until (Value <> StartValue) or (Count = -1);
  StartValue := Value;
  DelayLoop := Count;
end;

{ Delays a specified number of milliseconds. DosSleep is too inexact on }
{ small time intervals. More over, the least time interval for DosSleep }
{ is 1 timer tick (usually 31ms). That is why for small time intervals  }
{ special delay routine is used. Unfortunately, even this routine cannot}
{ be exact in the multitasking environment.                             }

procedure Delay(MS: Longint);
var
  StartValue,Value: ULong;
  Count: Longint;
begin
  if MS >= 31 then DosSleep(MS)
 else
  begin
    DosQuerySysInfo(qsv_Ms_Count,qsv_Ms_Count,StartValue,SizeOf(StartValue));
    Value := StartValue;
    Count := MS;
    repeat
      DelayLoop(DelayCount,Value);
      Dec(Count)
    until (Value-StartValue >= MS) or (Count <= 0);
  end;
end;

{ Calculates 1ms delay count for DelayLoop routine. }
{ CalcDelayCount is called once at startup.         }

procedure CalcDelayCount;
var
  Interval,StartValue,Value: ULong;
begin
  DosQuerySysInfo(qsv_Timer_Interval,qsv_Timer_Interval,Interval,SizeOf(Interval));
  DosQuerySysInfo(qsv_Ms_Count,qsv_Ms_Count,StartValue,SizeOf(StartValue));
  repeat
    DosQuerySysInfo(qsv_Ms_Count,qsv_Ms_Count,Value,SizeOf(Value));
  until Value <> StartValue;
  DelayCount := -DelayLoop(-1,Value) div Interval * 10;
  if DelayCount = 0 then Inc(DelayCount);
end;

{ Plays sound of a specified frequency and duration.                    }

procedure PlaySound(Freq,Duration: Longint);
begin
  DosBeep(Freq,Duration);
end;

{ Do line feed operation }

procedure LineFeed;
var
  Cell: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  VioScrollUp(Hi(WindMin),Lo(WindMin),Hi(WindMax),Lo(WindMax),1,Cell,0);
end;

{ Outputs packed string to the CRT device }

procedure WritePackedString(S: PChar; Len: Longint);
var X,Y,cX,cY : SmallWord;
    i,sP,sL   : Longint;
    C         : Char;
begin
 VioGetCurPos(Y,X,0);
 i := 0;
 While i < Len do
  begin
   sP := i; sL := 0;
   cX := X; cY := Y;
   repeat
    C := S[i]; Inc(i);
    if C in [^J,^M,^H,^G] then break;
    Inc(sL); Inc(X);
   until (i >= pred(Len)) or (X > Lo(WindMax));
   if sL <> 0 then VioWrtCharStrAtt(@S[sP], sL, cY, cX, TextAttr, 0);
   case C of
    ^J : if Y >= Hi(WindMax) then LineFeed else Inc(Y); { Line Feed       }
    ^M : X := Lo(WindMin);                              { Carriage return }
    ^H : if X > Lo(WindMin) then Dec(X);                { Backspace       }
    ^G : begin                                          { Bell            }
          if (X <> cX) or (Y <> cY) then VioSetCurPos(Y,X,0);
          cX := X; cY := Y;
          VioWrtTTY(@C,1,0);
         end;
   end;
   if X > Lo(WindMax)
    then begin
          X := Lo(WindMin);
          Inc(Y);
         end;
   if Y > Hi(WindMax)
    then begin
          LineFeed;
          Y := Hi(WindMax);
         end;
   if (X <> cX) or (Y <> cY) then VioSetCurPos(Y,X,0);
  end;
end;

{ CRT text file I/O functions }

function CrtRead(var F: Text): Longint;
var
  CurPos : Longint;
  C      : Char;
begin
  with TextRec(F) do
  begin
    CurPos := 0;
    repeat
      ScanCode := 0;
      C := ReadKey;
      case C of
        ^H:                     { Backspace }
          if CurPos > 0 then
          begin
            WritePackedString(^H' '^H, 3);
            Dec(CurPos);
          end;
        #27:                    { Escape }
          while CurPos > 0 do
          begin
            WritePackedString(^H' '^H, 3);
            Dec(CurPos);
          end;
        ' '..#255:
          if CurPos < BufSize - 2 then
          begin
            BufPtr^[CurPos] := C;
            Inc(CurPos);
            WritePackedString(@C,1);
          end;
      end; { case }
    until (C = ^M) or (CheckEOF and (C = ^Z));
    BufPtr^[CurPos] := C;
    Inc(CurPos);
    if C = ^M then              { Carriage Return }
    begin
      BufPtr^[CurPos] := ^J;    { Line Feed }
      Inc(CurPos);
      WritePackedString(^M^J,2);
    end;
    BufPos := 0;
    BufEnd := CurPos;
  end;
  CrtRead := 0;                 { I/O result = 0: success }
end;

function CrtWrite(var F: Text): Longint;
begin
  with TextRec(F) do
  begin
    WritePackedString(PChar(BufPtr),BufPos);
    BufPos := 0;
  end;
  CrtWrite := 0;                { I/O result = 0: success }
end;

function CrtReturn(var F: Text): Longint;
begin
  CrtReturn := 0;               { I/O result = 0: success }
end;

function CrtOpen(var F: Text): Longint;
begin
  with TextRec(F) do
  begin
    CloseFunc := @CrtReturn;
    if Mode = fmInput then
    begin
      InOutFunc := @CrtRead;
      FlushFunc := @CrtReturn;
    end
   else
    begin
      Mode := fmOutput;
      InOutFunc := @CrtWrite;
      FlushFunc := @CrtWrite;
    end;
  end;
  CrtOpen := 0;                 { I/O result = 0: success }
end;

{ Associates a text file with CRT device.                               }

procedure AssignCrt(var F: Text);
begin
  with TextRec(F) do
  begin
    Handle := $FFFFFFFF;
    Mode := fmClosed;
    BufSize := SizeOf(Buffer);
    BufPtr := @Buffer;
    OpenFunc := @CrtOpen;
    Name[0] := #0;
  end;
end;

{ Signal Handler }

function CtrlBreakHandler(Report:       PExceptionReportRecord;
                          Registration: PExceptionRegistrationRecord;
                          Context:      PContextRecord;
                          P:            Pointer): ULong; cdecl;
begin
{$IFNDEF FPC}
  if not CheckBreak and (Report^.ExceptionNum = xcpt_Signal)
    then CtrlBreakHandler := xcpt_Continue_Execution
    else CtrlBreakHandler := xcpt_Continue_Search;
{$ENDIF}
end;

Procedure AssignConToCrt;
var hType,hAttr : Longint;
begin
 Move(Input, StdIn, sizeOf(StdIn));
 Move(Output, StdOut, sizeOf(StdOut));
 DosQueryHType(0, hType, hAttr);
 if (hType and 3 = 1) and (hAttr and 1 <> 0)
  then begin
        AssignCrt(Input);
        Reset(Input);
       end
  else RedirInput := True;
 DosQueryHType(1, hType, hAttr);
 if (hType and 3 = 1) and (hAttr and 2 <> 0)
  then begin
        AssignCrt(Output);
        ReWrite(Output);
       end
  else RedirOutput := True;
end;

begin
 GetLastMode;
 if (VioMode.fbType and vgmt_Graphics) <> 0 then TextMode(CO80);
 ReadNormAttr;
 SetWindowPos;
 AssignConToCrt;
 CalcDelayCount;
{$IFNDEF FPC}
 PrevXcptProc := XcptProc;
 XcptProc := @CtrlBreakHandler;
{$ENDIF}
end.

