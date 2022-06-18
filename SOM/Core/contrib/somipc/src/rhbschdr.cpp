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

#include <rhbopt.h>
/* IDL compiler */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rhbsc.h>

static const char retVal_name[]="_somC_retVal";
static const char extern_C_static[]="SOM_EXTERN_C_STATIC";
const char *RHBheader_emitter::szSOM_RESOLVE_DATA="SOM_RESOLVE_DATA";

class extern_c_proto_t
{
	RHBheader_emitter *emit;
	RHBoutput *out;
	int nest;
public:
	extern_c_proto_t(RHBheader_emitter *e,RHBoutput *o,int n) :
	  emit(e),
	  out(o),
	  nest(n)
	{
		if (emit->cplusplus)
		{
			emit->out_printf(out,"#ifdef %s\n",extern_C_static);
			emit->extern_c_begin(out,nest);
			emit->out_printf(out,"#endif\n");
		}
	}
	~extern_c_proto_t()
	{
		if (emit->cplusplus)
		{
			emit->out_printf(out,"#ifdef %s\n",extern_C_static);
			emit->extern_c_end(out,nest);
			emit->out_printf(out,"#endif\n");
		}
	}
};

/* main emitter */

RHBheader_emitter::RHBheader_emitter(RHBrepository *rr)
: RHBemitter(rr)
{
}

RHBtextfile::RHBtextfile(const char *n)
{
	fp=0;
	filename=0;

	if (n)
	{
		size_t x=strlen(n)+1;
		filename=new char[x];
		strncpy(filename,n,x);
	}
	else
	{
		fp=stdout;
	}

/*	fp=fopen(n,"w");

	if (!fp) 
	{
		fprintf(stderr,"failed to open %s for writing\n",n);
		exit(1);
	}*/
}

void RHBheader_emitter::preflight_macros_from_idl_filename(const char *f)
{
	char *p=idl_filestem;
	char buf[256];

	idl_name=f;

	strncpy(buf,f,sizeof(buf));

	f=&buf[strlen(buf)];

	while (f > buf)
	{
		f--;

		if ((*f==':') || (*f=='\\') || (*f=='/'))
		{
			f++;
			break;
		}
	}

	strncpy(idl_filestem,f,sizeof(idl_filestem));
	p=idl_filestem;

	while (*p)
	{
		if (*p=='.')
		{
			*p=0;
		}
		else
		{
			p++;
		}
	}

	snprintf(idl_conditional,sizeof(idl_conditional),"SOM_Module_%s_Source",idl_filestem);
	snprintf(file_guard,sizeof(file_guard),"SOM_Module_%s_Header_",idl_filestem);
}

boolean RHBheader_emitter::generate(RHBoutput *out,const char *f)
{
	preflight_macros_from_idl_filename(f);
	
	out_printf(out,"/* generated from %s */\n",f);

	if (cplusplus)
	{
		if (internal)
		{
			strncat(file_guard,"xih",sizeof(file_guard));
		}
		else
		{
			strncat(file_guard,"xh",sizeof(file_guard));
		}
	}
	else
	{
		if (internal)
		{
			strncat(file_guard,"ih",sizeof(file_guard));
		}
		else
		{
			strncat(file_guard,"h",sizeof(file_guard));
		}
	}

	out_printf(out,"/* internal conditional is %s */\n",idl_conditional);
	out_printf(out,"#ifndef %s\n",file_guard);
	out_printf(out,"\t#define %s 1\n",file_guard);

	if (internal)
	{
		out_printf(out,"\t#ifndef %s\n",idl_conditional);
		out_printf(out,"\t\t#define %s\n",idl_conditional);
		out_printf(out,"\t#endif /* %s */\n",idl_conditional);
	}

	generate_passthrus(out,GetRepository(),1);

	generate_headers(out,1);

	generate_module(out,GetRepository(),1);

	out_printf(out,"#endif /* %s */\n",file_guard);

	return 0;
}

void RHBheader_emitter::generate_passthrus(RHBoutput *out,
			RHBelement *root,int nest)
{
	RHBelement *p=root->children();

	while (p)
	{
		if (ShouldEmit(p))
		{
			RHBinterface *iface=p->is_interface();

			if (iface)
			{
				if (cplusplus)
				{
					if (internal)
					{
						generate_passthru(out,iface,"C_xih");
					}
					else
					{
						generate_passthru(out,iface,"C_xh");
					}
				}
				else
				{
					if (internal)
					{
						generate_passthru(out,iface,"C_ih");
					}
					else
					{
						generate_passthru(out,iface,"C_h");
					}
				}
			}
		}

		generate_passthrus(out,p,nest);

		p=p->next();
	}
}

void RHBheader_emitter::generate_module(RHBoutput *out,RHBmodule *mod,int nest)
{
	if (!internal)
	{
		generate_class_protos(out,mod,nest);
	}

	generate_types(out,mod,nest);
	generate_classes(out,mod,nest,0);
	generate_modules(out,mod,nest+1);
}

RHBtextfile::~RHBtextfile()
{
	if (fp)
	{
		fflush(fp);

		if (fp!=stdout)
		{
			fclose(fp);
		}
		fp=0;
	}
	if (filename)
	{
		delete []filename;
		filename=0;
	}
}

static long cr_count;

void RHBtextfile::write(const void *pn,size_t len)
{
	const char *n=(const char *)pn;

	if (!fp)
	{
		fp=fopen(filename,"w");

		if (!fp)
		{
			fprintf(stderr,"Failed to open '%s' for writing\n",filename);
			exit(1);
		}
	}

	if (len > 0)
	{
		if (fp)
		{
			fwrite(n,len,1,fp);

			if (len--)
			{
				if (*n=='\n')
				{
					cr_count++;

					if (cr_count > 4)
					{
						cr_count=0;
						printf("Bad formatting\n");
					}
				}
				else
				{
					cr_count=0;
				}

				n++;
			}
		}
	}
}

void RHBheader_emitter::generate_headers(RHBoutput *out,int nest)
{
	RHBinclude_list *g=GetRepository()->_pp.includes;

	RHBelement *e=g->children();

	if (out)
	{
		if (!internal)
		{
			dump_nest(out,nest);

			if (cplusplus)
			{
				out_printf(out,"#include <som.xh>\n");
			}
			else
			{
				out_printf(out,"#include <som.h>\n");
			}
		}
	}

	if (testonly)
	{
		out_printf(out,"/* this is only a test */\n");
	}
	else
	{
		while (e)
		{
			RHBinclude_file *i=e->is_include_file();

			if (i)
			{
				if (i->defined_in 
					&& i->defined_in->id 
#if 0
					/* emit only immediate children */
					&& !strcmp(i->defined_in->id,idl_name)
#endif
					)
				{
					if (i->id && i->id[0] && strcmp(i->id,"-"))
					{
						char fn[256];
						char *p;

						strncpy(fn,i->id,sizeof(fn));

						p=&fn[strlen(fn)];

						while (p > fn)
						{
							p--;

							if (*p=='.')
							{
								p++;
								if (cplusplus)
								{
									strncpy(p,"xh",fn+sizeof(fn)-p);
								}
								else
								{
									strncpy(p,"h",fn+sizeof(fn)-p);
								}

								p=fn;
							}
						}

						if (i->included_during==GetRepository())
						{
							if (!internal)
							{
								dump_nest(out,nest);
								out_printf(out,"#include <%s>\n",fn);
							}
						}
						else
						{
							if (!internal)
							{
								dump_nest(out,nest);
								out_printf(out,"/* #include <%s> */\n",fn);
							}
						}
					}
				}
			}

			e=e->next();
		}

		if (internal)
		{
			char fn[256];
			char *p;

			strncpy(fn,idl_name,sizeof(fn));

			p=&fn[strlen(fn)];

			while (p > fn)
			{
				p--;

				if (*p=='.')
				{
					p++;
					if (cplusplus)
					{
						strncpy(p,"xh",fn+sizeof(fn)-p);
					}
					else
					{
						strncpy(p,"h",fn+sizeof(fn)-p);
					}

					p=fn;
				}
			}

			dump_nest(out,nest);
			out_printf(out,"#include <%s>\n",short_file_name(fn));
		}
	}
}

void RHBheader_emitter::generate_class_protos(RHBoutput *out,RHBmodule *mod,int nest)
{
	RHBelement *e;

	e=mod->children();

	while (e)
	{
		RHBinterface *iface;

		iface=e->is_interface();

		if (iface)
		{
			/* generates class prototypes for
			classes that have either been defined
			in this IDL file, or contain no
			definition in any other IDL file */


			if (ShouldEmit(iface))
			{
/*				if (!iface->instanceData)*/
				{
					generate_class_proto(out,iface,nest);
				}
			}
/*			else
			{
				char buf[256];
				get_ir_name(iface,buf);
				printf("%s prototype not being emitted\n",buf);
			}*/
		}

		e=e->next();
	}
}

void RHBheader_emitter::generate_types(RHBoutput *out,RHBelement *mod,int nest)
{
	RHBelement *e;

	if (internal) 
	{
		return;
	}

	e=mod->children();

	while (e)
	{
		RHBtype *t=0;

		if (ShouldEmit(e))
		{
			RHBtype *is_foreign=0;
			RHBtype *tt=e->is_type();
			if (tt)
			{
				RHBtype_def *d=tt->is_typedef();

	/*			printf("%s\n",e->id);

				if (!strcmp(e->id,"sockaddr"))
				{
					__asm int 3;
				}
*/
				if (d)
				{
					is_foreign=is_SOMFOREIGN(d->alias_for);
				}
			}

			if (!is_foreign)
			{
				t=e->is_type();
			}
		}

		if (t)
		{
			RHBstruct *s;
			RHBtype_def *td;
			RHBenum *en;
			RHBunion *u;

			if (!generated_types.contains(t))
			{
				int did_generate=1;

				s=t->is_struct();

				if (s)
				{
					generate_struct(s,out,nest,0,1,0);
				}
				else
				{
					td=t->is_typedef();

					if (td)
					{
						generate_typedef(td,out,nest);
					}
					else
					{
						en=t->is_enum();

						if (en)
						{
							generate_enum(en,out,nest);
						}
						else
						{
							u=t->is_union();

							if (u)
							{
								generate_union(u,out,0,1);
							}
							else
							{
								RHBexception *ex;

								ex=t->is_exception();

								if (ex)
								{
									generate_exception(ex,out,0,0);
								}
								else
								{
									RHBconstant *cn;

									cn=t->is_constant();

									if (cn)
									{
										generate_constant(out,cn,0);
									}
									else
									{
										did_generate=0;
									}
								}
							}
						}
					}
				}

				if (did_generate)
				{
					if (!generated_types.contains(t))
					{
						generated_types.add(t);
					}

					if (!internal)
					{
						gen_typecode_macro(out,t,nest);
					}
				}
			}
		}

		e=e->next();
	}
}

void RHBheader_emitter::gen_typecode_macro(RHBoutput *out,RHBtype *type,int nest)
{
	if (gen_tc_macros)
	{
		char n[256];

		if (type->is_constant())
		{
			return;
		}

		if (type->is_exception())
		{
			return;
		}

		dump_nest(out,nest);

		out_printf(out,"#ifdef SOM_TypeCodeNew\n");

		dump_nest(out,nest+1);
		get_c_name(type,n,sizeof(n));
		out_printf(out,"#define TypeCodeNew_%s()  \\\n",n);

		gen_typecode_inner(out,type,nest+3);
		out_printf(out,"\n");

		dump_nest(out,nest);
		out_printf(out,"#endif /* SOM_TypeCodeNew */\n");
	}
}

void RHBheader_emitter::gen_typecode_inner(RHBoutput *out,RHBtype *type,int nest)
{
	char n[256];
	int end_with_null=0;

	while (type->is_typedef())
	{
		type=type->is_typedef()->alias_for;
	}

	dump_nest(out,nest);
	nest++;

	out_printf(out,"TypeCodeNew(");

	if (type->is_sequence())
	{
		out_printf(out,"tk_sequence,\\\n");

		gen_typecode_inner(out,type->is_sequence()->sequence_of,nest);
		RHBvalue *val=type->is_sequence()->length;
		long m=0;
		if (val)
		{
			m=val->numeric_value();
		}
		out_printf(out,",%ld",m);
	}
	else
	{
		if (type->is_pointer())
		{
			out_printf(out,"tk_pointer,\\\n");
			gen_typecode_inner(out,type->is_pointer()->pointer_to,nest);
		}
		else
		{
			if (type->is_interface())
			{
				out_printf(out,"tk_objref,");
				get_ir_name(type,n,sizeof(n));
				out_printf(out,"%c%s%c",double_quote,n,double_quote);
			}
			else
			{
				if (type->is_struct())
				{
					RHBelement *e=type->children();
					out_printf(out,"tk_struct,");
					get_ir_name(type,n,sizeof(n));
					out_printf(out,"%c%s%c",double_quote,n,double_quote);
					while (e)
					{
						RHBtype *t2=e->is_type();
						if (t2)
						{
							RHBstruct_element *el=t2->is_struct_element();
							if (el)
							{
								out_printf(out,",\\\n");
								dump_nest(out,nest);
								out_printf(out,"%c%s%c,\\\n",double_quote,el->id,double_quote);
								gen_typecode_inner(out,el->element_type,nest+1);
							}
						}
						e=e->next();
					}
					end_with_null=1;
				}
				else
				{
					if (type->is_base_type())
					{
						RHBbase_type *bt=type->is_base_type();

						out_printf(out,"tk_%s",bt->id);
					}
					else
					{
						if (type->is_string())
						{
							RHBvalue *val=0;
							RHBtype *t=type->is_string()->length;

							if (t)
							{
								val=t->is_value();
							}
							long m=0;
							if (val)
							{
								m=val->numeric_value();
							}
							out_printf(out,"tk_string,%ld",m);
						}
						else
						{
							if (type->is_enum())
							{
								RHBelement *e;
								out_printf(out,"tk_enum");
								get_ir_name(type,n,sizeof(n));
								out_printf(out,",%c%s%c",double_quote,n,double_quote);
								e=type->children();
								while (e)
								{
									out_printf(out,",\\\n");
									dump_nest(out,nest);
									out_printf(out,"%c%s%c",double_quote,e->id,double_quote);
									e=e->next();
								}
								end_with_null=1;
							}
							else
							{
								if (type->is_array())
								{
									out_printf(out,"tk_array,\\\n");
									gen_typecode_inner(out,type->is_array()->array_of,nest);
									RHBvalue *val=type->is_array()->elements;
									long m=0;
									if (val)
									{
										m=val->numeric_value();
									}
									out_printf(out,",%ld",m);
								}
								else
								{
									if (type->is_constant())
									{
										bomb("is_constant");
									}
									else
									{
										if (type->is_qualified())
										{
											RHBtype *bt;
											bt=type->is_qualified()->base_type;
											if (!strcmp(bt->id,"long"))
											{
												out_printf(out,"tk_ulong");
											}
											else
											{
												out_printf(out,"tk_ushort");
											}
										}
										else
										{
											printf("error %s\n",type->classname());
											bomb("error");
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (end_with_null)
	{
		out_printf(out,",NULL");
	}

	out_printf(out,")");
}

void RHBheader_emitter::generate_classes(RHBoutput *out,RHBmodule *mod,int nest,int apiOnlyFlag)
{
	RHBelement *e=mod->children();

	while (e)
	{
		RHBinterface *iface=e->is_interface();

		if (iface)
		{
			if (ShouldEmit(iface))
			{
				generate_class(out,iface,nest,apiOnlyFlag);
			}
		}

		e=e->next();
	}
}

void RHBheader_emitter::generate_modules(RHBoutput *out,RHBmodule *mod,int nest)
{
	RHBelement *e;

	e=mod->children();

	while (e)
	{
		RHBmodule *c;

		c=e->is_module();

		if (c)
		{
			generate_module(out,c,nest);
		}

		e=e->next();
	}
}

void RHBheader_emitter::generate_class_proto(RHBoutput *out,RHBinterface *iface,int nest)
{
	char n[256];

	get_c_name(iface,n,sizeof(n));

	if (!cplusplus)
	{
		if (strcmp(n,"SOMObject"))
		{
			boolean is_false=0;

			if (only_ibm_som && is_false)
			{
				dump_nest(out,nest);
				out_printf(out,"#ifndef %s\n",n);
				dump_nest(out,nest+1);
				out_printf(out,"#define %s SOMObject\n",n);
				dump_nest(out,nest);
				out_printf(out,"#endif /* %s */\n",n);
			}
			else
			{
				dump_nest(out,nest);
				out_printf(out,"#ifndef _IDL_%s_defined\n",n);
				dump_nest(out,nest+1);
				out_printf(out,"#define _IDL_%s_defined\n",n);
				dump_nest(out,nest+1);
				out_printf(out,"typedef SOMObject %s;\n",n);
				dump_nest(out,nest);
				out_printf(out,"#endif /* _IDL_%s_defined */\n",n);
			}
		}
	}

	if (cplusplus)
	{
		if (strcmp(n,"SOMObject"))
		{
			dump_nest(out,nest);
			out_printf(out,"class %s;\n",n);
		}
	}

	generate_sequence(out,iface,nest);
}

void RHBheader_emitter::generate_enum(RHBenum *en,RHBoutput *out,int nest)
{
	char n[256];
	RHBelement *el;

	get_c_name(en,n,sizeof(n));

	dump_nest(out,nest);
	out_printf(out,"#ifndef %s_defined\n",n);
	dump_nest(out,nest+1);
	out_printf(out,"#define %s_defined\n",n);

	dump_nest(out,nest+1);
	out_printf(out,"typedef unsigned long %s;\n",n);

	el=en->children();

	while (el)
	{
		RHBtype *t=el->is_type();
		if (t)
		{
			RHBvalue *val=t->is_value();

			if (val)
			{
				long m=val->numeric_value();

				if (0)
				{
					get_c_name(val,n,sizeof(n));
					out_printf(out,"/* #define %s %ldUL */\n",n,m);
				}

				get_c_name(en->parent(),n,sizeof(n));

				dump_nest(out,nest+1);

				if (n[0]) strncat(n,"_",sizeof(n)-1);

				out_printf(out,"#ifndef %s%s\n",n,el->id);

				dump_nest(out,nest+2);
				out_printf(out,"#define %s%s %ldUL\n",n,el->id,m);

				dump_nest(out,nest+1);
				out_printf(out,"#endif /* %s%s */\n",n,el->id);
			}
		}
		el=el->next();
	}

	gen_shortname(out,en,0,nest+1);

	get_c_name(en,n,sizeof(n));

	dump_nest(out,nest);
	out_printf(out,"#endif /* %s_defined */\n",n);
}

void RHBheader_emitter::generate_typedef(RHBtype_def *td,RHBoutput *out,int nest)
{
	char n[256];

/*	if (!nest) 
	{
		printf("blah\n");
	}
*/
	if (td->alias_for->is_sequence())
	{
		generate_sequence(out,td->alias_for->is_sequence()->sequence_of,nest);
	}

	get_c_name(td,n,sizeof(n));

	dump_nest(out,nest);

	out_printf(out,"typedef ");

	use_type(out,td->alias_for,n);

	out_printf(out,";\n");

	generate_sequence(out,td,nest);
}

void RHBheader_emitter::generate_exception(RHBexception *s,RHBoutput *out,int nesting,const char *alias_name)
{
	char c_name[256];
	RHBelement *el;

	get_c_name(s,c_name,sizeof(c_name));

	out_printf(out,"#define ex_%s   ",c_name);

	{
		char ir_name[256];
		get_ir_name(s,ir_name,sizeof(ir_name));
		out_printf(out,"%c%s%c\n",double_quote,ir_name,double_quote);
	}

	if (strcmp(c_name,s->id))
	{
		out_printf(out,"#ifndef SOM_DONT_USE_SHORT_NAMES\n");
		out_printf(out,"#ifndef SOMTGD_ex_%s\n",s->id);

		out_printf(out,"#ifndef ex_%s\n",s->id);

		out_printf(out,"#define ex_%s  ex_%s\n",s->id,c_name);

		out_printf(out,"#else\n");

		out_printf(out,"#define SOMTGD_ex_%s\n",s->id);
		out_printf(out,"#undef ex_%s\n",s->id);

		out_printf(out,"#endif /* ex_%s */\n",s->id);

		out_printf(out,"#endif /* SOMTGD_ex_%s */\n",s->id);

		out_printf(out,"#endif /* SOM_DONT_USE_SHORT_NAMES */\n");
	}

	dump_nest(out,nesting);

	if (alias_name)
	{
		out_printf(out,"typedef struct %s",alias_name);
	}
	else
	{
		out_printf(out,"typedef struct %s",c_name);
	}

	if (s->children())
	{
		out_printf(out,"\n");
		dump_nest(out,nesting);
		out_printf(out,"{\n");

		el=s->children();

		while (el)
		{
			RHBtype *t=el->is_type();

			if (t)
			{
				RHBstruct_element *se=t->is_struct_element();

				if (se)
				{
					generate_struct_element(se,out,nesting+1);
				}
			}

			el=el->next();
		}

		dump_nest(out,nesting);
		out_printf(out,"} ");
	}
	else
	{
		out_printf(out," ");
	}

	if (alias_name)
	{
		out_printf(out,"%s;\n",alias_name);
	}
	else
	{
		out_printf(out,"%s;\n",c_name);
	}

	gen_shortname(out,s,0,nesting);
}

void RHBheader_emitter::generate_struct(
			RHBstruct *s,
			RHBoutput *out,
			int nest,
			const char *alias_name,
			boolean user_data,
			boolean instanceData)
{
	char n[256];
	RHBelement *el;

	align_begin(out,user_data,nest);

	get_c_name(s,n,sizeof(n));

	dump_nest(out,nest);

	if (alias_name)
	{
		out_printf(out,"typedef struct %s",alias_name);
	}
	else
	{
		out_printf(out,"typedef struct %s",n);
	}

	if (s->children())
	{
		out_printf(out,"\n");

		dump_nest(out,nest);
		out_printf(out,"{\n");

		el=s->children();

		while (el)
		{
			RHBtype *t=el->is_type();

			if (t)
			{
				RHBstruct_element *se=t->is_struct_element();

				if (se)
				{
					generate_struct_element(se,out,nest+1);
				}
			}

			el=el->next();
		}

		dump_nest(out,nest);
		out_printf(out,"} ");
	}
	else
	{
		out_printf(out," ");
	}

	if (alias_name)
	{
		out_printf(out,"%s;\n",alias_name);
	}
	else
	{
		out_printf(out,"%s;\n",n);
	}

	align_end(out,user_data,nest);

	if (!instanceData)
	{
		if (user_data) 
		{
			generate_sequence(out,s,nest);
		}

		gen_shortname(out,s,0,nest);

		if (user_data)
		{
			gen_shortname_sequence(out,s,nest);
		}
	}
}

void RHBheader_emitter::generate_class(RHBoutput *out,RHBinterface *iface,int nest,int apiOnlyFlag)
{
	int doCloseAPI=0;

	if (!ShouldEmit(iface))
	{
		return;
	}

	if (iface->instanceData)
	{
		if (internal)
		{
			dump_nest(out,nest);
			out_printf(out,"#ifndef SOM_Scope\n");
			dump_nest(out,nest+1);
/*			if (cplusplus)
			{
				out_printf(out,"#define SOM_Scope   extern %cC%c \n",double_quote,double_quote);
			}
			else
			{
				out_printf(out,"#define SOM_Scope \n");
			}
*/
				out_printf(out,"#define SOM_Scope static\n");
			dump_nest(out,nest);
			
			out_printf(out,"#endif /* SOM_Scope */\n");

/*			get_c_name(iface,n);
			dump_nest(out,nest);
			i=sprintf(buf,"#ifndef %s_Class_Source\n",n);
			out->write(buf,i);
			dump_nest(out,nest+1);
			i=sprintf(buf,"#define %s_Class_Source\n",n);
			out->write(buf,i);
			dump_nest(out,nest);
			i=sprintf(buf,"#endif %s_Class_Source\n",n);
			out->write(buf,i);*/
		}
	}

	if (iface->instanceData && cplusplus && !internal)
	{
		char n[256];
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"#ifndef %s_API\n",n);
		out_printf(out,"#define %s_API\n",n);

		doCloseAPI=1;
	}

	generate_types(out,iface,nest);

	if (iface->instanceData && !generated_types.contains(iface))
	{
		size_t i=0;

		while (i < iface->_parents.length())
		{
			RHBelement *el=iface->_parents.get(i);

			if (el)
			{
				RHBinterface *par=el->is_interface();

				if (!generated_types.contains(par))
				{
				/*	char buf[256];
					get_ir_name(par,buf);*/
					generate_class(out,par,nest,apiOnlyFlag);
				}
			}

			i++;
		}

		generated_types.add(iface);

		if (!internal)
		{
			generate_somTDs(out,iface,nest);
		}

		if (!internal)
		{
			generate_name_lookups(out,iface,nest);
		}

		if (!internal)
		{
			generate_versions(out,iface,"MajorVersion","majorversion",nest);
			generate_versions(out,iface,"MinorVersion","minorversion",nest);
		}

		generate_class_data(out,iface,nest);


		if (doCloseAPI)
		{
			char n[256];
			get_c_name(iface,n,sizeof(n));
			out_printf(out,"#endif /* %s_API */\n",n);

			doCloseAPI=0;
		}

		if (!apiOnlyFlag)
		{
			if (internal)
			{
				generate_instance_data(out,iface,nest);
			}

			generate_interface(out,iface,nest);

			if (cplusplus)
			{
				if (internal)
				{
					generate_passthru(out,iface,"C_xih_after");
				}
				else
				{
					generate_passthru(out,iface,"C_xh_after");
				}
			}
			else
			{
				if (internal)
				{
					generate_passthru(out,iface,"C_ih_after");
				}
				else
				{
					generate_passthru(out,iface,"C_h_after");
				}
			}
		}
	}

	if (doCloseAPI)
	{
		char n[256];
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"#endif /* %s_API */\n",n);

		doCloseAPI=0;
	}
}

void RHBemitter::dump_nest(RHBoutput *out,int nest)
{
	while (nest--)
	{
		out->write("\t",1);
	}
}

void RHBheader_emitter::generate_struct_element(RHBstruct_element *el,RHBoutput *out,int nest)
{
	dump_nest(out,nest);

	use_type(out,el->element_type,el->id);

	out->write(";\n",2);
}

void RHBheader_emitter::init_type(RHBoutput *out,RHBtype *typ_init)
{
	RHBstruct *st=typ_init->is_struct();

	if (st)
	{
		RHBelement *el=st->children();

		out_printf(out,"{");

		while (el)
		{
			RHBtype *typ=el->is_type();

			if (typ)
			{
				RHBstruct_element *se=typ->is_struct_element();
				if (se)
				{
					init_type(out,se->element_type);
				}
			}

			el=el->next();

			if (el)
			{
				out_printf(out,",");
			}
		}

		out_printf(out,"}");
	}
	else
	{
		RHBsequence_type *seq=typ_init->is_sequence();
		if (seq)
		{
			out_printf(out,"{0,0,NULL}");
		}
		else
		{
			RHBtype_def *td=typ_init->is_typedef();

			if (td)
			{
				init_type(out,td->alias_for);
			}
			else
			{
				RHBany_type *at=typ_init->is_any();
				if (at)
				{
					out_printf(out,"{NULL,NULL}");
				}
				else
				{
					if (typ_init->is_interface()
						||
						typ_init->is_string()
						||
						typ_init->is_TypeCode())
					{
						out_printf(out,"NULL");
					}
					else
					{
						RHBarray *ar=typ_init->is_array();
						if (ar)
						{
							long val=ar->elements->numeric_value();
							while (val--)
							{
								init_type(out,ar->array_of);

								if (val)
								{
									out_printf(out,",");
								}
							}
						}
						else
						{
							out_printf(out,"0");
						}
					}
				}
			}
		}
	}
}

void RHBheader_emitter::use_type(RHBoutput *out,RHBtype *typ,const char *name)
{	
	RHBarray *ar=typ->is_array();;

	if (ar)
	{
		long val=ar->elements->numeric_value();
		char valstr[32];
		snprintf(valstr,sizeof(valstr),"[%ld]",val);
		int i=strlen(valstr);
		int k=name ? strlen(name) : 0;
		char *n2=new char[i+1+k];
		n2[i+k]=0;
		if (k)
		{
			memcpy(n2,name,k);
		}
		memcpy(n2+k,valstr,i);

		use_type(out,ar->array_of,n2);

/*		out_printf(out,"[%ld]",val);*/

		delete n2;

		return;
	}

	RHBinterface *iface=typ->is_interface();

	if (iface)
	{
		char n[256];

		get_c_name(typ,n,sizeof(n));

		if (cplusplus)
		{
			out_printf(out,"%s * ",n);
		}
		else
		{
			out_printf(out,"%s SOMSTAR ",n);
		}
	}
	else
	{
		RHBpointer_type *ptr=typ->is_pointer();

		if (ptr)
		{
			use_type(out,ptr->pointer_to,0);
			out_printf(out,"*");
		}
		else
		{
			RHBstring_type * str;

			str=typ->is_string();

			if (str)
			{
				/* hmm */
				out_printf(out,"%s ",get_corbastring());
			}
			else
			{
				RHBqualified_type *qual=typ->is_qualified();

				if (qual)
				{
					out_printf(out,"%s ",qual->id);
					use_type(out,qual->base_type,0);
				}
				else
				{
					char n[256];
					RHBstruct *st=typ->is_struct();

					if (st)
					{
						get_c_name(st,n,sizeof(n)); /* did say 'struct %s '*/
						out_printf(out,"%s ",n);
					}
					else
					{
						RHBunion *u=typ->is_union();

						if (u)
						{
							get_c_name(u,n,sizeof(n));
							out_printf(out,"%s ",n);
						}
						else
						{
							RHBsequence_type *seq=typ->is_sequence();

							if (seq)
							{
								get_sequence_name(seq,n,sizeof(n));

								out_printf(out,"%s ",n);
							}
							else
							{
								get_c_name(typ,n,sizeof(n));
		
								if (!n[0])
								{
									snprintf(n,sizeof(n),"<<%s>> ",typ->classname());
								}

								if (0==strcmp(n,"Istring"))
								{
									bomb("Istring");
								}

								out_printf(out,"%s ",n);
							}
						}
					}
				}
			}
		}
	}

	if (name) 
	{
		out_printf(out,"%s",name);
	}
}

void RHBheader_emitter::get_sequence_name(RHBsequence_type *seq,char *name,size_t len)
{
	char buf[256];

	if (seq==seq->sequence_of)
	{
		bomb("get_sequence_name");
	}

	get_most_primitive_name(seq->sequence_of,buf,sizeof(buf));

	snprintf(name,len,"_IDL_SEQUENCE_%s",buf);
}

RHBtype * RHBheader_emitter::get_most_primitive_name(RHBtype *typ,char *name,size_t namelen)
{
	RHBqualified_type *qual;

	*name=0;

	while (typ->is_typedef())
	{
		RHBtype_def *td=typ->is_typedef();

		if (td->alias_for->is_pointer())
		{
			break;
		}

		if (td->alias_for->is_sequence())
		{
			break;
		}

		if (td->alias_for->is_array())
		{
			break;
		}

		char buf[256];

		get_c_name(td->alias_for,buf,sizeof(buf));

		if (0==strcmp(buf,"SOMFOREIGN"))
		{
			break;
		}

		typ=td->alias_for;
	}

	qual=typ->is_qualified();

	if (qual)
	{
		if (0==strcmp(qual->id,"unsigned"))
		{
			strncpy(name,"u",namelen);

			get_c_name(qual->base_type,&name[1],namelen-1);
		}
		else
		{
			get_c_name(qual->base_type,name,namelen);
		}
	}
	else
	{
		RHBsequence_type *s=typ->is_sequence();

		if (s)
		{
			get_sequence_name(s,name,namelen);
		}
		else
		{
			if (typ->is_string())
			{
				strncpy(name,get_corbastring(),namelen);
			}
			else
			{
				get_c_name(typ,name,namelen);
			}
		}
	}

	return typ;
}

void RHBheader_emitter::generate_sequence(RHBoutput *out,RHBtype *typ,int nest)
{
	char n[256];

	if (!typ) return;

	typ=get_most_primitive_name(typ,n,sizeof(n));

	if (n[0])
	{
		int i=0;

		while (sequences.get(i))
		{
			const char *p=sequences.get(i);
			if (0==strcmp(p,n))
			{
				return;
			}

			i++;
		}

		sequences.add(n);

		dump_nest(out,nest);
		out_printf(out,"#ifndef _IDL_SEQUENCE_%s_defined\n",n);
		dump_nest(out,nest+1);
		out_printf(out,"#define _IDL_SEQUENCE_%s_defined\n",n);

		dump_nest(out,nest+1);

		if (typ->is_interface() 
				|| typ->is_pointer() 
				|| typ->is_qualified() 
				|| typ->is_array() 
				|| typ->is_sequence())
		{
			out_printf(out,"SOM_SEQUENCE_TYPEDEF_NAME(");

			use_type(out,typ,0);

			out_printf(out,",sequence(%s));\n",n);
		}
		else
		{
			out_printf(out,"SOM_SEQUENCE_TYPEDEF(%s);\n",n);
		}

		dump_nest(out,nest);
		
		out_printf(out,"#endif /* _IDL_SEQUENCE_%s_defined */\n",n);
	}
}

void RHBheader_emitter::generate_union(RHBunion *u,RHBoutput *out,int nesting,boolean user_data)
{
	char n[256];

	align_begin(out,user_data,nesting);

	get_c_name(u,n,sizeof(n));

	dump_nest(out,nesting);

	out_printf(out,"typedef struct %s ",n);

	if (u->elements.length())
	{
		unsigned long index=0;
		out->write("\n",1);

		dump_nest(out,nesting);

		out_printf(out,"{\n");

		dump_nest(out,nesting+1);

		use_type(out,u->switch_type,"_d");

		out_printf(out,";\n");

		dump_nest(out,nesting+1);

		out_printf(out,"union\n");

		dump_nest(out,nesting+1);

		out_printf(out,"{\n");

		while (index < u->elements.length())
		{
			RHBelement *el=u->elements.get(index);
			RHBtype *t=el->is_type();

			if (t)
			{
				RHBunion_element *e=t->is_union_element();

				if (e)
				{
					dump_nest(out,nesting+2);
					use_type(out,e->element_type,e->id);
					out_printf(out,";\n");
				}
			}

			index++;
		}

		dump_nest(out,nesting+1);
		out_printf(out,"} _u;\n");
		dump_nest(out,nesting);
		out_printf(out,"} ");
	}

	out_printf(out,"%s;\n",n);

	align_end(out,user_data,nesting);
}

void RHBheader_emitter::generate_instance_data(RHBoutput *out,RHBinterface *iface,int nest)
{
	if (internal)
	{
		generate_class_guard(out,nest,iface,1);

		if (iface->instanceData)
		{
			char n[256];

			get_c_name(iface,n,sizeof(n));

			strncat(n,"Data",sizeof(n)-1);

			if (iface->instanceData->children())
			{
				RHBelement *el;

				generate_struct(iface->instanceData,out,nest+1,n,1,1);

				el=iface->instanceData->children();

				while (el)
				{
					dump_nest(out,nest);
					out_printf(out,"#ifndef _%s\n",el->id);
					dump_nest(out,nest+1);
					out_printf(out,"#define _%s  (somThis->%s)\n",
							el->id,el->id);
					dump_nest(out,nest);
					out_printf(out,"#endif /* _%s */\n",el->id);
					el=el->next();
				}
			}
			else
			{
				dump_nest(out,nest+1);
				out_printf(out,"typedef void %s;\n",n);
			}
		}
		generate_class_guard(out,nest,iface,0);
	}
}

void RHBheader_emitter::generate_class_data(RHBoutput *out,RHBinterface *iface,int nest)
{
	if (!iface->classData)
	{
		char buf[256];
		RHBelement *e;
		RHBinterface *ifa;
		RHBstruct_element *se;
		RHBtype *token;

		get_c_name(iface,buf,sizeof(buf));
		strncat(buf,"ClassDataStructure",sizeof(buf)-1);
		iface->classData=new RHBstruct(0,buf);

		buf[0]=0;

		get_meta_class(iface,buf);

		if (!buf[0])
		{
			strncpy(buf,"SOMClass",sizeof(buf));
		}

		e=iface->find_named_element(0,buf,sizeof(buf));

		ifa=e->is_interface();
	
		strncpy(buf,"somMToken",sizeof(buf));

		e=iface->find_named_element(0,buf,sizeof(buf));

		token=e->is_type();

		se=new RHBstruct_element(0,"classObject");
		se->element_type=ifa;

		iface->classData->add(0,se);

		strncpy(buf,"releaseorder",sizeof(buf));

		e=iface->find_named_element(0,buf,sizeof(buf));

		if (e)
		{
			int i=0;

			while (e->modifier_data.get(i))
			{
				const char *p=e->modifier_data.get(i);
				i++;

				if (strcmp(p,","))
				{
					RHBoperation *op=0;
					RHBelement *oe;

					strncpy(buf,p,sizeof(buf));

					oe=iface->find_named_element(0,buf,sizeof(buf));
					if (oe) 
					{
						op=oe->is_operation();
					}

					se=new RHBstruct_element(0,p);

			/*		if (0==strcmp(p,"somClassOfNewClassWithParents"))
					{
						__asm int 3;
					}
			*/
					if (is_operation_procedure(op))
					{
						RHBtype_def *td;
						char n[256];
						strncpy(n,"somTD_",sizeof(n));
						get_c_name(op,&n[strlen(n)],sizeof(n)-strlen(n));
						td=new RHBtype_def(0,n);
						td->alias_for=token;
						se->element_type=td;
					}
					else
					{
						se->element_type=token;
					}

					iface->classData->add(0,se);
				}
			}
		}

		e=iface->children();

		while (e)
		{
			RHBoperation *op=e->is_operation();

			if (op)
			{
				strncpy(buf,op->id,sizeof(buf));

				if (!iface->classData->find_named_element(0,buf,sizeof(buf)))
				{
					printf("%c%s%c, line %d: warning: ",
							double_quote,
							op->defined_in->id,
							double_quote,
							op->defined_line
							);

					printf("%c%s%c is not in releaseorder\n",double_quote,op->id,double_quote);

					se=new RHBstruct_element(0,op->id);

					if (is_operation_procedure(op))
					{
						RHBtype_def *td;
						char n[256];
						strncpy(n,"somTD_",sizeof(n));
						get_c_name(op,&n[strlen(n)],sizeof(n)-strlen(n));
						td=new RHBtype_def(0,n);
						td->alias_for=token;
						se->element_type=td;
					}
					else
					{
						se->element_type=token;
					}

					iface->classData->add(0,se);
				}
			}

			e=e->next();
		}
	}

	if (!iface->cclassData)
	{
		char buf[256];
		RHBelement *e;
		RHBstruct_element *se;

		get_c_name(iface,buf,sizeof(buf));
		strncat(buf,"CClassDataStructure",sizeof(buf)-1);

		iface->cclassData=new RHBstruct(0,buf);

		se=new RHBstruct_element(0,"parentMtab");

		strncpy(buf,/*"somParentMtabStructPtr"*/ "somMethodTabs",sizeof(buf));
		e=iface->find_named_element(0,buf,sizeof(buf));
		se->element_type=e->is_type();
		iface->cclassData->add(0,se);

		se=new RHBstruct_element(0,"instanceDataToken");
		strncpy(buf,"somDToken",sizeof(buf));
		e=iface->find_named_element(0,buf,sizeof(buf));
		se->element_type=e->is_type();
		iface->cclassData->add(0,se);

		RHBelement_sequence vaops;

		if (get_va_operations(iface,&vaops))
		{
			size_t i=0;
			while (i < vaops.length())
			{
				RHBoperation *op=vaops.get(i)->is_operation();

				if (op)
				{
					se=new RHBstruct_element(0,op->id);
					strncpy(buf,"somMethodPtr",sizeof(buf));
					e=iface->find_named_element(0,buf,sizeof(buf));
					se->element_type=e->is_type();
					iface->cclassData->add(0,se);
				}
				i++;
			}
		}
	}

	if (!internal)
	{
		generate_struct(iface->classData,out,nest,0,0,0);
		generate_struct(iface->cclassData,out,nest,0,0,0);
	}

	if (!internal)
	{
		char n[256];

		get_c_name(iface,n,sizeof(n));

		dump_nest(out,nest);
		out_printf(out,"#ifdef %s\n",szSOM_RESOLVE_DATA);

		public_class_data(out,nest+1,iface,"ClassData");
		
		dump_nest(out,nest);
		out_printf(out,"#else /* %s */\n",szSOM_RESOLVE_DATA);

		nest++;


#ifdef _DEBUG
		if (!iface->has_modifier_value("dllname"))
		{
			out_printf(out,"/* interface for \"%s\" does not have \"dllname\" modifier */\n",n);
			out_printf(out,"#define %sClassData   _somC_private_%sClassData\n",n,n);
		}
#endif

		dump_nest(out,nest);
		out_printf(out,"SOMEXTERN \n");

		ifdef_import_export(out,iface,nest);
		dump_nest(out,nest);
		out_printf(out,"struct ");
		out_printf(out,"%sClassDataStructure SOMDLINK %sClassData;\n",n,n);

		nest--;
		dump_nest(out,nest);
		out_printf(out,"#endif /* %s */\n",szSOM_RESOLVE_DATA);
	}
	
	if (!internal)
	{
		char n[256];

		get_c_name(iface,n,sizeof(n));

		dump_nest(out,nest);
		out_printf(out,"#ifdef %s\n",szSOM_RESOLVE_DATA);

		public_class_data(out,nest+1,iface,"CClassData");
		
		dump_nest(out,nest);
		out_printf(out,"#else /* %s */\n",szSOM_RESOLVE_DATA);

		nest++;

		dump_nest(out,nest);
		
		out_printf(out,"SOMEXTERN \n");

		ifdef_import_export(out,iface,nest);
		dump_nest(out,nest);
		
		out_printf(out,"struct %sCClassDataStructure SOMDLINK ",n);

		out_printf(out,"%sCClassData;\n",n);

		nest--;

		dump_nest(out,nest);
		out_printf(out,"#endif /* %s */\n",szSOM_RESOLVE_DATA);
	}

	if (!internal)
	{
		char n[256];

		extern_c_begin(out,nest);
		dump_nest(out,nest);
		out_printf(out,"extern \n");
		ifdef_import_export(out,iface,nest);
		dump_nest(out,nest);

		get_actual_meta_class(iface,n,sizeof(n));
		if (cplusplus)
		{
			out_printf(out,"%s * ",n);
		}
		else
		{
			out_printf(out,"%s SOMSTAR ",n);
		}

		get_c_name(iface,n,sizeof(n));
		out_printf(out,"SOMLINK %sNewClass\n",n);

		dump_nest(out,2+nest);
		
		out_printf(out,"(integer4 somtmajorVersion,integer4 somtminorVersion);\n");

		extern_c_end(out,nest);

#if 0
		if (only_ibm_som)
		{
			/* this is IBM SOM 2.1 style */
			dump_nest(out,nest);
			out_printf(out,"#ifndef _%s\n",n);

			dump_nest(out,nest+1);
			out_printf(out,"#define _%s (%sClassData.classObject ? \\\n",n,n);

			dump_nest(out,nest+2);
			out_printf(out,"%sClassData.classObject : \\\n",n);

			dump_nest(out,nest+3);
			out_printf(out,"%sNewClass(%s_MajorVersion,%s_MinorVersion))\n",n,n,n);
			
			dump_nest(out,nest);
			out_printf(out,"#endif /* _%s */\n",n);
		}
#else
		{
			/* this is IBM SOM 3.0 style */
			dump_nest(out,nest);
			out_printf(out,"#define _SOMCLASS_%s (%sClassData.classObject)\n",n,n);

			dump_nest(out,nest);
			out_printf(out,"#ifndef SOMGD_%s\n",n);

			dump_nest(out,nest+1);
			out_printf(out,"#if (defined(_%s) || defined(__%s))\n",n,n);
			dump_nest(out,nest+2);
			out_printf(out,"#undef _%s\n",n);
			dump_nest(out,nest+2);
			out_printf(out,"#undef __%s\n",n);
			dump_nest(out,nest+2);
			out_printf(out,"#define SOMGD_%s 1\n",n);
			dump_nest(out,nest+1);
			out_printf(out,"#else\n");
			dump_nest(out,nest+2);
			out_printf(out,"#define _%s _SOMCLASS_%s\n",n,n);
			dump_nest(out,nest+1);
			out_printf(out,"#endif /* _%s */\n",n);

			dump_nest(out,nest);
			out_printf(out,"#endif /* SOMGD_%s */\n",n);

			dump_nest(out,nest);
			out_printf(out,"#define %s_classObj _SOMCLASS_%s\n",n,n);

			dump_nest(out,nest);
			out_printf(out,"#define _SOMMTOKEN_%s(method) ((somMToken)(%sClassData.method))\n",n,n);
		}
#endif

		if (!cplusplus)
		{
#if 0
			if (only_ibm_som)
			{
				/* IBM SOM 2.1 style */
				dump_nest(out,nest);
				out_printf(out,"#ifndef %sNew\n",n);
				dump_nest(out,nest+1);
				out_printf(out,"#define %sNew() \\\n",n);
				dump_nest(out,nest+2);
				out_printf(out,"(SOMClass_somNew(_%s))\n",n);
				dump_nest(out,nest);
				out_printf(out,"#endif /* New%s */\n",n);
			}
#else
			/* IBM SOM 3.0 style */
			{

				dump_nest(out,nest);
				out_printf(out,"#ifndef %sNew\n",n);
				dump_nest(out,nest+1);
				out_printf(out,"#define %sNew() ( _%s ? \\\n",n,n);
				dump_nest(out,nest+2);
				out_printf(out,"(SOMClass_somNew(_%s)) : \\\n",n);
				dump_nest(out,nest+2);
				out_printf(out,"( %sNewClass( \\\n",n);
				dump_nest(out,nest+3);
				out_printf(out,"%s_MajorVersion, \\\n",n);
				dump_nest(out,nest+3);
				out_printf(out,"%s_MinorVersion), \\\n",n);
				dump_nest(out,nest+2);
				out_printf(out,"SOMClass_somNew(_%s))) \n",n);
				dump_nest(out,nest);
				out_printf(out,"#endif /* New%s */\n",n);
			}
#endif
		}
	}
}

void RHBheader_emitter::public_class_data(RHBoutput *out,int nest,RHBinterface *iface,const char *dn)
{
	char n[256];

	get_c_name(iface,n,sizeof(n));

	dump_nest(out,nest);
	out_printf(out,"#if defined(%s) || defined(%s_Class_Source)\n",idl_conditional,n);

	nest++;

	dump_nest(out,nest);
	out_printf(out,"SOMEXTERN struct %s%sStructure _%s%s;\n",n,dn,n,dn);

	dump_nest(out,nest);
	out_printf(out,"#ifndef %s%s\n",n,dn); 

	dump_nest(out,nest+1);
	out_printf(out,"#define %s%s    _%s%s\n",n,dn,n,dn); 

	dump_nest(out,nest);
	out_printf(out,"#endif /* %s%s */\n",n,dn); 

	nest--;

	dump_nest(out,nest);
	out_printf(out,"#else\n");

	nest++;

	dump_nest(out,nest);
	out_printf(out,"SOMEXTERN struct %s%sStructure * SOMLINK resolve_%s%s(void);\n",n,dn,n,dn); 

	dump_nest(out,nest);
	out_printf(out,"#ifndef %s%s\n",n,dn); 

	dump_nest(out,nest+1);
	out_printf(out,"#define %s%s    (*(resolve_%s%s()))\n",n,dn,n,dn); 

	dump_nest(out,nest);
	out_printf(out,"#endif /* %s%s */\n",n,dn); 

	nest--;

	dump_nest(out,nest);
	out_printf(out,"#endif\n"); 
}

void RHBheader_emitter::extern_c_begin(RHBoutput *out,int nest)
{
/*	dump_nest(out,nest);
	i=sprintf(buf,"#ifdef __cplusplus\n");
	out->write(buf,i);
	dump_nest(out,nest+1);
	i=sprintf(buf,"extern %cC%c {\n",double_quote,double_quote);
	out->write(buf,i);
	dump_nest(out,nest);
	i=sprintf(buf,"#endif\n");
	out->write(buf,i);*/

	if (cplusplus)
	{
		dump_nest(out,nest);
		out_printf(out,"extern %cC%c {\n",double_quote,double_quote);
	}
}

void RHBheader_emitter::extern_c_end(RHBoutput *out,int nest)
{
/*	dump_nest(out,nest);
	i=sprintf(buf,"#ifdef __cplusplus\n");
	out->write(buf,i);
	dump_nest(out,nest+1);
	i=sprintf(buf,"}\n");
	out->write(buf,i);
	dump_nest(out,nest);
	i=sprintf(buf,"#endif\n");
	out->write(buf,i);*/

	if (cplusplus)
	{
		dump_nest(out,nest);
		out_printf(out,"}\n");
	}
}

void RHBheader_emitter::ifdef_import_export(RHBoutput *out,RHBinterface *iface,int nest)
{
	if (iface->has_modifier_value("dllname"))
	{
		char n[256];

		get_c_name(iface,n,sizeof(n));
		strncat(n,"_Class_Source",sizeof(n)-1);

		dump_nest(out,nest);
		out_printf(out,"#if defined(%s) || defined(%s)\n",idl_conditional,n);

		dump_nest(out,nest+1);
		out_printf(out,"#ifdef SOMDLLEXPORT\n");

		dump_nest(out,2+nest);
		out_printf(out,"SOMDLLEXPORT\n");

		dump_nest(out,nest+1);
		out_printf(out,"#endif /* SOMDLLEXPORT */\n");

		dump_nest(out,nest);
		out_printf(out,"#else /* %s || %s */\n",idl_conditional,n);

		dump_nest(out,nest+1);
		out_printf(out,"#ifdef SOMDLLIMPORT\n");

		dump_nest(out,2+nest);
		out_printf(out,"SOMDLLIMPORT\n");

		dump_nest(out,nest+1);
		out_printf(out,"#endif /* SOMDLLIMPORT */\n");

		dump_nest(out,nest);
		out_printf(out,"#endif /* %s || %s */\n",idl_conditional,n);
	}
}


void RHBheader_emitter::generate_sizeof_va_list_param(RHBoutput *out,RHBparameter *param,int nest)
{
	char n[256];

	if (!strcmp(param->mode,"in"))
	{
		RHBtype *t=unwind_typedef(param->parameter_type);

		if (t)
		{
			RHBtype *bt=get_va_type(t,1);

			if (bt)
			{
				if (bt->is_base_type())
				{
					if (bt->is_base_type()->_floating)
					{
						/* for this case, a float needs to stay
							as a float, and not get promoted */
						bt=t;
					}
				}

				if (bt->is_array() ||
					bt->is_union() ||
					bt->is_struct() ||
					bt->is_pointer() ||
					bt->is_interface() ||
					bt->is_sequence() ||
					bt->is_any() ||
					parameter_by_reference(param,NULL))
				{
					dump_nest(out,nest+1);
					out_printf(out,"+sizeof(void *)\n");
				}
				else
				{
					if (bt->is_enum())
					{
						get_c_name(bt,n,sizeof(n));
					}
					else
					{
						strncpy(n,bt->id,sizeof(n));

						while (bt->is_qualified())
						{
							bt=bt->is_qualified()->base_type;
							strncat(n,"_",sizeof(n)-1);
							strncat(n,bt->id,sizeof(n)-1);
						}

						if (!strcmp(n,"unsigned_long"))
						{
							strncpy(n,"unsigned long",sizeof(n));
						}
						else
						{
							if (!strcmp(n,"unsigned_short"))
							{
								strncpy(n,"unsigned short",sizeof(n));
							}
						}
					}

					dump_nest(out,nest+1);
					out_printf(out,"+sizeof(%s)\n",n);
				}
			}
		}
	}
	else
	{
		dump_nest(out,nest+1);
		out_printf(out,"+sizeof(void *)\n");
	}
}

boolean RHBheader_emitter::is_in_floating(RHBparameter *param)
{
	if (!strcmp(param->mode,"in"))
	{
		RHBtype *t=unwind_typedef(param->parameter_type);
		if (t)
		{
			RHBbase_type *bt=t->is_base_type();

			if (bt)
			{
				if (bt->_floating)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

static struct
{
	const char *name;
	const char *value;
} type_table[]={
	{"boolean","SOMRdRetbyte"},
	{"char","SOMRdRetbyte"},
	{"octet","SOMRdRetbyte"},
	{"short","SOMRdRethalf"},
	{"unsigned_short","SOMRdRethalf"},
	{"long","SOMRdRetsimple"},
	{"unsigned_long","SOMRdRetsimple"},
	{"unsigned_short","SOMRdRethalf"},
	{"float","SOMRdRetfloat"},
	{"double","SOMRdRetdouble"},
	{"long_double","SOMRdRetlongdouble"},
	{"void","SOMRdRetsimple"},
	{"TypeCode","SOMRdRetsimple"},
	{"Principal","SOMRdRetsimple"}
};

struct type_size
{
	int size;
	const char *type;
};

static struct type_size ret32bit[]=
{
	{1,"SOMRdRetbyte"},
	{2,"SOMRdRethalf"},
	{4,"SOMRdRetsimple"}
};

static struct type_size ret64bit[]=
{
	{1,"SOMRdRetbyte"},
	{2,"SOMRdRethalf"},
	{4,"SOMRdRetsimple"},
	{8,"SOMRdRetsmallaggregate"}
};

void RHBheader_emitter::type_size_expand(RHBoutput *out,int nest,const char *t,struct type_size *a,size_t n)
{
	size_t k=0;

	while (k < n)
	{
		dump_nest(out,nest);

		out_printf(out,"((sizeof(%s)==%d) ? %s : \n",t,a->size,a->type);

		a++;
		k++;
	}

	dump_nest(out,nest);
	out_printf(out,"SOMRdRetaggregate");
	while (k--)
	{
		out_printf(out,")");
	}
	out_printf(out,"\n");
}

void RHBheader_emitter::generate_rdsizetype(RHBoutput *out,const char *t,int nest)
{
	out_printf(out,"#ifdef SOMRdRetsmallaggregate\n");

	type_size_expand(out,nest+1,t,ret64bit,sizeof(ret64bit)/sizeof(ret64bit[0]));

	dump_nest(out,nest);
	out_printf(out,"#else\n");

	type_size_expand(out,nest+1,t,ret32bit,sizeof(ret32bit)/sizeof(ret32bit[0]));

	dump_nest(out,nest);
	out_printf(out,"#endif\n");
	dump_nest(out,nest);
}


void RHBheader_emitter::generate_somMethodInfo(
			RHBoutput *out,
			RHBinterface *iface,
			RHBoperation *op,
			int nest,
			int auto_stub)
{
	char n[256];

	if (auto_stub)
	{
		int has_floats=0;
		RHBtype *ret_type=unwind_typedef(op->return_type);
		RHBelement *elc=op->children();

		/* only do float map if can auto_stub */

		while (elc)
		{
			RHBparameter *param=elc->is_parameter();
			if (param)
			{
				if (is_in_floating(param))
				{
					has_floats++;
				}
			}
			elc=elc->next();
		}

		get_c_name(op,n,sizeof(n));

		if (has_floats)
		{
			int k=0;
			RHBelement *el=op->children();

			{
				char n5[256];
				snprintf(n5,sizeof(n5),"somFloatMap_%s",n);
				undoublebar(out,n5);
			}

			out_printf(out,"static somFloatMap somFloatMap_%s={\n",n); 

			while (el)
			{
				RHBparameter *param=el->is_parameter();
				if (param)
				{
					if (is_in_floating(param))
					{
						RHBelement *z=op->children(); /* was param->children !! */
						RHBtype *t=unwind_typedef(param->parameter_type);

						dump_nest(out,nest+2);
						out_printf(out,"sizeof(SOMObject *)\n");

						if (operation_needs_environment(iface,op))
						{
							dump_nest(out,nest+2);
							out_printf(out,"+sizeof(Environment *)\n");
						}

						if (operation_needs_context(iface,op))
						{
							dump_nest(out,nest+2);
							out_printf(out,"+sizeof(Context SOMSTAR)\n");
						}

						while (z && (z!=param))
						{
							RHBparameter *p=z->is_parameter();
							if (p)
							{
								generate_sizeof_va_list_param(out,p,nest+1);
							}
							z=z->next();
						}

						if (!strcmp(t->id,"float"))
						{
							dump_nest(out,nest+2);
							out_printf(out,"+SOMFMSingle\n");
						}

						dump_nest(out,nest+1);

						k++;

						if (k < has_floats)
						{
							out_printf(out,",\n");
						}
						else
						{
							out_printf(out,"+SOMFMLast");
						}
					}
				}

				el=el->next();
			}

			dump_nest(out,nest+1);
			out_printf(out,"};\n");
		}

		{
			char n2[256];
			strncpy(n2,"somMethodInfo_",sizeof(n2));
			strncat(n2,n,sizeof(n2)-1);
			undoublebar(out,n2);
		}

		out_printf(out,"static somMethodInfo somMethodInfo_%s={\n",n); 

		dump_nest(out,nest+1);

		if (ret_type)
		{
			if (ret_type->is_base_type() || ret_type->is_qualified())
			{
				char buf[256];
				size_t i=0;
				int k=sizeof(type_table)/sizeof(type_table[0]);
				strncpy(buf,ret_type->id,sizeof(buf));

				while (ret_type->is_qualified())
				{
					ret_type=ret_type->is_qualified()->base_type;
					strncat(buf,"_",sizeof(buf)-1);
					strncat(buf,ret_type->id,sizeof(buf)-1);
				}

				while (k--)
				{
					if (!strcmp(type_table[k].name,buf))
					{
						i=snprintf(buf,sizeof(buf),"%s",type_table[k].value);

						break;
					}
				}

				if (!i)
				{
					if (!strcmp(buf,"any"))
					{
						generate_rdsizetype(out,buf,nest+1);
					}
					else
					{
						bomb("internal error - was expecting any");
			
						i=strlen(buf);
						i+=snprintf(buf+i,sizeof(buf)-i," compiler error %s:%d\n",__FILE__,__LINE__);
					}
				}

				if (i) 
				{
					out_printf(out,"%s",buf);
				}
			}
			else
			{
				if (ret_type->is_sequence())
				{
					generate_rdsizetype(out,"GENERIC_SEQUENCE",nest+1);
				}
				else
				{
					if (ret_type->is_union() ||
						ret_type->is_struct())
					{
						char buf[256];
						get_c_name(ret_type,buf,sizeof(buf));
						generate_rdsizetype(out,buf,nest);
					}
					else
					{
						if (ret_type->is_pointer() ||
							ret_type->is_interface() ||
							ret_type->is_array() ||
							ret_type->is_string())
						{
							out_printf(out,"SOMRdRetsimple");
						}
						else
						{
							if (ret_type->is_enum())
							{
								out_printf(out,"SOMRdRetsimple");
							}
							else
							{
								char buf[256];
								get_c_name(ret_type,buf,sizeof(buf));

								bomb("internal compiler error");
	
								int i=strlen(buf);
								i+=snprintf(buf+i,sizeof(buf)-i," compiler error %s:%d\n",__FILE__,__LINE__);

								out_printf(out,"SOMRdRetsimple");
							}
						}
					}
				}
			}
		}
		else
		{
			out_printf(out,"SOMRdRetsimple");
		}

		if (has_floats)
		{
			out_printf(out,"|SOMRdFloatArgs,\n");
		}
		else
		{
			out_printf(out,"|SOMRdNoFloatArgs,\n");
		}

		dump_nest(out,nest+1);
		out_printf(out,"sizeof(SOMObject SOMSTAR)\n");

		if (operation_needs_environment(iface,op))
		{
			dump_nest(out,nest+1);
			out_printf(out,"+sizeof(Environment *)\n");
		}

		if (operation_needs_context(iface,op))
		{
			dump_nest(out,nest+1);
			out_printf(out,"+sizeof(Context SOMSTAR)\n");
		}

		elc=op->children();

		while (elc)
		{
			RHBparameter *param=elc->is_parameter();
			if (param)
			{
				generate_sizeof_va_list_param(out,param,nest);
			}

			elc=elc->next();
		}

		dump_nest(out,nest+1);

		if (has_floats)
		{
			get_c_name(op,n,sizeof(n));
			out_printf(out,",&somFloatMap_%s};\n",n);
		}
		else
		{
			out_printf(out,",NULL};\n");
		}
	}

	get_c_name(op,n,sizeof(n));

	{
		char n2[256];
		strncpy(n2,"somApRdInfo_",sizeof(n2));
		strncat(n2,n,sizeof(n2)-1);
		undoublebar(out,n2);
	}

	out_printf(out,"static somApRdInfo somApRdInfo_%s={\n",n);

	dump_nest(out,nest+1);
	out_printf(out,"(somMethodPtr)somRD_%s,\n",n);

	if (op_is_varg(op)) 
	{
		dump_nest(out,nest+1);
		out_printf(out,"NULL /* has va_list */,\n");
	}
	else
	{
		dump_nest(out,nest+1);
		out_printf(out,"(somMethodPtr)somAP_%s,\n",n);
	}

	dump_nest(out,nest+1);

	if (auto_stub)
	{
		out_printf(out,"&somMethodInfo_%s};\n",n);
	}
	else
	{
		out_printf(out,"NULL};\n");
	}
}

void RHBheader_emitter::call_parent_constructors(RHBoutput *out,RHBinterface *iface,int nest,const char *args)
{
	if (!args) args="";

	if (iface->_parents.length())
	{
		size_t k=0;
		while (k < iface->_parents.length())
		{
			char n[256];
			dump_nest(out,nest+2);

			get_c_name(iface->_parents.get(k),n,sizeof(n));

			out_printf(out,"%c %s(%s)\n",
				(k) ? ',' : ':',
				n,
				args);

			k++;
		}
	}
}

void RHBheader_emitter::generate_interface(RHBoutput *out,RHBinterface *iface,int nest)
{
	char n[256];
/*	RHBelement *el;*/

	/* generate as C++ first ... so we know what we're doing */

	/*
	if (!internal)
	{
		generate_versions(out,iface,"MajorVersion","majorversion",nest);
		generate_versions(out,iface,"MinorVersion","minorversion",nest);
	}
	*/

	get_c_name(iface,n,sizeof(n));

	if (cplusplus)
	{
		if (!internal)
		{
			out_printf(out,"\n");

			if (!iface->_parents.length())
			{
				out_printf(out,"#include <somcls.api>\n");
			}

			dump_nest(out,nest);
			out_printf(out,"class %s",n);

			if (iface->_parents.length())
			{
				long m=0;
				long l=iface->_parents.length();
				out_printf(out," : ");

				while (m < l)
				{
					RHBelement *el=iface->_parents.get(m);

					get_c_name(el,n,sizeof(n));

					out_printf(out,"public %s",n);

					m++;

					if (m < l)
					{
						out_printf(out,", ");
					}
				}
			}

			out_printf(out,"\n");

			dump_nest(out,nest);

			out_printf(out,"{\n");

			get_c_name(iface,n,sizeof(n));

			{
				/* this constructor is designed to allow MI C++
				   classes to properly construct themselves */

				const char *mtab=iface->_parents.length() ? "mtab" : "/* mtab */ ";

				/* generate protected parent constructor */
				dump_nest(out,nest);
				out_printf(out,"protected:\n");

				dump_nest(out,nest+1);
				out_printf(out,"%s(somMethodTab & %s)\n",n,mtab);

				call_parent_constructors(out,iface,nest,"mtab");

				dump_nest(out,nest+1);
				out_printf(out,"{}\n");
			}

		/*	if (iface->_parents.length()==0)
			{
				dump_nest(out,1);
				i=sprintf(buf,"somMethodTabPtr mtab;\n");
				out->write(buf,i);
			}*/

			dump_nest(out,nest);
			out_printf(out,"public:\n");

			RHBelement *el=iface->children();

			while (el)
			{
				RHBoperation *op=el->is_operation();

				if (op)
				{
					out_printf(out,"\n");
					generate_inline_operation(out,iface,op,nest);
				}

				el=el->next();
			}

	/*		if (iface->_parents.length()==0)*/
			{
				out_printf(out,"\n");
				dump_nest(out,nest+1);
				out_printf(out,"inline void operator delete(void *x)\n");
				dump_nest(out,nest+2);
				out_printf(out,"{ ((SOMObject *)x)->somFree(); };\n\n");
			}

#if 0
			if (iface->_parents.length())
#endif
			{
				char args[256];

				snprintf(args,sizeof(args),"*(%sCClassData.parentMtab->mtab)",n);

				{
					/* generate new operation */
					dump_nest(out,nest+1);
					out_printf(out,"inline void * operator new(size_t /* len */)\n");

					get_c_name(iface,n,sizeof(n));
					dump_nest(out,nest+1);
					out_printf(out,"{ if (!%sClassData.classObject)\n",n);

					dump_nest(out,nest+2);
					out_printf(out,"{ %sNewClass(%s_MajorVersion,%s_MinorVersion); }\n",n,n,n);

					dump_nest(out,nest+2);
					out_printf(out,"return (void *)SOM_Resolve((SOMObject *)(void *)%sClassData.classObject,\n",n);

					dump_nest(out,nest+3);
					out_printf(out,"SOMClass,somNewNoInit)((SOMClass *)(void *)%sClassData.classObject);\n",n);

					dump_nest(out,nest+1);
					out_printf(out,"};\n");
				}

				{
					/* generate default constructor */
					dump_nest(out,nest+1);
					out_printf(out,"%s()\n",n);

					call_parent_constructors(out,iface,nest,args);

					dump_nest(out,nest+1);
					out_printf(out,"{ if ((*(void **)(void *)this)==%sCClassData.parentMtab->mtab)\n",n);

					dump_nest(out,nest+2);
					out_printf(out,"((SOMObject *)(void *)this)->somDefaultInit(0); }\n");
				}

				{
					/* generate copy constructor operation */
					dump_nest(out,nest+1);
					out_printf(out,"%s(%s *fromObj)\n",n,n);

					call_parent_constructors(out,iface,nest,args);

					dump_nest(out,nest+1);
					out_printf(out,"{ if ((*(void **)(void *)this)==%sCClassData.parentMtab->mtab)\n",n);
					dump_nest(out,nest+2);
					out_printf(out,"((SOMObject *)(void *)this)->somDefaultCopyInit(0,(SOMObject*)(void*)fromObj); }\n");
				}
			}
#if 0
			else
			{
				dump_nest(out,nest+1);
				i=sprintf(buf,"void * operator new(size_t /* len */);\n\n");
				out->write(buf,i);
			}
#endif

			boolean done_somNew=0;

			if (is_multiple_inherited(iface))
			{
				if (iface->_parents.length())
				{
					RHBelement_sequence seq;

					determine_inherited(iface,iface,&seq);

					unsigned long i=0;

					while (i < seq.length())
					{
						RHBoperation *op;

						op=seq.get(i)->is_operation();

						if (iface->_metaclass_of.length())
						{
							char n2[256];
							get_ir_name(op,n2,sizeof(n2));
							if (!strcmp(n2,"::SOMClass::somNew"))
							{
								done_somNew=1;
								generate_inline_somNew_operation(out,iface,op,nest);

								op=0;
							}
						}

						if (op)
						{
							generate_inline_operation(out,iface,op,nest);
						}

						i++;
					}
				}
			}

			if (!done_somNew)
			{
				if (iface->_metaclass_of.length())
				{
					/* this probably means that 'somcls.idl' was not included */

					done_somNew=1;
					generate_inline_somNew_operation(out,iface,0,nest);
				}
			}

			dump_nest(out,nest);
			out_printf(out,"};\n");
		}
	}

	if (internal)
	{
		generate_class_guard(out,nest,iface,1);
	}

	{
		RHBelement *el=iface->children();

		while (el)
		{
			RHBoperation *op=el->is_operation();

			if (op)
			{
				if (is_operation_procedure(op))
				{
					if (!internal)
					{
						if (!cplusplus)
						{
							generate_macro_operation(out,iface,op,nest);
						}
					}
					else
					{
						generate_operation_proto(out,iface,op,nest);
					}
				}
				else
				{
					if (!internal)
					{
						if (!cplusplus)
						{
							generate_macro_operation(out,iface,op,nest);
						}
					}
					else
					{
						int do_auto_stubs=op->can_auto_stub();

						generate_somId(out,nest,op,"somMN","somId",0,0);
						generate_somId(out,nest,op,"somDS","somDI",0 /*1*/,1);

						generate_operation_proto(out,iface,op,nest);

						if (do_auto_stubs)
						{
							out_printf(out,"#ifdef SOM_METHOD_STUBS\n");
							get_c_name(op,n,sizeof(n));
							out_printf(out,"\t#define somRD_%s   0\n",n);
							out_printf(out,"\t#define somAP_%s   0\n",n);
							out_printf(out,"#else /* SOM_METHOD_STUBS */\n");
						}

						generate_redispatch_stub(out,iface,op,nest);

						if (op) 
						{
							generate_apply_stub(out,iface,op,nest);
						}


						if (do_auto_stubs)
						{
							out_printf(out,"#endif /* SOM_METHOD_STUBS */\n");
						}


						{
							out_printf(out,"#ifdef SOM_METHOD_STUBS\n");
							generate_somMethodInfo(out,iface,op,nest,do_auto_stubs);
							out_printf(out,"#endif /* SOM_METHOD_STUBS */\n");
						}
					}
				}
			}

			el=el->next();
		}
	}

	/* this should only be done if internal... */

	if (internal)
	{
		dump_nest(out,nest);
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"#ifndef %sMethodDebug\n",n);
		dump_nest(out,nest+1);
		out_printf(out,"#define %sMethodDebug(x,y)\n",n);
		dump_nest(out,nest);
		out_printf(out,"#endif /* %sMethodDebug */\n",n);

		if (iface->instanceData->children())
		{
			dump_nest(out,nest); 
			out_printf(out,"#ifdef SOM_DATA_THUNKS\n");

			nest++;

			{
				dump_nest(out,nest);
				get_c_name(iface,n,sizeof(n));
				out_printf(out,"#define %sGetData(somSelf) \\\n",n);
				dump_nest(out,nest+1);

				if (cplusplus)
				{
					out_printf(out,"(((%sData * (SOMLINK *)(%s *))",n,n);
				}
				else
				{
					out_printf(out,"(((%sData * (SOMLINK *)(%s SOMSTAR))",n,n);
				}

				get_c_name(iface,n,sizeof(n));
				out_printf(out,"(somMethodPtr)%sCClassData.instanceDataToken)(somSelf))\n",n);
			}

			nest--;
			dump_nest(out,nest); 
			out_printf(out,"#else /* SOM_DATA_THUNKS */\n");

			nest++;
			{
				dump_nest(out,nest);
				get_c_name(iface,n,sizeof(n));
				out_printf(out,"#define %sGetData(somSelf) \\\n",n);
				dump_nest(out,nest+1);
				out_printf(out,"((%sData *) \\\n",n);
				dump_nest(out,nest+2);
				out_printf(out,"somDataResolve(");
				if (cplusplus)
				{
					out_printf(out,"(SOMObject *)(void *)");
				}

				get_c_name(iface,n,sizeof(n));
				out_printf(out,"somSelf,%sCClassData.instanceDataToken))\n",n);
			}
			nest--;
			dump_nest(out,nest); 
			out_printf(out,"#endif /* SOM_DATA_THUNKS */\n");
		}
		else
		{
			dump_nest(out,nest);
			get_c_name(iface,n,sizeof(n));
			out_printf(out,"#define %sGetData(somSelf)  ",n);
			out_printf(out,"((%sData *)0)  /* class has no attributes */\n",n);
		}
	}

	if (internal)
	{
		RHBelement_sequence seq;
		unsigned long i=0;

		determine_overrides(iface,iface,&seq);

		while (i < seq.length())
		{
			RHBoperation *op=seq.get(i)->is_operation();

			if (op)
			{
				if (0==strcmp(op->id,"somDestruct"))
				{
					out_printf(out,"/* %s::%s (%s:%d)*/\n",
							iface->id,op->id,__FILE__,__LINE__);
				}
				else
				{
					if (0==strcmp(op->id,"somDefaultInit"))
					{
						out_printf(out,"/* %s::%s (%s:%d)*/\n",
								iface->id,op->id,__FILE__,__LINE__);
					}
				}

				if (op)
				{
					generate_somId(out,nest,op,"somDS","somDI",0/*1*/,1);  /* used for override registration */
					generate_operation_proto(out,iface,op,nest);
				}
			}

			i++;
		}

		generate_new_class(out,iface,&seq,nest);
	}

	if (internal)
	{
		unsigned long i=0;

		while (i < iface->_parents.length())
		{
			RHBinterface *par=iface->_parents.get(i)->is_interface();

			if (par)
			{
				RHBelement_sequence seq;
				unsigned long j=0;

				par->list_all_operations(&seq);

				while (j < seq.length())
				{
					generate_parent_macro(out,iface,par,seq.get(j)->is_operation(),nest);

					j++;
				}
			}

			i++;
		}

		generate_dts_macros(out,iface);
	}

	if (cplusplus)
    {
        if (internal)
        {
            RHBelement_sequence seq;
            unsigned long j=0;
            
            iface->list_all_operations(&seq);
            
			dump_nest(out,nest);
            out_printf(out,"#ifdef METHOD_MACROS\n");
            
            while (j < seq.length())
            {
                RHBelement *op=seq.get(j++);
                const char *p=op->id;
                dump_nest(out,nest+1);
                out_printf(out,"#ifndef _%s\n",p);
                
                dump_nest(out,nest+2);
                out_printf(out,"#define _%s somSelf->%s\n",p,p);
                dump_nest(out,nest+1);
                out_printf(out,"#endif /* _%s */\n",p);
            }
			dump_nest(out,nest);
            out_printf(out,"#endif /*METHOD_MACROS */\n");
        }
    }
    
	if (internal)
	{
		generate_class_guard(out,nest,iface,0);
	}
    
    if (!cplusplus)
	{
		if (!internal)
		{
			if (iface->_parents.length())
			{
				RHBelement_sequence dups;

				out_printf(out,"\n#ifndef SOM_DONT_USE_INHERITED_MACROS\n");

				generate_inherited_macros(out,&dups,iface,iface);

				out_printf(out,"#endif /* SOM_DONT_USE_INHERITED_MACROS */\n\n");
			}
		}
	}
}

void RHBheader_emitter::generate_new_class(RHBoutput *out,RHBinterface *iface,RHBelement_sequence *overrides,int nest)
{
	int num_statics=0;
	int num_procedures=0;
	int num_va=0;
	char n[256];
	long mask;
	RHBelement_sequence init_entries;
	int init_op_count=get_init_operations(iface,&init_entries);

	generate_somId(out,nest,iface,"somCN",0,0,1);
	get_meta_class(iface,n);

	if (n[0])
	{
		generate_somId(out,nest,iface->find_named_element(0,n,sizeof(n)),"somMCN",0,0,1);
	}

	{
		unsigned long k=0;
		
		while (k < iface->_parents.length())
		{
			char buf[256];
			char n2[256];

			RHBelement *el=iface->_parents.get(k);

			get_c_name(iface,n2,sizeof(n2));
			strncpy(buf,"somPCN_",sizeof(buf));
			strncat(buf,n2,sizeof(buf)-1);

			generate_somId(out,nest,el,buf,0,0,1);

			k++;
		}
	}

	if (iface->_parents.length())
	{
		unsigned long k=0;

		get_c_name(iface,n,sizeof(n));

		out_printf(out,"static const somConstId somPC%s[%d]={\n",
					n,
					(int)iface->_parents.length());

		while (k < iface->_parents.length())
		{
			char m[256];

			get_c_name(iface,m,sizeof(m));
			RHBelement *el=iface->_parents.get(k);

			get_c_name(el,n,sizeof(n));

			dump_nest(out,1);
			out_printf(out,"&somPCN_%s_%s",m,n);

			k++;

			if (k!=iface->_parents.length())
			{
				out_printf(out,",\n");
			}
		}

		out_printf(out,"};\n\n");
	}

	num_procedures=0;
	num_statics=0;

	{
		RHBelement *el=iface->children();

		while (el)
		{
			if (el->is_operation())
			{
				if (is_operation_procedure(el->is_operation()))
				{
					num_procedures++;
				}
				else
				{
					num_statics++;
				}
			}

			el=el->next();
		}
	}

	get_ir_name(iface,n,sizeof(n));

	out_printf(out,"/* class %s introduces %d methods, %d procedures and overrides %d */\n",
				n,num_statics,num_procedures,(int)overrides->length());

	/* generate class data structure */

	out_printf(out,"/* should have %sClassData defined here */\n",n);

	generate_internal_ctrl_macros(out,iface);
	enum_attributes(out,iface);

	/* static somStaticMethod_t <<classname>>SM */
	/* static somOverrideMethod_t <<classname>>OM */
	/* static somStaticClassInfo <<className>>SCI  */

	get_c_name(iface,n,sizeof(n));

	if (num_statics)
	{
		int x=num_statics;
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"static somStaticMethod_t %sSM[%d]={\n",
				n,(int)num_statics);

		RHBelement *el=iface->children();

		while (el)
		{	
			RHBoperation *op=el->is_operation();

			if (op)
			{
				if (!is_operation_procedure(op))
				{
					dump_nest(out,2);

					out_printf(out,"{\n");

					/* somMToken *classData */
					dump_nest(out,3);
					get_c_name(iface,n,sizeof(n));
					out_printf(out,"&%sClassData.%s,\n",n,op->id);

					/* somId *methodId */
					dump_nest(out,3);
					out_printf(out,"&somId_%s,\n",op->id);

					/* somId *methodDescriptor */

					dump_nest(out,3);
					get_c_name(op,n,sizeof(n));
					out_printf(out,"&somDI_%s,\n",n);

					/* somMethodPtr method */

					dump_nest(out,3);
					get_function_prefix(iface,n,sizeof(n));
					out_printf(out,"(somMethodPtr)%s%s,\n",n,op->id);

					
					out_printf(out,"#ifdef SOM_METHOD_STUBS\n");

					{
								/* indicate that we have the ABI info 
									technically you can't cast a data pointer
									to a function pointer, so we go via a size_t...
									*/
								dump_nest(out,3);
								out_printf(out,"(somMethodPtr)-1L,\n");

								dump_nest(out,3);
								get_c_name(op,n,sizeof(n));
								out_printf(out,"(somMethodPtr)(size_t)&somApRdInfo_%s\n",n);
					}

					out_printf(out,"#else /* SOM_METHOD_STUBS */\n");

					{
								/* somMethodPtr redispatch */

								dump_nest(out,3);
								get_c_name(op,n,sizeof(n));
								out_printf(out,"(somMethodPtr)somRD_%s,\n",n);

								/* somMethodPtr apply */

								dump_nest(out,3);
								if (op_is_varg(op))
								{
									out_printf(out,"(somMethodPtr)NULL\n");
								}
								else
								{
									get_c_name(op,n,sizeof(n));
									out_printf(out,"(somMethodPtr)somAP_%s\n",n);
								}
					}

					out_printf(out,"#endif /* SOM_METHOD_STUBS */\n");

					dump_nest(out,2);

					out_printf(out,"}");

					x--;

					if (x)
					{
						out_printf(out,",\n");
					}
					else
					{
						out_printf(out,"\n");
					}
				}
			}
			el=el->next();
		}

		dump_nest(out,1);
		out_printf(out,"};\n");
	}

	if (overrides->length())
	{
		unsigned long k=0;

		while (k < overrides->length())
		{
			RHBoperation *op=overrides->get(k)->is_operation();

			k++;
	/* this is not needed for overridden methods */
	/*		generate_somId(out,nest,op,"somMN","somId",0,0); */

			generate_somId(out,nest,op,"somDS","somDI",0 /*1*/,1);
		}

		get_c_name(iface,n,sizeof(n));
		out_printf(out,"static somOverrideMethod_t %sOM[%d]={\n",
				n,(int)overrides->length());

		k=0;

		while (k < overrides->length())
		{
			RHBoperation *op=overrides->get(k)->is_operation();

			k++;

			dump_nest(out,2);
			out_printf(out,"{\n");

			/* somId *methodId */

			dump_nest(out,3);

			char ii[256];
			get_c_name(op,ii,sizeof(ii));
			out_printf(out,"&somDI_%s,\n",ii /*op->id*/);

			/* somMethodPtr method */

			dump_nest(out,3);
			get_function_prefix(iface,n,sizeof(n));
			out_printf(out,"(somMethodPtr)%s%s\n",n,op->id);

			dump_nest(out,2);
			out_printf(out,"}");

			if (k==overrides->length())
			{
				out_printf(out,"\n");
			}
			else
			{
				out_printf(out,",\n");
			}
		}

		dump_nest(out,1);
		out_printf(out,"};\n");
	}
	/* somProcMethods_t */

	if (num_procedures)
	{
		get_c_name(iface,n,sizeof(n));

		out_printf(out,"static somProcMethods_t %sPM[%d]={\n",
					n,(int)num_procedures);

		RHBelement *el=iface->children();

		int x=num_procedures;

		while (el)
		{
			RHBoperation *op=el->is_operation();

			if (op)
			{
				if (is_operation_procedure(op))
				{
					dump_nest(out,1);

					get_c_name(iface,n,sizeof(n));

					out_printf(out,"{(somMethodPtr*)(void *)&%sClassData.%s,\n",n,op->id);

					dump_nest(out,2);

					get_function_prefix(iface,n,sizeof(n));
					out_printf(out,"(somMethodPtr)%s%s}\n",n,op->id);

					x--;

					if (x)
					{
						dump_nest(out,1);
						out_printf(out,",\n");
					}
				}
			}

			el=el->next();
		}

		dump_nest(out,1);
		out_printf(out,"};\n");
	}

	if (internal)
	{
		RHBelement_sequence vaops;

		num_va=get_va_operations(iface,&vaops);

		if (num_va)
		{
			unsigned long k=0;

			while (k < vaops.length())
			{
				RHBoperation *op=vaops.get(k)->is_operation();
				generate_va_stub(out,iface,op,nest,3,"somVA");
				k++;
			}

			k=0;

			get_c_name(iface,n,sizeof(n));

			out_printf(out,"static somVarargsFuncs_t %sVF[%d]={\n",n,num_va);

			while (k < vaops.length())
			{
				RHBoperation *op=vaops.get(k)->is_operation();

				out_printf(out,"{&%sCClassData.%s,(somMethodPtr)somVA_%s_%s}",
							n,op->id,n,op->id);

				if (++k < vaops.length())
				{
					out_printf(out,",\n");
				}
			}

			out_printf(out,"};\n");
		}

		if (useAlignmentHelper())
		{
			if (iface->instanceData->children())
			{
				get_c_name(iface,n,sizeof(n));
				out_printf(out,"struct _somC_%s_dataAlignment { octet _o; %sData _d; };\n",n,n);
			}
		}
	}

	if (get_cif(iface,n,sizeof(n)))
	{
		if (cplusplus)
		{
			out_printf(out,"static void SOMLINK %s(SOMClass *);\n",n);
		}
		else
		{
			out_printf(out,"static void SOMLINK %s(SOMClass SOMSTAR);\n",n);
		}
	}

	if (overrides && !is_kernel_class(iface))
	{
		long k1=overrides->length();

		while (k1--)
		{
			RHBoperation *op=overrides->get(k1)->is_operation();

			if (op)
			{
				long k2=iface->_parents.length();

				while (k2--)
				{
					RHBinterface *par=iface->_parents.get(k2)->is_interface();

					if (par)
					{
						generate_parent_resolved(out,iface,par,op,1);
					}
				}
			}
		}
	}

	get_c_name(iface,n,sizeof(n));

	if (cplusplus)
	{
		extern_c_proto_t extern_c_proto(this,out,nest);
		out_printf(out,"static void SOMLINK _somC_%s_classInit(SOMClass *);\n",n);
	}
	else
	{
		out_printf(out,"static void SOMLINK _somC_%s_classInit(SOMClass SOMSTAR);\n",n);
	}

	if (init_op_count)
	{
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"static somId somINIT%s[%d]={\n",n,init_op_count);

		int k=0;
		while (k < init_op_count)
		{
			RHBelement *ip=init_entries.get(k);

			out_printf(out,"(somId)&somMN_%s",ip->id);

			k++;

			if (k < init_op_count) 
			{
				out_printf(out,",\n");
			}
		}

		out_printf(out,"};\n");
	}

	get_c_name(iface,n,sizeof(n));
	out_printf(out,"static somStaticClassInfo %sSCI={\n",n);

	/* need to generate ... type 2 information only!!! */

	/* unsigned long layoutVersion */

	out_printf(out,"#ifdef SOMSCIVERSION\n");
	dump_nest(out,2);
	out_printf(out,"%d,  /* layoutVersion */\n",3);
	out_printf(out,"#else /* SOMSCIVERSION */\n");
	dump_nest(out,2);
	out_printf(out,"%d,  /* layoutVersion */\n",2);
	out_printf(out,"#endif /* SOMSCIVERSION */\n");

	/* unsigned long numStaticMethod */

	dump_nest(out,2);
	out_printf(out,"%d, /* numStaticMethod */\n",num_statics);

	/* unsigned long numStaticOverrides */

	dump_nest(out,2);
	out_printf(out,"%d, /* numStaticOverrides */\n",(int)overrides->length());

	/* unsigned long numNonInternalData */

	dump_nest(out,2);
	out_printf(out,"%d, /* numNonInternalData */\n",0);

	/* unsigned long numProcMethods */

	dump_nest(out,2);
	out_printf(out,"%d, /* numProcMethods */\n",(int)num_procedures);

	/* unsigned long numVarargsFuncs */

	dump_nest(out,2);
	out_printf(out,"%d, /* numVarargsFuncs */\n",num_va);

	/* unsigned long majorVersion */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"%s_MajorVersion,\n",n);

	/* unsigned long minorVersion */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"%s_MinorVersion,\n",n);

	/* unsigned long instanceDataSize */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	if (iface->instanceData->children())
	{
		out_printf(out,"sizeof(%sData), /* instanceDataSize */\n",n);
	}
	else
	{
		out_printf(out,"0, /* no instanceDataSize */\n");
	}

	/* unsigned long maxMethods */

	dump_nest(out,2);
	out_printf(out,"%d, /* maxMethods */\n",num_statics);

	/* unsigned long numParents */

	dump_nest(out,2);
	out_printf(out,"%d, /* numParents */\n",
		(int)iface->_parents.length());

	/* somId classId */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"&somCN_%s,\n",n);

	/* somId explicitMetaId */

	dump_nest(out,2);
	get_meta_class(iface,n);

	if (n[0])
	{
		RHBelement *el2=iface->find_named_element(0,n,sizeof(n));

		get_c_name(el2,n,sizeof(n));

		out_printf(out,"&somMCN_%s,\n",n);
	}
	else
	{
		out_printf(out,"0,\n");
	}

	/* long implicitParentMeta */

	dump_nest(out,2);
	out_printf(out,"0, /* implicitParentMeta */\n");

	/* somId *parents */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));

	if (iface->_parents.length())
	{
		out_printf(out,"somPC%s, /* parents */\n",n);
	}
	else
	{
		out_printf(out,"0, /* no parents */\n");
	}

	/* somClassDataStructure *cds */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"(somClassDataStructure *)(void *)&%sClassData,\n",n);

	/* somCClassDataStructure *ccds */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"(somCClassDataStructure *)(void *)&%sCClassData,\n",n);

	/* somStaticMethod_t *smt */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));

	if (num_statics)
	{
		out_printf(out,"%sSM,\n",n);
	}
	else
	{
		out_printf(out,"0,  /* no new methods */\n");
	}

	/* somOverrideMethod_t *omt */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));

	if (overrides->length())
	{
		out_printf(out,"%sOM,\n",n);
	}
	else
	{
		out_printf(out,"0,\n");
	}

	/* char *nitReferenceBase */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"0, /* nitReferenceBase */\n");

	/* somNonInternalData_t *nit */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	out_printf(out,"0, /* nit */\n");

	/* somProcMethodFuncs_t *pmt */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));

	if (num_procedures)
	{
		out_printf(out,"%sPM, /* %d procedures */\n",n,(int)num_procedures);
	}
	else
	{
		out_printf(out,"0, /* pmt no procedures*/\n");
	}

	/* somVarargsFuncs_t *vft */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));
	if (num_va)
	{
		out_printf(out,"%sVF, /* vft */\n",n);
	}
	else
	{
		out_printf(out,"0, /* vft */\n");
	}

	/* somTP_somClassInitFunc *cif */

	dump_nest(out,2);

#if 0
	if (get_cif(iface,n))
	{
		out_printf(out,"%s, /* cif */\n",n);
	}
	else
	{
		out_printf(out,"0, /* cif */\n");
	}
#else
	out_printf(out,"_somC_%s_classInit,\n",n);
#endif

	/* type 2 begin */

	/* long dataAlignment */

	dump_nest(out,2);
	get_c_name(iface,n,sizeof(n));

	if (iface->instanceData->children())
	{
		if (useAlignmentHelper())
		{
			out_printf(out,"(int)(size_t)&(((struct _somC_%s_dataAlignment *)0)->_d)\n",
				n);
		}
		else
		{
			out_printf(out,"(int)(size_t)&(((struct { octet _o; %sData _d; } *)0)->_d)\n",
				n);
		}
	}
	else
	{
		out_printf(out,"0 /* no dataAlignment */\n");
	}

	{
		out_printf(out,"#ifdef SOMSCIVERSION\n");

		dump_nest(out,2); out_printf(out,",");
		/* this is the new init stuff */

		/* directInitClasses not sure what these are */

		dump_nest(out,2); out_printf(out,"-1L,(somId *)0, /* directInitClasses */\n"); 

		/* numMethods */
		dump_nest(out,2); out_printf(out,"0,(somMethods_t *)0, /* numMethods */\n"); 

		/* protected data offset */
		dump_nest(out,2); out_printf(out,"0, /* protected data offset */\n");

		dump_nest(out,2); out_printf(out,"0, /* SOMSCIVERSION */\n");

		/* inherited methods */
		dump_nest(out,2); out_printf(out,"0,(somInheritedMethod_t *)0, /* inherited methods */\n"); 

		/* numClassData entries */

		/* should be count of class data entries excluding the classObject
		*/

		int count=count_ClassData(iface);

		dump_nest(out,2); out_printf(out,"%d,(somId *)0, /* class data entries */\n",count); 

		/* numMigratedMethods entries */
		dump_nest(out,2); out_printf(out,"0,(somMigratedMethod_t *)0, /* migrated methods */\n");

		/* numInitializers entries */

		if (init_op_count)
		{
			get_c_name(iface,n,sizeof(n));
			dump_nest(out,2); out_printf(out,"%d,somINIT%s, /* initializers */\n",
						init_op_count,n);
		}
		else
		{
			dump_nest(out,2); out_printf(out,"0,(somId *)0, /* initializers */\n");
		}

		/* somDTSClass entries */
		dump_nest(out,2); out_printf(out,"0,(somDynamicSCI *)0, /* DTS */\n");

		out_printf(out,"#endif /* SOMSCIVERSION */\n"); 
	}

	dump_nest(out,1);
	out_printf(out,"};\n");

	/* now generate the <xxx>NewClass(integer4,integer4) */

	char meta[256];
	get_actual_meta_class(iface,meta,sizeof(meta));
	get_c_name(iface,n,sizeof(n));

	if (cplusplus)
	{
		out_printf(out,"%s * SOMLINK %sNewClass(\n",meta,n);
	}
	else
	{
		out_printf(out,"%s SOMSTAR SOMLINK %sNewClass(\n",meta,n);
	}

	dump_nest(out,1);
	out_printf(out,"integer4 somtmajorVersion,integer4 somtminorVersion)\n");
	out_printf(out,"{\n");

	mask=0;

	if (iface->_parents.length())
	{
		unsigned long k=0;

		while (k < iface->_parents.length())
		{
			dump_nest(out,1);

			get_c_name(iface->_parents.get(k),n,sizeof(n));

			out_printf(out,"if (!%sNewClass(\n",n,sizeof(n));

			dump_nest(out,2);
			out_printf(out,"%s_MajorVersion,\n",n,sizeof(n));

			dump_nest(out,2);
			out_printf(out,"%s_MinorVersion))\n",n,sizeof(n));

			dump_nest(out,1);
			out_printf(out,"{\n");

			dump_nest(out,2);
			out_printf(out,"return NULL;\n");

			dump_nest(out,1);
			out_printf(out,"}\n");

			mask<<=1;
			mask|=1;

			k++;
		}
	}

	n[0]=0;
	get_meta_class(iface,n);
	if (n[0])
	{
		RHBelement *el=iface->find_named_element(0,n,sizeof(n));
		if (el)
		{
			get_c_name(el,n,sizeof(n));

			dump_nest(out,1);
			out_printf(out,"if (!%sNewClass(\n",n);

			dump_nest(out,2);
			out_printf(out,"%s_MajorVersion,\n",n);

			dump_nest(out,2);
			out_printf(out,"%s_MinorVersion))\n",n);

			dump_nest(out,1);
			out_printf(out,"{\n");

			dump_nest(out,2);
			out_printf(out,"return NULL;\n");

			dump_nest(out,1);
			out_printf(out,"}\n");
		}
	}

	meta[0]=0;

	get_actual_meta_class(iface,n,sizeof(n));

	if (strcmp(n,"SOMClass"))
	{
		if (cplusplus)
		{
			snprintf(meta,sizeof(meta),"(%s *)(void *)",n);
		}
		else
		{
			snprintf(meta,sizeof(meta),"(%s SOMSTAR)(void *)",n);
		}
	}

	get_c_name(iface,n,sizeof(n));
	dump_nest(out,1);

	out_printf(out,"return %ssomBuildClass(0x%lx,&%sSCI,\n",
				meta,mask,n);

	dump_nest(out,2);
	out_printf(out,"somtmajorVersion,\n");

	dump_nest(out,2);
	out_printf(out,"somtminorVersion);\n");

	if (iface->_parents.length())
	{
		if (0)
		{
			unsigned long k=0;

			while (k < iface->_parents.length())
			{
				dump_nest(out,1);
				get_c_name(iface->_parents.get(k),n,sizeof(n));
				out_printf(out,"somReleaseClassReference(%s_classObject);\n",n);
				k++;
			}
		}
	}

/*	dump_nest(out,1);
	get_c_name(iface,n);
	i=sprintf(buf,"return %s_classObject;\n",n);
	out->write(buf,i);
*/
	out_printf(out,"}\n");

	if (cplusplus)
	{
		out_printf(out,"static void SOMLINK _somC_%s_classInit(SOMClass *somSelf)\n",n);
	}
	else
	{
		out_printf(out,"static void SOMLINK _somC_%s_classInit(SOMClass SOMSTAR somSelf)\n",n);
	}

	out_printf(out,"{\n");

	out_printf(out,"/* do parent resolves... */\n");

	if (overrides && !is_kernel_class(iface))
	{
		long k1=overrides->length();

		while (k1--)
		{
			RHBoperation *op=overrides->get(k1)->is_operation();

			if (op)
			{
				long k2=iface->_parents.length();

				while (k2--)
				{
					RHBinterface *par=iface->_parents.get(k2)->is_interface();

					if (par)
					{
						generate_parent_resolved(out,iface,par,op,2);
					}
				}
			}
		}
	}

	out_printf(out,"/* call classinit if defined... */\n"); 

	if (get_cif(iface,n,sizeof(n)))
	{
		out_printf(out,"\t%s(somSelf);\n",n);
	}
	else
	{
		out_printf(out,"\tSOM_IgnoreWarning(somSelf);\n");
	}

	out_printf(out,"}\n");

	get_c_name(iface,n,sizeof(n));

	if (strcmp(n,"SOMObject") &&
		strcmp(n,"SOMClass") &&
		strcmp(n,"SOMClassMgr"))
	{
		generate_internal_class_data(out,iface);
		generate_internal_cclass_data(out,iface);
	}

	public_class_data_resolve(out,0,iface,"ClassData");
	public_class_data_resolve(out,0,iface,"CClassData");
}

void RHBheader_emitter::public_class_data_resolve(RHBoutput *out,int nest,RHBinterface *iface,const char *dn)
{
	char n[256];
	get_c_name(iface,n,sizeof(n));
	dump_nest(out,nest);
	out_printf(out,"#ifdef %s\n",szSOM_RESOLVE_DATA);
	dump_nest(out,nest+1);
	out_printf(out,"SOMEXTERN struct %s%sStructure * SOMLINK resolve_%s%s(void)\n",n,dn,n,dn);

	dump_nest(out,nest+1);
	out_printf(out,"{ return &_%s%s; }\n",n,dn);

	dump_nest(out,nest);
	out_printf(out,"#endif /* %s */\n",szSOM_RESOLVE_DATA);
}


void RHBheader_emitter::enum_attributes(RHBoutput *out,RHBinterface *iface)
{
RHBattribute *attr;
RHBelement *ch;

	ch=iface->children();

	while (ch)
	{
		if (ch->is_type())
		{
			attr=ch->is_type()->is_attribute();

			if (attr)
			{
				generate_internal_attribute(out,iface,attr);
			}
		}

		ch=ch->next();
	}
}

void RHBheader_emitter::generate_internal_attribute(RHBoutput *out,RHBinterface *iface,RHBattribute *attr)
{
char n[256];
int do_set=0,do_get=0;

	if (!attr->has_modifier_value("nodata"))
	{
		if (!attr->has_modifier_value("noset"))
		{
			do_set=1;
		}
		if (!attr->has_modifier_value("noget"))
		{
			do_get=1;
		}
	}

	if (!attr->readonly)
	{
		if (do_set)
		{
			get_c_name(attr,n,sizeof(n));
			out_printf(out,"/* set for attribute %s */\n",n);
			gen_set_attribute(out,iface,attr);
		}
		else
		{
			attr->build_id(n,sizeof(n));
			out_printf(out,"/* developer must provide set for attribute %s */\n",n);
		}
	}

	if (do_get)
	{
		gen_get_attribute(out,iface,attr);
	}
	else
	{
		attr->build_id(n,sizeof(n));
		out_printf(out,"/* developer must provide get for attribute %s */\n",n);
	}
}

void RHBheader_emitter::gen_set_attribute(RHBoutput *out,RHBinterface *iface,RHBattribute *attr)
{
char n[256];
int by_ref=0;
RHBtype *by_array=NULL;

	get_c_name(iface,n,sizeof(n));

	out_printf(out,"SOM_Scope void");

	get_function_prefix(iface,n,sizeof(n));
	out_printf(out," SOMLINK %s",n);

	get_c_name(iface,n,sizeof(n));
	out_printf(out,"_set_%s(",attr->id);

	use_type(out,iface,"somSelf,");

	if (operation_needs_environment(iface,0))
	{
		out_printf(out,"Environment *ev,");
	}

	{
		char buf[256];
		RHBparameter *param;
		RHBelement *el;
		snprintf(buf,sizeof(buf),"_set_%s",attr->id);
		el=iface->find_named_element(0,buf,sizeof(buf));
		param=el->children()->is_type()->is_parameter();
		by_ref=parameter_by_reference(param,NULL);
	}

	by_array=get_array_slice(attr->attribute_type);

	if (by_ref)
	{
		use_type(out,attr->attribute_type,0);
		out_printf(out," *value");
	}
	else
	{
		use_type(out,attr->attribute_type,"value");
	}

	out_printf(out,")\n{\n");

	if (by_array)
	{
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"%sData *somThis=%sGetData(somSelf);\n",n,n);
	}

	if (operation_needs_environment(iface,0))
	{
		out_printf(out,"SOM_IgnoreWarning(ev);\n");
	}

	dump_nest(out,1);

	if (by_array)
	{
		out_printf(out,"if (value != somThis->%s)\n"
					"\tmemcpy(somThis->%s,value,sizeof(somThis->%s));\n",
					attr->id,attr->id,attr->id);
	}
	else
	{
		if (by_ref)
		{
			out_printf(out,"%sGetData(somSelf)->%s=*value;\n",
				n,attr->id);
		}
		else
		{
			out_printf(out,"%sGetData(somSelf)->%s=value;\n",
				n,attr->id);
		}
	}

	out_printf(out,"}\n");
}

void RHBheader_emitter::gen_get_attribute(RHBoutput *out,RHBinterface *iface,RHBattribute *attr)
{
char n[256];

	get_c_name(iface,n,sizeof(n));

	out_printf(out,"SOM_Scope ");

	RHBtype *t=get_array_slice(attr->attribute_type);
	if (t)
	{
		use_type(out,t,0);

		out_printf(out,"* ");
	}
	else
	{
		use_type(out,attr->attribute_type,0);
	}

	get_function_prefix(iface,n,sizeof(n));
	out_printf(out," SOMLINK %s",n);

	get_c_name(iface,n,sizeof(n));
	out_printf(out,"_get_%s(",attr->id);

	use_type(out,iface,"somSelf");

	if (operation_needs_environment(iface,0))
	{
		out_printf(out,",Environment *ev");
	}

	out_printf(out,")\n{\n");

	if (operation_needs_environment(iface,0))
	{
		out_printf(out,"SOM_IgnoreWarning(ev);\n");
	}

	dump_nest(out,1);

	t=get_array_slice(attr->attribute_type);

	if (t)
	{
		int i=0;
		t=attr->attribute_type;
		while (t)
		{
			t=unwind_typedef(t);
			if (t->is_array())
			{
				t=t->is_array()->array_of;
				i++;
			}
			else
			{
				break;
			}
		}
		out_printf(out,"return %sGetData(somSelf)->%s",n,attr->id);
		if (i) i--;
		while (i--)
		{
			out_printf(out,"[0]");
		}
		out_printf(out,";\n");
	}
	else
	{
		out_printf(out,"return %sGetData(somSelf)->%s;\n",
			n,attr->id);
	}

	out_printf(out,"}\n");
}

void RHBheader_emitter::generate_internal_ctrl_macros(RHBoutput * /*out*/,RHBinterface * /*iface */)
{
/*	generate_begininit(out,iface,"somDefaultInit");
	generate_begininit(out,iface,"somDefaultCopyInit");
	generate_begininit(out,iface,"somDefaultConstCopyInit");
	generate_begininit(out,iface,"somDefaultVCopyInit");
	generate_begininit(out,iface,"somDefaultConstVCopyInit");
	generate_beginassignop(out,iface);
	generate_begindestruct(out,iface);
	generate_enddestruct(out,iface);
	generate_parent_initdestruct(out,iface);*/
}

boolean RHBheader_emitter::do_som21_macro(RHBinterface *iface,const char *name)
{
	if (iface->_parents.length())
	{
		return does_override(iface,name);
	}
	else
	{
		RHBelement *el;
		char buf[256];
		strncpy(buf,name,sizeof(buf));
		el=iface->find_named_element(0,buf,sizeof(buf));
		if (!el) return 0;
		if (!el->is_operation()) return 0;
	}

	return 1;
}

boolean RHBheader_emitter::does_override(RHBinterface *iface,const char *name)
{
	RHBelement *el;
	char buf[256];

	strncpy(buf,name,sizeof(buf));

	el=iface->find_named_element(0,buf,sizeof(buf));

	if (el)
	{
		if (el->has_modifier_value("override"))
		{
			return 1;
		}
	}

	strncpy(buf,"override",sizeof(buf));
	el=iface->find_named_element(0,buf,sizeof(buf));
	if (el)
	{
		if (el->has_modifier_value(name))
		{
			return 1;
		}
	}

	return 0;
}

#if 0
void RHBheader_emitter::generate_begininit(
			RHBoutput *out,
			RHBinterface *iface,
			const char *meth_name)
{
	char n[256];

	if (do_som21_macro(iface,meth_name))
	{
		get_c_name(iface,n);
		out_printf(out,"#define %s_BeginInitializer_%s \\\n",n,meth_name);

		dump_nest(out,2);
		i=sprintf(buf,"if (!ctrl) {\\\n");
		out->write(buf,i);
		dump_nest(out,3);
		i=sprintf(buf,"ctrl = &globalCtrl;\\\n");
		out->write(buf,i);
		dump_nest(out,3);
		i=sprintf(buf,"memcpy(ctrl,&(((somParentMtabStructPtr)(void *)%sCClassData.parentMtab)->initCtrl),sizeof(somInitCtrl));\\\n",n);
		out->write(buf,i);
		dump_nest(out,2);
		i=sprintf(buf,"}\\\n");
		out->write(buf,i);

		dump_nest(out,2);
		i=sprintf(buf,"myMask = ctrl->mask;\\\n");
		out->write(buf,i);

		if (iface->instanceData)
		{
			if (iface->instanceData->children())
			{
				dump_nest(out,2);
				i=sprintf(buf,"somThis=(%sData *)((char*)somSelf + ctrl->info->dataOffset);\\\n",n);
				out->write(buf,i);
			}
		}

		dump_nest(out,2);
		i=sprintf(buf,"ctrl->info = (somInitInfo*)(ctrl->infoSize + (char*)(ctrl->info)); \\\n");
		out->write(buf,i);

		dump_nest(out,2);
		i=sprintf(buf,"ctrl->mask += 1;\n");
		out->write(buf,i);
	}
}
#endif

#if 0
void RHBheader_emitter::generate_beginassignop(
			RHBoutput *out,
			RHBinterface *iface)
{
	char buf[256];
	char n[256];
	int i;

	get_c_name(iface,n);
	i=sprintf(buf,"#define %s_BeginAssignmentOp \\\n",n);
	out->write(buf,i);

	dump_nest(out,2);
	i=sprintf(buf,"if (!ctrl) {\\\n");
	out->write(buf,i);
	dump_nest(out,3);
	i=sprintf(buf,"ctrl = &globalCtrl;\\\n");
	out->write(buf,i);
	dump_nest(out,3);
	i=sprintf(buf,"memcpy(ctrl,&(((somParentMtabStructPtr)(void *)%sCClassData.parentMtab)->assignCtrl),sizeof(somAssignCtrl));\\\n",n);
	out->write(buf,i);
	dump_nest(out,2);
	i=sprintf(buf,"}\\\n");
	out->write(buf,i);


	dump_nest(out,2);
	i=sprintf(buf,"myMask = ctrl->mask;\\\n");
	out->write(buf,i);

	if (iface->instanceData)
	{
		if (iface->instanceData->children())
		{
			dump_nest(out,2);
			i=sprintf(buf,"somThis=(%sData *)((char*)somSelf + ctrl->info->dataOffset);\\\n",n);
			out->write(buf,i);
		}
	}

	dump_nest(out,2);
	i=sprintf(buf,"ctrl->info = (somAssignInfo*)(ctrl->infoSize + (char*)(ctrl->info)); \\\n");
	out->write(buf,i);

	dump_nest(out,2);
	i=sprintf(buf,"ctrl->mask += 1;\n");
	out->write(buf,i);
}
#endif

#if 0
void RHBheader_emitter::generate_begindestruct(
			RHBoutput *out,
			RHBinterface *iface)
{
	char buf[256];
	char n[256];
	int i;

	if (do_som21_macro(iface,"somDestruct"))
	{
		get_c_name(iface,n);
		i=sprintf(buf,"#define %s_BeginDestructor \\\n",n);
		out->write(buf,i);

		dump_nest(out,2);
		i=sprintf(buf,"if (!ctrl) {\\\n");
		out->write(buf,i);
		dump_nest(out,3);
		i=sprintf(buf,"ctrl = &globalCtrl;\\\n");
		out->write(buf,i);
		dump_nest(out,3);
		i=sprintf(buf,"memcpy(ctrl,&(((somParentMtabStructPtr)(void *)%sCClassData.parentMtab)->destructCtrl),sizeof(somDestructCtrl));\\\n",n);
		out->write(buf,i);
		dump_nest(out,2);
		i=sprintf(buf,"}\\\n");
		out->write(buf,i);

		if (iface->instanceData)
		{
			if (iface->instanceData->children())
			{
				dump_nest(out,2);
				i=sprintf(buf,"somThis=(%sData *)((char*)somSelf + ctrl->info->dataOffset);\\\n",n);
				out->write(buf,i);
			}
		}

		dump_nest(out,2);
		i=sprintf(buf,"myMask = ctrl->mask;\n");
		out->write(buf,i);
	}
}
#endif

#if 0
void RHBheader_emitter::generate_enddestruct(
			RHBoutput *out,
			RHBinterface *iface)
{
	char buf[256];
	char n[256];
	int i;
	int j;
	RHBinterface *par;

	if (do_som21_macro(iface,"somDestruct"))
	{

		get_c_name(iface,n);
		i=sprintf(buf,"#define %s_EndDestructor \\\n",n);
		out->write(buf,i);

		dump_nest(out,2);
		i=sprintf(buf,"ctrl->info = (somDestructInfo*)(ctrl->infoSize + (char*)(ctrl->info)); \\\n");
		out->write(buf,i);
		dump_nest(out,2);
		i=sprintf(buf,"ctrl->mask += 1; \\\n");
		out->write(buf,i);

		j=0;

		while (j < iface->_parents.length())
		{
			char m[256];

			par=iface->_parents.get(j)->is_type()->is_interface();

			dump_nest(out,2);
			get_c_name(par,m);

			i=sprintf(buf,"%s_DeInit_%s_somDestruct(somSelf,0,ctrl);\\\n",n,m);
			out->write(buf,i);
			j++;
		}

		dump_nest(out,2);
		i=sprintf(buf,"if (doFree) SOMClass_somDeallocate(%sClassData.classObject, (string)somSelf);\n",n);
		out->write(buf,i);
	}
}
#endif

#if 0
void RHBheader_emitter::generate_parent_initdestruct(
			RHBoutput *out,
			RHBinterface *iface)
{
	RHBinterface *par;
	int i;
	char n[256];
	char buf[256];
	int j;

	j=0;

	while (j < iface->_parents.length())
	{
		par=iface->_parents.get(j)->is_type()->is_interface();

		/* creator */

		if (do_som21_macro(iface,"somDefaultInit"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Init_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultInit(somSelf,ctrl) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultInit)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultInit)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl)\n");
			out->write(buf,i);
		}

		/* copy creator */

		if (do_som21_macro(iface,"somDefaultCopyInit"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Init_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultCopyInit(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}

		/* const copy creator */

		if (do_som21_macro(iface,"somDefaultConstCopyInit"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Init_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultConstCopyInit(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultConstCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultConstCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}

		/* volatile copy creator */

		if (do_som21_macro(iface,"somDefaultVCopyInit"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Init_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultVCopyInit(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultVCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}


		/* volatile const copy creator */

		if (do_som21_macro(iface,"somDefaultConstVCopyInit"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Init_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultConstVCopyInit(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultConstVCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultConstCopyInit)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}
		
		/* assignment */

		if (do_som21_macro(iface,"somDefaultAssign"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Assign_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultAssign(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}

		/* volatile assignment */

		if (do_som21_macro(iface,"somDefaultVAssign"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Assign_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultVAssign(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultVAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}

		/* const assignment */

		if (do_som21_macro(iface,"somDefaultConstAssign"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Assign_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultConstAssign(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultConstAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultConstAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}

		/* volatile const assignment */

		if (do_som21_macro(iface,"somDefaultConstVAssign"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_Assign_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDefaultConstVAssign(somSelf,ctrl,fromObj) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDefaultConstVAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultConstAssign)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,ctrl,fromObj)\n");
			out->write(buf,i);
		}

		/* destructor */

		if (do_som21_macro(iface,"somDestruct"))
		{
			get_c_name(iface,n);

			i=sprintf(buf,"#define %s_DeInit_",n);
			out->write(buf,i);

			get_c_name(par,n);

			i=sprintf(buf,"%s_somDestruct(somSelf,doFree,ctrl) \\\n",n);
			out->write(buf,i);

			dump_nest(out,2);

			i=sprintf(buf,"if (myMask[0]&1) \\\n");
			out->write(buf,i);

			dump_nest(out,3);

			i=sprintf(buf,"((somTD_SOMObject_somDestruct)\\\n");
			out->write(buf,i);

			dump_nest(out,4);

			i=sprintf(buf,"ctrl->info->defaultDestruct)\\\n");
			out->write(buf,i);

			dump_nest(out,5);

			i=sprintf(buf,"(somSelf,0,ctrl)\n");
			out->write(buf,i);
		}

		j++;
	}
}
#endif

void RHBheader_emitter::generate_internal_cclass_data(RHBoutput *out,RHBinterface *iface)
{
	char n[256];
	RHBelement_sequence vaops;
	int numva=get_va_operations(iface,&vaops);

	get_c_name(iface,n,sizeof(n));
	out_printf(out,"struct %sCClassDataStructure SOMDLINK %sCClassData",n,n);

	if (numva)
	{
		out_printf(out,"={(somMethodTabs)0,(somDToken)0\n");
	
		numva=0;

		while (numva < (int)vaops.length())
		{
			RHBoperation *op=vaops.get(numva)->is_operation();

			out_printf(out,",(somMethodPtr)somVA_%s_%s\n",n,op->id);

			numva++;
		}

		out_printf(out,"}");
	}

	out_printf(out,";\n");
}

void RHBheader_emitter::generate_internal_class_data(RHBoutput *out,RHBinterface *iface)
{
	char n[256];
	RHBelement *el;

	get_c_name(iface,n,sizeof(n));
	out_printf(out,"struct %sClassDataStructure SOMDLINK %sClassData={\n",n,n);

	el=iface->classData->children();

	while (el)
	{
		char val_done=0;

		dump_nest(out,1);

		out_printf(out,"/* %s */ ",el->id);

		if (strcmp(/*buf*/el->id,"classObject"))
		{
			RHBelement *e2=iface->children();

			while (e2)
			{
				if (0==strcmp(e2->id,el->id))
				{
					RHBoperation *op=e2->is_operation();

					if (op)
					{
						if (is_operation_procedure(op))
						{
					/*		get_c_name(op,n);

							i=sprintf(buf,"\n");
							out->write(buf,i);
							dump_nest(out,2);*/

					/*		i=sprintf(buf,"(somTD_%s)\n",n);
							out->write(buf,i);
							dump_nest(out,3);*/

							get_function_prefix(iface,n,sizeof(n));

							out_printf(out,"%s%s",n,op->id);

							val_done=1;

							e2=0;
						}
					}
				}

				if (e2)
				{
					e2=e2->next();
				}
			}
		}


		if (!val_done)
		{
			out_printf(out,"0");
		}

		el=el->next();

		if (el)
		{
			out_printf(out,",\n");
		}
	}

	out_printf(out,"};\n");
}

void RHBheader_emitter::generate_somId(RHBoutput *out,int nest,
		RHBelement *el,
		const char *strPrefix,
		const char *idPrefix,
		boolean ir_spec,boolean recurse)
{
	char str[256];
	char var[256];

#ifdef _WIN32
	#ifdef _DEBUG
		if (ir_spec) bomb("emitting somId for IR");
	#endif
#endif

	if (recurse)
	{
		get_ir_name(el,str,sizeof(str));

		if (!ir_spec)
		{
			char *p=str;

			if (0==memcmp("::",p,2))
			{
				while (p[2])
				{
					*p=p[2];
					p++;
				}

				*p=0;
			}
		}
	}
	else
	{
		strncpy(str,el->id,sizeof(str));
	}

	if (recurse)
	{
		get_c_name(el,var,sizeof(var));
	}
	else
	{
		strncpy(var,el->id,sizeof(var));
	}

	{
		char strName[256];

		snprintf(strName,sizeof(strName),"%s_%s",strPrefix,var);

		begin_generate_static(out,strName,nest);
		undoublebar(out,strName);
		dump_nest(out,nest+1);

		out_printf(out,"static const char * %s=",strName);
		out_printf(out,"%c%s%c;\n",double_quote,str,double_quote);
		
		end_generate_static(out,strName,nest);
	}

	if (idPrefix)
	{
		if (*idPrefix)
		{
			char idName[256];
			snprintf(idName,sizeof(idName),"%s_%s",idPrefix,var);
			begin_generate_static(out,idName,nest);
			undoublebar(out,idName);
			dump_nest(out,nest+1);
			out_printf(out,"static const somConstId %s=",idName);
			out_printf(out,"&%s_%s;\n",strPrefix,var);

			end_generate_static(out,idName,nest);
		}
	}
}

void RHBheader_emitter::begin_generate_static(RHBoutput *out,const char *name,int nest)
{
	dump_nest(out,nest);
	out_printf(out,"#ifndef static_%s\n",name);

	dump_nest(out,nest+1);
	out_printf(out,"#define static_%s\n",name);
}

void RHBheader_emitter::end_generate_static(RHBoutput *out,const char *name,int nest)
{
	dump_nest(out,nest);
	out_printf(out,"#endif /* static_%s */\n",name);
}


void RHBheader_emitter::generate_somTDs(RHBoutput *out,RHBinterface *iface,int nest)
{
	RHBelement *e=iface->children();

	while (e)
	{
		RHBoperation *op=e->is_operation();

		if (op)
		{
			generate_somTD(out,iface,op,nest);
		}

		e=e->next();
	}

	if (cplusplus)
	{
		RHBelement_sequence seq;
		unsigned long i=0;

		determine_inherited(iface,iface,&seq);

		while (i < seq.length())
		{
			RHBoperation *op=seq.get(i)->is_operation();
			generate_somTD(out,iface,op,nest);
			i++;
		}
	}
}


RHBtype *RHBheader_emitter::generate_array_slice_typedef(
	RHBoutput *out,int nest,RHBtype *t,const char *n)
{
	t=unwind_typedef(t);
	RHBarray *td=t->is_array();
	t=td->array_of;

	dump_nest(out,nest);
	out_printf(out,"typedef ");
	use_type(out,t,n);
	out_printf(out,";\n");

	return t;
}

RHBtype *RHBheader_emitter::get_array_slice(RHBtype *t)
{
	RHBtype *slice=NULL;

	while (t)
	{
		if (t->is_typedef())
		{
			t=t->is_typedef()->alias_for;
		}
		else
		{
			RHBarray *array=t->is_array();

			if (array)
			{
				t=array->array_of;

				slice=t;
			}
			else
			{
				break;
			}
		}
	}

	return slice;
}

void RHBheader_emitter::generate_somTD(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	extern_c_proto_t ext_c_proto(this,out,nest);
	char somTD_name[256];
	char somTP_name[256];

	{
		char buf[256];
		get_c_name(iface,buf,sizeof(buf));
		strncpy(somTP_name,"somTP_",sizeof(somTP_name));
		strncat(somTP_name,buf,sizeof(somTP_name)-1);
		strncat(somTP_name,"_",sizeof(somTP_name)-1);
		strncat(somTP_name,op->id,sizeof(somTP_name)-1);
	}

	{
		char buf[256];
		get_c_name(iface,buf,sizeof(buf));
		strncpy(somTD_name,"somTD_",sizeof(somTD_name));
		strncat(somTD_name,buf,sizeof(somTD_name)-1);
		strncat(somTD_name,"_",sizeof(somTD_name)-1);
		strncat(somTD_name,op->id,sizeof(somTD_name)-1);
	}

	dump_nest(out,nest);
	out_printf(out,"#ifdef __IBMC__\n");

	undoublebar(out,somTD_name);
	undoublebar(out,somTP_name);

	dump_nest(out,nest+1);
	out_printf(out,"typedef ");

	RHBtype *t=op->return_type;

	if (t)
	{
		RHBtype *slice=get_array_slice(t);

		if (slice)
		{
			use_type(out,slice,0);
			out->write("*",1);
		}
		else
		{
			use_type(out,t,0);
		}
	}
	else
	{
		out_printf(out,"void ");
	}

	out_printf(out,"(%s)",somTP_name);
	generate_parameter_list(out,iface,op,nest+1,1,0);
	out_printf(out,";\n");

	dump_nest(out,nest+1);
	out_printf(out,"#pragma linkage(%s,system)\n",somTP_name);

	dump_nest(out,nest+1);

	out_printf(out,"typedef %s *%s;\n",somTP_name,somTD_name);

	dump_nest(out,nest);
	out_printf(out,"#else /* __IBMC__ */\n");

	undoublebar(out,somTD_name);

	dump_nest(out,nest+1);

	out_printf(out,"typedef ");

	t=op->return_type;
	if (t)
	{
		RHBtype *slice=get_array_slice(t);

		if (slice)
		{
			use_type(out,slice,0);
			out->write("*",1);
		}
		else
		{
			use_type(out,t,0);
		}
	}
	else
	{
		out_printf(out,"void ");
	}

	out_printf(out,"(SOMLINK * %s)",somTD_name);

	generate_parameter_list(out,iface,op,nest+1,1,0);
	out_printf(out,";\n");

	dump_nest(out,nest);
	out_printf(out,"#endif /* __IBMC__ */\n");
}

void RHBheader_emitter::generate_parameter_list(
		RHBoutput *out,
		RHBinterface *iface,
		RHBoperation *op,
		int nesting,
		boolean include_somSelf,
		boolean do_const_in)
{
	RHBelement *e;
	boolean first_param=1;

	out_printf(out,"(\n");

	dump_nest(out,nesting+1);

	if (is_operation_noself(op))
	{
		include_somSelf=0;
	}

	if (include_somSelf)
	{
		first_param=0;

		if (do_const_in) 
		{
			out_printf(out,"const ");
		}

		use_type(out,iface,"somSelf");
	}

	if (operation_needs_environment(iface,op))
	{
		if (first_param)
		{
			first_param=0;
		}
		else
		{
			out_printf(out,",\n");
			dump_nest(out,nesting+1);
		}

		out_printf(out,"Environment *ev");
	}

	if (operation_needs_context(iface,op))
	{
		if (first_param)
		{
			first_param=0;
		}
		else
		{
			out_printf(out,",\n");
			dump_nest(out,nesting+1);
		}

		if (do_const_in)
		{
			out_printf(out,"const ");
		}

		if (cplusplus)
		{
			out_printf(out,"Context * ctx");
		}
		else
		{
			out_printf(out,"Context SOMSTAR ctx");
		}
	}

	e=op->children();

	while (e)
	{
		RHBparameter *param=e->is_parameter();

		if (param)
		{
			RHBtype *type=NULL;
			int by_ref;
			char name[256];

			name[0]=0;

			if (first_param)
			{
				first_param=0;
			}
			else
			{
				out_printf(out,",\n");
				dump_nest(out,nesting+1);
			}

			out_printf(out,"/* %s */ ",param->mode);

			by_ref=parameter_by_reference(param,NULL /* &type */);

			if (do_const_in && do_const_param(param))
			{
				out_printf(out,"const ");
			}

			if (!type) type=param->parameter_type;

			if (by_ref)
			{
				strcat(name,"*");
			}

			strcat(name,param->id);

			use_type(out,type,name);
		}

		e=e->next();
	}

	out_printf(out,")");
}

boolean RHBheader_emitter::operation_needs_environment(
		RHBinterface *iface,
		RHBoperation *op)
{
	RHBelement *e;
	const char *p;
	char buf[256];
	strncpy(buf,"callstyle",sizeof(buf));

	if (op)
	{
		if (iface != op->parent())
		{
			if (op->parent()->is_interface())
			{
				iface=op->parent()->is_interface();
			}
		}
	}

	e=iface->find_named_element(0,buf,sizeof(buf));
	if (!e) return 1;
	p=e->modifier_data.get(0);

	if (!p) return 1;

	if (strcmp(p,"oidl")) return 1;

	return 0;
}

boolean RHBheader_emitter::parameter_by_reference(RHBparameter *param,RHBtype **array_of)
{
	RHBtype *t=param->parameter_type;

	if (strcmp(param->mode,"in")) 
	{
		while (t)
		{
			if (t->is_array())
			{
				/* special case, an array is already a pointer of
					sorts */

				if (array_of)
				{
					*array_of=t->is_array()->array_of;

					return 1;
				}

				return 0;
			}

			if (t->is_typedef())
			{
				t=t->is_typedef()->alias_for;
			}
			else
			{
				break;
			}
		}

		return 1;
	}

	while (t)
	{
		if (t->is_base_type()) 
		{
			return t->is_base_type()->_in_by_ref;
		}

		if (t->is_interface()) return 0;
		if (t->is_enum()) return 0;
		if (t->is_pointer()) return 0;
		if (t->is_string()) return 0;
		if (t->is_array()) 
		{
			if (array_of)
			{
				*array_of=t->is_array()->array_of;

				return 1;
			}

			return 0;
		}

		if (t->is_sequence()) return 1;

		if (0==strcmp(t->id,"somId"))
		{
			return 0;
		}

		if (0==strcmp(t->id,"va_list"))
		{
			return 0;
		}

		if (0==strcmp(t->id,"somToken"))
		{
			return 0;
		}

		if (t->is_typedef())
		{
			t=t->is_typedef()->alias_for;
		}
		else
		{
			if (t->is_qualified())
			{
				t=t->is_qualified()->base_type;
			}
			else
			{
				t=0;
			}
		}
	}

	return 1;
}

boolean RHBheader_emitter::operation_needs_context(
		RHBinterface * /*iface*/,
		RHBoperation *op)
{
	if (op->context_list.length()) return 1;

	return 0;
}

void RHBheader_emitter::least_ambiguous_interface(RHBinterface *iface,const char *iface_name,char *buf,size_t buflen)
{
	buf[0]=0;

	if (cplusplus)
	{
		if (iface)
		{
/*			int i;

			i=iface->_parents.length();

			if (i)
			{
				char n[256];
				RHBinterface *par;

				par=iface->_parents.get(0)->is_interface();

				get_c_name(par,n);

				if (strcmp(n,iface_name))
				{
					if (i > 1)
					{
						char *p;
						p=&buf[strlen(buf)];
						i=sprintf(p,"(%s *)",n);
						buf=p+i;
					}

					least_ambiguous_interface(par,n,buf);
				}
			}*/

			snprintf(buf,buflen,"(%s *)(void *)",iface_name);
		}
	}
}

void RHBheader_emitter::generate_inline_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[1024];
	RHBelement *el;
	boolean is_proc,any_params,is_noself;
	const char *somSelf;
	
	is_noself=is_operation_noself(op);
	is_proc=is_operation_procedure(op);

	if (is_proc)
	{
		somSelf="this";
	}
	else
	{
		somSelf="this";
	}

	dump_nest(out,nest+1);

	out_printf(out,"inline ");

	if (is_noself)
	{
		out_printf(out,"static ");
	}

	if (op->return_type)
	{
		RHBtype *slice=get_array_slice(op->return_type);
		if (slice)
		{
			use_type(out,slice,0);
			out_printf(out,"* ");
		}
		else
		{
			use_type(out,op->return_type,0);
		}
	}
	else
	{
		out_printf(out,"void ");
	}

	if (op_is_varg(op))
	{
		char buf[256];
		get_c_name(op,buf,sizeof(buf));
		out_printf(out,"%s",buf);
	}
	else
	{
		out_printf(out,"%s",op->id);
	}

	generate_parameter_list(out,iface,op,2,0,1);

	out_printf(out,"\n");

	dump_nest(out,nest+1);
	out_printf(out,"{\n");

	dump_nest(out,nest+2);

	if (operation_has_return(op))
	{
		out_printf(out,"return ");
	}

	if (is_proc)
	{
/*		get_c_name(iface,n);*/
		get_c_name(op->parent(),n,sizeof(n));
		out_printf(out,"%sClassData.%s\n",n,op->id);
	}
	else
	{
		get_c_name(iface,n,sizeof(n));

		get_c_name(op->parent(),n,sizeof(n));
		out_printf(out,"SOM_Resolve((void *)%s,%s,%s)\n",
						somSelf,n,op->id);
	}

	dump_nest(out,nest+4);

	if (is_noself)
	{
		out_printf(out,"(");
		any_params=0;
	}
	else
	{
		get_c_name(op->parent(),n,sizeof(n));
		out_printf(out,"((%s *)(void *)%s",n,somSelf);
		any_params=1;
	}

	if (operation_needs_environment(iface,op))
	{
		if (any_params)
		{
			out_printf(out,",");
		}
		else
		{
			any_params=1;
		}
		out_printf(out,"ev");
	}

	if (operation_needs_context(iface,op))
	{
		if (any_params)
		{
			out_printf(out,",");
		}
		else
		{
			any_params=1;
		}
		out_printf(out,"(Context *)ctx");
	}

	el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			if (any_params)
			{
				out_printf(out,",");
			}
			else
			{
				any_params=1;
			}

			if (do_const_param(param))
			{
				RHBtype *array_of=NULL;
				out_printf(out,"(");

				if (parameter_by_reference(param,&array_of))
				{
					if (array_of)
					{
						use_type(out,array_of,NULL);
					}
					else
					{
						use_type(out,param->parameter_type,NULL);
					}

					out_printf(out," *");
				}
				else
				{
					use_type(out,param->parameter_type,NULL);
				}

				out_printf(out,")");
			}

			out_printf(out,"%s",param->id);
		}

		el=el->next();
	}

	out_printf(out,");\n");

	dump_nest(out,nest+1);
	out_printf(out,"};\n");

	if (op_is_varg(op))
	{
		generate_inline_varg_operation(out,iface,op,nest+1);
	}
}

boolean RHBheader_emitter::operation_has_return(RHBoperation *op)
{
	if (op->return_type)
	{	
		char buf[256];

		get_c_name(op->return_type,buf,sizeof(buf));

		if (strcmp(buf,"void")) return 1;
	}

	return 0;
}

void RHBheader_emitter::generate_macro_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[256];
	RHBelement *el;

	get_c_name(op,n,sizeof(n));

	dump_nest(out,nest);
	out_printf(out,"#ifndef %s\n",n);

	if (is_operation_procedure(op))
	{
		dump_nest(out,nest+1);
		out_printf(out,"#define %s ",n);
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"%sClassData.%s\n",n,op->id);
	}
	else
	{
		dump_nest(out,nest+1);
		out_printf(out,"#define %s(somSelf",n);

		if (operation_needs_environment(iface,op))
		{
			out_printf(out,",ev");
		}

		if (operation_needs_context(iface,op))
		{
			out_printf(out,",ctx");
		}

		el=op->children();

		while (el)
		{
			RHBparameter *param=el->is_parameter();

			if (param)
			{
				out_printf(out,",%s",param->id);
			}

			el=el->next();
		}

		out_printf(out,") \\\n");

		dump_nest(out,nest+2);

/*		i=sprintf(buf,"((somTD_%s)somResolve(somSelf,",n);
		out->write(buf,i);
		get_c_name(iface,n);
		i=sprintf(buf,"%sClassData.%s)) \\\n",
				n,op->id);
*/
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"SOM_Resolve(somSelf,%s,%s)  \\\n",
					n,op->id);

		dump_nest(out,nest+3);
		out_printf(out,"(somSelf");

		if (operation_needs_environment(iface,op))
		{
			out_printf(out,",ev");
		}

		if (operation_needs_context(iface,op))
		{
			out_printf(out,",ctx");
		}

		el=op->children();

		while (el)
		{
			RHBparameter *param=el->is_parameter();

			if (param)
			{
				out_printf(out,",%s",param->id);
			}

			el=el->next();
		}

		out_printf(out,")\n");
	}

	/* do the short name form */

	gen_shortname(out,op,"_",nest+1);

	dump_nest(out,nest);
	get_c_name(op,n,sizeof(n));
	out_printf(out,"#endif /* %s */\n",n);

	if (op_is_varg(op))
	{
		extern_c_begin(out,nest);

		if (nolegacy())
		{
			dump_nest(out,nest);
			out_printf(out,"#ifndef va_%s\n",n);
			dump_nest(out,nest+1);
			out_printf(out,"#define va_%s somva_%s\n",n,n);
			dump_nest(out,nest);
			out_printf(out,"#endif /* va_%s */\n",n);
		}
		else
		{
			generate_va_stub(out,iface,op,nest,0,"va");
		}
		generate_va_stub(out,iface,op,nest,1,"somva");
		extern_c_end(out,nest);
	}
}

void RHBheader_emitter::gen_shortname(RHBoutput *out,RHBtype *op,const char *lead,int nest)
{
	char n[1024];

	if (!op->id)
	{
		return;
	}

	get_c_name(op,n,sizeof(n));

	if (!strcmp(n,op->id)) 
	{
		return;
	}

	if (!lead) lead="";

	dump_nest(out,nest);
	out_printf(out,"#ifndef SOM_DONT_USE_SHORT_NAMES\n");

	dump_nest(out,nest+1);
	out_printf(out,"#ifndef SOMGD_%s%s\n",lead,op->id);

	dump_nest(out,nest+2);
	out_printf(out,"#if defined(%s%s)\n",lead,op->id);

	dump_nest(out,nest+3);
	out_printf(out,"#undef %s%s\n",lead,op->id);

	dump_nest(out,nest+3);
	out_printf(out,"#define SOMGD_%s%s\n",lead,op->id);

	dump_nest(out,nest+2);
	out_printf(out,"#else\n");

	get_c_name(op,n,sizeof(n));

	dump_nest(out,nest+3);
	out_printf(out,"#define %s%s %s\n",lead,op->id,n);

	dump_nest(out,nest+2);
	out_printf(out,"#endif\n");

	dump_nest(out,nest+1);
	out_printf(out,"#endif /* SOMGD_%s%s */\n",lead,op->id);

	dump_nest(out,nest);
	out_printf(out,"#endif /* SOM_DONT_USE_SHORT_NAMES */\n");
}

void RHBheader_emitter::generate_inherited_macros(RHBoutput *out,RHBelement_sequence *dups,RHBinterface *iface,RHBinterface *parent)
{
	if (parent)
	{
		unsigned long i=dups->length();

		while (i--)
		{
			if (parent==dups->get(i))
			{
				return;
			}
		}

		dups->add(parent);

		i=0;

		while (i < parent->_parents.length())
		{
			RHBelement *el=parent->_parents.get(i);

			generate_inherited_macros(out,dups,iface,el->is_interface());

			i++;
		}

		if (parent != iface)
		{
			if (iface)
			{
				RHBelement *el=parent->children();

				while (el)
				{
					RHBoperation *op=el->is_operation();

					if (op)
					{
						char buf[256];
						out_printf(out,"#define ");
						get_c_name(iface,buf,sizeof(buf));
						out->write(buf,strlen(buf));
						out->write("_",1);
						out->write(op->id,strlen(op->id));
						out->write(" ",1);

						get_c_name(parent,buf,sizeof(buf));
						out->write(buf,strlen(buf));
						out->write("_",1);
						out->write(op->id,strlen(op->id));
						out_printf(out,"\n");
					}

					el=el->next();
				}
			}
		}
	}
}

void RHBheader_emitter::generate_redispatch_stub_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[256];

	dump_nest(out,nest);
	out_printf(out,"static ");

	if (op->return_type)
	{
		RHBtype *as=get_array_slice(op->return_type);
		if (as)
		{
			use_type(out,as,0);
			out->write("* ",2);
		}
		else
		{
			use_type(out,op->return_type,0);
		}
	}
	else
	{
		out_printf(out,"void ");
	}

	get_c_name(op,n,sizeof(n));
	out_printf(out,"SOMLINK somRD_%s",n);
	generate_parameter_list(out,iface,op,2,1,0);
}

boolean RHBheader_emitter::type_is_sequence(RHBtype *t)
{
	if (t)
	{
		t=unwind_typedef(t);
		if (t->is_sequence()) return 1;
	}

	return 0;
}

boolean RHBheader_emitter::type_is_any(RHBtype *t)
{
	if (t)
	{
		char buf[256];
		t=unwind_typedef(t);
		get_c_name(t,buf,sizeof(buf));
		if (!strcmp(buf,"any")) return 1;
	}
	return 0;
}

void RHBheader_emitter::generate_redispatch_stub(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[256];
	RHBelement *el;

	/*  this has same signature as actual method,
			but folds it back onto somDispatch */

	{
		extern_c_proto_t extern_c_proto(this,out,nest);

		strncpy(n,"somRD_",sizeof(n));
		get_c_name(op,n+strlen(n),sizeof(n)-strlen(n));
		undoublebar(out,n);

		generate_redispatch_stub_proto(out,iface,op,nest);
		out_printf(out,";\n");
	}

	dump_nest(out,nest);
	out_printf(out,"#ifdef __IBMC__\n");
	
	dump_nest(out,nest+1);
	get_c_name(op,n,sizeof(n));
	out_printf(out,"#pragma linkage(somRD_%s,system)\n",n);

	dump_nest(out,nest);
	out_printf(out,"#endif /* __IBMC__ */\n");

	generate_redispatch_stub_proto(out,iface,op,nest);

	out_printf(out,"\n");
	dump_nest(out,nest);
	out_printf(out,"{\n");

	RHBtype *slice=NULL;

	if (operation_has_return(op))
	{
/*		i=sprintf(buf,"#ifdef _DEBUG\n");
		out->write(buf,i);

		dump_nest(out,nest+1);
		use_type(out,op->return_type,"__retVal");
		if (type_is_sequence(op->return_type))
		{
			i=sprintf(buf,"={0x80808080L,0x80808080L,(void *)0x80808080L}");
			out->write(buf,i);
		}
		else
		{
			if (type_is_any(op->return_type))
			{
				i=sprintf(buf,"={(TypeCode)0x80808080L,(void *)0x80808080L}");
				out->write(buf,i);
			}
		}
		i=sprintf(buf,";\n");
		out->write(buf,i);

		i=sprintf(buf,"#else\n");
		out->write(buf,i);*/

		dump_nest(out,nest+1);

		slice=get_array_slice(op->return_type);

		if (slice)
		{
			char retVal_assign[256];
			snprintf(retVal_assign,sizeof(retVal_assign),"* %s=NULL",retVal_name);
			use_type(out,slice,retVal_assign);
		}
		else
		{
			use_type(out,op->return_type,retVal_name);

			if (type_is_sequence(op->return_type))
			{
				out_printf(out,"={0,0,NULL}");
			}
			else
			{
				if (type_is_any(op->return_type))
				{
					out_printf(out,"={NULL,NULL}");
				}
			}
		}
		out_printf(out,";\n");

/*		i=sprintf(buf,"#endif\n");
		out->write(buf,i);*/
	}
	else
	{
		dump_nest(out,nest+1);
		out_printf(out,"somToken %s=NULL; /* actually returns a void */\n",retVal_name);
	}

	if (!slice)
	{
		if (!type_is_any(op->return_type))
		{
			if (!type_is_sequence(op->return_type))
			{
				generate_init_zero(out,retVal_name,op->return_type,nest,0);
			}
		}
	}

	dump_nest(out,nest+1);

	if (cplusplus)
	{
		out_printf(out,"somSelf->somDispatch(");
	}
	else
	{
		out_printf(out,"somva_SOMObject_somDispatch(somSelf,");
	}

	if (operation_has_return(op))
	{
		out_printf(out,"(somToken *)(void *)&%s,\n",retVal_name);
	}
	else
	{
		out_printf(out,"&%s,\n",retVal_name);
	}

	dump_nest(out,nest+2);
	out_printf(out,"(somId)somId_%s,somSelf",op->id);

	if (operation_needs_environment(iface,op))
	{
		out_printf(out,",ev");
	}
	if (operation_needs_context(iface,op))
	{
		out_printf(out,",ctx");
	}

	el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			out_printf(out,",\n");

			dump_nest(out,nest+3);

			if (!parameter_by_reference(param,NULL))
			{
				RHBtype *t=get_va_type(param->parameter_type,1);

				if (t != param->parameter_type)
				{
					out_printf(out,"(");
					use_type(out,t,0);
					out_printf(out,")");
				}
			}

			out_printf(out,"%s",param->id);
		}

		el=el->next();
	}
	
	out_printf(out,");\n");

	if (operation_has_return(op))
	{
		out_printf(out,"\n");
		dump_nest(out,nest+1);
		out_printf(out,"return %s;\n",retVal_name);
	}

	dump_nest(out,nest);
	out_printf(out,"}\n");
}

int RHBheader_emitter::align_for(long curr,int align)
{
	long dx;

	if (align < 2)
	{
		return 0;
	}

	curr+=align;

	dx=curr % align;

	if (dx)
	{
		return align-dx;
	}

	return 0;
}

void RHBheader_emitter::generate_apply_stub_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[256];

	dump_nest(out,nest);
	get_c_name(op,n,sizeof(n));
	out_printf(out,"static void SOMLINK somAP_%s",n);

	out_printf(out,"(\n");

	dump_nest(out,nest+1);
	use_type(out,iface,"somSelf");
	out_printf(out,",\n");

	dump_nest(out,nest+1);

	if (op->return_type)
	{
		RHBtype *slice=get_array_slice(op->return_type);
		if (slice)
		{
			use_type(out,slice,"*");
		}
		else
		{
			use_type(out,op->return_type,0);
		}
	}
	else
	{
		out_printf(out,"void ");

	}
	out_printf(out,"*%s,\n",retVal_name);

	dump_nest(out,nest+1);

#ifdef DO_FULL_TYPEDEF
	out_printf(out,"somTD_%s _somC_methodPtr,\n",n);
#else
	out_printf(out,"somMethodPtr _somC_methodPtr,\n");
#endif

	dump_nest(out,nest+1);
	out_printf(out,"va_list _somC_ap)");
}

void RHBheader_emitter::generate_apply_stub(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	/* apply stub has the form...

		void somAP_xxx(obj,void *__retVal,somMethodPtr __methodPtr,va_list __ap),
	
	    generated routine should pull each parameter
			from va_list __ap and 
			then call __methodPtr with same prototype as somTD_xxx
	*/

	RHBelement *el;
	char n[256];

	if (op_is_varg(op)) 
	{
		get_c_name(op,n,sizeof(n));
		dump_nest(out,nest);
		out_printf(out,"/* somAP_%s() has va_list, ignored */\n",n);
		return;
	}

	{
		extern_c_proto_t extern_c_proto(this,out,nest);

		strncpy(n,"somAP_",sizeof(n));
		get_c_name(op,n+strlen(n),sizeof(n)-strlen(n));
		undoublebar(out,n);

		generate_apply_stub_proto(out,iface,op,nest);
		out_printf(out,";\n");
	}

	dump_nest(out,nest);
	out_printf(out,"#ifdef __IBMC__\n");

	dump_nest(out,nest+1);
	get_c_name(op,n,sizeof(n));
	out_printf(out,"#pragma linkage(somAP_%s,system)\n",n);

	dump_nest(out,nest);
	out_printf(out,"#endif /* __IBMC__ */\n");

	generate_apply_stub_proto(out,iface,op,nest);

	out_printf(out,"\n");
	dump_nest(out,nest);
	out_printf(out,"{\n");

	if (operation_needs_environment(iface,op))
	{
		dump_nest(out,nest+1);
		out_printf(out,"Environment *ev;\n");
	}

	if (operation_needs_context(iface,op))
	{
		dump_nest(out,nest+1);
		if (cplusplus)
		{
			out_printf(out,"Context * ctx;\n");
		}
		else
		{
			out_printf(out,"Context SOMSTAR ctx;\n");
		}
	}

	el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			boolean by_ref=0;
			RHBtype *local_type=NULL;
			RHBtype *array_of=NULL;

			dump_nest(out,nest+1);

			by_ref=parameter_by_reference(param,&array_of);

			local_type=param->parameter_type;

			if (by_ref)
			{
				if (array_of) local_type=array_of;
			}
			else
			{
				RHBbase_type *x=unwind_typedef(local_type)->is_base_type();

				if (x)
				{
					if (x->_floating)
					{
						local_type=get_va_type(local_type,1);
					}
				}
			}

			if (get_array_slice(param->parameter_type))
			{
				char tdn[256];
				snprintf(tdn,sizeof(tdn),"_somC_slice_%s_t",param->id);
				generate_array_slice_typedef(out,0,param->parameter_type,tdn);
				dump_nest(out,nest+1);
				out_printf(out,"_somC_slice_%s_t *%s;\n",param->id,param->id);
			}
			else
			{
				use_type(out,local_type,0);

				if (by_ref)
				{
					out->write("*",1);
				}

				out_printf(out,"%s;\n",param->id);
			}
		}

		el=el->next();
	}

	out_printf(out,"\n");

	dump_nest(out,nest+1);
	out_printf(out,"somSelf=va_arg(_somC_ap,");
	use_type(out,iface,0);
	out_printf(out,");\n");

	if (operation_needs_environment(iface,op))
	{
		dump_nest(out,nest+1);
		out_printf(out,"ev=va_arg(_somC_ap,Environment *);\n");
	}

	if (operation_needs_context(iface,op))
	{
		dump_nest(out,nest+1);
		if (cplusplus)
		{
			out_printf(out,"ctx=va_arg(_somC_ap,Context *);\n");
		}
		else
		{
			out_printf(out,"ctx=va_arg(_somC_ap,Context SOMSTAR);\n");
		}
	}

	el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			RHBtype *type=NULL;

			dump_nest(out,nest+1);

			out_printf(out,"%s=",param->id);

			int by_ref=parameter_by_reference(param,&type);

			if (get_array_slice(param->parameter_type))
			{
				char tdn[256];
				snprintf(tdn,sizeof(tdn),"_somC_slice_%s_t",param->id);
				out_printf(out,"va_arg(_somC_ap,%s *)",tdn);
			}
			else
			{
				if (!type) type=param->parameter_type;

				use_va_arg(out,"_somC_ap",type,by_ref);
			}

			out_printf(out,";\n");
		}

		el=el->next();
	}

	out_printf(out,"\n");

	if (!operation_has_return(op))
	{
		dump_nest(out,nest+1);
		out_printf(out,"SOM_IgnoreWarning(%s);\n\n",retVal_name);
	}

	dump_nest(out,nest+1);

	if (operation_has_return(op))
	{
		out_printf(out,"*%s=",retVal_name);
	}

#ifdef DO_FULL_TYPEDEF
	out_printf(out,"_somC_methodPtr(somSelf");
#else
	get_c_name(op,n,sizeof(n));
	out_printf(out,"((somTD_%s)_somC_methodPtr)(somSelf",n);
#endif

	if (operation_needs_environment(iface,op))
	{
		out_printf(out,",ev");
	}

	if (operation_needs_context(iface,op))
	{
		out_printf(out,",ctx");
	}

	el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			boolean by_ref=parameter_by_reference(param,NULL);

			out_printf(out,",");

			if (!by_ref)
			{
				RHBbase_type *bt=unwind_typedef(param->parameter_type)->is_base_type();

				if (bt)
				{
					if (bt->_floating)
					{
						char nf[32];
						get_c_name(bt,nf,sizeof(nf));
	
						if (!strcmp(nf,"float"))
						{
							out_printf(out,"(%s)",nf);
						}
					}
				}
			}

			out_printf(out,"%s",param->id);
		}

		el=el->next();
	}

	out_printf(out,");\n");

	dump_nest(out,nest);
	out_printf(out,"}\n\n");

	generated_apply_stubs.add(op);
}

void RHBheader_emitter::use_va_arg(RHBoutput *out,const char *ap,RHBtype *typ,boolean byRef)
{
	/* added to cast so that shorts will be mapped from int etc */

	RHBtype *typ_cast=unwind_typedef(typ);
	if (!byRef)
	{
		if (typ_cast->is_qualified()||typ_cast->is_base_type())
		{
			out->write("(",1);
			if (byRef)
			{
				use_type(out,typ,0);
				out->write("*",1);
			}
			else
			{
				use_type(out,typ,0);
			}
			out->write(")",1);
		}
	}
	/* end of addition */

	out_printf(out,"va_arg(%s,",ap);

	if (byRef)
	{
		use_type(out,typ,0);
		out_printf(out,"*");
	}
	else
	{
		use_type(out,get_va_type(typ,1),0);
	}

	out_printf(out,")");
}

RHBtype *RHBheader_emitter::get_va_type(RHBtype *typ,boolean cast_float)
{
	RHBbase_type *bt;
	RHBtype_def *dt;
	RHBtype *orig;

	orig=typ;

	while (typ)
	{
		if (typ->is_interface())
		{
			break;
		}

		if (typ->is_pointer())
		{
			break;
		}

		dt=typ->is_typedef();

		if (dt)
		{
			typ=dt->alias_for;
		}
		else
		{
			bt=typ->is_base_type();

			if (bt)
			{
				if (bt->_va_type)
				{
					if (bt->_floating)
					{
						if (!cast_float)
						{
							return bt;
						}
					}

					return bt->_va_type;
				}

				return orig;
			}
			else
			{
				if (typ->is_qualified())
				{
					/* follow this */
					typ=typ->is_qualified()->base_type;
				}
				else
				{
					return orig;
				}
			}
		}
	}

	return orig;
}

void RHBheader_emitter::determine_inherited(RHBinterface *iface,RHBinterface *candidates,RHBelement_sequence *seq)
{
	if (candidates)
	{
		if (candidates != iface)
		{
			RHBelement *el=candidates->children();

			while (el)
			{
				RHBoperation *op;

				op=el->is_operation();

				if (op)
				{
					if (!seq->contains(op))
					{
/*						char buf[256];
						get_c_name(op,buf);
						printf("adding %s\n",buf);*/
						seq->add(op);
					}
				}

				el=el->next();
			}
		}

		unsigned long i=0;

		while (i < candidates->_parents.length())
		{
			RHBelement *el;

			el=candidates->_parents.get(i);

			determine_inherited(iface,el->is_interface(),seq);

			i++;
		}
	}
}

void RHBheader_emitter::determine_overrides(
				RHBinterface *iface,
				RHBinterface * /* candidates */,
				RHBelement_sequence *seq)
{
	RHBelement *el=iface->children();

	while (el)
	{
		RHBinherited_operation *iop=el->is_inherited_operation();

		if (iop)
		{
			if (does_override(iface,iop->id))
			{
				if (!seq->contains(iop->original))
				{
					seq->add(iop->original);
				}
			}
		}

		el=el->next();
	}

#if 0
	if (candidates)
	{
		if (candidates!=iface)
		{
			RHBelement *el=candidates->children();

			while (el)
			{
				int yes=0;
				RHBoperation *op=el->is_operation();

				if (op)
				{
					char buf[256];
					RHBelement *mod;

					strncpy(buf,op->id,sizeof(buf));
					mod=iface->find_named_element(0,buf,sizeof(buf));

					if (mod)
					{
						int i=0;

						while (mod->modifier_data.get(i))
						{
							const char *p=mod->modifier_data.get(i);
							if (0==strcmp(p,"override"))
							{
								yes=1;
								break;
							}

							i++;
						}
					}

					if (!yes)
					{
						strncpy(buf,"override",sizeof(buf));
						mod=iface->find_named_element(0,buf,sizeof(buf));
						if (mod)
						{
							int i;

							i=0;

							while (mod->modifier_data.get(i))
							{
								const char *p=mod->modifier_data.get(i);
								if (0==strcmp(p,op->id))
								{
									yes=1;
									break;
								}

								i++;
							}
						}
					}

					if (yes)
					{
						if (!seq->contains(op))
						{
							seq->add(op);
						}
					}
				}

				el=el->next();
			}
		}
	}

	if (iface)
	{
		if (candidates)
		{
			unsigned long i=0;

			while (i < candidates->_parents.length())
			{
				determine_overrides(iface,candidates->_parents.get(i)->is_interface(),seq);
	
				i++;
			}
		}
	}
#endif
}

void RHBheader_emitter::generate_operation_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[256];

/*	extern_c_begin(out,nest);*/

	/* SOM_Scope macro contains enough scoping */

	{
		extern_c_proto_t scoping(this,out,nest);

		get_function_prefix(iface,n,sizeof(n));
		strncat(n,op->id,sizeof(n)-1);
		undoublebar(out,n);

		dump_nest(out,nest);
		out_printf(out,"SOM_Scope ");

		if (op->return_type)
		{
			RHBtype *s=get_array_slice(op->return_type);
			if (s)
			{
				use_type(out,s,0);
				out_printf(out,"* ");
			}
			else
			{
				use_type(out,op->return_type,0);
			}
		}
		else
		{
			out_printf(out,"void ");
		}

		out_printf(out,"SOMLINK ");

		get_function_prefix(iface,n,sizeof(n));

		out_printf(out,"%s%s",n,op->id);

		generate_parameter_list(out,iface,op,0,1,0);
		out_printf(out,";\n");
	}

	dump_nest(out,nest);
	out_printf(out,"#ifdef __IBMC__\n");

	get_function_prefix(iface,n,sizeof(n));
	dump_nest(out,nest+1);
	out_printf(out,"#pragma linkage(%s%s,system)\n",n,op->id);
	dump_nest(out,nest);
	out_printf(out,"#endif /* __IBMC__ */\n");

/*	extern_c_end(out,nest);*/
}

void RHBheader_emitter::get_function_prefix(RHBinterface *iface,char *buf,size_t buflen)
{
	RHBelement *el;
	char n[256];

	strncpy(n,"functionprefix",sizeof(n));
	buf[0]=0;

	el=iface->find_named_element(0,n,sizeof(n));

	if (el)
	{
		const char *p=el->modifier_data.get(0);

		if (p)
		{
			while (*p)
			{
				if (*p!=double_quote)
				{
					if (!--buflen) break;

					*buf++ = *p;
				}
				p++;
			}

			*buf=0;
		}
	}
	else
	{
		/* 31-3-1999 */
/*		get_c_name(iface,buf);*/
		*buf=0;
	}
}

boolean RHBheader_emitter::get_actual_meta_class(RHBinterface *iface,char *buf,size_t buflen)
{
	if (get_meta_class(iface,buf)) 
	{
		RHBelement *el=iface->find_named_element(0,buf,buflen);

		if (el)
		{
			get_c_name(el,buf,buflen);

			return 1;
		}
	}

	strncpy(buf,"SOMClass",buflen);

	return 0;
}

RHBinterface *RHBheader_emitter::get_meta_class_interface(RHBinterface *iface)
{
	RHBelement *el;
	char n[256];

	if (!iface) return 0;

	strncpy(n,"metaclass",sizeof(n));

	el=iface->find_named_element(0,n,sizeof(n));

	if (el)
	{
		const char *p;
		char buffer[256];
		char *buf;

		buf=buffer;

		p=el->modifier_data.get(0);

		if (p)
		{
			while (*p)
			{
				if (*p!=double_quote)
				{
					*buf++ = *p;
				}
				p++;
			}

			*buf=0;

			el=iface->find_named_element(0,buffer,sizeof(buffer));

			if (el)
			{
				return el->is_interface();
			}
		}
	}

	unsigned long i=0;

	while (i < iface->_parents.length())
	{
		el=iface->_parents.get(i);

		RHBinterface *meta=get_meta_class_interface(el->is_interface());

		if (meta)
		{
			return meta;
		}

		i++;
	}

	return 0;
}

boolean RHBheader_emitter::get_meta_class(RHBinterface *iface,char *buf)
{
	RHBelement *el;
	char n[256];

	strncpy(n,"metaclass",sizeof(n));
	buf[0]=0;

	el=iface->find_named_element(0,n,sizeof(n));

	if (el)
	{
		const char *p=el->modifier_data.get(0);

		if (p)
		{
			while (*p)
			{
				if (*p!=double_quote)
				{
					*buf++ = *p;
				}
				p++;
			}

			*buf=0;

			return 1;
		}
	}

	unsigned long i=0;

	while (i < iface->_parents.length())
	{
		el=iface->_parents.get(i);

		if (get_meta_class(el->is_interface(),buf))
		{
			return 1;
		}

		i++;
	}

	return 0;
}


void RHBheader_emitter::generate_parent_macro(
			RHBoutput *out,
			RHBinterface *iface,
			RHBinterface *parent,
			RHBoperation *op,
			int nest)
{
	char buf[256];
	char n[256];

	get_c_name(op,buf,sizeof(buf));

	if ((!strcmp(buf,"SOMObject_somInit"))||
		(!strcmp(buf,"SOMObject_somUninit")))
	{
			get_c_name(iface,n,sizeof(n));
			strncat(n,"_parent_",sizeof(n)-1);
			get_c_name(parent,buf,sizeof(buf));
			strncat(n,buf,sizeof(n)-1);
			strncat(n,"_",sizeof(n)-1);
			strncat(n,op->id,sizeof(n)-1);

			dump_nest(out,nest);
			out_printf(out,"#ifndef %s\n",n);
			dump_nest(out,nest+1);
			out_printf(out,"#define %s(somSelf)\n",n);
			dump_nest(out,nest);
			out_printf(out,"#endif /* %s */\n",n);
	}
	else
	{
		int k=get_parent_index(iface,parent);

		if (is_dts_override(op))
		{
			dts_parent_macro(out,iface,k-1,parent,op,nest);

			k=0;
		}

		if (k)
		{
			dump_nest(out,nest);
			get_c_name(iface,n,sizeof(n));
			out_printf(out,"#define %s_parent_",n);
			get_c_name(parent,n,sizeof(n));
			out_printf(out,"%s_%s",n,op->id);
			generate_name_only_parameter_list(out,iface,op,0);
			out_printf(out,"   \\\n");

			dump_nest(out,nest+1);

			if (cplusplus)
			{
				get_c_name(iface,n,sizeof(n));
				strncat(n,"_",sizeof(n)-1);
				strncat(n,op->id,sizeof(n)-1);
			}
			else
			{
				get_c_name(op,n,sizeof(n));
			}

			if (is_kernel_class(iface))
			{
				get_function_prefix(parent,n,sizeof(n));
				out_printf(out,"%s%s",n,op->id);
			}
			else
			{
				out_printf(out,"((somTD_%s)\\\n",n);
				dump_nest(out,nest+2);
				if (generate_parent_resolved(out,iface,parent,op,3))
				{
					out_printf(out,")\\\n");
				}
				else
				{
					get_c_name(iface,n,sizeof(n));
					out_printf(out,"somParentNumResolve(%sCClassData.parentMtab,\\\n",n);
					dump_nest(out,nest+3);
					get_c_name(op->parent(),n,sizeof(n));
					out_printf(out,"%d,%sClassData.%s))\\\n",k,n,op->id);
				}
			}
			dump_nest(out,nest+4);
			generate_name_only_parameter_list(out,iface,op,cplusplus);
			out_printf(out,"\n");

            {
                dump_nest(out,nest);
                out_printf(out,"#ifndef SOM_DONT_USE_SHORT_NAMES\n");
                    dump_nest(out,nest+1);
                    out_printf(out,"#ifndef SOMGD_parent_%s\n",op->id);
                        dump_nest(out,nest+2);
                        out_printf(out,"#ifdef parent_%s\n",op->id);
                            dump_nest(out,nest+3);
                            out_printf(out,"#undef parent_%s\n",op->id);
                            dump_nest(out,nest+3);
                            out_printf(out,"#define SOMGD_parent_%s\n",op->id);
                        dump_nest(out,nest+2);
                        out_printf(out,"#else /* parent_%s */\n",op->id);
                            dump_nest(out,nest+3);
                            get_c_name(iface,n,sizeof(n));
                            out_printf(out,"#define parent_%s %s_parent_",op->id,n);
                            get_c_name(parent,n,sizeof(n));
                            out_printf(out,"%s_%s\n",n,op->id);
                        dump_nest(out,nest+2);
                        out_printf(out,"#endif /* parent_%s */\n",op->id);
                    dump_nest(out,nest+1);
                    out_printf(out,"#endif /* SOMGD_parent_%s */\n",op->id);
                dump_nest(out,nest);
                out_printf(out,"#endif /* SOM_DONT_USE_SHORT_NAMES */\n");
            }
		}
	}
}

int RHBheader_emitter::get_parent_index(RHBinterface *iface,RHBinterface *if2)
{
	unsigned long i=0;

	if (iface==if2) return 1;

	while (i < iface->_parents.length())
	{
		if (get_parent_index(iface->_parents.get(i)->is_interface(),if2))
		{
			return i+1;
		}

		i++;
	}

	return 0;
}

void RHBheader_emitter::generate_name_only_parameter_list(RHBoutput *out,RHBinterface *iface,RHBoperation *op,boolean expand_self)
{
	RHBelement *el;

	if (cplusplus && expand_self)
	{
		char n[256];
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"((%s *)(void *)somSelf",n);
	}
	else
	{
		out_printf(out,"(somSelf");
	}

	if (operation_needs_environment(iface,op))
	{
		out->write(",ev",3);
	}
	if (operation_needs_context(iface,op))
	{
		out->write(",ctx",4);
	}
	el=op->children();

	while (el)
	{
		RHBparameter *param;

		param=el->is_parameter();

		if (param)
		{
			out->write(",",1);
			out->write(param->id,strlen(param->id));
		}

		el=el->next();
	}

	out->write(")",1);
}


boolean RHBheader_emitter::has_same_ABI(RHBoperation *op1,RHBoperation *op2)
{
	int i=0;

	if (op1==op2) return 1;
	if (!op1) return 0;
	if (!op2) return 0;

	if (operation_needs_environment(0,op1)!=operation_needs_environment(0,op2))
	{
		return 0;
	}

	if (operation_needs_context(0,op1)!=operation_needs_context(0,op2))
	{
		return 0;
	}

	if (operation_has_return(op1)!=operation_has_return(op2))
	{
		return 0;
	}

	if (operation_has_return(op1))
	{
		if (!same_ABI_type(op1->return_type,op2->return_type))
		{
			return 0;
		}
	}

	while (1)
	{
		RHBtype *t1=0,*t2=0;
		RHBparameter *param1=op1->get_parameter(i);
		RHBparameter *param2=op2->get_parameter(i);

		i++;

		if ((!param1)&&(!param2))
		{
			break;
		}

		if (!param1) return 0;
		if (!param2) return 0;

		int ref1=parameter_by_reference(param1,NULL);
		int ref2=parameter_by_reference(param2,NULL);

		if (!ref1) 
		{
			t1=get_va_type(unwind_typedef(param1->parameter_type),0);
			ref1=some_ABI_kind_of_pointer(t1);
		}

		if (!ref2)
		{
			t2=get_va_type(unwind_typedef(param2->parameter_type),0);
			ref2=some_ABI_kind_of_pointer(t2);
		}

		if (ref1 && ref2)
		{
			continue;
		}

		if (ref1 || ref2)
		{
			return 0;
		}

		if (!same_ABI_type(t1,t2))
		{
			return 0;
		}
	}

	return 1;
}

boolean RHBheader_emitter::same_ABI_type(RHBtype *t1,RHBtype *t2)
{
	boolean ptr1,ptr2;

	t1=unwind_typedef(t1);
	t2=unwind_typedef(t2);

	if (t1==t2) return 1;

	ptr1=some_ABI_kind_of_pointer(t1);
	ptr2=some_ABI_kind_of_pointer(t2);

	if (ptr1 && ptr2) return 1;

	if (ptr1) return 0;
	if (ptr2) return 0;

	if (t1->is_sequence())
	{
		if (t2->is_sequence())
		{
			return 1;
		}
	}

	if (t1->is_any())
	{
		if (t2->is_any())
		{
			return 1;
		}
	}

	RHBbase_type *b1,*b2;

	b1=t1->is_base_type();
	b2=t2->is_base_type();

	if (b1 && b2)
	{
		if (b1->_floating != b2->_floating) return 0;
	}

	if (b1)
	{
		if (b1->_floating) 
		{
			return 0;
		}
	}

	if (b2)
	{
		if (b2->_floating) 
		{
			return 0;
		}
	}

/*
	won't do this check because this depends on this compiler
	knowing the alignments and sizes for the target platform,
	which we cant do if if we want to use the same headers on 
	Intel,PowerPC, DEC Alpha and AS400


	if (t1->_get_length()== t2->_get_length())
	{
		if (t1->_get_length())
		{
			if (t1->_get_alignment()==t2->_get_alignment())
			{
				if (t1->_get_alignment())
				{
					return 1;
				}
			}
		}
	}
*/
	return 0;
}

boolean RHBheader_emitter::some_ABI_kind_of_pointer(RHBtype *t)
{
	if (t->is_interface()) return 1;
	if (t->is_pointer()) return 1;
	if (t->is_string()) return 1;
	if (t->is_TypeCode()) return 1;

	return 0;
}

void RHBheader_emitter::generate_versions(RHBoutput *out,RHBinterface *iface,const char *ext,const char *mod_name,int nest)
{
	RHBelement *el;
	const char *p;
	char n[256];
	char buf[256];

	strncpy(buf,mod_name,sizeof(buf));
	el=iface->find_named_element(0,buf,sizeof(buf));

	p=0;

	if (el)
	{
		p=el->modifier_data.get(0);
	}

	if (!p) p="0";

	dump_nest(out,nest);
	get_c_name(iface,n,sizeof(n));

	out_printf(out,"#ifndef %s_%s\n",n,ext);
	dump_nest(out,nest+1);
	out_printf(out,"#define %s_%s   %s\n",n,ext,p);
	dump_nest(out,nest);
	out_printf(out,"#endif /* %s_%s */\n",n,ext);
}

boolean RHBheader_emitter::will_generate_static(const char *p)
{
	int i;

	i=0;

	while (generated_statics.get(i))
	{
		const char *q=generated_statics.get(i);
		if (0==strcmp(p,q))
		{
			return 0;
		}

		i++;
	}

	generated_statics.add(p);

	return 1;
}

boolean RHBheader_emitter::is_operation_noself(RHBoperation *op)
{
	const char *noself="noself";

	if (op)
	{
		int i=0;

		RHBelement *el=op->children();

		while (el)
		{
			if (!strcmp(el->id,noself))
			{
				RHBmodifier *mod=el->is_modifier();

				if (mod) return 1;
			}

			el=el->next();
		}

		while (i < op->modifier_data.length())
		{
			const char *p=op->modifier_data.get(i);

			if (!strcmp(p,noself))
			{
				return 1;
			}

			i++;
		}
	}

	return 0;
}

boolean RHBheader_emitter::is_operation_procedure(RHBoperation *op)
{
	const char *modname="procedure";

	if (op)
	{
		int i=0;

		RHBelement *el=op->children();

		while (el)
		{
			if (!strcmp(el->id,modname))
			{
				RHBmodifier *mod=el->is_modifier();

				if (mod) return 1;
			}

			el=el->next();
		}

		while (i < op->modifier_data.length())
		{
			const char *p=op->modifier_data.get(i);

			if (!strcmp(p,modname))
			{
				return 1;
			}

			i++;
		}
	}

	return 0;
}

#ifdef _PLATFORM_MACINTOSH_
void RHBheader_emitter::align_begin(RHBoutput *out,boolean user_data,int nest)
#else
void RHBheader_emitter::align_begin(RHBoutput *,boolean,int)
#endif
{
#ifdef _PLATFORM_MACINTOSH_
	char buf[256];
	int i;
	dump_nest(out,nest);
	i=sprintf(buf,"#ifdef _PLATFORM_MACINTOSH_\n");
	out->write(buf,i);

	dump_nest(out,nest+1);
	i=sprintf(buf,"#if powerc\n");
	out->write(buf,i);

	dump_nest(out,nest+2);
	if (user_data)
	{
		i=sprintf(buf,"#pragma options align=mac68k\n");
	}
	else
	{
		i=sprintf(buf,"#pragma options align=power\n");
	}

	out->write(buf,i);

	dump_nest(out,nest+1);
	i=sprintf(buf,"#endif /* powerc */\n");
	out->write(buf,i);

	dump_nest(out,nest);
	i=sprintf(buf,"#endif /* _PLATFORM_MACINTOSH_ */\n");
	out->write(buf,i);
#endif
}

#ifdef _PLATFORM_MACINTOSH_
void RHBheader_emitter::align_end(RHBoutput *out,boolean user_data,int nest)
#else
void RHBheader_emitter::align_end(RHBoutput *,boolean,int)
#endif
{
#ifdef _PLATFORM_MACINTOSH_
	char buf[256];
	int i;

	dump_nest(out,nest);
	i=sprintf(buf,"#ifdef _PLATFORM_MACINTOSH_\n");
	out->write(buf,i);

	dump_nest(out,nest+1);
	i=sprintf(buf,"#if powerc\n");
	out->write(buf,i);

	dump_nest(out,nest+2);
	i=sprintf(buf,"#pragma options align=reset\n");
	out->write(buf,i);

	dump_nest(out,nest+1);
	i=sprintf(buf,"#endif /* powerc */\n");
	out->write(buf,i);

	dump_nest(out,nest);
	i=sprintf(buf,"#endif /* _PLATFORM_MACINTOSH_ */\n");
	out->write(buf,i);
#endif
}

int RHBheader_emitter::is_unsigned(RHBtype *typ)
{
	while (typ->is_typedef())
	{
		typ=typ->is_typedef()->alias_for;
	}

	RHBqualified_type *q=typ->is_qualified();

	if (q)
	{
		if (!strcmp(q->id,"unsigned"))
		{
			return 1;
		}
	}

	if (typ->is_enum()) 
	{
		return 1;
	}

	return 0;
}

int RHBheader_emitter::is_long(RHBtype *t)
{
	while (t)
	{
		RHBtype_def *td=t->is_typedef();
		if (td)
		{
			t=td->alias_for;
		}
		else
		{
			RHBqualified_type *qt=t->is_qualified();

			if (qt)
			{
				t=qt->base_type;
			}
			else
			{
				RHBbase_type *bt=t->is_base_type();

				if (bt)
				{
					if (!strcmp(bt->id,"long"))
					{
						return 1;
					}
				}

				if (t->is_enum())
				{
					return 1;
				}

				return 0;
			}
		}
	}

	return 0;
}

void RHBheader_emitter::generate_constant(RHBoutput *out,RHBconstant *cn,int nest)
{
	char n[256];

	if (cn->is_const_TypeCode())
	{
		dump_nest(out,nest);
		get_c_name(cn,n,sizeof(n));
		out_printf(out,"/* TypeCode constant %s not emitted */\n",n);

		return;
	}

	dump_nest(out,nest);
	get_c_name(cn,n,sizeof(n));
	out_printf(out,"#ifndef %s\n",n);
	dump_nest(out,nest+1);
	out_printf(out,"#define %s   ",n);

	if (cn->is_numeric())
	{
		char extra[3]={0,0,0};
		char L=0;

		if (is_long(cn->constant_type))
		{
			L='L';
		}

		if (is_unsigned(cn->constant_type))
		{
			extra[0]='U';
			extra[1]=L;

			out_printf(out,"%lu%s\n",(unsigned long)cn->numeric_value(),extra);
		}
		else
		{
			extra[0]=L;
			out_printf(out,"%ld%s\n",(long)cn->numeric_value(),extra);
		}
	}
	else
	{
		if (cn->value_string)
		{
			out_printf(out,"\042%s\042\n",cn->value_string);
		}
		else
		{
			out_printf(out,"NULL\n");
		}
	}

	gen_shortname(out,cn,0,nest+1);

	dump_nest(out,nest);
	get_c_name(cn,n,sizeof(n));
	out_printf(out,"#endif /* %s */\n",n);
}

void RHBheader_emitter::generate_class_guard(RHBoutput *out,int nest,RHBinterface *iface,boolean how)
{
	if (internal)
	{
		char n[256];

		dump_nest(out,nest);

		get_c_name(iface,n,sizeof(n));

		if (how)
		{
			out_printf(out,"#ifdef %s_Class_Source\n",n);
		}
		else
		{
			out_printf(out,"#endif /* %s_Class_Source */\n",n);
		}
	}
}

void RHBheader_emitter::generate_passthru(RHBoutput *out,RHBinterface *iface,const char *which)
{
	int i=0;
	RHBelement *el;
	int j=0;

	el=0;
	while (iface->passthru_list.get(i))
	{
		el=iface->passthru_list.get(i);
		if (0==strcmp(which,el->id))
		{
			break;
		}

		i++;
		el=0;
	}

	if (!el) return;

	while (j < el->modifier_data.length())
	{
		const char *p=el->modifier_data.get(j);

		if (p)
		{
			if (*p==double_quote)
			{
				p++;
			}

			i=strlen(p);

			if (i)
			{
				if (p[i-1]==double_quote)
				{
					i--;
				}
			}

			out->write(p,i);
			out_printf(out,"\n");
		}

		j++;
	}

}

void RHBheader_emitter::generate_init_zero(
		RHBoutput *out,const char *name,RHBtype *typ,int nest,
		unsigned long /*initial_value*/)
{
/*	int i;
	char buf[256];*/
	char n[256];

	if (!typ) return;

	while (typ->is_typedef())
	{
		typ=typ->is_typedef()->alias_for;

		get_ir_name(typ,n,sizeof(n));

		if (!strcmp(n,"::SOMFOREIGN"))
		{
			return;
		}
	}

	while (typ->is_qualified())
	{
		typ=typ->is_qualified()->base_type;
	}

	get_c_name(typ,n,sizeof(n));

	if (!strcmp(n,"void")) return;

	out_printf(out,"#ifdef _DEBUG\n");

	dump_nest(out,nest+1);
	out_printf(out,"memset(&%s,0xCC,sizeof(%s));\n",
			name,name);

	out_printf(out,"#endif /* _DEBUG */\n");

#if 0
	if (!strcmp(n,"any"))
	{
		dump_nest(out,nest+1);
		i=sprintf(buf,"%s._type=(TypeCode)%ld;\n",name,initial_value);
		out->write(buf,i);
		dump_nest(out,nest+1);
		i=sprintf(buf,"%s._value=(void *)%ld;\n",name,initial_value);
		out->write(buf,i);
		typ=0;
	}

	if (!strcmp(n,"float"))
	{
		dump_nest(out,nest+1);
		i=sprintf(buf,"%s=(float)0.0;\n",name);
		out->write(buf,i);
		typ=0;
	}

	if (!strcmp(n,"double"))
	{
		dump_nest(out,nest+1);
		i=sprintf(buf,"%s=(double)0.0;\n",name);
		out->write(buf,i);
		typ=0;
	}

/*	if (!strcmp(n,"TypeCode"))
	{
		dump_nest(out,1);
		i=sprintf(buf,"%s=TC_void;\n",name);
		out->write(buf,i);
		typ=0;
	}
*/
	if (typ)
	{
		if (typ->is_sequence())
		{
			dump_nest(out,nest+1);
			i=sprintf(buf,"%s._length=%ld;\n",name,initial_value);
			out->write(buf,i);
			dump_nest(out,nest+1);
			i=sprintf(buf,"%s._maximum=%ld;\n",name,initial_value);
			out->write(buf,i);
			dump_nest(out,nest+1);
			i=sprintf(buf,"%s._buffer=%ld;\n",name,initial_value);
			out->write(buf,i);
			typ=0;
		}
	}

	if (typ)
	{
		if (typ->is_union() || typ->is_struct() || typ->is_array())
		{
			dump_nest(out,nest+1);
			i=sprintf(buf,"memset(&%s,%d,sizeof(%s));\n",name,(int)(unsigned char)initial_value,name);
			out->write(buf,i);
			typ=0;
		}
	}

	if (typ)
	{
/*		if (typ->is_interface())
		{
			dump_nest(out,nest+1);
			i=sprintf(buf,"%s=(%s SOMSTAR)%ld;\n",
						name,n,initial_value);
			out->write(buf,i);
		}
		else*/
		{
			if (!initial_value)
			{
				if (typ->is_base_type())
				{
					dump_nest(out,nest+1);
					i=sprintf(buf,"%s=%ld;\n",name,initial_value);
					out->write(buf,i);
					typ=0;
				}
			}
			
		    if (typ)
			{
				dump_nest(out,nest+1);
				i=sprintf(buf,"%s=(",name);
				out->write(buf,i);
				use_type(out,typ,0);
				i=sprintf(buf,")%ld;\n",initial_value);
				out->write(buf,i);
			}
		}
	}

	i=sprintf(buf,"\n");
	out->write(buf,i);

	i=sprintf(buf,"#endif /* _DEBUG */\n");
	out->write(buf,i);
#endif
}


boolean RHBheader_emitter::op_is_varg(RHBoperation *op)
{
	char buf[256];

	if (!op) return 0;

	RHBelement *el=op->children();
	
	RHBparameter *last_param=NULL;

	if (!el) return 0;

	while (el)
	{
		if (el->is_parameter())
		{
			last_param=el->is_parameter();
		}

		el=el->next();
	}

	if (!last_param) 
	{
		return 0;
	}

	if (strcmp(last_param->id,"ap"))
	{
		return 0;
	}

	get_c_name(last_param->parameter_type,buf,sizeof(buf));

	if (strcmp(buf,"va_list"))
	{
		return 0;
	}

/*	get_c_name(op,buf);

	printf("%s is varg op\n",buf);
*/
	return 1;
}

RHBelement *RHBheader_emitter::generate_va_proto(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest,int flag,const char *prefix)
{
	char n[256];
	RHBtype *t;
	RHBelement *el;
	RHBelement *penultimate=0;
	const char *linkage=flag ? "SOMLINK" : "";

	get_c_name(iface,n,sizeof(n));

	t=op->return_type;

	if (t)
	{
		use_type(out,t,0);
	}
	else
	{
		out_printf(out,"void ");
	}

	out_printf(out,"%s %s_%s_%s",linkage,prefix,n,op->id);

	get_c_name(iface,n,sizeof(n));
	out_printf(out,"(%s SOMSTAR somSelf,",n);

	if (operation_needs_environment(iface,op))
	{
		out_printf(out,"Environment *ev,");
	}

	if (operation_needs_context(iface,op))
	{
		out_printf(out,"Context SOMSTAR ctx,");
	}

	out_printf(out,"\n");

	el=op->children();

	while (el->next())
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			RHBelement *e2=NULL;
			RHBparameter *another=NULL;

			e2=param->next();

			while (e2)
			{
				if (e2->is_parameter())
				{
					another=e2->is_parameter();
				}

				e2=e2->next();
			}

			if (!another) break;

			penultimate=param;

			dump_nest(out,nest+2);

			use_type(out,param->parameter_type,0);

			if (parameter_by_reference(param,NULL))
			{
				out_printf(out,"*");
			}

			out_printf(out,"%s,\n",param->id);
		}

		el=el->next();
	}

	dump_nest(out,nest+2);
	out_printf(out,"...)");

	return penultimate;
}

void RHBheader_emitter::generate_va_stub(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest,int flags,const char *prefix)
{
	char n[256];
	RHBelement *el;
	RHBelement *penultimate;
	int do_macros=(flags & 2) ? 0 : 1;

	flags&=~2;

	if (!prefix)
	{
		prefix=flags ? "somva" : "va";
	}

	get_c_name(iface,n,sizeof(n));

	if (do_macros)
	{
		dump_nest(out,nest);
		out_printf(out,"#ifdef %s_VA_EXTERN\n",n);

		/* need to prototype the va function when it is not static... */

		dump_nest(out,nest+1);
		out_printf(out,"SOMEXTERN\n");

		if (iface->has_modifier_value("dllname"))
		{
			get_c_name(iface,n,sizeof(n));
dump_nest(out,nest+1);
			out_printf(out,"#ifdef %s_VA_STUBS\n",n);
dump_nest(out,nest+2);
			out_printf(out,"#ifdef SOMDLLEXPORT\n");
dump_nest(out,nest+3);
			out_printf(out,"SOMDLLEXPORT\n");
dump_nest(out,nest+2);
			out_printf(out,"#endif /* SOMDLLEXPORT */\n");
dump_nest(out,nest+1);
			out_printf(out,"#else\n");
			ifdef_import_export(out,iface,nest+2);
dump_nest(out,nest+1);
			out_printf(out,"#endif /* %s_VA_STUBS */\n",n);
		}

		dump_nest(out,nest+1);

		generate_va_proto(out,iface,op,nest,flags,prefix);
		
		out_printf(out,";\n");

		dump_nest(out,nest);
		out_printf(out,"#endif\n");
	}

	int final_endif=0;

	if (do_macros)
	{
		final_endif=1;
		dump_nest(out,nest);
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"#if (defined(%s_VA_STUBS)||!defined(%s_VA_EXTERN))\n",
				n,n);
	}

	if (do_macros)
	{
		dump_nest(out,nest+1);

		get_c_name(iface,n,sizeof(n));
		out_printf(out,"#ifdef %s_VA_EXTERN\n",n);

		dump_nest(out,nest+2);
		out_printf(out,"SOMEXTERN\n");

		dump_nest(out,nest+1);
		out_printf(out,"#else\n");
	}

	dump_nest(out,nest+2);
	out_printf(out,"static\n");

	if (do_macros)
	{
		dump_nest(out,nest+1);
		out_printf(out,"#endif\n");
	}

	dump_nest(out,nest+1);

	penultimate=generate_va_proto(out,iface,op,nest+1,flags,prefix);

	out_printf(out,"\n");

	get_c_name(iface,n,sizeof(n));

/*	if (do_macros)
	{
		get_c_name(iface,n,sizeof(n));
		dump_nest(out,nest);
		out_printf(out,"#if (defined(%s_VA_STUBS)||!defined(%s_VA_EXTERN))\n",
				n,n);
	}
*/
	dump_nest(out,nest+1);
	out_printf(out,"{\n");

	if (operation_has_return(op))
	{
		dump_nest(out,nest+2);
		use_type(out,op->return_type,"__result");
		out_printf(out,";\n");
	}

	dump_nest(out,nest+2);
	out_printf(out,"va_list ap;\n");

	dump_nest(out,nest+2);
	out_printf(out,"va_start(ap,%s);\n",penultimate->id);

	dump_nest(out,nest+2);
	if (operation_has_return(op))
	{
		out_printf(out,"__result=");
	}

/*	i=sprintf(buf,"((somTD_%s_%s)somResolve\n",n,op->id);
	out->write(buf,i);

	get_c_name(op->parent(),n);
	dump_nest(out,nest+2);
	i=sprintf(buf,"(somSelf,%sClassData.%s))\n",n,op->id);
	out->write(buf,i);
*/
	get_c_name(op->parent(),n,sizeof(n));
	out_printf(out,"SOM_Resolve(somSelf,%s,%s)\n",n,op->id);

	dump_nest(out,nest+4);
	out_printf(out,"(somSelf,");

	if (operation_needs_environment(iface,op))
	{
		out_printf(out,"ev,");
	}

	if (operation_needs_context(iface,op))
	{
		out_printf(out,"ctx,");
	}

	el=op->children();

	while (el->next())
	{
		if (el->is_parameter())
		{
			out_printf(out,"%s,",el->id);
		}

		if (el==penultimate) break;

		el=el->next();
	}

	out_printf(out,"ap);\n");

	dump_nest(out,nest+2);
	out_printf(out,"va_end(ap);\n");

	if (operation_has_return(op))
	{
		dump_nest(out,nest+2);
		out_printf(out,"return __result;\n");
	}

	dump_nest(out,nest+1);
	out_printf(out,"}\n");

/*	if (do_macros)
	{
		dump_nest(out,nest);
		out_printf(out,"#else\n");

		dump_nest(out,nest);
		out_printf(out,";\n");

		dump_nest(out,nest);
		out_printf(out,"#endif\n");
	}*/

	if (final_endif)
	{
		dump_nest(out,nest);
		out_printf(out,"#endif\n");
	}
}

void RHBheader_emitter::generate_inline_varg_operation(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	char n[256];
	RHBtype *t;
	RHBelement *el;
	RHBelement *ap=NULL;
	RHBelement *penult=NULL;

	const char *result_name="_somC_result";

	el=op->children();

	while (el)
	{
		if (el->is_parameter())
		{
			ap=el->is_parameter();
		}

		el=el->next();
	}

	el=op->children();

	while (el)
	{
		if (el->is_parameter())
		{
			if (el==ap) break;

			penult=el;
		}

		el=el->next();
	}

	get_c_name(op,n,sizeof(n));
/*	i=sprintf(buf,"\n",n);
	out->write(buf,i);*/

	dump_nest(out,nest);
	out_printf(out,"inline ");
	t=op->return_type;
	if (t)
	{
		use_type(out,t,0);
	}
	else
	{
		out_printf(out,"void ");
	}

	out_printf(out,"%s(",op->id);

	if (operation_needs_environment(iface,op))
	{
		out_printf(out,"Environment *ev,");
	}

	if (operation_needs_context(iface,op))
	{
		out_printf(out,"Context * ctx,");
	}

	out_printf(out,"\n");

	el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			dump_nest(out,nest+2);

			use_type(out,param->parameter_type,0);

			if (parameter_by_reference(param,NULL))
			{
				out_printf(out,"*");
			}

			out_printf(out,"%s,\n",param->id);

			if (param==penult) break;
		}

		el=el->next();
	}

	dump_nest(out,nest+2);
	out_printf(out,"...)\n");

	dump_nest(out,nest);
	out_printf(out,"{\n");

	if (operation_has_return(op))
	{
		dump_nest(out,nest+1);
		use_type(out,op->return_type,result_name);
		out_printf(out,";\n");
	}

	dump_nest(out,nest+1);
	out_printf(out,"va_list ap;\n");

	dump_nest(out,nest+1);
	out_printf(out,"va_start(ap,%s);\n",penult->id);

	dump_nest(out,nest+1);
	if (operation_has_return(op))
	{
		out_printf(out,"%s=",result_name);
	}

/*	get_c_name(iface,n);
	i=sprintf(buf,"((somTD_%s_%s)somResolve\n",n,op->id);
	out->write(buf,i);

	get_c_name(op->parent(),n);
	dump_nest(out,nest+2);
	i=sprintf(buf,"((SOMObject *)(void *)this,%sClassData.%s))\n",n,op->id);
	out->write(buf,i);
*/
	get_c_name(op->parent(),n,sizeof(n));
	out_printf(out,"SOM_Resolve(this,%s,%s)\n",n,op->id);

/*	get_c_name(iface,n);*/
	get_c_name(op->parent(),n,sizeof(n));
	dump_nest(out,nest+3);
	out_printf(out,"((%s *)(void *)this,",n);

	if (operation_needs_environment(iface,op))
	{
		out_printf(out,"ev,");
	}

	if (operation_needs_context(iface,op))
	{
		out_printf(out,"ctx,");
	}

	el=op->children();

	while (el)
	{
		if (el->is_parameter())
		{
			out_printf(out,"%s,",el->id);
			if (el==penult) break;
		}

		el=el->next();
	}

	out_printf(out,"ap);\n");

	dump_nest(out,nest+1);
	out_printf(out,"va_end(ap);\n");

	if (operation_has_return(op))
	{
		dump_nest(out,nest+1);
		out_printf(out,"return %s;\n",result_name);
	}

	dump_nest(out,nest);
	out_printf(out,"};\n");
}

void RHBheader_emitter::generate_inline_somNew_operation(RHBoutput *out,RHBinterface *iface,RHBoperation * /*op */,int nest)
{
	char n[256];
	RHBinterface *base=find_highest_common_parent(&iface->_metaclass_of);

	nest++;

	if (base)
	{
		get_c_name(base,n,sizeof(n));
	}
	else
	{
		strncpy(n,"SOMObject",sizeof(n));
	}

	dump_nest(out,nest);
	out_printf(out,"inline %s * somNew()\n",n);

	dump_nest(out,nest);
	out_printf(out,"{\n");

	dump_nest(out,nest+1);
	out_printf(out,"return (%s *)(void *)SOM_Resolve(this,SOMClass,somNew)\n",n);

	dump_nest(out,nest+2);
	out_printf(out,"((SOMClass *)(void *)this);\n");

	dump_nest(out,nest);
	out_printf(out,"}\n");
}

RHBinterface *RHBheader_emitter::find_highest_common_parent(RHBelement_sequence *seq)
{
	RHBinterface *ret=seq->get(0)->is_interface();
	size_t i=seq->length();

	while (i--)
	{
		RHBinterface *iface=seq->get(i)->is_interface();

		if (!iface->is_subclass_of(ret))
		{
			unsigned long j=0;

			while (j < iface->_parents.length())
			{
				i=0;

				ret=iface->_parents.get(j)->is_interface();

				while (i < seq->length())
				{
					i++;

					if (!seq->get(i)->is_interface()->is_subclass_of(ret))
					{
						ret=0;
						i=seq->length();
					}
				}

				j++;

				if (ret) j=iface->_parents.length();
			}

			i=seq->length();
		}
	}

	return ret;
}

boolean RHBheader_emitter::is_multiple_inherited(RHBinterface *iface)
{
	if (iface)
	{
		switch (iface->_parents.length())
		{
		case 0:
			return 0;
		case 1:
			return is_multiple_inherited(iface->_parents.get(0)->is_interface());
		default:
			return 1;
		}
	}

	return 0;
}

void RHBheader_emitter::generate_name_lookups(RHBoutput *out,RHBinterface *iface,int nest)
{
	RHBelement *e=iface->children();

	while (e)
	{
		RHBoperation *op=e->is_operation();

		if (op)
		{
			generate_name_lookup(out,iface,op,nest);
		}

		e=e->next();
	}

	if (cplusplus)
	{
		RHBelement_sequence seq;
		unsigned long i=0;

		determine_inherited(iface,iface,&seq);

		while (i < seq.length())
		{
			RHBoperation *op=seq.get(i)->is_operation();
			generate_name_lookup(out,iface,op,nest);
			i++;
		}
	}
}

void RHBheader_emitter::generate_name_lookup(RHBoutput *out,RHBinterface *iface,RHBoperation *op,int nest)
{
	if (op->has_modifier_value("namelookup"))
	{
		char n[256];

		dump_nest(out,nest);
		get_c_name(iface,n,sizeof(n));
		out_printf(out,"#ifndef %s_lookup_%s\n",
				n,op->id);

		dump_nest(out,nest+1);
		out_printf(out,"#define %s_lookup_%s",n,op->id);

		generate_name_only_parameter_list(out,iface,op,0);

		get_c_name(iface,n,sizeof(n));
		if (cplusplus)
		{
			out_printf(out,
				" ((somTD_%s_%s)somResolveByName((SOMObject *)(void *)somSelf,%c%s%c))",
				n,op->id,double_quote,op->id,double_quote);
		}
		else
		{
			out_printf(out,
				" ((somTD_%s_%s)somResolveByName(somSelf,%c%s%c))",
				n,op->id,double_quote,op->id,double_quote);
		}

		generate_name_only_parameter_list(out,iface,op,0);

		out_printf(out,"\n");
		
		dump_nest(out,nest+1);
		out_printf(out,"#ifndef lookup_%s\n",op->id);

		get_c_name(iface,n,sizeof(n));
		dump_nest(out,nest+2);
		out_printf(out,"#define lookup_%s %s_lookup_%s\n",
					op->id,n,op->id);

		dump_nest(out,nest+1);
		out_printf(out,"#endif /* lookup_%s */\n",op->id);

		dump_nest(out,nest);
		out_printf(out,"#endif /* %s_lookup_%s */\n",n,op->id);
	}
}

void RHBheader_emitter::gen_shortname_sequence(RHBoutput *out,RHBtype *type,int nest)
{
	char n[256];

	get_c_name(type,n,sizeof(n));

	if (!type->id) return;
	if (!n[0]) return;

	if (!strcmp(n,type->id))
	{
		return;
	}

	dump_nest(out,nest);
	out_printf(out,"#ifndef SOM_DONT_USE_SHORT_NAMES\n");

	dump_nest(out,nest+1);
	out_printf(out,"#ifndef _IDL_SEQUENCE_%s_defined\n",type->id);

	dump_nest(out,nest+2);
	out_printf(out,"#define _IDL_SEQUENCE_%s_defined\n",type->id);


	dump_nest(out,nest+2);
	out_printf(out,"#define _IDL_SEQUENCE_%s _IDL_SEQUENCE_%s\n",
				type->id,n);

	dump_nest(out,nest+1);
	out_printf(out,"#endif /* _IDL_SEQUENCE_%s_defined */\n",type->id);

	dump_nest(out,nest);
	out_printf(out,"#endif /* SOM_DONT_USE_SHORT_NAMES */\n");
}

int RHBheader_emitter::get_cif(RHBinterface *iface,char *buf,size_t buflen)
{
	RHBelement *el;
	char n[256];

	strncpy(n,"classinit",sizeof(n));
	buf[0]=0;

	el=iface->find_named_element(0,n,sizeof(n));

	if (el)
	{
		const char *p=el->modifier_data.get(0);
		char cif[256]={0};
		char *q=cif;

		if (p)
		{
			while (*p)
			{
				if (*p!=double_quote)
				{
					*q++ = *p;
				}
				p++;
			}

			*q=0;
		}

		get_function_prefix(iface,buf,buflen);
		strncat(buf,cif,buflen);

		return 1;
	}

	*buf=0;

	return 0;
}

boolean RHBheader_emitter::is_derived_from(RHBinterface *iface,RHBinterface *base)
{
	if (base)
	{
		unsigned long i=0;

		if (base==iface) return 1;

		while (i < iface->_parents.length())
		{
			if (is_derived_from(iface->_parents.get(i)->is_interface(),base))
			{
				return 1;
			}

			i++;
		}
	}

	return 0;
}

int RHBheader_emitter::generate_parent_resolved(RHBoutput *out,
												RHBinterface *iface,
												RHBinterface *parent,
												RHBoperation *op,
												int mode)
{
	int retVal=0;

	if (iface 
		&& parent 
		&& op 
		&& 
		strcmp(op->id,"somInit") && 
		strcmp(op->id,"somUninit") &&
		strcmp(op->id,"somDestruct"))
	{
		if (!op->has_modifier_value("init"))
		{
			if (does_override(iface,op->id))
			{
				if (is_derived_from(parent,op->parent()->is_interface()))
				{
					retVal=get_parent_index(iface,parent);

					if (retVal)
					{
						char tokenName[1024];
						char name[256],parent_name[256];

						get_c_name(iface,name,sizeof(name));
						get_c_name(parent,parent_name,sizeof(parent_name));

						snprintf(tokenName,sizeof(tokenName),"_somC_%s_parent_%s_%s_resolved",
									name,parent_name,op->id);

						switch (mode)
						{
						case 1:
							out_printf(out,"static somMethodPtr %s; /* %d */\n",tokenName,retVal);
							break;
						case 2:
							get_c_name(op->parent(),parent_name,sizeof(parent_name));
							out_printf(out,"\t%s=somParentNumResolve(%sCClassData.parentMtab,%d,%sClassData.%s);\n",
									tokenName,
									name,
									retVal,
									parent_name,
									op->id);
							break;
						case 3:
							out_printf(out,"%s",tokenName);
							break;
						default:
							retVal=0;
							break;
						}
					}
				}
			}
		}
	}

	return retVal;
}

int RHBheader_emitter::is_kernel_class(RHBinterface *k)
{
	char buf[256];
	get_c_name(k,buf,sizeof(buf));
	if (strcmp(buf,"SOMObject")&&
		strcmp(buf,"SOMClass")&&
		strcmp(buf,"SOMClassMgr"))
	{
		return 0;
	}
	return 1;
}

int RHBheader_emitter::count_ClassData(RHBinterface *p)
{
	int count=0;

	if (p)
	{
		if (p->classData)
		{
			RHBelement *el=p->classData->children();

			while (el)
			{
				if (strcmp(el->id,"classObject"))
				{
					count++;
				}

				el=el->next();
			}
		}
	}

	return count;
}

int RHBheader_emitter::get_init_operations(RHBinterface *iface,RHBelement_sequence *seq)
{
	RHBelement *el=iface->classData->children();

	while (el)
	{
		RHBelement *e2=iface->children();

		while (e2)
		{
			if (!strcmp(e2->id,el->id))
			{
				RHBoperation *op=e2->is_operation();

				if (op)
				{
					if (op->has_modifier_value("init"))
					{
						seq->add(op);
					}
				}
			}

			e2=e2->next();
		}

		el=el->next();
	}

	return seq->length();
}

int RHBheader_emitter::get_va_operations(RHBinterface *iface,RHBelement_sequence *seq)
{
	char buf[256];

	strncpy(buf,"releaseorder",sizeof(buf));

	RHBelement *e=iface->find_named_element(0,buf,sizeof(buf));

	if (e)
	{
		int i=0;

		while (e->modifier_data.get(i))
		{
			const char *p=e->modifier_data.get(i);
			i++;

			if (strcmp(p,","))
			{
				RHBelement *oe;

				strncpy(buf,p,sizeof(buf));

				oe=iface->find_named_element(0,buf,sizeof(buf));

				if (oe) 
				{
					RHBoperation *op=oe->is_operation();

					if (op && op_is_varg(op))
					{
						seq->add(op);
					}
				}
			}
		}
	}

	return seq->length();
}

typedef enum
{
	dts_destructor,dts_constructor,dts_assignment
} dts_override_type;

struct dts_override
{
	dts_override_type dts_type;
	const char *name;
	const char *initType;
	const char *member;
	const char *structInfo;
	const char *macro;
	const char *mtab;
	const char *structCtrl;
};

static struct dts_override dts_overrides[]={
	{dts_destructor,
		"somDestruct","Destructor","defaultDestruct",
		"somDestructInfo","DeInit","destructCtrl","somDestructCtrl"},
	{dts_constructor,
		"somDefaultInit","Initializer","defaultInit",
		"somInitInfo","Init","initCtrl","somInitCtrl"},
	{dts_assignment,
		"somDefaultAssign","AssignmentOp","defaultNCArgAssign",
		"somAssignInfo","Assign","assignCtrl","somAssignCtrl"},
	{dts_assignment,
		"somDefaultConstAssign","AssignmentOp","defaultConstAssign",
		"somAssignInfo","Assign","assignCtrl","somAssignCtrl"}
};

struct dts_override * RHBheader_emitter::is_dts_override(RHBoperation *op)
{
	int i=sizeof(dts_overrides)/sizeof(dts_overrides[0]);
	struct dts_override *s=dts_overrides;
	while (i--)
	{
		if (!strcmp(s->name,op->id)) return s;
		s++;
	}
	return 0;
}

void RHBheader_emitter::write_macro_args(RHBoutput *out,RHBoperation *op,int include_somSelf)
{	
	if (include_somSelf) 
	{
		out_printf(out,"(somSelf");
	}

	if (operation_needs_environment(op->parent()->is_interface(),op))
	{
		out_printf(out,",ev");
	}

	if (operation_needs_context(op->parent()->is_interface(),op))
	{
		out_printf(out,",ctx");
	}

	RHBelement *el=op->children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			out_printf(out,",%s",param->id);
		}

		el=el->next();
	}

	out_printf(out,")");
}

void RHBheader_emitter::dts_parent_macro(RHBoutput *out,RHBinterface *iface,int num,RHBinterface *parent,RHBoperation *op,int /* nest */)
{
	struct dts_override *dts=is_dts_override(op);
	char n[256],m[256],o[256],objcls[256];
	int parent_is_SOMObject;
	const char *somstar=cplusplus ? "*" : "SOMSTAR";

	get_c_name(iface,n,sizeof(n));
	get_c_name(parent,m,sizeof(m));
	get_c_name(op,o,sizeof(o));
	get_c_name(op->parent(),objcls,sizeof(objcls));

	parent_is_SOMObject=strcmp(m,"SOMObject") ? 0 : 1;

#if 0
	if (parent_is_SOMObject)
	{
		out_printf(out,"/* special case SOMObject */ \\\n");
	}
#endif

	{
		int parent_count=iface->_parents.length();
		out_printf(out,"#define %s_%s_%s_%s",n,dts->macro,m,op->id);
		write_macro_args(out,op,1);
		out_printf(out,"\\\n");

		if (dts->dts_type==dts_destructor)
		{
			/* destructor in reverse order */
			out_printf(out,"if (myMask[%d]&1) {\\\n",(parent_count-num-1));
		}
		else
		{
			out_printf(out,"if (myMask[%d]&1) {\\\n",num);
		}

#ifdef _DEBUG
		out_printf(out,
			"SOM_Assert(ctrl->info->cls==%sClassData.classObject,SOM_Fatal); \\\n",
			m);
#endif

		if (parent_is_SOMObject)
		{
			out_printf(out,"ctrl->info=(%s *)(ctrl->infoSize+(char *)ctrl->info); \\\n",dts->structInfo);
		}
		else
		{
			out_printf(out,"((somTD_%s)ctrl->info->%s) \\\n",
						o,dts->member);

			if (cplusplus)
			{
				out_printf(out,"(((%s %s)(void *)somSelf)",objcls,somstar);
			}
			else
			{
				out_printf(out,"(somSelf");
			}

			write_macro_args(out,op,0);
			out_printf(out,"; \\\n");
		}
		out_printf(out,"}\n");
	}
}

static void union_of_dts_ops(RHBheader_emitter *emitter,RHBinterface *iface,RHBelement_sequence *seq)
{
	int i=iface->_parents.length();
	while (i--)
	{
		union_of_dts_ops(emitter,iface->_parents.get(i)->is_interface(),seq);
	}
	RHBelement *el=iface->children();
	while (el)
	{
		RHBoperation *op=el->is_operation();

		if (op)
		{
			if (emitter->is_dts_override(op))
			{
				i=seq->length();
				while (i--)
				{
					if (seq->get(i)==op)
					{
						break;
					}
				}
				if (i < 0)
				{
					seq->add(op);
				}
			}
		}

		el=el->next();
	}
}

void RHBheader_emitter::generate_dts_macros(RHBoutput *out,RHBinterface *iface)
{
RHBelement_sequence seq;
const int num_parents=iface->_parents.length();
int done_destructor=0,done_assignment=0;

	union_of_dts_ops(this,iface,&seq);

	int it=seq.length();

	while (it--)
	{
		RHBoperation *op=seq.get(it)->is_operation();

		if (op)
		{
			if (does_override(iface,op->id))
			{
				struct dts_override *dts=is_dts_override(op);

				if (dts)
				{
					switch (dts->dts_type)
					{
					case dts_destructor:
						if (done_destructor)
						{
							dts=NULL;
						}
						else
						{
							done_destructor=1;
						}
						break;
					case dts_assignment:
						if (done_assignment)
						{
							dts=NULL;
						}
						else
						{
							done_assignment=1;
						}
						break;
					default:
						break;
					}
				}

				if (dts)
				{
					out_printf(out,"/* special macros for %s */\n",op->id);
					char n[256];
					char macro_name[1024]={0};

					get_c_name(iface,n,sizeof(n));

					switch (dts->dts_type)
					{
					case dts_destructor:
					case dts_assignment:
						snprintf(macro_name,sizeof(macro_name),"%s_Begin%s",
								n,
								dts->initType);
						break;
					default:
						snprintf(macro_name,sizeof(macro_name),"%s_Begin%s_%s",
								n,
								dts->initType,op->id);
						break;
					}

					out_printf(out,"#ifndef %s\n",macro_name); 
					out_printf(out,"#define %s \\\n",macro_name); 


					{
						out_printf(out,"if (!ctrl) { \\\n"); 
						out_printf(out,"ctrl=&globalCtrl; \\\n"); 
						out_printf(out,
							"memcpy(ctrl,&(((somParentMtabStructPtr)(void *)%sCClassData.parentMtab)->%s),sizeof(%s)); \\\n",
							n,dts->mtab,dts->structCtrl);
						out_printf(out,"} \\\n"); 
#ifdef _DEBUG
						if (dts->dts_type==dts_destructor)
						{
/*							i=sprintf(buf,"somPrintf(\"%s_BeginDestructor\\n\"); \\\n",n);
							out->write(buf,i);*/
						}
						out_printf(out,"SOM_Assert(ctrl->info->cls==%sClassData.classObject,SOM_Fatal); \\\n",n);
#endif
						if (iface->instanceData && iface->instanceData->children())
						{
							out_printf(out,"somThis=(%sData*)(((char *)somSelf)+ctrl->info->dataOffset); \\\n",n);
#ifdef _DEBUG
							out_printf(out,"SOM_Assert(somThis==%sGetData(somSelf),SOM_Fatal); \\\n",n);
#endif
						}

						if (dts->dts_type==dts_destructor)
						{
							out_printf(out,"myMask=ctrl->mask; \n");
						}
						else
						{
							out_printf(out,"myMask=ctrl->mask; \\\n");

							if (num_parents)
							{
								out_printf(out,"ctrl->mask+=%d; \\\n",num_parents);
							}

							out_printf(out,"ctrl->info=(%s *)(ctrl->infoSize+(char*)(ctrl->info));\n",
									dts->structInfo);
						}
					}

					if (dts->dts_type==dts_destructor)
					{
						out_printf(out,"#define %s_EndDestructor \\\n",n);
#ifdef _DEBUG
/*						i=sprintf(buf,"somPrintf(\"%s_EndDestructor\\n\"); \\\n",n);
						out->write(buf,i);*/
#endif
						out_printf(out,"ctrl->info=(%s *)(ctrl->infoSize+(char *)ctrl->info); \\\n",
								dts->structInfo);
						if (num_parents)
						{
							out_printf(out,"ctrl->mask+=%d; \\\n",num_parents);
						}

						int k=num_parents;
						while (k--)
						{
							RHBinterface *parent=iface->_parents.get(k)->is_interface();
							char m[256];
							get_c_name(parent,m,sizeof(m));
							out_printf(out,"%s_%s_%s_%s(somSelf,0,ctrl); \\\n",n,dts->macro,m,op->id);
						}

						if (cplusplus)
						{
							out_printf(out,
								"if (doFree) (*((somMethodTabPtr *)(void *)somSelf))->classObject->somDeallocate((somToken)(void *)somSelf);\n");
						}
						else
						{
							out_printf(out,
								"if (doFree) SOMClass_somDeallocate(somSelf->mtab->classObject,somSelf);\n");
						}
					}

					out_printf(out,"#endif /* %s */\n",macro_name); 

				}
			}
		}
	}
}

void RHBheader_emitter::undoublebar(RHBoutput *out,const char *n)
{
	if (cplusplus)
	{
		int x=strlen(n);
		int do_dd=0;

		if (x > 2)
		{
			const char *p=n;

			x-=2;

			while (x--)
			{
				if ((*p++)=='_')
				{
					if (*p=='_')
					{
						do_dd=1;

						break;
					}
				}
			}
		}

		if (do_dd)
		{
			char buf[1024];
			char *p=buf;
			const char *q=n;

			while (*q)
			{
				if (*q=='_')
				{
					*p++=*q++;
					if (*q=='_')
					{
						*p++='s';
						*p++='o';
						*p++='m';
						*p++='C';
					}
				}
				else
				{
					*p++=*q++;
				}
			}

			*p=0;

			if (strcmp(buf,n))
			{
				const char *gm="SOM_AVOID_DOUBLE_UNDERSCORES";
				out_printf(out,"#ifdef %s\n",gm);
				out_printf(out,"\t#ifdef %s\n",n);
				out_printf(out,"\t\t#error %s already defined\n",n);
				out_printf(out,"\t#else /* %s */\n",n);
				out_printf(out,"\t\t#define %s %s\n",n,buf);
				out_printf(out,"\t#endif /* %s */\n",n);
				out_printf(out,"#endif /* %s */\n",gm);
			}
		}
	}
}

boolean RHBheader_emitter::do_const_param(RHBparameter *param)
{
	if (cplusplus && param)
	{
#if 0
		if (strcmp(param->mode,"in")) return 0;

		RHBtype *typ=unwind_typedef(param->parameter_type);
		if (!typ) return 0;

		typ=unwind_typedef(typ);

		if (typ->is_pointer()) return 0;
		if (typ->is_array()) return 0;

		if (parameter_by_reference(param,NULL)) return 1;
		if (typ->is_struct()) return 1;
		if (typ->is_union()) return 1;
		if (some_ABI_kind_of_pointer(typ)) return 1;
		if (type_is_any(typ)) return 1;
#endif
	}

	return 0;
}

