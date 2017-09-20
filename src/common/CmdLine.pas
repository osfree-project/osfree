Unit CmdLine;

Interface

type
{ Parameter handling function definition }
 ParmHandleFn = Function(var Parm : string) : Byte;

{Command line parsing helper. PH gets called on each /# and -# parameter;}
{otherwise NH is called; they return number of chars starting from string}
{start which have to be stripped (i.e. when they were already recognized)}
{If ParmStr is #0 uses the real command line; otherwise ParmStr is used  }
 Procedure ParseCommandLine(const ParmStr : string; PH,NH : ParmHandleFn);
{ParseCommandLine additional helper to parse filenames and other stuff which}
{can contain spaces. To specify an parameter (i.e. an filename) containing  }
{spaces use " double quote " symbol. Retrieves the length of parsed string; }
{however if parameter contains quotes they are removed from DestStr         }
 Function  ParseName(var ParmStr : string; StartChar : Word; var DestStr : string) : Word;

Implementation uses Strings, strOp;

Procedure ParseCommandLine;
var S : String;
begin
 if ParmStr = #0
  then {$ifDef VIRTUALPASCAL}
       S := StrPas(GetASCIIZptr(System.CmdLine^, 2))
       {$else}
       Move(mem[PrefixSeg:$80], S, succ(mem[PrefixSeg:$80]))
       {$endIf}
  else S := ParmStr;
 While S <> '' do
  begin
   While (S <> '') and ((S[1] = ' ') or (S[1] = #9)) do Delete(S, 1, 1);
   if S <> ''
    then
     if (S[1] in ['/','-'])
      then begin
            Delete(S, 1, 1);
            if (@PH <> NIL) and (S <> '') then Delete(S, 1, PH(S));
           end
      else
     if @NH <> NIL
      then Delete(S, 1, NH(S))
      else Delete(S, 1, 1);
  end;
end;

Function ParseName;
var I,J : Word;
    fCh : Char;
begin
 I := StartChar;
 While (I < length(ParmStr)) and (ParmStr[I] in [#9, ' ']) do Inc(I);
 if I <= length(ParmStr)
  then begin
        J := I;
        if ParmStr[I] = '"'
         then fCh := '"'
         else fCh := ' ';
        repeat
         Inc(I);
        until (I > length(ParmStr)) or (ParmStr[I] = fCh);
        if fCh = '"'
         then begin DestStr := Copy(ParmStr, succ(J), pred(I - J)); Inc(I); end
         else DestStr := Copy(ParmStr, J, I - J);
       end
  else DestStr := '';
 ParseName := I - StartChar;
end;

end.
