{
    This file is part of the Free Component Library

    Pascal source parser
    Copyright (c) 2000-2005 by
      Areca Systems GmbH / Sebastian Guenther, sg@freepascal.org

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}


unit PParser;

interface

uses SysUtils, PasTree, PScanner;

resourcestring
  SErrNoSourceGiven = 'No source file specified';
  SErrMultipleSourceFiles = 'Please specify only one source file';
  SParserError = 'Error';
  SParserErrorAtToken = '%s at token "%s"';
  SParserUngetTokenError = 'Internal error: Cannot unget more tokens, history buffer is full';
  SParserExpectTokenError = 'Expected "%s"';
  SParserExpectedCommaRBracket = 'Expected "," or ")"';
  SParserExpectedCommaSemicolon = 'Expected "," or ";"';
  SParserExpectedCommaColon = 'Expected "," or ":"';
  SParserExpectedLBracketColon = 'Expected "(" or ":"';
  SParserExpectedLBracketSemicolon = 'Expected "(" or ";"';
  SParserExpectedColonSemicolon = 'Expected ":" or ";"';
  SParserExpectedSemiColonEnd = 'Expected ";" or "End"';
  SParserExpectedConstVarID = 'Expected "in", "var" or identifier';
  SParserExpectedColonID = 'Expected ":" or identifier';
  SParserSyntaxError = 'Syntax error';
  SParserTypeSyntaxError = 'Syntax error in type';
  SParserArrayTypeSyntaxError = 'Syntax error in array type';
  SParserInterfaceTokenError = 'Invalid token in interface section of unit';
  SParserInvalidTypeDef = 'Invalid type definition';
  SParserNoDirection = 'Argument direction not set';

type
  TPasTreeContainer = class
  protected
    FPackage: TPasPackage;
  public
    function CreateElement(AClass: TPTreeElement; const AName: String;
      AParent: TPasElement; const ASourceFilename: String;
      ASourceLinenumber: Integer): TPasElement;
    function CreateElement(AClass: TPTreeElement; const AName: String;
      AParent: TPasElement; AVisibility: TPasMemberVisibility;
      const ASourceFilename: String; ASourceLinenumber: Integer): TPasElement;
      virtual; abstract;
    function CreateFunctionType(const AName, AResultName: String; AParent: TPasElement;
      UseParentAsResultParent: Boolean; const ASourceFilename: String;
      ASourceLinenumber: Integer): TPasFunctionType;
    function FindElement(const AName: String): TPasElement; virtual; abstract;
    function FindModule(const AName: String): TPasModule; virtual;
    property Package: TPasPackage read FPackage;
  end;

  EParserError = class(Exception)
  private
    FFilename: String;
    FRow, FColumn: Integer;
  public
    constructor Create(const AReason, AFilename: String;
      ARow, AColumn: Integer);
    property Filename: String read FFilename;
    property Row: Integer read FRow;
    property Column: Integer read FColumn;
  end;


function ParseSource(AEngine: TPasTreeContainer;
  const FPCCommandLine, OSTarget, CPUTarget: String): TPasModule;


implementation

uses Classes;

type

  TDeclType = (declNone, declConst, declType, declVar, declThreadvar);

  TProcType = (ptProcedure, ptFunction, ptOperator);

  TPasParser = class
  private
    FFileResolver: TFileResolver;
    FScanner: TPascalScanner;
    FEngine: TPasTreeContainer;
    FCurToken: TToken;
    FCurTokenString: String;
    // UngetToken support:
    FTokenBuffer: array[0..1] of TToken;
    FTokenStringBuffer: array[0..1] of String;
    FTokenBufferIndex, FTokenBufferSize: Integer;

    procedure ParseExc(const Msg: String);
  protected
    function CreateElement(AClass: TPTreeElement; const AName: String;
      AParent: TPasElement): TPasElement;
    function CreateElement(AClass: TPTreeElement; const AName: String;
      AParent: TPasElement; AVisibility: TPasMemberVisibility): TPasElement;
  public
    Options : set of TPOptions;
    constructor Create(AScanner: TPascalScanner; AFileResolver: TFileResolver;
      AEngine: TPasTreeContainer);
    function CurTokenName: String;
    function CurTokenText: String;
    procedure NextToken;
    procedure UngetToken;
    procedure ExpectToken(tk: TToken);
    function ExpectIdentifier: String;

    function ParseType(Parent: TPasElement; Prefix : String): TPasType;
    function ParseType(Parent: TPasElement): TPasType;
    function ParseComplexType: TPasType;
    procedure ParseArrayType(Element: TPasArrayType);
    procedure ParseFileType(Element: TPasFileType);
    function ParseExpression: String;
    procedure AddProcOrFunction(ASection: TPasSection; AProc: TPasProcedure);
    function CheckIfOverloaded(AOwner: TPasClassType;
      const AName: String): TPasElement;

    procedure ParseMain(var Module: TPasModule);
    procedure ParseUnit(var Module: TPasModule);
    procedure ParseUsesList(ASection: TPasSection);
//    procedure ParseAbiList(ASection: TPasSection);
    procedure ParseError(ASection: TPasSection);
    function ParseConstDecl(Parent: TPasElement): TPasConst;
    function ParseTypeDecl(Parent: TPasElement): TPasType;
    procedure ParseInlineVarDecl(Parent: TPasElement; VarList: TList);
    procedure ParseInlineVarDecl(Parent: TPasElement; VarList: TList;
      AVisibility : TPasMemberVisibility; ClosingBrace: Boolean);
    procedure ParseVarDecl(Parent: TPasElement; List: TList);
    procedure ParseArgList(Parent: TPasElement; Args: TList; EndToken: TToken);
    procedure ParseProcedureOrFunctionHeader(Parent: TPasElement;
      Element: TPasProcedureType; ProcType: TProcType; OfObjectPossible: Boolean);
    function ParseProcedureOrFunctionDecl(Parent: TPasElement;
      ProcType: TProcType): TPasProcedure;
    procedure ParseRecordDecl(Parent: TPasRecordType; IsNested: Boolean);       // !!!: Optimize this. We have 3x the same wrapper code around it.
    function ParseClassDecl(Parent: TPasElement; const AClassName: String;
      AObjKind: TPasObjKind): TPasType;
    procedure ParseProperty(Element:TPasElement);

    property FileResolver: TFileResolver read FFileResolver;
    property Scanner: TPascalScanner read FScanner;
    property Engine: TPasTreeContainer read FEngine;

    property CurToken: TToken read FCurToken;
    property CurTokenString: String read FCurTokenString;
  end;


function TPasTreeContainer.CreateElement(AClass: TPTreeElement;
  const AName: String; AParent: TPasElement; const ASourceFilename: String;
  ASourceLinenumber: Integer): TPasElement;
begin
  Result := CreateElement(AClass, AName, AParent, visDefault, ASourceFilename,
    ASourceLinenumber);
end;

function TPasTreeContainer.CreateFunctionType(const AName, AResultName: String;
  AParent: TPasElement; UseParentAsResultParent: Boolean;
  const ASourceFilename: String; ASourceLinenumber: Integer): TPasFunctionType;
var
  ResultParent: TPasElement;
begin
  Result := TPasFunctionType(CreateElement(TPasFunctionType, AName, AParent,
    ASourceFilename, ASourceLinenumber));

  if UseParentAsResultParent then
    ResultParent := AParent
  else
    ResultParent := Result;

  TPasFunctionType(Result).ResultEl :=
    TPasResultElement(CreateElement(TPasResultElement, AResultName, ResultParent,
    ASourceFilename, ASourceLinenumber));
end;

function TPasTreeContainer.FindModule(const AName: String): TPasModule;
begin
  Result := nil;
end;

constructor EParserError.Create(const AReason, AFilename: String;
  ARow, AColumn: Integer);
begin
  inherited Create(AFilename+'('+IntToStr(ARow)+'): '+AReason);
  FFilename := AFilename;
  FRow := ARow;
  FColumn := AColumn;
end;

procedure TPasParser.ParseExc(const Msg: String);
begin
  raise EParserError.Create(Format(SParserErrorAtToken, [Msg, CurTokenName]),
    Scanner.CurFilename, Scanner.CurRow, Scanner.CurColumn);
end;

constructor TPasParser.Create(AScanner: TPascalScanner;
  AFileResolver: TFileResolver; AEngine: TPasTreeContainer);
begin
  inherited Create;
  FScanner := AScanner;
  FFileResolver := AFileResolver;
  FEngine := AEngine;
end;

function TPasParser.CurTokenName: String;
begin
  if CurToken = tkIdentifier then
    Result := 'Identifier ' + Scanner.CurTokenString
  else
    Result := TokenInfos[CurToken];
end;

function TPasParser.CurTokenText: String;
begin
  case CurToken of
    tkIdentifier, tkString, tkNumber, tkChar:
      Result := Scanner.CurTokenString;
    else
      Result := TokenInfos[CurToken];
  end;
end;

procedure TPasParser.NextToken;
begin
  if FTokenBufferIndex < FTokenBufferSize then
  begin
    // Get token from buffer
    FCurToken := FTokenBuffer[FTokenBufferIndex];
    FCurTokenString := FTokenStringBuffer[FTokenBufferIndex];
    Inc(FTokenBufferIndex);
  end else
  begin
    { We have to fetch a new token. But first check, wether there is space left
      in the token buffer.}
    if FTokenBufferSize = 2 then
    begin
      FTokenBuffer[0] := FTokenBuffer[1];
      FTokenStringBuffer[0] := FTokenStringBuffer[1];
      Dec(FTokenBufferSize);
      Dec(FTokenBufferIndex);
    end;
    // Fetch new token
    try
      repeat
        FCurToken := Scanner.FetchToken;
      until not (FCurToken in [tkWhitespace, tkComment]);
    except
      on e: EScannerError do
        raise EParserError.Create(e.Message,
          Scanner.CurFilename, Scanner.CurRow, Scanner.CurColumn);
    end;
    FCurTokenString := Scanner.CurTokenString;
    FTokenBuffer[FTokenBufferSize] := FCurToken;
    FTokenStringBuffer[FTokenBufferSize] := FCurTokenString;
    Inc(FTokenBufferSize);
    Inc(FTokenBufferIndex);
  end;
end;

procedure TPasParser.UngetToken;

begin
  if FTokenBufferIndex = 0 then
    ParseExc(SParserUngetTokenError)
  else
    Dec(FTokenBufferIndex);
end;


procedure TPasParser.ExpectToken(tk: TToken);
begin
  NextToken;
  if CurToken <> tk then
    ParseExc(Format(SParserExpectTokenError, [TokenInfos[tk]]));
end;

function TPasParser.ExpectIdentifier: String;
begin
  ExpectToken(tkIdentifier);
  Result := CurTokenString;
end;

function TPasParser.ParseType(Parent: TPasElement): TPasType;

begin
  Result:=ParseType(Parent,'');
end;

function TPasParser.ParseType(Parent: TPasElement; Prefix : String): TPasType;

  procedure ParseRange;
  begin
    Result := TPasRangeType(CreateElement(TPasRangeType, '', Parent));
    try
      TPasRangeType(Result).RangeStart := ParseExpression;
      ExpectToken(tkDotDot);
      TPasRangeType(Result).RangeEnd := ParseExpression;
    except
      Result.Free;
      raise;
    end;
  end;

var
  Name, s: String;
  EnumValue: TPasEnumValue;
  Ref: TPasElement;
begin
  Result := nil;         // !!!: Remove in the future
  NextToken;
  case CurToken of
    tkIdentifier:
      begin
        Name := CurTokenString;
        If (Prefix<>'') then
          Name:=Prefix+'.'+Name;
        NextToken;
        if CurToken = tkDot then
        begin
          ExpectIdentifier;
          Name := Name+'.'+CurTokenString;
        end else
          UngetToken;
        Ref := nil;
        s := UpperCase(Name);
        if      s = 'T_BYTE'    then Name := 'T_BYTE'
        else if s = 'T_BOOLEAN' then Name := 'T_BOOLEAN'
        else if s = 'T_CHAR'    then Name := 'T_CHAR'
        else if s = 'T_POINTER' then Name := 'T_POINTER'
        else if s = 'T_INT16'   then Name := 'T_INT16'
        else if s = 'T_INT32'   then Name := 'T_INT32'
        else if s = 'T_INT64'   then Name := 'T_INT64'
        else if s = 'T_DWORD'   then Name := 'T_DWORD'
        else if s = 'T_WORD'    then Name := 'T_WORD'
        else
          Ref := Engine.FindElement(Name);
        if Assigned(Ref) then
        begin
          {Result := TPasTypeRef(CreateElement(TPasTypeRef, Name, nil));
          TPasTypeRef(Result).RefType := Ref as TPasType;}
          Result := Ref as TPasType;
          Result.AddRef;
        end else
          Result := TPasUnresolvedTypeRef(CreateElement(TPasUnresolvedTypeRef, Name, nil));

        // !!!: Doesn't make sense for resolved types
        if Name = 'String' then
        begin
          NextToken;
          if CurToken = tkSquaredBraceOpen then
          begin
            // !!!: Parse the string length value and store it
            repeat
                  NextToken;
            until CurToken = tkSquaredBraceClose;
          end else
            UngetToken;
        end;
      end;
    tkCaret:
      begin
        Result := TPasPointerType(CreateElement(TPasPointerType, '', Parent));
        TPasPointerType(Result).DestType := ParseType(nil);
      end;
    tkFile:
      begin
        Result := TPasFileType(CreateElement(TPasFileType, '', Parent));
      end;
    tkArray:
      begin
        Result := TPasArrayType(CreateElement(TPasArrayType, '', Parent));
        ParseArrayType(TPasArrayType(Result));
      end;
    tkBraceOpen:
      begin
        Result := TPasEnumType(CreateElement(TPasEnumType, '', Parent));
        while True do
        begin
          NextToken;
          EnumValue := TPasEnumValue(CreateElement(TPasEnumValue,
            CurTokenString, Result));
          TPasEnumType(Result).Values.Add(EnumValue);
          NextToken;
          if CurToken = tkBraceClose then
            break
          else if CurToken in [tkEqual,tkAssign] then
            begin
            EnumValue.AssignedValue:=ParseExpression;
            NextToken;
            if CurToken = tkBraceClose then
              Break
            else if not (CurToken=tkComma) then
              ParseExc(SParserExpectedCommaRBracket);
            end
          else if not (CurToken=tkComma) then
            ParseExc(SParserExpectedCommaRBracket)
        end;
      end;
    tkSet:
      begin
        Result := TPasSetType(CreateElement(TPasSetType, '', Parent));
        try
          ExpectToken(tkOf);
          TPasSetType(Result).EnumType := ParseType(Result);
        except
          Result.Free;
          raise;
        end;
      end;
    tkRecord:
      begin
        Result := TPasRecordType(CreateElement(TPasRecordType, '', Parent));
        try
          ParseRecordDecl(TPasRecordType(Result), False);
        except
          Result.Free;
          raise;
        end;
      end;
    tkProcedure:
      begin
        Result := TPasProcedureType(
          CreateElement(TPasProcedureType, '', Parent));
        try
          ParseProcedureOrFunctionHeader(Result,
            TPasProcedureType(Result), ptProcedure, True);
        except
          Result.Free;
          raise;
        end;
      end;
    tkFunction:
      begin
        Result := Engine.CreateFunctionType('', 'Result', Parent, False,
          Scanner.CurFilename, Scanner.CurRow);
        try
          ParseProcedureOrFunctionHeader(Result,
            TPasFunctionType(Result), ptFunction, True);
        except
          Result.Free;
          raise;
        end;
      end;
    else
    begin
      UngetToken;
      ParseRange;
    end;
//      ParseExc(SParserTypeSyntaxError);
  end;
end;

function TPasParser.ParseComplexType: TPasType;
begin
  NextToken;
  case CurToken of
    tkProcedure:
      begin
        Result := TPasProcedureType(CreateElement(TPasProcedureType, '', nil));
        ParseProcedureOrFunctionHeader(Result,
          TPasProcedureType(Result), ptProcedure, True);
        UngetToken;        // Unget semicolon
      end;
    tkFunction:
      begin
        Result := Engine.CreateFunctionType('', 'Result', nil, False,
          Scanner.CurFilename, Scanner.CurRow);
        ParseProcedureOrFunctionHeader(Result,
          TPasFunctionType(Result), ptFunction, True);
        UngetToken;        // Unget semicolon
      end;
    else
    begin
      UngetToken;
      Result := ParseType(nil);
      exit;
    end;
  end;
end;

procedure TPasParser.ParseArrayType(Element: TPasArrayType);

Var
  S : String;

begin
  NextToken;
  S:='';
  case CurToken of
    tkSquaredBraceOpen:
      begin
        repeat
          NextToken;
          if CurToken<>tkSquaredBraceClose then
            S:=S+CurTokenText;
        until CurToken = tkSquaredBraceClose;
      Element.IndexRange:=S;
        ExpectToken(tkOf);
        Element.ElType := ParseType(nil);
      end;
    tkOf:
      begin
        NextToken;
        if CurToken = tkConst then
//          ArrayEl.AppendChild(Doc.CreateElement('const'))
        else
        begin
          UngetToken;
            Element.ElType := ParseType(nil);
        end
      end
    else
      ParseExc(SParserArrayTypeSyntaxError);
  end;
end;

procedure TPasParser.ParseFileType(Element: TPasFileType);


begin
  NextToken;
  If CurToken=tkOf then
    Element.ElType := ParseType(nil);
end;

function TPasParser.ParseExpression: String;
var
  BracketLevel: Integer;
  MayAppendSpace, AppendSpace, NextAppendSpace: Boolean;
begin
  SetLength(Result, 0);
  BracketLevel := 0;
  MayAppendSpace := False;
  AppendSpace := False;
  while True do
  begin
    NextToken;
    { !!!: Does not detect when normal brackets and square brackets are mixed
      in a wrong way. }
    if CurToken in [tkBraceOpen, tkSquaredBraceOpen] then
      Inc(BracketLevel)
    else if CurToken in [tkBraceClose, tkSquaredBraceClose] then
    begin
      if BracketLevel = 0 then
        break;
      Dec(BracketLevel);
    end else if (CurToken in [tkComma, tkSemicolon, tkColon, tkSquaredBraceClose,
      tkDotDot]) and (BracketLevel = 0) then
      break;

    if MayAppendSpace then
    begin
      NextAppendSpace := False;
      case CurToken of
        tkBraceOpen, tkBraceClose, tkDivision, tkEqual, tkCaret, tkAnd, tkAs,
          tkDiv, tkIs, tkMinus, tkMod, tkMul, tkNot, tkOf, tkOn,
          tkOr, tkPlus, tkSHL, tkSHR, tkXOR:
{        tkPlus.._ASSIGNMENT, _UNEQUAL, tkPlusASN.._XORASN, _AS, _AT, _IN, _IS,
          tkOf, _ON, _OR, _AND, _DIV, _MOD, _NOT, _SHL, _SHR, _XOR:}
          begin
            AppendSpace := True;
            NextAppendSpace := True;
          end;
      end;
      if AppendSpace then
        Result := Result + ' ';
      AppendSpace := NextAppendSpace;
    end else
      MayAppendSpace := True;
    if CurToken=tkString then
      begin
      If (Length(CurTokenText)>0) and (CurTokenText[1]=#0) then
        Writeln('First char is null : "',CurTokenText,'"');
      Result := Result + ''''+StringReplace(CurTokenText,'''','''''',[rfReplaceAll])+''''
      end
    else
      Result := Result + CurTokenText;
  end;
  UngetToken;
end;

procedure TPasParser.AddProcOrFunction(ASection: TPasSection;
  AProc: TPasProcedure);
var
  i: Integer;
  Member: TPasElement;
  OverloadedProc: TPasOverloadedProc;
begin
  for i := 0 to ASection.Functions.Count - 1 do
  begin
    Member := TPasElement(ASection.Functions[i]);
    if CompareText(Member.Name, AProc.Name) = 0 then
    begin
      if Member.ClassType = TPasOverloadedProc then
        TPasOverloadedProc(Member).Overloads.Add(AProc)
      else
      begin
        OverloadedProc := TPasOverloadedProc.Create(AProc.Name, ASection);
        OverloadedProc.Overloads.Add(Member);
        OverloadedProc.Overloads.Add(AProc);
        ASection.Functions[i] := OverloadedProc;
        ASection.Declarations[ASection.Declarations.IndexOf(Member)] :=
          OverloadedProc;
      end;
      exit;
    end;
  end;

  // Not overloaded, so just add the proc/function to the lists
  ASection.Declarations.Add(AProc);
  ASection.Functions.Add(AProc);
end;


// Returns the parent for an element which is to be created
function TPasParser.CheckIfOverloaded(AOwner: TPasClassType;
  const AName: String): TPasElement;
var
  i: Integer;
  Member: TPasElement;
begin
  for i := 0 to AOwner.Members.Count - 1 do
  begin
    Member := TPasElement(AOwner.Members[i]);
    if CompareText(Member.Name, AName) = 0 then
    begin
      if Member.ClassType = TPasOverloadedProc then
        Result := Member
      else
      begin
        Result := TPasOverloadedProc.Create(AName, AOwner);
        Result.Visibility := Member.Visibility;
        TPasOverloadedProc(Result).Overloads.Add(Member);
        AOwner.Members[i] := Result;
      end;
      exit;
    end;
  end;
  Result := AOwner;
end;


procedure TPasParser.ParseMain(var Module: TPasModule);
begin
  NextToken;
  case CurToken of
    tkUnit: ParseUnit(Module);
    else
      ParseExc(Format(SParserExpectTokenError, ['group']));
  end;
end;

// Starts after the "unit" token
procedure TPasParser.ParseUnit(var Module: TPasModule);
var
  CurBlock: TDeclType;
  Section: TPasSection;
  ConstEl: TPasConst;
  TypeEl: TPasType;
  ClassEl: TPasClassType;
  List: TList;
  i,j: Integer;
  VarEl: TPasVariable;
begin
  Module := nil;
  Module := TPasModule(CreateElement(TPasModule, ExpectIdentifier,
    Engine.Package));
  if Assigned(Engine.Package) then
  begin
    Module.PackageName := Engine.Package.Name;
    Engine.Package.Modules.Add(Module);
  end;
  ExpectToken(tkSemicolon);
  Section := TPasSection(CreateElement(TPasSection, '', Module));
  Module.InterfaceSection := Section;
  CurBlock := declNone;
  while True do
  begin
    NextToken;
    if CurToken = tkEnd then
      break;
    case CurToken of
      tkUses:
        ParseUsesList(Section);
//      tkAbi:
//        ParseAbiList(Section);
      tkConst:
        CurBlock := declConst;
      tkError:
        ParseError(Section);
      tkType:
        CurBlock := declType;
      tkVar:
        CurBlock := declVar;
      tkProcedure:
        begin
          AddProcOrFunction(Section, ParseProcedureOrFunctionDecl(Section, ptProcedure));
          CurBlock := declNone;
        end;
      tkFunction:
        begin
          AddProcOrFunction(Section, ParseProcedureOrFunctionDecl(Section, ptFunction));
          CurBlock := declNone;
        end;
      tkProperty:
        begin
          ExpectIdentifier;
          ParseProperty(CreateElement(TPasProperty, CurTokenString, Section));
        end;
      tkIdentifier:
        begin
          case CurBlock of
            declConst:
              begin
                ConstEl := ParseConstDecl(Section);
                Section.Declarations.Add(ConstEl);
                Section.Consts.Add(ConstEl);
              end;
            declType:
              begin
                TypeEl := ParseTypeDecl(Section);
                if Assigned(TypeEl) then        // !!!
                begin
                  Section.Declarations.Add(TypeEl);
                  if TypeEl.ClassType = TPasClassType then
                  begin
                    // Remove previous forward declarations, if necessary
                    for i := 0 to Section.Classes.Count - 1 do
                    begin
                      ClassEl := TPasClassType(Section.Classes[i]);
                      if CompareText(ClassEl.Name, TypeEl.Name) = 0 then
                      begin
                        Section.Classes.Delete(i);
                        for j := 0 to Section.Declarations.Count - 1 do
                          if CompareText(TypeEl.Name,
                            TPasElement(Section.Declarations[j]).Name) = 0 then
                          begin
                            Section.Declarations.Delete(j);
                            break;
                          end;
                        ClassEl.Release;
                        break;
                      end;
                    end;
                    // Add the new class to the class list
                    Section.Classes.Add(TypeEl)
                  end else
                    Section.Types.Add(TypeEl);
                end;
              end;
            declVar, declThreadVar:
              begin
                List := TList.Create;
                try
                  try
                    ParseVarDecl(Section, List);
                  except
                    for i := 0 to List.Count - 1 do
                      TPasVariable(List[i]).Release;
                    raise;
                  end;
                  for i := 0 to List.Count - 1 do
                  begin
                    VarEl := TPasVariable(List[i]);
                    Section.Declarations.Add(VarEl);
                    Section.Variables.Add(VarEl);
                  end;
                finally
                  List.Free;
                end;
              end;
          else
            ParseExc(SParserSyntaxError);
          end;
        end;
    else
      ParseExc(SParserInterfaceTokenError);
    end;
  end;
end;

// Starts after the "uses" token
procedure TPasParser.ParseUsesList(ASection: TPasSection);
var
  UnitName1: String;
  Element: TPasElement;
begin
    UnitName1 := ExpectIdentifier;

    NextToken;

        while CurToken = tkAlias do
        begin
          UnitName1:=UnitName1+'|'+ExpectIdentifier;
      NextToken;
        end;

        if CurToken = tkDefault then
        begin
          UnitName1:='!'+UnitName1;
      NextToken;
        end;

    if CurToken = tkSemicolon then
    begin
      Element := Engine.FindModule(UnitName1);
      if Assigned(Element) then
        Element.AddRef
      else
        Element := TPasType(CreateElement(TPasUnresolvedTypeRef, UnitName1,
          ASection));
      ASection.UsesList.Add(Element);
      exit;
    end else if CurToken <> tkAlias then
      ParseExc(SParserExpectedCommaSemicolon);
end;

// Starts after the "abi" token
(*procedure TPasParser.ParseAbiList(ASection: TPasSection);
var
  AbiName: String;
  Element: TPasElement;
begin
  while True do
  begin
    AbiName := ExpectIdentifier;

      Element := TPasType(CreateElement(TPasUnresolvedTypeRef, AbiName,
        ASection));
    ASection.AbiList.Add(Element);

    NextToken;

    if CurToken = tkSemicolon then
    begin
      break;
    end else
      ParseExc(SParserExpectedCommaSemicolon);
  end;
end;
*)
// Starts after the "error" token
procedure TPasParser.ParseError(ASection: TPasSection);
var
  ErrorCodeName: String;
  ErrorMsgName: String;
  Element: TPasElement;
  v: string;
begin
  ErrorCodeName:='';
  ErrorMsgName:='';

  while True do
  begin
    NextToken;
    if CurToken = tkIdentifier then
    begin
      UngetToken;
      ErrorCodeName := ExpectIdentifier;
    end else
      UngetToken;

    ExpectToken(tkNumber);
    v := CurTokenString;

    NextToken;
    if CurToken = tkIdentifier then
    begin
      UngetToken;
      ErrorMsgName := ExpectIdentifier;
    end else
      UngetToken;

    NextToken;

    if CurToken = tkSemicolon then
    begin
      break;
    end else if CurToken <> tkAlias then
      ParseExc(SParserExpectedCommaSemicolon);
  end;

  if ErrorCodeName<>'' then
  begin
    Element := TPasConst(CreateElement(TPasConst, ErrorCodeName, ASection));
    TPasConst(Element).Value:=v;
    ASection.Consts.Add(Element);
    ASection.Declarations.Add(Element);
  end;

  if ErrorMsgName<>'' then
  begin
    Element := TPasConst(CreateElement(TPasConst, ErrorMsgName, ASection));
    TPasConst(Element).Value:=v;
    ASection.Consts.Add(Element);
    ASection.Declarations.Add(Element);
  end;

end;

// Starts after the variable name
function TPasParser.ParseConstDecl(Parent: TPasElement): TPasConst;
begin
  Result := TPasConst(CreateElement(TPasConst, CurTokenString, Parent));

  try
    NextToken;
    if CurToken = tkColon then
      Result.VarType := ParseType(nil)
    else
      UngetToken;

    ExpectToken(tkEqual);
    Result.Value := ParseExpression;
    ExpectToken(tkSemicolon);
  except
    Result.Free;
    raise;
  end;
end;

// Starts after the type name
function TPasParser.ParseTypeDecl(Parent: TPasElement): TPasType;
var
  TypeName: String;

  procedure ParseRange;
  begin
    Result := TPasRangeType(CreateElement(TPasRangeType, TypeName, Parent));
    try
      TPasRangeType(Result).RangeStart := ParseExpression;
      ExpectToken(tkDotDot);
      TPasRangeType(Result).RangeEnd := ParseExpression;
      ExpectToken(tkSemicolon);
    except
      Result.Free;
      raise;
    end;
  end;

var
  EnumValue: TPasEnumValue;
  Prefix : String;

begin
  TypeName := CurTokenString;
  ExpectToken(tkEqual);
  NextToken;
  case CurToken of
    tkRecord:
      begin
        Result := TPasRecordType(CreateElement(TPasRecordType, TypeName,
          Parent));
        try
          ParseRecordDecl(TPasRecordType(Result), False);
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
        end;
      end;
    tkClass:
      begin
        Result := ParseClassDecl(Parent, TypeName, okClass);
      end;
//    tkInterface:
//      Result := ParseClassDecl(Parent, TypeName, okInterface);
    tkCaret:
      begin
        Result := TPasPointerType(CreateElement(TPasPointerType, TypeName,
          Parent));
        try
          TPasPointerType(Result).DestType := ParseType(nil);
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
        end;
      end;
    tkIdentifier:
      begin
        Prefix:=CurTokenString;
        NextToken;
        if CurToken = tkDot then
          begin
          ExpectIdentifier;
          NextToken;
          end
        else
          Prefix:='';
        if CurToken = tkSemicolon then
        begin
          UngetToken;
          UngetToken;
          Result := TPasAliasType(CreateElement(TPasAliasType, TypeName,
            Parent));
          try
            TPasAliasType(Result).DestType := ParseType(nil,Prefix);
            ExpectToken(tkSemicolon);
          except
            Result.Free;
            raise;
          end;
        end else if CurToken = tkSquaredBraceOpen then
        begin
          // !!!: Check for string type and store string length somewhere
          Result := TPasAliasType(CreateElement(TPasAliasType, TypeName,
            Parent));
          try
            TPasAliasType(Result).DestType :=
              TPasUnresolvedTypeRef.Create(CurTokenString, Parent);
            ParseExpression;
            ExpectToken(tkSquaredBraceClose);
            ExpectToken(tkSemicolon);
          except
            Result.Free;
            raise;
          end;
        end else
        begin
          UngetToken;
          UngetToken;
          ParseRange;
        end;
      end;
    tkFile:
      begin
        Result := TPasFileType(CreateElement(TPasFileType, TypeName, Parent));
        Try
          ParseFileType(TPasFileType(Result));
          ExpectToken(tkSemicolon);
        Except
          Result.free;
          Raise;
        end;
      end;
    tkArray:
      begin
        Result := TPasArrayType(CreateElement(TPasArrayType, TypeName, Parent));
        try
          ParseArrayType(TPasArrayType(Result));
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
        end;
      end;
    tkSet:
      begin
        Result := TPasSetType(CreateElement(TPasSetType, TypeName, Parent));
        try
          ExpectToken(tkOf);
          TPasSetType(Result).EnumType := ParseType(Result);
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
        end;
      end;
    tkBraceOpen:
      begin
        Result := TPasEnumType(CreateElement(TPasEnumType, TypeName, Parent));
        try
          while True do
          begin
            NextToken;
            EnumValue := TPasEnumValue(CreateElement(TPasEnumValue,
              CurTokenString, Result));
            TPasEnumType(Result).Values.Add(EnumValue);
            NextToken;
            if CurToken = tkBraceClose then
              break
            else if CurToken in [tkEqual,tkAssign] then
              begin
              EnumValue.AssignedValue:=ParseExpression;
              NextToken;
              if CurToken = tkBraceClose then
                Break
              else if not (CurToken=tkComma) then
                ParseExc(SParserExpectedCommaRBracket);
              end
            else if not (CurToken=tkComma) then
              ParseExc(SParserExpectedCommaRBracket)
          end;
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
        end;
      end;
    tkProcedure:
      begin
        Result := TPasProcedureType(CreateElement(TPasProcedureType, TypeName,
          Parent));
        try
          ParseProcedureOrFunctionHeader(Result,
            TPasProcedureType(Result), ptProcedure, True);
        except
          Result.Free;
          raise;
        end;
      end;
    tkFunction:
      begin
        Result := Engine.CreateFunctionType(TypeName, 'Result', Parent, False,
          Scanner.CurFilename, Scanner.CurRow);
        try
          ParseProcedureOrFunctionHeader(Result,
            TPasFunctionType(Result), ptFunction, True);
        except
          Result.Free;
          raise;
        end;
      end;
    tkType:
      begin
        Result := TPasTypeAliasType(CreateElement(TPasTypeAliasType, TypeName,
          Parent));
        try
          TPasTypeAliasType(Result).DestType := ParseType(nil);
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
        end;
      end;
    else
    begin
      UngetToken;
      ParseRange;
    end;
  end;
end;

// Starts after the variable name

procedure TPasParser.ParseInlineVarDecl(Parent: TPasElement; VarList: TList);
begin
  ParseInlineVarDecl(Parent, VarList, visDefault, False);
end;

procedure TPasParser.ParseInlineVarDecl(Parent: TPasElement; VarList: TList;
  AVisibility: TPasMemberVisibility; ClosingBrace: Boolean);
var
  VarNames: TStringList;
  i: Integer;
  VarType: TPasType;
  VarEl: TPasVariable;
begin
  VarNames := TStringList.Create;
  try
    while True do
    begin
      VarNames.Add(CurTokenString);
      NextToken;
      if CurToken = tkColon then
        break
      else if CurToken <> tkComma then
        ParseExc(SParserExpectedCommaColon);

      ExpectIdentifier;
    end;

    VarType := ParseComplexType;

    for i := 0 to VarNames.Count - 1 do
    begin
      VarEl := TPasVariable(CreateElement(TPasVariable, VarNames[i], Parent,
        AVisibility));
      VarEl.VarType := VarType;
      if i > 0 then
        VarType.AddRef;
      VarList.Add(VarEl);
    end;
    NextToken;
    // Records may be terminated with end, no semicolon
    if (CurToken <> tkEnd) and (CurToken <> tkSemicolon) and not
      (ClosingBrace and (CurToken = tkBraceClose)) then
      ParseExc(SParserExpectedSemiColonEnd);
  finally
    VarNames.Free;
  end;
end;

// Starts after the variable name
procedure TPasParser.ParseVarDecl(Parent: TPasElement; List: TList);
var
  i: Integer;
  VarType: TPasType;
  Value, S: String;
  M: string;
begin
  while True do
  begin
    List.Add(CreateElement(TPasVariable, CurTokenString, Parent));
    NextToken;
    if CurToken = tkColon then
      break
    else if CurToken <> tkComma then
      ParseExc(SParserExpectedCommaColon);
    ExpectIdentifier;
  end;
  VarType := ParseComplexType;
  for i := 0 to List.Count - 1 do
  begin
    TPasVariable(List[i]).VarType := VarType;
    if i > 0 then
      VarType.AddRef;
  end;
  NextToken;
  If CurToken=tkEqual then
    begin
    Value := ParseExpression;
    for i := 0 to List.Count - 1 do
      TPasVariable(List[i]).Value := Value;
    end
  else
    UngetToken;

  NextToken;
  if CurToken = tkAbsolute then
  begin
    // !!!: Store this information
    ExpectIdentifier;
  end else
    UngetToken;

  ExpectToken(tkSemicolon);
  M := '';
  while True do
  begin
    NextToken;
    if CurToken = tkIdentifier then
      begin
      s := UpperCase(CurTokenText);
      if s = 'CVAR' then
        begin
        M := M + '; cvar';
        ExpectToken(tkSemicolon);
        end
      else if (s = 'EXTERNAL') or (s = 'PUBLIC') or (s = 'EXPORT') then
        begin
        M := M + ';' + CurTokenText;
        if s = 'EXTERNAL' then
          begin
          NextToken;
          if ((CurToken = tkString) or (CurToken = tkIdentifier)) and (UpperCase(CurTokenText)<> 'NAME') then
            begin
            // !!!: Is this really correct for tkString?
            M := M + ' ' + CurTokenText;
            NextToken;
            end;
          end
        else
          NextToken;
        if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'NAME') then
          begin
          M := M + ' name ';
          NextToken;
          if (CurToken = tkString) or (CurToken = tkIdentifier) then
            // !!!: Is this really correct for tkString?
            M := M + CurTokenText
          else
            ParseExc(SParserSyntaxError);
          ExpectToken(tkSemicolon);
          end
        else if CurToken <> tkSemicolon then
          ParseExc(SParserSyntaxError);
      end else
      begin
        UngetToken;
        break;
      end
    end else
    begin
      UngetToken;
      break;
    end;
  end; // while

  if M <> '' then
    for i := 0 to List.Count - 1 do
      TPasVariable(List[i]).Modifiers := M;
end;

// Starts after the opening bracket token
procedure TPasParser.ParseArgList(Parent: TPasElement; Args: TList; EndToken: TToken);
var
  ArgNames: TStringList;
  IsUntyped: Boolean;
  Name, Value: String;
  i: Integer;
  Arg: TPasArgument;
  Access: TArgumentAccess;
  ArgType: TPasType;
begin
  while True do
  begin
    ArgNames := TStringList.Create;
    Access := argDefault;
    IsUntyped := False;
    ArgType := nil;
    while True do
    begin
      NextToken;
      if CurToken = tkInArg then
      begin
        Access := ArgIn;
        Name := ExpectIdentifier;
      end else if CurToken = tkInOutArg then
      begin
        Access := ArgInOut;
        Name := ExpectIdentifier;
      end else if CurToken = tkOutArg then
      begin
        Access := ArgOut;
        Name := ExpectIdentifier;
      end else if CurToken = tkIdentifier then
        Name := CurTokenString
      else
        ParseExc(SParserExpectedConstVarID);
      ArgNames.Add(Name);
      NextToken;
      if CurToken = tkColon then
        break
      else if ((CurToken = tkSemicolon) or (CurToken = tkBraceClose)) and
        (Access <> argDefault) then
      begin
        // found an untyped const or var argument
        UngetToken;
        IsUntyped := True;
        break
      end
      else if CurToken <> tkComma then
        ParseExc(SParserExpectedCommaColon);
    end;
    SetLength(Value, 0);
    if not IsUntyped then
    begin
      ArgType := ParseType(nil);
      NextToken;
      if CurToken = tkEqual then
      begin
        Value := ParseExpression;
      end else
        UngetToken;
    end;

    for i := 0 to ArgNames.Count - 1 do
    begin
      Arg := TPasArgument(CreateElement(TPasArgument, ArgNames[i], Parent));
      Arg.Access := Access;
      Arg.ArgType := ArgType;
      if (i > 0) and Assigned(ArgType) then
        ArgType.AddRef;
      Arg.Value := Value;
      Args.Add(Arg);
      If Access = argDefault then ParseExc(SParserNoDirection);
    end;

    ArgNames.Free;
    NextToken;
    if CurToken = EndToken then
      break;
  end;
end;

// Next token is expected to be a "(", ";" or for a function ":". The caller
// will get the token after the final ";" as next token.
procedure TPasParser.ParseProcedureOrFunctionHeader(Parent: TPasElement;
  Element: TPasProcedureType; ProcType: TProcType; OfObjectPossible: Boolean);

//Var
//  Tok : String;

begin
  NextToken;
  case ProcType of
    ptFunction:
      begin
        if CurToken = tkBraceOpen then
        begin
          NextToken;
          if (CurToken = tkBraceClose) then
          else
            begin
              UngetToken;
              ParseArgList(Parent, Element.Args, tkBraceClose);
            end;
          ExpectToken(tkColon);
        end else if CurToken <> tkColon then
          ParseExc(SParserExpectedLBracketColon);
        if Assigned(Element) then        // !!!
          TPasFunctionType(Element).ResultEl.ResultType := ParseType(Parent)
        else
          ParseType(nil);
      end;
    ptProcedure:
      begin
        if CurToken = tkBraceOpen then
        begin
          NextToken;
          if (CurToken = tkBraceClose) then
          else
            begin
              UngetToken;
              ParseArgList(Element, Element.Args, tkBraceClose);
            end
        end else if (CurToken = tkSemicolon) or (OfObjectPossible and (CurToken = tkOf)) then
          UngetToken
        else
          ParseExc(SParserExpectedLBracketSemicolon);
      end;
    ptOperator:
      begin
        ParseArgList(Element, Element.Args, tkBraceClose);
        NextToken;
        if (CurToken=tkIdentifier) then begin
          TPasFunctionType(Element).ResultEl.Name := CurTokenName;
          ExpectToken(tkColon);
        end
        else if (CurToken=tkColon) then
          TPasFunctionType(Element).ResultEl.Name := 'Result'
        else
          ParseExc(SParserExpectedColonID);
        if Assigned(Element) then        // !!!
          TPasFunctionType(Element).ResultEl.ResultType := ParseType(Parent)
        else
          ParseType(nil);
      end;
  end;

  NextToken;
  if CurToken = tkEqual then
  begin
    // for example: const p: procedure = nil;
    UngetToken;
    exit;
  end else
    UngetToken;

  ExpectToken(tkSemicolon);

  while True do
    begin
    NextToken;
    if (CurToken = tkSquaredBraceOpen) then
      begin
      repeat
        NextToken
      until CurToken = tkSquaredBraceClose;
      ExpectToken(tkSemicolon);
      end
    else
      begin
      UngetToken;
      break;
      end;
    end;
end;


procedure TPasParser.ParseProperty(Element:TPasElement);

  function GetAccessorName: String;
  begin
    ExpectIdentifier;
    Result := CurTokenString;

    while True do begin
      NextToken;
      if CurToken = tkDot then begin
        ExpectIdentifier;
        Result := Result + '.' + CurTokenString;
      end else
        break;
    end;

    if CurToken = tkSquaredBraceOpen then begin
      Result := Result + '[';
      NextToken;
      if CurToken in [tkIdentifier, tkNumber] then begin
        Result := Result + CurTokenString;
      end;
      ExpectToken(tkSquaredBraceClose);
      Result := Result + ']';
    end else
      UngetToken;

//    writeln(Result);

  end;

begin

  NextToken;
// if array prop then parse [ arg1:type1;... ]
  if CurToken = tkSquaredBraceOpen then begin
  // !!!: Parse array properties correctly
    ParseArgList(Element, TPasProperty(Element).Args, tkSquaredBraceClose);
    NextToken;
  end;

  if CurToken = tkColon then begin
// if ":prop_data_type" if supplied then read it
  // read property type
    TPasProperty(Element).VarType := ParseType(Element);
    NextToken;
  end;

  if CurToken <> tkSemicolon then begin
//  if indexed prop then read the index value
    if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'INDEX') then
//    read 'index' access modifier
      TPasProperty(Element).IndexValue := ParseExpression
    else
//    not indexed prop will be recheck for another token
      UngetToken;

    NextToken;
  end;

// if the accessors list is not finished
  if CurToken <> tkSemicolon then begin
    // read 'read' access modifier
    if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'READ') then
      TPasProperty(Element).ReadAccessorName := GetAccessorName
    else
//    not read accessor will be recheck for another token
      UngetToken;

    NextToken;
  end;

// if the accessors list is not finished
  if CurToken <> tkSemicolon then begin
    // read 'write' access modifier
    if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'WRITE') then
      TPasProperty(Element).WriteAccessorName := GetAccessorName
    else
//    not write accessor will be recheck for another token
      UngetToken;

    NextToken;
  end;

// if the specifiers list is not finished
  if CurToken <> tkSemicolon then begin
    // read 'stored' access modifier
    if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'STORED') then begin
      NextToken;
      if CurToken = tkTrue then
        TPasProperty(Element).StoredAccessorName := 'True'
      else if CurToken = tkFalse then
        TPasProperty(Element).StoredAccessorName := 'False'
      else if CurToken = tkIdentifier then
        TPasProperty(Element).StoredAccessorName := CurTokenString
      else
        ParseExc(SParserSyntaxError);
    end else
//    not stored accessor will be recheck for another token
      UngetToken;

    NextToken;
  end;

// if the specifiers list is not finished
  if CurToken <> tkSemicolon then begin
    if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'DEFAULT') then
//    read 'default' value modifier -> ParseExpression(DEFAULT <value>)
      TPasProperty(Element).DefaultValue := ParseExpression
    else
//    not "default <value>" prop will be recheck for another token
      UngetToken;

    NextToken;
  end;

// if the specifiers list is not finished
  if CurToken <> tkSemicolon then begin
    if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'NODEFAULT') then begin
//    read 'nodefault' modifier
      TPasProperty(Element).IsNodefault:=true;
    end;
//  stop recheck for specifiers - start from next token
    NextToken;
  end;

// after NODEFAULT may be a ";"
  if CurToken = tkSemicolon then begin
    // read semicolon
    NextToken;
  end;

  if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'DEFAULT') then begin
//  what is after DEFAULT token at the end
    NextToken;
    if CurToken = tkSemicolon then begin
//    ";" then DEFAULT=prop
      TPasProperty(Element).IsDefault := True;
      UngetToken;
    end else begin
//    "!;" then a step back to get phrase "DEFAULT <value>"
      UngetToken;
//    DefaultValue  -> ParseExpression(DEFAULT <value>)  and stay on the <value>
      TPasProperty(Element).DefaultValue := ParseExpression;
    end;

//!!  there may be DEPRECATED token
    NextToken;

  end;

// after DEFAULT may be a ";"
  if CurToken = tkSemicolon then begin
    // read semicolon
    NextToken;
  end;

  if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'DEPRECATED') then begin
//  nothing to do on DEPRECATED - just to accept
//    NextToken;
  end else
    UngetToken;;

//!!   else
//  not DEFAULT prop accessor will be recheck for another token
//!!    UngetToken;

{
  if CurToken = tkSemicolon then begin
    // read semicolon
    NextToken;
  end;
  if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'DEPRECATED') then begin
//  nothing to do - just to process
    NextToken;
  end;
  if CurToken = tkSemicolon then begin
    // read semicolon
    NextToken;
  end;
}



end;


// Starts after the "procedure" or "function" token
function TPasParser.ParseProcedureOrFunctionDecl(Parent: TPasElement;
  ProcType: TProcType): TPasProcedure;
var
  Name: String;
  i: Integer;
begin
  case ProcType of
    ptFunction:
      begin
        Name := ExpectIdentifier;
        Result := TPasFunction(CreateElement(TPasFunction, Name, Parent));
        Result.ProcType := Engine.CreateFunctionType('', 'Result', Result, True,
          Scanner.CurFilename, Scanner.CurRow);
      end;
    ptProcedure:
      begin
        Name := ExpectIdentifier;
        Result := TPasProcedure(CreateElement(TPasProcedure, Name, Parent));
        Result.ProcType := TPasProcedureType(CreateElement(TPasProcedureType, '',
          Result));
      end;
    ptOperator:
      begin
        NextToken;
        Name := 'operator ' + TokenInfos[CurToken];
        Result := TPasOperator(CreateElement(TPasOperator, Name, Parent));
        Result.ProcType := Engine.CreateFunctionType('', '__INVALID__', Result,
          True, Scanner.CurFilename, Scanner.CurRow);
      end;
  end;

  ParseProcedureOrFunctionHeader(Result, Result.ProcType, ProcType, False);

  if ProcType = ptOperator then
  begin
    Result.Name := Result.Name + '(';
    for i := 0 to Result.ProcType.Args.Count - 1 do
    begin
      if i > 0 then
        Result.Name := Result.Name + ', ';
      Result.Name := Result.Name +
        TPasArgument(Result.ProcType.Args[i]).ArgType.Name;
    end;
    Result.Name := Result.Name + '): ' +
      TPasFunctionType(Result.ProcType).ResultEl.ResultType.Name;
  end;
end;


// Starts after the "record" token
procedure TPasParser.ParseRecordDecl(Parent: TPasRecordType; IsNested: Boolean);
var
  VariantName: String;
  Variant: TPasVariant;
begin
  while True do
  begin
    if IsNested then
    begin
      if CurToken = tkBraceClose then
        break;
      NextToken;
      if CurToken = tkBraceClose then
        break;
    end else
    begin
      if CurToken = tkEnd then
        break;
      NextToken;
      if CurToken = tkEnd then
        break;
    end;
    if CurToken = tkCase then
    begin
      ExpectToken(tkIdentifier);
      VariantName := CurTokenString;
      NextToken;
      if CurToken = tkColon then
        Parent.VariantName := VariantName
      else
      begin
        UngetToken;
        UngetToken;
      end;
      Parent.VariantType := ParseType(Parent);
      Parent.Variants := TList.Create;

      ExpectToken(tkOf);

      while True do
      begin
        Variant := TPasVariant(CreateElement(TPasVariant, '', Parent));
        Parent.Variants.Add(Variant);
        Variant.Values := TStringList.Create;
        while True do
        begin
          Variant.Values.Add(ParseExpression);
          NextToken;
          if CurToken = tkColon then
            break
          else if CurToken <> tkComma then
            ParseExc(SParserExpectedCommaColon);
        end;
        ExpectToken(tkBraceOpen);
        Variant.Members := TPasRecordType(CreateElement(TPasRecordType, '',
          Variant));
        try
          ParseRecordDecl(Variant.Members, True);
        except
          Variant.Members.Free;
          raise;
        end;
        NextToken;
        if CurToken = tkSemicolon then
          NextToken;
        if (CurToken = tkEnd) or (CurToken = tkBraceClose) then
          break
        else
          UngetToken;
      end
    end else
      ParseInlineVarDecl(Parent, Parent.Members, visDefault, IsNested);
  end;
end;

// Starts after the "class" token
function TPasParser.ParseClassDecl(Parent: TPasElement;
  const AClassName: String; AObjKind: TPasObjKind): TPasType;
var
  CurVisibility: TPasMemberVisibility;

  procedure ProcessMethod(const MethodTypeName: String; HasReturnValue: Boolean);
  var
    Owner: TPasElement;
    Proc: TPasProcedure;
    s: String;
    pt: TProcType;
  begin
    ExpectIdentifier;
    Owner := CheckIfOverloaded(TPasClassType(Result), CurTokenString);
    if HasReturnValue then
    begin
      Proc := TPasFunction(CreateElement(TPasFunction, CurTokenString, Owner,
        CurVisibility));
      Proc.ProcType := Engine.CreateFunctionType('', 'Result', Proc, True,
        Scanner.CurFilename, Scanner.CurRow);
    end else
    begin
      // !!!: The following is more than ugly
      if MethodTypeName = 'constructor' then
        Proc := TPasConstructor(CreateElement(TPasConstructor, CurTokenString,
          Owner, CurVisibility))
      else if MethodTypeName = 'destructor' then
        Proc := TPasDestructor(CreateElement(TPasDestructor, CurTokenString,
          Owner, CurVisibility))
      else
        Proc := TPasProcedure(CreateElement(TPasProcedure, CurTokenString,
          Owner, CurVisibility));
      Proc.ProcType := TPasProcedureType(CreateElement(TPasProcedureType, '',
        Proc, CurVisibility));
    end;
    if Owner.ClassType = TPasOverloadedProc then
      TPasOverloadedProc(Owner).Overloads.Add(Proc)
    else
      TPasClassType(Result).Members.Add(Proc);

    if HasReturnValue then
      pt := ptFunction
    else
      pt := ptProcedure;
    ParseProcedureOrFunctionHeader(Proc, Proc.ProcType, pt, False);

    while True do
    begin
      NextToken;
      if CurToken = tkIdentifier then
      begin
        s := UpperCase(CurTokenString);
        if s = 'VIRTUAL' then
          Proc.IsVirtual := True
        else if s = 'DYNAMIC' then
          Proc.IsDynamic := True
        else if s = 'ABSTRACT' then
          Proc.IsAbstract := True
        else if s = 'OVERRIDE' then
          Proc.IsOverride := True
        else if s = 'REINTRODUCE' then
          Proc.IsReintroduced := True
        else if s = 'OVERLOAD' then
          Proc.IsOverload := True
        else if s = 'STATIC' then
          Proc.IsStatic := True
        else if s = 'MESSAGE' then begin
          Proc.IsMessage := True;
          repeat
            NextToken;
          until CurToken = tkSemicolon;
          UngetToken;
        end
        else if s = 'CDECL' then
{      El['calling-conv'] := 'cdecl';}
        else if s = 'PASCAL' then
{      El['calling-conv'] := 'cdecl';}
        else if s = 'STDCALL' then
{      El['calling-conv'] := 'stdcall';}
        else if s = 'OLDFPCCALL' then
{      El['calling-conv'] := 'oldfpccall';}
        else if s = 'EXTDECL' then
{      El['calling-conv'] := 'extdecl';}
        else if s = 'DEPRECATED' then
{      El['calling-conv'] := 'deprecated';}
        else
        begin
          UngetToken;
          break;
        end;
        ExpectToken(tkSemicolon);
      end else
      begin
        UngetToken;
        break;
      end;
    end;
  end;

var
  s, SourceFilename: String;
  i, SourceLinenumber: Integer;
  VarList: TList;
  Element: TPasElement;
begin
  // Save current parsing position to get it correct in all cases
  SourceFilename := Scanner.CurFilename;
  SourceLinenumber := Scanner.CurRow;

  NextToken;

  if (AObjKind = okClass) and (CurToken = tkOf) then
  begin
    Result := TPasClassOfType(Engine.CreateElement(TPasClassOfType, AClassName,
      Parent, SourceFilename, SourceLinenumber));
    ExpectIdentifier;
    UngetToken;                // Only names are allowed as following type
    TPasClassOfType(Result).DestType := ParseType(Result);
    ExpectToken(tkSemicolon);
    exit;
  end;


  Result := TPasClassType(Engine.CreateElement(TPasClassType, AClassName,
    Parent, SourceFilename, SourceLinenumber));

  try
    TPasClassType(Result).ObjKind := AObjKind;

    // Parse ancestor list
    if CurToken = tkBraceOpen then
    begin
      TPasClassType(Result).AncestorType := ParseType(nil);
      while True do
      begin
        NextToken;
        if CurToken = tkBraceClose then
          break;
        UngetToken;
        ExpectToken(tkComma);
        ExpectIdentifier;
        // !!!: Store interface name
      end;
      NextToken;
    end;

    if CurToken <> tkSemicolon then
    begin
      CurVisibility := visDefault;
      while CurToken <> tkEnd do
      begin
        case CurToken of
          tkIdentifier:
            begin
              s := LowerCase(CurTokenString);
              if s = 'private' then
                CurVisibility := visPrivate
              else if s = 'protected' then
                CurVisibility := visProtected
              else if s = 'public' then
                CurVisibility := visPublic
              else if s = 'published' then
                CurVisibility := visPublished
              else if s = 'automated' then
                CurVisibility := visAutomated
              else
              begin
                VarList := TList.Create;
                try
                  ParseInlineVarDecl(Result, VarList, CurVisibility, False);
                  for i := 0 to VarList.Count - 1 do
                  begin
                    Element := TPasElement(VarList[i]);
                    Element.Visibility := CurVisibility;
                    TPasClassType(Result).Members.Add(Element);
                  end;
                finally
                  VarList.Free;
                end;
              end;
            end;
          tkProcedure:
            ProcessMethod('procedure', False);
          tkFunction:
            ProcessMethod('function', True);
          tkConstructor:
            ProcessMethod('constructor', False);
          tkDestructor:
            ProcessMethod('destructor', False);
          tkProperty:
            begin
              ExpectIdentifier;
              Element := CreateElement(TPasProperty, CurTokenString, Result, CurVisibility);
              TPasClassType(Result).Members.Add(Element);
              ParseProperty(Element);
            end;
        end; // end case
        NextToken;
      end;
      // Eat semicolon after class...end
      ExpectToken(tkSemicolon);
    end;
  except
    Result.Free;
    raise;
  end;
end;

function TPasParser.CreateElement(AClass: TPTreeElement; const AName: String;
  AParent: TPasElement): TPasElement;
begin
  Result := Engine.CreateElement(AClass, AName, AParent,
    Scanner.CurFilename, Scanner.CurRow);
end;

function TPasParser.CreateElement(AClass: TPTreeElement; const AName: String;
  AParent: TPasElement; AVisibility: TPasMemberVisibility): TPasElement;
begin
  Result := Engine.CreateElement(AClass, AName, AParent, AVisibility,
    Scanner.CurFilename, Scanner.CurRow);
end;


function ParseSource(AEngine: TPasTreeContainer;
  const FPCCommandLine, OSTarget, CPUTarget: String): TPasModule;
var
  FileResolver: TFileResolver;
  Parser: TPasParser;
  Start, CurPos: PChar;
  Filename: String;
  Scanner: TPascalScanner;

  procedure ProcessCmdLinePart;
  var
    l: Integer;
    s: String;
  begin
    l := CurPos - Start;
    SetLength(s, l);
    if l > 0 then
      Move(Start^, s[1], l)
    else
      exit;
    if s[1] = '-' then
    begin
      case s[2] of
        'd':
          Scanner.Defines.Append(UpperCase(Copy(s, 3, Length(s))));
        'F':
          if s[3] = 'i' then
            FileResolver.AddIncludePath(Copy(s, 4, Length(s)));
        'S':
          if s[3]='d' then
            begin
              include(Scanner.Options,po_delphi);
              include(Parser.Options,po_delphi);
            end;
      end;
    end else
      if Filename <> '' then
        raise Exception.Create(SErrMultipleSourceFiles)
      else
        Filename := s;
  end;

var
  s: String;
begin
  Result := nil;
  FileResolver := nil;
  Scanner := nil;
  Parser := nil;
  try
    FileResolver := TFileResolver.Create;
    Scanner := TPascalScanner.Create(FileResolver);
    Scanner.Defines.Append('FPK');
    Scanner.Defines.Append('FPC');
    s := UpperCase(OSTarget);
    Scanner.Defines.Append(s);
    if s = 'LINUX' then
      Scanner.Defines.Append('UNIX')
    else if s = 'FREEBSD' then
    begin
      Scanner.Defines.Append('BSD');
      Scanner.Defines.Append('UNIX');
    end else if s = 'NETBSD' then
    begin
      Scanner.Defines.Append('BSD');
      Scanner.Defines.Append('UNIX');
    end else if s = 'SUNOS' then
    begin
      Scanner.Defines.Append('SOLARIS');
      Scanner.Defines.Append('UNIX');
    end else if s = 'GO32V2' then
      Scanner.Defines.Append('DPMI')
    else if s = 'BEOS' then
      Scanner.Defines.Append('UNIX')
    else if s = 'QNX' then
      Scanner.Defines.Append('UNIX');

    Parser := TPasParser.Create(Scanner, FileResolver, AEngine);
    Filename := '';

    if FPCCommandLine<>'' then
      begin
        Start := @FPCCommandLine[1];
        CurPos := Start;
        while CurPos[0] <> #0 do
        begin
          if CurPos[0] = ' ' then
          begin
            ProcessCmdLinePart;
            Start := CurPos + 1;
          end;
          Inc(CurPos);
        end;
        ProcessCmdLinePart;
      end;

    if Filename = '' then
      raise Exception.Create(SErrNoSourceGiven);

    Scanner.OpenFile(Filename);
    Parser.ParseMain(Result);
  finally
    Parser.Free;
    Scanner.Free;
    FileResolver.Free;
  end;
end;

end.
