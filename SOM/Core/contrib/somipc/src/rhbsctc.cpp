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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rhbsc.h>


static const char szSOMTC_Scope_[]=
	"#ifdef SOMTC_Scope\n"
	"SOMTC_Scope\n"
	"#else\n"
	"static\n"
	"#endif\n"
	;

static int count_elements(RHBemitter *em,RHBtype *type)
{
	int j=0;

	if (type)
	{
		RHBtype_def *td=type->is_typedef();

		while (td)
		{
			if (em->is_SOMFOREIGN(td)) return 3;

			td=td->alias_for->is_typedef();
		}

		if (type->is_sequence()) 
		{
			/* count and type */
			return 2;
		}
		if (type->is_array()) 
		{
			return 1;
		}
		if (type->is_enum())
		{
			j=1;
			RHBelement *el=type->children();
			while (el)
			{
				j++;
				el=el->next();
			}
		}
		if (type->is_struct() || type->is_exception())
		{
			j=1;
			RHBelement *el=type->children();
			while (el)
			{
				j+=2;
				el=el->next();
			}
		}
		if (type->is_interface())
		{
			char n[256];
			em->get_c_name(type,n,sizeof(n));
			if (!strcmp(n,"Principal")) return 0;
			return 1;
		}
		if (type->is_string())
		{
			return 1;
		}
	}

	return j;
}

boolean RHBtc_emitter::generate(RHBoutput *out,const char *f)
{
	printf("tc generator: '%s'\n",f);
	idl_name=f;
	build_initial_list(GetRepository());

#if 0
	size_t i=0;
	while (i < extern_data.length())
	{
		build_secondary_list(extern_data.get(i));
		i++;
	}

	i=0;
	while (i < static_data.length())
	{
		build_secondary_list(static_data.get(i));
		i++;
	}
	generate_externs(out);

	if (useAlignmentHelper())
	{
		generate_aligns(out,&extern_data);
		generate_aligns(out,&static_data);
		{
			const char *p="AlignmentHelper(GENERIC_SEQUENCE)\n";
			out->write(p,strlen(p));
		}
	}

	generate_strings(out);
	
	generate_tcs(out,&static_data);
#endif

	generate_tcs(out,&extern_data);

	return 1;
}

RHBtc_emitter::RHBtc_emitter(RHBrepository *rr) :
RHBemitter(rr)
{
}

RHBtc_emitter::~RHBtc_emitter()
{
}

void RHBtc_emitter::build_initial_list(RHBelement *el)
{
	RHBelement *e=el->children();

	while (e)
	{
		if (ShouldEmit(e))
		{
			char buf[256];
			RHBconstant *con;

			get_ir_name(e,buf,sizeof(buf));
/*			printf("%s\n",buf);*/

			con=e->is_constant();

			if (con)
			{
				if (con->is_const_TypeCode())
				{
					if (!extern_data.contains(con))
					{
						extern_data.add(con);
					}
				}
			}

			build_initial_list(e);
		}

		e=e->next();
	}
}

void RHBtc_emitter::generate_externs(RHBoutput *out)
{
	char n[256];
	size_t j=0;

	while (j < extern_data.length())
	{
		RHBconstant *c=extern_data.get(j)->is_constant();

		if (c->is_private)
		{
/*			__asm int 3;*/
			out_printf(out,"/* %s is private */\n",n);
		}
		else
		{
/*			i=sprintf(buf,"extern \n");
			out->write(buf,i);
			i=sprintf(buf,"#ifdef _WIN32\n");
			out->write(buf,i);
			i=sprintf(buf,"\t__declspec(dllexport)\n");
			out->write(buf,i);
			i=sprintf(buf,"#endif\n");
			out->write(buf,i);
*/

			out_printf(out,szSOMTC_Scope_);

			out_printf(out,"\t%s SOMDLINK ",struct_type_for_type(c->const_typecode_val));

			get_c_name(extern_data.get(j),n,sizeof(n));

			out_printf(out,"\t%s;\n",n);
			j++;
		}
	}

	out_printf(out,"#if PRAGMA_IMPORT_SUPPORTED\n");

	j=0;

	while (j < extern_data.length())
	{
		RHBconstant *c=extern_data.get(j)->is_constant();
		if (!c->is_private)
		{
			get_c_name(extern_data.get(j),n,sizeof(n));
			out_printf(out,"#pragma export list %s\n",n);
		}
		j++;
	}

	out_printf(out,"#endif /* PRAGMA_IMPORT_SUPPORTED */\n");
}

void RHBtc_emitter::generate_aligns(RHBoutput *out,RHBelement_sequence *seq)
{
	char n[256];
	size_t j=0;

	while (j < seq->length())
	{
		RHBelement *el=seq->get(j);
		RHBconstant *c=el->is_constant();
		RHBtype *val=0;

		if (c)
		{
			val=c->const_typecode_val;
		}

		if (!val)
		{
			get_c_name(el,n,sizeof(n));
			out_printf(out,"/* %s is not a TC constant */\n",n);
		}

		if (val)
		{
			get_c_name(val,n,sizeof(n));

			if (val->is_interface())
			{
				/* use a somToken for this */
#if 0
				out_printf(out,"#ifndef %s_SOMSTAR_defined\n",n);
				out_printf(out,"#define %s_SOMSTAR_defined\n",n);

				out_printf(out,"typedef %s SOMSTAR %s_SOMSTAR;\n",n,n);

				out_printf(out,"#endif /* %s_SOMSTAR_defined */\n",n); 

				strncat(n,"_SOMSTAR",sizeof(n));
#endif
				strncpy(n,"somToken",sizeof(n));
			}
			else
			{
				RHBqualified_type *qual=val->is_qualified();

				if (qual)
				{
					char *p=n+strlen(n);
					*p++=' ';
					get_c_name(qual->base_type,p,sizeof(n)-strlen(n));
				}
			}

			if (!val->is_sequence())
			{
				if (!val->is_array())
				{
					if (strcmp(n,"void"))
					{
						out_printf(out,"AlignmentHelper(%s)\n",n);
					}
				}
				else
				{
					out_printf(out,"/* %s is an array */\n",n);
				}
			}
			else
			{
				out_printf(out,"/* %s is a sequence */\n",n);
			}
		}

		j++;
	}
}

RHBconstant *RHBtc_emitter::find_constant_for(RHBtype *type,RHBelement_sequence *seq)
{
	size_t i=0;

	while (i < seq->length())
	{
		RHBelement *el=seq->get(i);
		RHBconstant *con=el->is_constant();
		if (con)
		{
			if (con->is_const_TypeCode())
			{
				char b1[256],b2[256];

				b1[0]=0; b2[0]=0;

				if (con->const_typecode_val)
				{
					get_c_name(con->const_typecode_val,b1,sizeof(b1));
				}

				if (type) 
				{
					get_c_name(type,b2,sizeof(b2));
				}

				if (con->const_typecode_val==type)
				{
					return con;
				}

				if (!strcmp(b1,b2))
				{
					RHBsequence_type *seq1;
					RHBsequence_type *seq2;

					seq1=con->const_typecode_val->is_sequence();
					seq2=type->is_sequence();

					if (seq1 && seq2)
					{
						if (unwind_typedef(seq1->sequence_of) == unwind_typedef(seq2->sequence_of))
						{
							return con;
						}
					}

					RHBarray *ar1;
					RHBarray *ar2;

					ar1=con->const_typecode_val->is_array();
					ar2=type->is_array();

					if (ar1 && ar2)
					{
						if (unwind_typedef(ar1->array_of) == unwind_typedef(ar2->array_of))
						{
							return con;
						}
					}

					RHBqualified_type *q1;
					RHBqualified_type *q2;

					q1=con->const_typecode_val->is_qualified();
					q2=type->is_qualified();

					if (q1 && 12)
					{
						if (q1->base_type==q2->base_type)
						{
							return con;
						}
					}

					printf("%s!=%s\n",b1,b2);
				}
			}
		}
		i++;
	}

	return 0;
}

void RHBtc_emitter::build_secondary_list(RHBelement *el)
{
	RHBelement *st=0;
	RHBconstant *con=el->is_constant();

	if (con)
	{
		if (con->is_const_TypeCode())
		{
			if (con->const_typecode_val)
			{
				st=con->const_typecode_val->is_struct();

				if (!st)
				{
					st=con->const_typecode_val->is_enum();

					if (st)
					{
						RHBelement *e2;
						need_string(st->id);
						e2=st->children();
						while (e2)
						{
							need_string(e2->id);
							e2=e2->next();
						}
					}
				}

				if (!st)
				{
					if (con->const_typecode_val->is_interface())
					{
						char buf[256];
						get_c_name(con->const_typecode_val,buf,sizeof(buf));
						if (strcmp(buf,"Principal"))
						{
							need_string(con->const_typecode_val->id);
						}
					}
				}

				RHBsequence_type *seq=con->const_typecode_val->is_sequence();

				if (seq)
				{
					need_type(seq->sequence_of);
				}

				RHBarray *ar=con->const_typecode_val->is_array();

				if (ar)
				{
					need_type(ar->array_of);
				}
			}
		}
	}
	else
	{
		if (el->is_type())
		{
			st=el->is_type()->is_struct();

			if (!st)
			{
				st=el->is_type()->is_enum();

/*				if (st)
				{
					__asm int 3;
				}*/
			}
		}
	}

	if (st)
	{
		need_string(st->id);

		el=st->children();

		while (el)
		{
			build_secondary_list(el);

			el=el->next();
		}
	}
	else
	{
		if (el->is_type())
		{
			RHBstruct_element *se;
			RHBenum *e;

			se=el->is_type()->is_struct_element();
	
			if (se)
			{
				need_string(se->id);
				need_type(se->element_type);
			}

			e=el->is_type()->is_enum();

			if (e)
			{
				RHBelement *en;
				need_string(e->id);
				en=e->children();
				while (en)
				{
					need_string(en->id);
					en=en->next();
				}
			}

			RHBinterface *iface=el->is_type()->is_interface();

			if (iface)
			{
				need_string(iface->id);
			}
		}
	}
}

void RHBtc_emitter::need_type(RHBtype *type)
{
	RHBconstant *con;

	if (type->is_typedef()) 
	{
		type=unwind_typedef(type);
	}

	con=find_constant_for(type,&extern_data);

	if (!con)
	{
		con=find_constant_for(type,&static_data);

		if (!con)
		{
			char n[256];
			char newbuf[256];

			get_c_name(type,n,sizeof(n));
	
			printf("no constant for type %s\n",n);

/*			if (!strcmp(n,"4")) __asm int 3;*/

			if (type->is_sequence())
			{
				snprintf(newbuf,sizeof(newbuf),"TC_seq_%s",n);

				RHBtype *t2=type->is_sequence()->sequence_of;

				need_type(unwind_typedef(t2));
			}
			else
			{
				if (type->is_array())
				{
					char m[256];
					RHBtype *t2=type->is_array()->array_of;
					get_c_name(t2,m,sizeof(m));
					snprintf(newbuf,sizeof(newbuf),"TC_array_%s_%s",m,n);

					need_type(unwind_typedef(t2));
				}
				else
				{
					if (type->is_qualified())
					{
						snprintf(newbuf,sizeof(newbuf),"TC__%s_%s",
								type->id,
								type->is_qualified()->base_type->id);
					}
					else
					{
						snprintf(newbuf,sizeof(newbuf),"TC__%s",n);
					}
				}
			}

			printf("adding typecode %s\n",newbuf);

			con=new RHBconstant(0,newbuf);
			con->const_typecode_val=type;
			con->is_private=1;
			strncpy(newbuf,"TypeCode",sizeof(newbuf));
			con->constant_type=GetRepository()->find_named_element(0,newbuf,sizeof(newbuf))->is_type();
			static_data.add(con);
		}
	}

	if (!referenced_tcs.contains(con))
	{
		referenced_tcs.add(con);
	}
}

void RHBtc_emitter::need_string(const char *p)
{
	if (!p) return;
	if (!*p) return;

/*	if (find_string(p)) return;
	char buf[256];
	strcpy(buf,p);

	RHBvalue *str;
	str=new RHBvalue(0,buf);
	str->value=new char [strlen(p)+1];
	strcpy(str->value,p);
	string_data.add(str);*/
}

RHBvalue *RHBtc_emitter::find_string(const char *str)
{
	if (!str) return 0;
	if (!*str) return 0;
	size_t i=0;
	while (i < string_data.length())
	{
		RHBvalue *val=string_data.get(i)->is_type()->is_value();

		if (!strcmp(val->value_string,str)) return val;
		i++;
	}

	return 0;
}


void RHBtc_emitter::generate_strings(RHBoutput *out)
{
	size_t i=0;
	while (i < string_data.length())
	{
		generate_string(out,string_data.get(i)->is_type()->is_value());
		i++;
	}
}

void RHBtc_emitter::generate_string(RHBoutput *out,RHBvalue *val)
{
	out_printf(out,"static char string_%s[]=\042%s\042;\n",
					val->id,val->value_string);
	out_printf(out,"static char *stringPtr_%s=string_%s;\n",
					val->id,val->id);
}

void RHBtc_emitter::generate_tcs(RHBoutput *out,RHBelement_sequence *seq)
{
	size_t i=0;
	while (i < seq->length())
	{
		generate_tc(out,seq->get(i)->is_type()->is_constant());

		i++;
	}
}

void RHBtc_emitter::generate_tc(RHBoutput *out,RHBconstant *c)
{
	if (c)
	{
		char n[256];
		RHBtype *type=c->const_typecode_val;

		get_c_name(c,n,sizeof(n));

		out_printf(out,"#ifndef __TCD_%s_generated\n",n);
		out_printf(out,"#define __TCD_%s_generated\n",n);
		out_printf(out,"/* generate TypeCodeData for %s */\n",n);

		generate_TypeCode(this,out,type,n);

		out_printf(out,"#endif /* __TCD_%s_generated */\n",n); 
	}
}

static const char *get_TCKind(RHBemitter *em,RHBtype *type,char *buf,size_t buflen)
{
	if (!type) return "tk_null";

	RHBtype_def *td=type->is_typedef();

	while (td)
	{
		if (em->is_SOMFOREIGN(td)) return "tk_foreign";

		td=td->alias_for->is_typedef();
	}

	if (type->is_base_type())
	{
		strncpy(buf,"tk_",buflen);
		em->get_c_name(type,&buf[3],buflen-3);
		return buf;
	}

	if (type->is_interface())
	{
		if (!count_elements(em,type))
		{
			return "tk_Principal";
		}
		else
		{
			return "tk_objref";
		}
	}

	if (type->is_sequence())
	{
		return "tk_sequence";
	}

	if (type->is_array())
	{
		return "tk_array";
	}

	if (type->is_enum())
	{
		return "tk_enum";
	}

	if (type->is_qualified())
	{
		RHBqualified_type *qual=type->is_qualified();

		if (!strcmp(qual->id,"signed"))
		{
			return get_TCKind(em,qual->base_type,buf,buflen);
		}

		strncpy(buf,"tk_u",buflen);

		em->get_c_name(qual->base_type,&buf[4],buflen-4);

		return buf;
	}

	if (type->is_any())
	{
		return "tk_any";
	}

	if (type->is_TypeCode())
	{
		return "tk_TypeCode";
	}

	if (type->is_struct())
	{
		return "tk_struct";
	}

	if (type->is_string())
	{
		return "tk_string";
	}

	if (type->is_exception())
	{
#if 0
		return "tk_exception";
#else
		return "tk_struct";
#endif
	}

	if (type->is_union())
	{
		return "tk_union";
	}

#ifdef _WIN32
	{
		bomb("type handling");
	}
#endif



	return "tk_null";
}

void RHBtc_emitter::generate_TCKind(RHBoutput *out,RHBtype *type)
{
	char buf[256];
	const char *p=get_TCKind(this,type,buf,sizeof(buf));

	out->write(p,strlen(p));
}

void RHBtc_emitter::generate_length(RHBoutput *out,RHBtype *type)
{
	if (type)
	{
		char n[256];
		get_c_name(type,n,sizeof(n));
		if ((!strcmp(n,"null"))||
			(!strcmp(n,"void")))
		{
			out_printf(out,"0");
		}
		else
		{
			if (type->is_interface())
			{
				out_printf(out,"sizeof(%s_SOMSTAR)",n);
			}
			else
			{
				if (type->is_sequence())
				{
					out_printf(out,"sizeof(GENERIC_SEQUENCE)");
				}
				else
				{
					if (type->is_qualified())
					{
						strncat(n," ",sizeof(n)-1);
						strncat(n,type->is_qualified()->base_type->id,sizeof(n)-1);
						out_printf(out,"sizeof(%s)",n);
					}
					else
					{
						out_printf(out,"sizeof(%s)",n);
					}
				}
			}
		}
	}
	else
	{
		out_printf(out,"0");
	}
}

/*
static void align_inline(RHBemitter *e,RHBoutput *out,const char *type)
{
	e->out_printf(out,"(int)&((struct { octet _o; %s _d; } *)0)->_d",type);
}
*/

static void align_inline2(const char *type,char *buf,size_t buflen)
{
	snprintf(buf,buflen,"(int)(size_t)&((struct { octet _o; %s _d; } *)0)->_d",type);
}

static void get_alignment(RHBemitter *em,RHBtype *type,char *buf,size_t buflen)
{
	strncpy(buf,"0",buflen);
	if (type)
	{
		char n[256];
		em->get_c_name(type,n,sizeof(n));
		if (strcmp(n,"null")&&strcmp(n,"void"))
		{
			if (type->is_interface())
			{
				align_inline2("void *",buf,buflen);
			}
			else
			{
				if (type->is_sequence())
				{
					align_inline2("GENERIC_SEQUENCE",buf,buflen);
				}
				else
				{
					if (type->is_array())
					{
						RHBarray *a=type->is_array();

						get_alignment(em,a->array_of,buf,buflen);

						return;
					}
					else
					{
						if (type->is_qualified())
						{
							while (type->is_qualified())
							{
								type=type->is_qualified()->base_type;
								strncat(n," ",sizeof(n)-1);
								strncat(n,type->id,sizeof(n)-1);
							}

							align_inline2(n,buf,buflen);
						}
						else
						{
							align_inline2(n,buf,buflen);
						}
					}
				}
			}
		}
	}
}

void RHBtc_emitter::generate_alignment(RHBoutput *out,RHBtype *type)
{
	char buf[1024];

	get_alignment(this,type,buf,sizeof(buf));

	out_printf(out,"%s",buf);
}

void RHBtc_emitter::generate_sequence_data(RHBoutput *out,const char *name,RHBtype *type)
{
	const char *et=element_type_for_type(type);

	if (!et) return;

	out_printf(out,"static %s _buffer_%s[]={\n",et,name);

	if (type->is_enum())
	{
		generate_enum_sequence_data(out,type->is_enum());
	}
	else
	{
		if (type->is_struct())
		{
			generate_struct_sequence_data(out,type->is_struct());
		}
		else if (type->is_exception())
		{
			generate_struct_sequence_data(out,type->is_exception());
		}
		else
		{
			if (type->is_interface())
			{
				generate_interface_sequence_data(out,type->is_interface());
			}
			else
			{
				if (type->is_string())
				{
					generate_string_sequence_data(out,type->is_string());
				}
				else
				{
					if (type->is_sequence())
					{
						generate_sequence_sequence_data(out,type->is_sequence());
					}
				}
			}
		}
	}

	out_printf(out,"};\n");
}

void RHBtc_emitter::generate_interface_sequence_data(RHBoutput *out,RHBinterface *iface)
{
	out_printf(out,"{&TC__string,&stringPtr_%s}",iface->id);
}

void RHBtc_emitter::generate_enum_sequence_data(RHBoutput *out,RHBenum *en)
{
	RHBelement *el=en->children();

	while (el)
	{
		out_printf(out,"\t\"%s\"",el->id);
		el=el->next();
		if (el)
		{
			out_printf(out,",\n");
		}
	}
}

void RHBtc_emitter::generate_sequence_sequence_data(RHBoutput *out,RHBsequence_type *seq)
{
	/* type followed by length */

	long len=0;

	RHBtype *type=unwind_typedef(seq->sequence_of);

	RHBconstant *con=find_constant_for(type,&extern_data);

	if (!con)
	{
		con=find_constant_for(type,&static_data);
	}

	out_printf(out,"{&TC__TypeCode,&ptr_%s}",con->id);

	if (seq->length)
	{
		len=seq->length->is_value()->numeric_value();
	}

	out_printf(out,",\n{&TC__long,&long_%ld}",len);
}

void RHBtc_emitter::generate_string_sequence_data(RHBoutput *out,RHBstring_type *str)
{
	long len=0;

	if (str->length)
	{
		len=str->length->is_value()->numeric_value();
	}

	out_printf(out,"{&TC__long,&long_%ld}",len);
}

void RHBtc_emitter::generate_struct_sequence_data(RHBoutput *out,RHBtype *str)
{
	RHBelement *el=str->children();

	while (el)
	{
		RHBstruct_element *e2=el->is_type()->is_struct_element();

		out_printf(out,"{\"%s\"",e2->id);

		RHBtype *type=unwind_typedef(e2->element_type);

		RHBconstant *con=find_constant_for(type,&extern_data);

		if (!con)
		{
			con=find_constant_for(type,&static_data);
		}

		out_printf(out,",(TypeCode)&%s}",con->id);

		el=el->next();

		if (el)
		{
			out_printf(out,",\n");
		}
	}

}

const char *RHBtc_emitter::struct_type_for_type(RHBtype *type)
{
	if (type)
	{
		if (type->is_pointer())
		{
			return "struct TypeCode_pointer";
		}

		if (type->is_struct())
		{
			return "struct TypeCode_struct";
		}

		if (type->is_exception())
		{
			return "struct TypeCode_exception";
		}

		if (type->is_string())
		{
			return "struct TypeCode_string";
		}

		if (type->is_sequence())
		{
			return "struct TypeCode_sequence";
		}

		if (type->is_array())
		{
			return "struct TypeCode_array";
		}

		if (type->is_interface())
		{
			char buf[256];
			get_c_name(type,buf,sizeof(buf));
			if (strcmp(buf,"Principal"))
			{
				return "struct TypeCode_objref";
			}
		}

		if (type->is_enum())
		{
			return "struct TypeCode_enum";
		}
	}

	return "struct TypeCode_base";
}

const char *RHBtc_emitter::element_type_for_type(RHBtype *type)
{
	if (type->is_struct() || type->is_exception())
	{
		return "struct TypeCode_struct_member";
	}

	if (type->is_enum())
	{
		return "char *";
	}

	return NULL;
}

void RHBtc_emitter::generate_idl_name(RHBoutput *out,RHBtype *type)
{
	char id[256];
	get_global_id(type,id,sizeof(id));
	out_printf(out,",\"%s\"",id);
}

void RHBtc_emitter::generate_name(RHBoutput *out,RHBtype *type)
{
/*	char buf[256];
	get_global_id(type,buf);*/
	out_printf(out,",\"%s\"",type->id);
}

void RHBtc_emitter::generate_extra_params(RHBoutput *out,RHBconstant *c,RHBtype *type)
{
	if (!type) return;

	RHBinterface *iface=type->is_interface();
	char n[256];

	if (iface)
	{
		get_ir_name(type,n,sizeof(n));

		if (strcmp(n,"::Principal"))
		{
/*			generate_idl_name(out,type);*/

			get_ir_name(type,n,sizeof(n));
			out_printf(out,",\"%s\"",n);
			
/*			i=sprintf(buf,",\"%s\"",iface->id);
			out->write(buf,i);
*/

/*			generate_name(out,type);*/
		}

		return;
	}

	RHBsequence_type *seq=type->is_sequence();

	if (seq)
	{
		RHBtype *tt=seq->sequence_of;
		long num=0;
		
		if (seq->length)
		{
			num=seq->length->numeric_value();
		}

		if (tt->is_typedef())
		{
			tt=unwind_typedef(tt);
		}

		RHBconstant *con=find_constant_for(tt,&extern_data);

		if (!con)
		{
			con=find_constant_for(tt,&static_data);
		}

		if (con)
		{
			out_printf(out,",(TypeCode)&%s,%ld",con->id,num);
		}
		else
		{
			char n2[256];
			get_c_name(tt,n2,sizeof(n2));
			out_printf(out,",(TypeCode)&TC__%s,%ld",n2 /*tt->id*/,num);
		}

		return;
	}

	RHBarray *array=type->is_array();

	if (array)
	{
		RHBtype *tt=array->array_of;
		long num=0;
		
		if (array->elements)
		{
			num=array->elements->numeric_value();
		}

		if (tt->is_typedef())
		{
			tt=unwind_typedef(tt);
		}

/*		i=sprintf(buf,",(TypeCode)&TC__%s,%ld",tt->id,num);
		out->write(buf,i);*/
		RHBconstant *con=find_constant_for(tt,&extern_data);

		if (!con)
		{
			con=find_constant_for(tt,&static_data);
		}

		if (con)
		{
			out_printf(out,",(TypeCode)&%s,%ld",con->id,num);
		}
		else
		{
			char n2[256];
			get_c_name(tt,n2,sizeof(n2));
			out_printf(out,",(TypeCode)&TC__%s,%ld",n2 /*tt->id*/,num);
		}

		return;
	}

	RHBenum *en=type->is_enum();

	if (en)
	{
/*		repeat count
		name
		ptr to element list
		*/
		long k=0;
		RHBelement *e=en->children();
		while (e)
		{
			k++;
			e=e->next();
		}
/*		get_c_name(en,n);
		i=sprintf(buf,",%ld,\"%s\",_buffer_TC__%s",k,en->id,n);*/
		get_c_name(c,n,sizeof(n));
		out_printf(out,",%ld,\"%s\",_buffer_%s",k,en->id,n);

/*		generate_idl_name(out,type);*/

		return;
	}

	RHBstruct *st=type->is_struct();

	if (st)
	{
		long k=0;
		RHBelement *e=st->children();
		while (e)
		{
			k++;
			e=e->next();
		}
		get_c_name(st,n,sizeof(n));
		out_printf(out,",%ld,\"%s\",_buffer_%s",k,st->id,c->id);
/*		generate_idl_name(out,type);*/

		return;
	}

	RHBexception *ex=type->is_exception();

	if (ex)
	{
		long k=0;
		RHBelement *e=ex->children();
		while (e)
		{
			k++;
			e=e->next();
		}
		get_c_name(ex,n,sizeof(n));
		out_printf(out,",%ld,\"%s\",_buffer_%s",k,ex->id,c->id);
/*		generate_idl_name(out,type);*/

		return;
	}
}


/* 2nd attempt at generating typecodes so they
   are self contained */

class dseg_bucket;

class dseg_bucket_base
{
public:
	dseg_bucket_base *next;
	dseg_bucket *parent;
	dseg_bucket *root;
	RHBtype *ref_tc;
	char name[256];

	dseg_bucket_base(const char *n) :
		next(NULL),
		parent(NULL),
		root(NULL),
		ref_tc(NULL)
	{
		strncpy(name,n,sizeof(name));
	}

	virtual ~dseg_bucket_base() { /* just avoids a warning in later g++
									about having virtuals but no virtual destructor */ }

	virtual void emit_define(RHBemitter *em,RHBoutput *out,int nest)=0;
	virtual void emit_value(RHBemitter *em,RHBoutput *out,int nest)=0;

	const char *get_name(char *buf,size_t buflen);
};

class dseg_bucket : public dseg_bucket_base
{
	int id;
public:
	dseg_bucket_base *list;
	RHBtype *root;

	dseg_bucket(const char *n,RHBtype *r) : 
		dseg_bucket_base(n),
		id(0),
		list(NULL),
		root(r)
	{
	}

	~dseg_bucket()
	{
		while (list)
		{
			dseg_bucket_base *b=list;
			list=b->next;
			delete b;
		}
	}

	void emit_define(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"struct {\n");
		dseg_bucket_base *p=list;
		while (p)
		{
			p->emit_define(em,out,nest+1);
			em->out_printf(out,";\n",name);
			p=p->next;
		}
		em->dump_nest(out,nest);
		em->out_printf(out,"} %s",name);
	}

	void emit_value(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"{\n");
		dseg_bucket_base *p=list;
		while (p)
		{
			p->emit_value(em,out,nest+1);
			p=p->next;
			if (p)
			{
				em->out_printf(out,",");
			}
			em->out_printf(out,"\n");
		}
		em->dump_nest(out,nest);
		em->out_printf(out,"}");
	}

	void emit_struct(RHBemitter *em,RHBoutput *out)
	{
		emit_define(em,out,0);

		em->out_printf(out,"=",name);

		emit_value(em,out,0);

		em->out_printf(out,";\n");
	}

	dseg_bucket_base *add(dseg_bucket_base *n)
	{
		if (list)
		{
			dseg_bucket_base *p=list; while (p->next) p=p->next;
			p->next=n;
		}
		else
		{
			list=n;
		}
		n->parent=this;
		return n;
	}

	dseg_bucket_base *add_TypeCode(RHBemitter *em,RHBtype *,struct RHBself_tracker *);
};

class dseg_bucket_type_name_value : public dseg_bucket_base
{
public:
	char type_name[256];
	char value[256];

	dseg_bucket_type_name_value(const char *t,const char *n,const char *v) : 
			dseg_bucket_base(n) 
	{
		strncpy(value,v,sizeof(value));
		strncpy(type_name,t,sizeof(type_name));
	}

	void emit_define(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"%s %s",type_name,name);
	}
	void emit_value(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"%s",value);
	}
};

class dseg_bucket_case_value : public dseg_bucket_base
{
public:
	char value[256];
	RHBtype *switch_type;

	dseg_bucket_case_value(RHBtype *st,const char *n,const char *v) : 
			dseg_bucket_base(n),switch_type(st)
	{
		strncpy(value,v,sizeof(value));
	}

	void emit_define(RHBemitter *em,RHBoutput *out,int nest)
	{
		char switch_type_name[256];
		char *p=switch_type_name;
		RHBtype *st=switch_type;

		while (st)
		{
			em->get_c_name(st,p,switch_type_name+sizeof(switch_type_name)-p);
			p+=strlen(p);

			if (st->is_qualified())
			{
				st=st->is_qualified()->base_type;

				*p++=' ';
				*p=0;
			}
			else
			{
				break;
			}
		}

		em->dump_nest(out,nest);
		em->out_printf(out,"union { %s val; long spacing; } %s",switch_type_name,name);
	}
	void emit_value(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"{%s}",value);
	}
};

class dseg_bucket_ptr : public dseg_bucket_base
{
public:
	char type_name[256];
	char prefix[256];
	dseg_bucket_base *ptr;

	dseg_bucket_ptr(const char *t,const char *n,const char *pref,dseg_bucket_base *p) : 
			dseg_bucket_base(n),
				ptr(p)
	{
		prefix[0]=0;
		if (pref) strncpy(prefix,pref,sizeof(prefix));
		strncpy(type_name,t,sizeof(type_name));
	}

	void emit_define(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"%s %s",type_name,name);
	}
	void emit_value(RHBemitter *em,RHBoutput *out,int nest)
	{
		char buf[256];
		em->dump_nest(out,nest);
		em->out_printf(out,"%s%s",prefix,ptr->get_name(buf,sizeof(buf)));
	}
};

const char * dseg_bucket_base::get_name(char *n,size_t nlen)
{
	if (parent) 
	{
		parent->get_name(n,nlen);
		strncat(n,".",nlen);
	}
	else
	{
		n[0]=0;
	}

	strncat(n,name,nlen);

	return n;
}

class dseg_bucket_array : public dseg_bucket
{
public:
	dseg_bucket_array(const char *n) : 
		dseg_bucket(n,NULL)
	{
	}
	void emit_define(RHBemitter *em,RHBoutput *out,int nest)
	{
		long n=0;
		list->emit_define(em,out,nest);
		dseg_bucket_base *p=list;
		while (p)
		{
			n++;
			p=p->next;
		}
		em->out_printf(out,"[%ld]",n);
	}
	void emit_value(RHBemitter *em,RHBoutput *out,int nest)
	{
		em->dump_nest(out,nest);
		em->out_printf(out,"{\n");

		dseg_bucket_base *p=list;
		while (p)
		{
			p->emit_value(em,out,nest+1);
			p=p->next;
			if (p)
			{
				em->out_printf(out,",");
			}
			em->out_printf(out,"\n");
		}

		em->dump_nest(out,nest);
		em->out_printf(out,"}");
	}
};

static void build_typecode(dseg_bucket *tc,RHBtype *type,RHBemitter *em,struct RHBself_tracker *tracker)
{
	char tname[256];

	tc->add(new dseg_bucket_type_name_value("TCKind","tag",get_TCKind(em,type,tname,sizeof(tname))));

	tname[0]=0;
	
	if (type)
	{
		em->get_c_name(type,tname,sizeof(tname));
	}

	char align[300];
	strncpy(align,"0",sizeof(align));

	get_alignment(em,type,align,sizeof(align));

	tc->add(new dseg_bucket_type_name_value("short","align",align));
	tc->add(new dseg_bucket_type_name_value("unsigned char","version","1"));
	tc->add(new dseg_bucket_type_name_value("unsigned char","flags","0"));

	if (!type) return;

	RHBenum *en=type->is_enum();

	if (en)
	{
		long n=count_elements(em,type)-1;
		snprintf(align,sizeof(align),"%ld",n);
		tc->add(new dseg_bucket_type_name_value("long","rptCount",align));

		snprintf(align,sizeof(align),"\"%s\"",type->id);
		tc->add(new dseg_bucket_type_name_value("char const * const","enumName",align));

/*		dseg_bucket_type_name_value("const void *","*/

		dseg_bucket *a=new dseg_bucket_array("_enumId");

		tc->add(new dseg_bucket_ptr("char const * const *","enumId","",a));

		tc->add(a);

		RHBelement *e=type->children();

		while (e)
		{
			char el_name[256];
			snprintf(el_name,sizeof(el_name),"\"%s\"",e->id);
			a->add(new dseg_bucket_type_name_value("char const * const","_enumId",el_name));
			e=e->next();
		}

		return;
	}

	RHBinterface *iface=type->is_interface();

	if (iface)
	{
		long n=count_elements(em,type);

		if (n)
		{
			char ifname[256];
			char ir_name[256];
			em->get_ir_name(iface,ir_name,sizeof(ir_name));
			snprintf(ifname,sizeof(ifname),"\"%s\"",ir_name);
			tc->add(new dseg_bucket_type_name_value("char const * const","interfaceId",ifname));
		}

		return;
	}

	RHBstring_type *str=type->is_string();

	if (str)
	{
		char maxBound[256];
		RHBvalue *val=NULL;
		if (str->length)
		{
			val=str->length->is_value();
		}
		strncpy(maxBound,"0",sizeof(maxBound));
		if (val)
		{
			snprintf(maxBound,sizeof(maxBound),"%ld",(long)val->numeric_value());
		}
		tc->add(new dseg_bucket_type_name_value("long","maxBound",maxBound));

		return;
	}

	RHBsequence_type *seq=type->is_sequence();

	if (seq)
	{
		char maxBound[256];
		RHBvalue *val=NULL;
		if (seq->length)
		{
			val=seq->length->is_value();
		}
		strncpy(maxBound,"0",sizeof(maxBound));
		if (val)
		{
			snprintf(maxBound,sizeof(maxBound),"%ld",(long)val->numeric_value());
		}

		dseg_bucket_base *a=tc->add_TypeCode(em,seq->sequence_of,tracker);

		tc->add(new dseg_bucket_ptr("void const *","tc","&",a));

		tc->add(new dseg_bucket_type_name_value("long","maxBound",maxBound));

		return;
	}

	RHBarray *ar=type->is_array();

	if (ar)
	{
		char maxBound[256];
		RHBvalue *val=NULL;
		if (ar->elements)
		{
			val=ar->elements->is_value();
		}
		strncpy(maxBound,"0",sizeof(maxBound));
		if (val)
		{
			snprintf(maxBound,sizeof(maxBound),"%ld",(long)val->numeric_value());
		}

		dseg_bucket_base *a=tc->add_TypeCode(em,ar->array_of,tracker);

		tc->add(new dseg_bucket_ptr("void const *","tc","&",a));

		tc->add(new dseg_bucket_type_name_value("long","maxBound",maxBound));

		return;
	}

	if (type->is_struct() || type->is_exception())
	{
		struct RHBself_tracker track2;
		track2.current=type;
		track2.previous=tracker;

		long l=(count_elements(em,type)-1)>>1;
		char rptCount[256];
		snprintf(rptCount,sizeof(rptCount),"%ld",l);

		tc->add(new dseg_bucket_type_name_value("long","rptCount",rptCount));

		char structName[256];

		snprintf(structName,sizeof(structName),"\"%s\"",type->id);
		tc->add(new dseg_bucket_type_name_value("char const * const","structName",structName));

		if (l)
		{
			dseg_bucket *my_ar=new dseg_bucket_array("_mbrs");

			tc->add(new dseg_bucket_ptr("void const *","mbrs","",my_ar));

			tc->add(my_ar);

			RHBelement *e=type->children();

			while (e)
			{
				RHBstruct_element *se=e->is_type()->is_struct_element();

				if (se)
				{
					dseg_bucket *bse=new dseg_bucket("_mbrs",NULL);

					my_ar->add(bse);

					dseg_bucket_base *et=tc->add_TypeCode(em,se->element_type,&track2);

					char mbrName[256];

					snprintf(mbrName,sizeof(mbrName),"\"%s\"",se->id);
					bse->add(new dseg_bucket_type_name_value("char const * const","mbrName",mbrName));

					bse->add(new dseg_bucket_ptr("void const *","mbrTc","&",et));
				}

				e=e->next();
			}
		}
		else
		{
			tc->add(new dseg_bucket_type_name_value("void const *","mbrs","NULL"));
		}

		return;
	}

	RHBtype_def *td=type->is_typedef();
	RHBtype *foreigner=NULL;

	while (td)
	{
		if (em->is_SOMFOREIGN(td->alias_for))
		{
			foreigner=td;
			break;
		}
		td=td->alias_for->is_typedef();
	}

	if (foreigner)
	{
		/* char *typeName */
		/* char *context */
		/* long length */

		const char *impctx=foreigner->get_modifier_string("impctx");
	/*	long len=4;*/ /* som defined default length */
		const char *name=foreigner->id;
		const char *length=foreigner->get_modifier_string("length");

		{
			char value[256];

			snprintf(value,sizeof(value),"\"%s\"",name);
			tc->add(new dseg_bucket_type_name_value("char const * const","typeName",value));
		}

		if (impctx)
		{
			char buf[256];
			char value[256];
			impctx=em->unquote(buf,impctx);

			snprintf(value,sizeof(value),"\"%s\"",impctx);
			tc->add(new dseg_bucket_type_name_value("char const * const","context",value));
		}
		else
		{
			tc->add(new dseg_bucket_type_name_value("char const * const","context","NULL"));
		}

		if (length)
		{
			char buf[256];
			length=em->unquote(buf,length);

			tc->add(new dseg_bucket_type_name_value("long","length",length));
		}
		else
		{
			char c_buf[256];
			char value[256];
			em->get_c_name(foreigner,c_buf,sizeof(c_buf));
			snprintf(value,sizeof(value),"sizeof(%s)",c_buf);
			tc->add(new dseg_bucket_type_name_value("long","length",value));
		}

		return;
	}

	RHBunion *un=type->is_union();

	if (un)
	{
		long rptCount=un->elements.length();
		char value[256];
		snprintf(value,sizeof(value),"%ld",rptCount);
		tc->add(new dseg_bucket_type_name_value("long","rptCount",value));
		snprintf(value,sizeof(value),"\"%s\"",un->id);
		tc->add(new dseg_bucket_type_name_value("char const *const","unionName",value));
		dseg_bucket_base *a=tc->add_TypeCode(em,un->switch_type,tracker);
		tc->add(new dseg_bucket_ptr("void const *","swTc","&",a));

		dseg_bucket_array *m=new dseg_bucket_array("_mbrs");
		tc->add(new dseg_bucket_ptr("void const *","mbrs","",m));

		tc->add(m);

		long i=0;

		while (i < rptCount)
		{
			RHBunion_element *el=un->elements.get(i)->is_type()->is_union_element();
			long lVal=0;

			dseg_bucket *mbr=new dseg_bucket("_mbrs",NULL);

			m->add(mbr);

			snprintf(value,sizeof(value),"%ld",lVal);

			if (el->switch_value)
			{
				/* interesting to see what happens in case of a enum... */

				lVal=el->switch_value->numeric_value();
				snprintf(value,sizeof(value),"%ld",lVal);

				mbr->add(new dseg_bucket_type_name_value("long","labelFlag","TCREGULAR_CASE"));
			}
			else
			{
				mbr->add(new dseg_bucket_type_name_value("long","labelFlag","TCDEFAULT_CASE"));
			}

			mbr->add(new dseg_bucket_case_value(un->switch_type,"labelValue",value));

			snprintf(value,sizeof(value),"\"%s\"",el->id);
			mbr->add(new dseg_bucket_type_name_value("char const * const","mbrName",value));

			mbr->add(new dseg_bucket_ptr("void const *","mbrTc","&",
				tc->add_TypeCode(em,el->element_type,tracker)));


			i++;
		}

		return;
	}

#ifdef _WIN32
	if (count_elements(em,type))
	{
		bomb("element count");
	}
#endif
}

void generate_TypeCode(RHBemitter *em,RHBoutput *out,RHBtype *type,const char *name)
{
	/* need to generate the type first

		then the data

		best done with a class that represents an element
		in the struct

	*/
	char tname[256];

	while (type)
	{
		RHBtype_def *td=type->is_typedef();
		if (!td) break;
		if (em->is_SOMFOREIGN(td->alias_for))
		{
			break;
		}
		type=td->alias_for;
	}

	strncpy(tname,"" /*"__somC_TC__"*/,sizeof(tname));
	strncat(tname,name,sizeof(tname)-1);

	dseg_bucket my_struct(tname,NULL);

	dseg_bucket *tc=new dseg_bucket("root",type);

	my_struct.add(tc);

	build_typecode(tc,type,em,0);

	if (em && out)
	{
		em->out_printf(out,szSOMTC_Scope_);
		my_struct.emit_struct(em,out);
	}
}

dseg_bucket_base *dseg_bucket::add_TypeCode(
		RHBemitter *em,
		RHBtype *t,
		struct RHBself_tracker *tracker)
{
	char n[256];
	if (parent)
	{
		return parent->add_TypeCode(em,t,tracker);
	}

	t=em->unwind_typedef(t);

	struct RHBself_tracker *t2=tracker;

	while (t2)
	{
		if (t2->current==t)
		{
			char n2[256];

			em->get_c_name(t,n,sizeof(n));
			align_inline2(n,n2,sizeof(n2));

			id++;
			snprintf(n,sizeof(n),"TC_%d",(int)id);

			dseg_bucket *b=new dseg_bucket(n,NULL);

			b->add(new dseg_bucket_type_name_value("TCKind","tag","tk_self"));
			b->add(new dseg_bucket_type_name_value("short","align",n2));
			b->add(new dseg_bucket_type_name_value("unsigned char","version","1"));
			b->add(new dseg_bucket_type_name_value("unsigned char","flags","0"));

			snprintf(n,sizeof(n),"\"%s\"",t->id);

			b->add(new dseg_bucket_type_name_value("char const * const","selfName",n));

			add(b);

			return b;
		}

		t2=t2->previous;
	}

	dseg_bucket_base *p=list;

	while (p)
	{
		if (p->ref_tc==t) return p;

		p=p->next;
	}

	id++;

	snprintf(n,sizeof(n),"TC_%d",(int)id);

	dseg_bucket *b=new dseg_bucket(n,t);

	b->ref_tc=t;

	add(b);

	build_typecode(b,t,em,tracker);

	return b;
}

