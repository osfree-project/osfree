{
    This file is part of the Free Component Library

    Pascal tree source file writer
    Copyright (c) 2003 by
      Areca Systems GmbH / Sebastian Guenther, sg@freepascal.org

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}


unit PasWrite;

interface

uses Classes, PasTree;

type
  TPasWriter = class
  private
    FStream: TStream;
    IsStartOfLine: Boolean;
    Indent, CurDeclSection: string;
    DeclSectionStack: TList;
    procedure IncIndent;
    procedure DecIndent;
    procedure IncDeclSectionLevel;
    procedure DecDeclSectionLevel;
    procedure PrepareDeclSection(const ADeclSection: string);
  public
    constructor Create(AStream: TStream);
    destructor Destroy; override;
    procedure wrt(const s: string);
    procedure wrtln(const s: string);
    procedure wrtln;

    procedure WriteElement(AElement: TPasElement);
    procedure WriteType(AType: TPasType);
    procedure WriteModule(AModule: TPasModule);
    procedure WriteSection(ASection: TPasSection);
    procedure WriteClass(AClass: TPasClassType);
    procedure WriteVariable(AVar: TPasVariable);
    procedure WriteProcDecl(AProc: TPasProcedure);
    procedure WriteProcImpl(AProc: TPasProcedureImpl);
    procedure WriteProperty(AProp: TPasProperty);
    procedure WriteImplBlock(ABlock: TPasImplBlock);
    procedure WriteImplElement(AElement: TPasImplElement;
      AAutoInsertBeginEnd: Boolean);
    procedure WriteImplCommand(ACommand: TPasImplCommand);
    procedure WriteImplCommands(ACommands: TPasImplCommands);
    procedure WriteImplIfElse(AIfElse: TPasImplIfElse);
    procedure WriteImplForLoop(AForLoop: TPasImplForLoop);
    property Stream: TStream read FStream;
  end;


procedure WritePasFile(AElement: TPasElement; const AFilename: string);
procedure WritePasFile(AElement: TPasElement; AStream: TStream);



implementation

uses SysUtils;

type
  PDeclSectionStackElement = ^TDeclSectionStackElement;
  TDeclSectionStackElement = record
    LastDeclSection, LastIndent: string;
  end;

constructor TPasWriter.Create(AStream: TStream);
begin
  FStream := AStream;
  IsStartOfLine := True;
  DeclSectionStack := TList.Create;
end;

destructor TPasWriter.Destroy;
var
  i: Integer;
  El: PDeclSectionStackElement;
begin
  for i := 0 to DeclSectionStack.Count - 1 do
  begin
    El := PDeclSectionStackElement(DeclSectionStack[i]);
    Dispose(El);
  end;
  DeclSectionStack.Free;
  inherited Destroy;
end;

procedure TPasWriter.wrt(const s: string);
begin
  if IsStartOfLine then
  begin
    if Length(Indent) > 0 then
      Stream.Write(Indent[1], Length(Indent));
    IsStartOfLine := False;
  end;
  Stream.Write(s[1], Length(s));
end;

const
  LF: string = #10;

procedure TPasWriter.wrtln(const s: string);
begin
  wrt(s);
  Stream.Write(LF[1], 1);
  IsStartOfLine := True;
end;

procedure TPasWriter.wrtln;
begin
  Stream.Write(LF[1], 1);
  IsStartOfLine := True;
end;

procedure TPasWriter.WriteElement(AElement: TPasElement);
begin
  if AElement.ClassType = TPasModule then
    WriteModule(TPasModule(AElement))
  else if AElement.ClassType = TPasSection then
    WriteSection(TPasSection(AElement))
  else if AElement.ClassType = TPasVariable then
    WriteVariable(TPasVariable(AElement))
  else if AElement.InheritsFrom(TPasType) then
    WriteType(TPasType(AElement))
  else if AElement.InheritsFrom(TPasProcedure) then
    WriteProcDecl(TPasProcedure(AElement))
  else if AElement.InheritsFrom(TPasProcedureImpl) then
    WriteProcImpl(TPasProcedureImpl(AElement))
  else if AElement.ClassType = TPasProperty then
    WriteProperty(TPasProperty(AElement))
  else
    raise Exception.Create('Writing not implemented for ' +
      AElement.ElementTypeName + ' nodes');
end;

procedure TPasWriter.WriteType(AType: TPasType);
begin
  if AType.ClassType = TPasUnresolvedTypeRef then
    wrt(AType.Name)
  else if AType.ClassType = TPasClassType then
    WriteClass(TPasClassType(AType))
  else
    raise Exception.Create('Writing not implemented for ' +
      AType.ElementTypeName + ' nodes');
end;


procedure TPasWriter.WriteModule(AModule: TPasModule);
begin
  wrtln('unit ' + AModule.Name + ';');
  wrtln;
  wrtln('interface');
  wrtln;
  WriteSection(AModule.InterfaceSection);
  Indent := '';
  wrtln;
  wrtln;
  wrtln('implementation');
  if Assigned(AModule.ImplementationSection) then
  begin
    wrtln;
    WriteSection(AModule.ImplementationSection);
  end;
  wrtln;
  wrtln('end.');
end;

procedure TPasWriter.WriteSection(ASection: TPasSection);
var
  i: Integer;
begin
  if ASection.UsesList.Count > 0 then
  begin
    wrt('uses ');
    for i := 0 to ASection.UsesList.Count - 1 do
    begin
      if i > 0 then
        wrt(', ');
      wrt(TPasElement(ASection.UsesList[i]).Name);
    end;
    wrtln(';');
    wrtln;
  end;

  CurDeclSection := '';

  for i := 0 to ASection.Declarations.Count - 1 do
    WriteElement(TPasElement(ASection.Declarations[i]));
end;

procedure TPasWriter.WriteClass(AClass: TPasClassType);
var
  i: Integer;
  Member: TPasElement;
  LastVisibility, CurVisibility: TPasMemberVisibility;
begin
  PrepareDeclSection('type');
  wrt(AClass.Name + ' = ');
  if AClass.IsPacked then
     wrt('packed ');                      // 12/04/04 - Dave - Added
  case AClass.ObjKind of
    okObject: wrt('object');
    okClass: wrt('class');
    okInterface: wrt('interface');
  end;
  if Assigned(AClass.AncestorType) then
    wrtln('(' + AClass.AncestorType.Name + ')')
  else
    wrtln;
  IncIndent;
  LastVisibility := visDefault;
  for i := 0 to AClass.Members.Count - 1 do
  begin
    Member := TPasElement(AClass.Members[i]);
    CurVisibility := Member.Visibility;
    if CurVisibility <> LastVisibility then
    begin
      DecIndent;
      case CurVisibility of
        visPrivate: wrtln('private');
        visProtected: wrtln('protected');
        visPublic: wrtln('public');
        visPublished: wrtln('published');
        visAutomated: wrtln('automated');
      end;
      IncIndent;
      LastVisibility := CurVisibility;
    end;
    WriteElement(Member);
  end;
  DecIndent;
  wrtln('end;');
  wrtln;
end;

procedure TPasWriter.WriteVariable(AVar: TPasVariable);
begin
  if (AVar.Parent.ClassType <> TPasClassType) and
    (AVar.Parent.ClassType <> TPasRecordType) then
    PrepareDeclSection('var');
  wrt(AVar.Name + ': ');
  WriteType(AVar.VarType);
  wrtln(';');
end;

procedure TPasWriter.WriteProcDecl(AProc: TPasProcedure);
var
  i: Integer;
begin
  wrt(AProc.TypeName + ' ' + AProc.Name);

  if Assigned(AProc.ProcType) and (AProc.ProcType.Args.Count > 0) then
  begin
    wrt('(');
    for i := 0 to AProc.ProcType.Args.Count - 1 do
      with TPasArgument(AProc.ProcType.Args[i]) do
      begin
        if i > 0 then
          wrt('; ');
        case Access of
          argConst: wrt('const ');
          argVar: wrt('var ');
        end;
        wrt(Name);
        if Assigned(ArgType) then
        begin
          wrt(': ');
          WriteElement(ArgType);
        end;
        if Value <> '' then
          wrt(' = ' + Value);
      end;
    wrt(')');
  end;

  if Assigned(AProc.ProcType) and
    (AProc.ProcType.ClassType = TPasFunctionType) then
  begin
    wrt(': ');
    WriteElement(TPasFunctionType(AProc.ProcType).ResultEl.ResultType);
  end;

  wrt(';');

  if AProc.IsVirtual then
    wrt(' virtual;');
  if AProc.IsDynamic then
    wrt(' dynamic;');
  if AProc.IsAbstract then
    wrt(' abstract;');
  if AProc.IsOverride then
    wrt(' override;');
  if AProc.IsOverload then
    wrt(' overload;');
  if AProc.IsReintroduced then
    wrt(' reintroduce;');
  if AProc.IsStatic then
    wrt(' static;');


  // !!!: Not handled: Message, calling conventions

  wrtln;
end;

procedure TPasWriter.WriteProcImpl(AProc: TPasProcedureImpl);
var
  i: Integer;
begin
  PrepareDeclSection('');
  wrt(AProc.TypeName + ' ');

  if AProc.Parent.ClassType = TPasClassType then
    wrt(AProc.Parent.Name + '.');

  wrt(AProc.Name);

  if Assigned(AProc.ProcType) and (AProc.ProcType.Args.Count > 0) then
  begin
    wrt('(');
    for i := 0 to AProc.ProcType.Args.Count - 1 do
      with TPasArgument(AProc.ProcType.Args[i]) do
      begin
        if i > 0 then
          wrt('; ');
        case Access of
          argConst: wrt('const ');
          argVar: wrt('var ');
        end;
        wrt(Name);
        if Assigned(ArgType) then
        begin
          wrt(': ');
          WriteElement(ArgType);
        end;
        if Value <> '' then
          wrt(' = ' + Value);
      end;
    wrt(')');
  end;

  if Assigned(AProc.ProcType) and
    (AProc.ProcType.ClassType = TPasFunctionType) then
  begin
    wrt(': ');
    WriteElement(TPasFunctionType(AProc.ProcType).ResultEl.ResultType);
  end;

  wrtln(';');
  IncDeclSectionLevel;
  for i := 0 to AProc.Locals.Count - 1 do
  begin
    if TPasElement(AProc.Locals[i]).InheritsFrom(TPasProcedureImpl) then
    begin
      IncIndent;
      if (i = 0) or not
        TPasElement(AProc.Locals[i - 1]).InheritsFrom(TPasProcedureImpl) then
        wrtln;
    end;

    WriteElement(TPasElement(AProc.Locals[i]));

    if TPasElement(AProc.Locals[i]).InheritsFrom(TPasProcedureImpl) then
      DecIndent;
  end;
  DecDeclSectionLevel;

  wrtln('begin');
  IncIndent;
  if Assigned(AProc.Body) then
    WriteImplBlock(AProc.Body);
  DecIndent;
  wrtln('end;');
  wrtln;
end;

procedure TPasWriter.WriteProperty(AProp: TPasProperty);
var
  i: Integer;
begin
  wrt('property ' + AProp.Name);
  if AProp.Args.Count > 0 then
  begin
    wrt('[');
    for i := 0 to AProp.Args.Count - 1 do;
      // !!!: Create WriteArgument method and call it here
    wrt(']');
  end;
  if Assigned(AProp.VarType) then
  begin
    wrt(': ');
    WriteType(AProp.VarType);
  end;
  if AProp.ReadAccessorName <> '' then
    wrt(' read ' + AProp.ReadAccessorName);
  if AProp.WriteAccessorName <> '' then
    wrt(' write ' + AProp.WriteAccessorName);
  if AProp.StoredAccessorName <> '' then
    wrt(' stored ' + AProp.StoredAccessorName);
  if AProp.DefaultValue <> '' then
    wrt(' default ' + AProp.DefaultValue);
  if AProp.IsNodefault then
    wrt(' nodefault');
  if AProp.IsDefault then
    wrt('; default');
  wrtln(';');
end;

procedure TPasWriter.WriteImplBlock(ABlock: TPasImplBlock);
var
  i: Integer;
begin
  for i := 0 to ABlock.Elements.Count - 1 do
  begin
    WriteImplElement(TPasImplElement(ABlock.Elements[i]), False);
    if TPasImplElement(ABlock.Elements[i]).ClassType = TPasImplCommand then
      wrtln(';');
  end;
end;

procedure TPasWriter.WriteImplElement(AElement: TPasImplElement;
  AAutoInsertBeginEnd: Boolean);
begin
  if AElement.ClassType = TPasImplCommand then
    WriteImplCommand(TPasImplCommand(AElement))
  else if AElement.ClassType = TPasImplCommands then
  begin
    DecIndent;
    if AAutoInsertBeginEnd then
      wrtln('begin');
    IncIndent;
    WriteImplCommands(TPasImplCommands(AElement));
    DecIndent;
    if AAutoInsertBeginEnd then
      wrtln('end;');
    IncIndent;
  end else if AElement.ClassType = TPasImplBlock then
  begin
    DecIndent;
    if AAutoInsertBeginEnd then
      wrtln('begin');
    IncIndent;
    WriteImplBlock(TPasImplBlock(AElement));
    DecIndent;
    if AAutoInsertBeginEnd then
      wrtln('end;');
    IncIndent;
  end else if AElement.ClassType = TPasImplIfElse then
    WriteImplIfElse(TPasImplIfElse(AElement))
  else if AElement.ClassType = TPasImplForLoop then
    WriteImplForLoop(TPasImplForLoop(AElement))
  else
    raise Exception.Create('Writing not yet implemented for ' +
      AElement.ClassName + ' implementation elements');
end;

procedure TPasWriter.WriteImplCommand(ACommand: TPasImplCommand);
begin
  wrt(ACommand.Command);
end;

procedure TPasWriter.WriteImplCommands(ACommands: TPasImplCommands);
var
  i: Integer;
  s: string;
begin
  for i := 0 to ACommands.Commands.Count - 1 do
  begin
    s := ACommands.Commands[i];
    if Length(s) > 0 then
      if (Length(s) >= 2) and (s[1] = '/') and (s[2] = '/') then
        wrtln(s)
      else
        wrtln(s + ';');
  end;
end;

procedure TPasWriter.WriteImplIfElse(AIfElse: TPasImplIfElse);
begin
  wrt('if ' + AIfElse.Condition + ' then');
  if Assigned(AIfElse.IfBranch) then
  begin
    wrtln;
    if (AIfElse.IfBranch.ClassType = TPasImplCommands) or
      (AIfElse.IfBranch.ClassType = TPasImplBlock) then
      wrtln('begin');
    IncIndent;
    WriteImplElement(AIfElse.IfBranch, False);
    DecIndent;
    if (AIfElse.IfBranch.ClassType = TPasImplCommands) or
      (AIfElse.IfBranch.ClassType = TPasImplBlock) then
      if Assigned(AIfElse.ElseBranch) then
        wrt('end ')
      else
        wrtln('end;')
    else
      if Assigned(AIfElse.ElseBranch) then
        wrtln;
  end else
    if not Assigned(AIfElse.ElseBranch) then
      wrtln(';')
    else
      wrtln;

  if Assigned(AIfElse.ElseBranch) then
    if AIfElse.ElseBranch.ClassType = TPasImplIfElse then
    begin
      wrt('else ');
      WriteImplElement(AIfElse.ElseBranch, True);
    end else
    begin
      wrtln('else');
      IncIndent;
      WriteImplElement(AIfElse.ElseBranch, True);
      if (not Assigned(AIfElse.Parent)) or
        (AIfElse.Parent.ClassType <> TPasImplIfElse) or
        (TPasImplIfElse(AIfElse.Parent).IfBranch <> AIfElse) then
        wrtln(';');
      DecIndent;
    end;
end;

procedure TPasWriter.WriteImplForLoop(AForLoop: TPasImplForLoop);
begin
  wrtln('for ' + AForLoop.Variable.Name + ' := ' + AForLoop.StartValue +
    ' to ' + AForLoop.EndValue + ' do');
  IncIndent;
  WriteImplElement(AForLoop.Body, True);
  DecIndent;
  if (AForLoop.Body.ClassType <> TPasImplBlock) and
    (AForLoop.Body.ClassType <> TPasImplCommands) then
      wrtln(';');
end;

procedure TPasWriter.IncIndent;
begin
  Indent := Indent + '  ';
end;

procedure TPasWriter.DecIndent;
begin
  if Indent = '' then
    raise Exception.Create('Internal indent error');
  SetLength(Indent, Length(Indent) - 2);
end;

procedure TPasWriter.IncDeclSectionLevel;
var
  El: PDeclSectionStackElement;
begin
  New(El);
  DeclSectionStack.Add(El);
  El^.LastDeclSection := CurDeclSection;
  El^.LastIndent := Indent;
  CurDeclSection := '';
end;

procedure TPasWriter.DecDeclSectionLevel;
var
  El: PDeclSectionStackElement;
begin
  El := PDeclSectionStackElement(DeclSectionStack[DeclSectionStack.Count - 1]);
  DeclSectionStack.Delete(DeclSectionStack.Count - 1);
  CurDeclSection := El^.LastDeclSection;
  Indent := El^.LastIndent;
  Dispose(El);
end;

procedure TPasWriter.PrepareDeclSection(const ADeclSection: string);
begin
  if ADeclSection <> CurDeclSection then
  begin
    if CurDeclsection <> '' then
      DecIndent;
    if ADeclSection <> '' then
    begin
      wrtln(ADeclSection);
      IncIndent;
    end;
    CurDeclSection := ADeclSection;
  end;
end;


procedure WritePasFile(AElement: TPasElement; const AFilename: string);
var
  Stream: TFileStream;
begin
  Stream := TFileStream.Create(AFilename, fmCreate);
  try
    WritePasFile(AElement, Stream);
  finally
    Stream.Free;
  end;
end;

procedure WritePasFile(AElement: TPasElement; AStream: TStream);
var
  Writer: TPasWriter;
begin
  Writer := TPasWriter.Create(AStream);
  try
    Writer.WriteElement(AElement);
  finally
    Writer.Free;
  end;
end;

end.
