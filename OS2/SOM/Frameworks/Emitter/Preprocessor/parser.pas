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
unit parser;

interface

uses
  classes, sysutils;

type

  TNodeType = (
    nt_abstract_forward_dcl,
    nt_abstract_interface_header,
    nt_abstract_value_dcl,
    nt_abstract_value_forward_dcl,
    nt_ampersand,
    nt_any,
    nt_array_declarator,
    nt_asterik,
    nt_attribute,
    nt_boolean,
    nt_boolean_literal,
    nt_case,
    nt_char,
    nt_character_literal,
    nt_wcharacter_literal,
    nt_circumflex,
    nt_const_dcl,
    nt_custom_value_header,
    nt_declarators,
    nt_default,
    nt_definitions,
    nt_double,
    nt_element_spec,
    nt_enum_type,
    nt_enumerator,
    nt_enumerators,
    nt_except_dcl,
    nt_exports,
    nt_fixed_array_sizes,
    nt_fixed_pt_literal,
    nt_fixed_pt_type,
    nt_float,
    nt_floating_pt_literal,
    nt_forward_dcl,
    nt_in,
    nt_include,
    nt_init_dcl,
    nt_init_param_decl,
    nt_init_param_decls,
    nt_inout,
    nt_integer_literal,
    nt_interface_dcl,
    nt_interface_header,
    nt_local_forward_dcl,
    nt_local_interface_header,
    nt_long,
    nt_long_double,
    nt_longlong,
    nt_member,
    nt_member_list,
    nt_minus_sign,
    nt_module,
    nt_native,
    nt_object,
    nt_octet,
    nt_oneway,
    nt_op_dcl,
    nt_op_type_spec,
    nt_out,
    nt_param_dcl,
    nt_param_dcls,
    nt_percent_sign,
    nt_plus_sign,
    nt_pragma,
    nt_private_state_member,
    nt_public_state_member,
    nt_raises_expr,
    nt_readonly_attribute,
    nt_scoped_name,
    nt_scoped_names,
    nt_sequence,
    nt_shiftleft,
    nt_shiftright,
    nt_short,
    nt_simple_declarator,
    nt_simple_declarators,
    nt_solidus,
    nt_string,
    nt_string_literal,
    nt_string_literals,
    nt_wstring_literal,
    nt_struct_type,
    nt_switch_body,
    nt_tilde,
    nt_typedef,
    nt_union_type,
    nt_unsigned_long,
    nt_unsigned_longlong,
    nt_unsigned_short,
    nt_value_box_dcl,
    nt_value_dcl,
    nt_value_elements,
    nt_value_forward_dcl,
    nt_value_header,
    nt_value_inheritance_spec,
    nt_valuebase,
    nt_vertical_line,
    nt_void,
    nt_wide_char,
    nt_wide_string,
    nt_import,
    nt_typeid,
    nt_typeprefix
  );

  TNode = class
  private
    FBranches : array[1..5] of TNode;
    FNodeType : TNodeType;
    FIdent : string;
    FStringLiteral: string;
    FWStringLiteral: string;
    FIntegerLiteral : longint;
    FCharacterLiteral : char;
    FWCharacterLiteral : char;
    FBooleanLiteral : boolean;
    FDouble : double;
    FLineNumber: integer;
    FFileName: string;
    FTopLevel : Boolean;
  public
    constructor Create(nodetype: TNodeType; sub: TNode; sub2: TNode;
      sub3: TNode; sub4: TNode;sub5: TNode);
    destructor Destroy; override;
    procedure setBranch(index: integer; node: TNode);
    function getBranch(index: integer): TNode;
    procedure setIdent(ident: string);
    function getIdent: string;
    procedure setIntegerLiteral(int: integer);
    procedure setStringLiteral(str: string);
    procedure setWStringLiteral(str: string);
    procedure setWCharacterLiteral(ch: char);
    procedure setCharacterLiteral(ch: char);
    procedure setFixedPtLiteral(fixed: double);
    procedure setFloatingPtLiteral(fl: double);
    procedure setBooleanLiteral(b: Boolean);
    function node_type: TNodeType;
    property integer_literal : longint read FIntegerLiteral;
    property character_literal : char read FCharacterLiteral;
    property wcharacter_literal : char read FWCharacterLiteral;
    property boolean_literal : boolean read FBooleanLiteral;
    property float_literal : double read FDouble;
    property string_literal : string read FStringLiteral;
    property wstring_literal : string read FWStringLiteral;
    property top_level : Boolean read FTopLevel;
    property line_number: integer read FLineNumber;
    property file_name : string read FFileName;
  end;

  TParser = class
  private
    FRootNode: TNode;
  public
    destructor Destroy; override;
    procedure setRootNode(node: TNode);
    function getRootNode: TNode;
    procedure parse(fname: string);
  end;

  node = TNode;
  _char = char;
  _wchar = char;
  _fixed = double;
  _float = double;
  _int = longint;
  _str = string[200];
  _wstr = {array [0..199] of WideChar;//}string[200];
  ident = string[200];

  procedure preprocessor_directive( text: string );


var
  theParser : TParser;
  idl_line_no: integer;
  toplevel_file,current_file: string;

implementation

uses
  idl,lexlib,yacclib;

procedure preprocessor_directive( text: string );
var
  i : integer;
  str: string;
begin
  if text[1] <> '#' then exit;
  str := copy(text,7,1000);
  i := 1;
  while i < Length(str) do
    begin
      if pos(str[i],'0123456789') = 0 then
        begin
          idl_line_no := StrToInt(copy(str,1,i-1));
          str := copy(str,i,1000);
          break;
        end;
      inc(i);
    end;
  str := copy(str,pos('"',str)+1,1000);
  current_file := copy(str,1,pos('"',str)-1);
end;

//*********************************************************************
//  TParser
//*********************************************************************
destructor TParser.Destroy;
begin
  FRootNode.Free;
end;

procedure TParser.parse(fname: string);
begin
  yyclear;
  toplevel_file := fname;
  current_file := fname;
  yyparse;
end;

function TParser.getRootNode: TNode;
begin
  Result := FRootNode
end;

procedure TParser.setRootNode(node: TNode);
begin
  FRootNode.Free;
  FRootNode := node;
end;

//*********************************************************************
// TNode
//*********************************************************************
constructor TNode.Create(nodetype: TNodeType; sub: TNode; sub2: TNode;
      sub3: TNode; sub4: TNode;sub5: TNode);
begin
  FNodeType := nodetype;
  FBranches[1] := sub;
  FBranches[2] := sub2;
  FBranches[3] := sub3;
  FBranches[4] := sub4;
  FBranches[5] := sub5;
  FLineNumber := idl_line_no;
  FFileName := current_file;
  FTopLevel := current_file = toplevel_file;
end;

destructor TNode.Destroy;
begin
  FBranches[1].Free;
  FBranches[2].Free;
  FBranches[3].Free;
  FBranches[4].Free;
  FBranches[5].Free;
end;

function TNode.getBranch(index: integer): TNode;
begin
  result := FBranches[index];
end;

function TNode.getIdent: string;
begin
  result := FIdent;
end;

procedure TNode.setBooleanLiteral(b: Boolean);
begin
  FBooleanLiteral := b
end;

procedure TNode.setBranch(index: integer; node: TNode);
begin
  FBranches[index].Free;
  FBranches[index] := node;
end;

procedure TNode.setCharacterLiteral(ch: char);
begin
  FCharacterLiteral := ch
end;

procedure TNode.setFixedPtLiteral(fixed: double);
begin
  FDouble := fixed;
end;

procedure TNode.setFloatingPtLiteral(fl: double);
begin
  FDouble := fl;
end;

procedure TNode.setIdent(ident: string);
begin
  FIdent := ident
end;

procedure TNode.setIntegerLiteral(int: integer);
begin
  FIntegerLiteral := int;
end;

procedure TNode.setStringLiteral(str: string);
begin
  FStringLiteral := str
end;

function TNode.node_type: TNodeType;
begin
  result := FNodeType
end;

procedure TNode.setWStringLiteral(str: string);
begin
  FWStringLiteral := str
end;

procedure TNode.setWCharacterLiteral(ch: char);
begin
  FWCharacterLiteral := ch
end;

end.
