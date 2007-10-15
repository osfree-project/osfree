%{
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
unit idl;

interface

uses Yacclib, Lexlib;

  function yyparse : Integer;

implementation

uses
  parser,SysUtils;

%}


%token T_AMPERSAND
%token T_ANY
%token T_ASTERISK
%token T_ATTRIBUTE
%token T_BOOLEAN
%token T_CASE
%token T_CHAR
%token <_char> T_CHARACTER_LITERAL
%token <_wchar> T_WCHARACTER_LITERAL
%token T_CIRCUMFLEX
%token T_COLON
%token T_COMMA
%token T_CONST
%token T_CONTEXT
%token T_DEFAULT
%token T_DOUBLE
%token T_ENUM
%token T_EQUAL
%token T_EXCEPTION
%token T_FALSE
%token T_FIXED
%token <_fixed> T_FIXED_PT_LITERAL
%token T_FLOAT
%token <_float> T_FLOATING_PT_LITERAL
%token T_GREATER_THAN_SIGN
%token <ident> T_IDENTIFIER
%token T_IN
%token T_INOUT
%token <_int> T_INTEGER_LITERAL
%token T_INTERFACE
%token T_LEFT_CURLY_BRACKET
%token T_LEFT_PARANTHESIS
%token T_LEFT_SQUARE_BRACKET
%token T_LESS_THAN_SIGN
%token T_LONG
%token T_MINUS_SIGN
%token T_MODULE
%token T_OCTET
%token T_ONEWAY
%token T_OUT
%token T_PERCENT_SIGN
%token T_PLUS_SIGN
%token T_RAISES
%token T_READONLY
%token T_RIGHT_CURLY_BRACKET
%token T_RIGHT_PARANTHESIS
%token T_RIGHT_SQUARE_BRACKET
%token T_SCOPE
%token T_SEMICOLON
%token T_SEQUENCE
%token T_SHIFTLEFT
%token T_SHIFTRIGHT
%token T_SHORT
%token T_SOLIDUS
%token T_STRING
%token <_str> T_STRING_LITERAL
%token <_wstr> T_WSTRING_LITERAL
%token <_str> T_PRAGMA
%token <_str> T_INCLUDE
%token T_STRUCT
%token T_SWITCH
%token T_TILDE
%token T_TRUE
%token T_OBJECT
%token T_TYPEDEF
%token T_UNION
%token T_UNSIGNED
%token T_VERTICAL_LINE
%token T_VOID
%token T_WCHAR
%token T_WSTRING
%token T_UNKNOWN
%token T_ABSTRACT
%token T_VALUETYPE
%token T_TRUNCATABLE
%token T_SUPPORTS
%token T_CUSTOM
%token T_PUBLIC
%token T_PRIVATE
%token T_FACTORY
%token T_NATIVE
%token T_VALUEBASE
%token T_IMPORT
%token T_TYPEID
%token T_TYPEPREFIX
%token T_LOCAL

%type <node> definitions definition module interface interface_dcl
%type <node> forward_dcl interface_header interface_body exports export
%type <node> interface_inheritance_spec interface_names scoped_names
%type <node> scoped_name value_forward_dcl value value_box_dcl value_abs_dcl
%type <node> value_dcl value_body value_elements value_header
%type <node> value_inheritance_spec value_inheritance_bases value_names value_name
%type <node> value_element state_member init_param_decls init_dcl init_param_decl
%type <node> init_param_attribute const_dcl const_type const_exp or_expr
%type <node> xor_expr and_expr shift_expr add_expr mult_expr unary_expr primary_expr
%type <node> positive_int_const literal type_dcl simple_type_spec type_spec
%type <node> template_type_spec base_type_spec constr_type_spec declarators
%type <node> declarator complex_declarator simple_declarator floating_pt_type
%type <node> integer_type signed_short_int signed_int signed_longlong_int
%type <node> signed_long_int unsigned_short_int unsigned_int unsigned_longlong_int
%type <node> unsigned_long_int wide_char_type char_type octet_type boolean_type
%type <node> struct_type object_type any_type member_list member switch_type_spec
%type <node> union_type switch_body case element_spec case_label enumerators
%type <node> enum_type enumerator sequence_type string_type array_declarator
%type <node> wide_string_type fixed_array_sizes fixed_array_size simple_declarators
%type <node> attr_dcl except_dcl members member op_attribute op_dcl parameter_dcls
%type <node> op_type_spec param_dcls param_dcl raises_expr param_attribute
%type <node> string_literals context_expr param_type_spec fixed_pt_const_type
%type <node> fixed_pt_type value_base_type import imported_scope type_id_dcl
%type <node> type_prefix_dcl
%type <_str>   T_string_literal
%type <_wstr>  T_wstring_literal

%start specification

%%

specification	: /*empty*/
		  { theParser.setRootNode(nil); }
		| definitions
	          {theParser.setRootNode( $1 ); }
		;
definitions	: definition
	          {$$ := TNode.Create ( nt_definitions, $1 );}
		| definition definitions
	          {$$ := TNode.Create ( nt_definitions, $1 );
		   $$.setBranch( 2, $2 );}
		;
definition	: type_dcl T_SEMICOLON
	          {$$ := $1;}
		| const_dcl T_SEMICOLON
	          {$$ := $1;}
		| except_dcl T_SEMICOLON
	          {$$ := $1;}
		| interface T_SEMICOLON
	          {$$ := $1;}
		| module T_SEMICOLON
	          {$$ := $1;}
		| value T_SEMICOLON
		  {$$ := $1;}
		| type_id_dcl T_SEMICOLON
	  	  {$$ := $1;}
		| type_prefix_dcl T_SEMICOLON
	  	  {$$ := $1;}
		| import T_SEMICOLON
		  {$$ := $1;}
	        | T_PRAGMA      /*New*/
	          {$$ := TNode.Create ( nt_pragma );
		    $$.setIdent( $1 );}
	        | T_INCLUDE      /*New*/
	          {$$ := TNode.Create ( nt_include );
		    $$.setIdent( $1 );}
		;
module		: T_MODULE T_IDENTIFIER T_LEFT_CURLY_BRACKET
                   definitions T_RIGHT_CURLY_BRACKET
	          {$$ := TNode.Create ( nt_module, $4 );
		    $$.setIdent( $2 );}
		;
interface	: interface_dcl
	          {$$ := $1;}
		| forward_dcl
	          {$$ := $1;}
		;
interface_dcl	: interface_header T_LEFT_CURLY_BRACKET interface_body
                                         T_RIGHT_CURLY_BRACKET
	          {$$ := TNode.Create ( nt_interface_dcl, $1, $3 );}
		;
forward_dcl	: T_INTERFACE T_IDENTIFIER
	          { $$ := TNode.Create ( nt_forward_dcl );
		    $$.setIdent( $2 );}
		| T_ABSTRACT T_INTERFACE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_abstract_forward_dcl );
		    $$.setIdent( $3 );}
		| T_LOCAL T_INTERFACE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_local_forward_dcl );
		    $$.setIdent( $3 );}
		;
interface_header : T_INTERFACE T_IDENTIFIER
	          {$$ := TNode.Create ( nt_interface_header );
		    $$.setIdent( $2 );}
		| T_INTERFACE T_IDENTIFIER interface_inheritance_spec
	          {$$ := TNode.Create ( nt_interface_header );
		    $$.setIdent( $2 );
		    $$.setBranch( 1, $3 );}
		| T_ABSTRACT T_INTERFACE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_abstract_interface_header );
		    $$.setIdent( $3 );}
		| T_ABSTRACT T_INTERFACE T_IDENTIFIER interface_inheritance_spec
	          {$$ := TNode.Create ( nt_abstract_interface_header );
		    $$.setIdent( $3 );
		    $$.setBranch( 1, $4 );}
		| T_LOCAL T_INTERFACE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_local_interface_header );
		    $$.setIdent( $3 );}
		| T_LOCAL T_INTERFACE T_IDENTIFIER interface_inheritance_spec
	          {$$ := TNode.Create ( nt_local_interface_header );
		    $$.setIdent( $3 );
		    $$.setBranch( 1, $4 );}
		;
interface_body	: /*empty*/
	          { $$ := nil; }
		| exports
	          {$$ := $1;}
		;
exports		: export
	          {$$ := TNode.Create ( nt_exports, $1 );}
		| export exports
	          {$$ := TNode.Create ( nt_exports, $1 );
		    $$.setBranch( 2, $2 );}
		;
export		: type_dcl T_SEMICOLON
	          { $$ := $1;}
		| const_dcl T_SEMICOLON
	          {$$ := $1;}
		| except_dcl T_SEMICOLON
	          {$$ := $1;}
		| attr_dcl T_SEMICOLON
	          {$$ := $1;}
		| op_dcl T_SEMICOLON
	          {$$ := $1;}
		| type_id_dcl T_SEMICOLON
	  	  {$$ := $1;}
		| type_prefix_dcl T_SEMICOLON
	  	  {$$ := $1;}
	        | T_PRAGMA      /*New*/
	          {$$ := TNode.Create ( nt_pragma );
		    $$.setIdent( $1 );}
	        | T_INCLUDE      /*New*/
	          {$$ := TNode.Create ( nt_include );
		    $$.setIdent( $1 );}
		;
interface_inheritance_spec : T_COLON interface_names
	          {$$ := $2;}
		;
interface_names	: scoped_names	;

scoped_names	: scoped_name
	          {$$ := TNode.Create ( nt_scoped_names, $1 );}
		| scoped_name T_COMMA scoped_names
	          {$$ := TNode.Create ( nt_scoped_names, $1 );
		    $$.setBranch( 2, $3 );}
		;
scoped_name	: T_IDENTIFIER
	          {$$ := TNode.Create ( nt_scoped_name );
		    $$.setIdent( $1 );}
	        | T_SCOPE T_IDENTIFIER
	          {$$ := TNode.Create ( nt_scoped_name );
		   $$.setIdent('::'+$2);}
		| scoped_name T_SCOPE T_IDENTIFIER
	          {$$ := $1;
		   $$.setIdent($$.getIdent()+'::'+$3);}
		;
value		: value_dcl
		| value_abs_dcl
		| value_box_dcl
		| value_forward_dcl
		;
value_forward_dcl : T_VALUETYPE T_IDENTIFIER
		  { $$ := TNode.Create ( nt_value_forward_dcl );
		    $$.setIdent( $2 );}
		| T_ABSTRACT T_VALUETYPE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_abstract_value_forward_dcl );
		    $$.setIdent( $3 );}
		;
value_box_dcl	: T_VALUETYPE T_IDENTIFIER type_spec
		  {$$ := TNode.Create ( nt_value_box_dcl, $3 );
		    $$.setIdent( $2 ); }
		;
value_abs_dcl	: T_ABSTRACT T_VALUETYPE T_IDENTIFIER
		T_LEFT_CURLY_BRACKET value_body T_RIGHT_CURLY_BRACKET
		  {$$ := TNode.Create ( nt_abstract_value_dcl );
		    $$.setBranch( 1, TNode.Create( nt_value_header ) );
		    $$.getBranch( 1 ).setIdent( $3 );
		    $$.setBranch( 2, $5 );}
		| T_ABSTRACT T_VALUETYPE T_IDENTIFIER value_inheritance_spec
			T_LEFT_CURLY_BRACKET value_body T_RIGHT_CURLY_BRACKET
		  {$$ := TNode.Create ( nt_abstract_value_dcl );
		    $$.setBranch( 1, TNode.Create( nt_value_header ) );
		    $$.getBranch( 1 ).setIdent( $3 );
		    $$.getBranch( 1 ).setBranch( 1, $4 );
		    $$.setBranch( 2, $6 );}
		;
value_body	: /*empty*/
		  { $$ := nil;}
		| exports
		  {$$ := $1;}
		;
value_dcl	: value_header T_LEFT_CURLY_BRACKET value_elements
		T_RIGHT_CURLY_BRACKET
		  {$$ := TNode.Create ( nt_value_dcl, $1, $3 );}
		| value_header T_LEFT_CURLY_BRACKET T_RIGHT_CURLY_BRACKET
		  {$$ := TNode.Create ( nt_value_dcl, $1 );}
		;
value_elements	: value_element
		  { $$ := TNode.Create ( nt_value_elements, $1 );}
		| value_element value_elements
		  {$$ := TNode.Create ( nt_value_elements, $1 );
		    $$.setBranch( 2, $2 );}
		;
value_header	: T_VALUETYPE T_IDENTIFIER value_inheritance_spec
		  {$$ := TNode.Create ( nt_value_header, $3 );
		    $$.setIdent( $2 );}
		| T_CUSTOM T_VALUETYPE T_IDENTIFIER value_inheritance_spec
		  {$$ := TNode.Create ( nt_custom_value_header, $4 );
		    $$.setIdent( $3 );}
		| T_VALUETYPE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_value_header );
		    $$.setIdent( $2 );}
		| T_CUSTOM T_VALUETYPE T_IDENTIFIER
		  {$$ := TNode.Create ( nt_custom_value_header );
		    $$.setIdent( $3 );}
		;
value_inheritance_spec	: T_COLON value_inheritance_bases
		  {$$ := $2;}
		| T_COLON value_inheritance_bases T_SUPPORTS interface_names
		  {$$ := $2;
		    $$.setBranch( 4, $4 );}
		| T_SUPPORTS interface_names
		  {$$ := TNode.Create ( nt_value_inheritance_spec );
		    $$.setBranch( 4, $2 );}
		;
value_inheritance_bases	: value_name
		  {$$ := TNode.Create ( nt_value_inheritance_spec );
		    $$.setBranch( 1, $1 );}
		| value_name T_COMMA value_names
		  {$$ := TNode.Create ( nt_value_inheritance_spec );
		    $$.setBranch( 1, $1 );
		    $$.setBranch( 3, $3 );}
		| T_TRUNCATABLE value_name
		  {$$ := TNode.Create ( nt_value_inheritance_spec );
		    $$.setBranch( 2, $2 );}
		| T_TRUNCATABLE value_name T_COMMA value_names
		  {$$ := TNode.Create ( nt_value_inheritance_spec );
		    $$.setBranch( 2, $2 );
		    $$.setBranch( 3, $4 );}
		;
value_names	: scoped_names	;

value_name	: scoped_name	;

value_element	: export
		| state_member
		| init_dcl
		;
state_member	: T_PUBLIC type_spec declarators T_SEMICOLON
		  {$$ := TNode.Create ( nt_public_state_member, $2, $3 );}
		| T_PRIVATE type_spec declarators T_SEMICOLON
		  {$$ := TNode.Create ( nt_private_state_member, $2, $3 );}
		;
init_dcl	: T_FACTORY T_IDENTIFIER
		T_LEFT_PARANTHESIS init_param_decls T_RIGHT_PARANTHESIS
		T_SEMICOLON
		  {$$ := TNode.Create ( nt_init_dcl, $4 );
		    $$.setIdent( $2 );}
		;
init_param_decls : init_param_decl
		  {$$ := TNode.Create ( nt_init_param_decls, $1 );}
		| init_param_decl T_COMMA init_param_decls
		  {$$ := TNode.Create ( nt_init_param_decls, $1 );
		    $$.setBranch( 2, $3 );}
		;
init_param_decl	: init_param_attribute param_type_spec simple_declarator
		  {$$ := TNode.Create ( nt_init_param_decl, $2, $3 );}
		;
init_param_attribute : T_IN
		  {$$ := nil;}
		;
const_dcl	: T_CONST const_type T_IDENTIFIER T_EQUAL const_exp
	          {$$ := TNode.Create ( nt_const_dcl, $2, $5 );
		    $$.setIdent( $3 );}
		;
const_type	: integer_type
	          {$$ := $1;}
		| char_type
	          {$$ := $1;}
		| wide_char_type
	          {$$ := $1;}
		| boolean_type
	          {$$ := $1;}
		| floating_pt_type
	          {$$ := $1;}
		| string_type
	         {$$ := $1;}
		| wide_string_type
	          {$$ := $1;}
		| fixed_pt_const_type
	          {$$ := $1;}
		| scoped_name
	          {$$ := $1;}
		| octet_type
		  {$$ := $1;}
		;
const_exp	: or_expr
	          {$$ := $1;}
		;
or_expr		: xor_expr
	          {$$ := $1;}
		| or_expr T_VERTICAL_LINE xor_expr
	          {$$ := TNode.Create ( nt_vertical_line, $1, $3 );}
		;
xor_expr	: and_expr
	          {$$ := $1;}
		| xor_expr T_CIRCUMFLEX and_expr
	          {$$ := TNode.Create ( nt_circumflex, $1, $3 );}
		;
and_expr	: shift_expr
	          {$$ := $1;}
		| and_expr T_AMPERSAND shift_expr
	          {$$ := TNode.Create ( nt_ampersand, $1, $3 );}
		;
shift_expr	: add_expr
	          {$$ := $1;}
		| shift_expr T_SHIFTRIGHT add_expr
	          { $$ := TNode.Create ( nt_shiftright, $1, $3 );}
		| shift_expr T_SHIFTLEFT add_expr
	          {$$ := TNode.Create ( nt_shiftleft, $1, $3 );}
		;
add_expr	: mult_expr
	          {$$ := $1;}
		| add_expr T_PLUS_SIGN mult_expr
	          {$$ := TNode.Create ( nt_plus_sign, $1, $3 );}
		| add_expr T_MINUS_SIGN mult_expr
	          {$$ := TNode.Create ( nt_minus_sign, $1, $3 );}
		;
mult_expr 	: unary_expr
	          { $$ := $1;}
		| mult_expr T_ASTERISK unary_expr
	          {$$ := TNode.Create ( nt_asterik, $1, $3 );}
		| mult_expr T_SOLIDUS unary_expr
	          { $$ := TNode.Create ( nt_solidus, $1, $3 );}
		| mult_expr T_PERCENT_SIGN unary_expr
	          { $$ := TNode.Create ( nt_percent_sign, $1, $3 );}
		;
unary_expr	: T_MINUS_SIGN primary_expr
	          { $$ := TNode.Create ( nt_minus_sign, $2 );}
		| T_PLUS_SIGN primary_expr
	          { $$ := TNode.Create ( nt_plus_sign, $2 );}
		| T_TILDE primary_expr
	          {$$ := TNode.Create ( nt_tilde, $2 );}
		| primary_expr
		   {$$ := $1;}
		;
primary_expr	: scoped_name
	          { $$ := $1;  }
		| literal
        	  {$$ := $1;}
		| T_LEFT_PARANTHESIS const_exp T_RIGHT_PARANTHESIS
	          {$$ := $2;}
		;
literal   	: T_FIXED_PT_LITERAL
	          { $$ := TNode.Create ( nt_fixed_pt_literal );
		    $$.setFixedPtLiteral( $1 );}
		| T_FLOATING_PT_LITERAL
	          { $$ := TNode.Create ( nt_floating_pt_literal );
		    $$.setFloatingPtLiteral( $1 );}
		| T_INTEGER_LITERAL
	          { $$ := TNode.Create ( nt_integer_literal );
		    $$.setIntegerLiteral( $1 );}
		| T_string_literal
	          { $$ := TNode.Create ( nt_string_literal );
		    $$.setStringLiteral( $1 );}
		| T_wstring_literal
	          { $$ := TNode.Create ( nt_wstring_literal );
		    $$.setWStringLiteral( $1 );}
		| T_CHARACTER_LITERAL
	          { $$ := TNode.Create ( nt_character_literal );
		    $$.setCharacterLiteral( $1 ); }
		| T_WCHARACTER_LITERAL
	          { $$ := TNode.Create ( nt_wcharacter_literal );
		    $$.setWCharacterLiteral( $1 ); }
		|  T_TRUE  /*boolean_literal*/
	          {$$ := TNode.Create ( nt_boolean_literal );
		    $$.setBooleanLiteral( true );}
		| T_FALSE /*boolean_literal*/
	          {$$ := TNode.Create ( nt_boolean_literal );
		    $$.setBooleanLiteral( false );}
		;
positive_int_const : const_exp
	          {$$ := $1;}
		;
type_dcl	: T_TYPEDEF type_spec declarators
	          {$$ := TNode.Create ( nt_typedef, $2, $3 );}
		| struct_type
	          {$$ := $1;}
		| union_type
	          {$$ := $1;}
		| enum_type
	          {$$ := $1;}
		| T_NATIVE simple_declarator
		  {$$ := TNode.Create ( nt_native, $2 );}
		;
type_spec	: simple_type_spec
          	{$$ := $1;}
		| constr_type_spec
	          {$$ := $1;}
		;
simple_type_spec : base_type_spec
	          {$$ := $1;}
		| template_type_spec
	          {$$ := $1;}
		| scoped_name
	          {$$ := $1;}
		;
base_type_spec	: floating_pt_type
	          {$$ := $1;}
		| integer_type
	          { $$ := $1;}
		| char_type
	          { $$ := $1;}
		| wide_char_type
	          {$$ := $1;}
		| boolean_type
	          {$$ := $1;}
		| octet_type
	          { $$ := $1;}
		| any_type
	          {$$ := $1;}
		| object_type
	          {$$ := $1;}
		| value_base_type
		  {$$ := $1;}
		;
template_type_spec : sequence_type
	          { $$ := $1;}
		| string_type
	          {$$ := $1;}
		| wide_string_type
	          { $$ := $1;}
		| fixed_pt_type
	          {$$ := $1;}
		;
constr_type_spec : struct_type
	          { $$ := $1;}
		| union_type
	          {$$ := $1;}
		| enum_type
	          {$$ := $1;}
		;

declarators	: declarator
	          {$$ := TNode.Create ( nt_declarators, $1 );}
		| declarator T_COMMA declarators
	          {$$ := TNode.Create ( nt_declarators, $1 );
		    $$.setBranch( 2, $3 );}
		;
declarator	: simple_declarator
	          {$$ := $1;}
		| complex_declarator
	          {$$ := $1;}
		;
simple_declarator : T_IDENTIFIER
	          {$$ := TNode.Create ( nt_simple_declarator );
		    $$.setIdent( $1 );}
		;
complex_declarator : array_declarator
	          {$$ := $1;}
		;
floating_pt_type : T_FLOAT
	          { $$ := TNode.Create ( nt_float );}
		| T_DOUBLE
	          { $$ := TNode.Create ( nt_double );}
		| T_LONG T_DOUBLE
	          {$$ := TNode.Create ( nt_long_double );}
		;
integer_type	: signed_int
	          { $$ := $1;}
		| unsigned_int
	          {$$ := $1;}
		;
signed_int	: signed_long_int
	          {$$ := $1;}
		| signed_short_int
	          {$$ := $1;}
		| signed_longlong_int
	          {$$ := $1;}
		;
signed_short_int : T_SHORT
	          {$$ := TNode.Create ( nt_short );}
		;
signed_long_int	: T_LONG
	          {$$ := TNode.Create ( nt_long );}
		;
signed_longlong_int : T_LONG T_LONG
	          {$$ := TNode.Create ( nt_longlong );}
		;
unsigned_int 	: unsigned_long_int
	          {$$ := $1;}
		| unsigned_short_int
	          {$$ := $1;}
		| unsigned_longlong_int
	          {$$ := $1;}
		;
unsigned_short_int : T_UNSIGNED T_SHORT
	          {$$ := TNode.Create ( nt_unsigned_short );}
		;
unsigned_long_int : T_UNSIGNED T_LONG
	          { $$ := TNode.Create ( nt_unsigned_long );}
		;
unsigned_longlong_int	: T_UNSIGNED T_LONG T_LONG
	          {$$ := TNode.Create ( nt_unsigned_longlong );}
		;
char_type 	: T_CHAR
	          {$$ := TNode.Create ( nt_char );}
		;
wide_char_type	: T_WCHAR
	          { $$ := TNode.Create ( nt_wide_char );}
		;
boolean_type	: T_BOOLEAN
	          {$$ := TNode.Create ( nt_boolean );}
		;
octet_type	: T_OCTET
	          {$$ := TNode.Create ( nt_octet );}
		;
any_type	: T_ANY
	          {$$ := TNode.Create ( nt_any );}
		;
object_type	: T_OBJECT
                   {$$ := TNode.Create ( nt_object );}
		;
struct_type	: T_STRUCT T_IDENTIFIER T_LEFT_CURLY_BRACKET member_list
                                               T_RIGHT_CURLY_BRACKET
	          {$$ := TNode.Create ( nt_struct_type, $4 );
		    $$.setIdent( $2 ); }
		;
member_list	: member
	          { $$ := TNode.Create ( nt_member_list, $1 ); }
		| member member_list
	          { $$ := TNode.Create ( nt_member_list, $1 );
		    $$.setBranch( 2, $2 ); }
		;
member		: type_spec declarators T_SEMICOLON
	          {$$ := TNode.Create ( nt_member, $1, $2 );}
	        | T_PRAGMA      /*New*/
          	  {$$ := TNode.Create ( nt_pragma );
		    $$.setIdent( $1 );}
	        | T_INCLUDE      /*New*/
	          {$$ := TNode.Create ( nt_include );
		    $$.setIdent( $1 );}
		;
union_type	: T_UNION T_IDENTIFIER T_SWITCH T_LEFT_PARANTHESIS
          		switch_type_spec T_RIGHT_PARANTHESIS T_LEFT_CURLY_BRACKET
		        switch_body T_RIGHT_CURLY_BRACKET
	          {$$ := TNode.Create ( nt_union_type, $5, $8 );
		    $$.setIdent( $2 );}
		;
switch_type_spec : integer_type
	          {$$ := $1;}
		| char_type
	          {$$ := $1;}
		| boolean_type
	          {$$ := $1;}
		| enum_type
	          {$$ := $1;}
		| scoped_name
	          {$$ := $1;}
		;
switch_body	: case
	          {$$ := TNode.Create ( nt_switch_body, $1 );}
		| case switch_body
	          {$$ := TNode.Create ( nt_switch_body, $1, $2 );}
		;
case		: case_label case
	          { $$ := TNode.Create ( nt_case, $1, $2 );}
		| case_label element_spec T_SEMICOLON
	          { $$ := TNode.Create ( nt_case, $1 );
		    $$.setBranch( 3, $2 );}
		| case_label T_PRAGMA element_spec T_SEMICOLON   /* New */
        	  {$$ := TNode.Create ( nt_case, $1 );
		    $$.setBranch( 3, $3 );
		    $$.setBranch( 4, TNode.Create  (nt_pragma));
		    $$.getBranch( 4).setIdent($2);}
		;
case_label	: T_CASE const_exp T_COLON
	          {$$ := $2;}
		| T_DEFAULT T_COLON
	          { $$ := TNode.Create ( nt_default );}
		;
element_spec	: type_spec declarator
	          { $$ := TNode.Create ( nt_element_spec, $1, $2 );}
		;
enum_type	: T_ENUM T_IDENTIFIER T_LEFT_CURLY_BRACKET enumerators
                                         T_RIGHT_CURLY_BRACKET
	          {$$ := TNode.Create ( nt_enum_type, $4 );
		    $$.setIdent( $2 );}
		;
enumerators	: enumerator
	          {$$ := $1;}
		| enumerator T_COMMA enumerators
	          { $$ := TNode.Create ( nt_enumerators, $1, $3 );}
		;
enumerator	: T_IDENTIFIER
	          {$$ := TNode.Create ( nt_enumerator );
	           $$.setIdent( $1 );}
		;
sequence_type	: T_SEQUENCE T_LESS_THAN_SIGN simple_type_spec T_COMMA
                        positive_int_const T_GREATER_THAN_SIGN
	          {$$ := TNode.Create ( nt_sequence, $3, $5 );}
		| T_SEQUENCE T_LESS_THAN_SIGN simple_type_spec T_GREATER_THAN_SIGN
	          {$$ := TNode.Create ( nt_sequence, $3 );}
		;
string_type	: T_STRING T_LESS_THAN_SIGN positive_int_const T_GREATER_THAN_SIGN
          	  {$$ := TNode.Create ( nt_string, $3 );}
		| T_STRING
	          {$$ := TNode.Create ( nt_string );}
		;
wide_string_type : T_WSTRING T_LESS_THAN_SIGN positive_int_const T_GREATER_THAN_SIGN
	          {$$ := TNode.Create ( nt_wide_string, $3 );}
		| T_WSTRING
	          {$$ := TNode.Create ( nt_wide_string );}
		;
array_declarator : T_IDENTIFIER fixed_array_sizes
	          {$$ := TNode.Create ( nt_array_declarator, $2 );
		    $$.setIdent( $1 );}
		;

fixed_array_sizes : fixed_array_size
	          {$$ := TNode.Create ( nt_fixed_array_sizes, $1 );}
		| fixed_array_size fixed_array_sizes
	          {$$ := TNode.Create ( nt_fixed_array_sizes, $1 );
		   $$.setBranch( 2, $2 );}
		;
fixed_array_size : T_LEFT_SQUARE_BRACKET positive_int_const T_RIGHT_SQUARE_BRACKET
	          {$$ := $2;}
		;
attr_dcl	: T_ATTRIBUTE param_type_spec simple_declarators
	          {$$ := TNode.Create ( nt_attribute, $2, $3 );}
		| T_READONLY T_ATTRIBUTE param_type_spec simple_declarators
	          {$$ := TNode.Create ( nt_readonly_attribute, $3, $4 );}
		;
simple_declarators : simple_declarator
	          {$$ := TNode.Create ( nt_simple_declarators, $1 );}
		| simple_declarator T_COMMA simple_declarators
	          {$$ := TNode.Create ( nt_simple_declarators, $1 );
		   $$.setBranch( 2, $3 );}
		;
except_dcl	: T_EXCEPTION T_IDENTIFIER T_LEFT_CURLY_BRACKET members
                                          T_RIGHT_CURLY_BRACKET
	          {$$ := TNode.Create ( nt_except_dcl, $4 );
		    $$.setIdent( $2 );}
		;

members 	: /*empty*/
	          {$$ := nil;}
		| member members
	          {$$ := TNode.Create ( nt_member_list, $1 );
		   $$.setBranch( 2, $2 );}
		;
op_dcl		: op_attribute op_type_spec T_IDENTIFIER parameter_dcls
                                       raises_expr context_expr
	          {$$ := TNode.Create ( nt_op_dcl, $1, $2, $4, $5, $6 );
		   $$.setIdent( $3 );}
		;
op_attribute	: /*empty*/
	          {$$ := nil;}
		| T_ONEWAY
	          {$$ := TNode.Create ( nt_oneway );}
		;
op_type_spec : param_type_spec
	          {$$ := TNode.Create ( nt_op_type_spec, $1 );}
		| T_VOID
	          {$$ := TNode.Create ( nt_void );}
		;
parameter_dcls	: T_LEFT_PARANTHESIS param_dcls T_RIGHT_PARANTHESIS
	          {$$ := $2;}
		| T_LEFT_PARANTHESIS T_RIGHT_PARANTHESIS
	          {$$ := nil;}
		;

param_dcls	: param_dcl
	          {$$ := TNode.Create ( nt_param_dcls, $1 ); }
		| param_dcl T_COMMA param_dcls
	          {$$ := TNode.Create ( nt_param_dcls, $1 );
		   $$.setBranch( 2, $3 );}
		;

param_dcl	: param_attribute param_type_spec simple_declarator
	          {$$ := TNode.Create ( nt_param_dcl, $1, $2, $3 );}
		;

param_attribute	: T_IN
	          { $$ := TNode.Create ( nt_in );}
		| T_OUT
	          {$$ := TNode.Create ( nt_out );}
		| T_INOUT
	          {$$ := TNode.Create ( nt_inout );}
		;
raises_expr	: /*empty*/
	          {$$ := nil;}
		| T_RAISES T_LEFT_PARANTHESIS scoped_names T_RIGHT_PARANTHESIS
	          {$$ := TNode.Create ( nt_raises_expr, $3 );}
		;
context_expr	: /*empty*/
	          {$$ := nil;}
		| T_CONTEXT T_LEFT_PARANTHESIS string_literals T_RIGHT_PARANTHESIS
	          {$$ := $3;}
		;
string_literals	: T_string_literal
	          {$$ := TNode.Create ( nt_string_literals );
		   $$.setIdent( $1 );}
		| T_string_literal T_COMMA string_literals
	          {$$ := TNode.Create ( nt_string_literals, $3 );
		   $$.setIdent( $1 );}
		;
T_string_literal : T_STRING_LITERAL
		  {$$ := $1;}
		| T_STRING_LITERAL T_string_literal
		  {$$ := $1 + ' ' + $2;}
		;
T_wstring_literal : T_WSTRING_LITERAL
		  {$$ := $1;}
		| T_WSTRING_LITERAL T_wstring_literal
		  {$$ := $1 + ' ' + $2;}
		;
param_type_spec	: base_type_spec
	          { $$ := $1;}
		| string_type
	          {$$ := $1;}
		| wide_string_type
	          {$$ := $1;}
		| scoped_name
	          {$$ := $1;}
		;
fixed_pt_type 	: T_FIXED T_LESS_THAN_SIGN positive_int_const T_COMMA
        	      T_INTEGER_LITERAL T_GREATER_THAN_SIGN
	          {$$ := TNode.Create ( nt_fixed_pt_type, $3 );
		    $$.setIntegerLiteral( $5 );}
		;
fixed_pt_const_type : T_FIXED
	          { $$ := TNode.Create ( nt_fixed_pt_type );}
		;
value_base_type	: T_VALUEBASE
		  {$$ := TNode.Create ( nt_valuebase );}
		;
import		: T_IMPORT imported_scope
		  {$$ := TNode.Create ( nt_import, $2 );}
		;
imported_scope	: scoped_name
		| T_STRING_LITERAL
	          {$$ := TNode.Create ( nt_string_literals );
		   $$.setIdent( $1 );}
		;
type_prefix_dcl	: T_TYPEPREFIX scoped_name T_string_literal
	  	  {$$ := TNode.Create (nt_typeprefix, $2);
	    	   $$.setIdent ($3);}
		;
type_id_dcl	: T_TYPEID scoped_name T_string_literal
	  	  {$$ := TNode.Create (nt_typeid, $2);
	    	   $$.setIdent ($3);}
		;

%%

{$I idllex.pas}

end.
