{**
  @file CHWriter.pas
  @brief C header emitter.

  Generates C headers from a PasTree module, ABI data and a layout.
  Supports syscall functions with interrupt, service and call conventions
  through #pragma aux directives.
}
unit CHWriter;

interface

uses
  Classes, SysUtils, PasTree, ABIReader, LayoutParser;

{**
  @brief Write C headers for all groups of a module.
  @param Module Parsed .uni module.
  @param ABIData Parsed .abi data.
  @param Layout Parsed layout.
  @param DefaultOutput Default output file name.
  @param H2IncCompat Emit NOINC / INC markers.
  @param NoCommon Emit INCL_NOCOMMON checks.
  @param CppWrapping Wrap with extern "C".
  @param IBMWrapping Wrap with IBM C/C++ pragmas.
}
procedure WriteCHeaders(Module: TPasModule; ABIData: TABIData; Layout: TLayout;
  const DefaultOutput: string; H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean);

{**
  @brief Write a single C header for a selected group.
  @param Module Parsed .uni module.
  @param ABIData Parsed .abi data.
  @param Layout Parsed layout.
  @param OutputFile Target file name.
  @param H2IncCompat Emit NOINC / INC markers.
  @param NoCommon Emit INCL_NOCOMMON checks.
  @param CppWrapping Wrap with extern "C".
  @param IBMWrapping Wrap with IBM C/C++ pragmas.
}
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
    procedure WriteSyscallPragma(AFunc: TPasProcedureBase;
      ProcType: TPasProcedureType; const RetType: string; Syscall: TABISyscall);
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

{**
  @brief Map a .uni primitive type to a C type.
  @param Name Primitive type name.
  @return C type name.
}
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
  else if SameText(Name, 'VOID') then Result := 'void'
  else if Name = 'pointer' then Result := 'void *'
  else Result := Name;
end;

{**
  @brief Convert an .abi numeric token into the assembler form.

  "$21" becomes "21h"; "-1" stays "-1"; a decimal token stays as is.
}
function ConvertAsmNumber(const S: string): string;
begin
  if S = '' then
    Result := ''
  else if S[1] = '$' then
    Result := Copy(S, 2, MaxInt) + 'h'
  else
    Result := S;
end;

{**
  @brief Convert a far address token "$SSSS:$OOOO" into "SSSSh:OOOOh".
}
function ConvertFarAddr(const S: string): string;
var
  p: Integer;
begin
  p := Pos(':', S);
  if p = 0 then
    Result := ConvertAsmNumber(S)
  else
    Result := ConvertAsmNumber(Copy(S, 1, p - 1)) + ':' +
              ConvertAsmNumber(Copy(S, p + 1, MaxInt));
end;

{**
  @brief Split a register or register pair into individual names.
  @param Regs Register name or pair ("ah", "ds:dx").
  @param List Output list receiving one or two names.
}
procedure SplitRegs(const Regs: string; List: TStringList);
var
  p: Integer;
  S: string;
begin
  S := Regs;
  while S <> '' do
  begin
    p := Pos(':', S);
    if p = 0 then
    begin
      List.Add(S);
      Break;
    end
    else
    begin
      List.Add(Copy(S, 1, p - 1));
      Delete(S, 1, p);
    end;
  end;
end;

{**
  @brief Parse an 8-bit decimal or hexadecimal token.
  @param S Token from the .abi file.
  @param N Output numeric value.
  @return True if the token is a scalar in the range 0..255.
}
function ParseByteNumber(const S: string; out N: Integer): Boolean;
var
  V: Integer;
begin
  Result := False;
  N := 0;
  if S = '' then Exit;
  if S[1] = '$' then
    V := StrToIntDef('$' + Copy(S, 2, MaxInt), -1)
  else
    V := StrToIntDef(S, -1);
  if (V >= 0) and (V <= 255) then
  begin
    N := V;
    Result := True;
  end;
end;

{**
  @brief Check whether a type is a pointer, resolving alias chains.

  A type is treated as a pointer if it is TPasPointerType, if it is an
  alias of a pointer, or if its name is VOID (untyped pointer).
}
function IsPointerType(AType: TPasType): Boolean;
begin
  Result := False;
  if AType = nil then Exit;
  if SameText(AType.Name, 'VOID') then
    Result := True
  else if AType is TPasPointerType then
    Result := True
  else if AType is TPasAliasType then
    Result := IsPointerType(TPasAliasType(AType).DestType);
end;

{**
  @brief Build a load instruction "mov val,ptr" or "mov val,seg:[ofs]".
  @param ValReg Value register name.
  @param PtrRegs Pointer register or pair ("di" or "es:di").
  @return Assembler instruction text.
}
function MakePtrLoad(const ValReg, PtrRegs: string): string;
var
  p: Integer;
begin
  p := Pos(':', PtrRegs);
  if p > 0 then
    Result := 'mov ' + ValReg + ',' +
              Copy(PtrRegs, 1, p - 1) + ':[' + Copy(PtrRegs, p + 1, MaxInt) + ']'
  else
    Result := 'mov ' + ValReg + ',[' + PtrRegs + ']';
end;

{**
  @brief Build a store instruction "mov ptr,val" or "mov seg:[ofs],val".
  @param PtrRegs Pointer register or pair ("di" or "es:di").
  @param ValReg Value register name.
  @return Assembler instruction text.
}
function MakePtrStore(const PtrRegs, ValReg: string): string;
var
  p: Integer;
begin
  p := Pos(':', PtrRegs);
  if p > 0 then
    Result := 'mov ' + Copy(PtrRegs, 1, p - 1) + ':[' + Copy(PtrRegs, p + 1, MaxInt) +
              '],' + ValReg
  else
    Result := 'mov [' + PtrRegs + '],' + ValReg;
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
    for i := 0 to Section.Declarations.Count - 1 do
    begin
      El := TPasElement(Section.Declarations[i]);
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

{**
  @brief Find a group by name in a module tree.
  @param Element Module or group to search in.
  @param GroupName Group name.
  @return Found group or nil.
}
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
  ArgIsPointer: Boolean;
  BaseName: string;
begin
  if AType is TPasAliasType then
  begin
    CType := PrimitiveToC(TPasAliasType(AType).DestType.Name);
    wln('typedef ' + CType + ' ' + AType.Name + ';');
  end
  else if AType is TPasPointerType then
  begin
    if Assigned(TPasPointerType(AType).DestType) then
    begin
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
    Convention := '_System';
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
          begin
            ArgIsPointer := IsPointerType(Arg.ArgType);
            BaseName := Arg.ArgType.Name;
            if ArgIsPointer then
              Args := Args + PrimitiveToC(BaseName) + ' *' + Arg.Name
            else
              Args := Args + PrimitiveToC(BaseName) + ' ' + Arg.Name;
          end;
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
          begin
            ArgIsPointer := IsPointerType(Arg.ArgType);
            BaseName := Arg.ArgType.Name;
            if ArgIsPointer then
              Args := Args + PrimitiveToC(BaseName) + ' *' + Arg.Name
            else
              Args := Args + PrimitiveToC(BaseName) + ' ' + Arg.Name;
          end;
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
  StructAttr := nil;
  if Assigned(FABIData) then
    StructAttr := FABIData.FindStructure(Stru.Name);

  if Assigned(StructAttr) and (StructAttr.Pack > 0) then
  begin
    if FH2IncCompat then wln('/* NOINC */');
    wln('#pragma pack(push, ' + IntToStr(StructAttr.Pack) + ')');
    if FH2IncCompat then wln('/* INC */');
  end;

  PrefixTag := '';
  if FH2IncCompat and Assigned(StructAttr) and (StructAttr.Prefix <> '') then
    PrefixTag := ' /* ' + StructAttr.Prefix + ' */';

  wln('typedef struct _' + Stru.Name + ' {' + PrefixTag);

  for i := 0 to Stru.Members.Count - 1 do
  begin
    VarEl := TPasVariable(Stru.Members[i]);

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

  wln('} ' + Stru.Name + ';');

  if Assigned(StructAttr) and (StructAttr.Pack > 0) then
  begin
    if FH2IncCompat then wln('/* NOINC */');
    wln('#pragma pack(pop)');
    if FH2IncCompat then wln('/* INC */');
  end;
end;

{**
  @brief Emit a #pragma aux directive for an interrupt, service or call syscall.

  Output form:
  @code
  #pragma aux <name> = \
      "mov ..." \                       -- fixed-value registers (rkValue)
      "mov val,[ptr]" \                 -- inout loads (before the call)
      "<control instruction>" \         -- int NNh | hlt/db/db NOT | call SSSS:OOOO
      "jc err" \                        -- CF=1 goes to err (error in AX)
      "mov [ptr],val" \                 -- out/inout stores (on success)
      "xor ax,ax" \                     -- APIRET = 0 on success
      "err:" \                          -- end of CF branch
      value [reg1 reg2 ...] \           -- rkResult registers
      parm [..] [..] [..] \             -- one bracketed group per parameter, in order
      modify [reg1 reg2 ...];           -- every register from inputs and outputs
  @endcode

  Registers inside [...] are separated by spaces, as required by OpenWatcom.
  Out-value parameters (Arg.Access = argOut) with a non-pointer type receive
  a pointer register in inputs and a value register in outputs; inout
  parameters (argInOut) are loaded from the pointer before the call and
  stored back on success. Parameters whose type is VOID or a pointer are
  not stored through an extra register: the caller passes the pointer by
  value, and the callee writes through it directly.
  If Syscall.UsesCF is set, the store block is guarded by the CF branch and
  AX is cleared on success. Labels are plain names ("err") local to the
  pragma aux block.

  All inconsistencies (missing register for a parameter, unknown parameter
  in a register, missing or extra result register, invalid Number, CF
  without a result register, out/inout parameter without pointer or value
  register) raise an exception with the function name.
}
procedure TCHWriter.WriteSyscallPragma(AFunc: TPasProcedureBase;
  ProcType: TPasProcedureType; const RetType: string; Syscall: TABISyscall);
var
  i, j: Integer;
  Reg: TABIRegister;
  Arg: TPasArgument;
  PtrReg, ValReg: TABIRegister;
  MovLines: TStringList;
  InstrLines: TStringList;
  PreLoadLines: TStringList;
  PostStoreLines: TStringList;
  ParmGroups: TStringList;
  ValueParts: TStringList;
  ModifyParts: TStringList;
  RegNames: TStringList;
  ParamRegs: TStringList;
  ParameterNames: TStringList;
  NumVal: Integer;
  HasResult: Boolean;
  ParmLine: string;

  {**
    @brief Build a space-separated register list from a TStringList.
  }
  function JoinRegs(List: TStringList): string;
  var
    k: Integer;
  begin
    Result := '';
    for k := 0 to List.Count - 1 do
    begin
      if k > 0 then Result := Result + ' ';
      Result := Result + List[k];
    end;
  end;

  {**
    @brief Find a parameter-bound register in a list.
  }
  function FindParamReg(const Args: TList; const Target: string): TABIRegister;
  var
    k: Integer;
    R: TABIRegister;
  begin
    Result := nil;
    for k := 0 to Args.Count - 1 do
    begin
      R := TABIRegister(Args[k]);
      if (R.Kind = rkParam) and SameText(R.Target, Target) then
      begin
        Result := R;
        Exit;
      end;
    end;
  end;

begin
  if not (SameText(Syscall.Convention, 'interrupt') or
          SameText(Syscall.Convention, 'service') or
          SameText(Syscall.Convention, 'call')) then
    raise Exception.CreateFmt(
      'Function "%s": unsupported syscall convention "%s"',
      [AFunc.Name, Syscall.Convention]);

  ParameterNames := TStringList.Create;
  MovLines := TStringList.Create;
  InstrLines := TStringList.Create;
  PreLoadLines := TStringList.Create;
  PostStoreLines := TStringList.Create;
  ParmGroups := TStringList.Create;
  ValueParts := TStringList.Create;
  ModifyParts := TStringList.Create;
  RegNames := TStringList.Create;
  try
    if Assigned(ProcType) then
      for i := 0 to ProcType.Args.Count - 1 do
        ParameterNames.Add(TPasArgument(ProcType.Args[i]).Name);

    { fixed-value registers (rkValue) produce mov instructions }
    for i := 0 to Syscall.Inputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Inputs[i]);
      if Reg.Kind = rkValue then
      begin
        if Pos(':', Reg.Regs) > 0 then
          raise Exception.CreateFmt(
            'Function "%s": fixed value on a register pair "%s" is not supported',
            [AFunc.Name, Reg.Regs]);
        MovLines.Add('    "mov ' + Reg.Regs + ',' + ConvertAsmNumber(Reg.Target) + '" \');
      end;
    end;
    for i := 0 to Syscall.Outputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Outputs[i]);
      if Reg.Kind = rkValue then
      begin
        if Pos(':', Reg.Regs) > 0 then
          raise Exception.CreateFmt(
            'Function "%s": fixed value on a register pair "%s" is not supported',
            [AFunc.Name, Reg.Regs]);
        MovLines.Add('    "mov ' + Reg.Regs + ',' + ConvertAsmNumber(Reg.Target) + '" \');
      end;
    end;

    { out/inout parameters with a non-pointer type: pointer register in
      inputs, value register in outputs. Pointer-typed and VOID parameters
      are skipped — the caller passes the pointer by value and the callee
      writes through it directly. }
    if Assigned(ProcType) then
      for i := 0 to ProcType.Args.Count - 1 do
      begin
        Arg := TPasArgument(ProcType.Args[i]);
        if (Arg.Access <> argOut) and (Arg.Access <> argInOut) then Continue;
        if IsPointerType(Arg.ArgType) then Continue;

        PtrReg := FindParamReg(Syscall.Inputs, Arg.Name);
        ValReg := FindParamReg(Syscall.Outputs, Arg.Name);

        if PtrReg = nil then
          raise Exception.CreateFmt(
            'Function "%s": no pointer register in inputs for out/inout parameter "%s"',
            [AFunc.Name, Arg.Name]);
        if ValReg = nil then
          raise Exception.CreateFmt(
            'Function "%s": no value register in outputs for out/inout parameter "%s"',
            [AFunc.Name, Arg.Name]);
        if Pos(':', ValReg.Regs) > 0 then
          raise Exception.CreateFmt(
            'Function "%s": out/inout parameter "%s" value register is a pair "%s"',
            [AFunc.Name, Arg.Name, ValReg.Regs]);

        if Arg.Access = argInOut then
          PreLoadLines.Add('    "' + MakePtrLoad(ValReg.Regs, PtrReg.Regs) + '" \');

        PostStoreLines.Add('    "' + MakePtrStore(PtrReg.Regs, ValReg.Regs) + '" \');
      end;

    { control transfer instruction }
    if SameText(Syscall.Convention, 'interrupt') then
    begin
      if not ParseByteNumber(Syscall.Number, NumVal) then
        raise Exception.CreateFmt(
          'Function "%s": interrupt number "%s" is not a scalar in range 0..255',
          [AFunc.Name, Syscall.Number]);
      InstrLines.Add('    "int ' + ConvertAsmNumber(Syscall.Number) + '" \');
    end
    else if SameText(Syscall.Convention, 'service') then
    begin
      if not ParseByteNumber(Syscall.Number, NumVal) then
        raise Exception.CreateFmt(
          'Function "%s": service code "%s" is not a scalar in range 0..255',
          [AFunc.Name, Syscall.Number]);
      InstrLines.Add('    "hlt" \');
      InstrLines.Add('    "db ' + IntToStr(NumVal) + '" \');
      InstrLines.Add('    "db NOT ' + IntToStr(NumVal) + '" \');
    end
    else if SameText(Syscall.Convention, 'call') then
    begin
      if Pos(':', Syscall.Number) = 0 then
        raise Exception.CreateFmt(
          'Function "%s": call target "%s" is not a far address (expected SSSS:OOOO)',
          [AFunc.Name, Syscall.Number]);
      InstrLines.Add('    "call ' + ConvertFarAddr(Syscall.Number) + '" \');
    end;

    { CF conversion and out/inout stores }
    if Syscall.UsesCF then
    begin
      HasResult := False;
      for i := 0 to Syscall.Outputs.Count - 1 do
        if TABIRegister(Syscall.Outputs[i]).Kind = rkResult then
          HasResult := True;
      if not HasResult then
        raise Exception.CreateFmt(
          'Function "%s": CF conversion requires a result register',
          [AFunc.Name]);

      InstrLines.Add('    "jc err" \');
      for i := 0 to PostStoreLines.Count - 1 do
        InstrLines.Add(PostStoreLines[i]);
      InstrLines.Add('    "xor ax,ax" \');
      InstrLines.Add('    "err:" \');
    end
    else
      for i := 0 to PostStoreLines.Count - 1 do
        InstrLines.Add(PostStoreLines[i]);

    { parm: one bracketed group per parameter in signature order.
      Only registers from inputs participate; outputs hold values
      written by DOS and are not part of the calling convention. }
    if Assigned(ProcType) and (ProcType.Args.Count > 0) then
    begin
      ParmLine := '    parm';
      for i := 0 to ProcType.Args.Count - 1 do
      begin
        Arg := TPasArgument(ProcType.Args[i]);
        ParamRegs := TStringList.Create;
        try
          for j := 0 to Syscall.Inputs.Count - 1 do
          begin
            Reg := TABIRegister(Syscall.Inputs[j]);
            if (Reg.Kind = rkParam) and SameText(Reg.Target, Arg.Name) then
              SplitRegs(Reg.Regs, ParamRegs);
          end;
          if ParamRegs.Count = 0 then
            raise Exception.CreateFmt(
              'Function "%s": no register bound to parameter "%s"',
              [AFunc.Name, Arg.Name]);
          ParmLine := ParmLine + ' [' + JoinRegs(ParamRegs) + ']';
        finally
          ParamRegs.Free;
        end;
      end;
      ParmLine := ParmLine + ' \';
      ParmGroups.Add(ParmLine);
    end;

    { value block }
    for i := 0 to Syscall.Outputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Outputs[i]);
      if Reg.Kind = rkResult then
        SplitRegs(Reg.Regs, ValueParts);
    end;

    { modify block: flat list of every register, deduplicated }
    for i := 0 to Syscall.Inputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Inputs[i]);
      SplitRegs(Reg.Regs, RegNames);
    end;
    for i := 0 to Syscall.Outputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Outputs[i]);
      SplitRegs(Reg.Regs, RegNames);
    end;
    for i := 0 to RegNames.Count - 1 do
      if ModifyParts.IndexOf(RegNames[i]) < 0 then
        ModifyParts.Add(RegNames[i]);

    { validation: every rkParam target must be a real parameter }
    for i := 0 to Syscall.Inputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Inputs[i]);
      if (Reg.Kind = rkParam) and (ParameterNames.IndexOf(Reg.Target) < 0) then
        raise Exception.CreateFmt(
          'Function "%s": register "%s" bound to unknown parameter "%s"',
          [AFunc.Name, Reg.Regs, Reg.Target]);
    end;
    for i := 0 to Syscall.Outputs.Count - 1 do
    begin
      Reg := TABIRegister(Syscall.Outputs[i]);
      if (Reg.Kind = rkParam) and (ParameterNames.IndexOf(Reg.Target) < 0) then
        raise Exception.CreateFmt(
          'Function "%s": register "%s" bound to unknown parameter "%s"',
          [AFunc.Name, Reg.Regs, Reg.Target]);
    end;

    { validation: result register must match the return type }
    HasResult := ValueParts.Count > 0;
    if SameText(RetType, 'void') and HasResult then
      raise Exception.CreateFmt(
        'Function "%s": void function has a result register', [AFunc.Name]);
    if (not SameText(RetType, 'void')) and (not HasResult) then
      raise Exception.CreateFmt(
        'Function "%s": non-void function has no result register', [AFunc.Name]);

    { assemble the directive }
    wln('#pragma aux ' + AFunc.Name + ' = \');
    for i := 0 to MovLines.Count - 1 do
      wln(MovLines[i]);
    for i := 0 to PreLoadLines.Count - 1 do
      wln(PreLoadLines[i]);
    for i := 0 to InstrLines.Count - 1 do
      wln(InstrLines[i]);
    if ValueParts.Count > 0 then
      wln('    value [' + JoinRegs(ValueParts) + '] \');
    for i := 0 to ParmGroups.Count - 1 do
      wln(ParmGroups[i]);
    if ModifyParts.Count > 0 then
      wln('    modify [' + JoinRegs(ModifyParts) + '];');
  finally
    ParameterNames.Free;
    MovLines.Free;
    InstrLines.Free;
    PreLoadLines.Free;
    PostStoreLines.Free;
    ParmGroups.Free;
    ValueParts.Free;
    ModifyParts.Free;
    RegNames.Free;
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
  Syscall: TABISyscall;
begin
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
  Syscall := nil;
  if Assigned(FABIData) then
  begin
    Entries := FABIData.FindEntry(AFunc.Name);
    try
      if Entries.Count > 0 then
        Convention := TABIEntry(Entries[0]).Convention;
    finally
      Entries.Free;
    end;

    if Convention = '' then
    begin
      Syscall := FABIData.FindSyscall(AFunc.Name);
      if Assigned(Syscall) then
        Convention := Syscall.Convention;
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
        begin
          IsPointer := IsPointerType(Arg.ArgType);
          BaseTypeName := Arg.ArgType.Name;
        end;
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

  if Assigned(Syscall) then
    wln(RetType + ' ' + AFunc.Name + '(' + Args + ');')
  else
    wln(RetType + ' ' + Convention + ' ' + AFunc.Name + '(' + Args + ');');

  if Assigned(Syscall) and
     (SameText(Convention, 'interrupt') or
      SameText(Convention, 'service') or
      SameText(Convention, 'call')) then
    WriteSyscallPragma(AFunc, ProcType, RetType, Syscall);
end;

procedure TCHWriter.WriteVariable(AVar: TPasVariable);
var
  TypeName: string;
begin
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
  wln('#define ' + AConst.Name + ' ' + AConst.Value);
end;

procedure TCHWriter.Generate(Module: TPasModule; const OutputFileName: string);
var
  Desc, BaseName, GuardName, IncMacro: string;
  LayoutEntry: TLayoutEntry;
begin
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

{**
  @brief Write a set of C headers, one per layout group.

  Content is first written to a temporary file; on success the target
  file is replaced atomically. On failure the temporary file is removed.
}
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
    RootStream.Free;
    if FileExists(TempName) then
      DeleteFile(TempName);
    raise;
  end;

  RootStream.Free;

  if FileExists(DefaultOutput) then
    DeleteFile(DefaultOutput);
  RenameFile(TempName, DefaultOutput);
end;

{**
  @brief Write a single C header for one selected group.

  Content is first written to a temporary file; on success the target
  file is replaced atomically. On failure the temporary file is removed.
}
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
    Stream.Free;
    if FileExists(TempName) then
      DeleteFile(TempName);
    raise;
  end;

  Stream.Free;

  if FileExists(OutputFile) then
    DeleteFile(OutputFile);
  RenameFile(TempName, OutputFile);
end;

end.
