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
unit codebase;

interface

uses
  parser,classes,sysutils,ir_int,orbtypes,ir,code_int,{$IFDEF MSWINDOWS}comobj,{$ENDIF}
  codeutil,idldb;

type

  TExcept_declare_mode = (ed_all,ed_in,ed_out);

  TCodebase = class
  protected
    FDB: TDB;
    FIncludes : TStringList;
    FRepository: IRepository;
    FParams : TStrings;
    FDeclared : TStringList;
    FForwards : IInterfaceList;
    FImplemented : TStringList;
    Marshallers : TStringList;
  protected
    function get_name(cont : IContained) : string;
    function declare(name: string): Boolean;
    function add_marshaller(name: string; it: IIRObject): Boolean;
    function base_sequence_str(seq : ISequenceDef): string;
  public
    function attribute_str(attr: IAttributeDef; write: Boolean): string;
    procedure declare_marshaller(cont : IIDLType);
    function operation_str(op: IOperationDef): string;
    function param_str(p: ParameterDescription): string;
    function type_str(t: IIRObject; recursive: Boolean = false): string;
    function get_base_list(int: IInterfaceDef):IInterfaceList;
    procedure rget_base_list(int: IInterfaceDef; ilist: IInterfaceList);
    function prepare_int_list(int: IInterfaceDef; except_first: Boolean = false) : IInterfaceList;
    procedure declare_all_methods2(int: IInterfaceDef; suffix: string = ''; except_first: Boolean = false);
    procedure declare_all_methods(int: IInterfaceDef; suffix: string = ''; except_first: Boolean = false);
    procedure declare_attributes(int: IInterfaceDef; suffix: string = ''; write_properties: Boolean = false);
    procedure declare_methods(int: IInterfaceDef; suffix: string = '');
    procedure stub_attribute(name: string;attr: IAttributeDef; for_abstract: Boolean);
    procedure stub_method(name: string;op: IOperationDef; for_abstract: Boolean);
    procedure stub_clp_attribute(name: string;attr: IAttributeDef);
    procedure stub_clp_attribute2(name, basename: string; attr: IAttributeDef);
    procedure stub_clp_method(name, basename: string;op: IOperationDef);
    function marshaller_for(it: IIDLType): string;
    procedure method_dispatch(cname: string; int: IInterfaceDef; passenv: Boolean);
    procedure method_vars(op: IOperationDef; var firstcall: Boolean);
    procedure attr_method_vars(attr: IAttributeDef; var firstcall: Boolean);
    procedure method_call(op: IOperationDef; var firstcall: Boolean);
    procedure attr_method_call(attr: IAttributeDef; var firstcall: Boolean);
    procedure int_marshaller(int: IInterfaceDef);
    function def_value(it: IIDLType): string;
    procedure struct_marshaller(cont : IContained);
    procedure enum_marshaller(cont : IContained);
    procedure value_marshaller(cont : IContained);
    procedure seq_marshaller(seq : ISequenceDef);
    procedure array_marshaller(arr : IArrayDef);
    procedure alias_marshaller(al : IAliasDef);
    procedure union_marshaller(u : IUnionDef);
    function is_base_sequence(seq : ISequenceDef): Boolean;
    function initializer_decl(init: Initializer; target: IValueDef): string;
    procedure value_member(vmd: IValueMemberDef; all: boolean);
    procedure marshaller_decl(name : string; it: IIRObject);
    function marshaller_for_array(arr: IArrayDef): string;
    function marshaller_for_seq(s: ISequenceDef): string;
    function array_str(arr: IArrayDef): string;
    procedure ex_members_decl(ename: string; members: StructMemberSeq; mode : TExcept_declare_mode = ed_all);
    procedure except_marshaller(ex : IExceptionDef);
    function expand_name(t: IIDLType; recursive: Boolean = false): string;
    procedure add_forwards(int: IContained);
    procedure common_servant_methods_declaration;
    procedure serv_method(name: string; op: IOperationDef; const intf: string = '');
    procedure serv_attribute(name: string; attr: IAttributeDef; const intf: string = '');
  public
    class function original_type(t: IIDLType): IIDLType;
  end;



implementation

uses
  scanner,orb_int;

//*********************************************************************
//  TCodebase
//*********************************************************************
function TCodebase.get_name(cont : IContained) : string;
begin
  if (FParams.IndexOf('-without-prefix') >= 0) or name_without_prefix(cont) then
    Result := cont.name
  else
    Result := cont._absolute_name;
end;

function TCodebase.attribute_str(attr: IAttributeDef; write: Boolean): string;
var
  tstr,name: string;
begin
  tstr := type_str(attr.get_type_def);
  name := attr.name;
  if write then
    result := 'procedure _set_'+name+'(const val: '+tstr+');'
  else
    result := 'function _get_'+name+': '+tstr+';'
end;

function TCodebase.operation_str(op: IOperationDef): string;
var
  params : ParDescriptionSeq;
  i : integer;
begin
  if op._result.kind = tk_void then
    result := 'procedure '
  else
    result := 'function ';
  result := result + safe_name(op.name);
  params := op.get_params;
  if Length(params) > 0 then
    begin
      result := result + '(';
      for i := 0 to Pred(Length(params)) do
        begin
          if i > 0 then Result := Result + '; ';
          Result := Result + param_str(params[i]);
        end;
      result := result + ')';
    end;
  if op._result.kind <> tk_void then
    result := result + ': '+type_str(op.get_result_def);
  result := result + ';';
end;

function TCodebase.param_str(p: ParameterDescription): string;
begin
  result := safe_name(p.name) + ': ' + type_str(p.type_def);
  if p.mode = PARAM_OUT then
    result := 'out '+ result
  else if p.mode = PARAM_INOUT then
    result := 'var '+ result
  else
    result := 'const '+ result;
end;

function TCodebase.type_str(t: IIRObject; recursive: Boolean): string;
var
  tname : string;
  bound : longint;
begin
  case t.def_kind of
    dk_Enum,dk_alias,dk_Struct : begin
        if recursive then
          result := safe_name(get_name(t as IContained))
        else
          result := 'T'+safe_name(get_name(t as IContained));
      end;
    dk_Array : result := expand_name(t as IIDLType);
    dk_Sequence : begin
      result := safe_name(get_name(t as IContained));
      if result <> '' then
        result := 'T' + result
      else
        begin
          tname := type_str((t as ISequenceDef).element_type_def);
          bound := (t as ISequenceDef).bound;
          if bound = 0 then
            result := 'array of '+tname
          else
            result := 'array [0..'+inttostr(bound-1)+'] of '+tname
        end;
    end;
    dk_Interface, dk_Union, dk_Value, dk_ValueBox, dk_Exception : if recursive then
        result := get_name(t as IContained)
      else
        result := 'I'+get_name(t as IContained);
    dk_Native: result := (t as IContained).name;
    dk_Primitive :
      case (t as IIDLType)._type.kind of
        tk_long: result := 'long';
        tk_longlong: result := 'longlong';
        tk_short : result := 'short';
        tk_ushort : result := '_ushort';
        tk_ulong : result := '_ulong';
        tk_ulonglong : result := '_ulonglong';
        tk_float : result := 'float';
        tk_double : result := 'double';
        tk_boolean : result := 'boolean';
        tk_char : result := 'char';
        tk_wchar : result := 'widechar';
        tk_octet : result := 'octet';
        tk_any : result := 'IAny';
        tk_TypeCode : result := 'ITypeCode';
        tk_string : result := 'string';
        tk_wstring : result := 'widestring';
        tk_objref : result := 'IORBObject';
        tk_Principal : result := 'IPrincipal';
        else
          assert(false,'not imlplemented');
      end;
    else
      assert(false,'not imlplemented');
  end;
end;

procedure TCodebase.declare_all_methods(int: IInterfaceDef; suffix: string = ''; except_first: Boolean = false);
var
  i,n : integer;
  base: IInterfaceList;
  tmp: IInterfaceDef;
begin
  base := int.get_base_interfaces;
  if except_first then n := 1 else n := 0;
  for i := n to Pred(base.count) do
    begin
      tmp := base[i] as IInterfaceDef;
      writeln(Format('{ %s }', [get_name(tmp as IContained)]));
      declare_all_methods(tmp,suffix, except_first or (i = n));
    end;
  declare_methods(int,suffix);
  declare_attributes(int,suffix);
end;

function TCodebase.get_base_list(int: IInterfaceDef):IInterfaceList;
begin
  result := int.get_base_interfaces;
end;

procedure TCodebase.rget_base_list(int: IInterfaceDef; ilist: IInterfaceList);
var
  i: integer;
  base : IInterfaceList;
begin
  ilist.Add(int);
  base := get_base_list(int);
  for i:=0 to Pred(base.count) do begin
    rget_base_list(base[i] as IInterfaceDef, ilist);
  end;
end;

function TCodebase.prepare_int_list(int: IInterfaceDef; except_first: Boolean = false) : IInterfaceList;
var
  i,n,j, bs : integer;
  base: IInterfaceList;
  first_base : IInterfaceList;
  all : IInterfaceList;
  tmp_list : IInterfaceList;
  tmp: IInterfaceDef;
begin
  all := TInterfaceList.Create;
  base := get_base_list(int);
  bs := base.count;
  if bs > 0 then begin
    first_base := TInterfaceList.Create;
    rget_base_list(base[0] as IInterfaceDef, first_base);
    if except_first then n := 1 else n := 0;
    for i := n to Pred(bs) do begin
      tmp := base[i] as IInterfaceDef;
      tmp_list := TInterfaceList.Create;
      rget_base_list(tmp, tmp_list);
      for j:=0 to Pred(tmp_list.count) do begin
        if (all.IndexOf(tmp_list[j]) < 0) and (first_base.IndexOf(tmp_list[j]) < 0) then begin
          all.Add(tmp_list[j]);
        end;
      end;
    end;
    if not except_first then begin
      for i:=0 to Pred(first_base.count) do
          all.Add(first_base[i]);
    end;
  end;
  result := all;
end;

procedure TCodebase.declare_all_methods2(int: IInterfaceDef; suffix: string = ''; except_first: Boolean = false);
var
  i: integer;
  all : IInterfaceList;
begin
  all := prepare_int_list(int, except_first);

  for i:=0 to Pred(all.count) do begin
    declare_methods(all[i] as IInterfaceDef,suffix);
    declare_attributes(all[i] as IInterfaceDef,suffix);
  end;

  declare_methods(int,suffix);
  declare_attributes(int,suffix);
end;

procedure TCodebase.declare_attributes(int: IInterfaceDef; suffix: string; write_properties: Boolean);
var
  attributes : IInterfaceList;
  i : integer;
  attr : IAttributeDef;
  aname: string;
begin
  attributes := (int as IContainer).contents(dk_Attribute);
  for i := 0 to Pred(attributes.count) do
    begin
      attr:= attributes[i] as IAttributeDef;
      if attr.get_mode = ATTR_NORMAL then
        writeln('    '+attribute_str(attr,true)+suffix);
      writeln('    '+attribute_str(attr,false)+suffix);
    end;
  if write_properties then
    for i := 0 to Pred(attributes.count) do
      begin
        attr:= attributes[i] as IAttributeDef;
        aname := attr.name;
        write('    property '+safe_name(aname)+': '+ type_str(attr.get_type_def)+' read _get_'+aname);
        if attr.get_mode = ATTR_NORMAL then
          writeln(' write _set_'+aname+';')
        else
          writeln(';');
      end;
end;

procedure TCodebase.declare_methods(int: IInterfaceDef; suffix: string);
var
  methods: IInterfaceList;
  i : integer;
begin
  methods := (int as IContainer).contents(dk_Operation);
  for i := 0 to Pred(methods.count) do
    writeln('    '+operation_str((methods[i] as IOperationDef))+suffix);
end;

procedure TCodebase.method_call(op: IOperationDef; var firstcall: Boolean);
var
  name,shift,ename,aname: string;
  params : ParDescriptionSeq;
  param: ParameterDescription;
  i,rlen: integer;
begin
  name := op.name;
  if firstcall then
    writeln('  if req.op_name = '''+name+''' then')
  else
    writeln('  else if req.op_name = '''+name+''' then');
  writeln('    begin');
  params := op.get_params;
  for i := 0 to Pred(Length(params)) do
    begin
      param := params[i];
      case param.mode of
        PARAM_IN: System.write('      req.add_in_arg(');
        PARAM_OUT: System.write('      req.add_out_arg(');
        PARAM_INOUT: System.write('      req.add_inout_arg(');
      end;
      writeln('StaticAny('+marshaller_for(param.type_def)+',@'+safe_name(name)+'_'+param.name+') as IStaticAny);');
    end;
  if op.get_result_def._type.kind <> tk_void  then
    writeln('      req.set_result(StaticAny('+marshaller_for(op.get_result_def)+',@'+safe_name(name)+'_result) as IStaticAny);');
  writeln('      if not req.read_args() then  exit;');
  shift := '';
  rlen := Length(op.exceptions^);
  if rlen > 0 then
    begin
      write('      req.set_exceptions_tc([');
      for i := 0 to Pred(rlen) do begin
        if i > 0 then write(', ');
        ename := get_name(op.exceptions^[i]);
        write('_tc_'+ename+'.typecode');
      end;
      writeln(']);');
      writeln('      try');
      shift := '  ';
    end;
  if op.get_result_def._type.kind <> tk_void  then
    write(shift+'      '+safe_name(name)+'_result := ')
  else
    write(shift+'      ');
  write(safe_name(name));
  if Length(params) > 0 then
    begin
      write('(');
      for i := 0 to Pred(Length(params)) do
        begin
          if i <> 0 then write(', ');
          param := params[i];
          write(safe_name(name)+'_'+param.name);
        end;
      write(')');
    end;
  writeln(';');
  if rlen > 0 then
    begin
      writeln('      except');
      for i := 0 to Pred(rlen) do
        begin
          ename := op.exceptions^[i].name;
          aname := 'T'+get_name(op.exceptions^[i]);
          writeln('        on '+ ename + ': '+aname+' do');
          writeln('          req.set_exception('+aname+'.Create('+ename+') as IORBException);');
        end;
      writeln('      end;');
    end;
  writeln('      req.write_results();');
  writeln('      exit;');
  write('    end');
  firstcall := false;
end;

procedure TCodebase.method_vars(op: IOperationDef; var firstcall: Boolean);
var
  name: string;
  pdesc: ParameterDescription;
  params: ParDescriptionSeq;
  i : integer;
begin
  name := safe_name(op.name);
  if op.get_result_def._type.kind <> orbtypes.tk_void then
    begin
      if firstcall then
        begin
          firstcall := false;
          writeln('var');
        end;
      writeln('  '+name+'_result: '+type_str(op.get_result_def)+';');
    end;
  params := op.get_params;
  for i := 0 to Pred(Length(params)) do
    begin
      pdesc := params[i];
      if firstcall then
        begin
          firstcall := false;
          writeln('var');
        end;
      writeln('  '+name+'_'+pdesc.name+': '+type_str(pdesc.type_def)+';');
    end;
end;

procedure TCodebase.stub_attribute(name: string;attr: IAttributeDef; for_abstract: Boolean);
var
  stub_name, attrname, tname: string;
begin
  attrname := attr.name;
  stub_name := 'T' + name+ '_stub';
  tname := type_str(attr.get_type_def);
  if attr.get_mode = ATTR_NORMAL then
    begin
      writeln('procedure '+stub_name+'._set_'+attrname+'(const val: '+tname+');');
      writeln('var');
      writeln('  req: IStaticRequest;');
      writeln('  _val: IStaticAny;');
      writeln('begin');
      writeln('  _val := StaticAny('+
         marshaller_for(attr.get_type_def)+
         ',@val);');
      writeln('  req := StaticRequest(self,''_set_'+attrname+''');');
      writeln('  req.add_in_arg(_val);');
      writeln('  req.invoke;');
      writeln('  dorb_static_throw(req);');
      writeln('end;');
      writeln;
    end;
  writeln('function '+stub_name+'._get_'+attrname+': '+tname+';');
  writeln('var');
  writeln('  req: IStaticRequest;');
  writeln('  _result: IStaticAny;');
  writeln('begin');
  writeln('  _result := StaticAny('+
     marshaller_for(attr.get_type_def)+', @result);');
  if for_abstract then
    writeln('  req := StaticRequest(self._to_object,''_get_'+attrname+''');')
  else
    writeln('  req := StaticRequest(self,''_get_'+attrname+''');');
  writeln('  req.set_result(_result);');
  writeln('  req.invoke;');
  writeln('  dorb_static_throw(req);');
  //writeln('  result := '+tname+'(_result.value^);');
  writeln('end;');
  writeln;
end;

procedure TCodebase.stub_method(name: string; op: IOperationDef; for_abstract: Boolean);
var
  stub_name,ename, operation: string;
  i,j,len : integer;
  params: ParDescriptionSeq;
  res: IIDLType;
  ex : IExceptionDef;
begin
  operation := operation_str(op);
  stub_name := 'T' + name+ '_stub';
  j := pos(' ',operation);
  writeln(copy(operation,1,j)+stub_name+'.'+copy(operation,j+1,10000));
  writeln('var');
  writeln('  req: IStaticRequest;');
  params := op.get_params;
  for i := 0 to Pred(Length(params)) do
    writeln('  _'+safe_name(params[i].name)+': IStaticAny;');
  res := op.get_result_def;
  if res._type.kind <> orbtypes.tk_void then
    writeln('  _result: IStaticAny;');
  writeln('begin');
  for i := 0 to Pred(Length(params)) do
    writeln('  _'+safe_name(params[i].name)+' := StaticAny('+
      marshaller_for(params[i].type_def)+
      ',@'+safe_name(params[i].name)+');');
  if res._type.kind <> orbtypes.tk_void then
    writeln('  _result := StaticAny('+
      marshaller_for(res)+', @result);');
  if for_abstract then
    writeln('  req := StaticRequest(self._to_object,'''+op.name+''');')
  else
    writeln('  req := StaticRequest(self,'''+op.name+''');');
  for i := 0 to Pred(Length(params)) do
    case params[i].mode of
      PARAM_IN: writeln('  req.add_in_arg(_'+safe_name(params[i].name)+');');
      PARAM_OUT: writeln('  req.add_out_arg(_'+safe_name(params[i].name)+');');
      PARAM_INOUT: writeln('  req.add_inout_arg(_'+safe_name(params[i].name)+');');
    end;
  if res._type.kind <> orbtypes.tk_void then
    writeln('  req.set_result(_result);');
  len := Length(op.exceptions^);
  if len > 0 then begin
    write('  req.set_exceptions_tc([');
    for i := 0 to Pred(len) do begin
      if i > 0 then write(', ');
      ex := op.exceptions^[i];
      ename := get_name(ex as IContained);
      write('_tc_'+ename+'.typecode');
    end;
    writeln(']);');
  end;
  if op.get_mode = OP_ONEWAY then
    writeln('  req.oneway;')
  else
    writeln('  req.invoke;');
  write('  dorb_static_throw(req');
  for i := 0 to Pred(len) do
    begin
      if i > 0 then write(', ')
      else write(',[');
      ex := op.exceptions^[i];
      ename := get_name(ex as IContained);
      write(ename+'_marshaller, '''+ex.id+'''');
      if i = len - 1 then write(']');
    end;
  writeln(');');
  //if res._type.kind <> orbtypes.tk_void then
    //writeln('  result := '+type_str(res)+'(_result.value^);');
  writeln('end;');
  writeln;
end;

function TCodebase.marshaller_for(it: IIDLType): string;
var
  idef: IInterfaceDef;
  seq: ISequenceDef;
  tc: ITypeCode;
begin
  case it._type.kind of
    tk_long : result := 'stat_long';
    tk_longlong : result := 'stat_longlong';
    tk_enum: result := get_name(it as IContained) + '_marshaller';
    tk_ulong : result := 'stat_ulong';
    tk_ulonglong : result := 'stat_ulonglong';
    tk_octet : result := 'stat_octet';
    tk_char : result := 'stat_char';
    tk_wchar : result := 'stat_widechar';
    tk_any: result := 'stat_any';
    tk_short: result := 'stat_short';
    tk_float : result := 'stat_float';
    tk_double : result := 'stat_double';
    tk_ushort: result := 'stat_ushort';
    tk_TypeCode : result := 'stat_TypeCode';
    tk_string : result := 'stat_string';
    tk_wstring : result := 'stat_widestring';
    tk_boolean : result := 'stat_boolean';
    tk_alias :  result := marshaller_for(original_type(it));
    tk_sequence : begin
         seq := it as ISequenceDef;
         if is_base_sequence(seq) then
           begin
             tc := seq._type.content_type.unalias;
             case tc.kind of
               tk_boolean: result := 'stat_seq_boolean';
               tk_char : result := 'stat_seq_char';
               tk_wchar : result := 'stat_seq_wchar';
               tk_octet : result := 'stat_seq_octet';
               tk_short : result := 'stat_seq_short';
               tk_long : result := 'stat_seq_long';
               tk_ushort : result := 'stat_seq_ushort';
               tk_ulong : result := 'stat_seq_ulong';
               tk_longlong : result := 'stat_seq_longlong';
               tk_ulonglong : result := 'stat_seq_ulongulong';
               tk_float : result := 'stat_seq_float';
               tk_double : result := 'stat_seq_double';
               tk_longdouble : result := 'stat_seq_longdouble';
               tk_string : result := 'stat_seq_string';
               tk_wstring : result := 'stat_seq_wstring';
               tk_fixed : result := 'stat_seq_fixed';
               tk_objref : result := 'stat_seq_object';
               tk_any : result := 'stat_seq_any';
               else
                 assert(false,'unknown base type for sequence');
             end;
           end
         else
           begin
             result := expand_name(seq);
             result := result + '_marshaller';
           end;
      end;
    tk_array : result := marshaller_for_array(it as IArrayDef)+'_marshaller';
    tk_struct,
    tk_value,
    tk_value_box,
    tk_union : result := get_name(it as IContained) + '_marshaller';
    tk_objref : begin
        result := 'stat_object';
        idef := TInterfaceDef._narrow(it as IORBObject);
        if idef <> nil then
           result := get_name(idef) + '_marshaller';
      end;
    tk_Principal : begin
        result := 'stat_principal';
        idef := TInterfaceDef._narrow(it as IORBObject);
        if idef <> nil then
           result := get_name(idef) + '_marshaller';
      end;
    else
      assert(false,'not implemented');
  end;
end;

procedure TCodebase.int_marshaller(int: IInterfaceDef);
var
  name,mname,iname: string;
begin
  if FDB.repoid_defined(int.id) then exit;
  name := get_name(int);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  iname := 'I'+name;
  mname := 'T'+name+'_marshaller';
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  if int.get_is_abstract then
    begin
      writeln('var');
      writeln('  ab: IAbstractBase;');
      writeln('begin');
      writeln('  result := false;');
      writeln('  if not TAbstractBase._demarshal(dec,ab) then exit;');
      writeln('  '+iname+'(addr^) := T'+name+'._narrow(ab);');
      writeln('  result := (ab = nil) or (Pointer(addr^) <> nil);');
    end
  else
    begin
      writeln('var');
      writeln('  obj: IORBObject;');
      writeln('begin');
      writeln('  result := false;');
      writeln('  if not stat_Object.demarshal(dec,@obj) then exit;');
      writeln('  '+iname+'(addr^) := T'+name+'._narrow(obj);');
      writeln('  result := (obj = nil) or (Pointer(addr^) <> nil);');
    end;
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  if int.get_is_abstract then
    begin
      writeln('begin');
      writeln('  TAbstractBase._marshal(enc,'+iname+'(addr^) as IAbstractBase);');
    end
  else
    begin
      writeln('var');
      writeln('  obj: IORBObject;');
      writeln('begin');
      writeln('  obj := '+iname+'(addr^) as IORBObject;');
      writeln('  stat_Object.marshal(enc,@Pointer(obj));');
    end;
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  writeln('begin');
  writeln('  '+iname+'(addr^) := nil;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  '+iname+'(addr^) := T'+name+'_stub.Create();');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  '+iname+'(dst^) := '+iname+'(src^);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

function TCodebase.def_value(it: IIDLType): string;
begin
  case it.def_kind of
    dk_Interface, dk_Value, dk_ValueBox : result := 'nil';
    dk_Alias : result := def_value(original_type(it));
    dk_sequence : result := 'nil';
    //dk_Enum : result := type_str(it)+'(0)';
    dk_Enum : result := (it as IEnumDef).members^[0];
    dk_struct : begin
        result := '';
        //assert(false,'no default value for struct');
      end;
    dk_Primitive :
      case it._type.kind of
        tk_long,tk_short,tk_ushort,
        tk_longlong, tk_ulonglong,
        tk_ulong,tk_float,tk_double : result := '0';
        tk_boolean : result := 'false';
        tk_char : result := '#0';
        tk_wchar : result := '#0';
        tk_octet : result := '0';
        tk_any : result := 'nil';
        tk_string : result := '''''';
        tk_wstring : result := '''''';
        tk_objref : result := 'nil';
        tk_Principal : result := 'nil';
        tk_TypeCode : result := 'nil';
        else
          assert(false,'not imlplemented');
      end;
    else
      assert(false,'not imlplemented');
  end;
end;

procedure TCodebase.marshaller_decl(name : string; it: IIRObject);
begin
  if not declare(name+'_marshaller') then exit;
  if not add_marshaller(name,it) then exit;
  writeln('  T'+ name + '_marshaller = class(TAbstractStat,IStaticTypeInfo)');
  writeln('  protected');
  writeln('    procedure _assign(dst,src: Pointer); override;');
  writeln('    function demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  writeln('    procedure marshal(enc: IEncoder; const addr : Pointer);');
  writeln('    procedure _create(var addr: Pointer); override;');
  writeln('    procedure _free(var addr: Pointer);');
  //if FParams.IndexOf('-any') > 0 then
    writeln('    function typecode: ITypeCode; override;');
  writeln('  end;');
  writeln;
end;

procedure TCodebase.declare_marshaller(cont : IIDLType);
var
  name: string;
  t : IIDLType;
begin
  t := original_type(cont);
  case t.def_kind of
    dk_Primitive: exit;
    dk_Sequence: if is_base_sequence(t as ISequenceDef) then
      exit
    else
      name := marshaller_for_seq(t as ISequenceDef);
    dk_Array : name := marshaller_for_array(t as IArrayDef);
    else
      name := get_name(t as IContained);
  end;
  marshaller_decl(name,t);
end;

procedure TCodebase.except_marshaller(ex : IExceptionDef);
var
  id,name,mname,tstr: string;
  members: StructMemberSeq;
  member : StructMember;
  i, mlen : integer;
begin
  members := nil;
  if FDB.repoid_defined(ex.id) then exit;
  name := get_name(ex);
  id := ex.id;
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  mname := 'T'+name+'_marshaller';
  members := ex.members^;
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  mlen := length(members);
  writeln('var');
  writeln('  repoid: string;');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : tstr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : tstr := 'T'+name + '_'+member.name + '_array';
        else
          tstr := type_str(member.type_def)
      end;
      writeln('  _'+members[i].name+ ': '+ tstr+';');
    end;
  writeln('begin');
  writeln('  result := false;');
  writeln('  if not dec.except_begin(repoid) then exit;');
  for i := 0 to Pred(mlen) do
    begin
      writeln('  if not '+marshaller_for(members[i].type_def)+'.demarshal(dec,@_'+safe_name(members[i].name)+') then exit;');
      writeln('  I'+name+'(addr^).'+members[i].name+'(_'+members[i].name+');');
    end;
  writeln('  if not dec.except_end then exit;');
  writeln('  result := true;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  if mlen > 0 then writeln('var');
  for i := 0 to Pred(mlen) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : tstr := 'T'+name + '_'+member.name + '_seq';
        dk_Array : tstr := 'T'+name + '_'+member.name + '_array';
        else
          tstr := type_str(member.type_def)
      end;
      writeln('  _'+members[i].name+ ': '+ tstr+';');
    end;
  writeln('begin');
  writeln('  enc.except_begin('''+id+''');');
  for i := 0 to Pred(length(members)) do
    begin
      writeln('  _'+members[i].name+':= I'+name+'(addr^).'+members[i].name+';');
      writeln('  '+marshaller_for(members[i].type_def)+'.marshal(enc,@_'+members[i].name+');');
    end;
  writeln('  enc.except_end;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(addr^) := nil;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(addr^) := T'+name+'.Create();');
  writeln('end;');
  writeln;
  // _assign
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(dst^) := I'+name+'(src^);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

procedure TCodebase.struct_marshaller(cont : IContained);
var
  name,mname: string;
  members: StructMemberSeq;
  i : integer;
  sdef: IStructDef;
begin
  members := nil;
  if FDB.repoid_defined(cont.id) then exit;
  name := get_name(cont);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  mname := 'T'+name+'_marshaller';
  sdef := cont as IStructDef;
  members := sdef.members^;
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  writeln('var');
  writeln('  struct: T'+safe_name(name)+';');
  writeln('begin');
  writeln('  result := false;');
  for i := 0 to Pred(length(members)) do
    writeln('  if not '+marshaller_for(sdef.members^[i].type_def)+'.demarshal(dec,@struct.'+safe_name(sdef.members^[i].name)+') then exit;');
  writeln('  T'+safe_name(name)+'(addr^) := struct;');
  writeln('  result := true;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  writeln('var');
  writeln('  struct: T'+safe_name(name)+';');
  writeln('begin');
  writeln('  struct := T'+safe_name(name)+'(addr^);');
  for i := 0 to Pred(length(members)) do
    writeln('  '+marshaller_for(sdef.members^[i].type_def)+'.marshal(enc,@struct.'+safe_name(sdef.members^[i].name)+');');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  writeln('begin');
  writeln('{$HINTS OFF}'); //some records do not need Finalize
  writeln('  System.Finalize(T' + safe_name(name) + '(addr^));');
  writeln('{$HINTS ON}');
  writeln('  FreeMem(addr);');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  addr := AllocMem(sizeOf(T'+safe_name(name)+'));');
  writeln('{$HINTS OFF}'); //some records do not need Initialize
  writeln('  System.Initialize(T' + safe_name(name) + '(addr^));');
  writeln('{$HINTS ON}');
  writeln('end;');
  writeln;
  // _assign
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  T'+safe_name(name)+'(dst^) := T'+safe_name(name)+'(src^);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

procedure TCodebase.union_marshaller(u : IUnionDef);
var
  i,ind,cnt : integer;
  name,mname,tmp : string;
  dname : string;
  members : UnionMemberSeq;
  member : UnionMember;
  emembers : EnumMemberSeq;
  enum : IEnumDef;
  discr: IIDLType;
begin
  members := nil;
  emembers := nil;
  if FDB.repoid_defined((u as IContained).id) then exit;
  name := get_name(u as IContained);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  mname := 'T'+name+'_marshaller';
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  writeln('var');
  writeln('  discr: '+type_str(u.discriminator_type_def)+';');
  members := u.members^;
  cnt := Length(members);
  discr := original_type(u.discriminator_type_def);
  if discr.def_kind = dk_Enum then
    begin
      enum := TEnumDef._narrow(discr as IORBObject);
      assert(enum <> nil);
      emembers := enum.members^;
    end
  else
    emembers := nil;
  i := 0;
  while i < cnt do
    begin
      member := members[i];
      if member.type_def.def_kind = dk_Array then
        writeln('  _'+member.name+' : T'+name+'_'+member.name+'_array;')
      else if member.type_def.def_kind = dk_Sequence then
        writeln('  _'+member.name+' : T'+name+'_'+member.name+'_seq;')
      else
        writeln('  _'+member.name+' : '+type_str(member.type_def)+';');
      while (i+1 < cnt) and (members[i+1].name = member.name) do inc(i);
      inc(i);
    end;
  writeln('begin');
  writeln('  result := false;');
  dname := marshaller_for(u.discriminator_type_def);
  writeln('  if not '+dname+'.demarshal(dec,@discr) then exit;');
  writeln('  I'+name+'(addr^) := T'+name+'.Create;');
  writeln('  case discr of');
  i := 0;
  while i < cnt do
    begin
      member := members[i];
      // default
      if member._label.get_type.kind = tk_octet then
        begin
          writeln('    else begin');
          writeln('        if not '+marshaller_for(members[i].type_def)+'.demarshal(dec,@_'+members[i].name+') then exit;');
          writeln('        I'+name+'(addr^).set_'+members[i].name+'(_'+members[i].name+');');
          writeln('      end;');
        end
      else
        begin
          if discr.def_kind = dk_Enum then
            begin
              ind := member._label.get_variant;
              assert(ind < Length(emembers));
              tmp := emembers[ind];
            end
          else if discr._type.kind = tk_char then
            tmp := '''' + member._label.get_variant+ ''''
          else
            tmp := member._label.get_variant;
          write('    '+tmp);
          while (i+1 < cnt) and (members[i+1].name = member.name) do
            begin
              tmp := members[i+1]._label.get_variant;
              write(', '+ tmp);
              inc(i);
            end;
          writeln(' : begin');
          writeln('        if not '+marshaller_for(members[i].type_def)+'.demarshal(dec,@_'+members[i].name+') then exit;');
          writeln('        I'+name+'(addr^).set_'+members[i].name+'(_'+members[i].name+');');
          writeln('      end;');
        end;
      inc(i);
    end;
  writeln('  end;');
  writeln('  result := true;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  writeln('var');
  writeln('  discr: '+type_str(u.discriminator_type_def)+';');
  i := 0;
  while i < cnt do
    begin
      member := members[i];
      if member.type_def.def_kind = dk_Array then
        writeln('  _'+member.name+' : T'+name+'_'+member.name+'_array;')
      else if member.type_def.def_kind = dk_Sequence then
        writeln('  _'+member.name+' : T'+name+'_'+member.name+'_seq;')
      else
        writeln('  _'+member.name+' : '+type_str(member.type_def)+';');
      while (i+1 < cnt) and (members[i+1].name = member.name) do inc(i);
      inc(i);
    end;
  writeln('begin');
  writeln('  discr := I'+name+'(addr^).discriminator;');
  dname := marshaller_for(u.discriminator_type_def);
  writeln('  ' + dname + '.marshal(enc, @discr);');
  writeln('  case discr of');
  i := 0;
  while i < cnt do
    begin
      member := members[i];
      // default
      if member._label.get_type.kind = tk_octet then
        begin
          writeln('    else begin');
          writeln('      _'+members[i].name+' := I'+name+'(addr^).get_'+members[i].name+';');
          writeln('      '+marshaller_for(members[i].type_def)+'.marshal(enc,@_'+members[i].name+');');
          writeln('    end;');
        end
      else
        begin
          if discr.def_kind = dk_Enum then
            begin
              ind := member._label.get_variant;
              assert(ind < Length(emembers));
              tmp := emembers[ind];
            end
          else if discr._type.kind = tk_char then
            tmp := '''' + member._label.get_variant+ ''''
          else
            tmp := member._label.get_variant;
          write('    '+tmp);
          while (i+1 < cnt) and (members[i+1].name = member.name) do
            begin
              tmp := members[i+1]._label.get_variant;
              write(', '+ tmp);
              inc(i);
            end;
          writeln(' : begin');
          writeln('      _'+members[i].name+' := I'+name+'(addr^).get_'+members[i].name+';');
          writeln('      '+marshaller_for(members[i].type_def)+'.marshal(enc,@_'+members[i].name+');');
          writeln('    end;');
        end;
      inc(i);
    end;
  writeln('  end;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(addr^) := nil;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(addr^) := T'+name+'.Create();');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(dst^) := I'+name+'(src^);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

procedure TCodebase.enum_marshaller(cont : IContained);
var
  name,mname: string;
begin
  if FDB.repoid_defined(cont.id) then exit;
  name := get_name(cont);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  mname := 'T'+name+'_marshaller';
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  writeln('begin');
  writeln('  result := dec.get_ulong(_ulong(addr^));');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  writeln('begin');
  writeln('  enc.put_ulong(_ulong(addr^));');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  writeln('begin');
  writeln('  freemem(addr);');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  addr := allocmem(sizeOf(_ulong));');
  writeln('end;');
  writeln;
  // _assign
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  T'+name+'(dst^) := T'+name+'(src^);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

procedure TCodebase.alias_marshaller(al : IAliasDef);
var
  t : IIDLType;
begin
  t :=  original_type(al as IIDLType);
  case t.def_kind of
    dk_Primitive:;
    dk_Struct:;
    dk_Sequence : if not is_base_sequence(t as ISequenceDef) then
       seq_marshaller(t as ISequenceDef);
    dk_Array : array_marshaller(t as IArrayDef);
    dk_Enum : enum_marshaller(t as IContained);
    dk_Union : union_marshaller(t as IUnionDef);
    dk_Interface : int_marshaller(t as IInterfaceDef);
    else
      assert(false,'not implemented');
  end;
end;

procedure TCodebase.seq_marshaller(seq : ISequenceDef);
var
  name,mname,tstr,str: string;
  bound : longint;
  t: IIDLType;
begin
  if FDB.repoid_defined(FDB.pseudo_repoid(seq)) then exit;
  name := expand_name(seq);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  mname := 'T'+name+'_marshaller';
  tstr := expand_name(seq.element_type_def);
  bound := seq.bound;
  t := original_type(seq.element_type_def);
  //if (t.def_kind = dk_Sequence) and is_base_sequence(t as ISequenceDef) then
    //tstr := base_sequence_str(t as ISequenceDef);
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  // declare array
  {writeln('type ');
  if bound = 0 then
    writeln('  '+name+' = array of '+ tstr+';')
  else
    writeln('  '+name+' = array [0..'+inttostr(bound-1)+'] of '+ tstr+';');
  writeln('  P'+name+' = ^'+ name+';');}
  writeln;
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  writeln('var');
  writeln('  len: _ulong;');
  writeln('  i: integer;');
  writeln('  val: '+tstr+';');
  writeln('begin');
  writeln('  result := false;');
  writeln('  if not dec.seq_begin(len) then exit;');
  if bound = 0 then begin
    writeln('  SetLength('+name+'(addr^),len);');
    writeln('  if len <> 0 then');
    writeln('    for i := 0 to Pred(len) do');
    str := '  ';
  end
  else
    writeln('  for i := 0 to '+inttostr(bound-1)+' do');
  writeln(str+'    begin');
  writeln(str+'      if not '+marshaller_for(seq.element_type_def)+'.demarshal(dec,@val) then exit;');
  writeln(str+'      '+name+'(addr^)[i] := val; ');
  writeln(str+'    end;');
  writeln('  if not dec.seq_end() then exit;');
  writeln('  result := true;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  writeln('var');
  if bound = 0 then
    writeln('  len: _ulong;');
  writeln('  i: integer;');
  writeln('begin');
  if bound = 0 then begin
    writeln('  len := Length('+name+'(addr^));');
    writeln('  enc.seq_begin(len);');
  end
  else
    writeln('  enc.seq_begin(' + IntToStr(bound) + ');');
  if bound = 0 then begin
    writeln('  if len <> 0 then');
    writeln('    for i := 0 to Pred(len) do');
    write('  ');
  end
  else
    writeln('  for i := 0 to '+inttostr(bound-1)+' do');
  writeln('    '+marshaller_for(seq.element_type_def)+'.marshal(enc,@'+name+'(addr^)[i]);');
  writeln('  enc.seq_end;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  if (bound <> 0) and (t._type.kind in [tk_string,tk_objref]) then begin
    writeln('var');
    writeln('  i: integer;');
  end;
  writeln('begin');
  if bound = 0 then begin
    writeln('  setLength('+name+'(addr^),0);');
    writeln('  freemem(addr);');
  end
  else begin
    if t._type.kind in [tk_string,tk_objref] then begin
      writeln('  for i := 0 to '+inttostr(bound-1)+' do ');
      if t._type.kind = tk_string then
        writeln('    string(Pointer(longint(addr)+i*4)^) := '''';')
      else
        writeln('    '+tstr+'(Pointer(longint(addr)+i*4)^) := nil;');
    end;
    writeln('  freemem(addr,sizeof('+name+'));');
  end;
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  if bound = 0 then
    writeln('  addr := allocmem(sizeOf(_ulong));')
  else
    writeln('  addr := allocmem(sizeOf('+name+'));');
  writeln('end;');
  writeln;
  // _assign
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  _ulong(dst^) := _ulong(src^);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

function TCodebase.is_base_sequence(seq : ISequenceDef): Boolean;
var
  tc: ITypeCode;
begin
  result := false;
  tc := seq._type;
  tc := tc.unalias;
  if tc._length <> 0 then exit;
  tc := tc.content_type.unalias;
  { TODO -oOVS : uncomplete, must be rewritten }
  result := (tc.is_base_type and (tc.kind <> tk_fixed)) or tc.is_object or tc.is_any;
end;

procedure TCodebase.value_member(vmd: IValueMemberDef; all: boolean);
var
  mname,tname: string;
begin
  if all or (vmd.get_access = PUBLIC_MEMBER) then
    begin
      mname := vmd.name;
      tname := type_str(vmd.get_type_def);
      writeln('    function '+mname+' : '+tname+'; overload;');
      writeln('    procedure '+mname+'(val : '+tname+'); overload;');
    end;
end;

function TCodebase.initializer_decl(init: Initializer; target: IValueDef): string;
var
  j: integer;
  memb : StructMember;
begin
  result := 'function '+safe_name(init.name)+'(';
  for j := 0 to Pred(Length(init.members)) do
    begin
      if j > 0 then result := result + '; ';
      memb := init.members[j];
      result := result + memb.name+': '+type_str(memb.type_def);
    end;
  result := result + '): I'+get_name(target as IContained)+';';
end;

procedure TCodebase.stub_clp_attribute(name: string;attr: IAttributeDef);
var
  stub_name, attrname, tname: string;
  val : IIDLType;
begin
  attrname := attr.name;
  stub_name := 'T' + name+ '_stub_clp';
  tname := type_str(attr.get_type_def);
  if attr.get_mode = ATTR_NORMAL then
    begin
      writeln('procedure '+stub_name+'._set_'+attrname+'(const val: '+tname+');');
      writeln('var');
      writeln('  _srv: IServant;');
      writeln('  srv: I'+name+';');
      writeln('begin');
      writeln('  _srv := _preinvoke();');
      writeln('  if _srv <> nil then');
      writeln('    begin');
      writeln('      srv := T'+name+'_serv._narrow(_srv);');
      writeln('      if srv <> nil then');
      writeln('        '+'srv._set_'+attrname+'(val);');
      writeln('    end;');
      writeln('  _postinvoke();');
      writeln('end;');
      writeln;
    end;
  writeln('function '+stub_name+'._get_'+attrname+': '+tname+';');
  writeln('var');
  writeln('  _srv: IServant;');
  writeln('  srv: I'+name+';');
  writeln('begin');
  val := original_type(attr.get_type_def);
  if not (val._type.kind in [tk_void,tk_struct,tk_array,tk_union]) then
    writeln('  result := '+def_value(val)+';');
  writeln('  _srv := _preinvoke();');
  writeln('  if _srv <> nil then');
  writeln('    begin');
  writeln('      srv := T'+name+'_serv._narrow(_srv);');
  writeln('      if srv <> nil then');
  writeln('        result := srv._get_'+attrname);
  writeln('    end;');
  writeln('  _postinvoke();');
  writeln('end;');
  writeln;
end;

procedure TCodebase.stub_clp_attribute2(name, basename: string; attr: IAttributeDef);
var
  stub_name, attrname, tname: string;
  val : IIDLType;
begin
  attrname := attr.name;
  stub_name := 'T' + name+ '_stub_clp';
  tname := type_str(attr.get_type_def);
  if attr.get_mode = ATTR_NORMAL then
    begin
      writeln('procedure '+stub_name+'._set_'+attrname+'(const val: '+tname+');');
      writeln('var');
      writeln('  _srv: IServant;');
      writeln('  srv: I' + basename + ';');
      writeln('begin');
      writeln('  _srv := _preinvoke();');
      writeln('  if _srv <> nil then');
      writeln('    begin');
      writeln('      srv := T'+basename+'_serv._narrow(_srv);');
      writeln('      if srv <> nil then');
      writeln('        '+'srv._set_'+attrname+'(val);');
      writeln('    end;');
      writeln('  _postinvoke();');
      writeln('end;');
      writeln;
    end;
  writeln('function '+stub_name+'._get_'+attrname+': '+tname+';');
  writeln('var');
  writeln('  _srv: IServant;');
  writeln('  srv: I'+basename+';');
  writeln('begin');
  val := original_type(attr.get_type_def);
  if not (val._type.kind in [tk_void,tk_struct,tk_array,tk_union]) then
    writeln('  result := '+def_value(val)+';');
  writeln('  _srv := _preinvoke();');
  writeln('  if _srv <> nil then');
  writeln('    begin');
  writeln('      srv := T'+basename+'_serv._narrow(_srv);');
  writeln('      if srv <> nil then');
  writeln('        result := srv._get_'+attrname);
  writeln('    end;');
  writeln('  _postinvoke();');
  writeln('end;');
  writeln;
end;

procedure TCodebase.stub_clp_method(name, basename: string; op: IOperationDef);
var
  stub_name, operation: string;
  i,j : integer;
  params: ParDescriptionSeq;
  res: IIDLType;
begin
  operation := operation_str(op);
  stub_name := 'T' + name+ '_stub_clp';
  j := pos(' ',operation);
  writeln(copy(operation,1,j)+stub_name+'.'+copy(operation,j+1,10000));
  writeln('var');
  writeln('  _srv: IServant;');
  writeln('  srv: I'+basename+';');
  writeln('begin');
  res := original_type(op.get_result_def);
  if not (res._type.kind in [tk_void,tk_struct,tk_array,tk_union]) then
    writeln('  result := '+def_value(op.get_result_def)+';');
  writeln('  _srv := _preinvoke();');
  writeln('  if _srv <> nil then');
  writeln('    begin');
  writeln('      srv := T'+basename+'_serv._narrow(_srv);');
  writeln('      if srv <> nil then');
  operation := 'srv.'+safe_name(op.name);
  res := op.get_result_def;
  if res._type.kind <> orbtypes.tk_void then
    operation := 'result := '+operation;
  params := op.get_params;
  if Length(params) > 0 then
    begin
      operation := operation + '(';
      for i := 0 to Pred(Length(params)) do
        begin
          if i > 0 then operation := operation + ', ';
          operation := operation + safe_name(params[i].name);
        end;
      operation := operation + ')';
    end;

  writeln('        '+operation);
  writeln('    end;');
  writeln('  _postinvoke();');
  writeln('end;');
  writeln;
end;

procedure TCodebase.method_dispatch(cname: string; int: IInterfaceDef; passenv: Boolean);
var
  base,methods,attributes: IInterfaceList;
  i,j : integer;
  firstcall: Boolean;
begin
  if passenv then
    writeln('function '+cname+'._dispatch(const req: IStaticServerRequest; const env: IEnvironment): Boolean;')
  else
    writeln('function '+cname+'._dispatch(const req: IStaticServerRequest): Boolean;');
  methods := (int as IContainer).contents(dk_Operation);
  attributes := (int as IContainer).contents(dk_Attribute);
  firstcall := true;
  for i := 0 to Pred(methods.count) do
    method_vars(methods[i] as IOperationDef,firstcall);
  for i := 0 to Pred(attributes.Count) do
    attr_method_vars(attributes[i] as IAttributeDef,firstcall);
  base := int.get_base_interfaces;
  for j := 1 to Pred(base.count) do
    begin
      methods := (base[j] as IContainer).contents(dk_Operation);
      for i := 0 to Pred(methods.count) do
        method_vars(methods[i] as IOperationDef,firstcall);
      attributes := (base[j] as IContainer).contents(dk_Attribute);
      for i := 0 to Pred(attributes.Count) do
        attr_method_vars(attributes[i] as IAttributeDef,firstcall);
    end;
  writeln('begin');
  writeln('  result := true;');

  methods := (int as IContainer).contents(dk_Operation);
  firstcall := true;
  for i := 0 to Pred(methods.count) do
    begin
      if not firstcall then
        writeln;
      method_call(methods[i] as IOperationDef,firstcall);
    end;
  attributes := (int as IContainer).contents(dk_Attribute);
  for i := 0 to Pred(attributes.Count) do
    begin
      if not firstcall then
        writeln;
      attr_method_call(attributes[i] as IAttributeDef,firstcall);
    end;
  for j := 1 to Pred(base.count) do
    begin
      methods := (base[j] as IContainer).contents(dk_Operation);
      for i := 0 to Pred(methods.count) do
        begin
          if not firstcall then
            writeln;
          method_call(methods[i] as IOperationDef,firstcall);
        end;
      attributes := (base[j] as IContainer).contents(dk_Attribute);
      for i := 0 to Pred(attributes.count) do
        begin
          if not firstcall then
            writeln;
          attr_method_call(attributes[i] as IAttributeDef,firstcall);
        end;
    end;
  if (methods.count <> 0) or (attributes.Count <> 0) then
    writeln(';');
  if base.count > 0 then
    begin
      if passenv then
        writeln('  if not inherited _dispatch(req,env) then')
      else
        writeln('  if not inherited _dispatch(req) then');
      writeln('    result := false;');
    end
  else if (methods.count <> 0) or (attributes.Count <> 0) then
    writeln('  result := false;');
  writeln('end;');
  writeln;
end;

procedure TCodebase.array_marshaller(arr: IArrayDef);
var
  tstr,name,mname : string;
  size : integer;
  t: IIDLType;
begin
  if FDB.repoid_defined(FDB.pseudo_repoid(arr)) then exit;
  name := expand_name(arr);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  size := arr.length;
  mname := 'T'+name+'_marshaller';
  t := original_type(arr.element_type_def);
  if FDB.repoid_defined(FDB.pseudo_repoid(t)) then
    tstr := type_str(arr.element_type_def)
  else
    tstr := expand_name(t);
  if t.def_kind = dk_Array then
    array_marshaller(t as IArrayDef);
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  // declare array
  writeln('type ');
  writeln('  '+name+' = array [0..'+inttostr(size-1)+'] of '+ tstr+';');
  writeln('  P'+name+' = ^'+ name+';');
  writeln;
  writeln('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  writeln('var');
  writeln('  i: integer;');
  writeln('begin');
  writeln('  result := false;');
  writeln('  for i := 0 to '+inttostr(size-1)+' do');
  if t.def_kind = dk_Primitive then
    writeln('    if not dec.'+coder_method('get_', tstr)+'(P'+name+'(addr)^[i]) then exit;')
  else
    writeln('    if not '+marshaller_for(t)+'.demarshal(dec,@P'+name+'(addr)^[i]) then exit;');
  writeln('  result := true;');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  writeln('var');
  writeln('  i: integer;');
  writeln('begin');
  writeln('  for i := 0 to '+inttostr(size-1)+' do');
  if t.def_kind = dk_Primitive then
    writeln('    enc.'+coder_method('put_', tstr)+'(P'+name+'(addr)^[i]);')
  else
    writeln('    '+marshaller_for(t)+'.marshal(enc,@P'+name+'(addr)^[i]);');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  if t._type.kind in [tk_string,tk_objref] then
    begin
      writeln('var');
      writeln('  i: integer;');
    end;
  writeln('begin');
  if t._type.kind in [tk_string,tk_objref] then
    begin
      writeln('  for i := 0 to '+inttostr(size-1)+' do ');
      if t._type.kind = tk_string then
        writeln('    string(Pointer(longint(addr)+i*4)^) := '''';')
      else
        writeln('    '+tstr+'(Pointer(longint(addr)+i*4)^) := nil;');
    end;
  writeln('  freemem(addr,sizeof('+name+'));');
  writeln('end;');
  writeln;
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  addr := allocmem(sizeOf('+name+'));');
  writeln('end;');
  writeln;
  // _assign
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  P'+name+'(dst)^ := P'+name+'(src)^;');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any') > 0 then
    //begin
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

function TCodebase.expand_name(t: IIDLType; recursive: Boolean = false): string;
var
  otype: IIDLType;
begin
  otype := original_type(t);
  case otype.def_kind of
    dk_Array : with otype as IArrayDef  do
       begin
         result := 'A'+inttostr(length)+'_'+expand_name(element_type_def,true);
         //if not recursive then result := result + '_array';
         result := result + '_array';
       end;
    dk_Sequence : with otype as ISequenceDef  do
      begin
        if bound > 0 then
          result := 'B'+inttostr(bound)+'_'+expand_name(element_type_def,true)
        else
          result := expand_name(element_type_def,true);
        //if not recursive then result := result + '_seq';
        result := result + '_seq';
      end;
    else
      result := type_str(t,recursive);
  end;
end;

function TCodebase.marshaller_for_seq(s: ISequenceDef): string;
begin
  result := expand_name(s);
end;

function TCodebase.marshaller_for_array(arr: IArrayDef): string;
begin
  result := expand_name(arr);
end;

class function TCodebase.original_type(t: IIDLType): IIDLType;
begin
  result := t;
  while result.def_kind = dk_alias do result := (result as IAliasDef).original_type_def;
end;

function TCodeBase.array_str(arr: IArrayDef): string;
begin
  result := 'array [0..'+inttostr(arr.length-1)+'] of ';
  if arr.element_type_def.def_kind = dk_Array then
    result := result + array_str(arr.element_type_def as IArrayDef)
  else
    result := result + type_str(arr.element_type_def)+';'
end;

function TCodebase.declare(name: string): Boolean;
begin
  if FDeclared.IndexOf(name) < 0 then
    begin
      FDeclared.add(name);
      result := true;
    end
  else
    result :=  false;
end;

function TCodebase.add_marshaller(name: string; it: IIRObject): Boolean;
begin
  if Marshallers.IndexOf(name) < 0 then
    begin
      Marshallers.addObject(name,Pointer(it));
      result := true;
    end
  else
    result := false;
end;

procedure TCodebase.ex_members_decl(ename: string; members: StructMemberSeq; mode : TExcept_declare_mode);
var
  i : integer;
  member : StructMember;
  tstr : string;
  mname: string;
begin
  for i := 0 to Pred(Length(members)) do
    begin
      member := members[i];
      case member.type_def.def_kind of
        dk_Sequence : tstr := 'T'+ename + '_'+member.name + '_seq';
        dk_Array : tstr := 'T'+ename + '_'+member.name + '_array';
        else
          tstr := type_str(member.type_def)
      end;
      mname := safe_name(member.name);
      if mode in [ed_all,ed_out] then
        writeln('    function '+mname+': ' + tstr+'; overload;');
      if mode in [ed_all,ed_in] then
        writeln('    procedure '+mname+'(const val : ' + tstr+'); overload;');
    end;
end;

procedure TCodebase.add_forwards(int: IContained);
begin
  if FDB.repoid_defined(int.id) then exit;
  if FDeclared.IndexOf(get_name(int)) >= 0 then exit;
  if FForwards.IndexOf(int) < 0 then FForwards.add(int);
end;

procedure TCodebase.value_marshaller(cont: IContained);
var
  name,cname,mname : string;
begin
  name := get_name(cont as IContained);
  if FImplemented.IndexOf(name+'_marshaller') >= 0 then exit;
  FImplemented.add(name+'_marshaller');
  cname := 'T'+name;
  mname := cname+'_marshaller';
  writeln('//***********************************************************');
  writeln('// '+mname);
  writeln('//***********************************************************');
  // _create
  writeln('procedure '+mname+'._create(var addr: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(addr^) := '+cname+'.Create as I'+name+';');
  writeln('end;');
  writeln;
  // _assign
  writeln('procedure '+mname+'._assign(dst, src: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(dst^) := I'+name+'(src^);');
  writeln('end;');
  writeln;
  // _free
  writeln('procedure '+mname+'._free(var addr: Pointer);');
  writeln('begin');
  writeln('  I'+name+'(addr^) := nil;');
  writeln('end;');
  writeln;
  //  demarshal
  writeln('function '+mname+'.demarshal(dec: IDecoder;');
  writeln('  const addr: Pointer): Boolean;');
  writeln('var');
  writeln('  vb : IValueBase;');
  writeln('begin');
  writeln('  result := TValueBase._demarshal(dec,vb,''' + cont.id + ''');');
  writeln('  if not result then exit;');
  writeln('  I'+name+'(addr^) := '+cname+'._downcast(vb);');
  writeln('  result := (vb = nil) or (vb <> nil) and (Pointer(addr^) <> nil);');
  writeln('end;');
  writeln;
  //  marshal
  writeln('procedure '+mname+'.marshal(enc: IEncoder; const addr: Pointer);');
  writeln('begin');
  writeln('  TValueBase._marshal(enc,I'+name+'(addr^) as IValueBase);');
  writeln('end;');
  writeln;
  //if FParams.IndexOf('-any')>=0 then
    //begin
      //  typecode
      writeln('function '+mname+'.typecode: ITypeCode;');
      writeln('begin');
      writeln('  result := _tc_'+name+'.typecode;');
      writeln('end;');
      writeln;
    //end;
end;

procedure TCodebase.common_servant_methods_declaration;
begin
  writeln('    function _primary_interface(const objid: ObjectID; const poa: IPOA): string; override;');
  writeln('    function _make_stub(const poa: IPOA; const obj : IORBObject): IORBObject; override;');
  writeln('    function _is_a(const repoid: RepositoryID): Boolean; override;');
  writeln('    procedure invoke(const serv: IStaticServerRequest); override;');
  writeln('    function _dispatch(const req: IStaticServerRequest): Boolean;');
  writeln('    function narrow_helper(const str: string): Pointer; override;');
end;

procedure TCodebase.serv_method(name: string; op: IOperationDef; const intf: string);
var
  stub_name, operation: string;
  params: ParDescriptionSeq;
  i, j : integer;
  res: IIDLType;
begin
  operation := operation_str(op);
  stub_name := 'T' + name+ '_serv';
  j := pos(' ',operation);
  writeln(copy(operation,1,j)+stub_name+'.'+copy(operation,j+1,10000));
  writeln('begin');
  if intf <> '' then
    operation := '(FImplementation as ' + intf + ').'+safe_name(op.name)
  else
    operation := 'FImplementation.'+safe_name(op.name);
  res := op.get_result_def;
  if res._type.kind <> orbtypes.tk_void then
    operation := 'result := '+operation;
  params := op.get_params;
  if Length(params) > 0 then
    begin
      operation := operation + '(';
      for i := 0 to Pred(Length(params)) do
        begin
          if i > 0 then operation := operation + ', ';
          operation := operation + safe_name(params[i].name);
        end;
      operation := operation + ')';
    end;

  writeln('  ' + operation + ';');
  writeln('end;');
  writeln;
end;

procedure TCodebase.serv_attribute(name: string; attr: IAttributeDef; const intf: string);
var
  stub_name, operation: string;
  j : integer;
begin
  operation := attribute_str(attr, false);
  stub_name := 'T' + name+ '_serv';
  j := pos(' ',operation);
  writeln(copy(operation,1,j)+stub_name+'.'+copy(operation,j+1,10000));
  writeln('begin');
  if intf <> '' then
    writeln('  result := (FImplementation as ' + intf + ').' + attr._describe.name + ';')
  else
    writeln('  result := FImplementation.' + attr._describe.name + ';');
  writeln('end;');
  writeln;
  if attr.mode = ATTR_NORMAL then begin
    operation := attribute_str(attr, true);
    j := pos(' ',operation);
    writeln(copy(operation,1,j)+stub_name+'.'+copy(operation,j+1,10000));
    writeln('begin');
    if intf <> '' then
      writeln('  (FImplementation as ' + intf + ').' + attr._describe.name + ' := val')
    else
      writeln('  FImplementation.' + attr._describe.name + ' := val');
    writeln('end;');
    writeln;
  end;
end;

procedure TCodebase.attr_method_vars(attr: IAttributeDef;
  var firstcall: Boolean);
var
  name: string;
begin
  name := attr._describe.name;
  if firstcall then begin
    firstcall := false;
    writeln('var');
  end;
  writeln('  '+name+'_get_value: '+type_str(attr.get_type_def)+';');
  if attr.mode = ATTR_NORMAL then
    writeln('  '+name+'_set_value: '+type_str(attr.get_type_def)+';');
end;

procedure TCodebase.attr_method_call(attr: IAttributeDef;
  var firstcall: Boolean);
var
  name: string;
begin
  name := attr._describe.name;
  if firstcall then
    writeln('  if req.op_name = '''+'_get_'+name+''' then')
  else
    writeln('  else if req.op_name = '''+'_get_'+name+''' then');
  writeln('    begin');
  writeln('      req.set_result(StaticAny('+marshaller_for(attr.get_type_def)+',@'+name+'_get_value) as IStaticAny);');
  writeln('      if not req.read_args() then  exit;');
  writeln('      '+name+'_get_value := _get_'+name+';');
  writeln('      req.write_results();');
  writeln('      exit;');
  write('    end');
  if attr.mode = ATTR_NORMAL then begin
    writeln;
    writeln('  else if req.op_name = '''+'_set_'+name+''' then');
    writeln('    begin');
    write('      req.add_in_arg(');
    writeln('StaticAny('+marshaller_for(attr.get_type_def)+',@'+name+'_set_value) as IStaticAny);');
    writeln('      if not req.read_args() then  exit;');
    writeln('      _set_'+name+'('+name+'_set_value);');
    writeln('      req.write_results();');
    writeln('      exit;');
    write('    end');
  end;
  firstcall := false;
end;

function TCodebase.base_sequence_str(seq: ISequenceDef): string;
var
  otype: IIDLType;
begin
  otype := original_type(seq.element_type_def);
  result := type_str(otype) + '_seq';
end;

end.
