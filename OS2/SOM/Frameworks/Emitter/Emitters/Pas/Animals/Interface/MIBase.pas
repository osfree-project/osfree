unit MIBase;
{ $ MODE DELPHI}
{.$mode objfpc}

interface

uses sysutils;

Type
  EWrongClass = class(Exception);

  TMIBaseClass = class;

  PParentClassList = ^TParentClassList;
  TParentClassList = array [0..0] of TMIBaseClass;

  TMIBaseClassClass = class of TMIBaseClass;

  TMIBaseClass = class
  protected
    FOwner: TMIBaseClass;
    FParents: PParentClassList;
    FParentsCount: Longint;
    function QueryClassType(AClassType: TMIBaseClassClass): TMIBaseClass;
    destructor Destroy; virtual;
    procedure SetParentsCount(AParentsCount: Longint);
    procedure SetParent(AClass: TMIBaseClass);
  public
    constructor Create;
    procedure DestroyTarget; // уничтожения всего объекта-контейнера из любой "точки"
    function QueryAs(AClassType: TMIBaseClassClass): TMIBaseClass;
    function QyeryTarget: TMIBaseClass;
  end;


implementation

constructor TMIBaseClass.Create;
begin
  inherited Create;
end;

destructor TMIBaseClass.Destroy;
var i: Longint;
begin
  for i := 0 to FParentsCount-1 do
    FParents^[i].Destroy;
  FreeMem(FParents);
  inherited Destroy;
end;

function TMIBaseClass.QyeryTarget: TMIBaseClass;
begin
  if FOwner = nil then Result := Self
  else Result := FOwner.QyeryTarget
end;

procedure TMIBaseClass.SetParentsCount(AParentsCount: Longint);
begin
  if AParentsCount <= 0 then Exit;
  GetMem(FParents,SizeOf(PParentClassList)*FParentsCount);
  FParentsCount := 0;
end;

procedure TMIBaseClass.SetParent(AClass: TMIBaseClass);
begin
  if AClass = nil then Exit;
  FParents^[FParentsCount] := AClass;
  AClass.FOwner := Self;
  Inc(FParentsCount);
end;

function TMIBaseClass.QueryClassType(AClassType: TMIBaseClassClass): TMIBaseClass;
var
  i: Longint;
begin
  if Self is AClassType then Result := Self
  else begin
    Result := nil;
    for i := 0 to FParentsCount-1 do
    begin
      Result := FParents^[i].QueryClassType(AClassType);
      if Result<>nil then Break;
    end;
  end;
end;

function TMIBaseClass.QueryAs(AClassType: TMIBaseClassClass): TMIBaseClass;
begin
  if FOwner<>nil then Result := FOwner.QueryAs(AClassType)
  else begin
    Result := QueryClassType(AClassType);
    if Result = nil then raise EWrongClass.Create('Wrong class')
  end;
end;

procedure TMIBaseClass.DestroyTarget;
begin
  QyeryTarget.Destroy;
end;

end.