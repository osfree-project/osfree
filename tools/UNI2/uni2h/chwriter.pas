unit CHWriter;

interface

uses
  Classes, SysUtils, PasTree, ABIReader, LayoutParser;

procedure WriteCHeaders(Module: TPasModule; ABIData: TABIData; Layout: TLayout;
  const DefaultOutput: string; H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean);
procedure WriteSingleCHeader(Module: TPasModule; ABIData: TABIData; Layout: TLayout;
  const OutputFile: string; H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean);

implementation

const
  BannerWidth = 76;
  Prefix: string = '* ';
  Suffix: string = ' *';

type
  TCHWriter = class
  private
    FStream: TStream;
    FLayout: TLayout;
    FABIData: TABIData;
    FH2IncCompat: Boolean;
    FNoCommon: Boolean;
    FCppWrapping: Boolean;
    FIBMWrapping: Boolean;
    FNoChildFiles: Boolean;
    procedure w(const s: string);
    procedure wln(const s: string);
    procedure wln;
    procedure WriteBannerLine(const Text: string; Indent: Integer = 0);
    procedure WriteModuleHeader(const GroupName, Description: string; RootElement: TPasElement);
    procedure WriteMacroList(RootElement: TPasElement);
    procedure WriteProlog(const FileBaseName, GroupName: string);
    procedure WriteEpilog;
    procedure WriteComment(Comment: TPasComment);
    procedure ProcessGroupContents(Group: TPasElement; OwnFile: Boolean);
    procedure ProcessGroup(Group: TPasElement; const CurrentFileName: string);
    procedure ProcessElement(El: TPasElement);
    procedure WriteType(AType: TPasType);
    procedure WriteStructure(Stru: TPasRecordType);
    procedure WriteFunction(AFunc: TPasProcedureBase);
    procedure WriteVariable(AVar: TPasVariable);
    procedure WriteConstant(AConst: TPasConst);
    function GetIncludedMacro(const GroupName: string; const Entry: TLayoutEntry): string;
    function GetGuardName(const BaseName: string; const Entry: TLayoutEntry): string;
    function GetCascadeMacro(const Entry: TLayoutEntry; const GroupName: string): string;
    function GetIncludeMacro(const Entry: TLayoutEntry; const GroupName: string): string;
    procedure GenerateForGroup(Group: TPasElement; const OutputFileName: string);
    procedure GenerateSingleFile(Module: TPasModule; const OutputFileName: string);
  public
    constructor Create(AStream: TStream; AData: TABIData; ALayout: TLayout;
      AH2IncCompat, ANoCommon, ACppWrapping, AIBMWrapping: Boolean;
      ANoChildFiles: Boolean = False);
    procedure Generate(Module: TPasModule; const OutputFileName: string);
  end;

function PrimitiveToC(const Name: string): string;
begin
  if Name = 'int8' then Result := 'char'
  else if Name = 'uint8' then Result := 'unsigned char'
  else if Name = 'int16' then Result := 'short'
  else if Name = 'uint16' then Result := 'unsigned short'
  else if Name = 'int32' then Result := 'long'
  else if Name = 'uint32' then Result := 'unsigned long'
  else if Name = 'int64' then Result := 'long long'
  else if Name = 'uint64' then Result := 'unsigned long long'
  else if Name = 'float' then Result := 'float'
  else if Name = 'double' then Result := 'double'
  else if Name = 'char' then Result := 'char'
  else if Name = 'bool' then Result := '_Bool'
  else if Name = 'pointer' then Result := 'void *'
  else Result := Name;
end;

constructor TCHWriter.Create(AStream: TStream; AData: TABIData; ALayout: TLayout;
  AH2IncCompat, ANoCommon, ACppWrapping, AIBMWrapping: Boolean;
  ANoChildFiles: Boolean = False);
begin
  inherited Create;
  FStream := AStream;
  FABIData := AData;
  FLayout := ALayout;
  FH2IncCompat := AH2IncCompat;
  FNoCommon := ANoCommon;
  FCppWrapping := ACppWrapping;
  FIBMWrapping := AIBMWrapping;
  FNoChildFiles := ANoChildFiles;
end;

procedure TCHWriter.w(const s: string);
begin
  if (Length(s) > 0) and Assigned(FStream) then
    FStream.Write(s[1], Length(s));
end;

procedure TCHWriter.wln(const s: string);
begin
  w(s);
  wln;
end;

procedure TCHWriter.wln;
const
  LF: string = #10;
begin
  if Assigned(FStream) then
    FStream.Write(LF[1], 1);
end;

procedure TCHWriter.WriteComment(Comment: TPasComment);
begin
  wln('/* ' + TrimLeft(Comment.Name) + ' */');
end;

procedure TCHWriter.WriteBannerLine(const Text: string; Indent: Integer = 0);
var
  MaxTextLen: Integer;
  Words: TStringList;
  i: Integer;
  CurLine: string;
begin
  MaxTextLen := BannerWidth - Length(Prefix) - Length(Suffix) - Indent;
  if Length(Text) <= MaxTextLen then
  begin
    wln(Prefix + StringOfChar(' ', Indent) + Text +
        StringOfChar(' ', MaxTextLen - Length(Text)) + Suffix);
  end
  else
  begin
    Words := TStringList.Create;
    try
      Words.DelimitedText := Text;
      CurLine := '';
      for i := 0 to Words.Count - 1 do
      begin
        if CurLine = '' then
          CurLine := Words[i]
        else if Length(CurLine) + 1 + Length(Words[i]) <= MaxTextLen then
          CurLine := CurLine + ' ' + Words[i]
        else
        begin
          wln(Prefix + StringOfChar(' ', Indent) + CurLine +
              StringOfChar(' ', MaxTextLen - Length(CurLine)) + Suffix);
          CurLine := Words[i];
        end;
      end;
      if CurLine <> '' then
        wln(Prefix + StringOfChar(' ', Indent) + CurLine +
            StringOfChar(' ', MaxTextLen - Length(CurLine)) + Suffix);
    finally
      Words.Free;
    end;
  end;
end;

procedure TCHWriter.WriteModuleHeader(const GroupName, Description: string; RootElement: TPasElement);
begin
  wln('/' + StringOfChar('*', BannerWidth) + '\');
  WriteBannerLine('');
  WriteBannerLine('Copyright (c) 2004-2010, 2026 osFree project');
  WriteBannerLine('');
  WriteBannerLine('Module Name: ' + UpperCase(GroupName) + '.H');
  WriteBannerLine('');

  if Description <> '' then
  begin
    WriteBannerLine(Description);
    WriteBannerLine('');
  end;

  WriteMacroList(RootElement);

  WriteBannerLine('Generated: ' + FormatDateTime('yyyy-mm-dd hh:nn:ss', Now));
  WriteBannerLine('');

  WriteBannerLine('WARNING! Automaticaly generated file! Don''t edit it manually!');
  wln('\' + StringOfChar('*', BannerWidth) + '/');
  wln;
end;

procedure TCHWriter.WriteMacroList(RootElement: TPasElement);
var
  Section: TPasSection;
  i: Integer;
  El: TPasElement;
  MacroName, Desc: string;
  LayoutEntry: TLayoutEntry;
  Lines: TStringList;
  MaxLen: Integer;
  LineText: string;
begin
  if not Assigned(RootElement) then
    Exit;

  if RootElement is TPasModule then
    Section := TPasModule(RootElement).InterfaceSection
  else if RootElement is TPasGroup then
    Section := TPasGroup(RootElement).InterfaceSection
  else
    Exit;

  if not Assigned(Section) or (Section.Declarations.Count = 0) then
    Exit;

  Lines := TStringList.Create;
  try
    if not (RootElement is TPasModule) then
    begin
      LayoutEntry := FLayout.FindByGroup(RootElement.Name);
      if LayoutEntry <> nil then
      begin
        if LayoutEntry.HasMacro or (LayoutEntry.CascadeMacro <> '') then
        begin
          if LayoutEntry.HasMacro then
            MacroName := GetIncludeMacro(LayoutEntry, RootElement.Name)
          else
            MacroName := GetCascadeMacro(LayoutEntry, RootElement.Name);
          Desc := LayoutEntry.Description;
          if Desc <> '' then
          begin
            if LayoutEntry.HasMacro and LayoutEntry.InvertMacro then
              Desc := Desc + ' - excluded if symbol defined'
            else if LayoutEntry.HasMacro then
              Desc := Desc + ' - only included if symbol defined';
            Lines.Add(MacroName + #9 + Desc);
          end;
        end;
      end;
    end;

    for i := 0 to Section.Declarations.Count - 1 do
    begin
      El := TPasElement(Section.Declarations[i]);
      if El is TPasGroup then
      begin
        LayoutEntry := FLayout.FindByGroup(El.Name);
        if LayoutEntry = nil then Continue;
        if not (LayoutEntry.HasMacro or (LayoutEntry.CascadeMacro <> '')) then Continue;
        if LayoutEntry.HasMacro then
          MacroName := GetIncludeMacro(LayoutEntry, El.Name)
        else
          MacroName := GetCascadeMacro(LayoutEntry, El.Name);
        Desc := LayoutEntry.Description;
        if Desc <> '' then
        begin
          if LayoutEntry.HasMacro and LayoutEntry.InvertMacro then
            Desc := Desc + ' - excluded if symbol defined'
          else if LayoutEntry.HasMacro then
            Desc := Desc + ' - only included if symbol defined';
          Lines.Add(MacroName + #9 + Desc);
        end;
      end;
    end;

    if Lines.Count = 0 then
      Exit;

    MaxLen := 0;
    for i := 0 to Lines.Count - 1 do
    begin
      MacroName := Copy(Lines[i], 1, Pos(#9, Lines[i]) - 1);
      if Length(MacroName) > MaxLen then
        MaxLen := Length(MacroName);
    end;

    WriteBannerLine('');
    WriteBannerLine('The following symbols are used in this file for conditional sections.');
    WriteBannerLine('');

    for i := 0 to Lines.Count - 1 do
    begin
      MacroName := Copy(Lines[i], 1, Pos(#9, Lines[i]) - 1);
      Desc := Copy(Lines[i], Pos(#9, Lines[i]) + 1, MaxInt);
      LineText := MacroName + StringOfChar(' ', MaxLen - Length(MacroName) + 3) + '-  ' + Desc;
      WriteBannerLine(LineText, 2);
    end;

    WriteBannerLine('');
  finally
    Lines.Free;
  end;
end;

function TCHWriter.GetIncludedMacro(const GroupName: string; const Entry: TLayoutEntry): string;
begin
  if (Entry <> nil) and Entry.HasIncluded then
  begin
    if Entry.IncludedMacro <> '' then
      Result := Entry.IncludedMacro
    else
      Result := 'INCL_' + UpperCase(GroupName) + 'INCLUDED';
  end
  else
    Result := '';
end;

function TCHWriter.GetGuardName(const BaseName: string; const Entry: TLayoutEntry): string;
begin
  if (Entry <> nil) and Entry.HasGuard then
  begin
    if Entry.GuardName <> '' then
      Result := Entry.GuardName
    else
      Result := '__' + UpperCase(BaseName) + '__';
  end
  else
    Result := '';
end;

function TCHWriter.GetCascadeMacro(const Entry: TLayoutEntry; const GroupName: string): string;
begin
  if Entry.CascadeMacro <> '' then
    Result := Entry.CascadeMacro
  else if Entry.MacroName <> '' then
    Result := Entry.MacroName
  else
    Result := 'INCL_' + UpperCase(GroupName);
end;

function TCHWriter.GetIncludeMacro(const Entry: TLayoutEntry; const GroupName: string): string;
begin
  if Entry.MacroName <> '' then
    Result := Entry.MacroName
  else
    Result := 'INCL_' + UpperCase(GroupName);
end;

procedure TCHWriter.WriteProlog(const FileBaseName, GroupName: string);
begin
  if FH2IncCompat then wln('/* NOINC */');

  if FIBMWrapping then
  begin
    wln('#if __IBMC__ || __IBMCPP__');
    wln('   #pragma info( none )');
    wln('      #ifndef __CHKHDR__');
    wln('         #pragma info( none )');
    wln('      #endif');
    wln('   #pragma info( restore )');
    wln('#endif');
  end;

  if FCppWrapping then
  begin
    wln('#ifdef __cplusplus');
    wln('      extern "C" {');
    wln('#endif');
  end;

  if FH2IncCompat then wln('/* INC */');
end;

procedure TCHWriter.WriteEpilog;
begin
  if FH2IncCompat then wln('/* NOINC */');

  if FCppWrapping then
  begin
    wln('#ifdef __cplusplus');
    wln('        }');
    wln('#endif');
  end;

  if FIBMWrapping then
  begin
    wln('#if __IBMC__ || __IBMCPP__');
    wln('   #pragma info( none )');
    wln('      #ifndef __CHKHDR__');
    wln('         #pragma info( restore )');
    wln('      #endif');
    wln('   #pragma info( restore )');
    wln('#endif');
  end;

  if FH2IncCompat then wln('/* INC */');
end;

procedure TCHWriter.ProcessGroupContents(Group: TPasElement; OwnFile: Boolean);
var
  Section: TPasSection;
  i: Integer;
  El: TPasElement;
  IncludeMacroName, CascadeMacro: string;
  LayoutEntry: TLayoutEntry;
  IsRoot: Boolean;
  HasCascadeGroups: Boolean;
  HasChildWithOutput: Boolean;
  HasOwnDecl: Boolean;
begin
  WriteLn('DEBUG: ProcessGroupContents: Group=', Group.Name, ' OwnFile=', OwnFile);

  IncludeMacroName := 'INCL_' + UpperCase(Group.Name);
  if Assigned(FLayout) then
  begin
    LayoutEntry := FLayout.FindByGroup(Group.Name);
    if LayoutEntry <> nil then
      if LayoutEntry.HasMacro then
        IncludeMacroName := GetIncludeMacro(LayoutEntry, Group.Name);
  end;

  IsRoot := (Group is TPasModule) and OwnFile;

  if Group is TPasModule then
    Section := TPasModule(Group).InterfaceSection
  else
    Section := TPasGroup(Group).InterfaceSection;

  HasCascadeGroups := False;
  HasChildWithOutput := False;
  HasOwnDecl := False;

  if Assigned(Section) then
  begin
    WriteLn('DEBUG: Section.Declarations.Count=', Section.Declarations.Count);
    for i := 0 to Section.Declarations.Count - 1 do
    begin
      El := TPasElement(Section.Declarations[i]);
      WriteLn('DEBUG: Decl[', i, '] Class=', El.ClassName, ' Name=', El.Name);
      if El is TPasGroup then
      begin
        HasChildWithOutput := True;
        LayoutEntry := nil;
        if Assigned(FLayout) then
          LayoutEntry := FLayout.FindByGroup(El.Name);
        if not IsRoot and (LayoutEntry <> nil) and (LayoutEntry.CascadeMacro <> '') then
          HasCascadeGroups := True;
      end
      else
        HasOwnDecl := True;
    end;
  end;

  if not (HasCascadeGroups or HasChildWithOutput or HasOwnDecl) then
    Exit;

  if not OwnFile then
    wln('#ifdef ' + IncludeMacroName);

  if Assigned(Section) then
  begin
    if HasCascadeGroups then
    begin
      LayoutEntry := nil;
      if Assigned(FLayout) then
        LayoutEntry := FLayout.FindByGroup(Group.Name);
      if LayoutEntry <> nil then
        CascadeMacro := GetCascadeMacro(LayoutEntry, Group.Name)
      else
        CascadeMacro := IncludeMacroName;

      wln('/* if ' + CascadeMacro + ' defined then define all the symbols */');
      wln('#ifdef ' + CascadeMacro);
      for i := 0 to Section.Declarations.Count - 1 do
      begin
        El := TPasElement(Section.Declarations[i]);
        if El is TPasGroup then
        begin
          LayoutEntry := FLayout.FindByGroup(El.Name);
          if LayoutEntry.CascadeMacro <> '' then
            wln('   #define ' + GetCascadeMacro(LayoutEntry, El.Name));
        end;
      end;
      wln('#endif /* ' + CascadeMacro + ' */');
      wln;
    end;

    for i := 0 to Section.Declarations.Count - 1 do
    begin
      El := TPasElement(Section.Declarations[i]);
      if El is TPasGroup then
        ProcessGroup(El, '');
    end;

    if HasOwnDecl then
    begin
      if OwnFile and not IsRoot then
      begin
        LayoutEntry := nil;
        if Assigned(FLayout) then
          LayoutEntry := FLayout.FindByGroup(Group.Name);
        if (LayoutEntry = nil) or (not LayoutEntry.HasMacro) then
          wln('#ifdef ' + IncludeMacroName);
      end;

      if FNoCommon then
        wln('#if (defined(' + IncludeMacroName + ') || !defined(INCL_NOCOMMON))');

      for i := 0 to Section.Declarations.Count - 1 do
      begin
        El := TPasElement(Section.Declarations[i]);
        if not (El is TPasGroup) then
          ProcessElement(El);
      end;

      if FNoCommon then
        wln('#endif /* common */');

      if OwnFile and not IsRoot then
      begin
        LayoutEntry := nil;
        if Assigned(FLayout) then
          LayoutEntry := FLayout.FindByGroup(Group.Name);
        if (LayoutEntry = nil) or (not LayoutEntry.HasMacro) then
          wln('#endif /* ' + IncludeMacroName + ' */');
      end;
    end;
  end;

  if not OwnFile then
    wln('#endif /* ' + IncludeMacroName + ' */');
end;

procedure TCHWriter.ProcessGroup(Group: TPasElement; const CurrentFileName: string);
var
  LayoutEntry: TLayoutEntry;
  NewFileName: string;
  ChildStream: TFileStream;
  ChildWriter: TCHWriter;
  IncludeMacroName, Description, BaseName, GuardName, IncMacro: string;
  Invert: Boolean;
begin
  WriteLn('DEBUG: ProcessGroup: Group=', Group.Name);

  IncludeMacroName := 'INCL_' + UpperCase(Group.Name);
  NewFileName := '';
  LayoutEntry := nil;
  Description := '';
  GuardName := '';
  IncMacro := '';
  Invert := False;
  if Assigned(FLayout) then
  begin
    LayoutEntry := FLayout.FindByGroup(Group.Name);
    if LayoutEntry <> nil then
    begin
      if LayoutEntry.OutputFile <> '' then
        NewFileName := LayoutEntry.OutputFile;
      Description := LayoutEntry.Description;
      if LayoutEntry.HasMacro then
      begin
        IncludeMacroName := GetIncludeMacro(LayoutEntry, Group.Name);
        Invert := LayoutEntry.InvertMacro;
      end;
      GuardName := GetGuardName(ChangeFileExt(ExtractFileName(NewFileName), ''), LayoutEntry);
      IncMacro := GetIncludedMacro(Group.Name, LayoutEntry);
    end;
  end;

  if NewFileName <> '' then
  begin
    WriteLn('DEBUG: ProcessGroup: NewFileName=', NewFileName);
    if Description <> '' then
      wln('/* ' + Description + ' */');
    if LayoutEntry <> nil then
    begin
      if LayoutEntry.HasMacro then
      begin
        if Invert then
        begin
          wln('#ifndef ' + IncludeMacroName);
          wln('   #include <' + NewFileName + '>');
          wln('#endif /* ' + IncludeMacroName + ' */');
        end
        else
        begin
          wln('#ifdef ' + IncludeMacroName);
          wln('   #include <' + NewFileName + '>');
          wln('#endif');
        end;
      end
      else
        wln('#include <' + NewFileName + '>');
    end
    else
      wln('#include <' + NewFileName + '>');

    wln;

    if not FNoChildFiles then
    begin
      BaseName := ChangeFileExt(ExtractFileName(NewFileName), '');
      WriteLn('DEBUG: Создание дочернего файла ', NewFileName);
      ChildStream := TFileStream.Create(NewFileName, fmCreate);
      ChildWriter := TCHWriter.Create(ChildStream, FABIData, FLayout,
                                      FH2IncCompat, FNoCommon,
                                      FCppWrapping, FIBMWrapping);
      try
        ChildWriter.GenerateForGroup(Group, NewFileName);
      finally
        ChildWriter.Free;
        ChildStream.Free;
      end;
    end;
  end
  else
    ProcessGroupContents(Group, False);
end;

procedure TCHWriter.GenerateForGroup(Group: TPasElement; const OutputFileName: string);
var
  Desc, BaseName, GuardName, IncMacro: string;
  LayoutEntry: TLayoutEntry;
begin
  WriteLn('DEBUG: GenerateForGroup: Group=', Group.Name, ' Output=', OutputFileName);
  Desc := '';
  GuardName := '';
  IncMacro := '';
  LayoutEntry := FLayout.FindByGroup(Group.Name);
  if LayoutEntry <> nil then
  begin
    Desc := LayoutEntry.Description;
    GuardName := GetGuardName(ChangeFileExt(ExtractFileName(OutputFileName), ''), LayoutEntry);
    IncMacro := GetIncludedMacro(Group.Name, LayoutEntry);
  end;

  BaseName := ChangeFileExt(ExtractFileName(OutputFileName), '');
  WriteModuleHeader(BaseName, Desc, Group);
  WriteProlog(BaseName, Group.Name);

  if GuardName <> '' then
  begin
    wln('#ifndef ' + GuardName);
    if FH2IncCompat then wln('/* NOINC */');
    wln('#define ' + GuardName);
    if FH2IncCompat then wln('/* INC */');
  end;

  if IncMacro <> '' then
  begin
    wln('#define ' + IncMacro);
    wln;
  end;

  ProcessGroupContents(Group, True);

  if GuardName <> '' then
    wln('#endif /* ' + GuardName + ' */');
  wln;
  WriteEpilog;
end;

function FindGroupByName(Element: TPasElement; const GroupName: string): TPasElement;
var
  i: Integer;
  Section: TPasSection;
begin
  Result := nil;
  if Element = nil then Exit;
  if Element is TPasModule then
    Section := TPasModule(Element).InterfaceSection
  else if Element is TPasGroup then
    Section := TPasGroup(Element).InterfaceSection
  else
    Section := nil;

  if Assigned(Section) then
    for i := 0 to Section.Declarations.Count - 1 do
    begin
      Result := TPasElement(Section.Declarations[i]);
      if (Result is TPasGroup) and (CompareText(Result.Name, GroupName) = 0) then
        Exit;
      Result := FindGroupByName(Result, GroupName);
      if Result <> nil then Exit;
    end;
end;

procedure TCHWriter.GenerateSingleFile(Module: TPasModule; const OutputFileName: string);
var
  LayoutEntry: TLayoutEntry;
  Group: TPasElement;
  BaseName: string;
begin
  WriteLn('DEBUG: GenerateSingleFile: Output=', OutputFileName);
  BaseName := ExtractFileName(OutputFileName);
  LayoutEntry := FLayout.FindByOutputFile(BaseName);
  if LayoutEntry = nil then
    raise Exception.CreateFmt('No layout entry for output file %s', [BaseName]);

  if CompareText(LayoutEntry.GroupName, Module.Name) = 0 then
    Group := Module
  else
  begin
    Group := FindGroupByName(Module, LayoutEntry.GroupName);
    if Group = nil then
      raise Exception.CreateFmt('Group %s not found in module', [LayoutEntry.GroupName]);
  end;

  GenerateForGroup(Group, OutputFileName);
end;

procedure TCHWriter.ProcessElement(El: TPasElement);
begin
  WriteLn('DEBUG: ProcessElement: Class=', El.ClassName, ' Name=', El.Name);
  if El is TPasComment then
    WriteComment(TPasComment(El))
  else if El is TPasRecordType then
    WriteStructure(TPasRecordType(El))
  else if El is TPasType then
    WriteType(TPasType(El))
  else if El is TPasFunction then
    WriteFunction(TPasFunction(El))
  else if El is TPasProcedure then
    WriteFunction(TPasProcedure(El))
  else if El is TPasConst then
    WriteConstant(TPasConst(El))
  else if El is TPasVariable then
    WriteVariable(TPasVariable(El));
end;

procedure TCHWriter.WriteType(AType: TPasType);
var
  CType: string;
  RetType: string;
  Convention: string;
  Args: string;
  i: Integer;
  Arg: TPasArgument;
begin
  WriteLn('DEBUG: WriteType: ', AType.ClassName, ' Name=', AType.Name);

  if AType is TPasAliasType then
  begin
    CType := PrimitiveToC(TPasAliasType(AType).DestType.Name);
    wln('typedef ' + CType + ' ' + AType.Name + ';');
  end
  else if AType is TPasPointerType then
  begin
    if Assigned(TPasPointerType(AType).DestType) then
    begin
      // Если указывает на процедурный тип, используем его имя
      if (TPasPointerType(AType).DestType is TPasProcedureType) or
         (TPasPointerType(AType).DestType is TPasFunctionType) then
        CType := TPasPointerType(AType).DestType.Name
      else
        CType := PrimitiveToC(TPasPointerType(AType).DestType.Name);

      if CType = 'void' then
        wln('typedef void *' + AType.Name + ';')
      else
        wln('typedef ' + CType + ' *' + AType.Name + ';');
    end
    else
      wln('typedef void *' + AType.Name + ';');
  end
  else if AType is TPasProcedureType then
  begin
    Convention := '_System';  // позже можно брать из ABI
    Args := 'void';
    if Assigned(TPasProcedureType(AType).Args) and
       (TPasProcedureType(AType).Args.Count > 0) then
    begin
      Args := '';
      for i := 0 to TPasProcedureType(AType).Args.Count - 1 do
      begin
        Arg := TPasArgument(TPasProcedureType(AType).Args[i]);
        if i > 0 then Args := Args + ', ';
        if Assigned(Arg.ArgType) then
        begin
          if Arg.ArgType is TPasPointerType then
            Args := Args + PrimitiveToC(TPasPointerType(Arg.ArgType).DestType.Name) + ' *' + Arg.Name
          else
            Args := Args + PrimitiveToC(Arg.ArgType.Name) + ' ' + Arg.Name;
        end
        else
          Args := Args + 'void ' + Arg.Name;
      end;
    end;
    wln('typedef void (' + Convention + ' ' + AType.Name + ') (' + Args + ');');
  end
  else if AType is TPasFunctionType then
  begin
    Convention := '_System';
    if Assigned(TPasFunctionType(AType).ResultEl) and
       Assigned(TPasFunctionType(AType).ResultEl.ResultType) then
      RetType := PrimitiveToC(TPasFunctionType(AType).ResultEl.ResultType.Name)
    else
      RetType := 'int';
    Args := 'void';
    if Assigned(TPasFunctionType(AType).Args) and
       (TPasFunctionType(AType).Args.Count > 0) then
    begin
      Args := '';
      for i := 0 to TPasFunctionType(AType).Args.Count - 1 do
      begin
        Arg := TPasArgument(TPasFunctionType(AType).Args[i]);
        if i > 0 then Args := Args + ', ';
        if Assigned(Arg.ArgType) then
        begin
          if Arg.ArgType is TPasPointerType then
            Args := Args + PrimitiveToC(TPasPointerType(Arg.ArgType).DestType.Name) + ' *' + Arg.Name
          else
            Args := Args + PrimitiveToC(Arg.ArgType.Name) + ' ' + Arg.Name;
        end
        else
          Args := Args + 'void ' + Arg.Name;
      end;
    end;
    wln('typedef ' + RetType + ' (' + Convention + ' ' + AType.Name + ') (' + Args + ');');
  end;
end;

procedure TCHWriter.WriteStructure(Stru: TPasRecordType);
var
  i: Integer;
  VarEl: TPasVariable;
  TypeName: string;
  StructAttr: TABIStructure;
  IndexRange: string;
  ArraySize: Integer;
  p: Integer;
  PrefixTag: string;
  FieldComment: string;
begin
  WriteLn('DEBUG: WriteStructure: ', Stru.Name);
  StructAttr := nil;
  if Assigned(FABIData) then
    StructAttr := FABIData.FindStructure(Stru.Name);

  if Assigned(StructAttr) and (StructAttr.Pack > 0) then
  begin
    if FH2IncCompat then wln('/* NOINC */');
    wln('#pragma pack(push, ' + IntToStr(StructAttr.Pack) + ')');
    if FH2IncCompat then wln('/* INC */');
  end;

  // Формируем тег префикса
  PrefixTag := '';
  if FH2IncCompat and Assigned(StructAttr) and (StructAttr.Prefix <> '') then
    PrefixTag := ' /* ' + StructAttr.Prefix + ' */';

  // Выводим строку typedef struct с фигурной скобкой в той же строке
  wln('typedef struct _' + Stru.Name + ' {' + PrefixTag);

  for i := 0 to Stru.Members.Count - 1 do
  begin
    VarEl := TPasVariable(Stru.Members[i]);

    WriteLn('DEBUG:   Поле ', i, ': ', VarEl.Name, ' тип=', VarEl.VarType.ClassName, ' имя типа=', VarEl.VarType.Name);

    if Assigned(VarEl.VarType) then
    begin
      FieldComment := '';
      if VarEl.InlineComment <> '' then
        FieldComment := ' /* ' + VarEl.InlineComment + ' */';

      if VarEl.VarType is TPasPointerType then
        TypeName := PrimitiveToC(TPasPointerType(VarEl.VarType).DestType.Name) + ' *'
      else if VarEl.VarType is TPasArrayType then
      begin
        if Assigned(TPasArrayType(VarEl.VarType).ElType) then
          TypeName := PrimitiveToC(TPasArrayType(VarEl.VarType).ElType.Name)
        else
          TypeName := 'void';
        IndexRange := TPasArrayType(VarEl.VarType).IndexRange;
        ArraySize := 1;
        p := Pos('..', IndexRange);
        if p > 0 then
          ArraySize := StrToIntDef(Copy(IndexRange, p + 2, Length(IndexRange) - p - 1), 1) -
                       StrToIntDef(Copy(IndexRange, 1, p - 1), 0) + 1;
        wln('  ' + TypeName + ' ' + VarEl.Name + '[' + IntToStr(ArraySize) + '];' + FieldComment);
        Continue;
      end
      else
        TypeName := PrimitiveToC(VarEl.VarType.Name);

      wln('  ' + TypeName + ' ' + VarEl.Name + ';' + FieldComment);
    end;
  end;

  // Закрывающая строка структуры
  wln('} ' + Stru.Name + ';');

  if Assigned(StructAttr) and (StructAttr.Pack > 0) then
  begin
    if FH2IncCompat then wln('/* NOINC */');
    wln('#pragma pack(pop)');
    if FH2IncCompat then wln('/* INC */');
  end;
end;

procedure TCHWriter.WriteFunction(AFunc: TPasProcedureBase);
var
  ProcType: TPasProcedureType;
  FuncType: TPasFunctionType;
  RetType, Convention, Args, BaseTypeName, TypeName: string;
  i: Integer;
  Arg: TPasArgument;
  Entries: TList;
  IsPointer: Boolean;
begin
  WriteLn('DEBUG: WriteFunction: ', AFunc.Name);
  if AFunc is TPasFunction then
  begin
    FuncType := TPasFunctionType(TPasFunction(AFunc).ProcType);
    RetType := FuncType.ResultEl.ResultType.Name;
    if RetType = '' then
      RetType := FuncType.ResultEl.ResultType.GetDeclaration(False);
    ProcType := FuncType;
  end
  else
  begin
    ProcType := TPasProcedure(AFunc).ProcType;
    RetType := 'void';
  end;

  Convention := '';
  if Assigned(FABIData) then
  begin
    Entries := FABIData.FindEntry(AFunc.Name);
    try
      if Entries.Count > 0 then
        Convention := TABIEntry(Entries[0]).Convention;
    finally
      Entries.Free;
    end;
  end;
  if Convention = '' then
    Convention := '_System';

  Args := '';
  if Assigned(ProcType) and (ProcType.Args.Count > 0) then
  begin
    for i := 0 to ProcType.Args.Count - 1 do
    begin
      Arg := TPasArgument(ProcType.Args[i]);
      if i > 0 then Args := Args + ', ';
      IsPointer := False;
      BaseTypeName := '';
      if Assigned(Arg.ArgType) then
      begin
        if Arg.ArgType is TPasPointerType then
        begin
          IsPointer := True;
          BaseTypeName := TPasPointerType(Arg.ArgType).DestType.Name;
        end
        else
          BaseTypeName := Arg.ArgType.Name;
      end
      else
        BaseTypeName := 'void';
      TypeName := PrimitiveToC(BaseTypeName);
      if IsPointer then
        Args := Args + TypeName + ' *' + Arg.Name
      else
        case Arg.Access of
          argIn:    Args := Args + TypeName + ' ' + Arg.Name;
          argOut,
          argInOut: Args := Args + TypeName + ' *' + Arg.Name;
        end;
    end;
  end
  else
    Args := 'void';

  wln(RetType + ' ' + Convention + ' ' + AFunc.Name + '(' + Args + ');');
end;

procedure TCHWriter.WriteVariable(AVar: TPasVariable);
var
  TypeName: string;
begin
  WriteLn('DEBUG: WriteVariable: ', AVar.Name);
  if Assigned(AVar.VarType) then
  begin
    if AVar.VarType is TPasPointerType then
      TypeName := PrimitiveToC(TPasPointerType(AVar.VarType).DestType.Name) + ' *'
    else
      TypeName := PrimitiveToC(AVar.VarType.Name);
    wln('extern ' + TypeName + ' ' + AVar.Name + ';');
  end;
end;

procedure TCHWriter.WriteConstant(AConst: TPasConst);
begin
  WriteLn('DEBUG: WriteConstant: ', AConst.Name, ' = ', AConst.Value);
  wln('#define ' + AConst.Name + ' ' + AConst.Value);
end;

procedure TCHWriter.Generate(Module: TPasModule; const OutputFileName: string);
var
  Desc, BaseName, GuardName, IncMacro: string;
  LayoutEntry: TLayoutEntry;
begin
  WriteLn('DEBUG: Generate: Output=', OutputFileName);
  Desc := '';
  GuardName := '';
  IncMacro := '';
  LayoutEntry := nil;
  if Assigned(FLayout) then
    LayoutEntry := FLayout.FindByGroup(Module.Name);
  if LayoutEntry <> nil then
  begin
    Desc := LayoutEntry.Description;
    GuardName := GetGuardName(ChangeFileExt(ExtractFileName(OutputFileName), ''), LayoutEntry);
    IncMacro := GetIncludedMacro(Module.Name, LayoutEntry);
  end;

  BaseName := ChangeFileExt(ExtractFileName(OutputFileName), '');
  WriteModuleHeader(BaseName, Desc, Module);
  WriteProlog(BaseName, Module.Name);

  if GuardName <> '' then
  begin
    wln('#ifndef ' + GuardName);
    if FH2IncCompat then wln('/* NOINC */');
    wln('#define ' + GuardName);
    if FH2IncCompat then wln('/* INC */');
  end;

  if IncMacro <> '' then
  begin
    wln('#define ' + IncMacro);
    wln;
  end;

  ProcessGroupContents(Module, True);

  if GuardName <> '' then
    wln('#endif /* ' + GuardName + ' */');
  wln;
  WriteEpilog;
end;

procedure WriteCHeaders(Module: TPasModule; ABIData: TABIData; Layout: TLayout;
  const DefaultOutput: string; H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean);
var
  RootStream: TFileStream;
  Writer: TCHWriter;
  Dir: string;
  TempName: string;
begin
  Dir := ExcludeTrailingPathDelimiter(ExtractFilePath(DefaultOutput));
  if Dir <> '' then
    ForceDirectories(Dir);

  TempName := DefaultOutput + '.tmp';

  // Создаём временный файл
  RootStream := TFileStream.Create(TempName, fmCreate);
  try
    Writer := TCHWriter.Create(RootStream, ABIData, Layout,
                               H2IncCompat, NoCommon, CppWrapping, IBMWrapping);
    try
      Writer.Generate(Module, DefaultOutput);
    finally
      Writer.Free;
    end;
  except
    // При ошибке удаляем временный файл
    RootStream.Free;
    if FileExists(TempName) then
      DeleteFile(TempName);
    raise;
  end;

  RootStream.Free;

  // Удаляем целевой файл, если существует, и переименовываем временный
  if FileExists(DefaultOutput) then
    DeleteFile(DefaultOutput);
  RenameFile(TempName, DefaultOutput);
end;

procedure WriteSingleCHeader(Module: TPasModule; ABIData: TABIData; Layout: TLayout;
  const OutputFile: string; H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean);
var
  Stream: TFileStream;
  Writer: TCHWriter;
  Dir: string;
  TempName: string;
begin
  Dir := ExcludeTrailingPathDelimiter(ExtractFilePath(OutputFile));
  if Dir <> '' then
    ForceDirectories(Dir);

  TempName := OutputFile + '.tmp';

  // Создаём временный файл
  Stream := TFileStream.Create(TempName, fmCreate);
  try
    Writer := TCHWriter.Create(Stream, ABIData, Layout,
                               H2IncCompat, NoCommon, CppWrapping, IBMWrapping,
                               True);
    try
      Writer.GenerateSingleFile(Module, OutputFile);
    finally
      Writer.Free;
    end;
  except
    // При ошибке удаляем временный файл
    Stream.Free;
    if FileExists(TempName) then
      DeleteFile(TempName);
    raise;
  end;

  Stream.Free;

  // Удаляем целевой файл, если существует, и переименовываем временный
  if FileExists(OutputFile) then
    DeleteFile(OutputFile);
  RenameFile(TempName, OutputFile);
end;

end.
