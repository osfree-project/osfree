Program MapSym;

Uses
  Sym;

{
/A Omits Alphabetical sorting of symbols.

/N Includes source code line Numbers in *.SYM file.

/L Produces verbose Listing.
}

Var
  SymbolFile: TSymFile;

Begin
  SymbolFile:=TSymFile.Create('jpegio.sym');
End.
