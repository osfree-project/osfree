Program UNI2H;

uses
  abi, defwrite, hwrite, pparser, pastree, sysutils, classes, getopts;

Type
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

  function FindInModule(AModule: TPasModule; const LocalName: String): TPasElement;
  var
    l: TList;
    i: Integer;
  begin
    l := AModule.InterfaceSection.Declarations;
    for i := 0 to l.Count - 1 do
    begin
      Result := TPasElement(l[i]);
      if CompareText(Result.Name, LocalName) = 0 then
        exit;
    end;
    Result := nil;
 end;

var
  i: Integer;
  //ModuleName, LocalName: String;
  Module: TPasElement;
begin
{!!!: Don't know if we ever will have to use the following:
  i := Pos('.', AName);
  if i <> 0 then
  begin
    WriteLn('Dot found in name: ', AName);
    Result := nil;
  end else
  begin}
    Result := FindInModule(CurModule, AName);
    if not Assigned(Result) then
      for i := CurModule.InterfaceSection.UsesList.Count - 1 downto 0 do
      begin
        Module := TPasElement(CurModule.InterfaceSection.UsesList[i]);
        if Module.ClassType = TPasModule then
        begin
          Result := FindInModule(TPasModule(Module), AName);
          if Assigned(Result) then
            exit;
        end;
      end;
  {end;}
end;

var
  AEngine: TUNIAPIEngine;
  AModule: TPasModule;
  c : char;
  optionindex : Longint;
  theopts : array[1..2] of TOption;
  emitter: string;
  unifile: string;
  outfile: string;
begin
  with theopts[1] do
  begin
    name:='emitter';
    has_arg:=Required_argument;
    flag:=nil;
    value:='e';
  end;
  with theopts[2] do
  begin
    name:='abi';
    has_arg:=Required_argument;
    flag:=nil;
    value:='a';
  end;


  c:=#0;
  repeat
    c:=getlongopts('e:a:',@theopts[1],optionindex);
    case c of
      'e': emitter:=optarg;
      'a': abifile:=optarg;
//      '?',':' : usage();
    end; { case }
  until c=endofoptions;

  writeln(abifile);
  if (ParamCount-OptInd)=1 then
  begin
    unifile:=paramstr(optind);
    inc(optind);
    outfile:=paramstr(optind);
    AEngine:=TUNIAPIEngine.Create;
    AModule:=ParseSource(AEngine, unifile,'','');
    if emitter='def' then WriteDefFile(AModule, outfile) 
    else if emitter='h' then WriteHFile(AModule, outfile) 
    else WriteHFile(AModule, outfile);
  end else Writeln('error');
end.
