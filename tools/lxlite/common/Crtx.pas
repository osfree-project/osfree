{$S-,R-,Q-,I-}
{$ifndef fpc}{$Cdecl-,OrgName-,AlignRec-,Use32+}{$endif}
{$ifdef fpc}
{$Mode objfpc} {$H+}
{$ModeSwitch nestedprocvars}
{$endif}

unit Crtx;

interface uses
{$ifdef fpc}
  Crt
{$ifdef os2}
  , doscalls
{$endif};
{$else}
  MyCrt, vpsyslow
{$ifdef os2}
  , os2base
{$endif};
{$endif}

const
  TextAttr: Byte = LightGray;   { Current text attribute }
  RedirOutput: boolean = False; {True if stdOut is redirected}
  RedirInput: boolean = False;  {True if stdIn is redirected}

var
  LastMode: Word;               { Current text mode }
  WindMin: Word;                { Window upper left coordinates }
  WindMax: Word;                { Window lower right coordinates }
  StdIn,                        { Old standard input }
  StdOut: Text;                 { and output streams }

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

implementation

var
  NormAttr: Byte;
  DelayCount: Longint;
  PrevXcptProc: Pointer;
  {ScreenSize: tSysPoint;}

{ Setups window coordinates }

procedure SetWindowPos;
begin
  WindMin := 0;
  {WindMax := ScreenSize.x - 1 + (ScreenSize.y - 1) shl 8;}
end;

{ Associates a text file with CRT device.                               }

procedure AssignCrt(var F: Text);
begin
{$ifdef virtualpascal}
  MyCrt.AssignCrt(f);
{$else}
  Crt.AssignCrt(f);
{$endif}
end;

{ Determines if a key has been pressed on the keyboard and returns True }
{ if a key has been pressed                                             }

function KeyPressed: Boolean;
begin
{$ifdef virtualpascal}
  KeyPressed := MyCrt.KeyPressed;
{$else}
  KeyPressed := Crt.KeyPressed;
{$endif}
end;

{ Reads a character from the keyboard and returns a character or an     }
{ extended scan code.                                                   }

function ReadKey: Char;
begin
{$ifdef virtualpascal}
  ReadKey := MyCrt.ReadKey;
{$else}
  ReadKey := Crt.ReadKey;
{$endif}
end;

{ Selects a specific text mode. The valid text modes are:               }
{   BW40: 40x25 Black and white                                         }
{   CO40  40x25 Color                                                   }
{   BW80  80x25 Black and white                                         }
{   CO80  80x25 Color                                                   }
{   Mono  80x25 Black and white                                         }
{   Font8x8 (Add-in) 43-/50-line mode                                   }

procedure TextMode(Mode: Integer);
begin
  TextAttr := LightGray;
{$ifdef virtualpascal}
  MyCrt.TextMode(Mode);
{$else}
  Crt.TextMode(Mode);
{$endif}
  {GetLastMode;}
  NormVideo;
  SetWindowPos;
  ClrScr;
end;

{ Defines a text window on the screen.                                  }

procedure Window(X1,Y1,X2,Y2: Byte);
begin
{$ifdef virtualpascal}
  MyCrt.Window(X1,Y1,X2,Y2);
{$else}
  Crt.Window(X1,Y1,X2,Y2);
{$endif}
end;

{ Moves the cursor to the given coordinates within the screen.          }

procedure GotoXY(X,Y: Byte);
begin
{$ifdef virtualpascal}
  MyCrt.GotoXY(X,Y);
{$else}
  Crt.GotoXY(X,Y);
{$endif}
end;

{ Returns the X coordinate of the current cursor location.              }

function WhereX: Byte;
begin
{$ifdef virtualpascal}
  WhereX := MyCrt.WhereX;
{$else}
  WhereX := Crt.WhereX;
{$endif}
end;

{ Returns the Y coordinate of the current cursor location.              }

function WhereY: Byte;
begin
{$ifdef virtualpascal}
  WhereY := MyCrt.WhereY;
{$else}
  WhereY := Crt.WhereY;
{$endif}
end;

{ Clears the screen and returns the cursor to the upper-left corner.    }

procedure ClrScr;
begin
{$ifdef virtualpascal}
  MyCrt.ClrScr;
{$else}
  Crt.ClrScr;
{$endif}
end;

{ Clears all characters from the cursor position to the end of the line }
{ without moving the cursor.                                            }

procedure ClrEol;
var
  X,Y, Len: Word;
  Buffer: Array[0..255] of char;
begin
  {X := WhereX; Y := WhereY;
  Len := succ(Lo(WindMax)-X);
  fillchar(Buffer, Len, ' ');
  SysWrtCharStrAtt(@Buffer, Len, X, Y, TextAttr);}
{$ifdef virtualpascal}
  MyCrt.ClrEol;
{$else}
  Crt.ClrEol;
{$endif}
end;

{ Inserts an empty line at the cursor position.                         }

procedure InsLine;
begin
{$ifdef virtualpascal}
  MyCrt.InsLine;
{$else}
  Crt.InsLine;
{$endif}
end;

{ Deletes the line containing the cursor.                               }

procedure DelLine;
begin
{$ifdef virtualpascal}
  MyCrt.DelLine;
{$else}
  Crt.DelLine;
{$endif}
end;

{ Selects the foreground character color.                               }

procedure TextColor(Color: Byte);
begin
  if Color > White then Color := (Color and $0F) or $80;
  TextAttr := (TextAttr and $70) or Color;
{$ifdef virtualpascal}
  MyCrt.TextColor(Color);
{$else}
  Crt.TextColor(Color);
{$endif}
end;

{ Selects the background color.                                         }

procedure TextBackground(Color: Byte);
begin
  TextAttr := (TextAttr and $8F) or ((Color and $07) shl 4);
{$ifdef virtualpascal}
  MyCrt.TextBackground(Color);
{$else}
  Crt.TextBackground(Color);
{$endif}
end;

{ Selects low intensity characters.                                     }

procedure LowVideo;
begin
  TextAttr := TextAttr and $F7;
{$ifdef virtualpascal}
  MyCrt.LowVideo;
{$else}
  Crt.LowVideo;
{$endif}
end;

{ Selects high-intensity characters.                                    }

procedure HighVideo;
begin
  TextAttr := TextAttr or $08;
{$ifdef virtualpascal}
  MyCrt.HighVideo;
{$else}
  Crt.HighVideo;
{$endif}
end;

{ Selects normal intensity characters.                                  }

procedure NormVideo;
begin
  TextAttr := NormAttr;
{$ifdef virtualpascal}
  MyCrt.NormVideo;
{$else}
  Crt.NormVideo;
{$endif}
end;

{ Delays a specified number of milliseconds. }

procedure Delay(MS: Longint);
begin
{$ifdef virtualpascal}
  MyCrt.Delay(MS);
{$else}
  Crt.Delay(MS);
{$endif}
end;

{ Plays sound of a specified frequency and duration.                    }

{$ifdef virtualpascal}
procedure PlaySound(Freq,Duration: Longint);
begin
  SysBeepEx(Freq, Duration);
end;

{$IFDEF DPMI32}
procedure Sound(Hz: Word);
begin
  Crt.SysSound(Hz);
end;

procedure NoSound;
begin
  Crt.SysNoSound;
end;
{$ENDIF}
{$endif}

Procedure AssignConToCrt;
var hType,hAttr : Longint;
begin
{$IFDEF OS2}
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
{$ELSE}
  AssignCrt(Input);  Reset(Input);
  AssignCrt(Output); ReWrite(Output);
{$ENDIF}
end;

begin
  AssignConToCrt;
end.
