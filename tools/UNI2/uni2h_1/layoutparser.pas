unit LayoutParser;

interface

uses Classes, SysUtils;

type
  TLayoutEntry = class
    GroupName: string;
    OutputFile: string;
    HasMacro: Boolean;
    MacroName: string;
    InvertMacro: Boolean;
    HasGuard: Boolean;
    GuardName: string;
    HasIncluded: Boolean;
    IncludedMacro: string;
    CascadeMacro: string;   // одиночный макрос каскада (пусто = нет каскада)
    Description: string;
  end;

  TLayout = class
  private
    FEntries: TList;
  public
    constructor Create;
    destructor Destroy; override;
    procedure Add(Entry: TLayoutEntry);
    function FindByGroup(const AGroupName: string): TLayoutEntry;
    function FindByOutputFile(const AOutputFile: string): TLayoutEntry;
  end;

procedure LoadLayout(const AFilename: string; out Layout: TLayout);

implementation

constructor TLayout.Create;
begin
  FEntries := TList.Create;
end;

destructor TLayout.Destroy;
var
  i: Integer;
begin
  for i := 0 to FEntries.Count - 1 do
    TLayoutEntry(FEntries[i]).Free;
  FEntries.Free;
  inherited;
end;

procedure TLayout.Add(Entry: TLayoutEntry);
begin
  FEntries.Add(Entry);
end;

function TLayout.FindByGroup(const AGroupName: string): TLayoutEntry;
var
  i: Integer;
  Entry: TLayoutEntry;
begin
  for i := 0 to FEntries.Count - 1 do
  begin
    Entry := TLayoutEntry(FEntries[i]);
    if CompareStr(Entry.GroupName, AGroupName) = 0 then
    begin
      Result := Entry;
      Exit;
    end;
  end;
  Result := nil;
end;

function TLayout.FindByOutputFile(const AOutputFile: string): TLayoutEntry;
var
  i: Integer;
  Entry: TLayoutEntry;
begin
  for i := 0 to FEntries.Count - 1 do
  begin
    Entry := TLayoutEntry(FEntries[i]);
    if CompareStr(ExtractFileName(Entry.OutputFile), ExtractFileName(AOutputFile)) = 0 then
    begin
      Result := Entry;
      Exit;
    end;
  end;
  Result := nil;
end;

procedure LoadLayout(const AFilename: string; out Layout: TLayout);
var
  F: TextFile;
  Line, FullLine: string;
  i, p: Integer;
  Token: string;
  Entry: TLayoutEntry;
  InGroupDecl: Boolean;
  SemicolonFound: Boolean;
begin
  Layout := TLayout.Create;
  AssignFile(F, AFilename);
  Reset(F);
  try
    Entry := nil;
    InGroupDecl := False;

    while not EOF(F) do
    begin
      ReadLn(F, Line);
      FullLine := Trim(Line);
      if (FullLine = '') or (FullLine[1] = '#') or (Pos('//', FullLine) = 1) then
        Continue;

      if SameText(FullLine, 'layout;') then Continue;
      if SameText(FullLine, 'begin') then Continue;
      if SameText(FullLine, 'end.') or SameText(FullLine, 'end;') then Break;

      if Pos('group', FullLine) = 1 then
      begin
        if Entry <> nil then
        begin
          Layout.Add(Entry);
          Entry := nil;
        end;

        Entry := TLayoutEntry.Create;
        InGroupDecl := True;
        SemicolonFound := False;

        Delete(FullLine, 1, 5);
        FullLine := TrimLeft(FullLine);

        p := 1;
        while (p <= Length(FullLine)) and (FullLine[p] <> ':') and
              (FullLine[p] <> ';') and (FullLine[p] <> ' ') do
          Inc(p);
        Entry.GroupName := Copy(FullLine, 1, p - 1);
        Delete(FullLine, 1, p - 1);
        FullLine := TrimLeft(FullLine);

        if (Length(FullLine) > 0) and (FullLine[1] = ':') then
        begin
          Delete(FullLine, 1, 1);
          FullLine := TrimLeft(FullLine);
          p := 1;
          while (p <= Length(FullLine)) and (FullLine[p] <> ';') and
                (FullLine[p] <> ' ') do
            Inc(p);
          Entry.OutputFile := Copy(FullLine, 1, p - 1);
          Delete(FullLine, 1, p - 1);
          FullLine := TrimLeft(FullLine);
        end;

        if (Length(FullLine) > 0) and (FullLine[1] = ';') then
        begin
          Layout.Add(Entry);
          Entry := nil;
          InGroupDecl := False;
          Continue;
        end;

        Line := FullLine;
      end
      else
        Line := FullLine;

      if InGroupDecl and (Entry <> nil) and (Line <> '') then
      begin
        SemicolonFound := False;
        if Line[Length(Line)] = ';' then
        begin
          SemicolonFound := True;
          SetLength(Line, Length(Line) - 1);
        end;

        i := 1;
        while i <= Length(Line) do
        begin
          while (i <= Length(Line)) and (Line[i] = ' ') do Inc(i);
          if i > Length(Line) then Break;

          p := i;
          while (i <= Length(Line)) and (Line[i] <> '=') and (Line[i] <> ' ') do
            Inc(i);
          Token := Copy(Line, p, i - p);

          if (i <= Length(Line)) and (Line[i] = '=') then
          begin
            Inc(i);
            if Token = 'desc' then
            begin
              while (i <= Length(Line)) and (Line[i] = ' ') do Inc(i);
              if (i <= Length(Line)) and (Line[i] = '"') then
              begin
                Inc(i);
                p := i;
                while (i <= Length(Line)) and (Line[i] <> '"') do Inc(i);
                if i <= Length(Line) then
                begin
                  Entry.Description := Copy(Line, p, i - p);
                  Inc(i);
                end;
              end;
            end
            else
            begin
              p := i;
              while (i <= Length(Line)) and (Line[i] <> ' ') do Inc(i);
              if p < i then
              begin
                if Token = 'macro' then
                begin
                  Entry.HasMacro := True;
                  Entry.MacroName := Copy(Line, p, i - p);
                end
                else if Token = 'guard' then
                begin
                  Entry.HasGuard := True;
                  Entry.GuardName := Copy(Line, p, i - p);
                end
                else if Token = 'included' then
                begin
                  Entry.HasIncluded := True;
                  Entry.IncludedMacro := Copy(Line, p, i - p);
                end
                else if Token = 'cascade' then
                begin
                  Entry.CascadeMacro := Copy(Line, p, i - p);
                end;
              end;
            end;
          end
          else
          begin
            if Token = 'cascade' then
              Entry.CascadeMacro := ''   // автоимя
            else if Token = 'invert' then
              Entry.InvertMacro := True
            else if Token = 'macro' then
              Entry.HasMacro := True
            else if Token = 'guard' then
              Entry.HasGuard := True
            else if Token = 'included' then
              Entry.HasIncluded := True;
          end;
        end;

        if SemicolonFound then
        begin
          InGroupDecl := False;
          Layout.Add(Entry);
          Entry := nil;
        end;
      end;
    end;

    if Entry <> nil then
      Layout.Add(Entry);
  finally
    CloseFile(F);
  end;
end;

end.
