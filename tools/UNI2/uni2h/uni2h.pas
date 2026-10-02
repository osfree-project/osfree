program UNI2H;

uses
  ABIReader, CHWriter, MKWriter, pparser, pastree, sysutils, classes, getopts,
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

procedure PrintUsage;
begin
  WriteLn('Usage: uni2h --emitter=<lang> [options] [<file.uni> [<output>]]');
  WriteLn;
  WriteLn('Emitters:');
  WriteLn('  h         C header (requires --layout, <file.uni>)');
  WriteLn('  makefile  Import library makefile (requires --abi, -b/--library)');
  WriteLn;
  WriteLn('Common options:');
  WriteLn('  -e, --emitter=<lang>   Target emitter');
  WriteLn('  -a, --abi=<file>       ABI description');
  WriteLn;
  WriteLn('C emitter options:');
  WriteLn('  -l, --layout=<file>    Layout file');
  WriteLn('  -c, --h2inc-compat     Emit NOINC/INC markers');
  WriteLn('  -n, --no-common        Emit INCL_NOCOMMON checks');
  WriteLn('  -p, --cpp              Wrap with extern "C"');
  WriteLn('  -i, --ibm              Wrap with IBM C/C++ pragmas');
  WriteLn;
  WriteLn('Makefile emitter options:');
  WriteLn('  -b, --library=<name>   Library name from .abi');
end;

var
  AEngine: TUNIAPIEngine;
  AModule: TPasModule;
  c : char;
  optionindex : Longint;
  theopts : array[1..9] of TOption;
  emitter: string;
  unifile: string;
  outfile: string;
  abifile: string;
  layoutfile: string;
  libname: string;
  ABIData: TABIData;
  Layout: TLayout;
  H2IncCompat, NoCommon, CppWrapping, IBMWrapping: Boolean;
begin
  with theopts[1] do begin name:='emitter';      has_arg:=Required_argument; flag:=nil; value:='e'; end;
  with theopts[2] do begin name:='abi';          has_arg:=Required_argument; flag:=nil; value:='a'; end;
  with theopts[3] do begin name:='layout';       has_arg:=Required_argument; flag:=nil; value:='l'; end;
  with theopts[4] do begin name:='h2inc-compat'; has_arg:=No_argument;       flag:=nil; value:='c'; end;
  with theopts[5] do begin name:='no-common';    has_arg:=No_argument;       flag:=nil; value:='n'; end;
  with theopts[6] do begin name:='cpp';          has_arg:=No_argument;       flag:=nil; value:='p'; end;
  with theopts[7] do begin name:='ibm';          has_arg:=No_argument;       flag:=nil; value:='i'; end;
  with theopts[8] do begin name:='library';      has_arg:=Required_argument; flag:=nil; value:='b'; end;
  with theopts[9] do begin name:='';             has_arg:=No_argument;       flag:=nil; value:=#0; end;

  c:=#0;
  emitter := '';
  abifile := '';
  layoutfile := '';
  outfile := '';
  libname := '';
  H2IncCompat := False;
  NoCommon := False;
  CppWrapping := False;
  IBMWrapping := False;

  try
    repeat
      c:=getlongopts('e:a:l:cnpib:', @theopts[1], optionindex);
      case c of
        'e': emitter:=optarg;
        'a': abifile:=optarg;
        'l': layoutfile:=optarg;
        'c': H2IncCompat := True;
        'n': NoCommon := True;
        'p': CppWrapping := True;
        'i': IBMWrapping := True;
        'b': libname:=optarg;
      end;
    until c=endofoptions;

    if emitter = '' then
    begin
      PrintUsage;
      Halt(1);
    end;

    if emitter = 'makefile' then
    begin
      if abifile = '' then
      begin
        WriteLn('ABI file required');
        Halt(1);
      end;
      if libname = '' then
      begin
        WriteLn('Library name required (-b/--library)');
        Halt(1);
      end;
      LoadABI(abifile, ABIData);
      WriteMakefile(ABIData, libname);
    end
    else if emitter = 'h' then
    begin
      if layoutfile = '' then
      begin
        WriteLn('Layout file required for H emitter');
        Halt(1);
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

          if outfile <> '' then
            WriteSingleCHeader(AModule, ABIData, Layout, outfile,
                               H2IncCompat, NoCommon, CppWrapping, IBMWrapping)
          else
          begin
            outfile := ChangeFileExt(AModule.Name, '.h');
            WriteCHeaders(AModule, ABIData, Layout, outfile,
                          H2IncCompat, NoCommon, CppWrapping, IBMWrapping);
          end;
        finally
          AEngine.Free;
        end;
      end else
        WriteLn('error');
    end
    else
      WriteLn('Emitter not implemented: ', emitter);
  except
    on E: Exception do
    begin
      WriteLn('Error: ', E.Message);
      Halt(1);
    end;
  end;
end.
