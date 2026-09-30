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


unit IncWrite;

interface

uses Classes, PasTree, ABI;

type
  TIncWriter = class
  private
    FStream: TStream;
    IsStartOfLine: Boolean;
    Indent, CurDeclSection: string;
    DeclSectionStack: TList;
    //FAbiList: TPasSection;
    procedure IncIndent;
    procedure DecIndent;
    procedure IncDeclSectionLevel;
    procedure DecDeclSectionLevel;
    procedure PrepareDeclSection(const ADeclSection: string);
    //function MultiAbiGet(Symbol: String): TAbi;
  public
    constructor Create(AStream: TStream);
    destructor Destroy; override;
    procedure wrt(const s: string);
    procedure wrtln(const s: string);
    procedure wrtln;

    procedure WriteElement(AElement: TPasElement);
    procedure WriteRecordType(AElement: TPasRecordType; NestingLevel: Integer);
    procedure WriteType(AType: TPasType; ATypeDecl: boolean);
    procedure WriteModule(AModule: TPasModule);
    procedure WriteSection(ASection: TPasSection);
    procedure WriteClass(AClass: TPasClassType);
    procedure WriteConstant(AVar: TPasConst);
    procedure WriteVariable(AVar: TPasVariable);
    procedure WriteProcDecl(AProc: TPasProcedure);
    procedure WriteFunctionType(AProc: TPasFunctionType);
    procedure WriteProcedureType(AProc: TPasProcedureType);
    procedure WriteProperty(AProp: TPasProperty);
    property Stream: TStream read FStream;
  end;


procedure WriteIncFile(AElement: TPasElement; const AFilename: string);
procedure WriteIncFile(AElement: TPasElement; AStream: TStream);



implementation

uses SysUtils;

type
  PDeclSectionStackElement = ^TDeclSectionStackElement;
  TDeclSectionStackElement = record
    LastDeclSection, LastIndent: string;
  end;

constructor TIncWriter.Create(AStream: TStream);
begin
  FStream := AStream;
  IsStartOfLine := True;
  DeclSectionStack := TList.Create;
end;

destructor TIncWriter.Destroy;
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

procedure TIncWriter.wrt(const s: string);
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

procedure TIncWriter.wrtln(const s: string);
begin
  wrt(s);
  Stream.Write(LF[1], 1);
  IsStartOfLine := True;
end;

procedure TIncWriter.wrtln;
begin
  Stream.Write(LF[1], 1);
  IsStartOfLine := True;
end;

procedure TIncWriter.WriteElement(AElement: TPasElement);
begin
  if AElement.ClassType = TPasModule then
    WriteModule(TPasModule(AElement))
  else if AElement.ClassType = TPasSection then
    WriteSection(TPasSection(AElement))
  else if AElement.ClassType = TPasVariable then
    WriteVariable(TPasVariable(AElement))
  else if AElement.ClassType = TPasConst then
    WriteConstant(TPasConst(AElement))
  else if AElement.InheritsFrom(TPasType) then
    WriteType(TPasType(AElement), true)
  else if AElement.InheritsFrom(TPasProcedure) then
    WriteProcDecl(TPasProcedure(AElement))
  else if AElement.ClassType = TPasProperty then
    WriteProperty(TPasProperty(AElement))
  else
    raise Exception.Create('Writing not implemented for ' +
      AElement.ElementTypeName + ' nodes');
end;

function ConvertToASMType(S: String): String;
begin
  Result:=S;
  if S='pointer' then Result:='void *' else // How to correctly handle this?
  if S='T_POINTER' then Result:='void *' else
  if S='T_CHAR' then Result:='char' else
  if S='T_BYTE' then Result:='unsigned char' else
  if S='T_WORD' then Result:='unsigned short' else
  if S='T_DWORD' then Result:='unsigned long' else
  if S='T_INT16' then Result:='short' else
  if S='T_INT32' then Result:='long' else
  if S='T_UINT' then Result:='unsigned int';
end;

procedure TIncWriter.WriteType(AType: TPasType; ATypeDecl: boolean);
begin
  if AType.ClassType = TPasUnresolvedTypeRef then
    wrt(ConvertToASMType(AType.Name))
  else if AType.ClassType = TPasClassType then
    WriteClass(TPasClassType(AType))
  else if AType.ClassType = TPasPointerType then
  begin
    wrt(ConvertToASMType(TPasAliasType(AType).Name));

    if ATypeDecl then wrt(' TYPEDEF ');

    if Assigned(TPasPointerType(AType).DestType) then
    begin
      if ATypeDecl then wrt('PTR '+ConvertToASMType(TPasPointerType(AType).DestType.Name));
    end else begin
      if ATypeDecl then wrt('PTR ');
    end;

    if (not ATypeDecl) and (TPasAliasType(AType).Name='') and Assigned(TPasPointerType(AType).DestType) then
	begin
	  wrt(' PTR '+ConvertToASMType(TPasPointerType(AType).DestType.Name));
	end;
    if ATypeDecl then wrtln('');
  end else if AType.ClassType = TPasAliasType then
  begin
    wrt(ConvertToASMType(TPasAliasType(AType).Name));
    if ATypeDecl then WrtLn(' TYPEDEF '+ConvertToASMType(TPasAliasType(AType).DestType.Name));
  end else if AType.ClassType = TPasRecordType then
  begin
    if aTypeDecl then
      WriteRecordType(TPasRecordType(AType), 0)
    else
      wrt(TPasRecordType(AType).Name);
  end else if AType.ClassType = TPasArrayType then
  begin
    if ATypeDecl then
      wrt(ConvertToASMType(TPasArrayType(AType).ElType.Name)+' '+TPasAliasType(AType).Name+'[' + TPasArrayType(AType).IndexRange + ']')
    else
      wrt(TPasAliasType(AType).Name);

    if ATypeDecl then WrtLn(';');
  end else if AType.ClassType = TPasFunctionType then
  begin
    WriteFunctionType(TPasFunctionType(AType));
  end else if AType.ClassType = TPasProcedureType then
  begin
    WriteProcedureType(TPasProcedureType(AType));
  end else
    raise Exception.Create('Writing not implemented for ' +
      AType.ElementTypeName + ' type nodes');
end;


procedure TIncWriter.WriteModule(AModule: TPasModule);
begin
  WrtLn( ';*****************************************************************************');
  WrtLn( ';    '+AModule.Name+'.inc');
  WrtLn( ';    (C) 2004-2024 osFree project');
  WrtLn( ';');;
  WrtLn( ';    WARNING! Automaticaly generated file! Don''t edit it manually!');
  WrtLn( ';*****************************************************************************');
  WrtLn;
  WrtLn( '		ifndef __'+Upcase(AModule.Name)+'_INC__');
  WrtLn( '__'+Upcase(AModule.Name)+'_INC__	EQU	1');
  WrtLn( '		ifndef APIENTRY');
  WrtLn( 'APIENTRY	EQU	C');
  WrtLn( '		endif');
  WrtLn( '		ifndef USHORT');
  WrtLn( 'USHORT	TYPEDEF	WORD');
  WrtLn( '		endif');
  WrtLn( '		ifndef SHORT');
  WrtLn( 'SHORT	TYPEDEF	WORD');
  WrtLn( '		endif');
  WrtLn( '		ifndef UCHAR');
  WrtLn( 'UCHAR		TYPEDEF	BYTE');
  WrtLn( '		endif');
  WrtLn( '		ifndef CHAR');
  WrtLn( 'CHAR		TYPEDEF	BYTE');
  WrtLn( '		endif');
  WrtLn( '		ifndef ULONG');
  WrtLn( 'ULONG		TYPEDEF	DWORD');
  WrtLn( '		endif');
{$ifdef 0}
  WrtLn;
  WrtLn( '#ifndef NULL');
  WrtLn( '#define NULL  0');
  WrtLn( '#endif');
  WrtLn;
  WrtLn( '#ifndef TRUE');
  WrtLn( '#define TRUE  1');
  WrtLn( '#endif');
  WrtLn;
  WrtLn( '#ifndef FALSE');
  WrtLn( '#define FALSE 0');
  WrtLn( '#endif');
  WrtLn;
  WrtLn( '#ifndef VOID');
  WrtLn( '#define VOID void');
  WrtLn( '#endif');
  WrtLn;
  WrtLn( '#ifndef EXPENTRY');
  WrtLn( '#define EXPENTRY  _System');
  WrtLn( '#endif');
  WrtLn;
  WrtLn;
  WrtLn( '#ifndef FAR');
  WrtLn( '#define FAR');
  WrtLn( '#endif');
  WrtLn;
  WrtLn( '#ifndef NEAR');
  WrtLn( '#define NEAR');
  WrtLn( '#endif');
{$endif}
  WrtLn;
  WriteSection(AModule.InterfaceSection);
  Indent := '';
  wrtln;
  wrtln;
  WrtLn( '	endif	; __'+Upcase(AModule.Name)+'_INC__');
end;

function LastPos(Needle: Char; Haystack: String): integer;
begin
  for Result := Length(Haystack) downto 1 do
    if Haystack[Result] = Needle then
      Break;
end;

procedure TIncWriter.WriteSection(ASection: TPasSection);
var
  i: Integer;
  UsesName: String;
  Alias: String;
  BaseName: String;
begin

  CurDeclSection := '';

  //FAbiList:=ASection;

  for i := 0 to ASection.Declarations.Count - 1 do
    WriteElement(TPasElement(ASection.Declarations[i]));

  if ASection.UsesList.Count > 0 then
  begin
    for i := 0 to ASection.UsesList.Count - 1 do
    begin
          UsesName:=TPasElement(ASection.UsesList[i]).Name;

          While pos('|',UsesName)>0 do
          begin
            BaseName:=Copy(UsesName, 1, Pos('|', UsesName)-1);
                If BaseName[1]='!' then Delete(BaseName, 1, 1);
            Alias:=Copy(UsesName, LastPos('|', UsesName)+1, Length(UsesName)-LastPos('|', UsesName));
                Delete(UsesName, LastPos('|', UsesName), Length(UsesName)-LastPos('|', UsesName)+1);
        wrtln('	ifdef INCL_'+upcase(Alias));
        wrtln('INCL_'+upcase(BaseName)+'	EQU	1');
        wrtln('	endif');
          end;

          If UsesName[1]='!' then
          begin
                Delete(UsesName, 1, 1);
        wrtln('INCL_'+upcase(UsesName)+'	EQU	1');
          end;

      wrtln('	ifdef INCL_'+upcase(UsesName));
      wrtln('  	include '+UsesName+'.inc');
      wrtln('	endif');
      wrtln;
    end;
  end;

end;

(*function TIncWriter.MultiAbiGet(Symbol: String): TAbi;
var
  i: word;
begin
  if assigned(fabilist) then if FAbiList.AbiList.Count>0 then
  for i := 0 to FAbiList.AbiList.Count - 1 do
  begin
    Result:=AbiGet(TPasElement(FAbiList.AbiList[i]).Name+'.abi', Symbol);
    If Result.Name=Symbol then break;
    Result.Name:='';
  end;
end;
*)

procedure TIncWriter.WriteClass(AClass: TPasClassType);
var
  i: Integer;
  Member: TPasElement;
  LastVisibility, CurVisibility: TPasMemberVisibility;
begin
  PrepareDeclSection('type');
  wrt(AClass.Name + ' = ');
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

procedure TIncWriter.WriteVariable(AVar: TPasVariable);
begin
  if (AVar.Parent.ClassType <> TPasClassType) and
    (AVar.Parent.ClassType <> TPasRecordType) then
    PrepareDeclSection('var');
  wrt(AVar.Name + ': ');
  WriteType(AVar.VarType, false);
  wrtln(';');
end;

function ConvertToC(S: String): String;
var
  p: integer;
begin

  While pos('$', S)>0 do
  begin
    p:=pos('$', S);
    S:=Copy(S, 1, p-1)+'0'+Copy(S, p+1, Length(S)-p)+'H';
  end;

  While pos(' or ', S)>0 do
  begin
    p:=pos(' or ', S);
    S:=Copy(S, 1, p-1)+' | '+Copy(S, p+4, Length(S)-p-1);
  end;

  while pos(' shl ', S)>0 do
  begin
    p:=pos(' shl ', S);
    S:=Copy(S, 1, p-1)+' << '+Copy(S, p+5, Length(S)-p-1);
  end;

  while pos('not ', S)>0 do
  begin
    p:=pos('not ', S);
    S:=Copy(S, 1, p-1)+'~'+Copy(S, p+4, Length(S)-p-1);
  end;

  Result:=S;
end;

procedure TIncWriter.WriteConstant(AVar: TPasConst);
begin
  if (AVar.Parent.ClassType <> TPasClassType) and
    (AVar.Parent.ClassType <> TPasRecordType) then
  begin
    wrt(AVar.Name+' EQU	');
    if (trim(AVar.Value)[1]='-') then wrt('(');
    Wrt(ConvertToC(AVar.Value));
    if (trim(AVar.Value)[1]='-') then wrt(')');
    Wrtln('');
  end;
end;

procedure TIncWriter.WriteProcDecl(AProc: TPasProcedure);
var
  i: Integer;
  ABI: TABI;
begin
{
  if Assigned(AProc.ProcType) and
    (AProc.ProcType.ClassType = TPasFunctionType) then
  begin
    WriteType(TPasFunctionType(AProc.ProcType).ResultEl.ResultType, false);
  end else begin
    wrt('VOID');
  end;
}

  ABI:=AbiGet(AProc.Name);
  If ABI.Name='' then raise Exception.Create('ABI not found for '+AProc.Name);
  wrt(AProc.Name+' PROTO '+ABI.CallingConvertion+' ');

  if Assigned(AProc.ProcType) and (AProc.ProcType.Args.Count > 0) then
  begin
    for i := 0 to AProc.ProcType.Args.Count - 1 do
      with TPasArgument(AProc.ProcType.Args[i]) do
      begin
        if i > 0 then
          wrt(', ');

		// @todo: add all MASM keywords
		if (UpCase(Name)='FWAIT') or
		   (UpCase(Name)='STR') or
		   (UpCase(Name)='CX') then wrt('_');

        wrt(Name+':');
        case Access of
          argIn:    wrt('');
          argInOut: wrt(' PTR ');
          argOut:   wrt(' PTR ');
        end;
        if Assigned(ArgType) then
        begin
          WriteType(ArgType, false);
//          wrt(' ');
        end;
      end;
//  end else begin
//    wrt('(VOID)');
  end;

  wrtln;
end;

procedure TIncWriter.WriteProcedureType(AProc: TPasProcedureType);
var
  i: Integer;
  ABI: TABI;
begin
  ABI:=AbiGet(AProc.Name);
  if ABI.Name='' then raise Exception.Create('No ABI found for '+AProc.Name);
  wrt('VOID ('+ABI.CallingConvertion+' '+AProc.Name+')');

  if (AProc.Args.Count > 0) then
  begin
    wrt('(');
    for i := 0 to AProc.Args.Count - 1 do
      with TPasArgument(AProc.Args[i]) do
      begin
        if i > 0 then
          wrt(', ');
        case Access of
          argIn:    wrt('');
          argInOut: wrt('');
          argOut: wrt('');
        end;
        case Access of
          argIn:    wrt('');
          argInOut: wrt(' PTR ');
          argOut:   wrt(' PTR ');
        end;
        if Assigned(ArgType) then
        begin
          WriteType(ArgType, false);
          wrt(' ');
        end;
        wrt(Name);
      end;
    wrt(')');
  end else begin
    wrt('(VOID)');
  end;

  wrt(';');

  wrtln;
end;

procedure TIncWriter.WriteFunctionType(AProc: TPasFunctionType);
var
  i: Integer;
  ABI: TAbi;
begin
  WriteType(AProc.ResultEl.ResultType, false);

  ABI:=AbiGet(AProc.Name);
  if ABI.Name='' then raise Exception.Create('No ABI found for '+AProc.Name);
  wrt('('+ABI.CallingConvertion+' '+AProc.Name+')');

  if (AProc.Args.Count > 0) then
  begin
    wrt('(');
    for i := 0 to AProc.Args.Count - 1 do
      with TPasArgument(AProc.Args[i]) do
      begin
        if i > 0 then
          wrt(', ');
        case Access of
          argIn: wrt('');
          argInOut: wrt('');
          argOut: wrt('');
        end;
        case Access of
          argIn: wrt('');
          argInOut: wrt(' PTR ');
          argOut: wrt(' PTR ');
        end;
        if Assigned(ArgType) then
        begin
          WriteType(ArgType, false);
          wrt(' ');
        end;
        wrt(Name);
      end;
    wrt(')');
  end else begin
    wrt('(VOID)');
  end;

  wrt(';');

  wrtln;
end;


procedure TIncWriter.WriteProperty(AProp: TPasProperty);
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
    WriteType(AProp.VarType, false);
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


procedure TIncWriter.IncIndent;
begin
  Indent := Indent + '  ';
end;

procedure TIncWriter.DecIndent;
begin
  if Indent = '' then
    raise Exception.Create('Internal indent error');
  SetLength(Indent, Length(Indent) - 2);
end;

procedure TIncWriter.IncDeclSectionLevel;
var
  El: PDeclSectionStackElement;
begin
  New(El);
  DeclSectionStack.Add(El);
  El^.LastDeclSection := CurDeclSection;
  El^.LastIndent := Indent;
  CurDeclSection := '';
end;

procedure TIncWriter.DecDeclSectionLevel;
var
  El: PDeclSectionStackElement;
begin
  El := PDeclSectionStackElement(DeclSectionStack[DeclSectionStack.Count - 1]);
  DeclSectionStack.Delete(DeclSectionStack.Count - 1);
  CurDeclSection := El^.LastDeclSection;
  Indent := El^.LastIndent;
  Dispose(El);
end;

procedure TIncWriter.PrepareDeclSection(const ADeclSection: string);
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

procedure TIncWriter.WriteRecordType(AElement: TPasRecordType; NestingLevel: Integer);
var
  i: Integer;
  Variable: TPasVariable;
  Typ: TPasType;
  ABI: TABI;
  Prefix: ShortString;
begin
  Prefix:='';

  wrt(AElement.Name);

  if not (AElement.Parent is TPasVariant) then 
  begin
    wrtln(' STRUC');
    ABI:=AbiGet(AElement.Name);
    If ABI.Name<>'' then 
	begin
      Prefix:=ABI.Prefix+'_';
	end else raise Exception.Create('No ABI found for '+AElement.Name);
  end;

  for i := 0 to AElement.Members.Count - 1 do
  begin
    Variable:=TPasVariable(AElement.Members[i]);

    if Variable.VarType.ClassType = TPasArrayType then
    begin
      wrtln(ConvertToASMType(TPasArrayType(Variable.VarType).ElType.Name)+' '+Variable.Name+'[' + TPasArrayType(Variable.VarType).IndexRange + '];');
    end else begin
      if Variable.VarType.ClassType = TPasPointerType then
	  if (AElement.Name=TPasPointerType(Variable.VarType).DestType.Name) then wrt('struct ');
	  
//      WriteType(TPasType(Variable.VarType), false);
      wrt(Prefix+Variable.Name+' '); //{'+Variable.VarType.ClassName+'}

      wrt(ConvertToASMType(Variable.VarType.Name));
      
      wrtln(' ?');
    end;
  end;
  wrtln(AElement.Name+' ENDS');
end;

procedure WriteIncFile(AElement: TPasElement; const AFilename: string);
var
  Stream: TFileStream;
begin
  Stream := TFileStream.Create(AFilename, fmCreate);
  try
    WriteIncFile(AElement, Stream);
  finally
    Stream.Free;
  end;
end;

procedure WriteIncFile(AElement: TPasElement; AStream: TStream);
var
  Writer: TIncWriter;
begin
  Writer := TIncWriter.Create(AStream);
  try
    Writer.WriteElement(AElement);
  finally
    Writer.Free;
  end;
end;

end.
