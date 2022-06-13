/**************************************************************************
 *
 *  Copyright 1998-2010, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */
 
/* 
 * $Id$
 */

#ifndef __RHBOPT_H__
#error __RHBOPT_H__ not defined
#endif


/* SOM Compiler

  compiles IDL files (and only IDL files)

  generates...

  .h
  .ih

  C++ will be supported by testing for __cplusplus on compilation...

  .xih should just include .ih
  .xh should just include .h

  */

/* currently, 'union' is missing, 

module U
{
	union KeyUnion switch (short)
	{
		case 1: octet data_1;
		case 2: short data_2;
		case 4: long data_4;
		default: string data_z;
	};
};

  should produce

typedef struct U_KeyUnion 
{
	short _d;
	union 
	{
		octet  data_1;
		short  data_2;
		long  data_4;
		string  data_z;
	} _u;
} U_KeyUnion;

  but, oops!

	g++ may need -fno-rtti for binary compatibility?

*/


#include <ctype.h>

#ifdef _WIN32
	#ifndef _PLATFORM_WIN32_
		#define _PLATFORM_WIN32_
	#endif
#else
	#if (!defined(_PLATFORM_MACINTOSH_)) && (!defined(__OS2__)) && (!defined(_PLATFORM_UNIX_))
		/* previously used to look for 'unix' first */
		#define _PLATFORM_UNIX_
	#endif
#endif

#ifndef HAVE_SNPRINTF
extern int snprintf(char *,size_t,const char *,...);
#endif
#ifndef HAVE_VSNPRINTF
extern int vsnprintf(char *,size_t,const char *,va_list);
#endif

extern void bomb(const char *);

extern char double_quote;
extern char single_quote;

typedef unsigned char boolean;
typedef unsigned char octet;
typedef char * string;

class RHBPreprocessor;
class RHBinclude_file;
class RHBinclude_list;
class RHBFile;

class RHBStream
{
public:
	RHBStream();
	virtual ~RHBStream();
	virtual char get_char()=0;
	virtual int is_eof()=0;
	virtual void un_get(char c)=0;	
	virtual RHBinclude_file *File()=0;
	virtual RHBFile *is_file()=0;
	virtual int GetLine();
	virtual void print_identity(FILE *)=0;
};

class RHBFile : public RHBStream
{
	int argc;
	char **argv;
	FILE *fp;
	char un_got;
	char filename[1024];
	int line_number;
public:
	inline int failed()
	{
		if (fp) return 0;
		return 1;
	}
	int GetLine();
	RHBinclude_file *incl;
	RHBFile(int argc,char **argv,FILE *);
	RHBFile *parent;
	RHBFile *is_file();
	char get_char();
	void un_get(char c);
	int is_eof();
	void print_identity(FILE *);
	RHBinclude_file *File();
	void update_file_line(const char *,int);

	inline const char *_get_filename()
	{
		if (filename[0])
		{
			return filename;
		}

		return NULL;
	}
};

class RHBelement;
class RHBrepository;

typedef boolean (*RHBshould_emit)(RHBelement *);

boolean RHBshould_emit_on(RHBelement *);
boolean RHBshould_emit_off(RHBelement *);
boolean RHBshould_emit_default(RHBelement *);

class RHBPreprocessor
{
	char text[1024];
	long lText;
	int in_text;
	RHBStream *streamStack[1024];
	int stream_level;
	int get_next_token(char *buf);
	void Close();
	void past_white_space();
	RHBshould_emit emit_root;
	long error_count;
public:
	RHBinclude_list *includes;
	RHBinclude_file *CurrentFile();
	int CurrentLine();
	void err(const char *,const char *);
	void push_stream(RHBStream *str);
	void add_path(const char *p);
	RHBPreprocessor();
	int get_token(char *buf,size_t buflen);
	char get_non_whitespace();

	RHBStream *currStream();

	inline long Errors()
	{
		return error_count;
	}

	inline void SetEmitRoot(RHBshould_emit a)
	{
		emit_root=a;
	}

	inline RHBshould_emit GetEmitRoot()
	{
		return emit_root;
	}

	RHBelement *scope;

	void pragma_clear();
};

class RHBmodule;
class RHBtype;
class RHBparameter;
class RHBoperation;
class RHBinherited_operation;
class RHBimplementation;
class RHBinterface;
class RHBexception;
class RHBconstant;
class RHBenum;
class RHBcontext_string;
class RHBelement;
class RHBsequence_type;
class RHBstring_type;
class RHBvalue;
class RHBmodifier;
class RHBtype_def;
class RHBstruct;
class RHBstruct_element;
class RHBarray;
class RHBpointer_type;
class RHBqualified_type;
class RHBsequence_type;
class RHBunion;
class RHBunion_element;
class RHBbase_type;
class RHBany_type;
class RHBTypeCode_type;
class RHBattribute;

class RHBelement_sequence
{
	unsigned long _length;
	unsigned long _maximum;
	RHBelement **_buffer;
public:
	RHBelement_sequence();
	~RHBelement_sequence();

	RHBelement *get(unsigned long i);

	inline unsigned long length()
	{
		return _length;
	}

	void add(RHBelement *e);
	void remove(RHBelement *e);
	RHBelement *contains(RHBelement *e);
	int index(RHBelement *e);
};

class RHBelement_new_scope
{
	RHBPreprocessor *pp;
	RHBelement *old;
public:
	RHBelement_new_scope(RHBPreprocessor *p,RHBelement *e);
	~RHBelement_new_scope();
};

class RHBmodifier_list
{
	int _length,_maximum;
	string *_buffer;
public:
	RHBmodifier_list();
	~RHBmodifier_list();
	void add(const char *p);
	const char * get(int i);
	inline int length()
	{
		return _length;
	}
};

class RHBelement
{
protected:
	const char *class_name;
	RHBelement *_parent;
	RHBelement *_next;
	RHBelement *_children;
/*	RHBelement *_last;*/
	RHBelement *_mem_next;
public:
	void remove_quotes(char *buf);
	RHBinclude_file *defined_in;
	int defined_line;
	RHBshould_emit emit_root;

static void delete_all(RHBelement *ex);
static RHBelement *_mem_first;

	inline RHBelement *next()
	{
		return _next;
	}

	inline RHBelement *children()
	{
		return _children;
	}

	inline RHBelement *parent()
	{
		return _parent;
	}
	inline const char *classname()
	{
		return class_name;
	}

	RHBmodifier_list modifier_data;

	void read_modifier(RHBPreprocessor *pp,char *buf,size_t buflen,boolean passthru_flag);
	void add_modifier_value(const char *buf);

	int has_modifier_value(const char *p);
	const char *get_modifier_string(const char *name);
	void set_pragma_modifier(RHBPreprocessor *pp,const char *name,const char *value);

	void build_id(char *buf,size_t buflen);
	char *id;
	boolean is_called(const char *buf);
	void _set_id(const char *buf);
	char *prefix;
	void _set_prefix(const char *buf);
	void _set_global_id(const char *buf);
	void _set_version(const char *buf);
	char *global_id;
	char *version;
	RHBelement(const char *class_n,RHBPreprocessor *pp,const char *name);
	virtual ~RHBelement();
	const char * _get_id();
	void add(RHBPreprocessor *pp,RHBelement *child);
	void add_nocheck(RHBPreprocessor *pp,RHBelement *child);
	void remove(RHBelement *child);
	virtual void build(RHBPreprocessor *pp)=0;
	virtual RHBmodule *is_module();
	virtual RHBinherited_operation *is_inherited_operation();
	virtual RHBinterface *is_interface();
	virtual RHBparameter *is_parameter();
	virtual RHBoperation *is_operation();
	virtual RHBtype *is_type();
	virtual RHBexception *is_exception();
	virtual RHBconstant *is_constant();
	virtual RHBcontext_string *is_context();
	virtual RHBinclude_file *is_include_file();
	virtual RHBvalue *is_value();
	virtual RHBmodifier *is_modifier();
	virtual RHBrepository *is_repository();
	virtual RHBattribute *is_attribute();
	RHBtype *wrap_with_pointer(RHBPreprocessor *pp,char *buf,size_t buflen,RHBtype *t);
	RHBtype *wrap_as_array(RHBPreprocessor *pp,char *buf,size_t buflen,RHBtype *t);
	RHBelement * find_named_element(RHBPreprocessor *pp,char *buf,size_t nlen);
	RHBelement *has_named_element(const char *);
	RHBinterface * read_interface(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBtype * read_type(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBtype * read_struct(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBtype * read_union(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBtype * read_typedef(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBmodule * read_module(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBconstant * read_constant(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBexception * read_exception(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBenum * read_enum(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBelement * read_pragma(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBelement *read_token(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBvalue *read_number(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBvalue *read_string_value(RHBPreprocessor *pp,char *buf,size_t buflen);
	void dump_ids(const char *p);

	void reset_defined_in(RHBPreprocessor *pp);

	virtual void destroy();

	void gen_global_id();
};

class RHBinclude_file : public RHBelement
{
public:
	RHBinclude_file(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBinclude_file *is_include_file();
	RHBelement *included_during;
};

class RHBinclude_list : public RHBelement
{
public:
	RHBinclude_list(RHBPreprocessor *pp,const char *name);
	RHBinclude_file *included(RHBPreprocessor *pp,const char *fname,RHBinclude_file *curr);
	void build(RHBPreprocessor *pp);
};

class RHBmodule : public RHBelement
{
public:
	RHBmodule(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBmodule *is_module();
};

class RHBrepository : public RHBmodule
{
public:
	RHBrepository(RHBPreprocessor *_pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBrepository *is_repository();
	RHBPreprocessor &_pp;
};

class RHBtype : public RHBelement
{
public:
	RHBtype(RHBPreprocessor *pp,const char *name);
	RHBtype *is_type();
	void build(RHBPreprocessor *pp);
	virtual RHBenum *is_enum();
	virtual RHBstruct *is_struct();
	virtual RHBtype_def *is_typedef();
	virtual RHBstruct_element *is_struct_element();
	virtual RHBarray *is_array();
	virtual RHBpointer_type * is_pointer();
	virtual RHBstring_type *is_string();
	virtual RHBqualified_type *is_qualified();
	virtual RHBsequence_type *is_sequence();
	virtual RHBunion *is_union();
	virtual RHBunion_element *is_union_element();
	virtual RHBbase_type *is_base_type();
	virtual RHBany_type *is_any();
	virtual RHBTypeCode_type *is_TypeCode();
};

class RHBbase_type : public RHBtype
{
public:
	RHBbase_type(RHBPreprocessor *pp,
			const char *name,
			RHBtype *va_type,
			boolean in_by_ref,
			boolean floating);

	boolean _floating;
	RHBtype * _va_type;
	boolean _in_by_ref;
	RHBbase_type *is_base_type();

static void boot_root_types(RHBPreprocessor *pp,RHBrepository *rp);
};

class RHBTypeCode_type: public RHBbase_type
{
public:
	RHBTypeCode_type(RHBPreprocessor *pp,
			const char *name,
			RHBtype *va_type,
			boolean in_by_ref);
	RHBTypeCode_type *is_TypeCode();
};

class RHBany_type: public RHBbase_type
{
public:
	RHBany_type(RHBPreprocessor *pp,
			const char *name,
			RHBtype *va_type,
			boolean in_by_ref);
	RHBany_type *is_any();
};

class RHBqualified_type : public  RHBtype
{
public:
	RHBqualified_type(RHBPreprocessor *pp,const char *name);
	RHBtype *base_type;
	RHBqualified_type *is_qualified();
};

class RHBtype_def : public RHBtype
{
public:
	RHBtype_def(RHBPreprocessor *pp,const char *name);
	RHBtype *alias_for;
	RHBtype_def *is_typedef();
};

class RHBarray : public RHBtype
{
public:
	RHBarray(RHBPreprocessor *pp,const char *name);
	RHBvalue *elements;
	RHBtype *array_of;
	RHBarray *is_array();
};

class RHBstruct : public RHBtype
{
public:
	RHBstruct(RHBPreprocessor *pp,const char *name);
	RHBstruct *is_struct();
};

class RHBunion : public RHBtype
{
public:
	RHBunion(RHBPreprocessor *pp,const char *name);
	RHBtype *switch_type;
	RHBunion *is_union();
	RHBelement_sequence elements;
	long default_index;
};

class RHBstruct_element : public RHBtype
{
public:
	RHBstruct_element(RHBPreprocessor *pp,const char *name);
	RHBtype *element_type;
	RHBstruct_element *is_struct_element();
};

class RHBunion_element : public RHBtype
{
public:
	RHBunion_element(RHBPreprocessor *pp,const char *name);
	RHBtype *element_type;
	RHBvalue *switch_value;
	RHBunion_element *is_union_element();
};

class RHBpointer_type : public RHBtype
{
public:
	RHBpointer_type(RHBPreprocessor *pp,const char *name);
	char type[2];
	RHBtype *pointer_to;
	RHBpointer_type * is_pointer();
};

class RHBsequence_type : public RHBtype
{
public:
	RHBsequence_type(RHBPreprocessor *pp,const char *name);
	RHBvalue * length;
	RHBtype *sequence_of;
	RHBsequence_type *is_sequence();
};

class RHBstring_type : public RHBtype
{
public:
	RHBstring_type(RHBPreprocessor *pp,const char *name);
	RHBvalue * length;
	RHBstring_type *is_string();
};

class RHBinterface : public RHBtype
{
public:
	RHBinterface(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBinterface *is_interface();

	RHBelement_sequence _parents;
	RHBelement_sequence passthru_list;
	RHBelement_sequence _metaclass_of;

	void read_implementation(RHBPreprocessor *pp,char *buf,size_t buflen);
	void build_instance_data(RHBPreprocessor *pp);

	RHBstruct *instanceData;
	RHBstruct *classData;
	RHBstruct *cclassData;

	void list_all_operations(RHBelement_sequence *seq);
	void inherit_operations(RHBPreprocessor *pp,RHBinterface *iface);

	boolean is_subclass_of(RHBinterface *p);
};

class RHBparameter : public RHBtype
{
public:
	RHBparameter(RHBPreprocessor *pp,const char *name);
	char mode[6];
	RHBtype *parameter_type;
	RHBparameter *is_parameter();
};

class RHBattribute : public RHBtype
{
public:
	RHBattribute *is_attribute();
	RHBattribute(RHBPreprocessor *pp,const char *name);
	boolean readonly;
	RHBtype *attribute_type;
	void generate_accessors(RHBPreprocessor *pp,RHBinterface *iface);
};

class RHBvalue : public RHBtype
{
public:
	RHBvalue(RHBPreprocessor *pp,const char *name);
	~RHBvalue();
	boolean was_quoted;
	string value_string;
	RHBvalue *is_value();
	RHBconstant *assigned_value;
	virtual long numeric_value();
	long read_hex_value(const char *p);
	long read_octal_value(const char *p);
};

class RHBconstant : public RHBvalue
{
public:
	RHBconstant(RHBPreprocessor *pp,const char *name);
	void read_c_value(RHBPreprocessor *pp,char *buf,size_t buflen);
	RHBconstant *is_constant();
	RHBtype *constant_type;
	RHBvalue *const_val;
	RHBtype *const_typecode_val;
	virtual long numeric_value();
	boolean is_numeric();
	boolean is_const_TypeCode();
	RHBenum *is_const_enum();
	boolean is_private;
};

class RHBequation : public RHBconstant
{
public:
	RHBequation(RHBPreprocessor *pp,const char *name);
	virtual long numeric_value();
	RHBvalue *val1;
	RHBvalue *val2;
	char op[5];
};

class RHBenum : public RHBtype
{
public:
	RHBenum(RHBPreprocessor *pp,const char *name);
	RHBenum *is_enum();
};

class RHBenum_val : public RHBvalue
{
public:
	RHBenum_val(RHBPreprocessor *pp,const char *name);
};

class RHBexception : public RHBtype
{
public:
	RHBexception(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBexception *is_exception();
};

class RHBmodifier : public RHBtype
{
public:
	RHBmodifier(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBmodifier *is_modifier();
};

class RHBinherited_operation : public RHBtype
{
public:
	RHBinherited_operation(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBinherited_operation *is_inherited_operation();
	RHBoperation *original;
};

class RHBcontext_string: public RHBelement
{
public:
	RHBcontext_string(RHBPreprocessor *pp,const char *name);
	~RHBcontext_string();
	void build(RHBPreprocessor *pp);
	string value;
	RHBcontext_string *is_context();

};

class RHBoperation : public RHBtype
{
public:
	RHBoperation(RHBPreprocessor *pp,const char *name);
	void build(RHBPreprocessor *pp);
	RHBelement_sequence exception_list;
	RHBelement_sequence context_list;
	boolean oneWay;
	RHBtype *return_type;
	RHBoperation *is_operation();
	RHBparameter *get_parameter(int i);
	boolean can_auto_stub();
};

class RHBattribute_accessor : public RHBoperation
{
public:
	RHBattribute_accessor(RHBPreprocessor *pp,const char *name);
	RHBattribute *for_attribute;
};

class RHBoutput
{
public:
	virtual void write(const void *p,size_t len)=0;
	virtual ~RHBoutput();
};

class RHBtextfile : public RHBoutput
{
	char *filename;
	FILE *fp;
public:
	RHBtextfile(const char *name);
	~RHBtextfile();
	void write(const void *p,size_t len);
};

class RHBemitter
{
	RHBrepository *_rr;
public:
	const char *idl_name;
	inline RHBrepository *GetRepository()
	{
		return _rr;
	}
	int nolegacy()
	{
		return 1;
	}
	const char *get_corbastring();
	const char *unquote(char *tgt,const char *src);
	RHBemitter(RHBrepository *rr);
	virtual ~RHBemitter();

	boolean CheckTrueCORBA(RHBtype *type);
	virtual boolean ShouldEmit(RHBelement *t);

	RHBelement_sequence generated_references;
	RHBelement_sequence generated_definitions;
	RHBelement_sequence is_generating_reference;
	RHBelement_sequence is_generating_definition;

	boolean has_generated_reference(RHBtype *type);
	boolean has_generated_definition(RHBtype *type);

	virtual boolean must_generate(RHBtype *type);
	virtual boolean generate(RHBoutput *out,const char *f)=0;
	void get_ir_name(RHBelement *element,char *buf,size_t buflen);
	void get_c_name(RHBelement *element,char *buf,size_t buflen);
	void get_global_id(RHBelement *element,char *buf,size_t buflen);
	const char *short_file_name(const char *long_file_name);
	static RHBtype *unwind_typedef(RHBtype *t);
	RHBtype *is_SOMFOREIGN(RHBtype *t);

	void out_printf(RHBoutput *,const char *,...);
	void dump_nest(RHBoutput *out,int nest);
};

/* this is the base class for header file generators,
	first C then C++ derived from those */

class RHBheader_emitter : public RHBemitter
{
static const char *szSOM_RESOLVE_DATA;
public:
	RHBmodifier_list sequences;
	RHBelement_sequence generated_types;
	RHBelement_sequence generated_apply_stubs;
	RHBmodifier_list generated_statics;
	int is_kernel_class(RHBinterface *);

	char idl_filestem[256];
	char idl_conditional[512];
	char file_guard[512];
	RHBheader_emitter(RHBrepository *rr);
	boolean cplusplus;
	boolean internal;
	boolean testonly;

	void preflight_macros_from_idl_filename(const char *);

	void undoublebar(RHBoutput *out,const char *);

	void type_size_expand(RHBoutput *out,int nest,const char *t,struct type_size *a,size_t n);
	int get_cif(RHBinterface *iface,char *buf,size_t buflen);
	int is_unsigned(RHBtype *type);
	int is_long(RHBtype *type);
	int count_ClassData(RHBinterface *);
	int get_init_operations(RHBinterface *,RHBelement_sequence *);
	int get_va_operations(RHBinterface *,RHBelement_sequence *);

	boolean useAlignmentHelper(void) { return cplusplus; };
	boolean is_derived_from(RHBinterface *iface,RHBinterface *base);

	void generate_type(RHBtype *type,RHBoutput *out,int nesting);
	void generate_prototype(RHBtype *type,RHBoutput *out);
	void generate_passthrus(RHBoutput *out,RHBelement *root,int nest);
	void generate_usage(RHBtype *type,RHBoutput *out);
	boolean generate(RHBoutput *out,const char *f);

	void generate_exception(RHBexception *s,RHBoutput *out,int nesting,const char *alias_name);
	void generate_struct(RHBstruct *s,RHBoutput *out,int nesting,const char *alias_name,boolean user_data,boolean instanceData);
	void generate_union(RHBunion *s,RHBoutput *out,int nesting,boolean user_data);
	void generate_typedef(RHBtype_def *s,RHBoutput *out,int nesting);
	void generate_enum(RHBenum *s,RHBoutput *out,int nesting);
	static boolean is_in_floating(RHBparameter *param);
	void generate_sizeof_va_list_param(RHBoutput *,RHBparameter *,int nesting);
	void generate_headers(RHBoutput *out,int nest);
	void generate_module(RHBoutput *out,RHBmodule *mod,int nest);
	void generate_modules(RHBoutput *out,RHBmodule *mod,int nest);
	void generate_class_protos(RHBoutput *out,RHBmodule *mod,int nest);
	void generate_class_proto(RHBoutput *out,RHBinterface *iface,int nest);
	void generate_types(RHBoutput *out,RHBelement *mod,int nest);
	void generate_classes(RHBoutput *out,RHBmodule *mod,int nest,int apiOnlyFlag);
	void generate_class(RHBoutput *out,RHBinterface *iface,int nest,int apiOnlyFlag);
	void generate_instance_data(RHBoutput *out,RHBinterface *iface,int nest);
	void generate_class_data(RHBoutput *out,RHBinterface *iface,int nest);
	void generate_somTDs(RHBoutput *out,RHBinterface *iface,int nest);
	void generate_somTD(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_name_lookups(RHBoutput *out,RHBinterface *iface,int nest);
	void generate_name_lookup(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	RHBelement *generate_va_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest,int flag,const char *prefix);
	void generate_va_stub(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest,int flag,const char *prefix);
	void generate_parameter_list(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nesting,boolean include_somSelf,boolean do_const_in);
	void generate_name_only_parameter_list(RHBoutput *out,RHBinterface *iface,RHBoperation *op,boolean expand_self);
	void generate_inline_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_inline_somNew_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	RHBinterface *find_highest_common_parent(RHBelement_sequence *seq); 
	void generate_inline_varg_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_macro_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_inherited_macros(RHBoutput *out,RHBelement_sequence *duplicates,RHBinterface *iface,RHBinterface *parent);
	struct dts_override *is_dts_override(RHBoperation *op);
	void dts_parent_macro(RHBoutput *out,RHBinterface *iface,int num,RHBinterface *parent,RHBoperation *op,int nest);
	void write_macro_args(RHBoutput *out,RHBoperation *op,int include_somself);
	void generate_dts_macros(RHBoutput *out,RHBinterface *);
	void generate_redispatch_stub(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_redispatch_stub_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_apply_stub(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_apply_stub_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void generate_struct_element(RHBstruct_element *el,RHBoutput *out,int nest);
	void generate_somId(RHBoutput *out,int nest,RHBelement *el,
				const char *strPrefix,
				const char *idPrefix,boolean ir_spec,boolean recurse);
	RHBtype *generate_array_slice_typedef(RHBoutput *,int,RHBtype *,const char *n);
	boolean do_const_param(RHBparameter *param);
	void call_parent_constructors(RHBoutput *,RHBinterface *,int,const char *args);
	void use_type(RHBoutput *out,RHBtype *typ,const char *name);
	void init_type(RHBoutput *out,RHBtype *typ);
	void get_sequence_name(RHBsequence_type *seq,char *name,size_t len);
	void generate_rdsizetype(RHBoutput *,const char *,int);
	void generate_operation_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest);
	void get_function_prefix(RHBinterface *iface,char *buf,size_t buflen);
	boolean get_meta_class(RHBinterface *iface,char *buf);
	boolean get_actual_meta_class(RHBinterface *iface,char *buf,size_t buflen);
	RHBinterface *get_meta_class_interface(RHBinterface *iface);
	void generate_parent_macro(RHBoutput *out,RHBinterface *iface,RHBinterface *parent,RHBoperation *op,int nest);
	RHBtype * get_most_primitive_name(RHBtype *typ,char *name,size_t namelen);
	void generate_sequence(RHBoutput *out,RHBtype *typ,int nest);
	void generate_constant(RHBoutput *out,RHBconstant *typ,int nest);
	void generate_interface(RHBoutput *out,RHBinterface *iface,int nest);
	int get_parent_index(RHBinterface *iface,RHBinterface *if2);
	void generate_new_class(RHBoutput *out,RHBinterface *op,RHBelement_sequence *overrides,int nest);
	boolean operation_needs_environment(RHBinterface *iface,RHBoperation *op);
	boolean operation_needs_context(RHBinterface *iface,RHBoperation *op);
	boolean parameter_by_reference(RHBparameter *param,RHBtype **array_of);
	boolean operation_has_return(RHBoperation *op);
	void use_va_arg(RHBoutput *out,const char *ap,RHBtype *typ,boolean byRef);
	RHBtype *get_va_type(RHBtype *typ,boolean cast_floating);
	int align_for(long curr,int align);
	void determine_overrides(RHBinterface *iface,RHBinterface *candidates,RHBelement_sequence *seq);
	void determine_inherited(RHBinterface *iface,RHBinterface *candidates,RHBelement_sequence *seq);
	boolean does_override(RHBinterface *iface,const char *name);
	boolean do_som21_macro(RHBinterface *iface,const char *name);
	boolean has_same_ABI(RHBoperation *op1,RHBoperation *op2);
	boolean same_ABI_type(RHBtype *t1,RHBtype *t2);
	boolean some_ABI_kind_of_pointer(RHBtype *t);
	void generate_versions(RHBoutput *out,RHBinterface *iface,const char *ext,const char *mod_name,int nest);
	boolean is_multiple_inherited(RHBinterface *iface);
	boolean will_generate_static(const char *p);
	void ifdef_import_export(RHBoutput *out,RHBinterface *iface,int nest);
	void extern_c_begin(RHBoutput *out,int nest);
	void extern_c_end(RHBoutput *out,int nest);
	void align_begin(RHBoutput *out,boolean user_data,int nest);
	void align_end(RHBoutput *out,boolean user_data,int nest);
	boolean is_operation_procedure(RHBoperation *op);
	boolean is_operation_noself(RHBoperation *op);
	void least_ambiguous_interface(RHBinterface *iface,const char *iface_name,char *buf,size_t buflen);

	void generate_class_guard(RHBoutput *out,int nest,RHBinterface *iface,boolean how);

	void begin_generate_static(RHBoutput *out,const char *name,int nest);
	void end_generate_static(RHBoutput *out,const char *name,int nest);
	void generate_init_zero(RHBoutput *out,const char *name,RHBtype *typ,int nest,unsigned long initial_value);

	void generate_somMethodInfo(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest,int auto_stub);
	void generate_internal_class_data(RHBoutput *out,RHBinterface *iface);
	void generate_internal_cclass_data(RHBoutput *out,RHBinterface *iface);
	void generate_internal_ctrl_macros(RHBoutput *out,RHBinterface *iface);
	void generate_beginassignop(
			RHBoutput *out,
			RHBinterface *iface);
	void generate_begininit(
			RHBoutput *out,
			RHBinterface *iface,
			const char *meth_name);
	void generate_begindestruct(
			RHBoutput *out,
			RHBinterface *iface);
	void generate_enddestruct(
			RHBoutput *out,
			RHBinterface *iface);
	void generate_parent_initdestruct(
			RHBoutput *out,
			RHBinterface *iface);
	void enum_attributes(RHBoutput *out,RHBinterface *iface);
	void generate_internal_attribute(RHBoutput *out,RHBinterface *iface,RHBattribute *attr);
	int generate_parent_resolved(RHBoutput *out,RHBinterface *iface,RHBinterface *parent,RHBoperation *op,int mode);
	void gen_set_attribute(RHBoutput *out,RHBinterface *iface,RHBattribute *attr);
	void gen_get_attribute(RHBoutput *out,RHBinterface *iface,RHBattribute *attr);
	void generate_passthru(RHBoutput *out,RHBinterface *iface,const char *which);

/*	inline void * operator new(size_t s)
	{
		return 0;
	}*/

	void gen_typecode_macro(RHBoutput *out,RHBtype *type,int nest);
	void gen_typecode_inner(RHBoutput *out,RHBtype *type,int nest);
	void gen_shortname(RHBoutput *out,RHBtype *type,const char *lead,int nest);
	void gen_shortname_sequence(RHBoutput *out,RHBtype *type,int nest);

	boolean type_is_sequence(RHBtype *t);
	boolean type_is_any(RHBtype *t);
	boolean op_is_varg(RHBoperation *op);
	RHBtype *get_array_slice(RHBtype *t);

	void public_class_data(RHBoutput *out,int nest,RHBinterface *iface,const char *n);
	void public_class_data_resolve(RHBoutput *out,int nest,RHBinterface *iface,const char *n);
};

typedef int (* myFunc)(int x,int y);

class SOMIR_element;
class SOMIR_record;
class SOMIR_free;

class RHBir_file : public RHBoutput
{
	FILE *fp;
	SOMIR_free *freeListItems;
	long fileLength;
	char filename[256];
public:
	long _get_fileLength() { return fileLength; }
	RHBir_file(const char *name);
	const char *_get_filename() { return filename; }

	long rootContainer;

	void write(const void *p,size_t len);

	~RHBir_file();

	void seek(long);
	void read(void *,unsigned long);
	boolean load(void);
	void free(long,long);
	long alloc(long,const char *,int);
};

class RHBir_emitter : public RHBemitter
{
public:
	RHBir_file *irFile;
	RHBir_emitter(RHBrepository *rr,RHBir_file *);
	~RHBir_emitter();
	 boolean generate(RHBoutput *out,const char *f);
};

#ifdef USE_SOURCE_EMITTER
class RHBsource_emitter: public RHBheader_emitter
{
public:
	RHBsource_emitter(RHBrepository *rr,RHBinclude_list *incls);
	boolean generate(RHBoutput *out,const char *f);
	void gen_def_for_each_class(RHBoutput *out,RHBelement *el);

	void gen_enum_classes(RHBoutput *out,RHBelement *el);
	void gen_overrides(RHBoutput *out,RHBinterface *iface);
	void gen_override_source(RHBoutput *out,RHBinterface *iface,RHBoperation *op);
	void gen_introduced(RHBoutput *out,RHBinterface *iface);
	void gen_introduced_source(RHBoutput *out,RHBinterface *iface,RHBoperation *op);
};
#endif

class RHBkernel_emitter: public RHBheader_emitter
{
public:
	RHBkernel_emitter(RHBrepository *rr);
	boolean generate(RHBoutput *out,const char *f);
	RHBinterface *get_interface(const char *);
	boolean can_override_op(RHBoperation *);
	boolean is_overridden(RHBoperation *);
	boolean is_aggregate(RHBoperation *);
};

class RHBapi_emitter: public RHBheader_emitter
{
public:
	RHBapi_emitter(RHBrepository *rr);
	boolean generate(RHBoutput *out,const char *f);
};

#define tk_null      1
#define tk_void      2
#define tk_short     3
#define tk_long      4
#define tk_ushort    5
#define tk_ulong     6
#define tk_float     7
#define tk_double    8
#define tk_boolean   9
#define tk_char      10
#define tk_octet     11
#define tk_any       12
#define tk_TypeCode  13
#define tk_Principal 14
#define tk_objref    15
#define tk_struct    16
#define tk_union     17
#define tk_enum      18
#define tk_string    19
#define tk_sequence  20
#define tk_array     21

#define tk_pointer   101 
#define tk_self      102 
#define tk_foreign   103 


#define only_ibm_som  1
#define gen_tc_macros 0

struct RHBself_tracker
{
	RHBtype *current;
	RHBself_tracker *previous;
};

class RHBtc_emitter: public RHBemitter
{
	boolean useAlignmentHelper(void) { return 1 /*0 */; };
public:
	RHBtc_emitter(RHBrepository *rr);
	~RHBtc_emitter();
	boolean generate(RHBoutput *out,const char *f);
	RHBelement_sequence extern_data;
	RHBelement_sequence static_data;
	RHBelement_sequence string_data;
	RHBelement_sequence referenced_tcs;
	void build_initial_list(RHBelement *el);
	void generate_externs(RHBoutput *out);
	void generate_aligns(RHBoutput *out,RHBelement_sequence *seq);
	RHBconstant *find_constant_for(RHBtype *type,RHBelement_sequence *seq);
	void build_secondary_list(RHBelement *el);
	void need_type(RHBtype *type);
	void need_string(const char *str);
	RHBvalue *find_string(const char *str);
	void generate_strings(RHBoutput *out);
	void generate_string(RHBoutput *out,RHBvalue *val);
	void generate_tc_pointers(RHBoutput *out,RHBelement_sequence *seq);
	void generate_tc_pointer(RHBoutput *out,RHBconstant *c);
	void generate_tcs(RHBoutput *out,RHBelement_sequence *seq);
	void generate_tc(RHBoutput *out,RHBconstant *c);
	void generate_TCKind(RHBoutput *out,RHBtype *type);
	void generate_length(RHBoutput *out,RHBtype *type);
	void generate_alignment(RHBoutput *out,RHBtype *type);
	void generate_sequence_data(RHBoutput *out,const char *name,RHBtype *type);
	void generate_interface_sequence_data(RHBoutput *out,RHBinterface *iface);
	void generate_enum_sequence_data(RHBoutput *out,RHBenum *en);
	void generate_sequence_sequence_data(RHBoutput *out,RHBsequence_type *seq);
	void generate_string_sequence_data(RHBoutput *out,RHBstring_type *str);
	void generate_struct_sequence_data(RHBoutput *out,RHBtype *str);
	void generate_extra_params(RHBoutput *out,RHBconstant *con,RHBtype *type);
	const char *struct_type_for_type(RHBtype *type);
	const char *element_type_for_type(RHBtype *type);
	void generate_idl_name(RHBoutput *out,RHBtype *type);
	void generate_name(RHBoutput *out,RHBtype *type);
};

void generate_TypeCode(RHBemitter *em,RHBoutput *out,RHBtype *type,const char *name);

