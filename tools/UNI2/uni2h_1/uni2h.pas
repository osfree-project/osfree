program UNI2H;

uses
  ABIReader, CHWriter, pparser, pastree, sysutils, classes, getopts,
  LayoutParser, PScanner;

type
  TUNIAPIEngine = class(TPasTreeContainer)
  private
    CurModule: TPasModule;
  public
    function CreateElement(AClass: TPTreeElement; const AName: String;
      AParent: TPasElement; AVisibility :TPasMemberVisibility;
      const ASourceFilename: String; ASourceLinenumber: Integer): TPasElement; override;
    function FindElement(const AName: String): TPasElement; override;
  end;

function TUNIAPIEngine.CreateElement(AClass: TPTreeElement; const AName: String;
  AParent: TPasElement; AVisibility : TPasMemberVisibility;
  const ASourceFilename: String; ASourceLinenumber: Integer): TPasElement;
begin
  Result := AClass.Create(AName, AParent);
  if AClass.InheritsFrom(TPasModule) then
    CurModule := TPasModule(Result);
end;

function TUNIAPIEngine.FindElement(const AName: String): TPasElement;
var
  l: TList;
  i: Integer;
begin
  l := CurModule.InterfaceSection.Declarations;
  for i := 0 to l.Count - 1 do
  begin
    Result := TPasElement(l[i]);
    if CompareStr(Result.Name, AName) = 0 then
      exit;
  end;
  Result := nil;
end;

var
  AEngine: TUNIAPIEngine;
  AModule: TPasModule;
  c : char;
  optionindex : Longint;
  theopts : array[1..7] of TOption;
  emitter: string;
  unifile: string;
  outfile: string;
  abifile: string;
  layoutfile: string;
  ABIData: TABIData;
  Layout: TLayout;
  H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean;
begin
  with theopts[1] do begin name:='emitter'; has_arg:=Required_argument; flag:=nil; value:='e'; end;
  with theopts[2] do begin name:='abi';     has_arg:=Required_argument; flag:=nil; value:='a'; end;
  with theopts[3] do begin name:='layout';  has_arg:=Required_argument; flag:=nil; value:='l'; end;
  with theopts[4] do begin name:='h2inc-compat'; has_arg:=No_argument; flag:=nil; value:='c'; end;
  with theopts[5] do begin name:='no-common';    has_arg:=No_argument; flag:=nil; value:='n'; end;
  with theopts[6] do begin name:='cpp';          has_arg:=No_argument; flag:=nil; value:='p'; end;
  with theopts[7] do begin name:='ibm';          has_arg:=No_argument; flag:=nil; value:='i'; end;

  c:=#0;
  emitter := '';
  abifile := '';
  layoutfile := '';
  outfile := '';
  H2IncCompat := False;
  NoCommon := False;
  CppWrapping := False;
  IBMWrapping := False;

  try
    repeat
      c:=getlongopts('e:a:l:cnpi', @theopts[1], optionindex);
      case c of
        'e': emitter:=optarg;
        'a': abifile:=optarg;
        'l': layoutfile:=optarg;
        'c': H2IncCompat := True;
        'n': NoCommon := True;
        'p': CppWrapping := True;
        'i': IBMWrapping := True;
      end;
    until c=endofoptions;

    // Проверка обязательных параметров
    if emitter = 'h' then
    begin
      if layoutfile = '' then
      begin
        WriteLn('Layout file required for H emitter');
        Halt(1);
      end;
    end;

    if (ParamCount - OptInd) >= 1 then
    begin
      unifile := ParamStr(OptInd);
      if OptInd + 1 <= ParamCount then
        outfile := ExpandFileName(ParamStr(OptInd + 1))
      else
        outfile := '';

      if abifile <> '' then
        LoadABI(abifile, ABIData)
      else begin
        WriteLn('ABI file required');
        Halt(1);
      end;

      Layout := nil;
      if layoutfile <> '' then
        LoadLayout(layoutfile, Layout);

      AEngine:=TUNIAPIEngine.Create;
      try
        AModule := ParseSource(AEngine, unifile, '', '');

        if emitter = 'h' then
        begin
          if outfile <> '' then
          begin
            WriteSingleCHeader(AModule, ABIData, Layout, outfile,
                               H2IncCompat, NoCommon, CppWrapping, IBMWrapping);
          end
          else
          begin
            outfile := ChangeFileExt(AModule.Name, '.h');
            WriteCHeaders(AModule, ABIData, Layout, outfile,
                          H2IncCompat, NoCommon, CppWrapping, IBMWrapping);
          end;
        end else
          WriteLn('Emitter not implemented: ', emitter);
      finally
        AEngine.Free;
      end;
    end else
      WriteLn('error');
  except
    on E: Exception do
    begin
      WriteLn('Error: ', E.Message);
      Halt(1);
    end;
  end;
end.
