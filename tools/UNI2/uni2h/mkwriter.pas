{**
  @file MKWriter.pas
  @brief Makefile emitter for import libraries.

  Generates makefile.mk containing the ADD_LIBOPT variable,
  which is passed to wlib to create an import library from
  the entries of a library in .abi.
}
unit MKWriter;

interface

uses Classes, SysUtils, ABIReader;

procedure WriteMakefile(ABIData: TABIData; const LibraryName: string);

implementation

const
  ContinuationIndent = 13;   // длина "ADD_LIBOPT = "

type
  TImportRec = record
    HasOrdinal: Boolean;
    Ordinal: Integer;
    SymName: string;
    Line: string;
  end;
  PImportRec = ^TImportRec;

function CompareImports(a, b: Pointer): Integer;
var
  Ra, Rb: PImportRec;
begin
  Ra := PImportRec(a);
  Rb := PImportRec(b);

  if Ra^.HasOrdinal <> Rb^.HasOrdinal then
  begin
    if not Ra^.HasOrdinal then
      Result := -1
    else
      Result := 1;
    Exit;
  end;

  if Ra^.HasOrdinal then
  begin
    if Ra^.Ordinal < Rb^.Ordinal then
      Result := -1
    else if Ra^.Ordinal > Rb^.Ordinal then
      Result := 1
    else
      Result := 0;
  end
  else
    Result := CompareText(Ra^.SymName, Rb^.SymName);
end;

procedure CollectImports(ABIData: TABIData; Lib: TABIModule; List: TList);
var
  i, j, k, MatchCount: Integer;
  E: TABIEntry;
  M: TABIModule;
  ME: TABIEntry;
  Match: TABIEntry;
  MatchModule: TABIModule;
  Rec: PImportRec;
  Line: string;
begin
  for i := 0 to Lib.Entries.Count - 1 do
  begin
    E := TABIEntry(Lib.Entries[i]);

    if E.IsAlias then
      Continue;

    if E.LogicalName = '' then
      raise Exception.CreateFmt(
        'Library "%s": entry with empty logical name', [Lib.Name]);

    Match := nil;
    MatchModule := nil;
    MatchCount := 0;

    for j := 0 to ABIData.Modules.Count - 1 do
    begin
      M := TABIModule(ABIData.Modules[j]);
      if M.ModuleType <> mtDLL then
        Continue;
      for k := 0 to M.Entries.Count - 1 do
      begin
        ME := TABIEntry(M.Entries[k]);
        if SameText(ME.LogicalName, E.LogicalName) and
           SameText(ME.Convention, E.Convention) then
        begin
          Inc(MatchCount);
          if Match = nil then
          begin
            Match := ME;
            MatchModule := M;
          end;
        end;
      end;
    end;

    if MatchCount = 0 then
      raise Exception.CreateFmt(
        'Entry "%s" (%s): no matching symbol in modules',
        [E.LogicalName, E.Convention]);

    if MatchCount > 1 then
      raise Exception.CreateFmt(
        'Entry "%s" (%s): ambiguous, %d matches in modules',
        [E.LogicalName, E.Convention, MatchCount]);

    Line := '++' + Match.LogicalName + '.' + MatchModule.Name;
    if Match.RealName <> '' then
      Line := Line + '.' + Match.RealName;
    if Match.Number <> 0 then
      Line := Line + '.' + IntToStr(Match.Number);

    New(Rec);
    Rec^.HasOrdinal := Match.Number <> 0;
    Rec^.Ordinal := Match.Number;
    Rec^.SymName := Match.LogicalName;
    Rec^.Line := Line;
    List.Add(Rec);
  end;
end;

procedure WriteMakefile(ABIData: TABIData; const LibraryName: string);
var
  Lib: TABIModule;
  Imports: TList;
  Stream: TFileStream;
  TempName: string;
  i, Count: Integer;
  M: TABIModule;
  Line, Pad: string;
begin
  Lib := nil;
  Count := 0;
  for i := 0 to ABIData.Modules.Count - 1 do
  begin
    M := TABIModule(ABIData.Modules[i]);
    if (M.ModuleType = mtLibrary) and SameText(M.Name, LibraryName) then
    begin
      Lib := M;
      Inc(Count);
    end;
  end;
  if Count = 0 then
    raise Exception.CreateFmt('Library "%s" not found in .abi', [LibraryName]);
  if Count > 1 then
    raise Exception.CreateFmt('Duplicate library "%s" in .abi', [LibraryName]);

  Imports := TList.Create;
  try
    CollectImports(ABIData, Lib, Imports);
    if Imports.Count > 0 then
      Imports.Sort(@CompareImports);

    TempName := 'makefile.mk.tmp';
    Stream := TFileStream.Create(TempName, fmCreate);
    try
      Pad := StringOfChar(' ', ContinuationIndent);

      Line := 'ADD_LIBOPT = ';
      Stream.Write(Line[1], Length(Line));

      for i := 0 to Imports.Count - 1 do
      begin
        Line := PImportRec(Imports[i])^.Line;
        if i > 0 then
        begin
          Line := ' &'#10 + Pad;
          Stream.Write(Line[1], Length(Line));
        end;
        Line := PImportRec(Imports[i])^.Line;
        Stream.Write(Line[1], Length(Line));
      end;

      Line := #10;
      Stream.Write(Line[1], 1);
    except
      Stream.Free;
      if FileExists(TempName) then
        DeleteFile(TempName);
      raise;
    end;
    Stream.Free;

    if FileExists('makefile.mk') then
      DeleteFile('makefile.mk');
    RenameFile(TempName, 'makefile.mk');
  finally
    for i := 0 to Imports.Count - 1 do
      Dispose(PImportRec(Imports[i]));
    Imports.Free;
  end;
end;

end.
