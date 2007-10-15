{***********************************************************}
{   This file is part of the DORB project.                  }
{                                                           }
{   See the file License, included in this distribution,    }
{   for details about the copyright.                        }
{                                                           }
{   Send comments and/or bug reports to: vic@inec.ru        }
{***********************************************************}
unit codegen;

interface

uses
  parser,classes,sysutils,ir_int,codeunit;

type



  TCodegen = class
  private
    intunit,implunit: TUnit;
    FContainer: IContainer;
    FMarshallers : TStrings;
    FIntDeclarations : TStrings;
  protected
    function scoped_names(node: TNode): string;
    procedure forward_dcl(node: TNode);
    procedure int_exports(node: TNode; list: TStrings);
    procedure int_export(node: TNode; list: TStrings);
    procedure write_header(s: TFileStream; fname: string);
    procedure definitions(node: TNode);
    procedure definition(node: TNode);
    procedure interface_dcl(node: TNode);
    function operation_dcl(node: TNode): string;
    function parameter_dcls(node: TNode): string;
    function parameter_dcl(node: TNode): string;
    function param_type_spec(node: TNode): string;
    function op_type_spec(node: TNode): string;
    function string_type(node: TNode): string;
    function wstring_type(node: TNode): string;
    procedure method_stub(decl: string;node: TNode;cname: string);
    function marshaller_for(node: TNode): string;
    procedure enum_params(params: TNode; list: TStrings);
    procedure addMarshaller(name: string);
    procedure marshallers();
    procedure new_marshaller(node: TNode);
    procedure skel_impl(node: TNode);
    function safe_name(name: string): string;
    procedure value_dcl(node: TNode);
    procedure enumerate(node: TNode; list: TList);
  public
    constructor Create(con: IContainer);
    destructor Destroy(); override;
    procedure generate(name: string);
  end;



implementation


const
  stub_suffix = '_stub';
  skel_suffix = '_skel';
  factory_suffix = '_factory';

//*********************************************************************
//  TCodegen
//*********************************************************************
constructor TCodegen.Create(con: IContainer);
begin
  FContainer := con;
  intunit := TUnit.Create();
  implunit := TUnit.Create();
  FMarshallers := TStringList.Create;
  FIntDeclarations := TStringList.Create;
end;

destructor TCodegen.Destroy;
begin
  intunit.free;
  implunit.free;
  FMarshallers.Free;
  FIntDeclarations.free;
end;

procedure TCodegen.generate(name: string);
var
  intname,implname: string;
begin
  intname := name+'_int';
  implname := name;
  intunit.save(name+'_int');
  implunit.save(name);
  // form interface unit
{  write_header(int,intname);
  int.println('uses');
  int.println('  ORBTypes;');
  int.println;
  int.println('type');
  int.println;

  // form code interface part
  write_header(codeint,implname);
  codeint.println('uses');
  codeint.println('  orb_int,orb,req_int,code_int,imr,imr_int,env_int,except_int,ORBTypes,');
  codeint.println('  stdstat,poa,poa_int,'+intname+';');
  codeint.println;
  codeint.println('type');
  codeint.println;

  // form code implementation part
  codeimpl.println('implementation');
  codeimpl.println;
  codeimpl.println('uses');
  codeimpl.println('  exceptions,static,throw;');
  codeimpl.println;

  //definitions(FNode);
  marshallers();

  // finalize interface unit
  int.println;
  int.println('implementation');
  int.println('end.');
  int.SaveToFile(intname+'.pas');

  // finalize implementation unit
  codeimpl.println('end.');
  codeimpl.SaveToStream(codeint);
  codeint.SaveToFile(implname+'.pas');}
end;

procedure TCodegen.interface_dcl(node: TNode);
var
  dname,cname,iname,skel_name,stub_name : string;
  inheritance,idlname,idlname_str: string;
  methods : TStrings;
  i,ind : integer;
begin
{  methods := TStringList.Create;
  try
    dname := node.getBranch(1).getIdent;
    FIntDeclarations.addObject(dname,node);
    iname := 'I'+dname;
    cname := 'T'+dname;
    idlname := 'IDL:'+dname+':1.0';
    idlname_str := chr(39)+idlname+chr(39);
    stub_name := cname+stub_suffix;
    skel_name := cname+skel_suffix;
    inheritance := scoped_names(node.getBranch(1).getBranch(1));
    // interface unit
    int.print('    '+iname+ ' = interface');
    if (pos(',',inheritance) < 1) and (inheritance <> '') then
      int.println('('+inheritance+')')
    else
      int.println;
    if node.getBranch(2) <> nil then
      begin
        int_exports(node.getBranch(2),methods);
        for i := 0 to Pred(methods.count) do
          int.println('      '+methods[i]);
      end;
    int.println('    end;');
    int.println;

    // base class
    // declaration
    codeint.println('    '+cname+' = class(TORBObject,IORBObject,'+iname+')');
    codeint.println('    protected');
    for i := 0 to Pred(methods.count) do
      codeint.println('      '+methods[i]+' virtual;');
    codeint.println('      function narrow_helper(const str: string): Pointer; override;');
    codeint.println('    public');
    codeint.println('      class function _narrow(const obj : IORBObject): '+iname+';');
    codeint.println('      class function narrow_helper2(const obj: IORBObject): Boolean;');
    codeint.println('    end;');
    codeint.println;
    // implementation
    codeimpl.println('//*********************************************************************');
    codeimpl.println('// '+cname);
    codeimpl.println('//*********************************************************************');
    for i := 0 to Pred(methods.count) do
      begin
        ind := pos(' ',methods[i]);
        codeimpl.println(copy(methods[i],1,ind)+cname+'.'+copy(methods[i],ind+1,100));
        codeimpl.println('begin');
        codeimpl.println('  // must be overriden in descendant class');
        codeimpl.println('end;');
        codeimpl.println;
      end;
    codeimpl.println('function '+cname+'.narrow_helper(const str: string): Pointer;');
    codeimpl.println('begin');
    codeimpl.println('  if str = '+idlname_str+ ' then');
    codeimpl.println('    result := Pointer('+iname+'(self)'+')');
    codeimpl.println('  else');
    codeimpl.println('    result := nil;');
    codeimpl.println('end;');
    codeimpl.println;
    codeimpl.println('class function '+cname+'._narrow(const obj : IORBObject): '+iname+';');
    codeimpl.println('var');
    codeimpl.println('  p: Pointer;');
    codeimpl.println('  stub: '+stub_name+';');
    codeimpl.println('begin');
    codeimpl.println('  result := nil;');
    codeimpl.println('  if obj = nil then exit;');
    codeimpl.println('  p := obj.narrow_helper('+idlname_str+');');
    codeimpl.println('  if p <> nil then');
    codeimpl.println('    result := '+iname+'(p)');
    codeimpl.println('  else');
    codeimpl.println('    begin');
    codeimpl.println('      if narrow_helper2(obj) or obj._is_a_remote('+idlname_str+') then');
    codeimpl.println('        begin');
    codeimpl.println('          stub := '+stub_name+'.Create();');
    codeimpl.println('          stub.assign(obj);');
    codeimpl.println('          result := stub;');
    codeimpl.println('        end;');
    codeimpl.println('    end;');
    codeimpl.println('end;');
    codeimpl.println;
    codeimpl.println('class function '+cname+'.narrow_helper2(const obj: IORBObject): Boolean;');
    codeimpl.println('begin');
    codeimpl.println('  result := obj.repoid = '+idlname_str);
    codeimpl.println('end;');
    codeimpl.println;

    // stub
    // declaration
    if inheritance <> '' then
      inheritance := iname+','+inheritance
    else
      inheritance := iname;
    codeint.println('    '+stub_name+' = class('+cname+','+inheritance+')');
    for i := 0 to Pred(methods.count) do
      codeint.println('      '+methods[i]+' override;');
    codeint.println('    end;');
    codeint.println;
    // implementation
    if methods.count > 0 then
      begin
        codeimpl.println('//*********************************************************************');
        codeimpl.println('// '+stub_name);
        codeimpl.println('//*********************************************************************');
        for i := 0 to Pred(methods.count) do
          method_stub(methods[i],TNode(methods.objects[i]),stub_name);
      end;

    // skel
    //declaration
    codeint.println('    '+skel_name+' = class(TStaticMethodDispatcher,IStaticInterfaceDispatcher,'+iname+')');
    codeint.println('    protected');
    for i := 0 to Pred(methods.count) do
      codeint.println('      '+methods[i]+' virtual; abstract;');
    codeint.println('      function _dispatch(const req: IStaticServerRequest): Boolean;');
    codeint.println('    end;');
    codeint.println;
    // implementation
    skel_impl(node);
  finally
    methods.Free;
  end;
  }
end;

procedure TCodegen.definitions(node: TNode);
var
  list: TList;
  i : integer;
begin
  list := TList.Create;
  try
    enumerate(node,list);
    for i := 0 to Pred(list.count) do
      definition(TNode(list[i]));
  finally
    list.free;
  end;
end;

procedure TCodegen.definition(node: TNode);
begin
  if node = nil then exit;
  case node.node_type of
    nt_interface_dcl: interface_dcl(node);
    nt_forward_dcl: forward_dcl(node);
    nt_value_dcl: value_dcl(node);
    else
      assert(false,'not implemented');
  end;
end;

procedure TCodegen.forward_dcl(node: TNode);
begin
{  int.println('    I'+node.getIdent+ ' = interface;');
  int.println
  }
end;

procedure TCodegen.write_header(s: TFileStream; fname: string);
begin
{  s.println('// Generated by DORB idl2pas compiler, '+FormatDateTime('dd/mm/yyyy',now));
  s.println('unit '+fname+';');
  s.println;
  s.println('interface');
  s.println;
  }
end;

procedure TCodegen.int_export(node: TNode; list: TStrings);
begin
  if node = nil then exit;
  case node.node_type of
    nt_op_dcl: list.addObject(operation_dcl(node),node);
  end;
end;

procedure TCodegen.int_exports(node: TNode; list: TStrings);
begin
  if node = nil then exit;
  int_export(node.getBranch(1),list);
  if node.getBranch(2) <> nil then
    int_exports(node.getBranch(2),list);
end;

function TCodegen.parameter_dcl(node: TNode): string;
begin
  result := '';
  if node.getBranch(1).node_type = nt_out then
    result := 'out '
  else
    result := 'const ';
  AppendStr(result,node.getBranch(3).getIdent+': '+param_type_spec(node.getBranch(2)));
end;

function TCodegen.parameter_dcls(node: TNode): string;
begin
  result := '';
  if node = nil then exit;
  result := parameter_dcl(node.getBranch(1));
  if node.getBranch(2) <> nil then
    AppendStr(result,'; '+parameter_dcls(node.getBranch(2)));
end;

function TCodegen.op_type_spec(node: TNode): string;
begin
  result := param_type_spec(node.getBranch(1));
end;

function TCodegen.string_type(node: TNode): string;
begin
  if node.getBranch(1) = nil then
    result := 'string'
  else
    result := 'string['+IntToStr(node.getBranch(1).getIntegerLiteralt)+']';
end;

function TCodegen.wstring_type(node: TNode): string;
begin
  if node.getBranch(1) = nil then
    result := 'widestring'
  else
    result := 'widestring['+IntToStr(node.getBranch(1).getIntegerLiteralt)+']';
end;

function TCodegen.scoped_names(node: TNode): string;
begin
  result := '';
  if node = nil then exit;
  result := 'I'+node.getBranch(1).getIdent;
  if node.getBranch(2) <> nil then
    result := ', ' + scoped_names(node.getBranch(2));
end;

function TCodegen.operation_dcl(node: TNode): string;
var
  temp : string;
begin
  result := '';
  if node = nil then exit;
  if node.getBranch(2).node_type = nt_void then
    result := 'procedure '
  else
    result := 'function ';
  AppendStr(result,safe_name(node.getIdent));
  temp := parameter_dcls(node.getBranch(3));
  if temp <> '' then appendStr(result,'('+temp+')');
  if node.getBranch(2).node_type <> nt_void then
    AppendStr(result,': '+op_type_spec(node.getBranch(2)));
  appendStr(result,';');
end;

procedure TCodegen.method_stub(decl: string; node: TNode; cname: string);
var
  func: Boolean;
  i :integer;
  plist : TStringList;
begin
{  func :=  node.getBranch(2).node_type <> nt_void;
  if func then
    codeimpl.print('function ')
  else
    codeimpl.print('procedure ');
  codeimpl.println(cname+'.'+copy(decl,pos(' ',decl)+1,100));
  codeimpl.println('var');
  codeimpl.println('  req : IStaticRequest;');
  plist := TStringList.Create;
  try
    enum_params(node.getBranch(3),plist);
    for i := 0 to Pred(plist.count) do
      codeimpl.println('  _'+plist[i]+': IStaticAny;');
    if func then
      codeimpl.println('  _result : IStaticAny;');
    codeimpl.println('begin');
    for i := 0 to Pred(plist.count) do
      with TNode(plist.objects[i]) do
        codeimpl.println('  _'+plist[i]+' := TStaticAny.Create('+
          marshaller_for(getBranch(2))+',@'+getBranch(3).getIdent+');');
    if func then
      codeimpl.println('  _result := TStaticAny.Create('+
          marshaller_for(node.getBranch(2).getBranch(1))+');');
    codeimpl.println('  req := TStaticRequest.Create(self,'+chr(39)+node.getIdent+chr(39)+');');
    for i := 0 to Pred(plist.count) do
      with TNode(plist.objects[i]) do
        begin
          codeimpl.print('  req.add_');
          case getBranch(1).node_type of
            nt_in: codeimpl.print('in');
            nt_out: codeimpl.print('out');
            nt_inout: codeimpl.print('inout');
          end;
          codeimpl.println('_arg(_'+getBranch(3).getIdent+');');
        end;
  finally
    plist.free;
  end;
  if func then
    codeimpl.println('  req.set_result(_result);');
  codeimpl.println('  req.invoke;');
  codeimpl.println('  dorb_static_throw(req);');
  if func then
    codeimpl.println('  result := '+param_type_spec(node.getBranch(2).getBranch(1))+'(_result.value^);');
  codeimpl.println('end;');
  codeimpl.println;
  }
end;

procedure TCodegen.enum_params(params: TNode; list: TStrings);
var
  param: TNode;
begin
  if params = nil then exit;
  while true do
    begin
      param := params.getBranch(1);
      list.addObject(param.getBranch(3).getIdent,param);
      params := params.getBranch(2);
      if params = nil then break;
    end;
end;

procedure TCodegen.addMarshaller(name: string);
begin
  if FMarshallers.IndexOf(name) < 0 then
    FMarshallers.Add(name);
end;

procedure TCodegen.marshallers();
var
  i,ind : integer;
begin
{  if FMarshallers.count = 0 then exit;
  for i := 0 to Pred(FMarshallers.count) do
    begin
      ind := FIntDeclarations.IndexOf(FMarshallers[i]);
      if ind >= 0 then
        new_marshaller(TNode(FIntDeclarations.objects[ind]));
    end;
  codeint.println('var');
  codeimpl.println('initialization');
  for i := 0 to Pred(FMarshallers.count) do
    begin
      codeint.println('  '+FMarshallers[i]+'_marshaller : IStaticTypeInfo;');
      codeimpl.println('  '+FMarshallers[i]+ '_marshaller := T'+FMarshallers[i]+'_marshaller.Create;');
    end;
  codeint.println;}
end;

procedure TCodegen.new_marshaller(node: TNode);
var
  mname,dname,cname,iname : string;
begin
{  dname := node.getBranch(1).getIdent;
  iname := 'I' + dname;
  cname := 'T' + dname;
  mname := cname + '_marshaller';
  // interface part
  codeint.println('    '+mname+' = class(TAbstractStat,IStaticTypeInfo)');
  codeint.println('    protected');
  codeint.println('      function demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  codeint.println('      procedure marshal(enc: IEncoder; const addr : Pointer);');
  codeint.println('      function typecode(): ITypeCode;');
  codeint.println('      procedure _create(var addr: Pointer); override;');
  codeint.println('      procedure _free(var addr: Pointer);');
  codeint.println('    end;');
  codeint.println;
  // implementation part
  codeimpl.println('//*********************************************************************');
  codeimpl.println('// '+mname);
  codeimpl.println('//*********************************************************************');
  codeimpl.println('function '+mname+'.demarshal(dec: IDecoder;const addr : Pointer): Boolean;');
  codeimpl.println('var');
  codeimpl.println('  obj: IORBObject;');
  codeimpl.println('begin');
  codeimpl.println('  result := false;');
  codeimpl.println('  if not stat_Object.demarshal(dec,@obj) then exit;');
  codeimpl.println('  '+iname+'(addr^) := '+cname+'._narrow(obj);');
  codeimpl.println('  result := (obj = nil) or (Pointer(addr^) <> nil);');
  codeimpl.println('end;');
  codeimpl.println;
  codeimpl.println('procedure '+mname+'.marshal(enc: IEncoder;const addr : Pointer);');
  codeimpl.println('begin');
  codeimpl.println('  stat_Object.marshal(enc,addr);');
  codeimpl.println('end;');
  codeimpl.println;
  codeimpl.println('procedure '+mname+'._create(var addr: Pointer);');
  codeimpl.println('begin');
  codeimpl.println('  '+iname+'(addr^) := '+cname+'.Create();');
  codeimpl.println('end;');
  codeimpl.println;
  codeimpl.println('procedure '+mname+'._free(var addr: Pointer);');
  codeimpl.println('begin');
  codeimpl.println('  '+iname+'(addr^) := nil;');
  codeimpl.println('end;');
  codeimpl.println;
  codeimpl.println('function '+mname+'.typecode: ITypeCode;');
  codeimpl.println('begin');
  codeimpl.println('  // not implemented');
  codeimpl.println('  result := nil;');
  codeimpl.println('end;');
  codeimpl.println;}
end;

procedure TCodegen.skel_impl(node: TNode);
var
  dname,cname,iname,skel_name: string;
  mlist,plist,allparams : TStrings;
  i,j : integer;
  method : TNode;
begin
{  dname := node.getBranch(1).getIdent;
  iname := 'I'+dname;
  cname := 'T'+dname;
  skel_name := cname+skel_suffix;
  codeimpl.println('//*********************************************************************');
  codeimpl.println('// '+skel_name);
  codeimpl.println('//*********************************************************************');
  codeimpl.println('function '+skel_name+'._dispatch(const req: IStaticServerRequest): Boolean;');
  mlist := TStringList.Create;
  plist := TStringList.Create;
  allparams := TStringList.Create;
  try
    // get methods
    int_exports(node.getBranch(2),mlist);
    // collect vars
    for i := 0 to Pred(mlist.count) do
      begin
        method := TNode(mlist.objects[i]);
        if method.getBranch(2).node_type <> nt_void then
          allparams.addObject('_'+safe_name(method.getIdent),method.getBranch(2).getBranch(1));
        enum_params(method.getBranch(3),plist);
        for j := 0 to Pred(plist.count) do
          if allparams.IndexOf(plist[j]) < 0 then
            allparams.AddObject(plist[j],TNode(plist.objects[j]).getBranch(2));
        plist.clear;
      end;
    if allparams.count > 0 then
      begin
        // variables
        codeimpl.println('var');
        for i := 0 to Pred(allparams.count) do
          begin
            codeimpl.println('  '+allparams[i]+' : '+
              param_type_spec(TNode(allparams.objects[i]))+';');
            codeimpl.println('  _'+allparams[i]+' : IStaticAny;');
          end;
      end;
    codeimpl.println('begin');
    codeimpl.println('  result := true;');
    // operations
    for i := 0 to Pred(mlist.count) do
      begin
        method := TNode(mlist.objects[i]);
        if i = 0 then
          codeimpl.print('  if ')
        else
          begin
            codeimpl.println;
            codeimpl.print('  else if ');
          end;
        codeimpl.println('req.op_name = '+chr(39)+method.getIdent+chr(39)+' then');
        codeimpl.println('    begin');
        enum_params(method.getBranch(3),plist);
        for j := 0 to Pred(plist.count) do
          begin
            codeimpl.println('      _'+plist[j]+
              ' := TStaticAny.Create('+
              marshaller_for(TNode(plist.objects[j]).getBranch(2))+
              ',@'+plist[j]+');');
          end;
        if method.getBranch(2).node_type <> nt_void then
          begin
            codeimpl.println('      __'+safe_name(method.getIdent)+' := TStaticAny.Create('+
              marshaller_for(method.getBranch(2).getBranch(1))+
              ',@_'+safe_name(method.getIdent)+');');
            codeimpl.println('      req.set_result(__'+safe_name(method.getIdent)+');');
          end;
        codeimpl.println('      if not req.read_args then exit;');
        if method.getBranch(2).node_type <> nt_void then
          codeimpl.print('      _'+safe_name(method.getIdent)+ ' := ')
        else
          codeimpl.print('      ');
        codeimpl.print(safe_name(method.getIdent));
        if plist.count > 0 then
          begin
            codeimpl.print('(');
            for j := 0 to Pred(plist.count) do
              begin
                if j = 0 then
                  codeimpl.print(plist[j])
                else
                  codeimpl.print(', '+plist[j]);
              end;
            codeimpl.print(')');
          end;
        codeimpl.println(';');
        codeimpl.println('      req.write_results();');
        codeimpl.print('    end');
        plist.clear;
      end;
    if mlist.count > 0 then
      codeimpl.println(';');
  finally
    mlist.free;
    plist.free;
  end;
  codeimpl.println('end;');
  codeimpl.println
  }
end;

function TCodegen.safe_name(name: string): string;
begin
  result := name;
  if result = 'create' then
    result := '_create'
  else if result = 'destroy' then
    result := '_destroy';
end;

function TCodegen.marshaller_for(node: TNode): string;
begin
  case node.node_type of
    nt_long : result := 'stat_long';
    nt_unsigned_long : result := 'stat_ulong';
    nt_object: result := 'stat_object';
    nt_any: result := 'stat_any';
    nt_string : result := 'stat_string';
    nt_boolean : result := 'stat_boolean';
    nt_scoped_name : begin
        result := node.getIdent+'_marshaller';
        addMarshaller(node.getIdent);
      end;
  end;
end;

function TCodegen.param_type_spec(node: TNode): string;
begin
  result := '';
  if node = nil then exit;
  case node.node_type of
    nt_string: result := string_type(node);
    nt_wide_string: result := wstring_type(node);
    nt_long : result := 'longint';
    nt_boolean : result := 'boolean';
    nt_short : result := 'smallint';
    nt_unsigned_short : result := '_ushort';
    nt_unsigned_long : result := '_ulong';
    nt_scoped_name : result := 'I'+node.getIdent;
    else
      assert(false,'not imlplemented');
  end;
end;

procedure TCodegen.value_dcl(node: TNode);
var
  elements : TList;
  dname,cname,iname,idlname,idlname_str: string;
  finame,fcname,inheritance, decl_type,decl_name: string;
  i,j,passed : integer;
  factories : TList;
  all_methods,properties : TStringList;
  declarators : TList;
begin
{  elements := TList.Create;
  factories := TList.Create;
  all_methods := TStringList.Create;
  properties := TStringList.Create;
  declarators := TList.Create;
  try
    dname := node.getBranch(1).getIdent;
    FIntDeclarations.addObject(dname,node);
    iname := 'I'+dname;
    cname := 'T'+dname;
    idlname := 'IDL:'+dname+':1.0';
    idlname_str := chr(39)+idlname+chr(39);
    finame := iname+ factory_suffix;
    fcname := cname+ factory_suffix;
    // interface unit
    int.println('    '+iname+ ' = interface');
    enumerate(node.getBranch(2),elements);
    passed := 0;
    for i := 0 to Pred(elements.count) do
      case TNode(elements[i]).node_type of
        nt_init_dcl: factories.add(elements[i]);
        nt_public_state_member: begin
            enumerate(TNode(elements[i]).getBranch(2),declarators);
            decl_type := param_type_spec(TNode(elements[i]).getBranch(1));
            // properties
            for j := passed to Pred(declarators.count) do
              begin
                decl_name := TNode(declarators[j]).getIdent;
                all_methods.addObject('function get_'+decl_name+': '+decl_type+';',nil);
                all_methods.addObject('procedure set_'+decl_name+'(const value: '+decl_type+');',nil);
                properties.add('property '+decl_name+': '+decl_type+
                  ' read get_'+decl_name+
                  ' write set_'+decl_name+';');
              end;
            passed := declarators.count;  
          end;
        nt_private_state_member: begin
            enumerate(TNode(elements[i]).getBranch(2),declarators);
            passed := declarators.count;
          end;
        else // export
          int_export(TNode(elements[i]),all_methods);
      end;
    for i := 0 to Pred(all_methods.count) do
      int.println('      '+all_methods[i]);
    for i := 0 to Pred(properties.count) do
      int.println('      '+properties[i]);
    int.println('    end;');
    int.println;
  finally
    elements.free;
    factories.free;
    all_methods.free;
    declarators.free;
    properties.free;
  end;
  }
end;

procedure TCodegen.enumerate(node: TNode; list: TList);
begin
{  if node = nil then exit;
  list.Add(node.getBranch(1));
  if node.getBranch(2) <> nil then
    enumerate(node.getBranch(2),list);
    }
end;

end.
