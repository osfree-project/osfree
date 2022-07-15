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
{$ifndef fpc}{$Cdecl-,OrgName-,AlignRec-,Use32+}{$else}{$Align 1}{$endif}

unit MyCrt;

interface uses vpsyslow;

const

{ CRT modes }

  BW40          = 0;            { 40x25 B/W on Color Adapter   }
  CO40          = 1;            { 40x25 Color on Color Adapter }
  BW80          = 2;            { 80x25 B/W on Color Adapter   }
  CO80          = 3;            { 80x25 Color on Color Adapter }
  Mono          = 7;            { 80x25 on Monochrome Adapter  }
  Font8x8       = 256;          { Add-in for 8x8 font          }

  MON1          = $FE;          { Monochrome, ASCII chars only }
  MON2          = $FD;          { Monochrome, graphics chars   }
  COL1          = $FC;          { Color, ASCII chars only      }
  COL2          = $FB;          { Color, graphics chars        }

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
  ScreenSize: tSysPoint;

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
procedure SetWindowPos;
procedure GetLastMode;

{ The following procedures are not implemented

procedure Sound(Hz: Word);
procedure NoSound;

use new procedure PlaySound instead

}
{$IFDEF DPMI32}
procedure Sound(Hz: Word);
procedure NoSound;
{$ENDIF}

procedure PlaySound(Freq,Duration: Longint);

implementation uses Dos, {$ifdef OS2} Os2Def, {$IFDEF FPC} DosCalls, KbdCalls, VioCalls{$ELSE} Os2Base{$ENDIF}{$endif};

{ Private variables }

var
  NormAttr: Byte;
  DelayCount: Longint;
  PrevXcptProc: Pointer;

{ Determines if a key has been pressed on the keyboard and returns True }
{ if a key has been pressed                                             }

function KeyPressed: Boolean;
var
  Key: KbdKeyInfo;
begin
  KeyPressed := SysKeyPressed;
end;

{ Reads a character from the keyboard and returns a character or an     }
{ extended scan code.                                                   }

function ReadKey: Char;
var
  Key: KbdKeyInfo;
begin
  ReadKey := SysReadKey;
end;

{ Reads normal character attribute }

procedure ReadNormAttr;
begin
  NormAttr := SysReadAttributesAt(WhereX-1, WhereY-1);
  NormVideo;
end;

{ Setups window coordinates }

procedure SetWindowPos;
begin
  WindMin := 0;
  WindMax := ScreenSize.x - 1 + (ScreenSize.y - 1) shl 8;
end;

{ Stores current video mode in LastMode }

procedure GetLastMode;
begin
  LastMode := SysTvGetScrMode( @ScreenSize);
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
  TextAttr := LightGray;
  SysTvSetScrMode( Mode );
  GetLastMode;
  NormVideo;
  SetWindowPos;
  ClrScr;
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
      if (X2 < ScreenSize.x) and (Y2 < ScreenSize.y) then
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
    if (X1 <= Lo(WindMax)) and (Y1 <= Hi(WindMax)) then
      SysTVSetCurPos( X1, Y1 );
  end;
end;

{ Returns the X coordinate of the current cursor location.              }

function WhereX: Byte;
var
  X,Y: SmallWord;
begin
  SysGetCurPos(X, Y);
  WhereX := X - Lo(WindMin) + 1;
end;

{ Returns the Y coordinate of the current cursor location.              }

function WhereY: Byte;
var
  X,Y: SmallWord;
begin
  SysGetCurPos(X,Y);
  WhereY := Y - Hi(WindMin) + 1;
end;

{ Clears the screen and returns the cursor to the upper-left corner.    }

procedure ClrScr;
var
  Cell: SmallWord;
begin
  SysScrollUp( Lo(WindMin), Hi(WindMin), Lo(WindMax), Hi(WindMax), Hi(WindMax)-Hi(WindMin)+1, Ord( ' ' ) + TextAttr shl 8 );
  GotoXY(1,1);
end;

{ Clears all characters from the cursor position to the end of the line }
{ without moving the cursor.                                            }

procedure ClrEol;
var
  X,Y, Len: SmallWord;
  Buffer: Array[0..255] of char;
begin
  SysGetCurPos(X, Y);
  Len := succ(Lo(WindMax)-X);
  fillchar(Buffer, Len, ' ');
  SysWrtCharStrAtt(@Buffer, Len, X, Y, TextAttr);
end;

{ Inserts an empty line at the cursor position.                         }

procedure InsLine;
var
  Cell,X,Y: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  SysGetCurPos(X,Y);
  SysScrollDn(Lo(WindMin),Y,Lo(WindMax),Hi(WindMax),1,Cell);
end;

{ Deletes the line containing the cursor.                               }

procedure DelLine;
var
  Cell,X,Y: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  SysGetCurPos(X,Y);
  SysScrollUp(Lo(WindMin),Y,Lo(WindMax),Hi(WindMax),1,Cell);
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

{
{ Delays a specified number of milliseconds. }

procedure Delay(MS: Longint);
begin
  SysCtrlSleep( MS );
end;

{ Plays sound of a specified frequency and duration.                    }

procedure PlaySound(Freq,Duration: Longint);
begin
  SysBeepEx(Freq, Duration);
end;

{$IFDEF DPMI32}
procedure Sound(Hz: Word);
begin
  SysSound(Hz);
end;

procedure NoSound;
begin
  SysNoSound;
end;
{$ENDIF}


{ Do line feed operation }

procedure LineFeed;
var
  Cell: SmallWord;
begin
  Cell := Ord(' ') + TextAttr shl 8;
  SysScrollUp(Lo(WindMin),Hi(WindMin),Lo(WindMax),Hi(WindMax),1,Cell);
end;

{ Outputs packed string to the CRT device }

type
  PWin32Cell = ^TWin32Cell;
  TWin32Cell = record
    Ch:     SmallWord;
    Attr:   SmallWord;
  end;

procedure WritePackedString(S: PChar; Len: Longint);
var
  Buf: array[1..256] of Char;
  I,BufChars: Integer;
  X,Y:        SmallWord;
  X1,Y1:      Longint;
  C:          Char;

  procedure FlushBuffered;
  begin
    If BufChars > 0 then
    begin
      SysWrtCharStrAtt(@Buf, BufChars, X1, Y1, TextAttr);
      BufChars := 0;
      X1 := X;
      Y1 := Y;
    end;
  end;

begin
  SysGetCurPos(X, Y);
  BufChars := 0;
  X1 := X;
  Y1 := Y;
  for I := 0 to Len - 1 do
  begin
    C := S[I];
    case C of
      ^J:               { Line Feed }
        begin
          FlushBuffered;
          {$IFDEF LINUX}
          X := Lo(WindMin);
          x1 := x;
          {$ENDIF}
          if Y >= Hi(WindMax) then LineFeed else Inc(Y);
          y1 := y;
        end;
      //--{$IFNDEF LINUX}
      ^M:               { Carriage return }
        begin
          FlushBuffered;
          X := Lo(WindMin);
          x1 := x;
        end;
      //--{$ENDIF}
      ^H:               { Backspace }
        begin
          FlushBuffered;
          if X > Lo(WindMin) then Dec(X);
          if X1 > Lo(WindMin) then Dec(X1);
        end;
      ^G:               { Bell }
        SysBeep;
      else
        Inc(BufChars);
        Buf[BufChars] := C;
        Inc(X);
        if X > Lo(WindMax) then
        begin
          FlushBuffered;
          X := Lo(WindMin);
          X1 := X;
          Inc(Y);
          if Y > Hi(WindMax) then
          begin
            FlushBuffered;
            LineFeed;
            Y := Hi(WindMax);
          end;
          Y1 := Y;
        end;
    end;
  end;
  FlushBuffered;
  SysTVSetCurPos(X, Y);
end;

{ CRT text file I/O functions }

function CrtRead(var F: Text): Longint;
var
  CurPos: Longint;
  C: Char;
begin
  with TextRec(F) do
  begin
    CurPos := 0;
    repeat
      SysFlushKeyBuf;
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

function CrtClose(var F: Text): Longint;
begin
  {$IfDef Linux}
  if TextRec(F).Mode = fmInput then
    SysTVDoneCursor  // There is no SysTvDoneKbd
  else
    SysTVDoneCursor;
  {$EndIf Linux}
  CrtClose := 0;                { I/O result = 0: success }  
end;

function CrtOpen(var F: Text): Longint;
begin
  with TextRec(F) do
  begin
    CloseFunc := @CrtClose;
    if Mode = fmInput then
    begin
      {$IFDEF LINUX}
      SysTvKbdInit;
      {$ENDIF}
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

var
  PrevCtrlBreakHandler : TCtrlBreakHandler = nil;

function CrtCtrlBreakHandler: Boolean;
begin
  Result:=false;
  // let other CtrlBreakHandler process it (TVision)
  if Assigned(PrevCtrlBreakHandler) then
    Result:=PrevCtrlBreakHandler;
  // if not handled  we look at CheckBreak flag  if me may terminate
  if not Result then
    Result := not CheckBreak;
end;

Procedure AssignConToCrt;
var hType,hAttr : Longint;
begin
 Move(Input, StdIn, sizeOf(StdIn));
 Move(Output, StdOut, sizeOf(StdOut));
{$IFDEF WIN32}
 hAttr := GetFileType(GetStdHandle(STD_INPUT_HANDLE));
 if hAttr <> FILE_TYPE_CHAR
  then begin
        AssignCrt(Input);
        Reset(Input);
       end
  else RedirInput := True;
 hAttr := GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
 if hAttr <> FILE_TYPE_CHAR
  then begin
        AssignCrt(Output);
        ReWrite(Output);
       end
  else RedirOutput := True;
{$ENDIF}
{$IFDEF OS2}
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
{$ENDIF}
{$IF 0}
  AssignCrt(Input);  Reset(Input);
  AssignCrt(Output); ReWrite(Output);
{$ENDIF}
end;

begin
  SysTvInitCursor;
  GetLastMode;
  SetWindowPos;
  ReadNormAttr;
  {AssignConToCrt;}
  PrevCtrlBreakHandler := CtrlBreakHandler;
  CtrlBreakHandler := CrtCtrlBreakHandler;
  SysCtrlSetCBreakHandler;
end.
