{
    This file is part of the Free Component Library

    Pascal parse tree classes
    Copyright (c) 2000-2005 by
      Areca Systems GmbH / Sebastian Guenther, sg@freepascal.org

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}


unit PasTree;

interface

uses Classes;

resourcestring
  // Parse tree node type names
  SPasTreeElement = 'generic element';
  SPasTreeSection = 'unit section';
  SPasTreeModule = 'module';
  SPasTreePackage = 'package';
  SPasTreeType = 'generic type';
  SPasTreePointerType = 'pointer type';
  SPasTreeAliasType = 'alias type';
  SPasTreeTypeAliasType = '"type" alias type';
  SPasTreeRangeType = 'range type';
  SPasTreeArrayType = 'array type';
  SPasTreeFileType = 'file type';
  SPasTreeEnumValue = 'enumeration value';
  SPasTreeEnumType = 'enumeration type';
  SPasTreeSetType = 'set type';
  SPasTreeRecordType = 'record type';
  SPasTreeArgument = 'argument';
  SPasTreeProcedureType = 'procedure type';
  SPasTreeResultElement = 'function result';
  SPasTreeFunctionType = 'function type';
  SPasTreeUnresolvedTypeRef = 'unresolved type reference';
  SPasTreeVariable = 'variable';
  SPasTreeConst = 'constant';
  SPasTreeProcedure = 'procedure';
  SPasTreeFunction = 'function';
  SPasTreeProcedureImpl = 'procedure/function implementation';
  SPasTreeConstructorImpl = 'constructor implementation';
  SPasTreeDestructorImpl = 'destructor implementation';

type

  TPasModule = class;

  TPasMemberVisibility = (visDefault, visPrivate, visProtected, visPublic,
    visPublished, visAutomated);

  TPasMemberVisibilities = set of TPasMemberVisibility;


  TPTreeElement = class of TPasElement;

  TPasElement = class
  private
    FRefCount: LongWord;
    FName: string;
    FParent: TPasElement;
  public
    Visibility: TPasMemberVisibility;
    SourceFilename: string;
    SourceLinenumber: Integer;
    constructor Create(const AName: string; AParent: TPasElement); virtual;
    procedure AddRef;
    procedure Release;
    function FullName: string;          // Name including parent's names
    function PathName: string;          // = Module.Name + FullName
    function GetModule: TPasModule;
    function ElementTypeName: string; virtual;
    function GetDeclaration(full : Boolean) : string; virtual;
    property RefCount: LongWord read FRefCount;
    property Name: string read FName write FName;
    property Parent: TPasElement read FParent;
  end;

  TPasComment = class(TPasElement)
  public
    function ElementTypeName: string; override;
  end;

  TPasSection = class(TPasElement)
  public
    Declarations, Types, Consts,
      Functions, Variables: TList;
    constructor Create(const AName: string; AParent: TPasElement); override;
    destructor Destroy; override;
    function ElementTypeName: string; override;
  end;

  TPasModule = class(TPasElement)
  public
    PackageName: string;
    InterfaceSection, ImplementationSection: TPasSection;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasGroup = class(TPasElement)
  public
    InterfaceSection: TPasSection;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full: boolean): string; override;
  end;

  TPasPackage = class(TPasElement)
  public
    Modules: TList;     // List of TPasModule objects
    constructor Create(const AName: string; AParent: TPasElement); override;
    destructor Destroy; override;
    function ElementTypeName: string; override;
  end;

  TPasType = class(TPasElement)
  public
    function ElementTypeName: string; override;
  end;

  TPasPointerType = class(TPasType)
  public
    DestType: TPasType;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : Boolean): string; override;
  end;

  TPasAliasType = class(TPasType)
  public
    DestType: TPasType;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : Boolean): string; override;
  end;

  TPasTypeAliasType = class(TPasAliasType)
  public
    function ElementTypeName: string; override;
  end;

  TPasRangeType = class(TPasType)
  public
    RangeStart, RangeEnd: string;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasArrayType = class(TPasType)
  public
    IndexRange : string;
    ElType: TPasType;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasFileType = class(TPasType)
  public
    ElType: TPasType;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasEnumValue = class(TPasElement)
  public
    IsValueUsed: Boolean;
    Value: Integer;
    AssignedValue : string;
    function ElementTypeName: string; override;
  end;

  TPasEnumType = class(TPasType)
  public
    Values: TList;      // List of TPasEnumValue objects
    constructor Create(const AName: string; AParent: TPasElement); override;
    destructor Destroy; override;
     function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
    Procedure GetEnumNames(Names : TStrings);
  end;

  TPasSetType = class(TPasType)
  public
    EnumType: TPasType;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasRecordType = class;

  TPasVariant = class(TPasElement)
  public
    Values: TStringList;
    Members: TPasRecordType;
    constructor Create(const AName: string; AParent: TPasElement); override;
    destructor Destroy; override;
  end;

  TPasRecordType = class(TPasType)
  public
    Members: TList;     // array of TPasVariable elements
    VariantName: string;
    VariantType: TPasType;
    Variants: TList;    // array of TPasVariant elements, may be nil!
    constructor Create(const AName: string; AParent: TPasElement); override;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;



  TArgumentAccess = (argDefault, argIn, argInOut, argOut);

  TPasArgument = class(TPasElement)
  public
    Access: TArgumentAccess;
    ArgType: TPasType;
    Value: string;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasProcedureType = class(TPasType)
  public
    Args: TList;        // List of TPasArgument objects
    IsOfObject: Boolean;
    constructor Create(const AName: string; AParent: TPasElement); override;
    destructor Destroy; override;
    class function TypeName: string; virtual;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
    procedure GetArguments(List : TStrings);
    function CreateArgument(const AName, AUnresolvedTypeName: string):
      TPasArgument;
  end;

  TPasResultElement = class(TPasElement)
  public
    ResultType: TPasType;
    destructor Destroy; override;
    function ElementTypeName : string; override;
  end;

  TPasFunctionType = class(TPasProcedureType)
  public
    ResultEl: TPasResultElement;
    destructor Destroy; override;
    class function TypeName: string; override;
    function ElementTypeName: string; override;
    function GetDeclaration(Full : boolean) : string; override;
  end;

  TPasUnresolvedTypeRef = class(TPasType)
  public
    // Typerefs cannot be parented! -> AParent _must_ be NIL
    constructor Create(const AName: string; AParent: TPasElement); override;
    function ElementTypeName: string; override;
  end;

  TPasTypeRef = class(TPasUnresolvedTypeRef)
  public
    // function GetDeclaration(full : Boolean): string; override;
    RefType: TPasType;
  end;

  TPasVariable = class(TPasElement)
  public
    VarType: TPasType;
    Value: string;
    Modifiers : string;
    InlineComment: string;   // комментарий после объявления поля (для H2INC)
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function GetDeclaration(full : boolean) : string; override;
  end;

  TPasConst = class(TPasVariable)
  public
    function ElementTypeName: string; override;
  end;

  TPasProcedureBase = class(TPasElement)
  public
    function TypeName: string; virtual; abstract;
  end;

  TPasProcedure = class(TPasProcedureBase)
  public
    IsVirtual, IsDynamic, IsAbstract, IsOverride,
      IsOverload, IsMessage, isReintroduced, isStatic: Boolean;
    ProcType: TPasProcedureType;
    destructor Destroy; override;
    function ElementTypeName: string; override;
    function TypeName: string; override;
    function GetDeclaration(full: Boolean): string; override;
    procedure GetModifiers(List: TStrings);
  end;

  TPasFunction = class(TPasProcedure)
  public
    function ElementTypeName: string; override;
    function GetDeclaration (full : boolean) : string; override;
  end;


const
  AccessNames: array[TArgumentAccess] of string[6] = ('', 'const ', 'var ', 'out ');

implementation

uses SysUtils;


{ Parse tree element type name functions }

function TPasElement.ElementTypeName: string; begin Result := SPasTreeElement end;
function TPasComment.ElementTypeName: string; begin Result := 'comment'; end;
function TPasSection.ElementTypeName: string; begin Result := SPasTreeSection end;
function TPasModule.ElementTypeName: string; begin Result := SPasTreeModule end;
function TPasPackage.ElementTypeName: string; begin Result := SPasTreePackage end;
function TPasType.ElementTypeName: string; begin Result := SPasTreeType end;
function TPasPointerType.ElementTypeName: string; begin Result := SPasTreePointerType end;
function TPasAliasType.ElementTypeName: string; begin Result := SPasTreeAliasType end;
function TPasTypeAliasType.ElementTypeName: string; begin Result := SPasTreeTypeAliasType end;
function TPasRangeType.ElementTypeName: string; begin Result := SPasTreeRangeType end;
function TPasArrayType.ElementTypeName: string; begin Result := SPasTreeArrayType end;
function TPasFileType.ElementTypeName: string; begin Result := SPasTreeFileType end;
function TPasEnumValue.ElementTypeName: string; begin Result := SPasTreeEnumValue end;
function TPasEnumType.ElementTypeName: string; begin Result := SPasTreeEnumType end;
function TPasSetType.ElementTypeName: string; begin Result := SPasTreeSetType end;
function TPasRecordType.ElementTypeName: string; begin Result := SPasTreeRecordType end;
function TPasArgument.ElementTypeName: string; begin Result := SPasTreeArgument end;
function TPasProcedureType.ElementTypeName: string; begin Result := SPasTreeProcedureType end;
function TPasResultElement.ElementTypeName: string; begin Result := SPasTreeResultElement end;
function TPasFunctionType.ElementTypeName: string; begin Result := SPasTreeFunctionType end;
function TPasUnresolvedTypeRef.ElementTypeName: string; begin Result := SPasTreeUnresolvedTypeRef end;
function TPasVariable.ElementTypeName: string; begin Result := SPasTreeVariable end;
function TPasConst.ElementTypeName: string; begin Result := SPasTreeConst end;
function TPasProcedure.ElementTypeName: string; begin Result := SPasTreeProcedure end;
function TPasFunction.ElementTypeName: string; begin Result := SPasTreeFunction end;



{ All other stuff: }


constructor TPasElement.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create;
  FName := AName;
  FParent := AParent;
end;

procedure TPasElement.AddRef;
begin
  Inc(FRefCount);
end;

procedure TPasElement.Release;
begin
  if FRefCount = 0 then
    Free
  else
    Dec(FRefCount);
end;

function TPasElement.FullName: string;
var
  p: TPasElement;
begin
  Result := Name;
  p := Parent;
  while Assigned(p) and not p.InheritsFrom(TPasSection) do
  begin
    if (Length(p.Name) > 0) then
      if Length(Result) > 0 then
        Result := p.Name + '.' + Result
      else
        Result := p.Name;
    p := p.Parent;
  end;
end;

function TPasElement.PathName: string;
var
  p: TPasElement;
begin
  Result := Name;
  p := Parent;
  while Assigned(p) do
  begin
    if (Length(p.Name) > 0) then
      if Length(Result) > 0 then
        Result := p.Name + '.' + Result
      else
        Result := p.Name;
    p := p.Parent;
  end;
end;

function TPasElement.GetModule: TPasModule;
begin
  if ClassType = TPasPackage then
    Result := nil
  else
  begin
    Result := TPasModule(Self);
    while Assigned(Result) and not (Result.ClassType = TPasModule) do
      Result := TPasModule(Result.Parent);
  end;
end;

function TPasElement.GetDeclaration (full : boolean): string;

begin
  if Full then
    Result := Name
  else
    Result := '';
end;

constructor TPasSection.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create(AName, AParent);
  Declarations := TList.Create;
  Types := TList.Create;
  Consts := TList.Create;
  Functions := TList.Create;
  Variables := TList.Create;
end;

destructor TPasSection.Destroy;
var
  i: Integer;
begin
  Variables.Free;
  Functions.Free;
  Consts.Free;
  Types.Free;

  for i := 0 to Declarations.Count - 1 do
    TPasElement(Declarations[i]).Release;
  Declarations.Free;


  inherited Destroy;
end;


destructor TPasModule.Destroy;
begin
  if Assigned(InterfaceSection) then
    InterfaceSection.Release;
  if Assigned(ImplementationSection) then
    ImplementationSection.Release;
  inherited Destroy;
end;


destructor TPasGroup.Destroy;
begin
  if Assigned(InterfaceSection) then
    InterfaceSection.Release;
  inherited Destroy;
end;

function TPasGroup.ElementTypeName: string;
begin
  Result := 'group';
end;

function TPasGroup.GetDeclaration(full: boolean): string;
begin
  Result := Name;
end;

constructor TPasPackage.Create(const AName: string; AParent: TPasElement);
begin
  if (Length(AName) > 0) and (AName[1] <> '#') then
    inherited Create('#' + AName, AParent)
  else
    inherited Create(AName, AParent);
  Modules := TList.Create;
end;

destructor TPasPackage.Destroy;
var
  i: Integer;
begin
  for i := 0 to Modules.Count - 1 do
    TPasModule(Modules[i]).Release;
  Modules.Free;
  inherited Destroy;
end;


destructor TPasPointerType.Destroy;
begin
  if Assigned(DestType) then
    DestType.Release;
  inherited Destroy;
end;


destructor TPasAliasType.Destroy;
begin
  if Assigned(DestType) then
    DestType.Release;
  inherited Destroy;
end;


destructor TPasArrayType.Destroy;
begin
  if Assigned(ElType) then
    ElType.Release;
  inherited Destroy;
end;

destructor TPasFileType.Destroy;
begin
  if Assigned(ElType) then
    ElType.Release;
  inherited Destroy;
end;


constructor TPasEnumType.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create(AName, AParent);
  Values := TList.Create;
end;

destructor TPasEnumType.Destroy;
var
  i: Integer;
begin
  for i := 0 to Values.Count - 1 do
    TPasEnumValue(Values[i]).Release;
  Values.Free;
  inherited Destroy;
end;

procedure TPasEnumType.GetEnumNames(Names: TStrings);
var
  i: Integer;
begin
  with Values do
  begin
    for i := 0 to Count - 2 do
      Names.Add(TPasEnumValue(Items[i]).Name + ',');
    if Count > 0 then
      Names.Add(TPasEnumValue(Items[Count - 1]).Name);
  end;
end;


destructor TPasSetType.Destroy;
begin
  if Assigned(EnumType) then
    EnumType.Release;
  inherited Destroy;
end;


constructor TPasVariant.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create(AName, AParent);
  Values := TStringList.Create;
end;

destructor TPasVariant.Destroy;
begin
  Values.Free;
  if Assigned(Members) then
    Members.Release;
  inherited Destroy;
end;


constructor TPasRecordType.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create(AName, AParent);
  Members := TList.Create;
end;

destructor TPasRecordType.Destroy;
var
  i: Integer;
begin
  for i := 0 to Members.Count - 1 do
    TPasVariable(Members[i]).Release;
  Members.Free;

  if Assigned(VariantType) then
    VariantType.Release;

  if Assigned(Variants) then
  begin
    for i := 0 to Variants.Count - 1 do
      TPasVariant(Variants[i]).Release;
    Variants.Free;
  end;

  inherited Destroy;
end;



destructor TPasArgument.Destroy;
begin
  if Assigned(ArgType) then
    ArgType.Release;
  inherited Destroy;
end;


constructor TPasProcedureType.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create(AName, AParent);
  Args := TList.Create;
end;

destructor TPasProcedureType.Destroy;
var
  i: Integer;
begin
  for i := 0 to Args.Count - 1 do
    TPasArgument(Args[i]).Release;
  Args.Free;
  inherited Destroy;
end;

class function TPasProcedureType.TypeName: string;
begin
  Result := 'procedure';
end;

function TPasProcedureType.CreateArgument(const AName,
  AUnresolvedTypeName: string): TPasArgument;
begin
  Result := TPasArgument.Create(AName, Self);
  Args.Add(Result);
  Result.ArgType := TPasUnresolvedTypeRef.Create(AUnresolvedTypeName, Result);
end;


destructor TPasResultElement.Destroy;
begin
  if Assigned(ResultType) then
    ResultType.Release;
  inherited Destroy;
end;


destructor TPasFunctionType.Destroy;
begin
  if Assigned(ResultEl) then
    ResultEl.Release;
  inherited Destroy;
end;


class function TPasFunctionType.TypeName: string;
begin
  Result := 'function';
end;


constructor TPasUnresolvedTypeRef.Create(const AName: string; AParent: TPasElement);
begin
  inherited Create(AName, nil);
end;


destructor TPasVariable.Destroy;
begin
  { Attention, in derived classes, VarType isn't necessarily set!
    (e.g. in Constants) }
  if Assigned(VarType) then
    VarType.Release;
  inherited Destroy;
end;






destructor TPasProcedure.Destroy;
begin
  if Assigned(ProcType) then
    ProcType.Release;
  inherited Destroy;
end;

function TPasProcedure.TypeName: string;
begin
  Result := ProcType.TypeName;
end;




{ ---------------------------------------------------------------------

  ---------------------------------------------------------------------}

function TPasModule.GetDeclaration(full : boolean): string;
begin
  Result := 'Unit ' + Name;
end;

function TPasPointerType.GetDeclaration (full : boolean) : string;
begin
  Result:='^'+DestType.Name;
  If Full then
    Result:=Name+' = '+Result;
end;

function TPasAliasType.GetDeclaration (full : boolean) : string;
begin
  Result:=DestType.Name;
  If Full then
    Result:=Name+' = '+Result;
end;

function TPasRangeType.GetDeclaration (full : boolean) : string;
begin
  Result:=RangeStart+'..'+RangeEnd;
  If Full then
    Result:=Name+' = '+Result;
end;

function TPasArrayType.GetDeclaration (full : boolean) : string;
begin
  Result:='Array['+IndexRange+'] of ';
  If Assigned(Eltype) then
    Result:=Result+ElType.Name
  else
    Result:=Result+'const';
  If Full Then
    Result:=Name+' = '+Result;
end;

function TPasFileType.GetDeclaration (full : boolean) : string;
begin
  Result:='File of ';
  If Assigned(Eltype) then
    Result:=Result+ElType.Name;
  If Full Then
    Result:=Name+' = '+Result;
end;

Function IndentStrings(S : TStrings; indent : Integer) : string;

Var
  I,CurrLen,CurrPos : Integer;


begin
  Result:='';
  CurrLen:=0;
  CurrPos:=0;
  For I:=0 to S.Count-1 do
    begin
    CurrLen:=Length(S[i]);
    If (CurrLen+CurrPos)>72 then
      begin
      Result:=Result+LineEnding+StringOfChar(' ',Indent);
      CurrPos:=Indent;
      end;
    Result:=Result+S[i];
    CurrPos:=CurrPos+CurrLen;
    end;
end;

function TPasEnumType.GetDeclaration (full : boolean) : string;

Var
  S : TStringList;

begin
  S:=TStringList.Create;
  Try
    If Full then
      S.Add(Name+' = (')
    else
      S.Add('(');
    GetEnumNames(S);
    S[S.Count-1]:=S[S.Count-1]+')';
    If Full then
      Result:=IndentStrings(S,Length(Name)+4)
    else
      Result:=IndentStrings(S,1);
  finally
    S.Free;
  end;
end;

function TPasSetType.GetDeclaration (full : boolean) : string;

Var
  S : TStringList;
  i : Integer;

begin
  If EnumType is TPasEnumType then
    begin
    S:=TStringList.Create;
    Try
      If Full then
        S.Add(Name+'= Set of (')
      else
        S.Add('Set of (');
      TPasEnumType(EnumType).GetEnumNames(S);
      S[S.Count-1]:=S[S.Count-1]+')';
      I:=Pos('(',S[0]);
      Result:=IndentStrings(S,i);
    finally
      S.Free;
    end;
    end
  else
    begin
    Result:='Set of '+EnumType.Name;
    If Full then
      Result:=Name+' = '+Result;
    end;
end;

function TPasRecordType.GetDeclaration (full : boolean) : string;

Var
  S,T : TStringList;
  temp : string;
  I,J : integer;

begin
  S:=TStringList.Create;
  T:=TstringList.Create;
  Try
    Temp:='record';
    If Full then
      Temp:=Name+' = '+Temp;
    S.Add(Temp);
    For I:=0 to Members.Count-1 do
      begin
      Temp:=TPasVariable(Members[i]).GetDeclaration(True);
      If Pos(LineEnding,Temp)>0 then
        begin
        T.Text:=Temp;
        For J:=0 to T.Count-1 do
          if J=T.Count-1 then
            S.Add('  '+T[J]+';')
          else
            S.Add('  '+T[J])
        end
      else
        S.Add('  '+Temp+';');
      end;
    S.Add('end');
    Result:=S.Text;
  finally
    S.free;
    T.free;
  end;
end;

procedure TPasProcedureType.GetArguments(List : TStrings);

Var
  T : string;
  I : Integer;

begin
  For I:=0 to Args.Count-1 do
    begin
    T:=AccessNames[TPasArgument(Args[i]).Access];
    T:=T+TPasArgument(Args[i]).GetDeclaration(True);
    If I=0 then
      T:='('+T;
    If I<Args.Count-1 then
      List.Add(T+';')
    else
      List.Add(T+')');
    end;
end;

function TPasProcedureType.GetDeclaration (full : boolean) : string;

Var
  S : TStringList;

begin
  S:=TStringList.Create;
  Try
    If Full then
      S.Add(Format('%s = ',[Name]));
    S.Add(TypeName);
    GetArguments(S);
    If IsOfObject then
      S.Add(' of object');
    If Full then
      Result:=IndentStrings(S,Length(S[0])+Length(S[1])+1)
    else
      Result:=IndentStrings(S,Length(S[0])+1);
  finally
    S.Free;
  end;
end;

function TPasFunctionType.GetDeclaration (full : boolean) : string;

Var
  S : TStringList;
  T : string;

begin
  S:=TStringList.Create;
  Try
    If Full then
      S.Add(Format('%s = ',[Name]));
    S.Add(TypeName);
    GetArguments(S);
    If Assigned(ResultEl) then
      begin
      T:=' : ';
      If (ResultEl.ResultType.Name<>'') then
        T:=T+ResultEl.ResultType.Name
      else
        T:=T+ResultEl.ResultType.GetDeclaration(False);
      S.Add(T);
      end;
    If IsOfObject then
      S.Add(' of object');
    If Full then
      Result:=IndentStrings(S,Length(S[0])+Length(S[1])+1)
    else
      Result:=IndentStrings(S,Length(S[0])+1);
  finally
    S.Free;
  end;
end;

function TPasVariable.GetDeclaration (full : boolean) : string;

Const
 Seps : Array[Boolean] of Char = ('=',':');

begin
  If Assigned(VarType) then
    begin
    If VarType.Name='' then
      Result:=VarType.GetDeclaration(False)
    else
      Result:=VarType.Name;
    Result:=Result+Modifiers;
    if (Value<>'') then
      Result:=Result+' = '+Value;
    end
  else
    Result:=Value;
  If Full then
    Result:=Name+' '+Seps[Assigned(VarType)]+' '+Result;
end;


Procedure TPasProcedure.GetModifiers(List : TStrings);

  Procedure DoAdd(B : Boolean; S : string);

  begin
    if B then
      List.add('; '+S);
  end;

begin
  Doadd(IsVirtual,' Virtual');
  DoAdd(IsDynamic,' Dynamic');
  DoAdd(IsOverride,' Override');
  DoAdd(IsAbstract,' Abstract');
  DoAdd(IsOverload,' Overload');
  DoAdd(IsReintroduced,' Reintroduce');
  DoAdd(IsStatic,' Static');
  DoAdd(IsMessage,' Message');
end;

function TPasProcedure.GetDeclaration (full : boolean) : string;

Var
  S : TStringList;
begin
  S:=TStringList.Create;
  try
    If Full then
      S.Add(TypeName+' '+Name);
    ProcType.GetArguments(S);
    GetModifiers(S);
    Result:=IndentStrings(S,Length(S[0]));
  finally
    S.Free;
  end;
end;

function TPasFunction.GetDeclaration (full : boolean) : string;

Var
  S : TStringList;
  T : string;

begin
  S:=TStringList.Create;
  try
    If Full then
      S.Add(TypeName+' '+Name);
    ProcType.GetArguments(S);
    If Assigned((Proctype as TPasFunctionType).ResultEl) then
      With TPasFunctionType(ProcType).ResultEl.ResultType do
        begin
        T:=' : ';
        If (Name<>'') then
          T:=T+Name
        else
          T:=T+GetDeclaration(False);
        S.Add(T);
        end;
    GetModifiers(S);
    Result:=IndentStrings(S,Length(S[0]));
  finally
    S.Free;
  end;
end;



function TPasArgument.GetDeclaration (full : boolean) : string;
begin
  If Assigned(ArgType) then
    begin
    If ArgType.Name<>'' then
      Result:=ArgType.Name
    else
      Result:=ArgType.GetDeclaration(False);
    If Full then
      Result:=Name+': '+Result;
    end
  else If Full then
    Result:=Name
  else
    Result:='';
end;

end.
