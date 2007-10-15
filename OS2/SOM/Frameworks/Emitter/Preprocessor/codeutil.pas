// **********************************************************************
//
// Copyright (c) 2001 - 2002 MT Tools.
//
// All Rights Reserved
//
// MT_DORB is based in part on the product DORB,
// written by Shadrin Victor
//
// See Readme.txt for contact information
//
// **********************************************************************
unit codeutil;

{$H+}

interface

uses
  classes,sysutils{,ir_int,orb_int,orbtypes};

type

  TPrintStream = class(TMemoryStream)
  public
    procedure println(str: string);
    procedure print(str: string);
  end;

  function safe_name(name: string): string;

  function coder_method(const method, atype: string): string;

//  function name_without_prefix(acont : IContained): boolean;

  function already_included(const file_name: string): boolean;

  function CreateStringGUID(const RepoId: string): string;

  function name_with_namespace(const AbsName, AName: string): string;

  function OpenFile(const AName: string): TFileStream;

const
  CR: byte = 13;

var
  ModuleWithoutPrefix: TStringList;
  StoreGUID: Boolean = False;
  NameSpaces: Boolean = False;
  StreamList: TStringList;

implementation

//uses IniFiles;//ir, IniFiles{$IFDEF MSWINDOWS}, Windows, ComObj{$ENDIF};

//var
//  IdlToPasIni: TIniFile = nil;

//*********************************************************************
//  TPrintStream
//*********************************************************************
procedure TPrintStream.println;
begin
  if str <> '' then
    write(PChar(str)^,length(str));
  write(CR,1);
end;

procedure TPrintStream.print(str: string);
begin
  write(PChar(str)^,length(str));
end;

function safe_name(name: string): string;
begin
  result := name;
  name := lowercase(name);
  if (name = 'and') or (name = 'array') or (name = 'as') or (name = 'asm') or (name = 'assign') or (name = 'automated') or (name = 'at') or
     (name = 'begin') or
     (name = 'case') or (name = 'class') or (name = 'const') or (name = 'constructor') or (name = 'create') or
     (name = 'destructor') or (name = 'dispinterface') or (name = 'div') or (name = 'do') or (name = 'downto') or (name = 'destroy')  or (name = 'date') or
     (name = 'else') or (name = 'end') or (name = 'except') or (name = 'exports') or
     (name = 'file') or (name = 'finalization') or (name = 'finally') or (name = 'for') or (name = 'function') or (name = 'free') or
     (name = 'goto') or
     (name = 'if') or (name = 'implementation') or (name = 'in') or (name = 'inherited') or (name = 'initialization') or (name = 'inline') or (name = 'interface') or (name = 'is') or
     (name = 'label') or (name = 'library') or
     (name = 'mod') or
     (name = 'nil') or (name = 'not') or
     (name = 'object') or (name = 'of') or (name = 'or') or (name = 'out') or (name = 'on') or
     (name = 'packed') or (name = 'procedure') or (name = 'program') or (name = 'property') or (name = 'private') or (name = 'protected') or (name = 'public') or (name = 'published') or
     (name = 'raise') or (name = 'record') or (name = 'repeat') or (name = 'resourcestring') or (name = 'result') or
     (name = 'set') or (name = 'shl') or (name = 'shr') or (name = 'string') or
     (name = 'then') or (name = 'threadvar') or (name = 'to') or (name = 'try') or (name = 'type') or
     (name = 'unit') or (name = 'until') or (name = 'uses') or
     (name = 'var') or
     (name = 'while') or (name = 'with') or
     (name = 'xor') then
    result := '_'+result;
end;

function coder_method(const method, atype: string): string;
begin
  result := atype;
  //remove _ prefix in some types
  if (atype = '_ushort') or (atype = '_ulong') or (atype = '_ulonglong') then
    Delete(result, 1, 1);
  result := method + result;
end;

function is_internal_module(const module_name: string): boolean;
begin
  result := (module_name = 'dynamic') or
            (module_name = 'corba') or
            (module_name = 'messaging') or
            (module_name = 'iop') or
            (module_name = 'policy') or
            (module_name = 'portableinterceptor');
end;

{function name_without_prefix(acont : IContained): boolean;
var
  cont: IContainer;
  con : IContained;
begin
  if acont = nil then begin
    result := false;
    Exit;
  end;
  cont := acont.defined_in;
  con := TContained._narrow(cont as IORBObject);
  if (acont.def_kind = dk_Interface) and (cont.def_kind = dk_Module) and
     (acont.name = 'Current') and (con.name = 'CORBA') then begin
    //prevent conflict with "Current" from CORBA module
    result := false;
    Exit;
  end;
  case cont.def_kind of
    dk_Module: begin
      result := is_internal_module(LowerCase(con.name)) or (ModuleWithoutPrefix.IndexOf(con.name) <> -1);
    end
    else begin
      result := name_without_prefix(con);
    end;
  end;
end;
}

function name_with_namespace(const AbsName, AName: string): string;
begin
  //Result := TTextRec(Output).Name + '.' + AName;
end;

function already_included(const file_name: string): boolean;
var
  fn: string;
begin
  fn := LowerCase(file_name);
  result := (fn = 'orb') or
            (fn = 'orbtypes') or
            (fn = 'dynamic') or
            (fn = 'codec') or
            (fn = 'iop') or
            (fn = 'messaging');
end;

function CreateStringGUID(const RepoId: string): string;

  function Generate: string;
  {$IFDEF LINUX}
  var
    newGUID: TGUID;
  {$ENDIF}
  begin
  {$IFDEF MSWINDOWS}
    Result := CreateClassID();
  {$ENDIF}
  {$IFDEF LINUX}
    CreateGUID(newGUID);
    Result := GUIDToString(newGUID);
  {$ENDIF}
  end;

begin
{
  if StoreGUID then begin
    if IdlToPasIni = nil then
      IdlToPasIni := TIniFile.Create(GetCurrentDir + '/idltopas.ini');
    Result := IdlToPasIni.ReadString(RepoId, 'GUID', '');
    if Result = '' then begin
      Result := Generate();
      IdlToPasIni.WriteString(RepoId, 'GUID', Result);
    end;
  end
  else
    Result := Generate();
}
end;

function OpenFile(const AName: string): TFileStream;
var
  name: string;
begin
  name:=trim(aname);
//  name := StringReplace(Trim(AName), '\', '/', [rfReplaceAll]);
  if StreamList.IndexOf(name) <> -1 then begin
    Result := TFileStream(StreamList.Objects[StreamList.IndexOf(name)]);
    Result.Position := 0;
  end
  else begin
    Result := TFileStream.Create(name, fmOpenRead);
    StreamList.AddObject(name, Result)
  end;
end;

initialization
  ModuleWithoutPrefix := TStringList.Create;
  StreamList := TStringList.Create;
finalization
//  ModuleWithoutPrefix.Free;
//  IdlToPasIni.Free;
  StreamList.Free;
end.
