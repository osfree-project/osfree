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
unit codeunit;

interface

uses
  codebase,classes,sysutils,ir_int,code_int,orbtypes,
  codeutil,{$IFDEF MSWINDOWS}Windows,ComObj,{$ENDIF}ir,idldb;

type

  TCodegen = class(TCodebase)
  private
    FConstants : IInterfaceList;
    procedure InternalGenerate(const AName: string; const AContents: IInterfaceList);
    procedure GenerateInterface(const AName: string; const AContents: IInterfaceList);
    procedure GenerateImplementation(const AName: string; const AContents: IInterfaceList);
    procedure ProcessRmic(const RepoId: string);
  protected
    // interface unit
    procedure int_declare_alias(cont : IAliasDef);
    procedure int_declare_array(arr: IArrayDef);
    procedure int_declare_sequence(seq: ISequenceDef);
    procedure int_declare_enum(enum : IEnumDef);
    procedure int_declare_interface(int: IInterfaceDef);
    procedure int_declare_struct(str: IStructDef);
    procedure int_declare_value(val: IValueDef);
    procedure int_declare_value_box(val: IValueBoxDef);
    procedure int_declare_module(m: IModuleDef);
    procedure int_declare_union(u: IUnionDef);
    procedure int_declare_exception(e : IExceptionDef);
    procedure int_declare_internal_type(cont_name,name: string; t: IIDLType);

    // code unit declarations
    procedure code_declare_alias(cont : IAliasDef);
    procedure code_declare_enum(enum : IEnumDef);
    procedure code_declare_interface(int: IInterfaceDef);
    procedure code_declare_struct(str: IStructDef);
    procedure code_declare_value(val: IValueDef);
    procedure code_declare_value_box(val: IValueBoxDef);
    procedure code_declare_module(m: IModuleDef);
    procedure code_declare_union(u: IUnionDef);
    procedure code_declare_array(a: IArrayDef);
    procedure code_declare_sequence(s: ISequenceDef);
    procedure code_declare_exception(e: IExceptionDef);


    // code unit implementations
    procedure code_impl_alias(cont : IAliasDef);
    procedure code_impl_enum(enum : IEnumDef);
    procedure code_impl_struct(str: IStructDef);
    procedure code_impl_interface(int: IInterfaceDef);
    procedure code_impl_value(val: IValueDef);
    procedure code_impl_value_box(val: IValueBoxDef);
    procedure code_impl_module(m: IModuleDef);
    procedure code_impl_union(u: IUnionDef);
    procedure code_impl_exception(e: IExceptionDef);

    // dependencies
    procedure dependencies(contents: IInterfaceList);
    procedure int_dependencies(int: IInterfaceDef);
    procedure union_dependencies(u: IUnionDef);
    procedure alias_dependencies(val: IAliasDef);
    procedure array_dependencies(a: IArrayDef);
    procedure seq_dependencies(s: ISequenceDef);
    procedure op_dependencies(oper: IOperationdef);
    procedure attr_dependencies(attr: IAttributeDef);
    procedure val_dependencies(val: IValueDef);
    procedure valbox_dependencies(val: IValueBoxDef);
    procedure cont_dependencies(val: IIRObject);
    procedure struct_dependencies(val: IStructDef);

    procedure collect_constant(con: IConstantDef);
    procedure declare_constant(con: IConstantDef);
    procedure declare_forwards();
    procedure ex_dependencies(ex: IExceptionDef);

    // impl unit declarations
    procedure final_declare_interface(int: IInterfaceDef);
    procedure final_declare_value(val: IValueDef);

    // impl unit implementations
    procedure final_impl_interface(int: IInterfaceDef);
    procedure final_impl_value(val: IValueDef);

    procedure final_attribute(name: string; attr: IAttributeDef);
    procedure final_method(name: string; def: IOperationDef);

    procedure impl_code(cont: IIRObject);
    procedure impl_declare(cont: IIRObject);
    procedure int_declare(cont: IIRObject);
    procedure write_header();
    function get_default_value_for_union(def: IIDLType;members: UnionMemberSeq): Variant;

    procedure final_code(cont: IContained);
    procedure final_declare(cont: IContained);

    function get_name(cont : IContained) : string;  overload;
    function get_name(int: IInterfaceDef) : string;  overload;
  public
    constructor Create(rep: IRepository; params: TStrings;db: TDB);
    destructor Destroy(); override;
    procedure generate;
  end;

implementation

uses
  orb_int, version_int;

//*********************************************************************
// TCodegen
//*********************************************************************
constructor TCodegen.Create(rep: IRepository; params: TStrings; db: TDB);
begin
  FRepository := rep;
  FParams := params;
  FDB := db;
  FDeclared := TStringList.Create;
  FForwards := TInterfaceList.Create;
  Marshallers := TStringList.Create;
  FImplemented := TStringList.Create;
  FConstants := TInterfaceList.Create;
  FIncludes := TStringList.Create;
end;

destructor TCodegen.Destroy();
begin
  FDeclared.free;
  FImplemented.free;
  Marshallers.free;
  FIncludes.free;
end;

function TCodeGen.get_name(cont : IContained) : string;
begin
   if (FParams.IndexOf('-without-prefix') >= 0) or name_without_prefix(cont) then
      Result := cont.name
   else
      Result := cont._absolute_name;
end;

function TCodeGen.get_name(int: IInterfaceDef) : string;
begin
   if (FParams.IndexOf('-without-prefix') >= 0) or name_without_prefix(int) then
      Result := int.name
   else
      Result := int._absolute_name;
end;

procedure TCodegen.generate;
var
  name,usesstr: string;
  i,cnt : integer;
  contents: IInterfaceList;

  function _ExtractFileName(str: string): string;
  var
    I: Integer;
  begin
    I := LastDelimiter('\:/', str);
    Result := Copy(str, I + 1, MaxInt);
  end;

  function HaveUndefined(const ACont: IContainer): Boolean;
  var
    module, other: IInterfaceList;
    i, j: Integer;
  begin
    Result := False;
    module := ACont.contents(dk_Module);
    for i := 0 to module.Count - 1 do begin
      Result := HaveUndefined(module[i] as IContainer);
      if Result then
        Exit;
      {other := (module[i] as IContainer).contents();
      for j := 0 to other.Count - 1 do
        if not Fdb.repoid_defined((other[j] as IContained).id) then begin
          Result := True;
          Break;
        end;}
    end;
      other := ACont.contents();
      for j := 0 to other.Count - 1 do
        if not Fdb.repoid_defined((other[j] as IContained).id) then begin
          Result := True;
          Break;
        end;
  end;

  procedure GenerateModule(const ACont: IContainer; const AName: string);
  var
    module, other: IInterfaceList;
    i: Integer;
    name: string;
  begin
    module := ACont.contents(dk_Module);
    for i := 0 to module.Count - 1 do begin
      if {not Fdb.repoid_defined((module[i] as IContained).id)} HaveUndefined(module[i] as IContainer) then begin
        name := AName + '_' + (module[i] as IContained).name;
        GenerateModule(module[i] as IContainer, name);
      end;
    end;
    FIncludes.Clear;
    dependencies(FRepository.contents());
    if FIncludes.IndexOf(AName) <> -1 then
      FIncludes.Delete(FIncludes.IndexOf(AName));
    other := ACont.contents();
    for i := other.Count - 1 downto 0 do
      if module.IndexOf(other[i]) <> -1 then
        other.Remove(other[i]);
    if other.Count <> 0 then
      InternalGenerate(AName, other);
  end;

begin
  try
    name := _ExtractFileName(FParams[0]);
    if Pos('.',name) > 0 then
      name := Copy(name,1,Pos('.',name)-1);

    if FParams.IndexOf('-ns') >= 0 then begin
      GenerateModule(FRepository, name);
    end
    else begin
      FIncludes.Clear;
      Marshallers.Clear;
      FConstants.Clear;
      dependencies(FRepository.contents());
      contents := FRepository.contents();
      InternalGenerate(name, contents);
    end;

    if FParams.IndexOf('-impl') >= 0 then
      begin
        assign(output,name+'_impl.pas');
        Rewrite(output);
        write_header();
        //writeln('unit '+name+'_impl;'+char(CR));
        writeln('unit '+name+'_impl;'); writeln;
        //writeln('interface'+ char(CR));
        writeln('interface'); writeln;
        writeln('uses');
        writeln('  orb_int, orb, req_int, code_int, imr, imr_int, env_int, except_int, orbtypes,');
        usesstr :='  exceptions, stdstat, std_seq, Classes, '+name+'_int, '+name;
        if FIncludes.count > 0 then
          begin
            cnt := FIncludes.count-1;
            for i := 0 to cnt do
              begin
                if (length(usesstr) > 80) and (i <> cnt) then
                  begin
                    writeln(usesstr+',');
                    usesstr := '  '+FIncludes[i]+'_int';
                  end
                else
                  usesstr := usesstr + ', ' + FIncludes[i]+'_int';
                if i = cnt then usesstr := usesstr+';';
              end
          end
        else
          usesstr := usesstr + ';';
        writeln(usesstr);
        writeln;
        //writeln('type'+ char(CR));
        writeln('type'); writeln;

        for i := 0 to Pred(contents.count) do
          final_declare(IContained(contents[i]));

        writeln('implementation');
        writeln;

        for i := 0 to Pred(contents.count) do
          final_code(IContained(contents[i]));

        writeln('end.');
        Close(output);
      end;
    Assign(output,'');
  except
    on e : Exception do
      begin
        Close(output);
        Assign(output,'');
        write('error: '+e.Message);
      end;
  end;
end;

{** calls interface declaration method for each
    definition kind }
procedure TCodegen.int_declare(cont: IIRObject);
begin
  case cont.def_kind of
    dk_Interface : int_declare_interface(cont as IInterfaceDef);
    dk_Struct: int_declare_struct(cont as IStructDef);
    dk_Alias : int_declare_alias(cont as IAliasDef);
    dk_Union : int_declare_union(cont as IUnionDef);
    dk_Enum : int_declare_enum(cont as IEnumDef);
    dk_Value : int_declare_value(cont as IValueDef);
    dk_ValueBox : int_declare_value_box(cont as IValueBoxDef);
    dk_Module : int_declare_module(cont as IModuleDef);
    dk_Exception : int_declare_exception(cont as IExceptionDef);
  end;
end;

{** calls class declaration method for each
    definition kind }
procedure TCodegen.impl_declare(cont: IIRObject);
//var
  //t : IIDLType;
  //c: IContained;
begin
  case cont.def_kind of
    dk_Interface : code_declare_interface(cont as IInterfaceDef);
    dk_Struct: code_declare_struct(cont as IStructDef);
    dk_Enum: code_declare_enum(cont as IEnumDef);
    dk_Alias : code_declare_alias((cont as IAliasDef));
    dk_Value : code_declare_value(cont as IValueDef);
    dk_ValueBox : code_declare_value_box(cont as IValueBoxDef);
    dk_Module : code_declare_module(cont as IModuleDef);
    dk_Union : code_declare_union(cont as IUnionDef);
    dk_Exception : code_declare_exception(cont as IExceptionDef);
    dk_Array : code_declare_array(cont as IArrayDef);
    dk_Sequence : code_declare_sequence(cont as ISequenceDef);
  end;
end;

{** calls class implementation method for each
    definition kind }
procedure TCodegen.impl_code(cont: IIRObject);
begin
  case cont.def_kind of
    dk_Interface : code_impl_interface(cont as IInterfaceDef);
    dk_Struct : code_impl_struct(cont as IStructDef);
    dk_Enum : code_impl_enum(cont as IEnumDef);
    dk_Alias: code_impl_alias(cont as IAliasDef);
    dk_Value : code_impl_value(cont as IValueDef);
    dk_ValueBox : code_impl_value_box(cont as IValueBoxDef);
    dk_Module : code_impl_module(cont as IModuleDef);
    dk_Union : code_impl_union(cont as IUnionDef);
    dk_Exception : code_impl_exception(cont as IExceptionDef);
  end;
end;

{** prints compiler information on top of each generated unit }
procedure TCodegen.write_header();
begin
  //writeln('// Generated by DORB idltopas compiler, '+FormatDateTime('dd/mm/yyyy',now));
  writeln('// ***************************************************************************');
  writeln('//                                                                            ');
  writeln('// Generated by the MTDORB IDL-to-Pascal Translator ('+ ORBVersion +'), '+FormatDateTime('dd/mm/yyyy hh:nn',now));
  writeln('//                                                                            ');
  writeln('// Copyright (c) 2002 - 2004                                                  ');
  writeln('// Millennium Group.                                                          ');
  writeln('// Samara, Russia                                                             ');
  writeln('//                                                                            ');
  writeln('// All Rights Reserved                                                        ');
  writeln('//                                                                            ');
  writeln('// ***************************************************************************');
end;

{** checks dependencies for operation definition }
procedure TCodegen.op_dependencies(oper: IOperationdef);
var
  params : ParDescriptionSeq;
  i : integer;
  paramdesc : ParameterDescription;
  cont : IContained;
begin
  if oper._result.kind in [tk_objref,tk_struct,tk_union] then
    begin
      cont := TContained._narrow(oper.get_result_def as IORBObject);
      if (cont <> nil) and (FDeclared.indexOf(get_name(cont)) < 0) then
        add_forwards(oper.get_result_def as IContained);
    end;
  params := oper.get_params;
  for i := 0 to Pred(Length(params)) do
    begin
      paramdesc := params[i];
      if paramdesc._type.kind in [tk_objref,{tk_struct,}tk_union] then
        begin
          cont := TContained._narrow(paramdesc.type_def as IORBObject);
          if cont = nil then continue;
          if FDeclared.indexOf(get_name(cont)) < 0 then
            add_forwards(cont);
        end;
    end;
end;

{** checks dependencies for value definition }
procedure TCodegen.val_dependencies(val: IValueDef);
begin
  if FIncludes.IndexOf('value') < 0 then
    FIncludes.add('value');
end;

{** checks dependencies for valuebox definition }
procedure TCodegen.valbox_dependencies(val: IValueBoxDef);
//var
  //orig_type: IIDLType;
begin
  if FIncludes.IndexOf('value') < 0 then
    FIncludes.add('value');
  {orig_type := val.original_type_def;
  if orig_type.def_kind = dk_Sequence then begin
  end;}
end;

procedure TCodegen.ex_dependencies(ex: IExceptionDef);
var
  i : integer;
  members : StructMemberSeq;
  cont,defin : IContained;
begin
  members := nil;
  if not declare(get_name(ex as IContained)) then exit;
  if FIncludes.IndexOf('except') < 0 then
    FIncludes.add('except');
  members := ex.members^;
  defin := TContained._narrow(ex.defined_in as IORBObject);
  for i := 0 to Pred(Length(members)) do
    begin
      if members[i].type_def.def_kind in [dk_Interface,dk_Value] then
        begin
          cont := TContained._narrow(members[i].type_def as IORBObject);
          if cont = nil then continue;
          if defin = cont then
            begin
              if FDB.repoid_defined(cont.id) then exit;
              if FForwards.IndexOf(cont) < 0 then FForwards.add(cont);
            end
          else if (FDeclared.indexOf(get_name(cont)) < 0) then
            add_forwards(cont);
        end;
      cont_dependencies(members[i].type_def);
    end;
end;

{** checks dependencies for interface definition }
procedure TCodegen.int_dependencies(int: IInterfaceDef);
var
  i : integer;
  list : IInterfaceList;
begin
  if not declare(get_name(int as IContained)) then exit;
  // check abstraction
  if int.get_is_abstract() then
    if FIncludes.IndexOf('value') < 0 then
      FIncludes.add('value');
  // check exceptions
  list := (int as IContainer).contents(dk_Exception);
  for i := 0 to pred(list.count) do
    ex_dependencies(list[i] as IExceptionDef);
  // check constants
  list := (int as IContainer).contents(dk_Constant);
  for i := 0 to pred(list.count) do
    collect_constant(list[i] as IConstantDef);
  // check forward declarations
  list := int.get_base_interfaces;
  for i := 0 to pred(list.count) do
    int_dependencies(list[i] as IInterfaceDef);
  list := (int as IContainer).contents(dk_Operation);
  for i := 0 to pred(list.count) do
    op_dependencies(list[i] as IOperationdef);
  list := (int as IContainer).contents(dk_Attribute);
  for i := 0 to pred(list.count) do
    attr_dependencies(list[i] as IAttributeDef);
end;

procedure TCodegen.alias_dependencies(val: IAliasDef);
begin
  with val.original_type_def do
    case def_kind of
      dk_Interface : add_forwards(val.original_type_def as IContained);
      dk_Struct : add_forwards(val.original_type_def as IContained);
    end;
  cont_dependencies(val.original_type_def);
end;

procedure TCodegen.cont_dependencies(val: IIRObject);
begin
  case val.def_kind of
    dk_Interface : int_dependencies(val as IInterfaceDef);
    dk_Value : val_dependencies(val as IValueDef);
    dk_ValueBox : valbox_dependencies(val as IValueBoxDef);
    dk_Module : dependencies((val as IContainer).contents);
    dk_Constant : collect_constant(val as IConstantDef);
    dk_Union : union_dependencies(val as IUnionDef);
    dk_Array : array_dependencies(val as IArrayDef);
    dk_Sequence : seq_dependencies(val as ISequenceDef);
    dk_Alias : alias_dependencies(val as IAliasDef);
    dk_Struct : struct_dependencies(val as IStructDef);
    dk_Exception : ex_dependencies(val as IExceptionDef);
  end;
end;

{** calls dependencies ckeck method for each definition kind }
procedure TCodegen.dependencies(contents: IInterfaceList);
var
  i,j : integer;
  id,fname : string;

  function _extractfilename(name: string): string;
  var
    idx, idx1: Integer;
    str: string;
  begin
    result := extractfilename(name);
    if FParams.IndexOf('-ns') >= 0 then begin
      idx := pos('/',result);
      idx1 := 0;
      while idx > 0 do begin
        idx1 := idx;
        result[idx] := '_';
        idx := pos('/',result);
      end;
      if idx1 <> 0 then begin
        idx := Pos('.', result);
        if idx > 1 then
          result := Copy(result, 1, idx - 1);
        str := Copy(Result, idx1 + 1, Length(Result) - idx1);
        Result := str + '_' + Copy(Result, 1, idx1 - 1);
      end;
    end
    else
      while pos('/',result) > 0 do
        result := copy(result,pos('/',result)+1,1000)
  end;
begin
  for i := 0 to pred(contents.count) do
    begin
      id := (contents[i] as IContained).id;
      if Fdb.repoid_defined(id) then
        begin
          fname := _extractfilename(Fdb.repoid_file_name(id));
          j := pos('.',fname);
          if j > 1 then
            fname := copy(fname,1,j-1);
          if FIncludes.IndexOf(fname) < 0 then
            FIncludes.add(fname);
        end;
      cont_dependencies(contents[i] as IContained);
    end;
end;

procedure TCodegen.code_declare_alias(cont: IAliasDef);
var
  t : IIDLType;
begin
  if FDB.repoid_defined((cont as IContained).id) then exit;
  //declare_marshaller(cont);
  t :=  original_type(cont as IIDLType);
  case t.def_kind of
    dk_Primitive:;
    dk_Struct:;
    dk_Sequence : if not is_base_sequence(t as ISequenceDef) then
       code_declare_sequence(t as ISequenceDef);
    dk_Array : code_declare_array(t as IArrayDef);
    dk_Enum : code_declare_enum(t as IEnumDef);
    dk_Union : code_declare_union(t as IUnionDef);
    dk_Interface : code_declare_interface(t as IInterfaceDef);
    else
      assert(false,'not implemented');
  end;
end;

procedure TCodegen.code_declare_enum(enum: IEnumDef);
begin
  if FDB.repoid_defined((enum as IContained).id) then exit;
  declare_marshaller(enum);
end;

procedure TCodegen.code_declare_module(m: IModuleDef);
var
  i : integer;
  contents : IInterfaceList;
begin
  //if FDB.repoid_defined(m.id) then exit;
  contents := (m as IContainer).contents();
  for i := 0 to Pred(contents.count) do
    impl_declare(contents[i] as IIRObject);
end;

procedure TCodegen.code_declare_interface(int: IInterfaceDef);
var
  name,iname,cname,basenames,bname,boname: string;
  base: IInterfaceList;
  i : integer;
  stub_name,skel_name , mdirective: string;
  contents : IInterfaceList;
  abstract_base_class : Boolean;
begin
  if FDB.repoid_defined(int.id) then exit;
  contents := (int as IContainer).contents();
  // internal declarations
  for i := 0 to Pred(contents.count) do
    impl_declare(contents[i] as IIRObject);
  name := get_name(int);
  iname := 'I'+name;
  cname := 'T'+name;
  stub_name := cname+ '_stub';
  base := int.get_base_interfaces;
  basenames := '';
  if int.get_is_abstract then
    boname := 'TAbstractBase'
  else if int.get_is_local then
    boname := 'TLocalORBObject'
  else
    boname := 'TORBObject';
  abstract_base_class := false;
  for i := 0 to Pred(base.count) do
    begin
      bname := get_name(base[i] as IContained);
      if i = 0 then
        begin
          if (base[i] as IInterfaceDef).get_is_abstract then
            begin
              abstract_base_class := true;
              boname := 'TORBObject,I'+bname+',IAbstractBase';
            end
          else
            boname := 'T'+bname;
        end;
      if i <> 0 then
        baseNames := baseNames + ',I'+bname;
    end;
  writeln('  {** '+int.id+' } ');
  writeln('  '+ cname + ' = class('+boname+','+iname+basenames+')');
  writeln('  protected');
  if abstract_base_class then
    begin
      writeln('    function _to_object: IORBObject;');
      writeln('    function _to_value: IValueBase;');
    end;
  //declare_all_methods(int,' virtual; abstract;',not abstract_base_class);
  declare_all_methods2(int,' virtual; abstract;',not abstract_base_class);
  writeln('    function narrow_helper(const str: string): Pointer; override;');
  writeln('  public');
  if int.get_is_abstract then
    begin
      writeln('    class function _downcast(const vb : IValueBase): '+iname+'; ');
      writeln('    class function _narrow(const obj : IORBObject): '+iname+'; overload;');
      writeln('    class function _narrow(const obj : IAbstractBase): '+iname+'; overload;');
    end
  else
    writeln('    class function _narrow(const obj : IORBObject): '+iname+';');
  writeln('    class function narrow_helper2(const obj: IORBObject): Boolean;');
  writeln('  end;');
  writeln;

  if int.get_is_local then
    Exit;

  //-----------  stub --------------------
  writeln('  {** stub for interface '+int.id+'} ');
  mdirective := ' override;';
  if base.count = 0 then
    writeln('  '+ stub_name + ' = class('+cname+')')
  else
    begin
      //mdirective := '';
      bname := get_name(base[0] as IContained);
      write('  '+ stub_name + ' = class('+cname);
      for i := 1 to Pred(base.count) do
        begin
          bname := get_name(base[i] as IContained);
          System.write(',I'+bname);
        end;
      writeln(')');
    end;
  writeln('  protected');
  //declare_all_methods(int,mdirective,false);
  declare_all_methods2(int,mdirective,false);
  writeln('  end;');
  writeln;
  if FParams.IndexOf('-poa') < 0 then
    // BOA
    begin
      writeln('  {** skeleton for interface '+int.id+'} ');
      skel_name := cname+ '_skel';
      //-----------  skeleton --------------------
      if base.count = 0 then
        writeln('  '+ skel_name + ' = class(TStaticMethodDispatcher,IStaticInterfaceDispatcher,'+iname+')')
      else
        begin
          bname := get_name(base[0] as IContained);
          write('  '+ skel_name + ' = class(T'+bname+'_skel,'+iname);
          for i := 1 to Pred(base.count) do
            begin
              bname := get_name(base[i] as IContained);
              write(',I'+bname);
            end;
          writeln(')');
        end;
      writeln('  protected');
      //declare_all_methods(int,' virtual; abstract;',true);
      declare_all_methods2(int,' virtual; abstract;',true);
      if base.count = 0 then
        writeln('    function _dispatch(const req: IStaticServerRequest; const env: IEnvironment): Boolean; virtual;')
      else
        writeln('    function _dispatch(const req: IStaticServerRequest; const env: IEnvironment): Boolean; override;');
      writeln('    function narrow_helper(const str: string): Pointer; override;');
      writeln('  public');
      writeln('    constructor Create();');
      writeln('  end;');
      writeln;
    end
  else
    // POA
    begin
      //-----------  POA stub --------------------
      writeln('  {** POA stub for interface '+int.id+'} ');
      if base.count = 0 then
        writeln('  '+ cname + '_stub_clp = class(TPOAStub,IPOAStub,'+iname+')')
      else
        begin
          bname := get_name(base[0] as IContained);
          write('  '+ cname + '_stub_clp = class(T'+bname+'_stub_clp,'+iname);
          for i := 1 to Pred(base.count) do
            begin
              bname := get_name(base[i] as IContained);
              write(',I'+bname);
            end;
          writeln(')');
        end;
      writeln('  protected');
      //declare_all_methods(int,' virtual;',True);
      declare_all_methods2(int,' virtual;',True);
      writeln('    function narrow_helper(const str: string): Pointer; override;');
      writeln('  end;');
      writeln;
      //-----------  servant --------------------
      writeln('  {** servant for interface '+int.id+'} ');
      if base.count = 0 then
        writeln('  '+ cname + '_serv = class(TPOAStaticImplementation,IPOAStaticImplementation,IServant,'+iname+')')
      else
        begin
          bname := get_name(base[0] as IContained);
          write('  '+ cname + '_serv = class(T'+bname+'_serv,'+iname);
          for i := 1 to Pred(base.count) do
            begin
              bname := get_name(base[i] as IContained);
              write(',I'+bname);
            end;
          writeln(')');
        end;
      if FParams.IndexOf('-notabs') >= 0 then begin
        writeln('  private');
        writeln('    FImplementation: ' + iname + ';');
      end;
      writeln('  protected');
      if FParams.IndexOf('-notabs') >= 0 then
        //declare_all_methods(int,' virtual;',true)
        declare_all_methods2(int,' virtual;',true)
      else
        //declare_all_methods(int,' virtual; abstract;',true);
        declare_all_methods2(int,' virtual; abstract;',true);

      common_servant_methods_declaration;
      writeln('  public');
      
      if FParams.IndexOf('-notabs') >= 0 then
        writeln('    constructor Create(const AImpl: ' + iname + ' = nil);');
        
      writeln('    function _this(): '+iname+';');
      writeln('    class function _narrow(const srv : IServant): '+iname+';');
      writeln('  end;');
      writeln;
    end;
  marshaller_decl(name,int)
end;

procedure TCodegen.code_declare_exception(e: IExceptionDef);
var
  name,typestr: string;
  member: StructMember;
  members: StructMemberSeq;
  i,mlen : integer;
  defIn: IContainer;
begin
  members := nil;
  if FDB.repoid_defined(e.id) then exit;
  name := get_name(e as IContained);
  //check local
  defIn := IContained(e).defined_in;
  if not ((defIn.def_kind = dk_Interface) and (defIn as IInterfaceDef).get_is_local) then
    marshaller_decl(name,e);
  members := e.members^;
  mlen := Length(members);
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : code_declare_sequence(member.type_def as ISequenceDef);
        dk_Array : code_declare_array(member.type_def as IArrayDef);
      end;
    end;
  writeln('  {** '+(e as IContained).id+' } ');
  writeln('  T'+name+' = class(UserException,I'+name+')');
  if mlen > 0 then  writeln('  private');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : typestr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : typestr := 'T'+name + '_'+member.name + '_array';
        else
          typestr := type_str(member.type_def)
      end;
      writeln('    F'+member.name+' : '+ typestr+';');
    end;
  writeln('  protected');
  writeln('    procedure throw; override;');
  writeln('    function clone(): IORBException; override;');
  writeln('    function repoid(): PChar; override;');
  writeln('    procedure encode(const enc: IEncoder); override;');
  writeln('    procedure encode_any(const a: IAny); override;');
  ex_members_decl(name,members,ed_in);
  writeln('  public');
  ex_members_decl(name,members,ed_out);
  writeln('    constructor Create(ex: T'+name+'); overload;');
  write('    constructor Create(');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : typestr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : continue;
        else
          typestr := type_str(member.type_def)
      end;
      if def_value(member.type_def) = '' then
        write('_'+member.name+': '+ typestr)
      else
        write('_'+member.name+': '+ typestr+' = '+def_value(member.type_def));
      if i < mlen-1 then write(';');
    end;
  writeln('); overload;');
  writeln('  end;');
  writeln;
end;

procedure TCodegen.code_declare_sequence(s: ISequenceDef);
var
  name, tstr: string;
  t : IIDLType;
begin
  if FDB.repoid_defined(FDB.pseudo_repoid(s)) then  exit;
  if is_base_sequence(s) then exit;
  name := expand_name(s);
  t := original_type(s.element_type_def);
  if t.def_kind = dk_array then
    code_declare_array(t as IArrayDef);

  tstr := expand_name(s.element_type_def);
  if s.bound = 0 then
    writeln('  '+name+' = array of '+ tstr+';')
  else
    writeln('  '+name+' = array [0..'+IntToStr(s.bound-1)+'] of '+ tstr+';');
  writeln('  P'+name+' = ^'+ name+';');
  writeln;

  marshaller_decl(name,s);
end;

procedure TCodegen.code_declare_array(a: IArrayDef);
var
  name: string;
  t : IIDLType;
begin
  if FDB.repoid_defined(FDB.pseudo_repoid(a)) then  exit;
  name := expand_name(a);
  t := original_type(a.element_type_def);
  if t.def_kind = dk_array then
    code_declare_array(t as IArrayDef);
  marshaller_decl(name,a);
end;

procedure TCodegen.code_declare_union(u: IUnionDef);
var
  name,typestr,last_name: string;
  member: UnionMember;
  members: UnionMemberSeq;
  i : integer;
begin
  members := nil;
  if FDB.repoid_defined((u as IContained).id) then exit;
  name := get_name(u as IContained);
  members := u.members^;
  for i := 0 to Pred(Length(members)) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : code_declare_sequence(member.type_def as ISequenceDef);
        dk_Array : code_declare_array(member.type_def as IArrayDef);
        dk_Struct : code_declare_struct(member.type_def as IStructDef);
      end;
    end;
  writeln('  {** '+(u as IContained).id+' } ');
  writeln('  T'+name+' = class(TInterfacedObject,I'+name+')');
  writeln('  private');
  last_name := '';
  for i := 0 to Pred(Length(members)) do
    begin
      member := members[i];
      if member.name = last_name then continue;
      case member.type_def.def_kind of
        dk_Sequence : typestr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : typestr := 'T'+name + '_'+member.name + '_array';
        else
          typestr := type_str(member.type_def)
      end;
      writeln('    F'+member.name+': '+typestr+';');
      last_name := member.name;
    end;
  writeln('    F'+'discriminator: '+type_str(u.discriminator_type_def)+';');
  writeln('  protected');
  last_name := '';
  for i := 0 to Pred(Length(members)) do
    begin
      member := members[i];
      if member.name = last_name then continue;
      case member.type_def.def_kind of
        dk_Sequence : typestr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : typestr := 'T'+name + '_'+member.name + '_array';
        else
          typestr := type_str(member.type_def)
      end;
      writeln('    procedure set_'+member.name+'(const val: '+typestr+');');
      writeln('    function get_'+member.name+': '+typestr+';');
      last_name := member.name;
    end;
    writeln('    function discriminator: '+type_str(u.discriminator_type_def)+';');
  writeln('  end;');
  writeln;
  declare_marshaller(u);
end;

procedure TCodegen.code_declare_struct(str: IStructDef);
var
  members : StructMemberSeq;
  i : integer;
  contents : IInterfaceList;
begin
  members := nil;
  if FDB.repoid_defined((str as IContained).id) then exit;
  contents := (str as IContainer).contents();
  // internal declarations
  for i := 0 to Pred(contents.count) do
    impl_declare(contents[i] as IIRObject);
  members := str.members^;
  for i := 0 to Pred(Length(members)) do
    case members[i].type_def.def_kind of
      dk_Sequence : if not is_base_sequence(members[i].type_def as ISequenceDef) then
        code_declare_sequence(members[i].type_def as ISequenceDef);
      dk_Array : code_declare_array(members[i].type_def as IArrayDef);
    end;
  declare_marshaller(str);
end;

procedure TCodegen.code_declare_value_box(val: IValueBoxDef);
var
  name, impl_name, int_name, type_name : string;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  name := get_name(val as IContained);
  impl_name := 'T'+name;
  int_name := 'I'+name;
  writeln('  {** '+(val as IContained).id+' } ');
  writeln('  '+impl_name+' = class(TValueBase,'+int_name+',IValueBase)');
  writeln('  protected');
  if val.original_type_def.def_kind = dk_Sequence then begin
    if is_base_sequence(val.original_type_def as ISequenceDef) then
      type_name := expand_name(val.original_type_def)
    else
      type_name := 'T' + name + '_Value'
  end
  else
    type_name := type_str(val.original_type_def);
  writeln('    FValue : '+type_name+';');
  writeln('    function _copy_value : IValueBase; override;');
  writeln('    function _value : '+type_name+'; overload;');
  writeln('    procedure _value(val : '+type_name+'); overload;');
  writeln('    procedure _marshal_members(const enc: IEncoder); override;');
  writeln('    procedure _get_marshal_info(const str: TStrings;out chunked: Boolean); override;');
  writeln('    function  _demarshal_members(const dec: Idecoder): Boolean; override;');
  writeln('    function narrow_helper(const repoid: string): Pointer; override;');
  writeln('  public');
  writeln('    constructor Create(val:'+ type_name+'); overload;');
  writeln('    constructor Create(val:'+ int_name+'); overload;');
  writeln('    class function _downcast(const vb : IValueBase): '+int_name+';');
  writeln('  end;');
  writeln;
  declare_marshaller(val as IIDLType);
  if (val.original_type_def.def_kind = dk_Sequence) and not is_base_sequence(val.original_type_def as ISequenceDef) then begin
    writeln('  ' + expand_name(val.original_type_def) + ' = ' + type_str(val.original_type_def) + ';');
    writeln;
    declare_marshaller(val.original_type_def as IIDLType);
  end;
end;

procedure TCodegen.code_declare_value(val: IValueDef);
var
  name, impl_name, int_name : string;
  mlist, list, supported : IInterfaceList;
  i : integer;
  vmd: IValueMemberDef;
  bval: IValueDef;
  int : IInterfaceDef;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  name := get_name(val as IContained);
  impl_name := 'T'+name;
  int_name := 'I'+name;
  writeln('  {** '+(val as IContained).id+' } ');
  bval := val.get_base_value;
  supported := val.get_supported_interfaces;
  write('  '+impl_name+' = class(');
  if bval = nil then
    write('TValueBase,')
  else
    write('T'+get_name(bval as IContained)+',');
  if supported.count > 0 then
    begin
      write('I'+(supported[0] as IContained).name+',');
      with supported[0] as IInterfaceDef do
        if get_is_abstract then
          write('IAbstractBase,');
    end;
  writeln(int_name+',IValueBase)');
  mlist := (val as IContainer).contents(dk_ValueMember);
  writeln('  protected');
  for i := 0 to Pred(mlist.count) do
    begin
      vmd := mlist[i] as IValueMemberDef;
      writeln('    F'+name+'_'+vmd.name+' : '+type_str(vmd.get_type_def)+';');
    end;
  for i := 0 to Pred(mlist.count) do
    value_member(mlist[i] as IValueMemberDef, True);
  if supported.count > 0 then
    with supported[0] as IInterfaceDef do
      if get_is_abstract then
        begin
          writeln('    function _to_object: IORBObject;');
          writeln('    function _to_value: IValueBase;');
        end;
  list := (val as IContainer).contents(dk_Operation);
  for i := 0 to Pred(list.count) do
    writeln('    '+operation_str(list[i] as IOperationDef)+' virtual; abstract;');
  // supported interfaces
  if supported.Count > 0 then
    begin
      int := supported[0] as IInterfaceDef;
      declare_methods(int,' virtual;');
      declare_attributes(int,' virtual;');
    end;
  writeln('    function _copy_value : IValueBase; override;');
  writeln('    procedure _copy_members(const v: TObject); override;');
  writeln('    procedure _marshal_members(const enc: IEncoder); override;');
  writeln('    procedure _get_marshal_info(const str: TStrings;out chunked: Boolean); override;');
  writeln('    function  _demarshal_members(const dec: Idecoder): Boolean; override;');
  writeln('    function narrow_helper(const repoid: string): Pointer; override;');
  writeln('  public');
  writeln('    class function _downcast(const vb : IValueBase): '+int_name+';');
  writeln('  end;');
  writeln;
  // Init
  if Length(val.initializers) > 0 then
    begin
      writeln('  {** factory for valuetype '+(val as IContained).id+'} ');
      writeln('  '+impl_name+'_init = class(TInterfacedObject,'+int_name+'_init,IValueFactory)');
      writeln('  protected');
      for i := 0 to Pred(Length(val.initializers)) do
        writeln('    '+initializer_decl(val.initializers[i],val)+' virtual; abstract;');
      writeln('    function narrow_helper(const repoid: string): Pointer;');
      writeln('    function create_for_umarshal(): IValueBase; virtual; abstract;');
      writeln('  public');
      writeln('    class function _downcast(const vf : IValueFactory): '+int_name+'_init;');
      writeln('  end;');
      writeln;
    end;
  declare_marshaller(val);
end;

procedure TCodegen.code_impl_alias(cont: IAliasDef);
begin
  if FDB.repoid_defined((cont as IContained).id) then exit;
  alias_marshaller(cont);
end;

procedure TCodegen.code_impl_enum(enum: IEnumDef);
begin
  if FDB.repoid_defined((enum as IContained).id) then exit;
  enum_marshaller(enum as IContained);
end;

procedure TCodegen.final_impl_value(val: IValueDef);
var
  name,ostr : string;
  op: IOperationDef;
  list, supported, methods, attributes : IInterfaceList;
  i,ind : integer;
  vmd: IValueMemberDef;
  int: IInterfaceDef;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  name := get_name(val as IContained);
  writeln('//***********************************************************');
  writeln('// T'+name+'_impl');
  writeln('//***********************************************************');
  list := (val as IContainer).contents(dk_ValueMember);
  // constructor
  write('constructor T'+name+'_impl.Create(');
  for i := 0 to Pred(list.count) do
    begin
      if i > 0 then write('; ');
      vmd := list[i] as IValueMemberDef;
      write('_'+vmd.name+': '+type_str(vmd.get_type_def));
    end;
  writeln(');');
  writeln('begin');
  writeln('  // TODO');
  writeln('end;');
  writeln;
  // methods
  list := (val as IContainer).contents(dk_Operation);
  for i := 0 to Pred(list.count) do
    begin
      op := list[i] as IOperationDef;
      ostr := operation_str(op);
      ind := Pos(' ',ostr);
      writeln(Copy(ostr,1,ind)+'T'+name+'_impl.'+copy(ostr,ind+1,10000));
      writeln('begin');
      writeln('  // TODO');
      if op._result.kind <> tk_void then
        writeln('  result := '+def_value(op.get_result_def));
      writeln('end;');
      writeln;
    end;
  // supported interfaces
  supported := val.get_supported_interfaces;
  if supported.Count > 0 then
    begin
      int := supported[0] as IInterfaceDef;
      methods := (int as IContainer).contents(dk_Operation);
      attributes := (int as IContainer).contents(dk_Attribute);
      for i := 0 to Pred(methods.Count) do
        final_method(name,methods[i] as IOperationDef);
      for i := 0 to Pred(attributes.Count) do
        final_attribute(name,attributes[i] as IAttributeDef);
    end;
  // Factory
  if Length(val.initializers) > 0 then
    begin
      writeln('//***********************************************************');
      writeln('// T'+name+'_factory');
      writeln('//***********************************************************');
      writeln('function T'+name+'_factory.create_for_umarshal(): IValueBase;');
      writeln('begin');
      write('  result := T'+name+'_impl.Create(');
      list := (val as IContainer).contents(dk_ValueMember);
      for i := 0 to Pred(list.count) do
        begin
          if i > 0 then write(', ');
          vmd := list[i] as IValueMemberDef;
          write(def_value(vmd.get_type_def));
        end;
      writeln(');');
      writeln('end;');
      writeln;
      
      for i := 0 to Pred(Length(val.initializers)) do
        begin
          ostr := initializer_decl(val.initializers[i],val);
          ind := Pos(' ',ostr);
          writeln(Copy(ostr,1,ind)+'T'+name+'_factory.'+copy(ostr,ind+1,10000));
          writeln('begin');
          writeln('  // TODO');
          writeln('  result := nil;');
          writeln('end;');
          writeln;
        end;
    end;
end;

procedure TCodegen.final_declare_value(val: IValueDef);
var
  name: string;
  list : IInterfaceList;
  i : integer;
  vmd: IValueMemberDef;
  supported: IInterfaceList;
  int: IInterfaceDef;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  name := get_name(val as IContained);
  writeln('  T'+name+'_impl = class(T'+name+')');
  writeln('  protected');
  list := (val as IContainer).contents(dk_Operation);
  for i := 0 to Pred(list.count) do
    writeln('    '+operation_str(list[i] as IOperationDef)+' override;');
  supported := val.get_supported_interfaces;
  if supported.Count > 0 then
    begin
      int := supported[0] as IInterfaceDef;
      declare_methods(int,' override;');
      declare_attributes(int,' override;');
    end;
  writeln('  public');
  list := (val as IContainer).contents(dk_ValueMember);
  write('    constructor Create(');
  for i := 0 to Pred(list.count) do
    begin
      if i > 0 then write('; ');
      vmd := list[i] as IValueMemberDef;
      write('_'+vmd.name+': '+type_str(vmd.get_type_def));
    end;
  writeln(');');
  writeln('  end;');
  writeln;
  // Factory
  if Length(val.initializers) > 0 then
    begin
      writeln('  T'+name+'_factory = class(T'+name+'_init)');
      writeln('  protected');
      for i := 0 to Pred(Length(val.initializers)) do
        writeln('    '+initializer_decl(val.initializers[i],val)+' override;');
      writeln('    function create_for_umarshal(): IValueBase; override;');
      writeln('  end;');
      writeln;
    end;
end;

procedure TCodegen.code_impl_exception(e: IExceptionDef);
var
  members: StructMemberSeq;
  member : StructMember;
  i, mlen : integer;
  tstr,name,id: string;
  defIn: IContainer;
  local: boolean;
begin
  members := nil;
  if FDB.repoid_defined(e.id) then exit;
  name := get_name(e as IContained);
  id := e.id;
  members := e.members^;
  mlen := Length(members);
  for i := 0 to Pred(mlen) do
    case members[i].type_def.def_kind of
      dk_Sequence : if not is_base_sequence(members[i].type_def as ISequenceDef) then
        seq_marshaller(members[i].type_def as ISequenceDef);
      dk_Array : array_marshaller(members[i].type_def as IArrayDef);
    end;
  //check local
  defIn := IContained(e).defined_in;
  local := (defIn.def_kind = dk_Interface) and (defIn as IInterfaceDef).get_is_local;
  if not local then
    except_marshaller(e);
  writeln('//***********************************************************');
  writeln('// T'+name);
  writeln('//***********************************************************');
  writeln('constructor T'+name+'.Create(ex: T'+name+');');
  writeln('begin');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : tstr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : tstr := 'T'+name + '_'+member.name + '_array';
        else
          tstr := type_str(member.type_def)
      end;
      writeln('  F'+member.name+':= ex.'+member.name+';');
    end;
  writeln('  inherited Create();');
  writeln('end;');
  writeln;

  writeln('procedure T'+name+'.throw;');
  writeln('begin');
  writeln('  raise T'+name+'.Create(self);');
  writeln('end;');
  writeln;

  writeln('function T'+name+'.clone(): IORBException;');
  writeln('begin');
  writeln('  result := T'+name+'.Create(self);');
  writeln('end;');
  writeln;

  writeln('procedure T'+name+'.encode(const enc: IEncoder);');
  if not local then begin
    writeln('var');
    writeln('  int : I'+name+';');
    writeln('begin');
    writeln('  int := self;');
    writeln('  '+name+'_marshaller.marshal(enc,@int);');
    writeln('end;');
  end
  else begin
    writeln('begin');
    writeln('  raise MARSHAL.Create;');
    writeln('end;');
  end;
  writeln;

  writeln('procedure T'+name+'.encode_any(const a: IAny);');
  if not local then begin
    writeln('var');
    writeln('  stat: IStaticAny;');
    writeln('begin');
    writeln('  stat := StaticAny('+name+'_marshaller, Self);');
    writeln('  a.from_static_any(stat);');
    writeln('end;');
  end
  else begin
    writeln('begin');
    writeln('  raise MARSHAL.Create;');
    writeln('end;');
  end;
  writeln;

  write('constructor T'+name+'.Create(');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : tstr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : continue;
        else
          tstr := type_str(member.type_def)
      end;
      write('_'+member.name+': '+ tstr);
      if i < mlen-1 then write(';');
    end;
  writeln(');');
  writeln('begin');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      if member.type_def.def_kind <> dk_Array then
        writeln('  F'+member.name+' := _'+member.name+';');
    end;
  writeln('  inherited Create();');
  writeln('end;');
  writeln;
  writeln('function T'+name+'.repoid(): PChar;');
  writeln('begin');
  writeln('  result := '''+id+''';');
  writeln('end;');
  writeln;
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : tstr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : tstr := 'T'+name + '_'+member.name + '_array';
        else
          tstr := type_str(member.type_def)
      end;
      writeln('procedure T'+name+'.'+member.name+'(const val: '+tstr+');');
      writeln('begin');
      writeln('  F'+member.name+' := val;');
      writeln('end;');
      writeln;
      writeln('function T'+name+'.'+member.name+': '+tstr+';');
      writeln('begin');
      writeln('  result := F'+member.name+';');
      writeln('end;');
      writeln;
    end;
end;

procedure TCodegen.code_impl_union(u: IUnionDef);
var
  i,ind : integer;
  members: UnionMemberSeq;
  member: UnionMember;
  typestr,name, val, last_name: string;
  discr: IIDLType;
  enum : IEnumDef;
  emembers : EnumMemberSeq;
begin
  members := nil;
  emembers := nil;
  if FDB.repoid_defined((u as IContained).id) then exit;
  name := get_name(u as IContained);
  members := u.members^;
  for i := 0 to Pred(Length(members)) do
    case members[i].type_def.def_kind of
      dk_Sequence : if not is_base_sequence(members[i].type_def as ISequenceDef) then
        seq_marshaller(members[i].type_def as ISequenceDef);
      dk_Array : array_marshaller(members[i].type_def as IArrayDef);
      dk_Struct : struct_marshaller(members[i].type_def as IContained);
    end;
  union_marshaller(u);
  writeln('//***********************************************************');
  writeln('// T'+name);
  writeln('//***********************************************************');
  discr := original_type(u.discriminator_type_def);
  if discr.def_kind = dk_Enum then
    begin
      enum := TEnumDef._narrow(discr as IORBObject);
      assert(enum <> nil);
      emembers := enum.members^;
    end
  else
    emembers := nil;
  last_name := '';
  for i := 0 to Pred(Length(members)) do
    begin
      member := members[i];
      if member.name = last_name then continue;
      case member.type_def.def_kind of
        dk_Sequence : typestr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : typestr := 'T'+name + '_'+member.name + '_array';
        else
          typestr := type_str(member.type_def)
      end;
      writeln('procedure T'+name+'.set_'+member.name+'(const val: '+typestr+');');
      writeln('begin');
      // default
      if member._label.get_type.kind = tk_octet then
        val := get_default_value_for_union(u.discriminator_type_def,members)
      else if discr.def_kind = dk_Enum then
        begin
          ind := member._label.get_variant;
          assert(ind < Length(emembers));
          val := emembers[ind];
        end
      else if discr._type.kind = tk_char then
        val := '''' + member._label.get_variant+ ''''
      else
        val := member._label.get_variant;
      writeln('  Fdiscriminator := '+val+';');
      writeln('  F'+member.name+' := val;');
      writeln('end;');
      writeln;
      writeln('function T'+name+'.get_'+member.name+': '+typestr+';');
      writeln('begin');
      writeln('  result := F'+member.name+';');
      writeln('end;');
      writeln;
      last_name := member.name;
    end;
  // discriminator
  writeln('function T'+name+'.discriminator: '+type_str(discr)+';');
  writeln('begin');
  writeln('  result := FDiscriminator;');
  writeln('end;');
  writeln;
end;

procedure TCodegen.code_impl_value_box(val: IValueBoxDef);
var
  name, cname, tname, repoid : string;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  name := get_name(val as IContained);
  repoid := (val as IContained).id;
  cname := 'T'+name;
  if val.original_type_def.def_kind = dk_Sequence then begin
    if is_base_sequence(val.original_type_def as ISequenceDef) then
      tname := expand_name(val.original_type_def)
    else
      tname := 'T' + name + '_Value'
  end
  else
    tname := type_str(val.original_type_def);
  writeln('//***********************************************************');
  writeln('// '+cname);
  writeln('//***********************************************************');
  writeln('constructor '+cname+'.Create(val: '+tname+');');
  writeln('begin');
  writeln('  inherited Create();');
  writeln('  FValue := val;');
  writeln('end;');
  writeln;
  writeln('constructor '+cname+'.Create(val: I'+name+');');
  writeln('begin');
  writeln('  inherited Create();');
  writeln('  FValue := val._value;');
  writeln('end;');
  writeln;
  writeln('function '+cname+'._value: '+tname+';');
  writeln('begin');
  writeln('  result := FValue;');
  writeln('end;');
  writeln;
  writeln('procedure '+cname+'._value(val : '+tname+');');
  writeln('begin');
  writeln('  FValue := val;');
  writeln('end;');
  writeln;
  // narrow_helper
  writeln('function '+cname+'.narrow_helper(const repoid: string): Pointer;');
  writeln('begin');
  writeln('  result := nil;');
  writeln('  if strcomp(PChar(repoid),'''+repoid+''') = 0 then');
  writeln('    result := Pointer(Self as I'+name+');');
  writeln('end;');
  writeln;
  // _downcast
  writeln('class function '+cname+'._downcast(const vb : IValueBase): I'+name+';');
  writeln('var');
  writeln('  ptr: pointer;');
  writeln('begin');
  writeln('  result := nil;');
  writeln('  if vb <> nil then');
  writeln('    begin');
  writeln('      ptr := vb.narrow_helper('''+repoid+''');');
  writeln('      if ptr <> nil then');
  writeln('        result := I'+name+'(ptr);');
  writeln('    end;');
  writeln('end;');
  writeln;
  // _get_marshal_info
  writeln('procedure '+cname+'._get_marshal_info(const str: TStrings; out chunked: Boolean);');
  writeln('begin');
  writeln('  str.Add('''+repoid+''');');
  writeln('  chunked := false;');
  writeln('end;');
  writeln;
  // _marshal_members
  writeln('procedure '+cname+'._marshal_members(const enc: IEncoder);');
  writeln('begin');
  writeln('  '+marshaller_for(val.original_type_def)+'.marshal(enc,@FValue);');
  writeln('end;');
  writeln;
  // _demarshal_members
  writeln('function  '+cname+'._demarshal_members(const dec: IDecoder): Boolean;');
  writeln('begin');
  writeln('  result := false;');
  writeln('  if not '+marshaller_for(val.original_type_def)+'.demarshal(dec,@FValue) then exit;');
  writeln('  result := true;');
  writeln('end;');
  writeln;
  // _copy_value
  writeln('function  '+cname+'._copy_value: IValueBase;');
  writeln('begin');
  writeln('  result := '+cname+'.Create(self) as IValueBase;');
  writeln('end;');
  writeln;
  // Marshaller
  value_marshaller(val as IContained);
  if (val.original_type_def.def_kind = dk_Sequence) and not is_base_sequence(val.original_type_def as ISequenceDef) then
    seq_marshaller(val.original_type_def as ISequenceDef);
end;

procedure TCodegen.code_impl_value(val: IValueDef);
var
  name, cname, tname, mname, fname, repoid : string;
  list, supported, alist, mlist: IInterfaceList;
  bval: IValueDef;
  i, j, n : integer;
  operation  : string;
  vmd: IValueMemberDef;
  iseq: initializerseq;
  int : IInterfaceDef;
  oper : IOperationDef;
  res: IIDLType;
  attr : IAttributeDef;
begin
  iseq := nil;
  if FDB.repoid_defined((val as IContained).id) then exit;
  repoid := (val as IContained).id;
  name := get_name(val as IContained);
  cname := 'T'+name;
  writeln('//***********************************************************');
  writeln('// '+cname);
  writeln('//***********************************************************');
  list := (val as IContainer).contents(dk_ValueMember);
  bval := val.get_base_value;
  for i := 0 to Pred(list.count) do
    begin
      vmd := list[i] as IValueMemberDef;
      mname := vmd.name;
      tname := type_str(vmd.get_type_def);
      writeln('function '+cname+'.'+mname+': '+tname+';');
      writeln('begin');
      writeln('  result := F'+name+'_'+mname+';');
      writeln('end;');
      writeln;
      writeln('procedure '+cname+'.'+mname+'(val : '+tname+');');
      writeln('begin');
      writeln('  F'+name+'_'+mname+' := val;');
      writeln('end;');
      writeln;
    end;
  // supported interfaces
  supported := val.get_supported_interfaces;
  for i := 0 to Pred(supported.count) do
    begin
      int := supported[i] as IInterfaceDef;
      if (i = 0) and int.get_is_abstract then
        begin
          writeln('function '+cname+'._to_object: IORBObject;');
          writeln('begin');
          writeln('  result := nil;');
          writeln('end;');
          writeln;
          writeln('function '+cname+'._to_value: IValueBase;');
          writeln('begin');
          writeln('  result := self;');
          writeln('end;');
          writeln;
        end;
      mlist := (int as IContainer).contents(dk_Operation);
      alist := (int as IContainer).contents(dk_Attribute);
      for j := 0 to Pred(mlist.count) do
        begin
          oper := mlist[j] as IOperationDef;
          operation := operation_str(oper);
          n := pos(' ',operation);
          writeln(copy(operation,1,n)+cname+'.'+copy(operation,n+1,10000));
          writeln('begin');
          res := oper.get_result_def;
          if res._type.kind <> orbtypes.tk_void then
            writeln('  result := '+def_value(res)+';')
          else
            writeln;
          writeln('end;');
          writeln;
        end;
      for j := 0 to Pred(alist.count) do
        begin
          attr := alist[j] as IAttributeDef;
          tname := type_str(attr.get_type_def);
          if attr.get_mode = ATTR_NORMAL then
            begin
              writeln('procedure '+cname+'._set_'+attr.name+'(const val: '+tname+');');
              writeln('begin');
              writeln;
              writeln('end;');
              writeln;
            end;
          res := attr.get_type_def;
          writeln('function '+cname+'._get_'+attr.name+': '+tname+';');
          writeln('begin');
          writeln('  result := '+def_value(res)+';');
          writeln('end;');
          writeln;
        end;
      break;
    end;
  // narrow_helper
  writeln('function '+cname+'.narrow_helper(const repoid: string): Pointer;');
  writeln('begin');
  writeln('  if strcomp(PChar(repoid),'''+repoid+''') = 0 then');
  writeln('    result := Pointer(Self as I'+name+')');
  for i := 0 to Pred(supported.count) do
    begin
      writeln('  else if strcomp(PChar(repoid),'''+(supported[i] as IInterfaceDef).id+''') = 0 then');
      writeln('    result := Pointer(Self as I'+get_name(supported[i] as IInterfaceDef)+')');
    end;
  writeln('  else');
  if assigned(bval) then
    writeln('    result := inherited narrow_helper(repoid);')
  else
    writeln('    result := nil;');
  writeln('end;');
  writeln;
  // _downcast
  writeln('class function '+cname+'._downcast(const vb : IValueBase): I'+name+';');
  writeln('var');
  writeln('  ptr: pointer;');
  writeln('begin');
  writeln('  result := nil;');
  writeln('  if vb <> nil then');
  writeln('    begin');
  writeln('      ptr := vb.narrow_helper('''+repoid+''');');
  writeln('      if ptr <> nil then');
  writeln('        result := I'+name+'(ptr);');
  writeln('    end;');
  writeln('end;');
  writeln;
  // _get_marshal_info
  writeln('procedure '+cname+'._get_marshal_info(const str: TStrings; out chunked: Boolean);');
  writeln('begin');
  writeln('  str.Add('''+repoid+''');');
  writeln('  chunked := false;');
  writeln('end;');
  writeln;
  // _marshal_members
  writeln('procedure '+cname+'._marshal_members(const enc: IEncoder);');
  writeln('begin');
  if assigned(bval) then
    writeln('  inherited _marshal_members(enc);');
  for i := 0 to Pred(list.count) do
    begin
      vmd := list[i] as IValueMemberDef;
      mname := vmd.name;
      writeln('  '+marshaller_for(vmd.get_type_def)+'.marshal(enc,@F'+name+'_'+mname+');');
    end;
  writeln('end;');
  writeln;
  // _demarshal_members
  writeln('function  '+cname+'._demarshal_members(const dec: IDecoder): Boolean;');
  writeln('begin');
  if assigned(bval) or (list.count > 0) then
    begin
      writeln('  result := false;');
      if assigned(bval) then
        writeln('  if not inherited _demarshal_members(dec) then exit;');
      for i := 0 to Pred(list.count) do
        begin
          vmd := list[i] as IValueMemberDef;
          mname := vmd.name;
          writeln('  if not '+marshaller_for(vmd.get_type_def)+'.demarshal(dec,@F'+name+'_'+mname+') then exit;');
        end;
    end;
  writeln('  result := true;');
  writeln('end;');
  writeln;
  // _copy_members
  writeln('procedure '+cname+'._copy_members(const v: TObject);');
  if list.count <> 0 then begin
    writeln('var');
    writeln('  other : T'+name+';');
  end;
  writeln('begin');
  writeln('  inherited _copy_members(v);');
  if list.count <> 0 then
    writeln('  other := v as T'+name+';');
  for i := 0 to Pred(list.count) do
    begin
      vmd := list[i] as IValueMemberDef;
      writeln('  '+vmd.name+'(other.'+vmd.name+');');
    end;
  writeln('end;');
  writeln;
  // _copy_value
  writeln('function  '+cname+'._copy_value: IValueBase;');
  writeln('var');
  writeln('  list: TStrings;');
  writeln('begin');
  writeln('  list := TStringList.Create;');
  writeln('  try');
  writeln('    result := _create(list,'''+(val as IContained).id+''');');
  writeln('    result._copy_members(self);');
  writeln('  finally');
  writeln('    list.free;');
  writeln('  end;');
  writeln('end;');
  writeln;
  // init
  iseq := val.initializers;
  if Length(iseq) > 0 then
    begin
      fname := cname+'_init';
      writeln('//***********************************************************');
      writeln('// '+fname);
      writeln('//***********************************************************');
      // _downcast
      writeln('class function '+fname+'._downcast(const vf : IValueFactory): I'+name+'_init;');
      writeln('var');
      writeln('  ptr: pointer;');
      writeln('begin');
      writeln('  result := nil;');
      writeln('  if vf <> nil then');
      writeln('    begin');
      writeln('      ptr := vf.narrow_helper('''+repoid+''');');
      writeln('      if ptr <> nil then result := I'+name+'_init(ptr);');
      writeln('    end;');
      writeln('end;');
      writeln;
      // narrow_helper
      writeln('function '+fname+'.narrow_helper(const repoid: string): Pointer;');
      writeln('begin');
      writeln('  result := nil;');
      writeln('  if strcomp(PChar(repoid),'''+repoid+''')= 0 then');
      writeln('    result := Pointer( self as I'+name+'_init);');
      writeln('end;');
      writeln;
    end;
  // Marshaller
  value_marshaller(val as IContained);
end;

procedure TCodegen.code_impl_module(m: IModuleDef);
var
  i : integer;
  contents : IInterfaceList;
begin
  //if FDB.repoid_defined(m.id) then exit;
  contents := (m as IContainer).contents();
  for i := 0 to Pred(contents.count) do
    impl_code(contents[i] as IContained);
end;

procedure TCodegen.code_impl_interface(int: IInterfaceDef);
var
  id_str, name, cname, iname, biname: string;
  bid_str, stub_name, skel_name: string;
  base, methods, bmethods, attributes: IInterfaceList;
  baseint: IInterfaceDef;
  i, j: integer;
  contents: IInterfaceList;
  abstract_base_class : Boolean;

  procedure impl_stub_methods(int: IInterfaceDef; except_first: boolean);
  var
    i : integer;
    list: IInterfaceList;
    n: integer;
  begin
    list := int.get_base_interfaces;
    if except_first then n := 1 else n := 0;
    for i := n to Pred(list.count) do
      impl_stub_methods(list[i] as IInterfaceDef, i = n);
    list := (int as IContainer).contents(dk_Operation);
    for i := 0 to Pred(list.count) do
      stub_method(name,list[i] as IOperationDef,int.get_is_abstract);
    list := (int as IContainer).contents(dk_Attribute);
    for i := 0 to Pred(list.count) do
      stub_attribute(name,list[i] as IAttributeDef,int.get_is_abstract);
  end;

  procedure impl_stub_methods2(int: IInterfaceDef; except_first: boolean);
  var
    i, j: integer;
    all: IInterfaceList;
    list: IInterfaceList;
  begin
      all := prepare_int_list(int, except_first);
      all.Add(int);

      for i := 0 to Pred(all.count) do begin
          list := (all[i] as IContainer).contents(dk_Operation);
          for j:=0 to Pred(list.count) do begin
            stub_method(name,list[j] as IOperationDef,int.get_is_abstract);
          end;
          list := (all[i] as IContainer).contents(dk_Attribute);
          for j:=0 to Pred(list.count) do begin
              stub_attribute(name,list[j] as IAttributeDef,int.get_is_abstract);
          end;
      end;
  end;

  procedure impl_serv_methods(int: IInterfaceDef);
  var
    i, j: integer;
    all: IInterfaceList;
    list: IInterfaceList;
  begin
    all := prepare_int_list(int, True);
    for i := 0 to all.Count - 1 do begin
      list := (all[i] as IContainer).contents(dk_Operation);
      for j := 0 to list.Count - 1 do
        serv_method(name, list[j] as IOperationDef, 'I' + get_name(all[i] as IContained));
    end;
  end;

  procedure impl_serv_attrbute(int: IInterfaceDef);
  var
    i, j: integer;
    all: IInterfaceList;
    list: IInterfaceList;
  begin
    all := prepare_int_list(int, True);
    for i := 0 to all.Count - 1 do begin
      list := (all[i] as IContainer).contents(dk_Attribute);
      for j := 0 to list.Count - 1 do
        serv_attribute(name, list[j] as IAttributeDef, 'I' + get_name(all[i] as IContained));
    end;
  end;

  procedure write_poa_stub;
  var i, j: integer;
      ibase, iattr: IInterfaceList;
  begin
    writeln('//***********************************************************');
    writeln('// '+cname+'_stub_clp');
    writeln('//***********************************************************');
    // narrow_helper
    writeln('function '+cname+'_stub_clp.narrow_helper(const str: string): Pointer;');
    writeln('begin');
    if (base.count = 0) or (base[0] as IInterfaceDef).get_is_abstract then
      writeln('  result := nil;');
    writeln('  if str = '+id_str+' then');
    writeln('    begin');
    writeln('      result := Pointer(self as '+iname+');');
    writeln('      exit;');
    if (base.count > 0) then begin
      if (base[0] as IInterfaceDef).get_is_abstract then
        j := 0
      else
        j := 1;

      for i := j to Pred(base.count) do begin
        ibase := TInterfaceList.Create;
        rget_base_list(base[i] as IInterfaceDef, ibase);
        for j:=0 to Pred(ibase.count) do begin
          writeln('    end');
          writeln('  else if str = '''+(ibase[j] as IInterfaceDef).id+''' then');
          writeln('    begin');
          writeln('      result := Pointer(self as I'+get_name(ibase[j] as IInterfaceDef)+');');
          writeln('      exit;');
        end;
      end;

      writeln('    end;');
      if not (base[0] as IInterfaceDef).get_is_abstract then
        writeln('  result := inherited narrow_helper(str);');
    end
    else
      writeln('  end;');
    writeln('end;');
    writeln; // helper done
    for i := 0 to Pred(methods.count) do
      stub_clp_method(name,name,methods[i] as IOperationDef);

    ibase := prepare_int_list(int, true);
    for j := 0 to Pred(ibase.count) do begin
      bmethods := (ibase[j] as IContainer).contents(dk_Operation);
      for i := 0 to Pred(bmethods.count) do begin
        stub_clp_method(name, get_name(ibase[j] as IInterfaceDef),bmethods[i] as IOperationDef);
      end;
      iattr := (ibase[j] as IContainer).contents(dk_Attribute);
      for i := 0 to Pred(iattr.count) do begin
        stub_clp_attribute2(name, get_name(ibase[j] as IInterfaceDef), iattr[i] as IAttributeDef);
      end;
    end;

    for i := 0 to Pred(attributes.Count) do begin
      stub_clp_attribute(name,attributes[i] as IAttributeDef);
    end;
  end;

begin
  if FDB.repoid_defined(int.id) then exit;
  contents := (int as IContainer).contents();
  // internal declarations
  for i := 0 to Pred(contents.count) do
    impl_code(contents[i] as IContained);
  name := get_name(IContained(int));
  cname := 'T' + name;
  stub_name := cname + '_stub';
  skel_name := cname + '_skel';
  iname := 'I' + name;
  writeln('//***********************************************************');
  writeln('// '+cname);
  writeln('//***********************************************************');
  methods := (int as IContainer).contents(dk_Operation);
  attributes := (int as IContainer).contents(dk_Attribute);
  base := int.get_base_interfaces;
  // narrow_helper
  writeln('function '+cname+'.narrow_helper(const str: string): Pointer;');
  writeln('begin');
  id_str := ''''+int.id+'''';
  abstract_base_class := (base.count > 0) and (base.First as IInterfaceDef).get_is_abstract;
  if (base.count = 0) or abstract_base_class then
    writeln('  result := nil;');
  writeln('  if str = '+id_str+' then');
  writeln('    begin');
  writeln('      result := Pointer(self as '+iname+');');
  writeln('      exit;');
  writeln('    end;');
  if abstract_base_class then
    j := 0
  else
    j := 1;
  for i := j to Pred(base.count) do
    begin
      baseint := base[i] as IInterfaceDef;
      bid_str := ''''+baseint.id+'''';
      iname := 'I' + get_name(baseint);
      writeln('  if str = '+bid_str+' then');
      writeln('    begin');
      writeln('      result := Pointer(self as '+iname+');');
      writeln('      exit;');
      writeln('    end;');
    end;
  if (base.count > 0) and not abstract_base_class{ or int.get_is_local} then
    writeln('  result := inherited narrow_helper(str);');
  writeln('end;');
  writeln;
  iname := 'I' + name;
  // if base class is abstarct
  if abstract_base_class then
    begin
      writeln('function '+cname+'._to_object: IORBObject;');
      writeln('begin');
      writeln('  result := self;');
      writeln('end;');
      writeln;
      writeln('function '+cname+'._to_value: IValueBase;');
      writeln('begin');
      writeln('  result := nil;');
      writeln('end;');
      writeln;
    end;
  // for abstract
  if int.get_is_abstract then
    begin
      writeln('class function '+cname+'._narrow(const obj : IAbstractBase): '+iname+';');
      writeln('begin');
      writeln('  result := _downcast(obj._to_value);');
      writeln('  if not assigned(result) then');
      writeln('    result := _narrow(obj._to_object);');
      writeln('end;');
      writeln;
      writeln('class function '+cname+'._downcast(const vb : IValueBase): '+iname+';');
      writeln('begin');
      writeln('  result := nil;');
      writeln('  if assigned(vb) then');
      writeln('    result := '+iname+'(vb.narrow_helper('+id_str+'));');
      writeln('end;');
      writeln;
    end;

  // _narrow
  writeln('class function '+cname+'._narrow(const obj : IORBObject): '+iname+';');
  writeln('var');
  writeln('  p: Pointer;');
  if not int.get_is_local then
    writeln('  stub: '+stub_name+';');
  writeln('begin');
  writeln('  result := nil;');
  writeln('  if obj = nil then exit;');
  writeln('  p := obj.narrow_helper('+id_str+');');
  writeln('  if p <> nil then');
  writeln('    result := '+iname+'(p)');
  if not int.get_is_local then begin
    writeln('  else');
    writeln('    begin');
    writeln('      if narrow_helper2(obj) or obj._is_a_remote('+id_str+') then');
    writeln('        begin');
    writeln('          stub := '+stub_name+'.Create();');
    writeln('          stub.assign(obj);');
    writeln('          result := stub;');
    writeln('        end;');
    writeln('    end;');
  end;
  writeln('end;');
  writeln;
  // narrow_helper2
  writeln('class function '+cname+'.narrow_helper2(const obj: IORBObject): Boolean;');
  writeln('begin');
  writeln('  result := obj.repoid = '+id_str);
  writeln('end;');
  writeln;

  if int.get_is_local then
    Exit;

  // stub methods
  writeln('//***********************************************************');
  writeln('// '+stub_name);
  writeln('//***********************************************************');
  //impl_stub_methods(int,false);
  impl_stub_methods2(int,false);
  if FParams.IndexOf('-poa') < 0 then
    begin
      // BOA skel
      writeln('//***********************************************************');
      writeln('// '+skel_name);
      writeln('//***********************************************************');
      writeln('constructor '+skel_name+'.Create;');
      writeln('var');
      writeln('  impl: IImplementationDef;');
      writeln('begin');
      writeln('  inherited Create();');
      writeln('  impl := find_impl('+id_str+','''+name+''');');
      writeln('  create_ref('''',nil,impl,'+id_str+');');
      for j := 1 to Pred(base.count) do
        begin
          baseint := base[j] as IInterfaceDef;
          writeln('  impl := find_impl('''+baseint.id+''','''+get_name(baseint)+''');');
          writeln('  create_ref('''',nil,impl,'''+baseint.id+''');');
        end;
      writeln('  register_dispatcher(IStaticInterfaceDispatcher(self));');
      writeln('end;');
      writeln;
      method_dispatch(skel_name,int,true);
      writeln('function '+skel_name+'.narrow_helper(const str: string): Pointer;');
      writeln('begin');
      if base.count = 0 then
        writeln('  result := nil;');
      writeln('  if str = '+id_str+' then');
      writeln('    begin');
      writeln('      result := Pointer(self as I'+name+');');
      writeln('      exit;');
      writeln('    end;');
      for i := 1 to Pred(base.count) do
        begin
          baseint := base[i] as IInterfaceDef;
          bid_str := ''''+baseint.id+'''';
          iname := 'I' + get_name(baseint);
          writeln('  if str = '+bid_str+' then');
          writeln('    begin');
          writeln('      result := Pointer(self as '+iname+');');
          writeln('      exit;');
          writeln('    end;');
        end;
      if base.count > 0 then
        begin
          writeln('  result := inherited narrow_helper(str);');
        end;
      writeln('end;');
      writeln;
    end
  else
    begin
      // POA stub
      write_poa_stub;
      // POA servant
      writeln('//***********************************************************');
      writeln('// '+cname+'_serv');
      writeln('//***********************************************************');
      if FParams.IndexOf('-notabs') >= 0 then begin
        // Create
        writeln('constructor '+cname+'_serv.Create(const AImpl: '+iname+');');
        writeln('begin');
        if base.count = 0 then
          writeln('  inherited Create();')
        else
          writeln('  inherited Create(AImpl);');
        writeln('  FImplementation := AImpl;');
        writeln('end;');
        writeln;
        // methods
        impl_serv_methods(int);
        for i := 0 to Pred(methods.count) do
          serv_method(name,methods[i] as IOperationDef);
        //get and set attributes methods
        impl_serv_attrbute(int);
        for i := 0 to Pred(attributes.Count) do
          serv_attribute(name, attributes[i] as IAttributeDef);
      end;
      
      // _this
      writeln('function '+cname+'_serv._this(): '+iname+';');
      writeln('var');
      writeln('  obj: IORBObject;');
      writeln('begin');
      writeln('  obj := this();');
      writeln('  result := '+cname+'._narrow(obj);');
      writeln('end;');
      writeln;
      // _is_a
      writeln('function '+cname+'_serv._is_a(const repoid: string): Boolean;');
      writeln('begin');
      writeln('  result := repoid = '+id_str+';');
      if base.count > 0 then begin
        base := int.get_base_interfaces;
        for i := 1 to Pred(base.count) do begin
          baseint := base[i] as IInterfaceDef;
          bid_str := ''''+baseint.id+'''';
          writeln('  if repoid = '+bid_str+' then');
          writeln('    begin');
          writeln('      result := true;');
          writeln('      exit;');
          writeln('    end;');
        end;
        writeln('  if not result then');
        writeln('    result := inherited _is_a(repoid);');
      end;
      writeln('end;');
      writeln;
      // _primary_interface
      writeln('function '+cname+'_serv._primary_interface(const objid: ObjectID; const poa: IPOA): string;');
      writeln('begin');
      writeln('  result := '+id_str+';');
      writeln('end;');
      writeln;
      // _make_stub
      writeln('function '+cname+'_serv._make_stub(const poa: IPOA; const obj : IORBObject): IORBObject;');
      writeln('begin');
      writeln('  result := '+cname+'_stub_clp.Create(poa,obj);');
      writeln('end;');
      writeln;
      writeln('procedure '+cname+'_serv.invoke(const serv: IStaticServerRequest);');
      writeln('begin');
      writeln('  if not _dispatch(serv) then');
      writeln('    begin');
      writeln('      serv.set_exception(BAD_OPERATION.Create(0, COMPLETED_NO) as IORBException);');
      writeln('      serv.write_results();');
      writeln('    end;');
      writeln('end;');
      writeln;
      // narrow_helper
      writeln('function '+cname+'_serv.narrow_helper(const str: string): Pointer;');
      writeln('begin');
      if base.count = 0 then
        writeln('  result := nil;');
      writeln('  if str = '+id_str+' then');
      writeln('    begin');
      writeln('      result := Pointer('+iname+'(self));');
      writeln('      exit;');
      writeln('    end;');
      base := int.get_base_interfaces;
      for i := 1 to Pred(base.count) do
        begin
          baseint := base[i] as IInterfaceDef;
          bid_str := ''''+baseint.id+'''';
          biname := 'I' + get_name(baseint);
          writeln('  if str = '+bid_str+' then');
          writeln('    begin');
          writeln('      result := Pointer(self as '+biname+');');
          writeln('      exit;');
          writeln('    end;');
        end;
      if base.count > 0 then
        begin
          writeln('  result := inherited narrow_helper(str);');
        end;
      writeln('end;');
      writeln;
      // _narrow
      writeln('class function '+cname+'_serv._narrow(const srv : IServant): '+iname+';');
      writeln('var');
      writeln('  p: Pointer;');
      writeln('begin');
      writeln('  result := nil;');
      writeln('  if srv = nil then exit;');
      writeln('  p := srv.narrow_helper('+id_str+');');
      writeln('  if p <> nil then');
      writeln('    result := '+iname+'(p)');
      writeln('end;');
      writeln;
      method_dispatch(cname+'_serv',int,false);
    end;
  //----------- marshaller ---------------
  int_marshaller(int);
end;


procedure TCodegen.code_impl_struct(str: IStructDef);
var
  i : integer;
  members : StructMemberSeq;
  contents : IInterfaceList;
begin
  members := nil;
  if FDB.repoid_defined((str as IContained).id) then exit;
  contents := (str as IContainer).contents();
  // internal declarations
  for i := 0 to Pred(contents.count) do
    impl_code(contents[i] as IContained);
  members := str.members^;
  for i := 0 to Pred(Length(members)) do
    case members[i].type_def.def_kind of
      dk_Sequence : if not is_base_sequence(members[i].type_def as ISequenceDef) then
        seq_marshaller(members[i].type_def as ISequenceDef);
      dk_Array : array_marshaller(members[i].type_def as IArrayDef);
    end;
  struct_marshaller(str as IContained)
end;

procedure TCodegen.int_declare_sequence(seq: ISequenceDef);
var
  tname : string;
  bound : longint;
begin
  //if FDB.repoid_defined(FDB.pseudo_repoid(seq)) then  exit;
  tname := type_str(seq.element_type_def);
  bound := seq.bound;
  if bound = 0 then
    writeln('array of '+ tname+';')
  else
    writeln('array [0..'+inttostr(bound-1)+'] of '+ tname+';')
end;

procedure TCodegen.int_declare_array(arr: IArrayDef);
begin
  if FDB.repoid_defined(FDB.pseudo_repoid(arr)) then  exit;
  writeln(array_str(arr));
end;

procedure TCodegen.int_declare_alias(cont: IAliasDef);
var
  c : IContained;
  t : IIDLType;
begin
  if FDB.repoid_defined((cont as IContained).id) then exit;
  c := cont as IContained;
  write('  T'+ get_name(c) +' = ');
  t :=  cont.original_type_def;
  case t.def_kind of
    dk_Sequence : int_declare_sequence(t as ISequenceDef);
    dk_Enum : writeln('T' + get_name(t as IContained)+';');
    dk_Primitive : writeln(type_str(t)+';');
    dk_Struct: writeln(type_str(t)+';');
    dk_Array : writeln(array_str(t as IArrayDef));
    dk_Alias : writeln('T'+get_name(t as IContained)+';');
    dk_Interface : begin
        if FDB.repoid_defined((t as IContained).id) then
          writeln(ChangeFileExt(FDB.repoid_file_name((t as IContained).id),'_int')+'.I'+(t as IContained).name)
        else
          writeln('I'+get_name(t as IContained)+';');
      end;
    else
      assert(false,'not implemented');
  end;
  writeln;
end;

procedure TCodegen.int_declare_enum(enum: IEnumDef);
var
  i : integer;
  name : string;
begin
  if FDB.repoid_defined((enum as IContained).id) then exit;
  name := get_name(enum as IContained);
  if not declare(name) then exit;
  writeln('  T'+name+' = (');
  for i := 0 to Pred(length(enum.members^)) do
    if i = Pred(length(enum.members^)) then
      writeln('    '+ safe_name(enum.members^[i]))
    else
      writeln('    '+ safe_name(enum.members^[i])+',');
  writeln('  );');
  writeln;
end;

procedure TCodegen.int_declare_module(m: IModuleDef);
var
  i : integer;
  contents : IInterfaceList;
begin
  if not FDB.repoid_defined(m.id) then begin
    writeln('// module '+m.name+' types');
    writeln;
  end;
  contents := (m as IContainer).contents();
  for i := 0 to Pred(contents.count) do
    int_declare(contents[i] as IContained);
end;

procedure TCodegen.int_declare_internal_type(cont_name,name: string; t: IIDLType);
var
  tmp : string;
begin
  case t.def_kind of
    dk_Union : int_declare_union(t as IUnionDef);
    dk_Enum : int_declare_enum(t as IEnumDef);
    dk_Struct: int_declare_struct(t as IStructDef);
    dk_Sequence: begin
        tmp := cont_name+'_'+name+'_seq';
        if not declare(tmp) then exit;
        write('  T'+tmp+' = ');
        int_declare_sequence(t as ISequenceDef);
        writeln;
      end;
    dk_Array: begin
        tmp := cont_name+'_'+name+'_array';
        if not declare(tmp) then exit;
        write('  T'+tmp+' = ');
        int_declare_array(t as IArrayDef);
        writeln;
      end;
  end;
end;

procedure TCodegen.int_declare_union(u: IUnionDef);
var
  members : UnionMemberSeq;
  member: UnionMember;
  name,typestr,last_name : string;
  i : integer;

  procedure internal_types;
  var
    i : integer;
    name: string;
  begin
    name := get_name(u as IContained);
    members := u.members^;
    for i := 0 to Pred(Length(members)) do
      int_declare_internal_type(name,members[i].name,members[i].type_def);
  end;

begin
  if FDB.repoid_defined((u as IContained).id) then exit;
  name := get_name(u as IContained);
  if not declare(name) then exit;
  internal_types;
  members := u.members^;
  writeln('  I'+name+' = interface');
  writeln('  ['''+CreateStringGUID(u._type.repoid)+''']');
  for i := 0 to Pred(Length(members)) do
    begin
      member := members[i];
      if member.name = last_name then continue;
      case member.type_def.def_kind of
        dk_Sequence : typestr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : typestr := 'T'+name + '_'+member.name + '_array';
        else
          typestr := type_str(member.type_def)
      end;
      writeln('    procedure set_'+member.name+'(const val: '+typestr+');');
      writeln('    function get_'+member.name+': '+typestr+';');
      last_name := member.name;
    end;
    writeln('    function discriminator: '+type_str(u.discriminator_type_def)+';');
  writeln('  end;');
  writeln;
end;

procedure TCodegen.int_declare_interface(int: IInterfaceDef);
var
  iname: string;
  base_int: IInterfaceList;
  i : integer;
  contents : IInterfaceList;
begin
  if FDB.repoid_defined(int.id) then exit;
  if not declare(get_name(IContained(int))) then exit;
  contents := (int as IContainer).contents();
  // internal declarations
  for i := 0 to Pred(contents.count) do
    int_declare(contents[i] as IContained);
  iname := 'I'+get_name(IContained(int));
  write('  '+iname+' = interface');
  base_int := int.get_base_interfaces;
  if base_int.Count > 0 then
    writeln('(I'+get_name(IContained(base_int[0]))+')')
  else
    writeln;
  writeln('  ['''+CreateStringGUID(int.id)+''']');
  declare_methods(int,'');
  declare_attributes(int,'',true);
  writeln('  end;');
  writeln;
end;

procedure TCodegen.int_declare_exception(e : IExceptionDef);
var
  ename: string;
  members: StructMemberSeq;
  i : integer;
begin
  members := nil;
  if FDB.repoid_defined(e.id) then exit;
  ename := safe_name(get_name(e as IContained));
  if not declare(ename) then exit;
  members := e.members^;
  // declare internally defined types
  for i := 0 to Pred(Length(members)) do
    int_declare_internal_type(ename,members[i].name,members[i].type_def);
  writeln('  I'+ename+' = interface(IUserException)');
  writeln('  ['''+CreateStringGUID(e._type.repoid)+''']');
  ex_members_decl(ename,members);
  writeln('  end;');
  writeln
end;

procedure TCodegen.int_declare_struct(str: IStructDef);
var
  sname,tstr: string;
  i : integer;
  members : StructMemberSeq;
begin
  members := nil;
  if FDB.repoid_defined((str as IContained).id) then exit;
  sname := safe_name(get_name(str as IContained));
  if not declare(sname) then exit;
  members := str.members^;
  // internal types
  for i := 0 to Pred(Length(members)) do
    int_declare_internal_type(sname,members[i].name,members[i].type_def);
  writeln('  T'+sname+' = record');
  for i := 0 to Pred(Length(members)) do
    begin
      case members[i].type_def.def_kind of
        dk_Sequence : tstr := 'T'+sname+'_'+members[i].name+'_seq';
        dk_Array : tstr := 'T'+sname+'_'+members[i].name+'_array';
        else
          tstr := type_str(members[i].type_def);
      end;
      writeln('    '+safe_name(members[i].name)+' : '+tstr+';');
    end;
  writeln('  end;');
  writeln;
end;

procedure TCodegen.int_declare_value_box(val: IValueBoxDef);
var
  vname,tname : string;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  vname := 'I'+get_name(val as IContained);
  if not declare(vname) then exit;
  if val.original_type_def.def_kind = dk_Sequence then begin
    if is_base_sequence(val.original_type_def as ISequenceDef) then
      tname := expand_name(val.original_type_def)
    else begin
      tname := 'T' + get_name(val as IContained) + '_Value';
      writeln('  ' + tname + ' = ' + type_str(val.original_type_def) + ';');
      writeln;
    end;
  end;
  writeln('  '+vname+' = interface(IValueBase)');
  writeln('  ['''+CreateStringGUID(val.id)+''']');
  if tname = '' then
    tname := type_str(val.original_type_def);
  writeln('    function _value : '+tname+'; overload;');
  writeln('    procedure _value(val : '+tname+'); overload;');
  writeln('  end;');
  writeln;
end;

procedure TCodegen.int_declare_value(val: IValueDef);
var
  vname : string;
  list, supported : IInterfaceList;
  bval : IValueDef;
  i : integer;
  int : IInterfaceDef;
begin
  if FDB.repoid_defined((val as IContained).id) then exit;
  list := (val as IContainer).contents();
  vname := 'I'+get_name(val as IContained);
  if not declare(vname) then exit;
  // internal declarations
  for i := 0 to Pred(list.count) do
    int_declare(list[i] as IContained);
  bval := val.get_base_value;
  supported := val.get_supported_interfaces;
  if bval <> nil then
    writeln('  '+vname+' = interface(I'+get_name(bval as IContained)+')')
  else if (supported.Count > 0) and (supported[0] as IInterfaceDef).get_is_abstract then
    writeln('  '+vname+' = interface(I'+get_name(supported[0] as IContained)+')')
  else
    writeln('  '+vname+' = interface(IValueBase)');
  writeln('  ['''+CreateStringGUID(val._type.repoid)+''']');
  list := (val as IContainer).contents(dk_Operation);
  for i := 0 to Pred(list.count) do
    writeln('    '+operation_str(list[i] as IOperationDef));
  list := (val as IContainer).contents(dk_ValueMember);
  for i := 0 to Pred(list.count) do
    value_member(list[i] as IValueMemberDef, {False}True);
  // supported interfaces
  for i := 0 to Pred(supported.count) do
    begin
      int := supported[i] as IInterfaceDef;
      declare_methods(int,'');
      declare_attributes(int,'',true);
      break; // multiple interfaces not supported
    end;
  writeln('  end;');
  writeln;
  // Init interface
  if Length(val.initializers) > 0 then
    begin
      writeln('  '+vname+'_init = interface(IValueFactory)');
      writeln('  ['''+CreateStringGUID(val._type.repoid + '/Factory')+''']');
      for i := 0 to Pred(Length(val.initializers)) do
        writeln('    '+initializer_decl(val.initializers[i],val));
      writeln('  end;');
      writeln;
    end;
end;

procedure TCodegen.final_code(cont: IContained);
var
  i : integer;
  contents : IInterfaceList;
begin
  case IIRObject(cont).def_kind of
    dk_Interface : final_impl_interface(IInterfaceDef(cont));
    dk_Value : final_impl_value(cont as IValueDef);
    dk_Module : begin
        contents := (cont as IContainer).contents;
        for i := 0 to Pred(contents.count) do
          final_code(contents[i] as IContained);
      end;
  end;
end;

procedure TCodegen.final_declare(cont: IContained);
var
  i : integer;
  contents: IInterfaceList;
begin
  case IIRObject(cont).def_kind of
    dk_Interface : final_declare_interface(IInterfaceDef(cont));
    dk_Value : final_declare_value(cont as IValueDef);
    dk_Module : begin
        contents := (cont as IContainer).contents;
        for i := 0 to Pred(contents.count) do
          final_declare(contents[i] as IContained);
      end;
  end;
end;

procedure TCodegen.final_declare_interface(int: IInterfaceDef);
var
  name : string;
begin
  if int.get_is_abstract then exit;
  if FDB.repoid_defined(int.id) then exit;
  name := get_name(IContained(int));
  if FParams.IndexOf('-poa') < 0 then
    writeln('  T'+name+'_impl = class(T'+name+'_skel)')
  else
    writeln('  T'+name+'_impl = class(T'+name+'_serv)');
  declare_all_methods2(int,' override;',false);
  writeln('  end;');
  writeln;
end;

procedure TCodegen.final_method(name: string; def: IOperationDef);
var
  mstr: string;
  ind: integer;
  res: IIDLType;
  i : integer;
begin
  mstr := operation_str(def);
  ind := Pos(' ',mstr);
  writeln(copy(mstr,1,ind)+'T'+name+'_impl.'+copy(mstr,ind+1,10000));
  writeln('begin');
  writeln('  // TODO');
  res := def.get_result_def;
  while res.def_kind = dk_alias do
    res := (res as IAliasDef).original_type_def;
  if not (res._type.kind in [tk_void,tk_struct,tk_array]) then
    writeln('  result := '+def_value(def.get_result_def)+';');
  for i := 0 to Pred(Length(def.exceptions^)) do
    begin
      writeln('  dorb_throw(T'+get_name(def.exceptions^[i])+'.Create() as IORBException);');
    end;
  writeln('end;');
  writeln;
end;

procedure TCodegen.final_attribute(name: string; attr: IAttributeDef);
var
  tstr: string;
begin
  tstr := type_str(attr.get_type_def);
  if attr.get_mode = ATTR_NORMAL then
    begin
      writeln('procedure T'+name+'_impl._set_'+attr.name+'(const val: '+tstr+');');
      writeln('begin');
      writeln('  // TODO');
      writeln('end;');
      writeln;
    end;
  writeln('function T'+name+'_impl._get_'+attr.name+': '+tstr+';');
  writeln('begin');
  writeln('  // TODO');
  writeln('  result := '+def_value(attr.get_type_def)+';');
  writeln('end;');
  writeln;
end;

procedure TCodegen.final_impl_interface(int: IInterfaceDef);
var
  name : string;
  base,methods,attributes : IInterfaceList;

  procedure final_methods(int: IInterfaceDef);
  var
    i : integer;
  begin
    base := int.get_base_interfaces;
    for i := 0 to Pred(base.count) do
      final_methods(int.get_base_interfaces[i] as IInterfaceDef);
    methods := (int as IContainer).contents(dk_Operation);
    attributes := (int as IContainer).contents(dk_Attribute);
    for i := 0 to Pred(methods.Count) do
      final_method(name,methods[i] as IOperationDef);
    for i := 0 to Pred(attributes.Count) do
      final_attribute(name,attributes[i] as IAttributeDef);
  end;

begin
  if int.get_is_abstract then exit;
  if FDB.repoid_defined(int.id) then exit;
  name := get_name(IContained(int));
  writeln('//***********************************************************');
  writeln('// '+name+'_impl');
  writeln('//***********************************************************');
  final_methods(int);
end;

procedure TCodegen.collect_constant(con: IConstantDef);
begin
  FConstants.add(con);
end;

procedure TCodegen.declare_constant(con: IConstantDef);
type
  ds = record
    case Boolean of
      true: (d: double );
      false: (arr: array[0..7] of byte);
  end;
  fs = record
    case Boolean of
      true: (f: float );
      false: (arr: array[0..3] of byte);
  end;
var
  name : string;
  val: Variant;
  ch: string;
  wch: widestring;
begin
  if FDB.repoid_defined(con.id) then Exit;
  name := get_name(con as IContained);
  val := con.value.get_variant;
  case con.type_def._type.kind of
    tk_string: writeln('  '+name+' = '''+val+''';');
    tk_wstring: writeln('  '+name+': WideString = '''+val+''';');
    tk_char: begin
        ch := val;
        if ord(ch[1]) >= 33 then
          writeln('  '+name+' = '''+ch+''';')
        else
          writeln('  '+name+' = #'+IntToStr(ord(char(ch[1])))+';')
      end;
    tk_wchar: begin
      wch := val;
      writeln('  '+name+': WideChar = #'+IntToStr(ord(wch[1]))+';');
    end;
    tk_short,tk_long,tk_ushort,tk_ulong: writeln('  '+name+' = '+IntToStr(val)+';');
    tk_float : writeln('  '+name+' = '+FloatToStrf(val,ffGeneral,7,0)+';');
    tk_double : writeln('  '+name+' = '+FloatToStr(val)+';');
    tk_boolean: if val = TRUE then
      writeln('  '+name+' = TRUE;')
    else
      writeln('  '+name+' = FALSE;');
    else
      assert(false,'not_implemented');
  end;
end;

function TCodegen.get_default_value_for_union(def: IIDLType;
  members: UnionMemberSeq): Variant;
var
  tc: ITypeCode;
  discr : IIDLType;
  enum: IEnumDef;
  i : integer;
  emembers: EnumMemberSeq;

  function in_use(val: Variant): Boolean;
  var
    j : integer;
  begin
    result := false;
    for j := 0 to Pred(Length(members)) do
      if (members[j]._label.get_type.kind <> tk_octet) and (members[j]._label.get_variant = val) then
        begin
          result := true;
          exit;
        end;
  end;
begin
  emembers := nil;
  tc := def._type;
  case tc.unalias.kind of
    tk_enum: begin
        discr := original_type(def);
        enum := TEnumDef._narrow(discr as IORBObject);
        assert(enum <> nil);
        emembers := enum.members^;
        for i := 0 to Pred(Length(emembers)) do
          if not in_use(i) then
            begin
              result := emembers[i];
              exit;
            end;
      end;
    tk_short,tk_long,tk_ushort,tk_ulong: begin
        for i := 0 to High(short) do
          if not in_use(i) then
            begin
              result := i;
              exit;
            end;
      end;
    tk_char : begin
        for i := 0 to High(Byte) do
          if not in_use(char(i)) then
            begin
              result := 'Char('+inttostr(i)+')';
              exit;
            end;
      end;
    else
      assert(false,'not implemented');
  end;
  raise Exception.Create('no values for default case');
end;

procedure TCodegen.union_dependencies(u: IUnionDef);
var
  i : integer;
  members : UnionMemberSeq;
begin
  members := u.members^;
  for i := 0 to Pred(Length(members)) do
    case members[i].type_def.def_kind of
      dk_Interface: add_forwards(members[i].type_def as IContained);
      dk_Struct,dk_Union,dk_Sequence,dk_Array :
        cont_dependencies(members[i].type_def);
    end;
  declare(get_name(u as IContained));
end;

procedure TCodegen.struct_dependencies(val: IStructDef);
var
  i : integer;
  members : StructMemberSeq;
begin
  members := val.members^;
  for i := 0 to Pred(Length(members)) do
    case members[i].type_def.def_kind of
      dk_Interface: add_forwards(members[i].type_def as IContained);
      dk_Struct,dk_Union,dk_Sequence,dk_Array :
        cont_dependencies(members[i].type_def);
    end;
  declare(get_name(val as IContained));
end;

procedure TCodegen.array_dependencies(a: IArrayDef);
begin
  with a.element_type_def do
    case def_kind of
      dk_Interface : add_forwards(a.element_type_def as IContained);
      dk_Struct : add_forwards(a.element_type_def as IContained);
    end;
end;

procedure TCodegen.seq_dependencies(s: ISequenceDef);
begin
  with s.element_type_def do
    case def_kind of
      dk_Interface : add_forwards(s.element_type_def as IContained);
      dk_Struct : add_forwards(s.element_type_def as IContained);
    end;
end;

procedure TCodegen.declare_forwards;
var
  i : integer;
begin
  for i := 0 to Pred(FForwards.count) do
    with IContained(FForwards[i]) do
      case def_kind of
        dk_Interface, dk_Union : begin writeln('  I'+get_name(IContained(FForwards[i]))+ ' = interface;'); writeln; end;
        //dk_Struct : begin writeln('  T'+get_name(IContained(FForwards[i]))+ ' = record;'); writeln; end;
      end;
end;

procedure TCodegen.attr_dependencies(attr: IAttributeDef);
var
  cont : IContained;
begin
  if attr._type.kind in [tk_objref,tk_struct,tk_union] then
    begin
      cont := TContained._narrow(attr.get_type_def as IORBObject);
      if (cont <> nil) and (FDeclared.indexOf(get_name(cont)) < 0) then
        add_forwards(cont);
    end;
end;

procedure TCodegen.InternalGenerate(const AName: string; const AContents: IInterfaceList);
begin
  try
    ProcessRmic('');
    if FIncludes.IndexOf(AName) <> -1 then
      FIncludes.Delete(FIncludes.IndexOf(AName));
    AssignFile(Output, AName+'_int.pas');
    Rewrite(Output);
    GenerateInterface(AName, AContents);
    CloseFile(Output);

    AssignFile(Output, AName+'.pas');
    Rewrite(Output);
    GenerateImplementation(AName, AContents);
    CloseFile(Output);
  except
    on e : Exception do begin
      CloseFile(Output);
      AssignFile(Output, '');
      write('error: ' + e.Message);
    end;
  end;
end;

procedure TCodegen.GenerateImplementation(const AName: string; const AContents: IInterfaceList);
var
  usesstr, temp: string;
  cnt, i: Integer;
  it : IIDLType;
begin
    // implementations
    write_header();
    //writeln('unit '+name+';'+char(CR));
    writeln('unit '+AName+';'); writeln;
    //writeln('interface'+ char(CR));
    writeln('interface'); writeln;
    if FIncludes.IndexOf('except') >= 0 then FIncludes.Delete(FIncludes.IndexOf('except'));
    if FParams.IndexOf('-poa') >= 0 then FIncludes.add('poa');
    writeln('uses');
    writeln('  Classes, orb_int, orb, req_int, code_int, imr, imr_int, env_int, stdstat, std_seq,');
    usesstr :='  orbtypes, exceptions, except_int, '+AName+'_int';
    if FIncludes.count > 0 then
      begin
        cnt := FIncludes.count-1;
        for i := 0 to cnt do
          begin
            if not already_included(FIncludes[i]) then begin
              if (length(usesstr) > 80) then
                begin
                  writeln(usesstr+',');
                  usesstr := '  '+FIncludes[i] + '_int';
                end
              else
                usesstr := usesstr + ', ' + FIncludes[i] + '_int';
              if (length(usesstr) > 80) and (i <> cnt) then
                begin
                  writeln(usesstr+',');
                  usesstr := '  '+FIncludes[i];
                end
              else
                usesstr := usesstr + ', ' + FIncludes[i];
            end;
            if i = cnt then usesstr := usesstr+';';
          end
      end
    else
      usesstr := usesstr + ';';
    writeln(usesstr);
    writeln;
    //writeln('type'+ char(CR));
    //if AContents.count > FConstants.Count then
      //writeln('type'); writeln;
    for i := 0 to AContents.count - 1 do
      if IContained(AContents[i]).def_kind <> dk_Constant then begin
        writeln('type'); writeln;
        Break;
      end;

    for i := 0 to Pred(AContents.count) do
      impl_declare(AContents[i] as IIRObject);

    if Marshallers.count > 0 then
      begin
        if FParams.IndexOf('-any') > 0 then
          for i := 0 to Pred(Marshallers.count) do
            begin
              if IIRObject(Pointer(Marshallers.Objects[i])).def_kind in [dk_Array,dk_Sequence] then
                temp := ''
              else
                temp := ': '+type_str(IIRObject(Pointer(Marshallers.Objects[i])));
              writeln('  function '+Marshallers[i]+'_to_any(const val'+temp+'): IAny;');
              writeln('  function any_to_'+Marshallers[i]+'(const a: IAny; var val'+temp+'): Boolean;');
              writeln;
            end;
        writeln('var');
        for i := 0 to Pred(Marshallers.count) do
          writeln('  '+ Marshallers[i]+ '_marshaller : IStaticTypeInfo;');
        for i := 0 to Pred(Marshallers.count) do
          writeln('  _tc_'+Marshallers[i]+' : ITypeCodeConst;');
        writeln;
      end;

    //writeln('implementation'+ char(CR));
    writeln('implementation'); writeln;
    writeln('uses');
    //writeln('  any,tcode,static,SysUtils,throw;'+ char(CR));
    writeln('  SysUtils, any, tcode, static, throw;'); writeln;

    //if (Marshallers.count > 0) and (FParams.IndexOf('-any') > 0) then
        //begin
          //writeln('var');
          //for i := 0 to Pred(Marshallers.count) do
            //writeln('  _tc_'+Marshallers[i]+' : ITypeCodeConst;');
          //writeln;
        //end;


    // implementation
    for i := 0 to Pred(AContents.count) do
      impl_code(AContents[i] as IIRObject);

    if Marshallers.count > 0 then
      begin
        if FParams.IndexOf('-any') > 0 then
          begin
            for i := 0 to Pred(Marshallers.count) do
              begin
                if IIRObject(Pointer(Marshallers.Objects[i])).def_kind in [dk_Array,dk_Sequence] then
                  temp := ''
                else
                  temp := ': '+type_str(IIRObject(Pointer(Marshallers.Objects[i])));
                writeln('function any_to_'+Marshallers[i]+'(const a: IAny; var val'+temp+'): Boolean;');
                writeln('var');
                writeln('  stat: IStaticAny;');
                writeln('begin');
                writeln('  stat := StaticAny('+Marshallers[i]+'_marshaller,@val);');
                writeln('  result := a.to_static_any(stat);');
                writeln('end;');
                writeln;
                writeln('function '+Marshallers[i]+'_to_any(const val'+temp+'): IAny;');
                writeln('var');
                writeln('  stat: IStaticAny;');
                writeln('begin');
                writeln('  stat := StaticAny('+Marshallers[i]+'_marshaller,@val);');
                writeln('  result := CreateAny;');
                writeln('  result.from_static_any(stat);');
                writeln('end;');
                writeln;
              end;
          end;
        writeln('initialization');
        for i := 0 to Pred(Marshallers.count) do
          writeln('  '+ Marshallers[i]+ '_marshaller := T'+ Marshallers[i]+ '_marshaller.Create();');
        //if FParams.IndexOf('-any') > 0 then
          for i := 0 to Pred(Marshallers.count) do
            begin
              it := TIDLType._narrow(IIRObject(Pointer(Marshallers.Objects[i])) as IORBObject);
              if it = nil then
                assert(false,'not implemented')
              else
                begin
                  write('  _tc_'+ Marshallers[i] + ' := CreateTypeCodeConst(');
                  temp := it._type.to_string;
                  while length(temp) > 120 do
                    begin
                      writeln(''''+copy(temp,1,110)+'''+');
                      write('    ');
                      temp := copy(temp,111,10000);
                    end;
                  writeln(''''+temp+''');');
                end;
            end;
      end;

    writeln('end.');
end;

procedure TCodegen.GenerateInterface(const AName: string; const AContents: IInterfaceList);
var
  usesstr: string;
  cnt, i: Integer;
begin
    write_header();
    //writeln('unit '+name+'_int;'+char(CR));
    writeln('unit '+AName+'_int;'); writeln;
    //writeln('{$MINENUMSIZE 4}'+char(CR));
    writeln('{$MINENUMSIZE 4}'); writeln;
    //writeln('interface'+ char(CR));
    writeln('interface'); writeln;
    writeln('uses');
    FDeclared.Clear;
    usesstr := '  code_int, orb_int, orbtypes';
    if FIncludes.count > 0 then
      begin
        cnt := FIncludes.count-1;
        for i := 0 to cnt do
          begin
            if not already_included(FIncludes[i]) then begin
              if (length(usesstr) > 80) {and (i <> cnt)} then
                begin
                  writeln(usesstr+',');
                  usesstr := '  '+FIncludes[i] + '_int';
                end
              else
                usesstr := usesstr + ', ' + FIncludes[i] + '_int';
            end;
            if i = cnt then usesstr := usesstr+';';
          end
      end
    else
      usesstr := usesstr + ';';
    writeln(usesstr);
    writeln;

    if FConstants.Count > 0 then
      begin
        i := 0;
        while i < FConstants.count do begin
          if not FDB.repoid_defined((FConstants[i] as IConstantDef).id) then Break;
          i := Succ(i);
        end; { while }
        if i < FConstants.count then
          //writeln('const'+ char(CR));
          writeln('const'); writeln;
        for i := 0 to Pred(FConstants.count) do
          declare_constant(FConstants[i] as IConstantDef);
        writeln;
      end;

    //writeln('type'+ char(CR));
    //if AContents.count > FConstants.Count then
      //writeln('type'); writeln;
    for i := 0 to AContents.count - 1 do
      if IContained(AContents[i]).def_kind <> dk_Constant then begin
        writeln('type'); writeln;
        Break;
      end;

    declare_forwards();

    for i := 0 to Pred(AContents.count) do
      int_declare(IContained(AContents[i]));

    //writeln('implementation'+ char(CR));
    writeln('implementation'); writeln;
    writeln('end.');
end;

var
  JavaClassList: TStrings;

procedure TCodegen.ProcessRmic(const RepoId: string);
var
  i, idx: Integer;
  b: Boolean;
begin
  for i := 0 to JavaClassList.Count - 1 do begin
    idx := FIncludes.IndexOf(JavaClassList[i]);
    b := idx <> -1;
    if b then begin
      FIncludes.Delete(idx);
      if FIncludes.IndexOf('EJB') = -1 then
        FIncludes.Add('EJB');
    end;
  end;
end;

initialization
  JavaClassList := TStringList.Create;
  JavaClassList.Add('Comparable_java_lang');
  JavaClassList.Add('Ex_java_lang');
  JavaClassList.Add('Exception_java_lang');
  JavaClassList.Add('Byte_java_lang');
  JavaClassList.Add('Short_java_lang');
  JavaClassList.Add('Integer_java_lang');
  JavaClassList.Add('Long_java_lang');
  JavaClassList.Add('Float_java_lang');
  JavaClassList.Add('Double_java_lang');
  JavaClassList.Add('Boolean_java_lang');
  JavaClassList.Add('Number_java_lang');
  JavaClassList.Add('Object_java_lang');
  JavaClassList.Add('StackTraceElement_java_lang');
  JavaClassList.Add('seq1_StackTraceElement_org_omg_boxedRMI_java_lang');  
  JavaClassList.Add('Throwable_java_lang');
  JavaClassList.Add('ThrowableEx_java_lang');
  JavaClassList.Add('CreateEx_javax_ejb');
  JavaClassList.Add('CreateException_javax_ejb');
  JavaClassList.Add('EJBHome_javax_ejb');
  JavaClassList.Add('EJBMetaData_javax_ejb');
  JavaClassList.Add('EJBObject_javax_ejb');
  JavaClassList.Add('Handle_javax_ejb');
  JavaClassList.Add('HomeHandle_javax_ejb');
  JavaClassList.Add('RemoveEx_javax_ejb');
  JavaClassList.Add('RemoveException_javax_ejb');
finalization
  JavaClassList.Free;
end.
