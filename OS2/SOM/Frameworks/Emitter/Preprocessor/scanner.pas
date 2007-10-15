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
unit scanner;

interface

uses
  Sysutils,{$IFDEF MSWINDOWS}Windows,{$ENDIF}{$IFNDEF VER130}Variants,{$ENDIF}
  ir_int,ir,Classes,parser,orbtypes,orb_int,any,tcode,Math,idldb;

type

  TScanner = class
  private
    FContainer : IRepository;
    FRoot : TNode;
    FVersion: string;
    FForwardList,FInterfaceList : TStrings;
    FDB : TDB;
  public
    constructor Create(root: TNode;con: IRepository; db: TDB);
    destructor Destroy(); override;
    procedure collect();
    function get_type(con: IContainer; node: TNode):IIDLType;
    function gen_repo_id(node: TNode): string;
    function gen_ident(node: TNode): string;
    procedure scan(con: IContainer; node: TNode);
    procedure add_forward_dcl(id: string);
    procedure remove_forward_dcl(id: string);
    function forward_exists(id: string): Boolean;
    function interface_exists(id: string): Boolean;
    procedure interface_dcl(con: IContainer; node: TNode);
    procedure value_dcl(con: IContainer; node: TNode; is_abstract: Boolean);
    procedure value_box_dcl(con: IContainer; node: TNode);
    procedure forward_value_dcl(con: IContainer; node: TNode; is_abstract: Boolean);
    procedure op_dcl(con: IContainer; node: TNode);
    procedure forward_dcl(con: IContainer; node: TNode; is_abstract: Boolean; is_local: Boolean);
    procedure state_member(con: IContainer;node: TNode; is_public : Boolean);
    function struct_type(con: IContainer; node: TNode) : IIDLType;
    procedure struct_members(con: IContainer; node: TNode; mem: PStructMemberSeq);
    procedure struct_member(con: IContainer; node: TNode; mem: PStructMemberSeq);
    procedure pragma(con: IContainer; node: TNode);
    procedure include(con: IContainer; node: TNode);
    procedure struct_member_decl(con: IContainer; node: TNode; _type: IIDLType; mem: PStructMemberSeq);
    procedure struct_member_decls(con: IContainer; node: TNode; _type: IIDLType; mem: PStructMemberSeq);
    function array_dcl(con: IContainer; node: TNode; _type: IIDLType): IIDLType;
    function const_val(con: IContainer; node: TNode): Variant;
    procedure const_dcl(con: IContainer; node: TNode);
    procedure const_fixed_dcl(con: IContainer; node: TNode);
    procedure typedef(con: IContainer; node: TNode);
    procedure native(con: IContainer; node: TNode);
    function enum(con: IContainer; node: TNode): IIDLType;
    procedure module(con: IContainer; node: TNode);
    procedure enum_members(node: TNode; mem: PEnumMemberSeq);
    procedure attributes(con: IContainer;node: TNode; readonly : Boolean);
    function union_dcl(con: IContainer; node: TNode): IIDLType;
    procedure exception_dcl(con: IContainer; node: TNode);
    procedure union_case(con: IContainer; node: TNode; descr: IIDLType; members: PUnionMemberSeq);
    procedure union_members(con: IContainer; node: TNode; descr: IIDLType; members: PUnionMemberSeq);
    procedure raises_dcl(con: IContainer;node: TNode;exceptions: PExceptionDefSeq);
    function is_typecode(node: TNode): Boolean;
    function is_principal(node: TNode): Boolean;
    procedure import(con: IContainer; node: TNode);
  end;

implementation

uses
  orb,ir_value,code_int,codebase,Lexlib,cpp,codeutil;

procedure scanner_exception(node: TNode; mess: string); overload;
begin
  raise Exception.Create(node.file_name+'('+inttostr(node.line_number)+'): '+mess);
end;

const
  ErrorMsg: array [0..40] of string = (
      (*   0 *) 'No Error',
	    (*   1 *) 'Not all forward declarations have been defined',
	    (*   2 *) 'scoped name ''%s'' not defined',
	    (*   3 *) 'scoped name ''%s'' is not a typedef',
	    (*   4 *) 'constant definition ''%s'' not of proper type',
	    (*   5 *) 'union switch type spec not of proper type',
	    (*   6 *) 'scoped name ''%s'' is not an exception',
	    (*   7 *) 'scoped name ''%s'' is not an IDLType',
	    (*   8 *) 'name ''%s'' is not a base interface',
	    (*   9 *) 'base interface ''%s'' is not yet defined',
	    (*  10 *) 'can not coerce ''%s'' to ''%s''',
	    (*  11 *) '''%s'' is not member of enum ''%s''',
	    (*  12 *) 'identifier ''%s'' defined twice in this scope',
	    (*  13 *) 'repository id for identifier ''%s'' already used',
	    (*  14 *) 'union ''%s'' has duplicate labels',
	    (*  15 *) 'reference to identifier ''%s'' is ambiguous',
	    (*  16 *) 'can not overload operation or attribute ''%s''',
	    (*  17 *) 'bad kind of recursion in constructed type ''%s''',
	    (*  18 *) 'oneway operation ''%s'' can not have result, out-parameters or raises-expressions',
	    (*  19 *) 'type ''%s'' has identical members',
	    (*  20 *) 'case label type is not an enum',
	    (*  21 *) 'no value for default case from discriminator type available',
	    (*  22 *) 'bad #pragma directive',
	    (*  23 *) 'identifier ''%s'' is not an interface',
	    (*  24 *) 'scoped name ''%s'' is not a constant',
	    (*  25 *) 'interface ''%s'' can not inherit from itself',
	    (*  26 *) 'custom value ''%s'' cannot be truncatable',
	    (*  27 *) 'abstract value ''%s'' cannot be truncatable',
	    (*  28 *) 'abstract value ''%s'' cannot inherit from concrete value ''%s''',
	    (*  29 *) 'cannot truncate ''%s'' to abstract value ''%s''',
      (*  30 *) 'illegal interface inheritance: ''%s'' cannot inherit from ''%s''',
      (*  31 *) 'internal error',
      (*  32 *) 'unknown preprocessor directive ''%s''',
      (*  33 *) 'interface defined with different repository id ''%s''',
	    (*  34 *) 'identifier ''%s'' is not a value',
      (*  35 *) 'value defined with different repository id ''%s''',
	    (*  36 *) 'cannot import ''%s'': no Interface Repository',
	    (*  37 *) 'cannot import ''%s'': not found in Interface Repository',
	    (*  38 *) 'cannot import ''%s'': no entry in Repository and no file',
	    (*  39 *) 'cannot import ''%s'': non-container object',
	    (*  40 *) 'import statement not at toplevel'
  );

procedure scanner_exception(AErrNum: Integer; node: TNode; mess: string = ''); overload;
begin
  raise Exception.Create(node.file_name+'('+inttostr(node.line_number)+'): '+Format(ErrorMsg[AErrNum], [mess]));
end;

//*************************************************
//  TScanner
//*************************************************
constructor TScanner.Create(root: TNode; con: IRepository; db: TDB);
begin
  FRoot := root;
  FContainer := con;
  FVersion := '1.0';
  FForwardList := TStringList.Create;
  FInterfaceList := TStringList.Create;
  FDB := db;
end;

destructor TScanner.Destroy();
begin
  FForwardList.free;
  FInterfaceList.free;
end;

procedure TScanner.collect;
begin
  scan(FContainer,FRoot);
  FDB.set_repoids(FContainer);
end;

function TScanner.gen_ident(node: TNode): string;
begin
  result := FDB.get_id_prefix+ node.getIdent;
end;

function TScanner.gen_repo_id(node: TNode): string;
var
  absname, name: string;
begin
  name := node.getIdent;
  if name[1] = ':' then
    //It is an absolute name
    absname := name
  else
    absname := FDB.get_current_scope() + node.getIdent;
  result := 'IDL:'+ FDB.get_prefix(absname) + name +':'+ FVersion;
  FDB.register_repoid(node,result);
end;

procedure TScanner.add_forward_dcl(id: string);
begin
  FForwardList.add(id);
end;

procedure TScanner.remove_forward_dcl(id: string);
begin
  if FForwardList.IndexOf(id) >= 0 then
    FForwardList.Delete(FForwardList.IndexOf(id));
  FInterfaceList.add(id);
end;

function TScanner.interface_exists(id: string): Boolean;
begin
  result := FInterfaceList.IndexOf(id) >= 0;
end;

function TScanner.forward_exists(id: string): Boolean;
begin
  result := FForwardList.IndexOf(id) >= 0;
end;

procedure TScanner.scan(con: IContainer; node: TNode);
var
  ntype : TNodeType;
begin
  if (node = nil) then exit;
  ntype := node.node_type;
  case ntype of
    nt_const_dcl : const_dcl(con,node);
    nt_module : module(con,node);
    nt_definitions, nt_exports: begin
        scan(con,node.getBranch(1));
        scan(con,node.getBranch(2));
      end;
    nt_enum_type: enum(con,node);
    nt_typedef : typedef(con,node);
    nt_native : native(con,node);
    nt_struct_type : struct_type(con,node);
    nt_forward_dcl : forward_dcl(con,node,false,false);
    nt_abstract_forward_dcl : forward_dcl(con,node,true,false);
    nt_local_forward_dcl : forward_dcl(con,node,false,true);
    nt_interface_dcl : interface_dcl(con,node);
    nt_op_dcl : op_dcl(con,node);
    nt_abstract_value_forward_dcl : forward_value_dcl(con,node,true);
    nt_value_forward_dcl : forward_value_dcl(con,node,false);
    nt_abstract_value_dcl: value_dcl(con,node,true);
    nt_value_dcl : value_dcl(con,node,false);
    nt_value_box_dcl : value_box_dcl(con,node);
    nt_public_state_member : state_member(con,node,true);
    nt_private_state_member : state_member(con,node,false);
    nt_attribute : attributes(con,node,false);
    nt_readonly_attribute :attributes(con,node,true);
    nt_union_type : union_dcl(con,node);
    nt_except_dcl : exception_dcl(con,node);
    nt_include : include(con,node);
    nt_pragma, nt_typeid, nt_typeprefix : pragma(con,node);
    nt_import : import(con,node);
    else
      assert(false,'not implemented');
  end;
end;

procedure TScanner.forward_dcl(con: IContainer; node: TNode; is_abstract: Boolean; is_local: Boolean);
var
  int : IContained;
  id : string;
  params : IInterfaceList;
begin
  id := gen_repo_id(node);
  if forward_exists(id) then  exit;
  if interface_exists(id) then exit;
  params := TInterfaceList.Create;
  int := con.create_interface(id,node.getIdent,FVersion,params,is_abstract,is_local) as  IContained;
  add_forward_dcl(id);
end;

procedure TScanner.forward_value_dcl(con: IContainer; node: TNode; is_abstract: Boolean);
var
  int : IContained;
  id : string;
  params : IInterfaceList;
begin
  id := gen_repo_id(node);
  if forward_exists(id) then  exit;
  if interface_exists(id) then exit;
  params := TInterfaceList.Create;
  int := IContained(con.create_value(id,node.getIdent,FVersion,false,is_abstract,nil,false,
    TInterfaceList.Create,TInterfaceList.Create,nil));
  add_forward_dcl(id);
end;

procedure TScanner.value_box_dcl(con: IContainer; node: TNode);
var
  id, name : string;
  val : IValueBoxDef;
begin
  id := gen_repo_id(node);
  name := node.getIdent;
  val := con.create_value_box(id,name,FVersion,get_type(con,node.getBranch(1)));
end;

procedure TScanner.value_dcl(con: IContainer; node: TNode; is_abstract: Boolean);
var
  hdr, elements, snames, sname, tmp: TNode;
  id, name : string;
  is_custom, have_concrete_int, is_truncatable : Boolean;
  list,supported,abstract_base : IInterfaceList;
  val,bvalue,base: IValueDef;
  element, pdecls, pdecl : TNode;
  init : Initializer;
  i : integer;
  it: IIDLType;
  idef: IInterfaceDef;
begin
  hdr := node.getBranch(1);
  id := gen_repo_id(hdr);
  name := hdr.getIdent;
  FDB.enter_scope(name);
  supported := TInterfaceList.Create;
  abstract_base := TInterfaceList.Create;
  is_custom :=  hdr.node_type = nt_custom_value_header;
  list := con.lookup_name(name,0,dk_Value);
  if (list.count > 0) then
    begin
      if not forward_exists(IContained(list[0]).id) then
        scanner_exception(node,'Duplicate value declaration');
    end;
  val := con.create_value(id,name,FVersion,is_custom,is_abstract,nil,false,
    TInterfaceList.Create,TInterfaceList.Create,nil);
  hdr := hdr.getBranch(1);
  if  hdr <> nil then
    begin
      // supported interfaces, base value, abstract base values
      snames := hdr.getBranch(4);
      have_concrete_int := false;
      while snames <> nil do
        begin
          sname := snames.getBranch(1);
          assert(sname.node_type = nt_scoped_name);
          it := get_type(con,sname);
          idef := TInterfaceDef._narrow(it as IORBObject);
          if not idef.get_is_abstract then
            begin
              if have_concrete_int then
                scanner_exception(node,'already have interface');
              have_concrete_int := true;
            end;
          if FForwardList.IndexOf(idef.id) >= 0 then
            scanner_exception(node,'interface not described');
          supported.Add(idef);
          snames := snames.getBranch(2);
        end;
      val.set_supported_interfaces(supported);
      // first element may be a concrete (non-abstract) value
      tmp := hdr.getBranch(1);
      is_truncatable := False;
      if tmp <> nil then
        begin
          if hdr.getBranch(2) <> nil then
            begin
              is_truncatable := true;
              tmp := hdr.getBranch(2)
            end;
          if (is_custom and is_truncatable) or (is_abstract and is_truncatable) then
            scanner_exception(node,'can`t be truncatable');
          if tmp.node_type = nt_scoped_names then
            tmp := tmp.getBranch(1);
          if tmp.node_type = nt_scoped_name then
            begin
              it := get_type(con,tmp);
              bvalue := TValueDef._narrow(it as IORBObject);
              if bvalue = nil then
                scanner_exception(node,'base value error');
              if FForwardList.IndexOf((bvalue  as IContained).id) >= 0 then
                scanner_exception(node,'valuetype not described');
              if val = bvalue then
                scanner_exception(node,'equivalent to base value');
              if is_abstract and not bvalue.is_abstract then
                scanner_exception(node,'base value is not abastract');
              if is_truncatable and bvalue.is_abstract then
                scanner_exception(node,'can`t be truncatable');
              if bvalue.is_abstract then
                begin
                  abstract_base.Add(bvalue);
                  bvalue := nil;
                end;
              val.set_base_value(bvalue);
            end
          else
            bvalue := nil;
        end;
      hdr := hdr.getBranch(3);
      // scan abstract base values
      while hdr <> nil do
        begin
          tmp := hdr.getBranch(1);
          it := get_type(con,tmp);
          base := TValueDef._narrow(it as IORBObject);
          if base = nil then
            scanner_exception(tmp,'is not valuetype');
          if FForwardList.IndexOf((base as IContained).id) >= 0 then
            scanner_exception(tmp,'valuetype not described');
          if base = val then
            scanner_exception(node,'equivalent to base value');
          abstract_base.Add(base);
          hdr := hdr.getBranch(2);
        end;
    end
  else
    begin
      is_truncatable := false;
      bvalue := nil;
    end;
  val.set_is_truncatable(is_truncatable);
  val.set_abstract_base_values(abstract_base);
  elements := node.getBranch(2);
  while elements <> nil do
    begin
      element := elements.getBranch(1);
      if element.node_type = nt_init_dcl then
        begin
          init.name := element.getIdent;
          pdecls := element.getBranch(1);
          while pdecls <> nil do
            begin
              pdecl := pdecls.getBranch(1);
              i := Length(init.members);
              setLength(init.members,i+1);
              init.members[i].name := pdecl.getBranch(2).getIdent;
              init.members[i].type_def := get_type(val as IContainer,pdecl.getBranch(1));
              init.members[i]._type := _tc_void;
              pdecls := pdecls.getBranch(2);
            end;
          i := length(val.initializers);
          val.resize_initializers(i+1);
          val.initializers[i] := init;
        end
      else
        scan(val as IContainer,element);
      elements := elements.getBranch(2);
    end;
  FDB.leave_scope;
end;

procedure TScanner.interface_dcl(con: IContainer; node: TNode);
var
  hdr,scoped_names: TNode;
  id : string;
  is_abstract, is_local : Boolean;
  list,baseint : IInterfaceList;
  int : IInterfaceDef;
  tmp: string;
  idlt : IIDLType;
  base: IInterfaceDef;
begin
  hdr := node.getBranch(1);
  id := gen_repo_id(hdr);
  is_abstract := hdr.node_type = nt_abstract_interface_header;
  is_local := hdr.node_type = nt_local_interface_header;
  list := con.lookup_name(hdr.getIdent,0,dk_Interface);
  if (list.count > 0) then
    begin
      tmp := IContained(list[0]).id;
      if not forward_exists(tmp) then
        scanner_exception(node,'Duplicate interface declaration');
    end;
  baseint := TInterfaceList.Create;
  int := con.create_interface(id,hdr.getIdent,FVersion,baseint,is_abstract,is_local) as IInterfaceDef;
  id := IContained(int).id();
  remove_forward_dcl(id);
  FDB.enter_scope(hdr.getIdent);//enter to scope before get scoped names
  scoped_names := hdr.getBranch(1);
  while scoped_names <> nil do
    begin
      hdr := scoped_names.getBranch(1);
      idlt := get_type(con,hdr);
      base := TInterfaceDef._narrow(idlt as IORBObject);
      if base = nil then
        scanner_exception(node,'Interface not found');
      if forward_exists(IContained(base).id) then
        scanner_exception(node,'Interface not defined');
      if int.get_is_abstract and not base.get_is_abstract then
        scanner_exception(node,'interface '+IContained(int).name+' can`t be abstract');
      baseint.Add(base);
      scoped_names := scoped_names.getBranch(2);
    end;
  //FDB.enter_scope(hdr.getIdent);
  scan(int as IContainer,node.getBranch(2));
  FDB.leave_scope;
end;

procedure TScanner.op_dcl(con: IContainer; node: TNode);
var
  idef: IInterfaceDef;
  vdef : IValueDef;
  omode: TOperationMode;
  param,params : TNode;
  oparams : ParDescriptionSeq;
  pmode : ParameterMode;
  ptype,rtype: IIDLType;
  paramdesc : ParameterDescription;
  exceptions: ExceptionDefSeq;
  id,name: string;
  i : integer;
begin
  idef := TInterfaceDef._narrow(con as IORBObject);
  if idef = nil then  vdef := TValueDef._narrow(con as IORBObject);
  assert(assigned(idef) or assigned(vdef));
  if node.getBranch(1) = nil then
    omode := OP_NORMAL
  else
    omode := OP_ONEWAY;
  setLength(oparams,0);
  params := node.getBranch(3);
  while params <> nil do
    begin
      param := params.getBranch(1);
      case param.getBranch(1).node_type of
        nt_out: pmode := PARAM_OUT;
        nt_inout: pmode := PARAM_INOUT;
        else
          pmode := PARAM_IN;
      end;
      if (omode = OP_ONEWAY) and (pmode <> PARAM_IN) then
        scanner_exception(18, node, node.getIdent);
      ptype := get_type(con,param.getBranch(2));
      paramdesc.name := param.getBranch(3).getIdent;
      paramdesc.mode := pmode;
      paramdesc.type_def := ptype;
      paramdesc._type := ptype._type;
      i := Length(oparams);
      setLength(oparams,i+1);
      oparams[i] := paramdesc;
      params := params.getBranch(2);
    end;
  if node.getBranch(2).node_type = nt_void then
    rtype := get_type(con,node.getBranch(2))
  else if omode <> OP_ONEWAY then
    rtype := get_type(con,node.getBranch(2).getBranch(1))
  else
    scanner_exception(18, node, node.getIdent);
  // raising
  if node.getBranch(4) <> nil then
    raises_dcl(con,node.getBranch(4),@exceptions);
  if (omode = OP_ONEWAY) and (Length(exceptions) <> 0) then
    scanner_exception(18, node, node.getIdent);
  name := node.getIdent;
  id := 'IDL:'+(con as IContained).name+'/'+name+':'+FVersion;
  try
    if idef <> nil then
      (con as IInterfaceDef).create_operation(id,name,FVersion,rtype,omode,oparams,exceptions)
    else
      (con as IValueDef).create_operation(id,name,FVersion,rtype,omode,oparams,exceptions)
  except
    on E: Exception do
      scanner_exception(node, E.Message);
  end; { try/except }
end;

function TScanner.get_type(con: IContainer; node: TNode): IIDLType;
var
  cont: IContained;
  it : IIDLType;
  bound: integer;
  ident: string;
begin
  result := nil;
  case node.node_type of
    nt_union_type : result := union_dcl(con,node);
    nt_enum_type: begin
        result := enum(con,node);
        FDB.register_anonymous(node,result);
      end;
    nt_sequence : begin
        it := get_type(con,node.getBranch(1));
        bound := 0;
        if node.getBranch(2) <> nil then
          bound := node.getBranch(2).integer_literal;
        result := FContainer.create_sequence(bound,it);
        FDB.register_anonymous(node,result);
      end;
    nt_struct_type : result := struct_type(con,node);
    nt_char: result := FContainer.get_primitive(pk_char);
    nt_wide_char: result := FContainer.get_primitive(pk_wchar);
    nt_void: result := FContainer.get_primitive(pk_void);
    nt_octet : result := FContainer.get_primitive(pk_octet);
    nt_float : result := FContainer.get_primitive(pk_float);
    nt_double : result := FContainer.get_primitive(pk_double);
    nt_long_double : result := FContainer.get_primitive(pk_longdouble);
    nt_long: result := FContainer.get_primitive(pk_long);
    nt_longlong: result := FContainer.get_primitive(pk_longlong);
    nt_unsigned_long: result := FContainer.get_primitive(pk_ulong);
    nt_unsigned_longlong: result := FContainer.get_primitive(pk_ulonglong);
    nt_unsigned_short : result := FContainer.get_primitive(pk_ushort);
    nt_short : result := FContainer.get_primitive(pk_short);
    nt_boolean: result := FContainer.get_primitive(pk_boolean);
    nt_string: result := FContainer.get_primitive(pk_string);
    nt_wide_string: result := FContainer.get_primitive(pk_wstring);
    nt_object: result := FContainer.get_primitive(pk_objref);
    nt_any : result := FContainer.get_primitive(pk_any);
    nt_scoped_name: begin
        ident := node.getIdent;
        if (ident[1] = ':') and (ident[2] = ':') then begin
          if con.def_kind = dk_Repository then begin
            Delete(ident, 1, 2);
            cont := con.find(ident);
          end
          else
            cont := FContainer.find(ident);
        end
        else
          cont := con.find(node.getIdent);
        if cont <> nil then
          begin
            result := TIDLType._narrow(cont as IORBObject);
            if result = nil then
              scanner_exception(node,'scoped name ''' +node.getIdent+ ''' not defined');
          end
        else if is_typecode(node) then
          result := FContainer.get_primitive(pk_TypeCode)
        else if is_principal(node) then
          result := FContainer.get_primitive(pk_Principal)
        else
          scanner_exception(node,'scoped name ''' +node.getIdent+ ''' not defined');
      end;
    else
      assert(false,'not imlplemented');
  end;
end;

procedure TScanner.state_member(con: IContainer; node: TNode;
  is_public: Boolean);
var
  v : IValueDef;
  itype: IIDLType;
  decl : TNode;
  vm :  IValueMemberDef;
  vis: TVisibility;
begin
  v := TValueDef._narrow(con as IORBObject);
  assert(v <> nil);
  itype := get_type(con,node.getBranch(1));
  node := node.getBranch(2);
  if is_public then
    vis := PUBLIC_MEMBER
  else
    vis := PRIVATE_MEMBER;
  while node <> nil do
    begin
      assert(node.node_type = nt_declarators);
      decl := node.getBranch(1);
      if decl.node_type = nt_simple_declarator then
         vm := v.create_value_member(gen_repo_id(decl),decl.getIdent,FVersion,itype,vis)
      else
        assert(false,'not imlplemented');
      node := node.getBranch(2);
    end;
end;

function TScanner.struct_type(con: IContainer; node: TNode): IIDLType;
var
  id, name : string;
  idef: IStructDef;
  members: StructMemberSeq;
begin
  assert(node.node_type = nt_struct_type);
  id := gen_repo_id(node);
  name := node.getIdent;
  FDB.enter_scope(name);
  idef := con.create_struct(id,name,FVersion,nil);
  struct_members(idef as IContainer,node.getBranch(1),@members);
  idef.members(members);
  FDB.leave_scope;
  result := idef;
end;

procedure TScanner.struct_member(con: IContainer; node: TNode; mem: PStructMemberSeq);
var
  t : IIDLType;
begin
  if node.node_type = nt_pragma then
    pragma(con,node)
  else if node.node_type = nt_include then
    include(con,node)
  else
    begin
      t := get_type(con,node.getBranch(1));
      struct_member_decls(con,node.getBranch(2),t,mem);
    end;
end;

procedure TScanner.struct_members(con: IContainer; node: TNode; mem: PStructMemberSeq);
begin
  if node = nil then exit;
  assert(node.node_type = nt_member_list);
  struct_member(con,node.getBranch(1),mem);
  node := node.getBranch(2);
  if node <> nil then struct_members(con,node,mem);
end;

procedure TScanner.struct_member_decls(con: IContainer; node: TNode;
  _type: IIDLType; mem: PStructMemberSeq);
begin
  assert(node.node_type = nt_declarators);
  struct_member_decl(con,node.getBranch(1),_type,mem);
  node := node.getBranch(2);
  if node <> nil then struct_member_decls(con,node,_type,mem);
end;

procedure TScanner.struct_member_decl(con: IContainer; node: TNode;
  _type: IIDLType; mem: PStructMemberSeq);
var
  i : integer;
  m: StructMember;
begin
  i := length(mem^);
  setLength(mem^,i+1);
  case node.node_type of
    nt_simple_declarator: begin
        m.name := node.getIdent;
        m.type_def := _type;
        m._type := _tc_void;
        mem^[i] := m;
      end;
    nt_array_declarator: begin
        m.name := node.getIdent;
        m.type_def := array_dcl(con,node.getBranch(1),_type);
        m._type := _tc_void;
        mem^[i] := m;
      end;
    else
      assert(false);
  end;
end;

procedure TScanner.include(con: IContainer; node: TNode);
var
  str: string;
  i : integer;
begin
  str := copy(node.getIdent,6,1000);
  i := pos(' ',str);
  if i < 1 then exit;
  str := copy(str,i+1,1000);
  i := pos(' ',str);
  if i < 1 then exit;
  str := copy(str,1,i-1);
  if StrToInt(str) = 1 then
    FDB.enter_include
  else
    FDB.leave_include;
end;

procedure TScanner.pragma(con: IContainer; node: TNode);
var
  txt, ptype, name, tmp : string;
  i: integer;
  c : IContained;
begin
  if node.node_type() = nt_typeid then begin
    ptype := 'ID';
    name := node.getBranch(1).getIdent();
    txt := node.getIdent();
  end
  else if node.node_type() = nt_typeprefix then begin
    ptype := 'prefix';
    name := node.getBranch(1).getIdent();
    txt := node.getIdent();
  end
  else begin
    assert(node.node_type() = nt_pragma);

    //handle plain old #pragma
    txt := trim(copy(trim(node.getIdent),8,200));
    i := pos(' ',txt);
    if i < 1 then scanner_exception(node,'invalid prefix declaration');
    ptype := copy(txt,1,i-1);
    txt := trim(copy(txt,i+1,200));
    if ptype = 'prefix' then
      begin
        i := pos('"',txt);
        if i < 1 then scanner_exception(node,'invalid prefix declaration');
        txt := copy(txt,i+1,200);
        i := pos('"',txt);
        if i < 1 then scanner_exception(node,'invalid prefix declaration');
        FDB.switch_prefix(copy(txt,1,i-1));
        exit;
      end;
    i := pos(' ',txt);
    if i < 1 then scanner_exception(node,'invalid prefix declaration');
    name := copy(txt,1,i-1);
    txt := copy(txt,i+1,200);
    if name[1] <> ':' then
      begin
        case con.def_kind of
          dk_Repository : tmp := '';
          dk_Module,dk_Interface,dk_Value,dk_Struct,dk_Union,dk_Exception : begin
              c := TContained._narrow(con as IORBObject);
              assert(c <> nil);
              tmp := c.absolute_name;
            end;
          else
            assert(false,'not implemented');
        end;
        name := tmp + '::' + name;
      end;
    if ptype = 'version' then
      begin
        i := 1;
        while (i < length(txt)) and (pos(txt[i],'0123456789.')>0) do inc(i);
        FDB.set_version(name,copy(txt,1,i));
      end
    else if ptype = 'ID' then
      begin
        i := pos('"',txt);
        if i < 1 then scanner_exception(node,'invalid prefix declaration');
        txt := copy(txt,i+1,200);
        i := pos('"',txt);
        if i < 1 then scanner_exception(node,'invalid prefix declaration');
        FDB.set_repo_id(name,copy(txt,1,i-1));
      end
    else
      assert(false,'not implemented');
    Exit;
  end;
  // Strip leading '_'
  if (Length(name) > 0) and (name[1] = '_') then
    Delete(name, 1, 1);
  if (Length(name) > 0) and (name[1] <> ':') then begin
    // Name is relative, make it absolute
    c := con.find(name);
    if c = nil then
      //The name wasn't defined. Assume its relative to the
      //current scope
      name := FDB.get_current_scope() + name
    else
      name := c.absolute_name();
  end;
  if ptype = 'prefix' then begin
    if name = '' then
      FDB.switch_prefix(txt)
    else
      FDB.set_prefix(name, txt);
  end
  else if ptype = 'ID' then
    FDB.set_repo_id(name, txt);
end;

function TScanner.array_dcl(con: IContainer; node: TNode;
  _type: IIDLType): IIDLType;
var
  c : Variant;
  l: longint;
  elem : IIDLType;
begin
  result := nil;
  assert(node.node_type = nt_fixed_array_sizes);
  c := const_val(con,node.getBranch(1));
  l := c;
  if node.getBranch(2) <> nil then
    elem := array_dcl(con,node.getBranch(2),_type)
  else
    elem := _type;
  result := FContainer.create_array(l,elem);
  FDB.register_anonymous(node,result);
end;

procedure TScanner.const_fixed_dcl(con: IContainer; node: TNode);
begin
  assert(false,'not implemented');
end;

procedure TScanner.const_dcl(con: IContainer; node: TNode);
var
  ctype : IIDLType;
  cname,repoid  : string;
  val : Variant;
  t: TNode;
  ct : IContained;
  c : IConstantDef;
  a : IAny;
  alias : IAliasDef;
begin
  t := node.getBranch(1);
  if t.node_type = nt_fixed_pt_type then
    begin
      const_fixed_dcl(con,node);
      exit;
    end;
  if t.node_type = nt_scoped_name then
    begin
      ct := con.find(t.getIdent);
      if ct = nil then scanner_exception(node,'type not found: '+ t.getIdent);
      alias := TAliasDef._narrow(ct as IORBObject);
      if ct = nil then scanner_exception(node,'alias not found: '+ t.getIdent);
      while alias <> nil do
        begin
          ctype := alias.original_type_def;
          alias := TAliasDef._narrow(ctype as IORBObject);
        end;
    end
  else
    ctype := get_type(con,t);
  cname := node.getIdent;
  if not (ctype._type.kind in [
    tk_float,tk_double, tk_long, tk_short,
    tk_ulong, tk_ushort, tk_boolean, tk_string,
    tk_wstring, tk_char, tk_octet, tk_wchar,
    tk_longlong, tk_ulonglong, tk_longdouble]) then
      scanner_exception(node,'bad constant type: '+cname);
  val := const_val(con,node.getBranch(2));
  repoid := gen_repo_id(node);
  a := CreateAny;
  case ctype._type.kind of
    tk_float : a.put_float(val);
    tk_double : a.put_double(val);
    tk_long : a.put_long(longint(val));
    tk_short : a.put_short(short(val));
    tk_ulong : a.put_ulong(_ulong(val));
    tk_ushort : a.put_ushort(_ushort(val));
    tk_boolean : a.put_boolean(Boolean(val));
    tk_string : a.put_string(string(val));
    tk_wstring : a.put_wstring(WideString(val));
    tk_char : a.put_char(PChar(string(val))^);
    tk_octet : a.put_octet(Octet(val));
    tk_wchar : a.put_wchar(PWideChar(WideString(val))^);
    tk_longlong: assert(false,'not implemented');
    tk_ulonglong: assert(false,'not implemented');
    tk_longdouble:assert(false,'not implemented');
  end;
  c := con.create_constant(repoid,node.getIdent,FVersion,ctype,a);
end;


procedure TScanner.typedef(con: IContainer; node: TNode);
var
  itype : IIDLType;
  dcl: TNode;
  id: string;
  a: IAliasDef;
  arr : IIDLType;
begin
  assert(node.node_type = nt_typedef);
  itype := get_type(con,node.getBranch(1));
  node := node.getBranch(2);
  while node <> nil do
    begin
      dcl := node.getBranch(1);
      case dcl.node_type of
        nt_simple_declarator: begin
            id := gen_repo_id(dcl);
            a := con.create_alias(id,dcl.getIdent,FVersion,itype);
          end;
        nt_array_declarator: begin
            id := gen_repo_id(dcl);
            arr := array_dcl(con,dcl.getBranch(1),itype);
            con.create_alias(id,dcl.getIdent,FVersion,arr);
          end;
        else
          assert(false,'not implemented');
      end;
      node := node.getBranch(2);
    end;
end;

procedure TScanner.native(con: IContainer; node: TNode);
var
  dcl: TNode;
  id: string;
begin
  assert(node.node_type = nt_native);
  dcl := node.getBranch(1);
  assert(dcl.node_type = nt_simple_declarator);
  id := gen_repo_id(dcl);
  con.create_native(id, dcl.getIdent, FVersion);
end;

function TScanner.enum(con: IContainer; node: TNode): IIDLType;
var
  mem: EnumMemberSeq;
begin
  enum_members(node.getBranch(1),@mem);
  result := con.create_enum(gen_repo_id(node),node.getIdent,FVersion,@mem);
end;

procedure TScanner.enum_members(node: TNode; mem: PEnumMemberSeq);
var
  i : integer;
begin
  case node.node_type of
    nt_enumerator: begin
        i := length(mem^);
        setLength(mem^,i+1);
        mem^[i] := node.getIdent;
      end;
    nt_enumerators: begin
        enum_members(node.getBranch(1),mem);
        enum_members(node.getBranch(2),mem);
      end;
  end;
end;

procedure TScanner.module(con: IContainer; node: TNode);
var
  id,name: string;
  c : IContained;
  m : IModuleDef;
begin
  id := gen_repo_id(node);
  name := node.getIdent;
  FDB.enter_scope(name);
  c := con.locate_id(id);
  if c <> nil then
    begin
      m := TModuleDef._narrow(c as IORBObject);
      assert(m <> nil);
    end
  else
    m := con.create_module(id,name,Fversion);
  scan(m as IContainer,node.getBranch(1));
  FDB.leave_scope;
end;

procedure TScanner.attributes(con: IContainer; node: TNode;
  readonly: Boolean);
var
  idef : IInterfaceDef;
  vdef : IValueDef;
  itype: IIDLType;
  p : TNode;
  name,repoid : string;
  attr : IAttributeDef;
begin
  idef := TInterfaceDef._narrow(con as IORBObject);
  vdef := TValueDef._narrow(con as IORBObject);
  assert(assigned(idef) or assigned(vdef));
  itype := get_type(con,node.getBranch(1));
  p := node.getBranch(2);
  while true do
    begin
      if p = nil then break;
      name := p.getBranch(1).getIdent;
      repoid := gen_repo_id(p.getBranch(1));
      if idef <> nil then
        attr := idef.create_attribute(repoid,name,FVersion,itype,AttributeMode(integer(readonly)))
      else
        attr := vdef.create_atribute(repoid,name,FVersion,itype,AttributeMode(integer(readonly)));
      p := p.getBranch(2);
    end;
end;

function TScanner.const_val(con: IContainer; node: TNode): Variant;
var
  c1,c2: Variant;
  cont: IContained;
  def: IConstantDef;
begin
  if node = nil then
    result := Null
  else
    begin
      c1 := const_val(con,node.getBranch(1));
      c2 := const_val(con,node.getBranch(2));
      case node.node_type of
        nt_floating_pt_literal : result := node.float_literal;
        nt_fixed_pt_literal : result := node.float_literal;
        nt_integer_literal: result := node.integer_literal;
        nt_string_literal: result := node.string_literal;
        nt_wstring_literal: result := node.wstring_literal;
        nt_character_literal: result := node.character_literal;
        nt_wcharacter_literal: result := WideString(node.wcharacter_literal);
        nt_boolean_literal: result := node.boolean_literal;
        nt_vertical_line: result := c1 or c2;
        nt_circumflex: result := intpower(c1,c2);
        nt_ampersand: result := c1 and c2;
        nt_shiftright: result := c1 shr c2;
        nt_shiftleft: result := c1 shl c2;
        nt_plus_sign: if node.getBranch(2) = nil then
            result := c1
          else
            result := c1 + c2;
        nt_minus_sign: if node.getBranch(2) = nil then
            result := - c1
          else
            result := c1 - c2;
        nt_asterik: result := c1 * c2;
        nt_solidus: result := c1 / c2;
        nt_tilde: result := not c1;
        nt_scoped_name: begin
            cont := con.find(node.getIdent);
            if cont <> nil then
              begin
                def := TConstantDef._narrow(cont as IORBObject);
                result := NULL;
                if def = nil then
                  scanner_exception(node,'constant not defined')
                else begin
                  result := def.value.get_variant();
                  if result = NULL then
                    scanner_exception(node,'can`t decode constant value');
                end;
              end
            else
              scanner_exception(node,Format('%s not found (expected name of a constant)', [node.getIdent]));
              //assert(false,'not imlplemented');
          end;
        else
          begin
            result := NULL;
            assert(false);
          end;
      end;
    end;
end;

function TScanner.union_dcl(con: IContainer; node: TNode): IIDLType;
var
  repoid,name : string;
  descr,odescr: IIDLType;
  members: UnionMemberSeq;
  prim: IPrimitiveDef;
  u : IUnionDef;
begin
  repoid := gen_repo_id(node);
  name := node.getIdent;
  FDB.enter_scope(name);
  descr := get_type(con,node.getBranch(1));
  odescr := TCodebase.original_type(descr);
  // check descriminator type
  case odescr.def_kind of
    dk_Enum:;
    dk_Primitive : begin
      prim := TPrimitiveDef._narrow(odescr as IORBObject);
        case prim.kind of
          pk_short,
          pk_long,
          pk_ushort,
          pk_ulong,
          pk_char,
          pk_wchar,
          pk_boolean,
          pk_longlong,
          pk_ulonglong:;
          else
            scanner_exception(node,'wrong union switch type');
        end;
      end;
    else
       scanner_exception(node,'wrong union switch type');
  end;
  u := con.create_union(repoid,name,FVersion,descr,nil);
  union_members(u as IContainer,node.getBranch(2),descr,@members);
  u.members(members);
  result := u;
  FDB.leave_scope;
end;

procedure TScanner.union_members(con: IContainer; node: TNode; descr: IIDLType; members: PUnionMemberSeq);
begin
  assert(node.node_type = nt_switch_body);
  union_case(con,node.getBranch(1),descr,members);
  if node.getBranch(2)  <> nil then
    union_members(con,node.getBranch(2),descr,members);
end;

procedure TScanner.union_case(con: IContainer; node: TNode; descr: IIDLType; members: PUnionMemberSeq);
var
  member: UnionMember;
  i : integer;
  procedure union_element_spec(node: TNode);
  var
    t: IIDLType;
  begin
    while node.getBranch(3) = nil do
      node := node.getBranch(2);
    node := node.getBranch(3);
    assert(node.node_type = nt_element_spec);
    t := get_type(con,node.getBranch(1));
    node := node.getBranch(2);
    case node.node_type of
      nt_simple_declarator : begin
          member.name := node.getIdent;
          member._type := _tc_void;
          member.type_def := t;
        end;
      nt_array_declarator : begin
          member.name := node.getIdent;
          member._type := _tc_void;
          member.type_def := array_dcl(con,node.getBranch(1),t);
        end;
      else
        assert(false);
    end;
  end;

  procedure union_case_label(node: TNode);
  var
    t : IIDLType;
    enum_tc : ITypeCode;
    scoped_name,_label: string;
    val: Variant;
    prim : IPrimitiveDef;
    str,lpos: PChar;
    i : integer;
  begin
    if node.node_type = nt_default then
      begin
        member._label := CreateAny;
        member._label.put_octet(0);
      end
    else
      begin
        t := TCodebase.original_type(descr);
        case t.def_kind of
          dk_Enum : begin
              if node.node_type <> nt_scoped_name then
                scanner_exception(node,'IDL error');
              enum_tc := t._type.unalias;
              scoped_name := node.getIdent;
              if pos('::',scoped_name) > 0 then
                begin
                  str := PChar(_label);
                  lpos := AnsiStrRScan(str,':');
                  _label := copy(_label,lpos-str+2,1000);
                end
              else
                _label := scoped_name;
              for i := 0 to Pred(enum_tc.member_count) do
                begin
                  if enum_tc.member_name(i) = _label then
                    begin
                      member._label.set_type(enum_tc);
                      member._label.enum_put(i);
                      exit;
                    end;
                end;
              scanner_exception(node,'enumeration value not defined');
            end;
          dk_Primitive : begin
              val := const_val(con,node);
              prim := TPrimitiveDef._narrow(t as IORBObject);
              member._label := CreateAny;
              case prim.kind of
                pk_short: member._label.put_short(val);
                pk_long:member._label.put_long(val);
                pk_ushort:member._label.put_ushort(val);
                pk_ulong:member._label.put_ulong(val);
                pk_char: member._label.put_char(string(val)[1]);
                pk_wchar: member._label.put_wchar(WideString(val)[1]);
                pk_boolean: member._label.put_boolean(val);
                pk_longlong: assert(false,'not implemented');
                pk_ulonglong:assert(false,'not implemented');
                else
                  assert(false,'not implemented');
              end;
            end;
        end;
      end;
  end;

begin
  if node.getBranch(4) <> nil then
    assert(false,'case PRAGMA not implemented');
  assert(node.node_type = nt_case);
  member._label := CreateAny;
  union_element_spec(node);
  union_case_label(node.getBranch(1));
  i := Length(members^);
  setLength(members^,i+1);
  (members^)[i] := member;
  if node.getBranch(2) <> nil then
    union_case(con,node.getBranch(2),descr,members);
end;

procedure TScanner.exception_dcl(con: IContainer; node: TNode);
var
  name, repoid: string;
  ex: IExceptionDef;
  members: StructMemberSeq;
begin
  repoid := gen_repo_id(node);
  name := node.getIdent;
  FDB.enter_scope(name);
  ex := con.create_exception(repoid,name,FVersion,nil);
  struct_members(ex as IContainer,node.getBranch(1),@members);
  ex.members(members);
  FDB.leave_scope;
end;

procedure TScanner.raises_dcl(con: IContainer; node: TNode;
  exceptions: PExceptionDefSeq);
var
  c : IContained;
  ex : IExceptionDef;
  len : integer;
begin
  assert(node.node_type = nt_raises_expr);
  node := node.getBranch(1);
  while node <> nil do
    begin
      c := con.find(node.getBranch(1).getIdent);
      if c = nil then
        scanner_exception(node,'exception not found '+node.getIdent);
      ex := TExceptionDef._narrow(c as IORBObject);
      if ex = nil then
        scanner_exception(node,'can`t cast '+node.getIdent+' to exception');
      len := Length(exceptions^);
      setLength(exceptions^,len+1);
      exceptions^[len] := ex;
      node := node.getBranch(2);
    end;
end;

function TScanner.is_principal(node: TNode): Boolean;
begin
  result :=  node.getIdent = 'Principal'
end;

function TScanner.is_typecode(node: TNode): Boolean;
begin
  result :=  (node.getIdent = 'TypeCode') or  (node.getIdent = 'CORBA::TypeCode')
end;

(*
 * Process import statement:
 *    import "Repository Id";
 *    import <scoped-name>
 * Imports are always at the beginning of a file, outside any scope.
 *)
procedure TScanner.import(con: IContainer; node: TNode);
var
  to_be_imported, fname: string;
  i: Integer;
  found: Boolean;
  old_yyinput, old_yyoutput : TStream;
  old_yyline  : string;
  old_yylineno, old_yycolno : Integer;
  old_yytext  : ShortString;
  old_yyleng  : Byte;
  old_yystate    : Integer;
  old_yyactchar  : Char;
  old_yylastchar : Char;
  old_yyrule     : Integer;
  old_yyreject   : Boolean;
  old_yydone     : Boolean;
  old_yyretval   : Integer;
  old_filename,real_file_name : string;
  old_parser: TParser;

  function IncludeTrailingBackslash(path: string): string;
  begin
    result := path;
    if Trim(path) = '' then exit;
    if (path[Length(path)] <> '/') and (path[Length(path)] <> '\') then
      result := path + '\'
  end;

begin
  assert(node.node_type = nt_import);

  node := node.getBranch(1);
  assert(node <> nil);
  assert((node.node_type = nt_string_literals) or (node.node_type = nt_scoped_name));
  to_be_imported := node.getIdent;

  (*
   * Import statements must be at toplevel. Well, actually, import
   * statements can appear only at the beginning of a file, but
   * adhering to that would screw up the grammar with respect to
   * includes.
   *)

  if (con.def_kind() <> dk_Repository) then
    scanner_exception(40, node);

  { TODO -oOVS : Check Interface Repository }

  // Okay, to_be_imported is a scoped name. Translate this to a file name
  // by replacing '::' with '/'.
  real_file_name := to_be_imported;
  StringReplace(real_file_name, '::', '/', [rfReplaceAll]);
  real_file_name := real_file_name + '.idl';
  fname := real_file_name;

  // Now traverse our include paths and see if we can find this file
  if not FileExists(fname) then begin
    found := False;
    for i := 0 to Pred(includes.count) do begin
      real_file_name := IncludeTrailingBackslash(includes[i]) + fname;
      if FileExists(real_file_name) then begin
        found := true;
        break;
      end;
    end;
    if not found then
      scanner_exception(38, node, to_be_imported);
  end;

  old_yyinput := yyinput;
  old_yyoutput := yyoutput;
  old_yyline  := yyline;
  old_yylineno := yylineno;
  old_yycolno := yycolno;
  old_yytext  := yytext;
  old_yyleng  := yyleng;
  old_yystate  := yystate;
  old_yyactchar  := yyactchar;
  old_yylastchar := yylastchar;
  old_yyrule     := yyrule;
  old_yyreject   := yyreject;
  old_yydone     := yydone;
  old_yyretval   := yyretval;
  old_filename := filename;
  old_parser := theParser;
  try
    filename := real_file_name;
    yylineno := 0;
    yycolno := 0;
    yystate := 0;
    yyclear;

    yyinput := nil;
    yyoutput := nil;

    yyoutput := TMemoryStream.Create;
    yyinput := codeutil.OpenFile(real_file_name);
    try
      yyprintln('#line 1 "'+real_file_name+'"');
      yyinput.position := 0;
      cpp.yylex;
    finally
      yyinput := nil;
    end;
    yyoutput.Position := 0;
    yyinput := yyoutput;
    {$IFDEF MSWINDOWS}
    yyoutput := THandleStream.Create(GetStdHandle(STD_OUTPUT_HANDLE));
    {$ENDIF}
    {$IFDEF LINUX}
    yyoutput := THandleStream.Create(STDOUT_FILENO);
    {$ENDIF}

    theParser := TParser.Create;
    try
      theParser.parse(real_file_name);
      with TScanner.Create(theParser.getRootNode,FContainer,FDB) do
        try
          collect;
        finally
          free;
        end;
    finally
      theParser.Free;
    end;

  finally
    yyclear;
    yyinput.Free;
    yyoutput.Free;
    filename := old_filename;
    yyinput := old_yyinput;
    yyoutput := old_yyoutput;
    yyline  := old_yyline;
    yylineno := old_yylineno;
    yycolno := old_yycolno;
    yytext  := old_yytext;
    yyleng  := old_yyleng;
    yystate  := old_yystate;
    yyactchar  := old_yyactchar;
    yylastchar := old_yylastchar;
    yyrule     := old_yyrule;
    yyreject   := old_yyreject;
    yydone     := old_yydone;
    yyretval   := old_yyretval;
    theParser  := old_parser;
  end;
end;

end.
