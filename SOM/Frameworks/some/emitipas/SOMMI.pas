unit SOMMI;

interface

uses SOM,MIBase;

Type
  TSOMMI = class(TMIBaseClass)
  protected
    FEv: TEnvironment;
    FSOMObject: Pointer;
    destructor Destroy; override;
  public
    constructor Create;
  end;

  TSOMObject = TSOMMI;

implementation

constructor TSOMMI.Create;
begin
  inherited Create;
//  FEv := somGetGlobalEnvironment;
end;

destructor TSOMMI.Destroy;
begin
//  somDestruct;
  inherited Destroy;
end;


end.
