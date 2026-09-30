{**
  @file ABIReader.pas
  @brief Parser for .abi files — the binary interface of the platform.

  The module reads a .abi file and builds a TABIData model containing:
  - modules (DLL) and static libraries with their exports;
  - system calls (syscall) with interrupt, service and call conventions;
  - structure attributes (pack, prefix, bitfield);
  - callback type attributes (convention).

  The .abi syntax is described in the uni2h v2.0 specification, section 3.
}
unit ABIReader;

interface

uses Classes, SysUtils;

{**
  @brief ABI module kind.
  @details mtDLL — dynamic library (module), mtLibrary — static library (library).
}
type
  TABIModuleType = (mtDLL, mtLibrary);

{**
  @brief Entry point in a module/library.
  @details Describes an export of a function or variable: ordinal, calling
  convention, OS version range, and (for syscall-like records) register lists.
}
type
  TABIEntry = class
    LogicalName: string;      {** @brief Logical name of the export }
    RealName: string;         {** @brief Real name for "entry REAL = LOGICAL" }
    Number: Integer;          {** @brief DLL ordinal }
    Convention: string;       {** @brief Calling convention }
    VersionMin: string;       {** @brief Minimum OS version (inclusive) }
    VersionMax: string;       {** @brief Maximum OS version (inclusive) }
    Inputs: string;           {** @brief Raw input register list }
    Outputs: string;          {** @brief Raw output register list }
    IsAlias: Boolean;         {** @brief Alias flag }
    AliasFor: string;         {** @brief Name the entry is an alias for }
  end;

{**
  @brief Kind of binding of a syscall register.

  Every register in a syscall is bound to exactly one of:
  - a fixed value (rkValue);
  - an input, output or inout parameter (rkParam);
  - the function result (rkResult).

  Free registers are not allowed.
}
type
  TABIRegisterKind = (rkValue, rkParam, rkResult);

{**
  @brief Register descriptor for a syscall directive.

  A register entry has the form:
  @code
  reg                  -- not allowed: free register
  reg=$XX              -- fixed value
  reg=N                -- fixed decimal value
  reg=param            -- bound to a parameter
  reg=Result           -- bound to the function result
  reg1:reg2=$XXXXXXXX  -- register pair with a fixed value
  reg1:reg2=param      -- register pair bound to a parameter
  reg1:reg2=Result     -- register pair bound to the result
  @endcode

  Regs holds the register name or pair ("ah", "ds:dx") as written.
  Kind defines the binding. Target holds the value, the parameter name,
  or is empty for a result binding.
}
type
  TABIRegister = class
    Regs: string;             {** @brief Register name or pair ("ah", "ds:dx") }
    Kind: TABIRegisterKind;   {** @brief Binding kind }
    Target: string;           {** @brief Value, parameter name, or '' }
  end;

{**
  @brief Bit-field descriptor for a structure.
}
type
  TABIBitField = class
    BitName: string;          {** @brief Bit-field name }
    BitPos: Integer;          {** @brief Bit position }
    Width: Integer;           {** @brief Width in bits (defaults to 1) }
  end;

{**
  @brief Structure attributes from .abi.
  @details Defines alignment, field-name prefix and bit-field layout.
}
type
  TABIStructure = class
    Name: string;             {** @brief Structure name }
    Pack: Integer;            {** @brief Alignment (#pragma pack) }
    Prefix: string;           {** @brief Field-name prefix }
    BitFields: TList;         {** @brief List of TABIBitField }
    constructor Create;
    destructor Destroy; override;
  end;

{**
  @brief Callback type attributes.
}
type
  TABICallback = class
    Name: string;             {** @brief Type name }
    Convention: string;       {** @brief Calling convention }
  end;

{**
  @brief System call.

  Describes a direct call not bound to a DLL/LIB. Distinguished by Convention:
  - interrupt — call via int N, where N = Number;
  - service   — call to an MVDM service via HLT; DB Number; DB NOT Number;
  - call      — far call to an absolute address, where Number = 'SSSS:OOOO'.

  Number is stored as a string exactly as written in the .abi file:
  '$21' for interrupt/service, '$1234:$5678' for call. The convention
  defines the interpretation; the emitter performs any formatting.

  Inputs and Outputs hold TABIRegister objects. Every register is bound
  either to a fixed value, a parameter or the result.

  If UsesCF is True, the CF flag is produced by the call and must be
  converted into an APIRET-style return value by the emitter. The
  emitter also fills out-value parameters from the result register
  when CF=0.
}
type
  TABISyscall = class
    Name: string;             {** @brief Syscall name }
    Number: string;           {** @brief Interrupt vector, service code or far address }
    Convention: string;       {** @brief 'interrupt', 'service' or 'call' }
    Inputs: TList;            {** @brief List of TABIRegister — inputs }
    Outputs: TList;           {** @brief List of TABIRegister — outputs }
    UsesCF: Boolean;          {** @brief True if CF is an error indicator }
    VersionMin: string;       {** @brief Minimum OS version }
    VersionMax: string;       {** @brief Maximum OS version }
    constructor Create;
    destructor Destroy; override;
  end;

{**
  @brief ABI module: a DLL or a static library.
}
type
  TABIModule = class
    Name: string;             {** @brief Module name }
    ModuleType: TABIModuleType; {** @brief Module kind }
    VersionMin: string;       {** @brief Minimum OS version }
    VersionMax: string;       {** @brief Maximum OS version }
    Entries: TList;           {** @brief List of TABIEntry }
    constructor Create;
    destructor Destroy; override;
  end;

{**
  @brief Root ABI model.
  @details Holds all modules, structures, callback types and syscalls
  parsed from a .abi file.
}
type
  TABIData = class
    Modules: TList;           {** @brief List of TABIModule }
    Structures: TList;        {** @brief List of TABIStructure }
    Callbacks: TList;         {** @brief List of TABICallback }
    Syscalls: TList;          {** @brief List of TABISyscall }
    constructor Create;
    destructor Destroy; override;

    {**
      @brief Find all entries with the given logical name.
      @param LogicalName Logical name of the export.
      @return List of TABIEntry (may be empty). Caller must free it.
    }
    function FindEntry(const LogicalName: string): TList;

    {**
      @brief Find structure attributes by name.
      @param Name Structure name.
      @return Pointer to TABIStructure or nil.
    }
    function FindStructure(const Name: string): TABIStructure;

    {**
      @brief Find a callback type by name.
      @param Name Type name.
      @return Pointer to TABICallback or nil.
    }
    function FindCallback(const Name: string): TABICallback;

    {**
      @brief Find a syscall by name.
      @param Name Syscall name.
      @return Pointer to TABISyscall or nil.
    }
    function FindSyscall(const Name: string): TABISyscall;
  end;

{**
  @brief Exception raised by the .abi parser.
}
type
  EABIError = class(Exception);

{**
  @brief Load a .abi file and build a TABIData model.
  @param AFilename Path to the .abi file.
  @param ABI Result — a newly created TABIData object.
}
procedure LoadABI(const AFilename: string; out ABI: TABIData);

implementation

{**
  @brief Internal .abi parser.
  @details Parses the file line by line, reads tokens and fills TABIData.
}
type
  TABIParser = class
  private
    F: TextFile;              {** @brief Input file }
    Line: string;             {** @brief Current line }
    LineNum: Integer;         {** @brief Current line number }
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
    procedure ParseSyscall(ABI: TABIData);
  end;

{ TABIStructure }

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

{ TABISyscall }

constructor TABISyscall.Create;
begin
  Inputs := TList.Create;
  Outputs := TList.Create;
  UsesCF := False;
end;

destructor TABISyscall.Destroy;
var
  i: Integer;
begin
  for i := 0 to Inputs.Count - 1 do
    TABIRegister(Inputs[i]).Free;
  Inputs.Free;
  for i := 0 to Outputs.Count - 1 do
    TABIRegister(Outputs[i]).Free;
  Outputs.Free;
  inherited;
end;

{ TABIModule }

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

{ TABIData }

constructor TABIData.Create;
begin
  Modules := TList.Create;
  Structures := TList.Create;
  Callbacks := TList.Create;
  Syscalls := TList.Create;
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
  for i := 0 to Syscalls.Count - 1 do
    TABISyscall(Syscalls[i]).Free;
  Syscalls.Free;
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

function TABIData.FindSyscall(const Name: string): TABISyscall;
var
  i: Integer;
begin
  for i := 0 to Syscalls.Count - 1 do
  begin
    Result := TABISyscall(Syscalls[i]);
    if CompareStr(Result.Name, Name) = 0 then
      Exit;
  end;
  Result := nil;
end;

{ TABIParser }

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
    { String literal: from the opening quote up to and including the closing one. }
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

{**
  @brief Parse a syscall directive.

  Expected syntax:
  @code
  syscall Name;
  begin
    [ number = N; ]                          -- N = scalar or far address
    convention = interrupt | service | call | 'string';
    [ version = 'X'[ - 'Y' ]; ]
    [ inputs  = ( reg[=val|param], ... ); ]
    [ outputs = ( reg[=val|param], cf, ... ); ]
  end;
  @endcode

  Clause order is free. Every register is bound either to a fixed value,
  a parameter or the result. Free registers are rejected. The token "cf"
  is accepted only in outputs and marks the call as returning an error
  indicator through the CF flag; it is stored in UsesCF.
}
procedure TABIParser.ParseSyscall(ABI: TABIData);
var
  Sys: TABISyscall;
  T: string;

  {**
    @brief Read a single register from an inputs/outputs list.

    The register token may contain a colon for a register pair ("ds:dx").
    If followed by '=', the right-hand token is classified:
    - a leading '$', digit or '-' means a fixed value (rkValue);
    - the name "Result" (case-insensitive) means the function result (rkResult);
    - anything else is a parameter name (rkParam).

    A register without '=' is rejected: free registers are not allowed.

    @return A new TABIRegister object; the caller adds it to a list.
  }
  function ReadRegister: TABIRegister;
  var
    RHS: string;
  begin
    Result := TABIRegister.Create;
    Result.Regs := ReadToken;
    Result.Kind := rkValue;
    Result.Target := '';

    if Result.Regs = '' then
      Error('Register name expected');

    if PeekToken = '=' then
    begin
      Expect('=');
      RHS := ReadToken;
      if RHS = '' then
        Error('Value or parameter name expected after "="');

      if (RHS[1] = '$') or (RHS[1] = '-') or
         ((RHS[1] >= '0') and (RHS[1] <= '9')) then
      begin
        Result.Kind := rkValue;
        Result.Target := RHS;
      end
      else if SameText(RHS, 'Result') then
      begin
        Result.Kind := rkResult;
        Result.Target := '';
      end
      else
      begin
        Result.Kind := rkParam;
        Result.Target := RHS;
      end;
    end
    else
      Error('Free register "' + Result.Regs +
            '" is not allowed; a register must be bound to a value, ' +
            'a parameter or the result');
  end;

begin
  Sys := TABISyscall.Create;
  try
    Sys.Name := ReadToken;
    Expect(';');
    SkipEmptyLines;
    Expect('begin');
    SkipEmptyLines;

    while True do
    begin
      T := ReadToken;
      if T = 'end' then Break;

      if SameText(T, 'number') then
      begin
        Expect('=');
        Sys.Number := ReadToken;
        Expect(';');
      end
      else if SameText(T, 'convention') then
      begin
        Expect('=');
        T := ReadToken;
        if (Length(T) >= 2) and (T[1] = '''') and (T[Length(T)] = '''') then
          Sys.Convention := Copy(T, 2, Length(T) - 2)
        else
          Sys.Convention := T;
        Expect(';');
      end
      else if SameText(T, 'version') then
      begin
        Expect('=');
        Sys.VersionMin := ReadVersion;
        if PeekToken = '-' then
        begin
          Expect('-');
          Sys.VersionMax := ReadVersion;
        end;
        Expect(';');
      end
      else if SameText(T, 'inputs') then
      begin
        Expect('=');
        Expect('(');
        while True do
        begin
          if SameText(PeekToken, 'cf') then
            Error('CF as an input flag is not supported');
          Sys.Inputs.Add(ReadRegister);
          if PeekToken = ',' then
            Expect(',')
          else
            Break;
        end;
        Expect(')');
        Expect(';');
      end
      else if SameText(T, 'outputs') then
      begin
        Expect('=');
        Expect('(');
        while True do
        begin
          if SameText(PeekToken, 'cf') then
          begin
            Expect('cf');
            Sys.UsesCF := True;
          end
          else
            Sys.Outputs.Add(ReadRegister);
          if PeekToken = ',' then
            Expect(',')
          else
            Break;
        end;
        Expect(')');
        Expect(';');
      end
      else
        Error('Unexpected syscall attribute "' + T + '"');

      SkipEmptyLines;
    end;

    Expect(';');
    ABI.Syscalls.Add(Sys);
  except
    Sys.Free;
    raise;
  end;
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
        Parser.ParseSyscall(ABI)
      else
        Parser.Error('Unexpected token "' + T + '"');
    end;
  finally
    CloseFile(Parser.F);
    Parser.Free;
  end;
end;

end.
