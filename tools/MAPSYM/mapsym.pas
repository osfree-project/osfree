Program MapSym;

Uses
  Sym,
  SysUtils;

{
/A Omits Alphabetical sorting of symbols.

/N Includes source code line Numbers in *.SYM file.

/L Produces verbose Listing.

/V View content of *.SYM file.
}

Var
  SymbolFile: TSymFile;
  I: Word;
  J: Word;
Begin
  Exit; // While not finished, just exit
  SymbolFile:=TSymFile.Create(ParamStr(1));
  WriteLn;
  WriteLn(' ', SymbolFile.ModuleName);
  WriteLn;
//  WriteLn('Number of constants: ', SymbolFile.ConstantsCount);
//  WriteLn('Number of segments: ', SymbolFile.SegmentsCount);

//  If SymbolFile.ConstantsCount>0 then
//  begin
//    WriteLn('Constants:');
//    For I:=0 to SymbolFile.ConstantsCount-1 do
//      WriteLn(SymbolFile.Constants[I].Name,'=',SymbolFile.Constants[I].Value);
//  end;

//  WriteLn;
  WriteLn('  Address         Publics by Value');
  WriteLn;

  If SymbolFile.SegmentsCount>0 then
  begin
    For I:=0 to SymbolFile.SegmentsCount-1 do
    begin
//      WriteLn('Segment: ',SymbolFile.Segments[I].Name);
//      WriteLn;
      For J:=0 to SymbolFile.Segments[I].SymbolsCount-1 do
      begin
        WriteLn(' ', IntToHex(SymbolFile.Segments[I].Number, 4), ':', IntToHex(SymbolFile.Segments[I].Symbols[J].Value,8), '       ', SymbolFile.Segments[I].Symbols[J].Name);
      end;
//      WriteLn;
    end;
  end;

End.
