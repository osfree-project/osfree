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
  SErrIfXXXNestingLimitReached = 'Nesting of $IFxxx too deep';
  SErrInvalidPPElse = '$ELSE without matching $IFxxx';
  SErrInvalidPPEndif = '$ENDIF without matching $IFxxx';

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
    tkasm,
    tkbegin,
    tkcase,
    tkclass,
    tkconst,
    tkconstructor,
    tkdestructor,
    tkdiv,
    tkdo,
    tkdownto,
    tkelse,
    tkend,
    tkexcept,
    tkexports,
    tkfalse,
    tkfile,
    tkfinalization,
    tkfinally,
    tkfor,
    tkfunction,
    tkgoto,
    tkif,
//    tkimplementation,
    tkin,
    tkinherited,
    tkinitialization,
    tkinline,
//    tkinterface,
    tkerror,
    tkis,
    tklabel,
    tklibrary,
    tkmod,
    tknil,
    tknot,
    tkobject,
    tkof,
    tkon,
    tkoperator,
    tkor,
    tkpacked,
    tkprocedure,
    tkprogram,
    tkproperty,
    tkraise,
    tkrecord,
    tkrepeat,
    tkResourceString,
    tkself,
    tkset,
    tkshl,
    tkshr,
//    tkstring,
    tkthen,
    tkthreadvar,
    tkto,
    tktrue,
    tktry,
    tktype,
    tkunit,
    tkuntil,
    tkuses,
    tkvar,
    tkwhile,
    tkwith,
    tkxor,
    tkalias
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

  TPascalScannerPPSkipMode = (ppSkipNone, ppSkipIfBranch, ppSkipElseBranch,
    ppSkipAll);

  TPOptions = (po_delphi);

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

    // Preprocessor $IFxxx skipping data
    PPSkipMode: TPascalScannerPPSkipMode;
    PPIsSkipping: Boolean;
    PPSkipStackIndex: Integer;
    PPSkipModeStack: array[0..255] of TPascalScannerPPSkipMode;
    PPIsSkippingStack: array[0..255] of Boolean;

    function GetCurColumn: Integer;
  protected
    procedure Error(const Msg: string);
    procedure Error(const Msg: string; Args: array of Const);
    function DoFetchToken: TToken;
  public
    Options : set of TPOptions;
    constructor Create(AFileResolver: TFileResolver);
    destructor Destroy; override;
    procedure OpenFile(const AFilename: string);
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
    'asm',
    'begin',
    'case',
    'class',
    'const',
    'constructor',
    'destructor',
    'div',
    'do',
    'downto',
    'else',
    'end',
    'except',
    'exports',
    'false',
    'file',
    'finalization',
    'finally',
    'for',
    'function',
    'goto',
    'if',
//    'implementation',
    'in',
    'inherited',
    'initialization',
    'inline',
//    'interface',
    'error',
    'is',
    'label',
    'library',
    'mod',
    'nil',
    'not',
    'object',
    'of',
    'on',
    'operator',
    'or',
    'packed',
    'procedure',
    'program',
    'property',
    'raise',
    'structure',
    'repeat',
    'resourcestring',
    'self',
    'set',
    'shl',
    'shr',
//    'string',
    'then',
    'threadvar',
    'to',
    'true',
    'try',
    'type',
    'module',
    'until',
    'uses',
    'var',
    'while',
    'with',
    'xor',
    'alias'
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
  // Dont' free the first element, because it is CurSourceFile
  while FIncludeStack.Count > 1 do
  begin
    TFileResolver(FIncludeStack[1]).Free;
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
      if not PPIsSkipping then
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
  TokenStart, CurPos: PChar;
  i: TToken;
  OldLength, SectionLength, NestingLevel, Index: Integer;
  Directive, Param: string;
  IncludeStackItem: TIncludeStackItem;
begin
  if TokenStr = nil then
    if not FetchLine then
    begin
      Result := tkEOF;
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
          until not (TokenStr[0] in ['0'..'9', 'A'..'F', 'a'..'F']);
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
        until not (TokenStr[0] in ['0'..'9', 'A'..'F', 'a'..'F']);
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
        Inc(TokenStr);
        if TokenStr[0] = '*' then
        begin
          // Old-style multi-line comment
          Inc(TokenStr);
          while (TokenStr[0] <> '*') or (TokenStr[1] <> ')') do
          begin
            if TokenStr[0] = #0 then
            begin
              if not FetchLine then
              begin
                Result := tkEOF;
                FCurToken := Result;
                exit;
              end;
            end else
              Inc(TokenStr);
          end;
          Inc(TokenStr, 2);
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
        Inc(TokenStr);
        if TokenStr[0] = '/' then       // Single-line comment
        begin
          Inc(TokenStr);
          TokenStart := TokenStr;
          FCurTokenString := '';
          while TokenStr[0] <> #0 do
            Inc(TokenStr);
          SectionLength := TokenStr - TokenStart;
          SetLength(FCurTokenString, SectionLength);
          if SectionLength > 0 then
            Move(TokenStart^, FCurTokenString[1], SectionLength);
          Result := tkComment;
          //WriteLn('Einzeiliger Kommentar: "', CurTokenString, '"');
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
    '{':        // Multi-line comment
      begin
        Inc(TokenStr);
        TokenStart := TokenStr;
        FCurTokenString := '';
        OldLength := 0;
        NestingLevel := 0;
        while (TokenStr[0] <> '}') or (NestingLevel > 0) do
        begin
          if TokenStr[0] = #0 then
          begin
            SectionLength := TokenStr - TokenStart + 1;
            SetLength(FCurTokenString, OldLength + SectionLength);
            if SectionLength > 1 then
              Move(TokenStart^, FCurTokenString[OldLength + 1],
                SectionLength - 1);
            Inc(OldLength, SectionLength);
            FCurTokenString[OldLength] := #10;
            if not FetchLine then
            begin
              Result := tkEOF;
              FCurToken := Result;
              exit;
            end;
            TokenStart := TokenStr;
          end else
          begin
            if not(po_delphi in Options) and (TokenStr[0] = '{') then
              Inc(NestingLevel)
            else if TokenStr[0] = '}' then
              Dec(NestingLevel);
            Inc(TokenStr);
          end;
        end;
        SectionLength := TokenStr - TokenStart;
        SetLength(FCurTokenString, OldLength + SectionLength);
        if SectionLength > 0 then
          Move(TokenStart^, FCurTokenString[OldLength + 1], SectionLength);
        Inc(TokenStr);
        Result := tkComment;
        //WriteLn('Kommentar: "', CurTokenString, '"');
        if (Length(CurTokenString) > 0) and (CurTokenString[1] = '$') then
        begin
          TokenStart := @CurTokenString[2];
          CurPos := TokenStart;
          while (CurPos[0] <> ' ') and (CurPos[0] <> #0) do
            Inc(CurPos);
          SectionLength := CurPos - TokenStart;
          SetLength(Directive, SectionLength);
          if SectionLength > 0 then
          begin
            Move(TokenStart^, Directive[1], SectionLength);
            Directive := UpperCase(Directive);
            if CurPos[0] <> #0 then
            begin
              TokenStart := CurPos + 1;
              CurPos := TokenStart;
              while CurPos[0] <> #0 do
                Inc(CurPos);
              SectionLength := CurPos - TokenStart;
              SetLength(Param, SectionLength);
              if SectionLength > 0 then
                Move(TokenStart^, Param[1], SectionLength);
            end else
              Param := '';
            // WriteLn('Direktive: "', Directive, '", Param: "', Param, '"');
            if (Directive = 'I') or (Directive = 'INCLUDE') then
            begin
              if not PPIsSkipping then
              begin
                IncludeStackItem := TIncludeStackItem.Create;
                IncludeStackItem.SourceFile := CurSourceFile;
                IncludeStackItem.Filename := CurFilename;
                IncludeStackItem.Token := CurToken;
                IncludeStackItem.TokenString := CurTokenString;
                IncludeStackItem.Line := CurLine;
                IncludeStackItem.Row := CurRow;
                IncludeStackItem.TokenStr := TokenStr;
                FIncludeStack.Add(IncludeStackItem);
                FCurSourceFile := FileResolver.FindIncludeFile(Param);
                if not Assigned(CurSourceFile) then
                  Error(SErrIncludeFileNotFound, [Param]);
                FCurFilename := Param;
                FCurRow := 0;
              end;
            end else if Directive = 'DEFINE' then
            begin
              if not PPIsSkipping then
              begin
                Param := UpperCase(Param);
                if Defines.IndexOf(Param) < 0 then
                  Defines.Add(Param);
              end;
            end else if Directive = 'UNDEF' then
            begin
              if not PPIsSkipping then
              begin
                Param := UpperCase(Param);
                Index := Defines.IndexOf(Param);
                if Index >= 0 then
                  Defines.Delete(Index);
              end;
            end else if Directive = 'IFDEF' then
            begin
              if PPSkipStackIndex = High(PPSkipModeStack) then
                Error(SErrIfXXXNestingLimitReached);
              PPSkipModeStack[PPSkipStackIndex] := PPSkipMode;
              PPIsSkippingStack[PPSkipStackIndex] := PPIsSkipping;
              Inc(PPSkipStackIndex);
              if PPIsSkipping then
              begin
                PPSkipMode := ppSkipAll;
                PPIsSkipping := true;
              end else
              begin
                Param := UpperCase(Param);
                Index := Defines.IndexOf(Param);
                if Index < 0 then
                begin
                  PPSkipMode := ppSkipIfBranch;
                  PPIsSkipping := true;
                end else
                  PPSkipMode := ppSkipElseBranch;
              end;
            end else if Directive = 'IFNDEF' then
            begin
              if PPSkipStackIndex = High(PPSkipModeStack) then
                Error(SErrIfXXXNestingLimitReached);
              PPSkipModeStack[PPSkipStackIndex] := PPSkipMode;
              PPIsSkippingStack[PPSkipStackIndex] := PPIsSkipping;
              Inc(PPSkipStackIndex);
              if PPIsSkipping then
              begin
                PPSkipMode := ppSkipAll;
                PPIsSkipping := true;
              end else
              begin
                Param := UpperCase(Param);
                Index := Defines.IndexOf(Param);
                if Index >= 0 then
                begin
                  PPSkipMode := ppSkipIfBranch;
                  PPIsSkipping := true;
                end else
                  PPSkipMode := ppSkipElseBranch;
              end;
            end else if Directive = 'IFOPT' then
            begin
              if PPSkipStackIndex = High(PPSkipModeStack) then
                Error(SErrIfXXXNestingLimitReached);
              PPSkipModeStack[PPSkipStackIndex] := PPSkipMode;
              PPIsSkippingStack[PPSkipStackIndex] := PPIsSkipping;
              Inc(PPSkipStackIndex);
              if PPIsSkipping then
              begin
                PPSkipMode := ppSkipAll;
                PPIsSkipping := true;
              end else
              begin
                { !!!: Currently, options are not supported, so they are just
                  assumed as not being set. }
                PPSkipMode := ppSkipIfBranch;
                PPIsSkipping := true;
              end;
            end else if Directive = 'IF' then
            begin
              if PPSkipStackIndex = High(PPSkipModeStack) then
                Error(SErrIfXXXNestingLimitReached);
              PPSkipModeStack[PPSkipStackIndex] := PPSkipMode;
              PPIsSkippingStack[PPSkipStackIndex] := PPIsSkipping;
              Inc(PPSkipStackIndex);
              if PPIsSkipping then
              begin
                PPSkipMode := ppSkipAll;
                PPIsSkipping := true;
              end else
              begin
                { !!!: Currently, expressions are not supported, so they are
                  just assumed as evaluating to false. }
                PPSkipMode := ppSkipIfBranch;
                PPIsSkipping := true;
              end;
            end else if Directive = 'ELSE' then
            begin
              if PPSkipStackIndex = 0 then
                Error(SErrInvalidPPElse);
              if PPSkipMode = ppSkipIfBranch then
                PPIsSkipping := false
              else if PPSkipMode = ppSkipElseBranch then
                PPIsSkipping := true;
            end else if Directive = 'ENDIF' then
            begin
              if PPSkipStackIndex = 0 then
                Error(SErrInvalidPPEndif);
              Dec(PPSkipStackIndex);
              PPSkipMode := PPSkipModeStack[PPSkipStackIndex];
              PPIsSkipping := PPIsSkippingStack[PPSkipStackIndex];
            end;
          end else
            Directive := '';
        end;
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

        // Check if this is a keyword or identifier
        // !!!: Optimize this!
        for i := tkAbsolute to tkXOR do
          if CompareText(CurTokenString, TokenInfos[i]) = 0 then
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
