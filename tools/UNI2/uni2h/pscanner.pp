{
    This file is part of the Free Component Library

    Pascal source lexical scanner
    Copyright (c) 2003 by
      Areca Systems GmbH / Sebastian Guenther, sg@freepascal.org

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}


unit PScanner;

interface

uses SysUtils, Classes;

resourcestring
  SErrInvalidCharacter = 'Invalid character ''%s''';
  SErrOpenString = 'string exceeds end of line';
  SErrIncludeFileNotFound = 'Could not find include file ''%s''';

type
  TToken = (
    tkEOF,
    tkWhitespace,
    tkComment,
    tkIdentifier,
    tkString,
    tkNumber,
    tkChar,
    // Simple (one-character) tokens
    tkBraceOpen,             // '('
    tkBraceClose,            // ')'
    tkMul,                   // '*'
    tkPlus,                  // '+'
    tkComma,                 // ','
    tkMinus,                 // '-'
    tkDot,                   // '.'
    tkDivision,              // '/'
    tkColon,                 // ':'
    tkSemicolon,             // ';'
    tkLessThan,              // '<'
    tkEqual,                 // '='
    tkGreaterThan,           // '>'
    tkAt,                    // '@'
    tkSquaredBraceOpen,      // '['
    tkSquaredBraceClose,     // ']'
    tkCaret,                 // '^'
    // Two-character tokens
    tkDotDot,                // '..'
    tkAssign,                // ':='
    tkNotEqual,              // '<>'
    tkLessEqualThan,         // '<='
    tkGreaterEqualThan,      // '>='
    tkPower,                 // '**'
    tkSymmetricalDifference, // '><'
    // Reserved words
    tkabsolute,
    tkand,
    tkarray,
    tkas,
    tkbegin,
    tkcase,
    tkclass,
    tkconst,
    tkInArg,
    tkInOutArg,
    tkOutArg,
        tkdefault,
        tkalias,
    tkconstructor,
    tkdestructor,
    tkdiv,
    tkdo,
    tkend,
    tkfalse,
    tkfile,
    tkfinally,
    tkfunction,
    tkinherited,
    tkerror,
    tkis,
    tkmod,
    tknil,
    tknot,
    tkof,
    tkon,
    tkor,
    tkprocedure,
    tkproperty,
    tkset,
    tkshl,
    tkshr,
    tkto,
    tktrue,
    tktype,
    tkGroup,
    tkuses,
    tkvar,
    tkwith,
    tkxor,
    // Новые ключевые слова для uni2h v2.0
    tkStructure,
    tkVariable
    );

  TLineReader = class
  public
    function IsEOF: Boolean; virtual; abstract;
    function ReadLine: string; virtual; abstract;
  end;

  TFileLineReader = class(TLineReader)
  private
    FTextFile: Text;
    FileOpened: Boolean;
  public
    constructor Create(const AFilename: string);
    destructor Destroy; override;
    function IsEOF: Boolean; override;
    function ReadLine: string; override;
  end;

  TStringLineReader = class(TLineReader)
  private
    FLines: TStringList;
    FIndex: Integer;
  public
    constructor Create(const AText: string);
    destructor Destroy; override;
    function IsEOF: Boolean; override;
    function ReadLine: string; override;
  end;

  TFileResolver = class
  private
    FIncludePaths: TStringList;
  public
    constructor Create;
    destructor Destroy; override;
    procedure AddIncludePath(const APath: string);
    function FindSourceFile(const AName: string): TLineReader;
    function FindIncludeFile(const AName: string): TLineReader;
  end;

  EScannerError       = class(Exception);
  EFileNotFoundError  = class(Exception);



  TPascalScanner = class
  private
    FFileResolver: TFileResolver;
    FCurSourceFile: TLineReader;
    FCurFilename: string;
    FCurRow: Integer;
    FCurToken: TToken;
    FCurTokenString: string;
    FCurLine: string;
    FDefines: TStrings;
    TokenStr: PChar;
    FIncludeStack: TList;

    function GetCurColumn: Integer;
  protected
    procedure Error(const Msg: string);
    procedure Error(const Msg: string; Args: array of Const);
    function DoFetchToken: TToken;
  public
    constructor Create(AFileResolver: TFileResolver);
    destructor Destroy; override;
    procedure OpenFile(const AFilename: string);
    procedure OpenString(const AText: string; const ANameForMessages: string);
    function FetchToken: TToken;

    property FileResolver: TFileResolver read FFileResolver;
    property CurSourceFile: TLineReader read FCurSourceFile;
    property CurFilename: string read FCurFilename;

    property CurLine: string read FCurLine;
    property CurRow: Integer read FCurRow;
    property CurColumn: Integer read GetCurColumn;

    property CurToken: TToken read FCurToken;
    property CurTokenString: string read FCurTokenString;

    property Defines: TStrings read FDefines;
  end;

const
  TokenInfos: array[TToken] of string = (
    'EOF',
    'Whitespace',
    'Comment',
    'Identifier',
    'string',
    'Number',
    'Character',
    '(',
    ')',
    '*',
    '+',
    ',',
    '-',
    '.',
    '/',
    ':',
    ';',
    '<',
    '=',
    '>',
    '@',
    '[',
    ']',
    '^',
    '..',
    ':=',
    '<>',
    '<=',
    '>=',
    '**',
    '><',
    // Reserved words
    'absolute',
    'and',
    'array',
    'as',
    'begin',
    'case',
    'class',
    'constant',
    'in',
    'inout',
    'out',
    'default',
    'alias',
    'constructor',
    'destructor',
    'div',
    'do',
    'end',
    'false',
    'file',
    'finally',
    'function',
    'inherited',
    'error',
    'is',
    'mod',
    'nil',
    'not',
    'of',
    'on',
    'or',
    'procedure',
    'property',
    'set',
    'shl',
    'shr',
    'to',
    'true',
    'type',
    'group',
    'uses',
    'var',
    'with',
    'xor',
    'structure',
    'variable'
  );


implementation

type
  TIncludeStackItem = class
    SourceFile: TLineReader;
    Filename: string;
    Token: TToken;
    TokenString: string;
    Line: string;
    Row: Integer;
    TokenStr: PChar;
  end;


constructor TFileLineReader.Create(const AFilename: string);
begin
  inherited Create;
  Assign(FTextFile, AFilename);
  Reset(FTextFile);
  FileOpened := true;
end;

destructor TFileLineReader.Destroy;
begin
  if FileOpened then
    Close(FTextFile);
  inherited Destroy;
end;

function TFileLineReader.IsEOF: Boolean;
begin
  Result := EOF(FTextFile);
end;

function TFileLineReader.ReadLine: string;
begin
  ReadLn(FTextFile, Result);
end;


constructor TStringLineReader.Create(const AText: string);
begin
  inherited Create;
  FLines := TStringList.Create;
  FLines.Text := AText;
  FIndex := 0;
end;

destructor TStringLineReader.Destroy;
begin
  FLines.Free;
  inherited Destroy;
end;

function TStringLineReader.IsEOF: Boolean;
begin
  Result := FIndex >= FLines.Count;
end;

function TStringLineReader.ReadLine: string;
begin
  // ДОПОЛНИТЕЛЬНАЯ ОТЛАДКА
  {$IFDEF DEBUG}
  WriteLn('DEBUG TStringLineReader.ReadLine: FIndex=', FIndex, ' Count=', FLines.Count);
  {$ENDIF}
  if FIndex >= FLines.Count then
  begin
    {$IFDEF DEBUG}
    WriteLn('DEBUG TStringLineReader.ReadLine: ВЫХОД ЗА ГРАНИЦЫ! Возвращаю пустую строку.');
    {$ENDIF}
    Result := '';
    Exit;
  end;
  Result := FLines[FIndex];
  {$IFDEF DEBUG}
  WriteLn('DEBUG TStringLineReader.ReadLine: строка="', Result, '"');
  {$ENDIF}
  Inc(FIndex);
end;


constructor TFileResolver.Create;
begin
  inherited Create;
  FIncludePaths := TStringList.Create;
end;

destructor TFileResolver.Destroy;
begin
  FIncludePaths.Free;
  inherited Destroy;
end;

procedure TFileResolver.AddIncludePath(const APath: string);
begin
  FIncludePaths.Add(IncludeTrailingPathDelimiter(ExpandFileName(APath)));
end;

function TFileResolver.FindSourceFile(const AName: string): TLineReader;
begin
  if not FileExists(AName) then
    Raise EFileNotFoundError.create(Aname)
  else
    try
      Result := TFileLineReader.Create(AName);
    except
      Result := nil;
    end;
end;

function TFileResolver.FindIncludeFile(const AName: string): TLineReader;
var
  i: Integer;
  FN : string;

begin
  Result := nil;
  If FileExists(AName) then
    Result := TFileLineReader.Create(AName)
  else
    begin
    I:=0;
    While (Result=Nil) and (I<FIncludePaths.Count) do
      begin
      Try
        FN:=FIncludePaths[i]+AName;
        If FileExists(FN) then
          Result := TFileLineReader.Create(FN);
      except
        Result:=Nil;
      end;
      Inc(I);
      end;
    end;
end;


constructor TPascalScanner.Create(AFileResolver: TFileResolver);
begin
  inherited Create;
  FFileResolver := AFileResolver;
  FIncludeStack := TList.Create;
  FDefines := TStringList.Create;
end;

destructor TPascalScanner.Destroy;
begin
  FDefines.Free;
  while FIncludeStack.Count > 1 do
  begin
    TIncludeStackItem(FIncludeStack[1]).Free;
    FIncludeStack.Delete(1);
  end;
  FIncludeStack.Free;

  CurSourceFile.Free;
  inherited Destroy;
end;

procedure TPascalScanner.OpenFile(const AFilename: string);
begin
  FCurSourceFile := FileResolver.FindSourceFile(AFilename);
  FCurFilename := AFilename;
  TokenStr := nil;
  FCurRow := 0;
  FCurLine := '';
end;

procedure TPascalScanner.OpenString(const AText: string; const ANameForMessages: string);
begin
  FCurSourceFile := TStringLineReader.Create(AText);
  FCurFilename := ANameForMessages;
  TokenStr := nil;
  FCurRow := 0;
  FCurLine := '';
end;

function TPascalScanner.FetchToken: TToken;
var
  IncludeStackItem: TIncludeStackItem;
begin
  while true do
  begin
    Result := DoFetchToken;
    if FCurToken = tkEOF then
      if FIncludeStack.Count > 0 then
      begin
        CurSourceFile.Free;
        IncludeStackItem :=
          TIncludeStackItem(FIncludeStack[FIncludeStack.Count - 1]);
        FIncludeStack.Delete(FIncludeStack.Count - 1);
        FCurSourceFile := IncludeStackItem.SourceFile;
        FCurFilename := IncludeStackItem.Filename;
        FCurToken := IncludeStackItem.Token;
        FCurTokenString := IncludeStackItem.TokenString;
        FCurLine := IncludeStackItem.Line;
        FCurRow := IncludeStackItem.Row;
        TokenStr := IncludeStackItem.TokenStr;
        IncludeStackItem.Free;
        Result := FCurToken;
      end else
        break
    else
      break;
  end;
end;

procedure TPascalScanner.Error(const Msg: string);
begin
  raise EScannerError.Create(Msg);
end;

procedure TPascalScanner.Error(const Msg: string; Args: array of Const);
begin
  raise EScannerError.CreateFmt(Msg, Args);
end;

function TPascalScanner.DoFetchToken: TToken;

  function FetchLine: Boolean;
  begin
    if CurSourceFile.IsEOF then
    begin
      FCurLine := '';
      TokenStr := nil;
      Result := false;
    end else
    begin
      FCurLine := CurSourceFile.ReadLine;
      TokenStr := PChar(CurLine);
      Result := true;
      Inc(FCurRow);
    end;
  end;

var
  TokenStart: PChar;
  i: TToken;
  OldLength, SectionLength: Integer;
  p: Integer;
  lineNum: Integer;
  sComment: string;
begin
  if TokenStr = nil then
    if not FetchLine then
    begin
      Result := tkEOF;
      FCurToken := Result;
      exit;
    end;

  // Проверка на директиву #line
  if (Length(FCurLine) >= 5) and (Copy(FCurLine, 1, 5) = '#line') then
  begin
    WriteLn('DEBUG DoFetchToken: обнаружена #line -> ', FCurLine);

    Delete(FCurLine, 1, 5);
    FCurLine := TrimLeft(FCurLine);

    p := 1;
    while (p <= Length(FCurLine)) and (FCurLine[p] <> ' ') and (FCurLine[p] <> '"') do
      Inc(p);
    if p > 1 then
    begin
      lineNum := StrToIntDef(Copy(FCurLine, 1, p-1), -1);
      if lineNum >= 0 then
        FCurRow := lineNum - 1;
    end;

    Delete(FCurLine, 1, p-1);
    FCurLine := TrimLeft(FCurLine);

    if (Length(FCurLine) >= 2) and (FCurLine[1] = '"') then
    begin
      Delete(FCurLine, 1, 1);
      p := Pos('"', FCurLine);
      if p > 0 then
      begin
        FCurFilename := Copy(FCurLine, 1, p-1);
      end;
    end;

    WriteLn('DEBUG DoFetchToken: #line обработана, FCurFilename=', FCurFilename, ', FCurRow=', FCurRow);

    TokenStr := nil;
    Result := tkWhitespace;
    FCurToken := Result;
    exit;
  end;

  FCurTokenString := '';

  case TokenStr[0] of
    #0:         // Empty line
      begin
        FetchLine;
        Result := tkWhitespace;
      end;
    #9, ' ':
      begin
        Result := tkWhitespace;
        repeat
          Inc(TokenStr);
          if TokenStr[0] = #0 then
            if not FetchLine then
            begin
              FCurToken := Result;
              exit;
            end;
        until not (TokenStr[0] in [#9, ' ']);
      end;
    '#':
      begin
        TokenStart := TokenStr;
        Inc(TokenStr);
        if TokenStr[0] = '$' then
        begin
          Inc(TokenStr);
          repeat
            Inc(TokenStr);
          until not (TokenStr[0] in ['0'..'9', 'A'..'F', 'a'..'f']);
        end else
          repeat
            Inc(TokenStr);
          until not (TokenStr[0] in ['0'..'9']);

        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[1], SectionLength);
        Result := tkChar;
      end;
    '&':
      begin
        TokenStart := TokenStr;
        repeat
          Inc(TokenStr);
        until not (TokenStr[0] in ['0'..'7']);
        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[1], SectionLength);
        Result := tkNumber;
      end;
    '$':
      begin
        TokenStart := TokenStr;
        repeat
          Inc(TokenStr);
        until not (TokenStr[0] in ['0'..'9', 'A'..'F', 'a'..'f']);
        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[1], SectionLength);
        Result := tkNumber;
      end;
    '%':
      begin
        TokenStart := TokenStr;
        repeat
          Inc(TokenStr);
        until not (TokenStr[0] in ['0','1']);
        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[1], SectionLength);
        Result := tkNumber;
      end;
    '''':
      begin
        Inc(TokenStr);
        TokenStart := TokenStr;
        OldLength := 0;
        FCurTokenString := '';

        while true do
        begin
          if TokenStr[0] = '''' then
            if TokenStr[1] = '''' then
            begin
              SectionLength := TokenStr - TokenStart + 1;
              SetLength(FCurTokenString, OldLength + SectionLength);
              if SectionLength > 0 then
                Move(TokenStart^, FCurTokenString[OldLength + 1], SectionLength);
              Inc(OldLength, SectionLength);
              Inc(TokenStr);
              TokenStart := TokenStr+1;
            end else
              break;

          if TokenStr[0] = #0 then
            Error(SErrOpenString);

          Inc(TokenStr);
        end;

        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, OldLength + SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[OldLength + 1], SectionLength);

        Inc(TokenStr);
        Result := tkString;
      end;
    '(':
      begin
        TokenStart := TokenStr;
        Inc(TokenStr);
        if TokenStr[0] = '*' then
        begin
          Inc(TokenStr);
          sComment := '';
          while True do
          begin
            if TokenStr[0] = #0 then
            begin
              sComment := sComment + sLineBreak;
              if not FetchLine then
              begin
                Result := tkEOF;
                FCurToken := Result;
                exit;
              end;
            end;
            if (TokenStr[0] = '*') and (TokenStr[1] = ')') then
              Break;
            sComment := sComment + TokenStr[0];
            Inc(TokenStr);
          end;
          Inc(TokenStr, 2);
          FCurTokenString := sComment;
          Result := tkComment;
        end else
          Result := tkBraceOpen;
      end;
    ')':
      begin
        Inc(TokenStr);
        Result := tkBraceClose;
      end;
    '*':
      begin
        Inc(TokenStr);
        if TokenStr[0] = '*' then
        begin
          Inc(TokenStr);
          Result := tkPower;
        end else
          Result := tkMul;
      end;
    '+':
      begin
        Inc(TokenStr);
        Result := tkPlus;
      end;
    ',':
      begin
        Inc(TokenStr);
        Result := tkComma;
      end;
    '-':
      begin
        Inc(TokenStr);
        Result := tkMinus;
      end;
    '.':
      begin
        Inc(TokenStr);
        if TokenStr[0] = '.' then
        begin
          Inc(TokenStr);
          Result := tkDotDot;
        end else
          Result := tkDot;
      end;
    '/':
      begin
        TokenStart := TokenStr;
        Inc(TokenStr);
        if TokenStr[0] = '/' then
        begin
          Inc(TokenStr);
          while (TokenStr[0] <> #0) do
            Inc(TokenStr);
          SectionLength := TokenStr - TokenStart - 2;
          SetLength(FCurTokenString, SectionLength);
          if SectionLength > 0 then
            Move((TokenStart+2)^, FCurTokenString[1], SectionLength);
          Result := tkComment;
        end else
          Result := tkDivision;
      end;
    '0'..'9':
      begin
        TokenStart := TokenStr;
        while true do
        begin
          Inc(TokenStr);
          case TokenStr[0] of
            '.':
              begin
                if TokenStr[1] in ['0'..'9', 'e', 'E'] then
                begin
                  Inc(TokenStr);
                  repeat
                    Inc(TokenStr);
                  until not (TokenStr[0] in ['0'..'9', 'e', 'E']);
                end;
                break;
              end;
            '0'..'9': ;
            'e', 'E':
              begin
                Inc(TokenStr);
                if TokenStr[0] = '-'  then
                  Inc(TokenStr);
                while TokenStr[0] in ['0'..'9'] do
                  Inc(TokenStr);
                break;
              end;
            else
              break;
          end;
        end;
        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[1], SectionLength);
        Result := tkNumber;
      end;
    ':':
      begin
        Inc(TokenStr);
        if TokenStr[0] = '=' then
        begin
          Inc(TokenStr);
          Result := tkAssign;
        end else
          Result := tkColon;
      end;
    ';':
      begin
        Inc(TokenStr);
        Result := tkSemicolon;
      end;
    '<':
      begin
        Inc(TokenStr);
        if TokenStr[0] = '>' then
        begin
          Inc(TokenStr);
          Result := tkNotEqual;
        end else if TokenStr[0] = '=' then
        begin
          Inc(TokenStr);
          Result := tkLessEqualThan;
        end else
          Result := tkLessThan;
      end;
    '=':
      begin
        Inc(TokenStr);
        Result := tkEqual;
      end;
    '>':
      begin
        Inc(TokenStr);
        if TokenStr[0] = '=' then
        begin
          Inc(TokenStr);
          Result := tkGreaterEqualThan;
        end else if TokenStr[0] = '<' then
        begin
          Inc(TokenStr);
          Result := tkSymmetricalDifference;
        end else
          Result := tkGreaterThan;
      end;
    '@':
      begin
        Inc(TokenStr);
        Result := tkAt;
      end;
    '[':
      begin
        Inc(TokenStr);
        Result := tkSquaredBraceOpen;
      end;
    ']':
      begin
        Inc(TokenStr);
        Result := tkSquaredBraceClose;
      end;
    '^':
      begin
        Inc(TokenStr);
        Result := tkCaret;
      end;
    'A'..'Z', 'a'..'z', '_':
      begin
        TokenStart := TokenStr;
        repeat
          Inc(TokenStr);
        until not (TokenStr[0] in ['A'..'Z', 'a'..'z', '0'..'9', '_']);
        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[1], SectionLength);

        for i := tkAbsolute to tkVariable do
          if CompareStr(CurTokenString, TokenInfos[i]) = 0 then
          begin
            Result := i;
            FCurToken := Result;
            exit;
          end;

        Result := tkIdentifier;
      end;
  else
    Error(SErrInvalidCharacter, [TokenStr[0]]);
  end;

  FCurToken := Result;
end;

function TPascalScanner.GetCurColumn: Integer;
begin
  Result := TokenStr - PChar(CurLine);
end;

end.
