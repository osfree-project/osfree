{$AlignCode-,AlignData-,AlignRec-,G3+,Speed-,Frame-,Use32+}
unit Country;

Interface uses os2def, os2base, miscUtil, strOp;

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
     Info        : CountryInfo;
     constructor Create(Country,CodePage : Word);
     function    DateStr(Options : Word) : string;
     function    TimeStr(Options : Word) : string;
    end;

Implementation

constructor tCountry.Create;
var cc  : CountryCode;
    len : Longint;
begin
 inherited Create;
 cc.Country := Country;
 cc.CodePage := CodePage;
 if DosQueryCtryInfo(sizeOf(Info), cc, Info, len) <> 0 then Fail;
end;

function tCountry.DateStr;
var dt : DateTime;
    S  : string[16];
    I  : Integer;

Procedure DateAdd(optMask, Val, Digits : Longint; Sep : Char);
const
    DOWname : array[0..6] of String[3] = ('Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun');
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
 DosGetDateTime(dt);
 case Info.fsDateFmt of
  0 : S := 'wmdy';
  1 : S := 'wdmy';
  2 : S := 'wymd';
  else S := '';
 end;
 For I := length(S) downto 1 do
  case S[I] of
   'y' : DateAdd(doYear,  dt.Year,    0,  '/');
   'm' : DateAdd(doMonth, dt.Month,   2,  '/');
   'd' : DateAdd(doDay,   dt.Day,     2,  '/');
   'w' : DateAdd(doDOW,   dt.WeekDay, -1, ' ');
  end;
 DateStr := S;
end;

function tCountry.TimeStr;
var dt    : DateTime;
    I,V,W : Longint;
    S     : String[16];
    sep   : Char;
begin
 DosGetDateTime(dt);
 S := '';
 For I := 1 to 4 do
  begin
   Sep := ':'; W := 2;
   case I of
    1 : if Options and toHour <> 0
         then V := dt.Hours
         else break;
    2 : if Options and toMinute <> 0
         then V := dt.Minutes
         else break;
    3 : if Options and toSecond <> 0
         then V := dt.Seconds
         else break;
    4 : if Options and toHundredths <> 0
         then begin
               V := dt.Hundredths;
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

