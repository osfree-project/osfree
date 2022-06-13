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

class op_seq
{
	unsigned long _length;
	RHBoperation *_buffer[256];
public:
	op_seq() : _length(0)
	{
	}

	void add(RHBoperation *op)
	{
		_buffer[_length++]=op;
	}

	unsigned long length() 
	{ 
		return _length; 
	}

	RHBoperation *get(unsigned long l)
	{
		if (l < _length) return _buffer[l];
		return NULL;
	}
};

class kernel_class
{
	RHBinterface *iface;
	kernel_class *parent;
	op_seq release_order;
	op_seq methods;

public:
	kernel_class(
		RHBkernel_emitter *emitter,
		RHBinterface *ifp,kernel_class *par)
		: iface(ifp),parent(par)
	{
		if (ifp)
		{
			RHBelement *e=ifp->children();

			while (e)
			{
				RHBoperation *op=e->is_operation();

				if (op)
				{
					if (!emitter->is_operation_procedure(op))
					{
						methods.add(op);
					}
				}

				e=e->next();
			}

			char buf[256];
			strncpy(buf,"releaseorder",sizeof(buf));
			e=iface->find_named_element(NULL,buf,sizeof(buf));

			if (e)
			{
				int i=0;

				while (e->modifier_data.get(i))
				{
					const char *p=e->modifier_data.get(i);
					i++;

					if (strcmp(p,","))
					{
						strncpy(buf,p,sizeof(buf));

						RHBelement *e2=iface->find_named_element(NULL,buf,sizeof(buf));

						if (e2)
						{
							release_order.add(e2->is_operation());
						}
						else
						{
							release_order.add(NULL);
						}
					}
				}
			}
		}
	}

	void gen_data_part(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		if (parent) parent->gen_data_part(emitter,out);

		if (iface->instanceData->children())
		{
			char n[256];
			emitter->get_c_name(iface,n,sizeof(n));
			emitter->out_printf(out,"%sData %s_data;\n",n,n);
		}
	}

	void gen_proto(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		char n[256];
		int total=0;

		kernel_class *p=this;

		while (p)
		{
			total+=(int)p->methods.length();
			p=p->parent;
		}

		emitter->get_c_name(iface,n,sizeof(n));

		emitter->out_printf(out,"struct %s_somMethodTabStruct {\n",n);
		emitter->out_printf(out,"SOMClass SOMSTAR classObject;\n");
		emitter->out_printf(out,"somClassInfo classInfo;\n");
		emitter->out_printf(out,"char *className;\n");
		emitter->out_printf(out,"long instanceSize;\n");
		emitter->out_printf(out,"long dataAlignment;\n");
		emitter->out_printf(out,"long mtabSize;\n");
		emitter->out_printf(out,"long protectedDataOffset;\n");
		emitter->out_printf(out,"somDToken prototectedDataToken;\n");
		emitter->out_printf(out,"somEmbeddedObjStruct *embeddedObj;\n");
		emitter->out_printf(out,"somMethodPtr entries[%d];\n",total);
		emitter->out_printf(out,"};\n"); 


		emitter->out_printf(out,"struct %s_somClassInfoMethodTab {\n",n); 
		emitter->out_printf(out,"struct somClassInfoData classInfo;\n"); 
		emitter->out_printf(out,"struct %s_somMethodTabStruct mtab;\n",n); 
		emitter->out_printf(out,"};\n"); 

		emitter->out_printf(out,"extern struct %s_somClassInfoMethodTab _%s_somClassInfoMethodTab;\n",n,n);

		emitter->out_printf(out,"struct %s_SOMAny {\n",n); 
		emitter->out_printf(out,"struct somMethodTabStruct * mtab;\n"); 

		gen_data_part(emitter,out);

		emitter->out_printf(out,"};\n"); 

/*		i=sprintf(buf,"static struct somDTokenData %s_somDTokenData={\n",n); out->write(buf,i);
		i=sprintf(buf,"(struct somMethodTabStruct *)(void *)&_%s_somMTabs,\n",n); out->write(buf,i);

		if (iface->instanceData->children())
		{
			i=sprintf(buf,"(short)&(((struct %s_SOMAny *)NULL)->%s_data)",n,n); out->write(buf,i);
		}
		else
		{
			i=sprintf(buf,"0"); out->write(buf,i);
		}

		i=sprintf(buf,"};\n"); out->write(buf,i);*/
	}

	void gen_class_object(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		char n[256];
		emitter->get_c_name(iface,n,sizeof(n));
		emitter->out_printf(out,"static struct SOMClass_SOMAny %s_classObject=SOMKERN_classObject(%s)\n",n,n); 
	}


	int overrides(RHBoperation *op)
	{
		if (op)
		{
			char buf[256];
			strncpy(buf,"override",sizeof(buf));
			RHBelement *el=iface->find_named_element(NULL,buf,sizeof(buf));
			if (el)
			{
				int i=0;
				while (i < el->modifier_data.length())
				{
					if (!strcmp(op->id,el->modifier_data.get(i)))
					{
						return 1;
					}

					i++;
				}
			}
			el=iface->children();
			while (el)
			{
				if (!strcmp(el->id,op->id))
				{
					int i=0;
					while (i < el->modifier_data.length())
					{
						if (!strcmp("override",el->modifier_data.get(i)))
						{
							return 1;
						}

						i++;
					}

					strncpy(buf,"override",sizeof(buf));

					if (el->find_named_element(NULL,buf,sizeof(buf)))
					{
						return 1;
					}

				}

				el=el->next();
			}
		}
		return 0;
	}

	void gen_vtable(int *idx,kernel_class *cls,RHBkernel_emitter *emitter,RHBoutput *out,int comma_last)
	{
		if (parent) parent->gen_vtable(idx,cls,emitter,out,1);

		unsigned long l=0;

		while (l < methods.length())
		{
			RHBoperation *op=methods.get(l);
			char n[256];

			if (cls->overrides(op) && strcmp(op->id,"somInit") && strcmp(op->id,"somUninit"))
			{
				emitter->get_function_prefix(cls->iface,n,sizeof(n));
			}
			else
			{
				emitter->get_function_prefix(op->parent()->is_interface(),n,sizeof(n));
			}

			emitter->out_printf(out,"/* %d */ (somMethodPtr)%s%s",*idx,n,op->id);

			l++;
			(*idx)++;

			if ((l < methods.length()) || comma_last)
			{
				out->write(",\n",2);
			}
		}
	}

	int useAlignmentHelper() { return 0; }

	void gen_mtab(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		char n[256];
		int total=0;
		int idx=0;

		kernel_class *p=this;

		while (p)
		{
			total+=(int)p->methods.length();
			p=p->parent;
		}

		emitter->get_c_name(iface,n,sizeof(n));

		emitter->out_printf(out,"{");

		emitter->out_printf(out,"(SOMClass SOMSTAR)(void *)&%s_classObject,\n",n);
		emitter->out_printf(out,"&%s_somClassInfo,\n",n);
		emitter->out_printf(out,"\"%s\",\n",n);
		emitter->out_printf(out,"sizeof(struct %s_SOMAny), /* instanceSize */\n",n); 

		if (iface->instanceData->children())
		{
			if (useAlignmentHelper())
			{
				emitter->out_printf(out,"(int)(size_t)&(((struct %s_dataAlignment *)0)->_d), /* dataAlignment */\n",n);
			}
			else
			{
				emitter->out_printf(out,"(int)(size_t)&(((struct { octet _o; %sData _d; } *)0)->_d), /* dataAlignment */\n",n);
			}
		}
		else
		{
			emitter->out_printf(out,"1, /* dataAlignment */\n");
		}

		emitter->out_printf(out,"%d, /* mtabSize */\n",total);
		emitter->out_printf(out,"sizeof(void *), /* protectedDataOffset */\n"); 
		emitter->out_printf(out,"NULL, /* protectedDataToken */ \n");
		emitter->out_printf(out,"NULL, /* embeddedObj */ \n"); 
		emitter->out_printf(out,"{ /* %d entries */\n",total); 

		gen_vtable(&idx,this,emitter,out,0);


		emitter->out_printf(out,"}}"); 
	}

	void gen(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		char n[256];
		boolean is_root=(boolean)(iface->_parents.length() ? 0 : 1);
		int readOnlyMTokens=1;

		if (!readOnlyMTokens)
		{
			printf("## warning generating writeable method thunks\n"); 
		}

		emitter->get_c_name(iface,n,sizeof(n));
		emitter->out_printf(out,"/* className=%s,\n--releaseorder=%d\n",
				n,(int)release_order.length());
		unsigned int l=0;
		while (l < release_order.length())
		{
			strncpy(n,"NULL",sizeof(n));
			if (release_order.get(l))
			{
				emitter->get_c_name(release_order.get(l),n,sizeof(n));
			}
			emitter->out_printf(out,"[%d]=%s\n",l,n);

			l++;
		}

		emitter->get_c_name(iface,n,sizeof(n));
		emitter->out_printf(out,"--methods=%d\n",(int)methods.length());
		l=0;
		while (l < methods.length())
		{
			strncpy(n,"NULL",sizeof(n));
			if (methods.get(l))
			{
				emitter->get_c_name(methods.get(l),n,sizeof(n));
			}
			emitter->out_printf(out,"[%d]=%s\n",l,n);

			l++;
		}

		emitter->out_printf(out,"*/\n");

		if (l)
		{
			emitter->get_c_name(iface,n,sizeof(n));
/*			i=sprintf(buf,"#if defined(SOM_METHOD_THUNKS) && (defined(USE_THREADS)||defined(SOM_METHOD_THUNKS_READONLY))\n");
			out->write(buf,i);
			i=sprintf(buf,"static somkern_resolve_thunk somMToken_thunks_%s[%d];\n",n,l);
			out->write(buf,i);
			i=sprintf(buf,"#endif\n");
			out->write(buf,i);*/
		}

		if (readOnlyMTokens)
		{
			emitter->out_printf(out,"#ifdef _WIN32\n"); 
			emitter->out_printf(out,"#ifdef __WATCOMC__\n"); 
			emitter->out_printf(out,"#pragma data_seg(\"_TEXT\",\"CODE\")\n");
			emitter->out_printf(out,"#else /* _WIN32 */\n");
			emitter->out_printf(out,"#pragma data_seg(\".text\",\"CODE\")\n"); 
			emitter->out_printf(out,"#endif /* !__WATCOMC__ */\n");
			emitter->out_printf(out,"#endif /* _WIN32 */\n"); 
		}

		emitter->get_c_name(iface,n,sizeof(n));
		emitter->out_printf(out,"static struct somMTokenData somMToken_%s[%d]={\n",
					n,(int)methods.length());

		l=0;

		while (l < methods.length())
		{
			int o1,o2;
			RHBoperation *op=methods.get(l);
/*			boolean is_overridden=emitter->is_overridden(op); */
			boolean is_aggregate=emitter->is_aggregate(op);
			RHBtype *rt=op->return_type;

			emitter->get_c_name(iface,n,sizeof(n));

			if (/*is_overridden ||*/ 1)
			{
				if (is_aggregate)
				{
					char tn[256];

					if (rt->is_sequence())
					{
						strncpy(tn,"GENERIC_SEQUENCE",sizeof(tn));
					}
					else
					{
						if (rt->is_any())
						{
							strncpy(tn,"any",sizeof(tn));
						}
						else
						{
							emitter->get_c_name(rt,tn,sizeof(tn));
						}
					}

					if (is_root)
					{
						emitter->out_printf(out,"{ somMToken_jump_aggregate_root(somMToken_%s,%d,%s)\n",
										iface->id,l,tn); 
					}
					else
					{
						emitter->out_printf(out,"{ somMToken_jump_aggregate(somMToken_%s,%d,%s)\n",
										iface->id,l,tn); 
					}
				}
				else
				{
					if (is_root)
					{
						emitter->out_printf(out,"{ somMToken_jump_root(somMToken_%s,%d)\n",iface->id,l); 
					}
					else
					{
						emitter->out_printf(out,"{ somMToken_jump(somMToken_%s,%d)\n",iface->id,l); 
					}
				}
			}
			else
			{
				emitter->out_printf(out,"{ somMToken_jump_raw(somMToken_%s,%d)\n",
					iface->id,l);
			}

			emitter->out_printf(out,"(struct somMethodTabStruct *)(void *)&_%s_somMTabs,{2,\n",iface->id); 

/* classData,
	methodId,
	methodDescriptor,
	method,
	redispatchStub,
	applyStub */

			emitter->out_printf(out,"&%sClassData.%s,\n",iface->id,op->id);

			emitter->out_printf(out,"&somMN_%s,\n",op->id);

			emitter->out_printf(out,"&somDS_%s_%s,\n",iface->id,op->id);
			
			emitter->get_function_prefix(iface,n,sizeof(n));
			emitter->out_printf(out,"(somMethodPtr)%s%s,\n",n,op->id);

			emitter->out_printf(out,"#ifdef SOM_METHOD_STUBS\n");

			{
				emitter->out_printf(out,"(somMethodPtr)((void *)-1L),\n");
				emitter->out_printf(out,"(somMethodPtr)&somApRdInfo_%s_%s\n",iface->id,op->id);
			}

			emitter->out_printf(out,"#else /* SOM_METHOD_STUBS */\n");

			{
				emitter->out_printf(out,"(somMethodPtr)somRD_%s_%s,\n",iface->id,op->id);

				if (emitter->op_is_varg(op))
				{
					emitter->out_printf(out,"NULL\n" /*,iface->id,op->id*/);
				}
				else
				{
					emitter->out_printf(out,"(somMethodPtr)somAP_%s_%s\n",iface->id,op->id);
				}
			}

			emitter->out_printf(out,"#endif /* SOM_METHOD_STUBS */\n");

			o1=o2=l;

			if (parent) o2+=(int)parent->methods.length();

			emitter->out_printf(out,"},%d,%d}\n",o1,o2); 

			l++;

			if (l < methods.length())
			{
				emitter->out_printf(out,","); 
			}
		}

		emitter->out_printf(out,"};\n");

		if (readOnlyMTokens)
		{
			emitter->out_printf(out,"#ifdef SOM_DATA_THUNKS\n");
			emitter->out_printf(out,"static struct somDTokenData %s_somDToken=\n",iface->id);
			emitter->out_printf(out,"{ somDToken_jump (struct somMethodTabStruct *)(void *)&_%s_somMTabs,\n",iface->id);

			if (iface->instanceData->children())
			{
				emitter->out_printf(out,"(short)(int)(size_t)&(((struct %s_SOMAny *)NULL)->%s_data)\n",iface->id,iface->id); 
			}
			else
			{
				emitter->out_printf(out,"0\n");
			}
			emitter->out_printf(out,"};\n");

			emitter->out_printf(out,"#endif /* SOM_DATA_THUNKS */\n");


			emitter->out_printf(out,"#ifdef _WIN32\n"); 
			emitter->out_printf(out,"#pragma data_seg()\n"); 
			emitter->out_printf(out,"#endif /* _WIN32 */\n"); 
		}
	}

	void gen_mtabList(RHBkernel_emitter *emitter,RHBoutput *out);

	int total_methods()
	{
		kernel_class *k=this;
		int total=0;

		while (k)
		{
			total+=(int)k->methods.length();
			k=k->parent;
		}

		return total;
	}

	void gen_parent_info(RHBkernel_emitter *emitter,RHBoutput *out,int *offset,int flag);
	void gen_somClassInfoMethodTab(RHBkernel_emitter *emitter,RHBoutput *out);
	void gen_somClassInfo(RHBkernel_emitter *emitter,RHBoutput *out,int /*offset*/);

	void gen_ccd(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		char n[256];
		RHBelement_sequence va_ops;
		/*int total=*/total_methods();
		/*int added=*/methods.length();
		int parent_total=1;
		if (parent) parent_total++;

		int num_vaops=emitter->get_va_operations(iface,&va_ops);
	/*	int offset=0;*/

		emitter->get_c_name(iface,n,sizeof(n));

		emitter->out_printf(out,"%sCClassDataStructure SOMDLINK %sCClassData={\n",n,n);

		emitter->out_printf(out,"(struct somMethodTabList *)(void *)&%s_somClassInfo.parents,\n",n);
		emitter->out_printf(out,"#ifdef SOM_DATA_THUNKS\n",n);
		emitter->out_printf(out,"&%s_somDToken\n",n);
		emitter->out_printf(out,"#else /* SOM_DATA_THUNKS */\n",n);
		emitter->out_printf(out,"&%s_somClassInfo.data_token\n",n);
		emitter->out_printf(out,"#endif /* SOM_DATA_THUNKS */\n",n);

		if (num_vaops)
		{
			int k=0;
			while (k < num_vaops)
			{
				RHBoperation *op=va_ops.get(k)->is_operation();

				if (op)
				{
					emitter->out_printf(out,",\n(somMethodPtr)somVA_%s_%s",n,op->id);
				}

				k++;
			}
		}

		emitter->out_printf(out,"};\n");
	}

	void gen_cd(RHBkernel_emitter *emitter,RHBoutput *out)
	{
		char n[256];
		/*int total=*/ total_methods();
		/*int added=*/ methods.length();
		int parent_total=1;
		if (parent) parent_total++;
		unsigned int offset=0;

		emitter->get_c_name(iface,n,sizeof(n));

		emitter->out_printf(out,"%sClassDataStructure SOMDLINK %sClassData={\n",n,n);

		emitter->out_printf(out,"(SOMClass SOMSTAR)(void *)&%s_classObject\n",n /*,n*/);

		while (offset < release_order.length())
		{
			RHBoperation *op=release_order.get(offset);

			if (op)
			{
				if (emitter->is_operation_procedure(op))
				{
					char t[256];
					emitter->get_function_prefix(iface,t,sizeof(t));
					emitter->out_printf(out,",%s%s\n",t,op->id);
				}
				else
				{
					unsigned int j=0;
					const char *m=emitter->is_overridden(op) ? "somMToken_override" : "somMToken_direct";

					while (j < methods.length())
					{
						RHBoperation *o2=methods.get(j);

						if (o2==op) break;
						if (!strcmp(o2->id,op->id)) break;
						j++;
					}

					emitter->out_printf(out,",%s(somMToken_%s+%d)/* %s */\n",m,n,j,op->id);
				}
			}
			else
			{
				emitter->out_printf(out,",NULL\n");
			}

			offset++;
		}

		emitter->out_printf(out,"};\n");
	}
};

void kernel_class::gen_somClassInfo(RHBkernel_emitter *emitter,RHBoutput *out,int /*offset*/)
{
	char n[256];
	int total=total_methods();
	long l_added=methods.length();
	int parent_total=1;
	if (parent) parent_total++;

	emitter->get_c_name(iface,n,sizeof(n));

	emitter->out_printf(out,"{SOMKERN_somClassInfo_SI,\n");
	emitter->out_printf(out,"%s_MajorVersion,%s_MinorVersion,\n",n,n);

	if (iface->instanceData->children())
	{
		if (useAlignmentHelper())
		{
			emitter->out_printf(out,"sizeof(%sData),(int)(size_t)&(((struct %s_dataAlignment *)0)->_d),",n,n);
		}
		else
		{
			emitter->out_printf(out,"sizeof(%sData),(int)(size_t)&(((struct { octet _o; %sData _d; } *)0)->_d),",n,n);
		}
	}
	else
	{
		emitter->out_printf(out,"0,0,");
	}

	if (parent)
	{
		emitter->out_printf(out,"1,");
	}
	else
	{
		emitter->out_printf(out,"0,");
	}

	emitter->out_printf(out,"&%sSCI,\n",n);

	emitter->out_printf(out,"{ somDToken_jump (struct somMethodTabStruct *)(void *)&_%s_somMTabs,\n",n);

	if (iface->instanceData->children())
	{
		emitter->out_printf(out,"(short)(int)(size_t)&(((struct %s_SOMAny *)NULL)->%s_data)},\n",n,n); 
	}
	else
	{
		emitter->out_printf(out,"0},\n" /*,n*/);
	}

	emitter->out_printf(out,"NULL, /* substituted */\n" /*,n*/);

#if 1
	emitter->out_printf(out,"(short)sizeof(%s_destructMask),\n",n);
#else
	i=sprintf(buf,"NULL, /* somInit */\n" /*,n */);
	if (parent)
	{
		char z[256];
		strncpy(z,"somInit",sizeof(z));
		RHBelement *op=parent->iface->find_named_element(NULL,z);
		if (overrides(op->is_operation()))
		{
			emitter->get_function_prefix(iface,z);
			i=sprintf(buf,"%s%s,\n",z,op->id);
		}
	}*/
#endif

	if (parent)
	{
		char z[256];
		strncpy(z,"somUninit",sizeof(z));
		RHBelement *op=parent->iface->find_named_element(NULL,z,sizeof(z));
		if (overrides(op->is_operation()))
		{
			emitter->get_function_prefix(iface,z,sizeof(z));
			emitter->out_printf(out,"%s%s,\n",z,op->id);
		}
		else
		{
			emitter->out_printf(out,"NULL, /* somUninit */\n" /*,n*/);
		}
	}
	else
	{
		emitter->out_printf(out,"NULL, /* somUninit */\n" /*,n*/);
	}

	/* parents */

	emitter->out_printf(out,"{(struct somMethodTabStruct *)(void *)&_%s_somMTabs,\n",n);

	if (parent)
	{
		emitter->out_printf(out,"&%s_somMethodTabList,",parent->iface->id);
	}
	else
	{
		emitter->out_printf(out,"NULL,");
	}

	emitter->out_printf(out,"(SOMClass SOMSTAR)(void *)&%s_classObject,\n",n);
	emitter->out_printf(out,"%s_somRenewNoInitNoZeroThunk,\n",n); 
	emitter->out_printf(out,"sizeof(struct %s_SOMAny),\n",n);

	emitter->out_printf(out,"/* initializers */ NULL,\n");
	emitter->out_printf(out,"/* resolvedMToken */ NULL,\n");

	emitter->out_printf(out,"SOMKERN_initCtrl(%s),\n",n); 
	emitter->out_printf(out,"SOMKERN_destructCtrl(%s),\n",n); 
	emitter->out_printf(out,"SOMKERN_assignCtrl(%s),\n",n);

	emitter->out_printf(out,"0, /* embeddedTotalCount */\n");
	emitter->out_printf(out,"0, /* hierarchyTotalCount */\n"); 
	emitter->out_printf(out,"0 /* unused */},\n"); 
	/* jump_table */

	emitter->out_printf(out,"{%d,%d},\n",total,total);

	/* added methods */

	emitter->out_printf(out,"{%ld,%ld,somMToken_%s},\n",l_added,l_added,n);

/*		i=sprintf(buf,"#if defined(SOM_METHOD_THUNKS) && (defined(USE_THREADS)||defined(SOM_METHOD_THUNKS_READONLY))\n");
	out->write(buf,i);
	i=sprintf(buf,"somMToken_thunks_%s,\n",n);
	out->write(buf,i);
	i=sprintf(buf,"#endif\n");
	out->write(buf,i);*/

	/* parent_jump_table */

	if (parent)
	{
		emitter->out_printf(out,"{1,1,&%s_somMethodTabPtr},\n",parent->iface->id);
	}
	else
	{
		emitter->out_printf(out,"{0,0,NULL},\n");
	}

	/* classes */

	emitter->out_printf(out,"{%d,%d,%s_somParentClassInfo},",parent_total,parent_total,n);

	/* keyed data thunk */

	emitter->out_printf(out,"{{%d,%d,(somToken)\"%s\"},{NULL,&_%s_somMTabs},0}",
			(int)strlen(n),(int)strlen(n),n,n);

	emitter->out_printf(out,"}");
}

void kernel_class::gen_somClassInfoMethodTab(RHBkernel_emitter *emitter,RHBoutput *out)
{
	char n[256];
	int offset=0;

	emitter->get_c_name(iface,n,sizeof(n));

	emitter->out_printf(out,"static struct somParentClassInfo %s_somParentClassInfo[]={\n",n);

	gen_parent_info(emitter,out,&offset,0);

	emitter->out_printf(out,"};\n");

	emitter->out_printf(out,"struct %s_somClassInfoMethodTab _%s_somClassInfoMethodTab={\n",n,n);

	gen_somClassInfo(emitter,out,offset);

	emitter->out_printf(out,",\n");

	gen_mtab(emitter,out);

	emitter->out_printf(out,"};\n");
}

void kernel_class::gen_parent_info(RHBkernel_emitter *emitter,RHBoutput *out,int *offset,int flag)
{
	char n[256];
	/*int total=*/total_methods();
	/*int added=*/methods.length();

	emitter->get_c_name(iface,n,sizeof(n));

	if (parent) parent->gen_parent_info(emitter,out,offset,1);

	emitter->out_printf(out,"{(struct somMethodTabStruct *)(void *)&_%s_somMTabs,",n); 

	if (iface->instanceData->children())
	{
		emitter->out_printf(out,"(short)(int)(size_t)&(((struct %s_SOMAny *)NULL)->%s_data),\n",n,n); 
	}
	else
	{
		emitter->out_printf(out,"0,");
	}

	emitter->out_printf(out,"%d,\n",*offset);

	(*offset)+=(int)methods.length();

	emitter->out_printf(out,"{(struct somMethodTabStruct *)(void *)&_%s_somMTabs,",n); 

	if (parent)
	{
		emitter->out_printf(out,"&%s_somMethodTabList},",parent->iface->id);
	}
	else
	{
		emitter->out_printf(out,"NULL},");
	}

	emitter->out_printf(out,"1}");

	if (flag) emitter->out_printf(out,",\n");
}


void kernel_class::gen_mtabList(RHBkernel_emitter *emitter,RHBoutput *out)
{
	char n[256];

	emitter->get_c_name(iface,n,sizeof(n));
	emitter->out_printf(out,"static struct somMethodTabList %s_somMethodTabList={\n",n);

	emitter->out_printf(out,"(struct somMethodTabStruct *)(void *)&_%s_somMTabs,\n",n);

	if (parent)
	{
		emitter->out_printf(out,"&%s_somMethodTabList",parent->iface->id);
	}
	else
	{
		emitter->out_printf(out,"NULL");
	}

	emitter->out_printf(out,"};\n" /*,n*/);

	emitter->out_printf(out,"static struct somMethodTabStruct *%s_somMethodTabPtr=(struct somMethodTabStruct *)(void *)&_%s_somMTabs;\n",
			n,n);
}



RHBkernel_emitter::RHBkernel_emitter(RHBrepository *rep)
:
/*RHBemitter(rep),*/
RHBheader_emitter(rep)
{
}

boolean RHBkernel_emitter::generate(RHBoutput *out,char const * /* str */)
{
	out_printf(out,"/* %s */\n",__FILE__);

	kernel_class somObject(this,get_interface("SOMObject"),NULL);
	kernel_class somClass(this,get_interface("SOMClass"),&somObject);
	kernel_class somClassMgr(this,get_interface("SOMClassMgr"),&somObject);

	somObject.gen_proto(this,out);
	somClass.gen_proto(this,out);
	somClassMgr.gen_proto(this,out);

	somObject.gen_class_object(this,out);
	somClass.gen_class_object(this,out);
	somClassMgr.gen_class_object(this,out);

	somObject.gen_mtabList(this,out);

	somObject.gen(this,out);
	somClass.gen(this,out);
	somClassMgr.gen(this,out);

	somObject.gen_somClassInfoMethodTab(this,out);
	somClass.gen_somClassInfoMethodTab(this,out);
	somClassMgr.gen_somClassInfoMethodTab(this,out);

	somObject.gen_ccd(this,out);
	somClass.gen_ccd(this,out);
	somClassMgr.gen_ccd(this,out);

	somObject.gen_cd(this,out);
	somClass.gen_cd(this,out);
	somClassMgr.gen_cd(this,out);

	return 1;
}


RHBinterface *RHBkernel_emitter::get_interface(const char *b)
{
	char buf[256];
	strncpy(buf,b,sizeof(buf));
	RHBelement *e=GetRepository()->find_named_element(NULL,buf,sizeof(buf));
	if (e) return e->is_interface();
	return NULL;
}

boolean RHBkernel_emitter::can_override_op(RHBoperation *op)
{
	static const char *special_ops[]={
		"somInit","somUninit"
	};
	int i=sizeof(special_ops)/sizeof(special_ops[0]);
	while (i--)
	{
		if (!strcmp(special_ops[i],op->id))
		{
			return 0;
		}
	}

	return 1;
}

boolean RHBkernel_emitter::is_overridden(RHBoperation *op)
{
	boolean rc=0;

	if (can_override_op(op))
	{
		RHBinterface *iface=op->parent()->is_interface();
		RHBelement *look=iface->parent()->children();

		while (look)
		{
			RHBinterface *if2=look->is_interface();

			if (if2 && (if2 != iface))
			{
				if (does_override(if2,op->id))
				{
					rc=1;

					break;
				}
			}

			look=look->next();
		}
	}

	return rc;
}

boolean RHBkernel_emitter::is_aggregate(RHBoperation *op)
{
	boolean rc=0;
	RHBtype *rt=op->return_type;

	while (rt)
	{
		RHBtype_def *td=rt->is_typedef();

		if (td)
		{
			rt=td->alias_for;
		}
		else
		{
			break;
		}
	}

	if (rt)
	{
		if (rt->is_struct() ||
			rt->is_union() || 
			rt->is_any() ||
			rt->is_sequence())
		{
			rc=1;
		}
	}

	return rc;
}

