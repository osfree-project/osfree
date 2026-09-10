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
  SParserInterfaceTokenError = 'Invalid token in interface section of group';
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
    function ParseGroup(Parent: TPasElement): TPasGroup;

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
    function ParseStructure(Parent: TPasElement): TPasRecordType;
    function ParseVariableDecl(Parent: TPasElement): TPasVariable;
    procedure ParseRecordDecl(Parent: TPasRecordType; IsNested: Boolean);
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
    FCurToken := FTokenBuffer[FTokenBufferIndex];
    FCurTokenString := FTokenStringBuffer[FTokenBufferIndex];
    Inc(FTokenBufferIndex);
  end else
  begin
    if FTokenBufferSize = 2 then
    begin
      FTokenBuffer[0] := FTokenBuffer[1];
      FTokenStringBuffer[0] := FTokenStringBuffer[1];
      Dec(FTokenBufferSize);
      Dec(FTokenBufferIndex);
    end;
    try
      repeat
        FCurToken := Scanner.FetchToken;
      until not (FCurToken = tkWhitespace);   // пропускаем только пробелы, комментарии оставляем
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
  Result := nil;
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
        Ref := Engine.FindElement(Name);
        if Assigned(Ref) then
        begin
          Result := Ref as TPasType;
          Result.AddRef;
        end else
          Result := TPasUnresolvedTypeRef(CreateElement(TPasUnresolvedTypeRef, Name, nil));

        if Name = 'String' then
        begin
          NextToken;
          if CurToken = tkSquaredBraceOpen then
          begin
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
        UngetToken;
      end;
    tkFunction:
      begin
        Result := Engine.CreateFunctionType('', 'Result', nil, False,
          Scanner.CurFilename, Scanner.CurRow);
        ParseProcedureOrFunctionHeader(Result,
          TPasFunctionType(Result), ptFunction, True);
        UngetToken;
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
var
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
      Result := Result + ''''+StringReplace(CurTokenText,'''','''''',[rfReplaceAll])+''''
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

  ASection.Declarations.Add(AProc);
  ASection.Functions.Add(AProc);
end;

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
  // ОТЛАДКА: выводим токен до group
  WriteLn('DEBUG ParseMain: токен = ', CurTokenName, ' текст = "', CurTokenString, '"');
  // Пропускаем комментарии, если они есть (но сейчас мы их не обрабатываем)
  while CurToken = tkComment do
  begin
    NextToken;
    WriteLn('DEBUG ParseMain: пропущен комментарий, следующий токен = ', CurTokenName);
  end;

  case CurToken of
    tkGroup: ParseUnit(Module);
    else
      ParseExc(Format(SParserExpectTokenError, ['group']));
  end;
end;

procedure TPasParser.ParseUnit(var Module: TPasModule);
var
  CurBlock: TDeclType;
  Section: TPasSection;
  ConstEl: TPasConst;
  TypeEl: TPasType;
  List: TList;
  i: Integer;
  VarEl: TPasVariable;
  GroupEl: TPasGroup;
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
  ExpectToken(tkBegin);

  Section := TPasSection(CreateElement(TPasSection, '', Module));
  Module.InterfaceSection := Section;
  CurBlock := declNone;

  while True do
  begin
    NextToken;
    if CurToken = tkComment then
    begin
      Section.Declarations.Add(TPasComment(CreateElement(TPasComment, CurTokenString, Section)));
      Continue;
    end;
    if CurToken = tkEnd then
      break;
    case CurToken of
      tkConst:
        CurBlock := declConst;
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
      tkStructure:
        begin
          TypeEl := TPasType(ParseStructure(Section));
          Section.Declarations.Add(TypeEl);
          Section.Types.Add(TypeEl);
          CurBlock := declNone;
        end;
      tkVariable:
        begin
          VarEl := ParseVariableDecl(Section);
          Section.Declarations.Add(VarEl);
          Section.Variables.Add(VarEl);
        end;
      tkGroup:
        begin
          GroupEl := ParseGroup(Section);
          Section.Declarations.Add(GroupEl);
          CurBlock := declNone;
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
                if Assigned(TypeEl) then
                begin
                  Section.Declarations.Add(TypeEl);
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

  ExpectToken(tkSemicolon);
end;

function TPasParser.ParseGroup(Parent: TPasElement): TPasGroup;
var
  GroupEl: TPasGroup;
  Section: TPasSection;
  ConstEl: TPasConst;
  TypeEl: TPasType;
  VarEl: TPasVariable;
  CurBlock: TDeclType;
  List: TList;
  i: Integer;
begin
  GroupEl := TPasGroup(CreateElement(TPasGroup, ExpectIdentifier, Parent));
  ExpectToken(tkSemicolon);
  ExpectToken(tkBegin);
  Section := TPasSection(CreateElement(TPasSection, '', GroupEl));
  GroupEl.InterfaceSection := Section;
  CurBlock := declNone;

  while True do
  begin
    NextToken;
    if CurToken = tkComment then
    begin
      Section.Declarations.Add(TPasComment(CreateElement(TPasComment, CurTokenString, Section)));
      Continue;
    end;
    if CurToken = tkEnd then
      break;
    case CurToken of
      tkConst:
        CurBlock := declConst;
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
      tkStructure:
        begin
          TypeEl := TPasType(ParseStructure(Section));
          Section.Declarations.Add(TypeEl);
          Section.Types.Add(TypeEl);
          CurBlock := declNone;
        end;
      tkVariable:
        begin
          VarEl := ParseVariableDecl(Section);
          Section.Declarations.Add(VarEl);
          Section.Variables.Add(VarEl);
        end;
      tkGroup:
        begin
          Section.Declarations.Add(ParseGroup(Section));
          CurBlock := declNone;
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
                if Assigned(TypeEl) then
                begin
                  Section.Declarations.Add(TypeEl);
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

  ExpectToken(tkSemicolon);
  Result := GroupEl;
end;

procedure TPasParser.ParseError(ASection: TPasSection);
begin
  // Пустая реализация, т.к. не используется
end;

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
    tkClass:
      Result := ParseClassDecl(Parent, TypeName, okClass);
    tkCaret:
      begin
        Result := TPasPointerType(CreateElement(TPasPointerType, TypeName, Parent));
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
        end else
          Prefix:='';
        if CurToken = tkSemicolon then
        begin
          UngetToken;
          UngetToken;
          Result := TPasAliasType(CreateElement(TPasAliasType, TypeName, Parent));
          try
            TPasAliasType(Result).DestType := ParseType(nil,Prefix);
            ExpectToken(tkSemicolon);
          except
            Result.Free;
            raise;
          end;
        end else if CurToken = tkSquaredBraceOpen then
        begin
          Result := TPasAliasType(CreateElement(TPasAliasType, TypeName, Parent));
          try
            TPasAliasType(Result).DestType := TPasUnresolvedTypeRef.Create(CurTokenString, Parent);
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
        try
          ParseFileType(TPasFileType(Result));
          ExpectToken(tkSemicolon);
        except
          Result.Free;
          raise;
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
            EnumValue := TPasEnumValue(CreateElement(TPasEnumValue, CurTokenString, Result));
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
        Result := TPasProcedureType(CreateElement(TPasProcedureType, TypeName, Parent));
        try
          ParseProcedureOrFunctionHeader(Result, TPasProcedureType(Result), ptProcedure, True);
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
          ParseProcedureOrFunctionHeader(Result, TPasFunctionType(Result), ptFunction, True);
        except
          Result.Free;
          raise;
        end;
      end;
    tkType:
      begin
        Result := TPasTypeAliasType(CreateElement(TPasTypeAliasType, TypeName, Parent));
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
      VarEl := TPasVariable(CreateElement(TPasVariable, VarNames[i], Parent, AVisibility));
      VarEl.VarType := VarType;
      if i > 0 then
        VarType.AddRef;
      VarList.Add(VarEl);
    end;

    // После разбора типа ожидается ';'
    ExpectToken(tkSemicolon);
    // Переходим к токену после ';' (комментарий или первое поле следующего объявления)
    NextToken;
    if CurToken = tkComment then
    begin
      if VarList.Count > 0 then
        TPasVariable(VarList[VarList.Count - 1]).InlineComment := CurTokenString;
      // Пропускаем комментарий и переходим к следующему токену
      NextToken;
    end;
    // Теперь CurToken указывает на первый токен следующего объявления или на 'end'/'}'
  finally
    VarNames.Free;
  end;
end;

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
  end else
    UngetToken;
  NextToken;
  if CurToken = tkAbsolute then
    ExpectIdentifier
  else
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
            M := M + ' ' + CurTokenText;
            NextToken;
          end;
        end else
          NextToken;
        if (CurToken = tkIdentifier) and (UpperCase(CurTokenText) = 'NAME') then
        begin
          M := M + ' name ';
          NextToken;
          if (CurToken = tkString) or (CurToken = tkIdentifier) then
            M := M + CurTokenText
          else
            ParseExc(SParserSyntaxError);
          ExpectToken(tkSemicolon);
        end else if CurToken <> tkSemicolon then
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
  end;
  if M <> '' then
    for i := 0 to List.Count - 1 do
      TPasVariable(List[i]).Modifiers := M;
end;

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
      end else
        ParseExc(SParserExpectedConstVarID);
      ArgNames.Add(Name);
      NextToken;
      if CurToken = tkColon then
        break
      else if ((CurToken = tkSemicolon) or (CurToken = tkBraceClose)) and
        (Access <> argDefault) then
      begin
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
        Value := ParseExpression
      else
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

procedure TPasParser.ParseProcedureOrFunctionHeader(Parent: TPasElement;
  Element: TPasProcedureType; ProcType: TProcType; OfObjectPossible: Boolean);
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
        if Assigned(Element) then
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
        if Assigned(Element) then
          TPasFunctionType(Element).ResultEl.ResultType := ParseType(Parent)
        else
          ParseType(nil);
      end;
  end;
  NextToken;
  if CurToken = tkEqual then
  begin
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
    end else
    begin
      UngetToken;
      break;
    end;
  end;
end;

procedure TPasParser.ParseProperty(Element:TPasElement);
begin
  // Пустая реализация, т.к. не используется
end;

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

function TPasParser.ParseStructure(Parent: TPasElement): TPasRecordType;
var
  StructName: string;
  Struct: TPasRecordType;
begin
  StructName := ExpectIdentifier;
  Struct := TPasRecordType(CreateElement(TPasRecordType, StructName, Parent));
  ExpectToken(tkSemicolon);
  NextToken;  // переходим к первому токену внутри структуры
  ParseRecordDecl(Struct, False);
  ExpectToken(tkSemicolon);
  Result := Struct;
end;

function TPasParser.ParseVariableDecl(Parent: TPasElement): TPasVariable;
begin
  Result := TPasVariable(CreateElement(TPasVariable, CurTokenString, Parent));
  NextToken;
  ExpectToken(tkColon);
  Result.VarType := ParseType(nil);
  ExpectToken(tkSemicolon);
end;

procedure TPasParser.ParseRecordDecl(Parent: TPasRecordType; IsNested: Boolean);
var
  VariantName: String;
  Variant: TPasVariant;
begin
  while True do
  begin
    // Проверяем условие выхода без смещения токена
    if IsNested then
    begin
      if CurToken = tkBraceClose then
        break;
    end else
    begin
      if CurToken = tkEnd then
        break;
    end;

    // Пропускаем комментарии между объявлениями
    if CurToken = tkComment then
    begin
      NextToken;
      Continue;
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
        NextToken;  // переходим к первому токену внутри варианта
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

function TPasParser.ParseClassDecl(Parent: TPasElement; const AClassName: String;
  AObjKind: TPasObjKind): TPasType;
begin
  // Заглушка, классы не используются
  Result := nil;
end;

function ExpandIncludes(const AFileName: string): string;
var
  BaseDir: string;

  function ExpandFile(const FName: string; Depth: Integer): string;
  var
    SL: TStringList;
    i: Integer;
    s, incName: string;
    p1, p2: Integer;
  begin
    if Depth > 10 then
      raise Exception.Create('Too many nested includes');
    if not FileExists(FName) then
      raise Exception.CreateFmt('Include file not found: %s', [FName]);

    SL := TStringList.Create;
    try
      SL.LoadFromFile(FName);
      Result := '';

      if Depth > 0 then
        Result := Result + '#line 1 "' + FName + '"' + sLineBreak;

      for i := 0 to SL.Count - 1 do
      begin
        s := Trim(SL[i]);

        if (Length(s) >= 7) and (Copy(s, 1, 7) = 'include') then
        begin
          p1 := Pos('''', s);
          if p1 > 0 then
          begin
            p2 := Pos('''', Copy(s, p1 + 1, Length(s) - p1));
            if p2 > 0 then
            begin
              incName := Copy(s, p1 + 1, p2 - 1);
              if incName <> '' then
              begin
                if ExtractFilePath(incName) = '' then
                  incName := ExtractFilePath(FName) + incName;

                // ОТЛАДКА: вывод информации о включении
                WriteLn('DEBUG ExpandIncludes: включаю файл "', incName, '" на глубине ', Depth + 1);

                Result := Result + '#line 1 "' + incName + '"' + sLineBreak;
                Result := Result + ExpandFile(incName, Depth + 1) + sLineBreak;
                Result := Result + '#line ' + IntToStr(i + 2) + ' "' + FName + '"' + sLineBreak;
              end
              else
                Result := Result + SL[i] + sLineBreak;
            end
            else
              Result := Result + SL[i] + sLineBreak;
          end
          else
            Result := Result + SL[i] + sLineBreak;
        end
        else
          Result := Result + SL[i] + sLineBreak;
      end;
    finally
      SL.Free;
    end;
  end;

begin
  Result := ExpandFile(ExpandFileName(AFileName), 0);
end;

function ParseSource(AEngine: TPasTreeContainer;
  const FPCCommandLine, OSTarget, CPUTarget: String): TPasModule;
var
  FileResolver: TFileResolver;
  Parser: TPasParser;
  Filename: String;
  Scanner: TPascalScanner;
  ExpandedText: string;
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
    Parser := TPasParser.Create(Scanner, FileResolver, AEngine);

    Filename := FPCCommandLine;
    if Filename = '' then
      raise Exception.Create(SErrNoSourceGiven);

    ExpandedText := ExpandIncludes(Filename);

    // ОТЛАДКА: выводим размер текста и первые строки
    WriteLn('DEBUG ParseSource: размер ExpandedText = ', Length(ExpandedText));
    if Length(ExpandedText) > 0 then
    begin
      WriteLn('DEBUG ParseSource: первые 200 символов:');
      WriteLn(Copy(ExpandedText, 1, 200));
    end;

    Scanner.OpenString(ExpandedText, Filename);
    Parser.ParseMain(Result);
  finally
    Parser.Free;
    Scanner.Free;
    FileResolver.Free;
  end;
end;

end.
