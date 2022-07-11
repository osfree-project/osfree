{&G3+}
{$ifndef fpc}
{$AlignCode-,AlignData-,AlignRec-,Speed-,Frame-,Use32+}
{$else}
{$Align 1}
{$Optimization STACKFRAME}
{$mode objfpc}
{$ModeSwitch nestedprocvars}
{$endif}
unit Country;

Interface uses
{$ifdef OS2}
  os2def, {$ifndef FPC}os2base{$else}doscalls{$endif},
{$endif} 
  miscUtil, strOp, dos;

const
    cyDefault    = 0; {Use default country}
    cpDefault    = 0; {Use default codepage}

  { Option flags for TimeStr }
    toHour       = $01;
    toMinute     = $02;
    toSecond     = $04;
    toHundredths = $08;
    toStdTimeL   = toHour + toMinute + toSecond;
    toStdTimeS   = toHour + toMinute;

  { Option flags for DateStr }
    doYear       = $01;
    doMonth      = $02;
    doDay        = $04;
    doDOW        = $08;
    doStdDateL   = doDOW + doYear + doMonth + doDay;
    doStdDateS   = doYear + doMonth + doDay;

type
    pCountry = ^tCountry;
    tCountry = object(tObject)
     datefmt     : Integer;
     constructor Create(Country,CodePage : Word);
     function    DateStr(Options : Word) : string;
     function    TimeStr(Options : Word) : string;
    end;

Implementation

constructor tCountry.Create(Country,CodePage : Word);
{$ifdef OS2}
var cc  : CountryCode;
    len : Longint;
    Info: CountryInfo;
{$endif}
begin
 inherited Create;
{$ifdef OS2}
 cc.Country := Country;
 cc.CodePage := CodePage;
 if DosQueryCtryInfo(sizeOf(Info), cc, Info, len) <> 0 then Fail;
 datefmt := Info.fsDateFmt;
{$else}
 datefmt := 1;
{$endif}
end;

function tCountry.DateStr(Options : Word) : string;
var S  : string[16];
    I  : Integer;
    Year,Month,Day,DayOfWeek:Word;

Procedure DateAdd(optMask, Val, Digits : Longint; Sep : Char);
const
    DOWname : array[0..6] of String[3] = ('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat');
var fst,lst : boolean;
begin
 Delete(S, I, 1);
 if Options and optMask <> 0
  then begin
        if I < length(S) then Insert(Sep, S, I);
        if Digits <> -1
         then Insert(sstr(Val, Digits, '0'), S, I)
         else Insert(DOWname[Val], S, I);
       end;
end;

begin
 GetDate(Year,Month,Day,DayOfWeek);
 case datefmt of
  0 : S := 'wmdy';
  1 : S := 'wdmy';
  2 : S := 'wymd';
  else S := '';
 end;
 For I := length(S) downto 1 do
  case S[I] of
   'y' : DateAdd(doYear,  Year+1980,    0,  '/');
   'm' : DateAdd(doMonth, Month,   2,  '/');
   'd' : DateAdd(doDay,   Day,     2,  '/');
   'w' : DateAdd(doDOW,   DayOfWeek, -1, ' ');
  end;
 DateStr := S;
end;

function tCountry.TimeStr(Options : Word) : string;
var I,V,W : Longint;
    S     : String[16];
    sep   : Char;
    Hour,Minute,Second,Sec100:Word;
begin
 GetTime(Hour,Minute,Second,Sec100);
 S := '';
 For I := 1 to 4 do
  begin
   Sep := ':'; W := 2;
   case I of
    1 : if Options and toHour <> 0
         then V := Hour
         else break;
    2 : if Options and toMinute <> 0
         then V := Minute
         else break;
    3 : if Options and toSecond <> 0
         then V := Second
         else break;
    4 : if Options and toHundredths <> 0
         then begin
               V := Sec100;
               Sep := '.'; W := 3;
              end
         else break;
   end;
   if S <> '' then Insert(Sep, S, succ(length(S)));
   S := S + sStr(V, W, '0');
  end;
 TimeStr := S;
end;

end.
