unit ABIReader;

interface

uses Classes, SysUtils;

type
  TABIModuleType = (mtDLL, mtLibrary);

  TABIEntry = class
    LogicalName: string;
    RealName: string;
    Number: Integer;
    Convention: string;
    VersionMin: string;
    VersionMax: string;
    Inputs: string;
    Outputs: string;
    IsAlias: Boolean;
    AliasFor: string;
  end;

  TABIBitField = class
    BitName: string;
    BitPos: Integer;
    Width: Integer;
  end;

  TABIStructure = class
    Name: string;
    Pack: Integer;
    Prefix: string;
    BitFields: TList;
    constructor Create;
    destructor Destroy; override;
  end;

  TABICallback = class
    Name: string;
    Convention: string;
  end;

  TABIModule = class
    Name: string;
    ModuleType: TABIModuleType;
    VersionMin: string;
    VersionMax: string;
    Entries: TList;
    constructor Create;
    destructor Destroy; override;
  end;

  TABIData = class
    Modules: TList;
    Structures: TList;
    Callbacks: TList;
    constructor Create;
    destructor Destroy; override;
    function FindEntry(const LogicalName: string): TList;
    function FindStructure(const Name: string): TABIStructure;
    function FindCallback(const Name: string): TABICallback;
  end;

  EABIError = class(Exception);

procedure LoadABI(const AFilename: string; out ABI: TABIData);

implementation

type
  TABIParser = class
  private
    F: TextFile;
    Line: string;
    LineNum: Integer;
    procedure Error(const Msg: string);
    procedure NextLine;
    procedure SkipEmptyLines;
    function ReadToken: string;
    function PeekToken: string;
    procedure Expect(const S: string);
    function ReadNumber: Integer;
    function ReadString: string;
    function ReadVersion: string;
    procedure ParseStructure(ABI: TABIData);
    procedure ParseType(ABI: TABIData);
    procedure ParseModule(ABI: TABIData; ModuleType: TABIModuleType);
    procedure ParseEntry(Module: TABIModule);
  end;

constructor TABIStructure.Create;
begin
  BitFields := TList.Create;
end;

destructor TABIStructure.Destroy;
var
  i: Integer;
begin
  for i := 0 to BitFields.Count - 1 do
    TABIBitField(BitFields[i]).Free;
  BitFields.Free;
  inherited;
end;

constructor TABIModule.Create;
begin
  Entries := TList.Create;
end;

destructor TABIModule.Destroy;
var
  i: Integer;
begin
  for i := 0 to Entries.Count - 1 do
    TABIEntry(Entries[i]).Free;
  Entries.Free;
  inherited;
end;

constructor TABIData.Create;
begin
  Modules := TList.Create;
  Structures := TList.Create;
  Callbacks := TList.Create;
end;

destructor TABIData.Destroy;
var
  i: Integer;
begin
  for i := 0 to Modules.Count - 1 do
    TABIModule(Modules[i]).Free;
  Modules.Free;
  for i := 0 to Structures.Count - 1 do
    TABIStructure(Structures[i]).Free;
  Structures.Free;
  for i := 0 to Callbacks.Count - 1 do
    TABICallback(Callbacks[i]).Free;
  Callbacks.Free;
  inherited;
end;

function TABIData.FindEntry(const LogicalName: string): TList;
var
  i, j: Integer;
  Modu: TABIModule;
  Entry: TABIEntry;
begin
  Result := TList.Create;
  for i := 0 to Modules.Count - 1 do
  begin
    Modu := TABIModule(Modules[i]);
    for j := 0 to Modu.Entries.Count - 1 do
    begin
      Entry := TABIEntry(Modu.Entries[j]);
      if CompareStr(Entry.LogicalName, LogicalName) = 0 then
        Result.Add(Entry);
    end;
  end;
end;

function TABIData.FindStructure(const Name: string): TABIStructure;
var
  i: Integer;
begin
  for i := 0 to Structures.Count - 1 do
  begin
    Result := TABIStructure(Structures[i]);
    if CompareStr(Result.Name, Name) = 0 then
      Exit;
  end;
  Result := nil;
end;

function TABIData.FindCallback(const Name: string): TABICallback;
var
  i: Integer;
begin
  for i := 0 to Callbacks.Count - 1 do
  begin
    Result := TABICallback(Callbacks[i]);
    if CompareStr(Result.Name, Name) = 0 then
      Exit;
  end;
  Result := nil;
end;

procedure TABIParser.Error(const Msg: string);
begin
  raise EABIError.CreateFmt('ABI Error (line %d): %s', [LineNum, Msg]);
end;

procedure TABIParser.NextLine;
begin
  if EOF(F) then
    Line := ''
  else
    ReadLn(F, Line);
  Inc(LineNum);
end;

procedure TABIParser.SkipEmptyLines;
begin
  while (TrimLeft(Line) = '') and not EOF(F) do
    NextLine;
  while (Pos('//', TrimLeft(Line)) = 1) and not EOF(F) do
    NextLine;
end;

function TABIParser.ReadToken: string;
var
  p: Integer;
begin
  SkipEmptyLines;
  Line := TrimLeft(Line);
  if Line = '' then
    Result := ''
  else if Line[1] = '''' then
  begin
    // строковый литерал
    p := 2;
    while (p <= Length(Line)) and (Line[p] <> '''') do
      Inc(p);
    if p > Length(Line) then
      Error('Unterminated string literal');
    Result := Copy(Line, 1, p);
    Delete(Line, 1, p);
  end
  else if Line[1] in [';', '=', '(', ')', ','] then
  begin
    Result := Line[1];
    Delete(Line, 1, 1);
  end
  else
  begin
    p := 1;
    while (p <= Length(Line)) and not (Line[p] in [' ', #9, ';', '=', '(', ')', ',']) do
      Inc(p);
    Result := Copy(Line, 1, p - 1);
    Delete(Line, 1, p - 1);
  end;
end;

function TABIParser.PeekToken: string;
var
  SavedLine: string;
  SavedLineNum: Integer;
begin
  SavedLine := Line;
  SavedLineNum := LineNum;
  Result := ReadToken;
  Line := SavedLine;
  LineNum := SavedLineNum;
end;

procedure TABIParser.Expect(const S: string);
var
  T: string;
begin
  T := ReadToken;
  if not SameText(T, S) then
    Error('Expected "' + S + '", got "' + T + '"');
end;

function TABIParser.ReadNumber: Integer;
var
  T: string;
begin
  T := ReadToken;
  if (Length(T) > 1) and (T[1] = '$') then
    Result := StrToIntDef('$' + Copy(T, 2, MaxInt), 0)
  else
    Result := StrToIntDef(T, 0);
end;

function TABIParser.ReadString: string;
begin
  Result := ReadToken;
  if (Length(Result) >= 2) and (Result[1] = '''') and (Result[Length(Result)] = '''') then
    Result := Copy(Result, 2, Length(Result) - 2)
  else
    Error('String literal expected');
end;

function TABIParser.ReadVersion: string;
var
  T: string;
begin
  T := ReadToken;
  if T = '''' then
    Result := ReadString
  else
    Result := T;
end;

procedure TABIParser.ParseStructure(ABI: TABIData);
var
  Stru: TABIStructure;
  FieldName: string;
  Bit: TABIBitField;
begin
  Stru := TABIStructure.Create;
  Stru.Name := ReadToken;
  Expect(';');
  SkipEmptyLines;
  if SameText(PeekToken, 'pack') then
  begin
    Expect('pack');
    Expect('=');
    Stru.Pack := ReadNumber;
    Expect(';');
    SkipEmptyLines;
  end;
  if SameText(PeekToken, 'prefix') then
  begin
    Expect('prefix');
    Expect('=');
    Stru.Prefix := ReadString;
    Expect(';');
    SkipEmptyLines;
  end;
  while SameText(PeekToken, 'bitfield') do
  begin
    Expect('bitfield');
    FieldName := ReadToken;
    Expect(';');
    SkipEmptyLines;
    while (PeekToken <> 'end') and (PeekToken <> '') do
    begin
      Bit := TABIBitField.Create;
      Bit.BitName := ReadToken;
      Expect('at');
      Bit.BitPos := ReadNumber;
      Bit.Width := 1;
      if SameText(PeekToken, 'width') then
      begin
        Expect('width');
        Bit.Width := ReadNumber;
      end;
      Expect(';');
      SkipEmptyLines;
      Stru.BitFields.Add(Bit);
    end;
    Expect('end');
    Expect(';');
    SkipEmptyLines;
  end;
  Expect('end');
  Expect(';');
  ABI.Structures.Add(Stru);
end;

procedure TABIParser.ParseType(ABI: TABIData);
var
  CB: TABICallback;
begin
  CB := TABICallback.Create;
  CB.Name := ReadToken;
  Expect(';');
  SkipEmptyLines;
  if SameText(PeekToken, 'convention') then
  begin
    Expect('convention');
    Expect('=');
    CB.Convention := ReadString;
    Expect(';');
    SkipEmptyLines;
  end;
  Expect('end');
  Expect(';');
  ABI.Callbacks.Add(CB);
end;

procedure TABIParser.ParseEntry(Module: TABIModule);
var
  Entry: TABIEntry;
  T: string;
  FirstName, SecondName: string;
begin
  Entry := TABIEntry.Create;
  FirstName := ReadToken;
  T := PeekToken;
  if T = '=' then
  begin
    Expect('=');
    SecondName := ReadToken;
    Expect(';');
    SkipEmptyLines;
    if SameText(PeekToken, 'begin') then
    begin
      Entry.RealName := FirstName;
      Entry.LogicalName := SecondName;
      Expect('begin');
      while True do
      begin
        T := ReadToken;
        if T = 'end' then Break;
        if SameText(T, 'convention') then
        begin
          Expect('='); Entry.Convention := ReadString; Expect(';');
        end
        else if SameText(T, 'number') then
        begin
          Expect('='); Entry.Number := ReadNumber; Expect(';');
        end
        else if SameText(T, 'version') then
        begin
          Expect('='); Entry.VersionMin := ReadVersion;
          if PeekToken = '-' then begin Expect('-'); Entry.VersionMax := ReadVersion; end;
          Expect(';');
        end
        else if SameText(T, 'inputs') then
        begin
          Expect('='); Expect('(');
          Entry.Inputs := ReadToken;
          while PeekToken = ',' do
          begin
            Expect(','); Entry.Inputs := Entry.Inputs + ',' + ReadToken;
          end;
          Expect(')'); Expect(';');
        end
        else if SameText(T, 'outputs') then
        begin
          Expect('='); Expect('(');
          Entry.Outputs := ReadToken;
          while PeekToken = ',' do
          begin
            Expect(','); Entry.Outputs := Entry.Outputs + ',' + ReadToken;
          end;
          Expect(')'); Expect(';');
        end
        else
          Error('Unexpected attribute "' + T + '"');
        SkipEmptyLines;
      end;
      Expect(';');
    end
    else
    begin
      Entry.IsAlias := True;
      Entry.LogicalName := FirstName;
      Entry.AliasFor := SecondName;
    end;
  end
  else
  begin
    Entry.LogicalName := FirstName;
    Expect(';');
    SkipEmptyLines;
    Expect('begin');
    while True do
    begin
      T := ReadToken;
      if T = 'end' then Break;
      if SameText(T, 'convention') then
      begin
        Expect('='); Entry.Convention := ReadString; Expect(';');
      end
      else if SameText(T, 'number') then
      begin
        Expect('='); Entry.Number := ReadNumber; Expect(';');
      end
      else if SameText(T, 'version') then
      begin
        Expect('='); Entry.VersionMin := ReadVersion;
        if PeekToken = '-' then begin Expect('-'); Entry.VersionMax := ReadVersion; end;
        Expect(';');
      end
      else if SameText(T, 'inputs') then
      begin
        Expect('='); Expect('(');
        Entry.Inputs := ReadToken;
        while PeekToken = ',' do
        begin
          Expect(','); Entry.Inputs := Entry.Inputs + ',' + ReadToken;
        end;
        Expect(')'); Expect(';');
      end
      else if SameText(T, 'outputs') then
      begin
        Expect('='); Expect('(');
        Entry.Outputs := ReadToken;
        while PeekToken = ',' do
        begin
          Expect(','); Entry.Outputs := Entry.Outputs + ',' + ReadToken;
        end;
        Expect(')'); Expect(';');
      end
      else
        Error('Unexpected attribute "' + T + '"');
      SkipEmptyLines;
    end;
    Expect(';');
  end;
  Module.Entries.Add(Entry);
end;

procedure TABIParser.ParseModule(ABI: TABIData; ModuleType: TABIModuleType);
var
  Modu: TABIModule;
  T: string;
begin
  Modu := TABIModule.Create;
  Modu.ModuleType := ModuleType;
  Modu.Name := ReadToken;
  Expect(';');
  SkipEmptyLines;
  while True do
  begin
    T := PeekToken;
    if T = 'begin' then Break;
    if SameText(T, 'version') then
    begin
      Expect('version');
      Expect('=');
      Modu.VersionMin := ReadVersion;
      if PeekToken = '-' then
      begin
        Expect('-');
        Modu.VersionMax := ReadVersion;
      end;
      Expect(';');
      SkipEmptyLines;
    end
    else
      Error('Expected "begin" or "version", got "' + T + '"');
  end;
  Expect('begin');
  SkipEmptyLines;
  while True do
  begin
    T := ReadToken;
    if T = 'end' then Break;
    if T = 'entry' then
      ParseEntry(Modu)
    else
      Error('Expected "entry", got "' + T + '"');
    SkipEmptyLines;
  end;
  Expect(';');
  ABI.Modules.Add(Modu);
end;

procedure LoadABI(const AFilename: string; out ABI: TABIData);
var
  Parser: TABIParser;
  T: string;
begin
  ABI := TABIData.Create;
  Parser := TABIParser.Create;
  try
    AssignFile(Parser.F, AFilename);
    Reset(Parser.F);
    Parser.LineNum := 0;
    Parser.NextLine;
    while True do
    begin
      Parser.SkipEmptyLines;
      if Parser.Line = '' then Break;
      T := Parser.ReadToken;
      if T = '' then Continue;
      if SameText(T, 'structure') then
        Parser.ParseStructure(ABI)
      else if SameText(T, 'type') then
        Parser.ParseType(ABI)
      else if SameText(T, 'module') then
        Parser.ParseModule(ABI, mtDLL)
      else if SameText(T, 'library') then
        Parser.ParseModule(ABI, mtLibrary)
      else if SameText(T, 'syscall') then
      begin
        while not SameText(Parser.ReadToken, 'end') do ;
      end
      else
        Parser.Error('Unexpected token "' + T + '"');
    end;
  finally
    CloseFile(Parser.F);
    Parser.Free;
  end;
end;

end.
