{
    This file is part of the Free Component Library

    Pascal tree source file writer
    Copyright (c) 2003 by
      Areca Systems GmbH / Sebastian Guenther, sg@freepascal.org

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}


unit DefWrite;

interface

uses Classes, PasTree, ABI;

type
  TDefWriter = class
  private
    FStream: TStream;
    FAbiList: TPasSection;
  public
    constructor Create(AStream: TStream);
    destructor Destroy; override;
    procedure wrt(const s: string);
    procedure wrtln(const s: string);
    procedure wrtln;

    procedure WriteElement(AElement: TPasElement);
    procedure WriteModule(AModule: TPasModule);
    procedure WriteSection(ASection: TPasSection);
    procedure WriteProcDecl(AProc: TPasProcedure);
    function MultiAbiGet(Symbol: String): TAbi;
    property Stream: TStream read FStream;
  end;


procedure WriteDefFile(AElement: TPasElement; const AFilename: string);
procedure WriteDefFile(AElement: TPasElement; AStream: TStream);



implementation

uses SysUtils;

constructor TDefWriter.Create(AStream: TStream);
begin
  FStream := AStream;
end;

destructor TDefWriter.Destroy;
begin
  inherited Destroy;
end;

procedure TDefWriter.wrt(const s: string);
begin
  Stream.Write(s[1], Length(s));
end;

const
  LF: string = #10;

procedure TDefWriter.wrtln(const s: string);
begin
  wrt(s);
  Stream.Write(LF[1], 1);
end;

procedure TDefWriter.wrtln;
begin
  Stream.Write(LF[1], 1);
end;

procedure TDefWriter.WriteElement(AElement: TPasElement);
begin
  if AElement.ClassType = TPasModule then
    WriteModule(TPasModule(AElement))
  else if AElement.ClassType = TPasSection then
    WriteSection(TPasSection(AElement))
  else if AElement.InheritsFrom(TPasProcedure) then
    WriteProcDecl(TPasProcedure(AElement));
end;


procedure TDefWriter.WriteModule(AModule: TPasModule);
begin
  WriteSection(AModule.InterfaceSection);
end;

procedure TDefWriter.WriteSection(ASection: TPasSection);
var
  i: Integer;
begin

  FAbiList:=ASection;
  
  for i := 0 to ASection.Declarations.Count - 1 do
    WriteElement(TPasElement(ASection.Declarations[i]));

end;

function TDefWriter.MultiAbiGet(Symbol: String): TAbi;
var
  i: word;
begin
  for i:=0 to FAbiList.AbiList.Count-1 do
  begin
    Result:=AbiGet(TPasElement(FAbiList.AbiList[i]).Name+'.abi', Symbol);
    If Result.Name=Symbol then break;
  end;
end;

procedure TDefWriter.WriteProcDecl(AProc: TPasProcedure);
var
  ABI: TAbi;
begin
  ABI:=MultiAbiGet(AProc.Name);
  if ABI.ImportName='' then
    wrtln('++'+AProc.Name+'.'+ABI.DLL+'.'+ABI.Ordinal)
  else
    wrtln('++'+AProc.Name+'.'+ABI.DLL+'.'+ABI.ImportName+'.'+ABI.Ordinal);
end;

procedure WriteDefFile(AElement: TPasElement; const AFilename: string);
var
  Stream: TFileStream;
begin
  Stream := TFileStream.Create(AFilename, fmCreate);
  try
    WriteDefFile(AElement, Stream);
  finally
    Stream.Free;
  end;
end;

procedure WriteDefFile(AElement: TPasElement; AStream: TStream);
var
  Writer: TDefWriter;
begin
  Writer := TDefWriter.Create(AStream);
  try
    Writer.WriteElement(AElement);
  finally
    Writer.Free;
  end;
end;

end.
