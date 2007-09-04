Program UNI2H;

uses
  hwrite,pparser, pastree, sysutils;

Type
  TUNIAPIEngine = class(TPasTreeContainer)
  public
    function CreateElement(AClass: TPTreeElement; const AName: String;
      AParent: TPasElement; AVisibility :TPasMemberVisibility;
      const ASourceFilename: String; ASourceLinenumber: Integer): TPasElement; override;
  end;


function TUNIAPIEngine.CreateElement(AClass: TPTreeElement; const AName: String;
  AParent: TPasElement; AVisibility : TPasMemberVisibility;
  const ASourceFilename: String; ASourceLinenumber: Integer): TPasElement;


begin
  Result := AClass.Create(AName, AParent);
{$ifdef 0}
  if Result.ClassType = TPasModule then
  begin
  end else
  if Result.ClassType=TPasUnresolvedTypeRef then
  begin
    WriteLn('#include <',Result.Name,'.h>');
  end else
  if Result.ClassType=TPasConst then
  begin
    WriteLn('#define ',Result.Name,' ',TPasConst(Result).Value);
  end else
  if Result.ClassType=TPasProcedureType then
  begin
    WriteLn(TPasProcedureType(Result).TypeName,' ',Result.Name);
  end else
  if Result.ClassType=TPasTypeRef{TPasAliasType} then
  begin
    WriteLn('typedef ',Result.Name{, TPasAliasType(Result).DestType.Name});
  end;
{$endif}
end;

var
  AEngine: TUNIAPIEngine;
  AModule: TPasModule;
begin
 AEngine:=TUNIAPIEngine.Create;
 AModule:=ParseSource(AEngine, paramstr(1),'','');
 WriteHFile(AModule, paramstr(2));
end.
