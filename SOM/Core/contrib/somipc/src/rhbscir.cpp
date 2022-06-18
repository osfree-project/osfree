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
#include <rhbscifr.h>
#include <somirfmt.h>

#define DIRTY_ELEMENT			1
#define DIRTY_MODIFIERS			2
#define DIRTY_DESCENDENTS		4
#define DIRTY_CONTAINER			8
#define DIRTY_DELETE			16
#define DIRTY_CREATE			32
#define DIRTY_ADD				64

#ifndef SEEK_SET
#		define SEEK_SET		0
#endif

#ifndef SEEK_CUR
#		define SEEK_CUR		1
#endif

#ifndef SEEK_END
#		define SEEK_END		2
#endif

#ifdef _DEBUG
#	define SOMIPC_DEBUG(x)				x
#else
#	define SOMIPC_DEBUG(x)
#endif

/* an element may be represented by a number of records

   after loading a set, if...
		the SOMIR_element has an element but no fileOffset then it's an addition
		if the SOMIR_element has a fileOffset but no element then it's a deletion

  strategy...

  map all the known elements to the records

  now know which are the new items and which are the old items, and hence
  which are the contained/containers that are going to get changed !!!!!

  delete all the deleteables, remove from tree as do so

  add all the new items hence allocating the fileOffsets for the main record
  check the records for the containers tally

  finally regenerate and check all the modifiers etc.

  */

#define printf do not use

#if defined(_WIN32) && defined(_DEBUG)
	#define SOMIPC_ASSERT(x)	if (!(x)) { bomb("assert failure"); }
#else
	#define SOMIPC_ASSERT(x)
#endif

class SOMIR_element;
class SOMIR_record;
class SOMIR_Contained;

class SOMIR_TypeCode
{
public:
	SOMIR_TypeCode *prev;
	RHBtype *type;

	SOMIR_TypeCode(SOMIR_TypeCode *p,RHBtype *t) : prev(p),type(t)
	{
	}
};

class SOMIR_free
{
public:
	long fileOffset,length;
	SOMIR_free *next;
	SOMIR_free(long o,long l) : fileOffset(o),length(l),next(NULL)
	{

	}
};

class SOMIR_record
{
	unsigned long _alloc;
public:
	long fileOffset;
	unsigned long _length,_maximum;
	octet *_buffer;

	SOMIR_record() : 
		_alloc(0),
		fileOffset(0),
		_length(0),
		_maximum(0),
		_buffer(NULL)
	{
	}

	void empty()
	{
		if (_buffer) delete[] _buffer;
		_length=0;
		_buffer=0;
		_maximum=0;
		_alloc=0;
	}

	void set_alloc(unsigned long x)
	{
		if (x)
		{
			if ( x > _alloc)
			{
				const unsigned long align=512;

				if (x & (align-1))
				{
					x=(x+align)&~(align-1);
				}

				SOMIPC_ASSERT(x > _alloc);
				SOMIPC_ASSERT(x > _length);

				octet *p=new octet[x];

				if (_length)
				{
					memcpy(p,_buffer,_length);
				}

				if (_buffer)
				{
					delete [] _buffer;
				}

				_buffer=p;
				_alloc=x;
			}
		}
		else
		{
			empty();
		}
	}

	void update_long(long offset,long value)
	{
		long n=value;
		octet *buf=_buffer+offset;

		SOMIPC_ASSERT((offset+value) <= (long)_length);

		buf[0]=(octet)(n >> 24);
		buf[1]=(octet)(n >> 16);
		buf[2]=(octet)(n >> 8);
		buf[3]=(octet)(n);
	}

	void write_long(long n)
	{
		octet buf[4];

		buf[0]=(octet)(n >> 24);
		buf[1]=(octet)(n >> 16);
		buf[2]=(octet)(n >> 8);
		buf[3]=(octet)(n);

		write(buf,sizeof(buf));
	}

	void write_string(const char *p)
	{
		if (p)
		{
			long n=strlen(p);
			write_long(n);
			write(p,1+n);
		}
		else
		{
			write_long(-1);
			write("",1);
		}
	}

	long read_long()
	{
		octet buf[4];
		read(buf,sizeof(buf));

		return (((long)buf[0])<<24)|
			   (((long)buf[1])<<16)|
			   (((long)buf[2])<<8)|
			   (((long)buf[3]));
	}

	const char *read_string()
	{
		long slen=read_long()+1;
		char *p=NULL;
		if (slen)
		{
			SOMIPC_ASSERT((_length+slen)<=_maximum);

			p=(char *)(void *)(_buffer+_length);
			_length+=slen;
		}
		else
		{
			_length++;
		}
		return p;
	}

	void load(RHBir_file *f,long off)
	{
		empty();

		fileOffset=off;

		if (off)
		{
			_maximum=4;
		}
		else
		{
			_maximum=32;
		}

		set_alloc(_maximum);

		f->seek(off);
		f->read(_buffer,_maximum);

		if (off)
		{
			long len=read_long();

			SOMIPC_ASSERT(len < 0x20000)

			set_alloc(len);

			_length=0;
			_maximum=len;

			f->seek(off);
			f->read(_buffer,_maximum);
		}
	}

	void save(RHBir_file *f)
	{
		if (fileOffset)
		{
			update_long(0,_length);

			f->seek(fileOffset);
			f->write(_buffer,_length);
		}
	}

	void write(const void *pv,unsigned long l)
	{
		if (l)
		{
			if ((_length+l) > _maximum)
			{
				set_alloc(_length+l);
				_maximum=_length+l;
			}

			memcpy(_buffer+_length,pv,l);

			_length+=l;
		}
	}

	void read(void *pv,unsigned long l)
	{
		SOMIPC_ASSERT(l);
		SOMIPC_ASSERT(l <= (_maximum-_length));

		if (l && (l <= (_maximum-_length)))
		{
			memcpy(pv,_buffer+_length,l);
			_length+=l;
		}
	}

	~SOMIR_record()
	{
		empty();
	}
};

class SOMIR_element
{
	long _dirty;						/* dirty bits */
	char *_name;
public:
	const char *name(void ) { return _name; }
	SOMIR_element *children;		/* children */
	SOMIR_element *next;			/* peer list */
	SOMIR_element *parent;			/* parent */
	long fileOffset;				/* actual offset in file, zero in unknown */
	RHBelement *element;			/* what this is supposed to represent */

	long dirty() { return _dirty; }
	void dirty(long x) { _dirty|=x; }

	SOMIR_element(RHBelement *el) : 
		_dirty(0),
		_name(NULL),
		children(NULL),
		next(NULL),
		parent(NULL),
		fileOffset(0),
		element(el)
	{
		if (el) _name=el->id;
	}

	void containedField(long offset,long value);
	long containedField(long offset);

	void print_id(FILE *fp)
	{
		if (parent) 
		{
			parent->print_id(fp);
			fprintf(fp,"::%s",_name);
		}
	}

	void set_name(const char *n)
	{
		size_t len=strlen(n)+1;
		_name=new char[len];
		memcpy(_name,n,len);
	}

	virtual ~SOMIR_element()
	{
		while (children)
		{
			SOMIR_element *p=children;
			children=children->next;
			delete p;
		}

		if (!element)
		{
			if (_name) delete[] _name;
		}
	}

	long containerOffset();
	void containerOffset(long);
	long operation_mode();
	long attribute_mode();
	long parameter_mode();

	void write_element_record(SOMIR_record *);

	long count()
	{
		long i=0;
		SOMIR_element *p=children;
		while (p)
		{
			i++;
			p=p->next;
		}
		return i;
	}

	void add(SOMIR_element *r)
	{
		SOMIPC_ASSERT(!r->parent)

		r->parent=this;

		if (children)
		{
			SOMIR_element *p=children;
			while (p->next) p=p->next;
			p->next=r;
		}
		else
		{
			children=r;
		}
	}

	void populate(const char *);

	void locate_children(SOMIR_record *container_buffer,boolean include_unknowns);

	void locate_all_children();

	void write_TypeCode(RHBtype *t,SOMIR_record *,SOMIR_TypeCode *ctx=0);

	virtual long _get_SOMIR_type()=0;
	virtual void dirty_delete(RHBir_file *,SOMIR_Contained *)=0;

	void update_containers();
	void update_elements();
	void update_modifiers();

#define CONTAINED_OVERRIDES    \
		void write_modifiers(SOMIR_record *); \
		long _get_SOMIR_type(); \
		void dirty_delete(RHBir_file *,SOMIR_Contained *);

	void purge(RHBir_file *f);
	void dirty_create(RHBir_file *f);

protected:
	virtual RHBir_emitter *emitter() 
	{
		if (parent) return parent->emitter();

		return NULL; 
	}		/* emitter */
};

class SOMIR_Repository : public SOMIR_element
{
private:
	RHBir_emitter *_emitter;
protected:
	RHBir_emitter *emitter() { return _emitter; }
public:
	SOMIR_Repository(RHBir_emitter *e) : 
		SOMIR_element(e->GetRepository()),
		_emitter(e)
	{
		
	}

	CONTAINED_OVERRIDES

	void update_depends();
};

class SOMIR_TypeDef : public SOMIR_element
{
public:
	SOMIR_TypeDef(RHBtype *c) :
	   SOMIR_element(c)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_ConstantDef : public SOMIR_element
{
public:
	SOMIR_ConstantDef(RHBconstant *c) :
	   SOMIR_element(c)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_ExceptionDef : public SOMIR_element
{
public:
	SOMIR_ExceptionDef(RHBexception *ex) :
	   SOMIR_element(ex)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_AttributeDef : public SOMIR_element
{
public:
	SOMIR_AttributeDef(RHBattribute *attr) :
	   SOMIR_element(attr)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_ParameterDef : public SOMIR_element
{
public:
	SOMIR_ParameterDef(RHBparameter *param) :
	   SOMIR_element(param)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_ModuleDef: public SOMIR_element
{
public:
	SOMIR_ModuleDef(RHBmodule *mod) :
	   SOMIR_element(mod)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_OperationDef: public SOMIR_element
{
public:
	SOMIR_OperationDef(RHBoperation *iface) :
	   SOMIR_element(iface)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_InterfaceDef: public SOMIR_element
{
public:
	SOMIR_InterfaceDef(RHBinterface *iface) :
	   SOMIR_element(iface)
   {
   }

	CONTAINED_OVERRIDES
};

class SOMIR_Contained
{
public:
	long length;
	long type;
	char *name;
	long zero1;
	long definedInOffset;
	long descendantsOffset;
	long modifiersOffset;
	union
	{
		struct
		{
			long contentsOffset;
		} interfaceDef;
		struct
		{
			long contentsOffset;
		} moduleDef;
		struct
		{  
			long contentsOffset;
			long zero3;
			long mode;
		} operationDef;
		struct
		{
			long mode;
		} attributeDef;
		struct
		{
			long mode;
		} parameterDef;
	} u;

	long contentsOffset(void)
	{
		switch (type)
		{
		case kSOMIR_InterfaceDef: return u.interfaceDef.contentsOffset;
		case kSOMIR_OperationDef: return u.operationDef.contentsOffset;
		case kSOMIR_ModuleDef: return u.moduleDef.contentsOffset;
		}

		return 0;
	}

	SOMIR_Contained(SOMIR_record *r)
	{
		name=NULL;

		length=r->read_long();
		type=r->read_long();

		SOMIPC_ASSERT((type>=0)&&(type<=11));
		SOMIPC_ASSERT(type!=kSOMIR_Container);
		SOMIPC_ASSERT(type!=kSOMIR_Contained);
		SOMIPC_ASSERT(type!=kSOMIR_Empty);

		long stringLen=1+r->read_long();

		SOMIPC_ASSERT(stringLen);
		SOMIPC_ASSERT(stringLen < 256);

		if (stringLen)
		{
			name=new char[stringLen];

			SOMIPC_ASSERT(name);

			r->read(name,stringLen);
		}

		zero1=r->read_long();
		definedInOffset=r->read_long();
		descendantsOffset=r->read_long();
		modifiersOffset=r->read_long();

		switch (type)
		{
		case kSOMIR_InterfaceDef:
			u.interfaceDef.contentsOffset=r->read_long();
			break;
		case kSOMIR_ModuleDef:
			u.moduleDef.contentsOffset=r->read_long();
			break;
		case kSOMIR_ParameterDef:
			u.parameterDef.mode=r->read_long();
			break;
		case kSOMIR_OperationDef:
			u.operationDef.contentsOffset=r->read_long();
			u.operationDef.zero3=r->read_long();
			u.operationDef.mode=r->read_long();
			break;
		case kSOMIR_AttributeDef:
			u.attributeDef.mode=r->read_long();
			break;
		case kSOMIR_ExceptionDef:
		case kSOMIR_ConstantDef:
		case kSOMIR_TypeDef:
			break;
		default:
			SOMIPC_ASSERT(!this);
			break;
		}

		SOMIPC_ASSERT(r->_length==r->_maximum);
	}

	~SOMIR_Contained()
	{
		if (name) delete [] name;
	}
};


RHBir_emitter::~RHBir_emitter()
{
}

RHBir_file::~RHBir_file()
{
	if (fp)
	{
		fclose(fp);
		fp=NULL;
	}

	while (freeListItems)
	{
		SOMIR_free *p=freeListItems;
		freeListItems=freeListItems->next;
		delete p;
	}
}


RHBir_file::RHBir_file(const char *name)
{
#if defined(_PLATFORM_WIN32_) || \
	defined(_PLATFORM_OS2_)
	const char *rdwr_mode="r+b";
#else
	const char *rdwr_mode="r+";
#	ifdef _WIN32
#		error should not be here
#	endif
#endif

	strncpy(filename,name,sizeof(filename));

	freeListItems=NULL;
	fileLength=0;

	fp=fopen(name,rdwr_mode);

#ifdef _WIN32
	SOMIPC_ASSERT(fp)
#endif

	if (!fp)
	{
		fprintf(stderr,"Failed to open '%s' for IR updating\n",name);
		exit(1);
	}

	fseek(fp,0L,SEEK_END);
	fileLength=ftell(fp);
}

void RHBir_file::write(const void *p,size_t len)
{
	if (fp)
	{
		if (len)
		{
			fwrite(p,len,1,fp);
		}
	}
}

RHBir_emitter::RHBir_emitter(RHBrepository *rr,RHBir_file *file) :
RHBemitter(rr),
irFile(file)
{
}

boolean RHBir_emitter::generate(RHBoutput *,const char *f)
{
	idl_name=f;

	if (!irFile->load())
	{
		fprintf(stderr,"file \"%s\" is not a SOMIR file\n",
			irFile->_get_filename());

		exit(1);
	}

	SOMIR_Repository irRepository(this);

	irRepository.populate("");

	irRepository.locate_all_children();

	irRepository.purge(irFile);

	irRepository.dirty_create(irFile);

	irRepository.update_containers();
	irRepository.update_elements();
	irRepository.update_modifiers();
	irRepository.update_depends();

	return 1;
}

void SOMIR_element::populate(const char *repos_id)
{
	RHBelement *el=element->children();

/*	printf("populate(%s)\n",repos_id); */

	while (el)
	{
		RHBtype *type=el->is_type();
		
		if (type || el->is_module())
		{
			SOMIR_element *ch=NULL;

			if (el->is_module())
			{
				RHBmodule *mod=el->is_module();

				if (mod)
				{
					ch=new SOMIR_ModuleDef(mod);
				}
			}

			if (!ch)
			{
				RHBinterface *iface=type->is_interface();
		
				if (iface) 
				{
					if (iface->instanceData)
					{
						ch=new SOMIR_InterfaceDef(iface);
					}
				}
			}

			if (!ch)
			{
				RHBoperation *op=type->is_operation();

				if (op)
				{
					ch=new SOMIR_OperationDef(op);
				}
			}

			if (!ch)
			{
				RHBparameter *param=type->is_parameter();

				if (param)
				{
					ch=new SOMIR_ParameterDef(param);
				}
			}

			if (!ch)
			{
				RHBattribute *attr=type->is_attribute();

				if (attr)
				{
					ch=new SOMIR_AttributeDef(attr);
				}
			}

			if (!ch)
			{
				RHBmodule *mod=type->is_module();

				if (mod)
				{
					ch=new SOMIR_ModuleDef(mod);
				}
			}

			if (!ch)
			{
				RHBexception *ex=type->is_exception();

				if (ex)
				{
					ch=new SOMIR_ExceptionDef(ex);
				}
			}

			if (!ch)
			{
				RHBtype_def *td=type->is_typedef();

				if (td)
				{
					ch=new SOMIR_TypeDef(td);
				}
			}

			if (!ch)
			{
				RHBconstant *c=type->is_constant();

				if (c)
				{
					ch=new SOMIR_ConstantDef(c);
				}
			}

			if (!ch)
			{
				RHBenum *td=type->is_enum();

				if (td)
				{
					ch=new SOMIR_TypeDef(td);
				}
			}

			if (!ch)
			{
				RHBstruct *td=type->is_struct();

				if (td)
				{
					ch=new SOMIR_TypeDef(td);
				}
			}

			if (!ch)
			{
				RHBunion *td=type->is_union();

				if (td)
				{
					ch=new SOMIR_TypeDef(td);
				}
			}

			if (ch)
			{
				char buf[256];

				snprintf(buf,sizeof(buf),"%s::%s",repos_id,ch->element->id);

				add(ch);

				ch->populate(buf);
			}
		}

		el=el->next();
	}
}

boolean RHBir_file::load()
{
	SOMIR_record r;

	r.load(this,0);

	long magic1=r.read_long();
	long magic2=r.read_long();
	rootContainer=r.read_long();
	long freeList=r.read_long();

	if ((magic1==kSOMIR_Magic1)
		&&
		(magic2==kSOMIR_Magic2))
	{
		if (freeList)
		{
			long freeOffset=freeList;
			SOMIR_free *last=NULL;

			while (freeOffset)
			{
				r.load(this,freeOffset);

				long len=r.read_long();
				long typ=r.read_long();
				long off=r.read_long();

				SOMIPC_ASSERT(typ==kSOMIR_Empty);

				if (typ==kSOMIR_Empty)
				{
					SOMIR_free *f=new SOMIR_free(freeOffset,len);

					if (last)
					{
						last->next=f;
					}
					else
					{
						freeListItems=f;
					}

					last=f;
				}

				freeOffset=off;
			}
		}

		return 1;
	}

	return 0;
}

void RHBir_file::seek(long off)
{
	fseek(fp,off,SEEK_SET);
}

void RHBir_file::read(void *pv,unsigned long len)
{
	fread(pv,len,1,fp);
}

void SOMIR_element::locate_all_children(void)
{
	SOMIR_record r;

	if (parent)
	{
		if (fileOffset)
		{
			/* this is some contained element */

			r.load(emitter()->irFile,fileOffset);

			SOMIR_Contained cnd(&r);

			SOMIPC_ASSERT(cnd.definedInOffset==parent->fileOffset);

			long contentsOffset=cnd.contentsOffset();

			if (contentsOffset)
			{
				r.load(emitter()->irFile,contentsOffset);

				locate_children(&r,1);
			}
		}
		else
		{
			dirty(DIRTY_CREATE);
			parent->dirty(DIRTY_CONTAINER);
/*			printf("element \"%s\" is new\n",name());*/
		}
	}
	else
	{
		/* this is the repository */

		long off=containerOffset();
		
		if (off)
		{
			SOMIPC_ASSERT(emitter()->irFile->rootContainer==off)

			r.load(emitter()->irFile,off);
		}
		else
		{
			r.empty();
			r.write_long(12);
			r.write_long(kSOMIR_Container);
			r.write_long(0);

			r._length=0;
		}

		locate_children(&r,0);
	}

	SOMIR_element *el=children;

	while (el)
	{
		el->locate_all_children();

		el=el->next;
	}
}

void SOMIR_element::locate_children(SOMIR_record *r,boolean include_unknowns)
{
	SOMIPC_DEBUG(long length=)r->read_long();
	SOMIPC_DEBUG(long type=)r->read_long();
	long items=r->read_long();

	SOMIPC_ASSERT(type==kSOMIR_Container)

	while (items--)
	{
		long offsetContained=r->read_long();
		long typeContained=r->read_long();
		long stringLen=r->read_long();
		char buf[256];

		if (stringLen > 0)
		{
			SOMIR_element *el=children;

			r->read(buf,stringLen+1);

			while (el)
			{
				if (!strcmp(el->name(),buf))
				{
					if (typeContained==el->_get_SOMIR_type())
					{
						/* it has to match the exact type as well */

						break;
					}
				}

				el=el->next;
			}

			if (include_unknowns && !el)
			{
/*				printf("element %s is to be deleted\n",buf);*/

				switch (typeContained)
				{
				case kSOMIR_InterfaceDef:
					el=new SOMIR_InterfaceDef(NULL);
					break;
				case kSOMIR_TypeDef:
					el=new SOMIR_TypeDef(NULL);
					break;
				case kSOMIR_ModuleDef:
					el=new SOMIR_ModuleDef(NULL);
					break;
				case kSOMIR_OperationDef:
					el=new SOMIR_OperationDef(NULL);
					break;
				case kSOMIR_AttributeDef:
					el=new SOMIR_AttributeDef(NULL);
					break;
				case kSOMIR_ExceptionDef:
					el=new SOMIR_ExceptionDef(NULL);
					break;
				case kSOMIR_ParameterDef:
					el=new SOMIR_ParameterDef(NULL);
					break;
				case kSOMIR_ConstantDef:
					el=new SOMIR_ConstantDef(NULL);
					break;
				default:
					SOMIPC_ASSERT(el);
					break;
				}
				/* need to create an empty one here if not a root repository */

				SOMIPC_ASSERT(el);

				el->set_name(buf);
				el->dirty(DIRTY_DELETE);

				add(el);

				dirty(DIRTY_CONTAINER);
			}

			if (el) 
			{
				el->fileOffset=offsetContained;
			}
		}
		else
		{
			r->read(buf,1);
		}
	}

	if (children)
	{
		SOMIR_element *el=children;

		while (el)
		{
			if (!el->fileOffset)
			{
/*				printf("new item \"%s\"\n",el->name());*/

				el->dirty(DIRTY_CREATE|DIRTY_ADD);

				dirty(DIRTY_CONTAINER);
			}

			el=el->next;
		}
	}
}

long SOMIR_Repository::_get_SOMIR_type()
{
	SOMIPC_ASSERT(parent);

	return kSOMIR_Container;
}

long SOMIR_TypeDef::_get_SOMIR_type() { return kSOMIR_TypeDef; }
long SOMIR_InterfaceDef::_get_SOMIR_type() { return kSOMIR_InterfaceDef; }
long SOMIR_OperationDef::_get_SOMIR_type() { return kSOMIR_OperationDef; }
long SOMIR_AttributeDef::_get_SOMIR_type() { return kSOMIR_AttributeDef; }
long SOMIR_ParameterDef::_get_SOMIR_type() { return kSOMIR_ParameterDef; }
long SOMIR_ConstantDef::_get_SOMIR_type() { return kSOMIR_ConstantDef; }
long SOMIR_ModuleDef::_get_SOMIR_type() { return kSOMIR_ModuleDef; }
long SOMIR_ExceptionDef::_get_SOMIR_type() { return kSOMIR_ExceptionDef; }

void SOMIR_Repository::dirty_delete(RHBir_file *,SOMIR_Contained *)
{
	SOMIPC_ASSERT(!this);
}

void SOMIR_ParameterDef::dirty_delete(RHBir_file *,SOMIR_Contained *)
{
}

void SOMIR_InterfaceDef::dirty_delete(RHBir_file *irFile,SOMIR_Contained *cnd)
{
	if (cnd->u.interfaceDef.contentsOffset)
	{
		SOMIR_record r;

		r.load(irFile,cnd->u.interfaceDef.contentsOffset);

		SOMIPC_DEBUG(long len=)r.read_long();
		long typ=r.read_long();

		irFile->free(cnd->u.interfaceDef.contentsOffset,typ);

		cnd->u.interfaceDef.contentsOffset=0;
	}
}

void SOMIR_ModuleDef::dirty_delete(RHBir_file *irFile,SOMIR_Contained *cnd)
{
	if (cnd->u.moduleDef.contentsOffset)
	{
		SOMIR_record r;

		r.load(irFile,cnd->u.moduleDef.contentsOffset);

		SOMIPC_DEBUG(long len=)r.read_long();
		long typ=r.read_long();

		irFile->free(cnd->u.moduleDef.contentsOffset,typ);

		cnd->u.moduleDef.contentsOffset=0;
	}
}

void SOMIR_AttributeDef::dirty_delete(RHBir_file *,SOMIR_Contained *)
{
}

void SOMIR_OperationDef::dirty_delete(RHBir_file *irFile,SOMIR_Contained *cnd)
{
	/* delete the container of parameters */

	if (cnd->u.operationDef.contentsOffset)
	{
		SOMIR_record r;

		r.load(irFile,cnd->u.operationDef.contentsOffset);

		SOMIPC_DEBUG(long len=)r.read_long();
		long typ=r.read_long();

		irFile->free(cnd->u.operationDef.contentsOffset,typ);

		cnd->u.operationDef.contentsOffset=0;
	}
}

void SOMIR_ExceptionDef::dirty_delete(RHBir_file *,SOMIR_Contained *)
{
}

void SOMIR_ConstantDef::dirty_delete(RHBir_file *,SOMIR_Contained *)
{
}

void SOMIR_TypeDef::dirty_delete(RHBir_file *,SOMIR_Contained *)
{
}

void SOMIR_element::purge(RHBir_file *irFile)
{
	SOMIR_element *el=children;

	while (el)
	{
		el->purge(irFile);

		el=el->next;
	}

	if (_dirty & DIRTY_DELETE)
	{
		SOMIPC_ASSERT(fileOffset);
		SOMIPC_ASSERT(!element);

		SOMIPC_ASSERT(parent->dirty() & DIRTY_CONTAINER);

		if (fileOffset)
		{
			SOMIR_record r;
			
			r.load(irFile,fileOffset);

			SOMIR_Contained cnd(&r);

			dirty_delete(irFile,&cnd);

			SOMIPC_ASSERT(cnd.modifiersOffset);

			irFile->free(fileOffset,cnd.type);

			fileOffset=0;

			if (cnd.modifiersOffset)
			{
				r.load(irFile,cnd.modifiersOffset);

				SOMIPC_DEBUG(long modlen=)r.read_long();
				long modtyp=r.read_long();

				irFile->free(cnd.modifiersOffset,modtyp);

				cnd.modifiersOffset=0;
			}

			if (cnd.descendantsOffset)
			{
				r.load(irFile,cnd.descendantsOffset);

				SOMIPC_DEBUG(long desclen=)r.read_long();
				long desctyp=r.read_long();

				irFile->free(cnd.descendantsOffset,desctyp);

				cnd.descendantsOffset=0;
			}
		}
	}
}

void RHBir_file::free(long off,long 
#ifdef _DEBUG
					  typ
#endif
					  )
{
	long write_off=kSOMIR_free_root;
	long len=0;

	SOMIPC_ASSERT(typ!=kSOMIR_Empty);
	SOMIPC_ASSERT((typ>=0)&&(typ<=11));
	SOMIPC_ASSERT(off);

/*	printf("RHBir_file::free(%ld,%ld)\n",off,typ); */

	{
		SOMIR_record check;

		check.load(this,off);

		long length=check.read_long();
		SOMIPC_DEBUG(long type=)check.read_long();

		SOMIPC_ASSERT(typ==type);

		len=length;

/*		fprintf(stderr,"freeing %ld bytes\n",len);*/
	}

	SOMIR_record r;

	r.write_long(kSOMIR_Empty);
	r.write_long(0);

	/* not changing length, so offset of four */

	seek(off+4);

	SOMIPC_ASSERT(r._length==8);

	write(r._buffer,r._length);

	SOMIR_free *f=new SOMIR_free(off,len);

	if (freeListItems)
	{
		SOMIR_free *p=freeListItems;

		while (p->next) p=p->next;

		p->next=f;

		write_off=p->fileOffset+8;
	}
	else
	{
		freeListItems=f;
	}

	seek(write_off);

	r.empty();

	r.write_long(off);

	SOMIPC_ASSERT(r._length==4);

	write(r._buffer,r._length);
}

void SOMIR_element::dirty_create(RHBir_file *f)
{
	if (_dirty & DIRTY_CREATE)
	{
		long type=_get_SOMIR_type();
		long slen=1+strlen(name());
		long len=4+4+4+slen; /* length,type,name */
		len+=16; /* zero1,definedInOffset,descendantsOffset,modifiersOffset */

		SOMIPC_ASSERT(!fileOffset);
		SOMIPC_ASSERT(element);

		switch (type)
		{
		case kSOMIR_InterfaceDef: len+=4; /* contentsOffset */ break;
		case kSOMIR_ModuleDef: len+=4; /* contentsOffset */ break;
		case kSOMIR_OperationDef: len+=12; /* contentsOffset,zero3,mode */ break;
		case kSOMIR_ParameterDef: len+=4; /* mode */ break;
		case kSOMIR_AttributeDef: len+=4; /* mode */ break;
		default:
			break;
		}

		fileOffset=f->alloc(len,__FILE__,__LINE__);

		SOMIPC_ASSERT(fileOffset);

		SOMIR_record r;

		r.fileOffset=fileOffset;

		write_element_record(&r);

		SOMIPC_ASSERT(r._length==(unsigned long)len);

		r.save(f);

		_dirty&=~DIRTY_CREATE;
	}

	SOMIR_element *el=children;

	while (el)
	{
		el->dirty_create(f);

		el=el->next;
	}
}

long RHBir_file::alloc(long len,
					   const char * /* filename */,
					   int /* line */ )
{
	SOMIR_free *f=freeListItems;
	SOMIR_free *prev=NULL;
	long write_off=kSOMIR_free_root;
	long value=0;
	long retVal=0;

	SOMIPC_ASSERT(len >= 12)

	while (f)
	{
		if (f->length==len)
		{
			break;
		}

		prev=f;
		f=f->next;
	}

	if (f)
	{
		/* reallocate by rechaining */

		retVal=f->fileOffset;

/*		printf("unchaining %ld,%ld\n",f->fileOffset,len); */

		if (prev)
		{
			write_off=prev->fileOffset+kSOMIR_free_next;

			prev->next=f->next;
		}
		else
		{
			freeListItems=f->next;
		}

		if (f->next)
		{
			value=f->next->fileOffset;
		}

		delete f;
	}
	else
	{
		/* allocate from the end */

/*		if (filename)
		{
			fprintf(stderr,"%s:%d, allocating %ld from end at %ld\n",filename,line,len,fileLength);
		}
*/

		SOMIR_record r2;

		r2.write_long(len);
		r2.write_long(kSOMIR_Empty);

		r2.set_alloc(len);

		{
			unsigned long i=len-r2._length;

			while (i)
			{
				static char buf[256];
				unsigned long n=(i > sizeof(buf)) ? sizeof(buf) : i;
				r2.write(buf,n);
				i-=n;
			}
		}

		SOMIPC_ASSERT(r2._length==(unsigned long)len);

		seek(fileLength);

		retVal=fileLength;

		long newLen=fileLength+len;

		write(r2._buffer,r2._length);

		fileLength=newLen;

		if (prev)
		{
			write_off=prev->fileOffset+kSOMIR_free_next;
		}
	}

	SOMIPC_ASSERT(write_off);

	seek(write_off);

	SOMIR_record r;

/*	printf("writing %ld at %ld\n",value,write_off);*/

	r.write_long(value);

	SOMIPC_ASSERT(r._length==4);

	write(r._buffer,r._length);

	return retVal;
}

void SOMIR_element::write_element_record(SOMIR_record *r)
{
	long type=_get_SOMIR_type();

	r->empty();

	r->write_long(0);
	r->write_long(type);
	r->write_string(name());

	SOMIPC_ASSERT(dirty() & DIRTY_CREATE);

	r->write_long(0); /* zero1 */
	r->write_long(parent->fileOffset);
	r->write_long(0); /* descendantsOffset */
	r->write_long(0); /* modifiers */

	switch (type)
	{
	case kSOMIR_InterfaceDef:
		r->write_long(0); /* contents */
		break;
	case kSOMIR_ModuleDef:
		r->write_long(0); /* contents */
		break;
	case kSOMIR_OperationDef:
		r->write_long(0); /* contents */
		r->write_long(0); /* zero3 */
		r->write_long(operation_mode()); /* mode */
		break;
	case kSOMIR_AttributeDef:
		r->write_long(attribute_mode()); /* mode */
		break;
	case kSOMIR_ParameterDef:
		r->write_long(parameter_mode()); /* mode */
		break;
	}

	r->update_long(0,r->_length);
}

void SOMIR_element::update_containers()
{
	RHBir_file *file=emitter()->irFile;
	SOMIR_element *el=children;

	if (parent)
	{
		if (fileOffset && element)
		{
			SOMIPC_ASSERT(fileOffset);

			/* then want exact match */

			if (dirty() & DIRTY_CONTAINER)
			{
				long nItems=0;
				SOMIR_record cnr;
				long newCnr=0;

/*				printf("DIRTY_CONTAINER: "); print_id(); printf("\n");*/

				long oldCnr=containerOffset();

				if (oldCnr)
				{
					file->free(oldCnr,kSOMIR_Container);

					oldCnr=0;
				}

				cnr.write_long(0);
				cnr.write_long(kSOMIR_Container);
				cnr.write_long(0);

				while (el)
				{
					if (el->fileOffset && el->element)
					{
/*						el->print_id(); printf("\n");*/

						SOMIPC_ASSERT(!(el->dirty() & DIRTY_DELETE))

						cnr.write_long(el->fileOffset);
						cnr.write_long(el->_get_SOMIR_type());
						cnr.write_string(el->name());

						nItems++;
					}

					el=el->next;
				}

				if (nItems)
				{
					cnr.update_long(0,cnr._length);
					cnr.update_long(8,nItems);

					newCnr=file->alloc(cnr._length,__FILE__,__LINE__);

					file->seek(newCnr);
					file->write(cnr._buffer,cnr._length);
				}

				containerOffset(newCnr);
			}
		}
	}
	else
	{
		/* need to add a single item */

		long toAdd=0;

		while (el)
		{
			if (el->dirty() & DIRTY_ADD) 
			{
/*				printf("add %s to root\n",el->name());*/
				toAdd++;
			}

			el=el->next;
		}

		if (toAdd)
		{
			long total=0;
/*			long totalOffset=0;*/
			SOMIR_record rootCnr;
			long originalLength=0;

			if (file->rootContainer)
			{
				rootCnr.load(file,file->rootContainer);
				originalLength=rootCnr._maximum;

				SOMIPC_DEBUG(long len=)rootCnr.read_long();

				SOMIPC_ASSERT(len==originalLength);

				SOMIPC_DEBUG(long type=)rootCnr.read_long();

				SOMIPC_ASSERT(type==kSOMIR_Container);

				long nItems=rootCnr.read_long();

				while (nItems--)
				{
					SOMIPC_DEBUG(long itemOffset=)rootCnr.read_long();
					SOMIPC_DEBUG(long itemType=)rootCnr.read_long();

					SOMIPC_ASSERT((itemOffset >= 32)&&(itemOffset<file->_get_fileLength()));
					SOMIPC_ASSERT((itemType>=0)&&(itemType<=7));

					rootCnr.read_string();

					total++;
				}

				SOMIPC_ASSERT(len==(long)rootCnr._length)

				file->free(file->rootContainer,kSOMIR_Container);
			}
			else
			{
				rootCnr.write_long(0);	/* length */
				rootCnr.write_long(kSOMIR_Container); /* type */
				rootCnr.write_long(toAdd);	/* n items */
			}

			el=children;

			while (el)
			{
				if (el->dirty() & DIRTY_ADD)
				{
/*					printf("root - DIRTY_ADD: ");	el->print_id(); printf("\n");*/

					SOMIPC_ASSERT(el->fileOffset);
					SOMIPC_ASSERT(el->element);

					rootCnr.write_long(el->fileOffset);
					rootCnr.write_long(el->_get_SOMIR_type());
					rootCnr.write_string(el->name());

					total++;
				}


				el=el->next;
			}

			if (!originalLength) {}	/* purely to avoid warning */

			rootCnr.fileOffset=file->alloc(rootCnr._length,__FILE__,__LINE__);

			rootCnr.update_long(0,rootCnr._length);
			rootCnr.update_long(8,total);

			rootCnr.save(file);

			containerOffset(rootCnr.fileOffset);
		}
	}

	el=children;

	while (el)
	{
		el->update_containers();

		el=el->next;
	}
}

long SOMIR_element::containerOffset()
{
	if (parent)
	{
		if (fileOffset)
		{
			SOMIR_record r;

			r.load(emitter()->irFile,fileOffset);

			SOMIR_Contained cnd(&r);

			SOMIPC_ASSERT(cnd.type==_get_SOMIR_type());

			switch (cnd.type)
			{
			case kSOMIR_InterfaceDef:
				return cnd.u.interfaceDef.contentsOffset;
			case kSOMIR_ModuleDef:
				return cnd.u.moduleDef.contentsOffset;
			case kSOMIR_OperationDef:
				return cnd.u.operationDef.contentsOffset;
			default:
				SOMIPC_ASSERT(fileOffset);
				break;
			}
		}
	}
	else
	{
		return emitter()->irFile->rootContainer;
	}

	return 0;
}

void SOMIR_element::containedField(long offset,long value)
{
	offset+=1+strlen(name())+fileOffset;

	SOMIPC_ASSERT(fileOffset);

	SOMIR_record r;
	
	r.write_long(value);
	
	RHBir_file *file=emitter()->irFile;

	file->seek(offset);
	file->write(r._buffer,r._length);
}

long SOMIR_element::containedField(long offset)
{
	offset+=1+strlen(name())+fileOffset;

	SOMIPC_ASSERT(fileOffset);

	SOMIR_record r;
	
	r.write_long(offset);
	
	RHBir_file *file=emitter()->irFile;

	file->seek(offset);
	file->read(r._buffer,r._length);

	r._length=0;

	return r.read_long();
}

void SOMIR_element::containerOffset(long off)
{
	RHBir_file *file=emitter()->irFile;

	if (parent)
	{
		switch (_get_SOMIR_type())
		{
		case kSOMIR_InterfaceDef:
		case kSOMIR_OperationDef:
		case kSOMIR_ModuleDef:
			containedField(28,off);
			break;
		default:
			SOMIPC_ASSERT(!file);
			break;
		}
	}
	else
	{
		SOMIR_record rootCnr;

		file->rootContainer=off;

		rootCnr.write_long(file->rootContainer);

		file->seek(8);

		SOMIPC_ASSERT(rootCnr._length==4);

		file->write(rootCnr._buffer,rootCnr._length);
	}

#ifdef _DEBUG
	if (parent && fileOffset)
	{
		long chkoff=containerOffset();
	
		SOMIPC_ASSERT(chkoff==off);

		SOMIR_record r;

		r.load(file,fileOffset);

		SOMIR_Contained cnd(&r);

		SOMIPC_ASSERT(cnd.u.moduleDef.contentsOffset==off);
	}
#endif
}

long SOMIR_element::operation_mode()
{
	return element->is_operation()->oneWay ? 2 : 1;
}

long SOMIR_element::attribute_mode()
{
	return element->is_attribute()->readonly ? 2 : 1;
}

long SOMIR_element::parameter_mode()
{
	long mode=1;
	RHBparameter *param=element->is_parameter();

	if (!strcmp(param->mode,"out"))
	{
		mode=2;
	}

	if (!strcmp(param->mode,"inout"))
	{
		mode=3;
	}
	return mode;
}

void SOMIR_element::update_modifiers()
{
	RHBir_file *file=emitter()->irFile;

	if (element && fileOffset)
	{
		SOMIR_record r;
		RHBelement *q=element->children();

/*		printf("update_modifiers: "); print_id(); printf("\n");
*/

/*		if (_get_SOMIR_type()==kSOMIR_ModuleDef)
		{
			__asm int 3;
		}
*/
		r.write_long(0);
		r.write_long(kSOMIR_Contained);
		r.write_long(fileOffset);
		r.write_long(0);
		
		long nModifiers=0;
		long needs_file=1;
		long needs_line=1;
		const char *filename=element->defined_in->id;

		switch (_get_SOMIR_type())
		{
		case kSOMIR_AttributeDef:
		case kSOMIR_ParameterDef:
		case kSOMIR_OperationDef:
			if (parent->element)
			{
				if (parent->element->defined_in)
				{
					const char *q=parent->element->defined_in->id;

					if (q && filename)
					{
						if (!strcmp(q,filename)) 
						{
							needs_file=0;
						}
					}
				}
			}
		default:
			break;
		}

		while (q)
		{
			if (q->is_modifier()) 
			{
				if (!strcmp(q->id,"file")) 
				{
					needs_file=0;
				}

				if (!strcmp(q->id,"line")) 
				{
					needs_line=0;
				}

				nModifiers++;
			}

			q=q->next();
		}

		long total_mods=nModifiers+needs_file+needs_line;

		r.write_long(total_mods);

		if (total_mods)
		{
			q=element->children();

			if (needs_file)
			{
				r.write_string("file");
				r.write_string(filename);
			}

			if (needs_line)
			{
				char linenum[23];
				snprintf(linenum,sizeof(linenum),"%ld",(long)element->defined_line);
				r.write_string("line");
				r.write_string(linenum);
			}

			long i=0;

			while (i < nModifiers)
			{
				RHBmodifier *mod=q->is_modifier();

				if (mod)
				{
					const char *name=mod->id;

					r.write_string(name);

/*					if (!strcmp(name,"dllname"))
					{

						__asm int 3;
					}
*/
					{
						SOMIR_record s;
						long j=0;

						while (j < mod->modifier_data.length())
						{
							const char *p=mod->modifier_data.get(j);

							if (p && p[0]) s.write(p,strlen(p));

							j++;
						}

						if (s._length)
						{
							long dequote=0;

							s.write("",1);

							long len=strlen((const char *)s._buffer);

							if ((s._buffer[0]=='\"')&&
								(s._buffer[len-1]=='\"'))
							{
								dequote=1;
								s._buffer[len-1]=0;
							}

							r.write_string((const char *)s._buffer+dequote);
						}
						else
						{
							r.write_string("");
						}
					}

					i++;
				}

				q=q->next();

				if (!q) break;
			}
		}

		switch (_get_SOMIR_type())
		{
		case kSOMIR_ModuleDef:
			break;
		case kSOMIR_InterfaceDef:
			{
				/* base_interfaces */
				RHBinterface *iface=element->is_interface();
				long n=iface->_parents.length();

				r.write_long(n);

				long i=0;

				while (i < n)
				{
					char buf[256];

					emitter()->get_ir_name(iface->_parents.get(i),buf,sizeof(buf));

					r.write_string(buf);

					i++;
				}
				/* instanceData */

				if (iface->instanceData)
				{

					if (!iface->instanceData->id)
					{
					}
				}

				write_TypeCode(iface->instanceData,&r);
			}
			break;
		case kSOMIR_OperationDef:
			{
				RHBoperation *op=element->is_operation();

				/* return type */
				write_TypeCode(op->return_type,&r);
				/* sequence(contexts) */

				{
					long n=op->context_list.length();
					long i=0;
					r.write_long(n);
					while (i < n)
					{
						char buf[256];

						emitter()->get_ir_name(op->context_list.get(i),buf,sizeof(buf));

						r.write_string(buf);

						i++;
					}
				}

				/* sequence(raises) */

				{
					long n=op->exception_list.length();
					long i=0;
					r.write_long(n);
					while (i < n)
					{
						char buf[256];

						emitter()->get_ir_name(op->exception_list.get(i),buf,sizeof(buf));

						r.write_string(buf);

						i++;
					}
				}
			}
			break;
		case kSOMIR_AttributeDef:
			{
				RHBattribute *attr=element->is_attribute();
				/* attr type */
				write_TypeCode(attr->attribute_type,&r);
			}
			break;
		case kSOMIR_ParameterDef:
			{
				RHBparameter *param=element->is_parameter();
				/* param type */
				write_TypeCode(param->parameter_type,&r);
			}
			break;
		case kSOMIR_ExceptionDef:
			{
				/* ex type */

				long i=0;

				/* if it has no elements then write a NULL
					type code */

				RHBelement *e=element->children();

				while (e)
				{
					if (e->is_type())
					{
						RHBtype *te=e->is_type();

						if (te->is_struct_element()) 
						{
							i++;
						}
					}

					e=e->next();
				}

				RHBexception *ex=NULL;

				if (i) ex=element->is_exception();

				write_TypeCode(ex,&r);
			}
			break;
		case kSOMIR_TypeDef:
			{
				RHBtype *typ=element->is_type();

				if (typ)
				{
					RHBtype_def *td=typ->is_typedef();
					/* type */

					if (td)
					{
						write_TypeCode(td->alias_for,&r);
					}
					else
					{
						write_TypeCode(typ,&r);
					}
				}
				else
				{
					SOMIPC_ASSERT(typ);
				}
			}
			break;
		case kSOMIR_ConstantDef:
			/* type */
			/* value */

			{
				RHBconstant *co=element->is_constant();

				write_TypeCode(co->constant_type,&r);

				char buf[256];
				const char *p=buf;

				if (co->is_numeric())
				{
					snprintf(buf,sizeof(buf),"%ld",(long)co->numeric_value());
				}
				else
				{
					p=co->value_string;
					if (!p)
					{
						if (co->const_val)
						{
							p=co->const_val->value_string;
						}
					}
				}
				r.write_string(p);
			}
			break;
		}

		if (r._length)
		{
			SOMIR_record r2;
			long off=24;
			boolean same=0;
			long newOff=0;

			r.update_long(0,r._length);

			r2.load(file,fileOffset);

			SOMIR_Contained cnd(&r2);

			SOMIPC_ASSERT(containedField(off)==cnd.modifiersOffset);

			if (cnd.modifiersOffset)
			{
				r2.load(file,cnd.modifiersOffset);

				r2._length=r2._maximum;

				same|=((r2._length==r._length)
					    && !memcmp(r2._buffer,r._buffer,r._length));

				if (!same)
				{
					/* no longer need this */
					file->free(cnd.modifiersOffset,kSOMIR_Contained);
				}
			}

			if (!same)
			{
				newOff=file->alloc(r._length,__FILE__,__LINE__);

				r.fileOffset=newOff;

				r.save(file);

/*				printf("updating modifiers(%ld,%ld)",newOff,r._length); print_id(); printf("\n");
*/

				containedField(off,newOff);
			}
		}
	}

	SOMIR_element *el=children;

	while (el)
	{
		el->update_modifiers();

		el=el->next;
	}
}

void SOMIR_element::update_elements()
{
	RHBir_file *file=emitter()->irFile;

	if (element && fileOffset)
	{
		switch (_get_SOMIR_type())
		{
		case kSOMIR_OperationDef:
		case kSOMIR_AttributeDef:
		case kSOMIR_ParameterDef:
			{
				SOMIR_record r;
				long off=28;
				long mode=0;
				long value=0;

				r.load(file,fileOffset);

				SOMIR_Contained cnd(&r);

				SOMIPC_ASSERT(cnd.type==_get_SOMIR_type());

				switch (cnd.type)
				{
				case kSOMIR_OperationDef:
					off=36;
					mode=cnd.u.operationDef.mode;
					value=operation_mode();
					break;
				case kSOMIR_AttributeDef:
					mode=cnd.u.attributeDef.mode;
					value=attribute_mode();
					break;
				case kSOMIR_ParameterDef:
					mode=cnd.u.parameterDef.mode;
					value=parameter_mode();
					break;
				}

				SOMIPC_ASSERT(mode==containedField(off));

				if (mode != value)
				{
/*					printf("updating "); print_id(); printf("\n");*/
					containedField(off,value);
				}
			}
			break;
		default:
			break;
		}
	}

	SOMIR_element *el=children;

	while (el)
	{
		el->update_elements();

		el=el->next;
	}
}

void SOMIR_element::write_TypeCode(RHBtype *t,SOMIR_record *r,SOMIR_TypeCode *ctx)
{
#define TYPECODE_HEADER		\
		r->write_long(0);	\
		r->write_long(1);	\
		r->write_long(0);

	if (!t)
	{
		r->write_long(-1);
	}

	while (t)
	{
		while (t->is_typedef())
		{
			RHBtype_def *td=t->is_typedef();

			char buf[256];
			emitter()->get_ir_name(td->alias_for,buf,sizeof(buf));

			if (!strcmp(buf,"::SOMFOREIGN"))
			{
				emitter()->get_ir_name(td,buf,sizeof(buf));

				r->write_long(tk_foreign);

				TYPECODE_HEADER

				r->write_string(td->id);

				const char *impctx=td->get_modifier_string("impctx");

				if (!impctx) impctx="";

				long slen=strlen(impctx);

				if (slen && 
					(impctx[0]=='\"') &&
					(impctx[slen-1]=='\"'))
				{
					slen-=2;
					memcpy(buf,impctx+1,slen);
					buf[slen]=0;
					impctx=buf;
				}

				r->write_string(impctx);

				const char *len=td->get_modifier_string("length");

				if (len)
				{
					r->write_long(atol(len));
				}
				else
				{
					r->write_long(sizeof(void *));
				}

				t=0;
				break;
			}

			t=td->alias_for;
		}

		if (!t) break;


		SOMIR_TypeCode newCtx(ctx,t);

		{
			SOMIR_TypeCode *s=ctx;

			while (s)
			{
				if (s->type==t)
				{
					char *st=t->id;

					r->write_long(tk_self);
					TYPECODE_HEADER
					r->write_string(st);

					break;
				}

				s=s->prev;
			}

			if (s) break;
		}

		char buf[256];
		emitter()->get_ir_name(t,buf,sizeof(buf));

		if (t->is_qualified())
		{
			RHBqualified_type *q=t->is_qualified();
			char buf[32];
			RHBtype *qt=q->base_type;
			while (qt->is_typedef())
			{
				qt=qt->is_typedef()->alias_for;
			}

			snprintf(buf,sizeof(buf),"%s::%s",q->id,qt->id);

			if (!strcmp(buf,"unsigned::long"))
			{
				r->write_long(tk_ulong);
				TYPECODE_HEADER
			}
			else
			{
				if (!strcmp(buf,"unsigned::short"))
				{
					r->write_long(tk_ushort);
					TYPECODE_HEADER
				}
				else
				{
					if (!strcmp(buf,"signed::long"))
					{
						r->write_long(tk_long);
						TYPECODE_HEADER
					}
					else
					{
						if (!strcmp(buf,"signed::short"))
						{
							r->write_long(tk_ulong);
							TYPECODE_HEADER
						}
						else
						{
							SOMIPC_ASSERT(!t);
						}
					}
				}
			}

			break;
		}
		
		if (t->is_interface())
		{
			if (!strcmp(buf,"::Principal"))
			{
				r->write_long(tk_Principal);
				TYPECODE_HEADER

				break;
			}

			r->write_long(tk_objref);
			TYPECODE_HEADER

			SOMIPC_ASSERT(buf[0]);

			r->write_string(buf);

			break;
		}
		
		if (t->is_base_type())
		{
			if (!strcmp(buf,"::long"))
			{
				r->write_long(tk_long);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::octet"))
			{
				r->write_long(tk_octet);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::short"))
			{
				r->write_long(tk_short);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::double"))
			{
				r->write_long(tk_double);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::float"))
			{
				r->write_long(tk_float);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::char"))
			{
				r->write_long(tk_char);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::boolean"))
			{
				r->write_long(tk_boolean);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::any"))
			{
				r->write_long(tk_any);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::void"))
			{
				r->write_long(tk_void);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::null"))
			{
				r->write_long(tk_null);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::TypeCode"))
			{
				r->write_long(tk_TypeCode);
				TYPECODE_HEADER

				break;
			}

			if (!strcmp(buf,"::Principal"))
			{
				r->write_long(tk_Principal);
				TYPECODE_HEADER

				break;
			}

			SOMIPC_ASSERT(!t)
		}

		if (t->is_pointer())
		{
			RHBpointer_type *p=t->is_pointer();

			r->write_long(tk_pointer);
			TYPECODE_HEADER
			
			write_TypeCode(p->pointer_to,r,&newCtx);

			break;
		}

		if (t->is_enum())
		{
			r->write_long(tk_enum);
			TYPECODE_HEADER

			SOMIPC_ASSERT(t->id);

			r->write_string(t->id);

			long i=0;

			RHBelement *e=t->children();

			while (e)
			{
				i++;

				e=e->next();
			}

			r->write_long(i);

			e=t->children();

			while (e)
			{
				r->write_string(e->id);

				e=e->next();
			}

			break;
		}

		if (t->is_struct() || t->is_exception())
		{
			r->write_long(tk_struct);
			TYPECODE_HEADER

			SOMIPC_ASSERT(t->id);

			r->write_string(t->id);

			long i=0;

			RHBelement *e=t->children();

			while (e)
			{
				if (e->is_type())
				{
					RHBtype *te=e->is_type();

					if (te->is_struct_element()) 
					{
						i++;
					}
				}

				e=e->next();
			}

			r->write_long(i);

			e=t->children();

			while (e)
			{
				if (e->is_type())
				{
					RHBtype *te=e->is_type();

					if (te->is_struct_element()) 
					{
						r->write_string(te->id);

						write_TypeCode(te->is_struct_element()->element_type,r,&newCtx);
					}
				}

				e=e->next();
			}
			break;
		}

		if (t->is_string())
		{
			RHBstring_type *ts=t->is_string();

			r->write_long(tk_string);
			TYPECODE_HEADER

			r->write_long(ts->length ? ts->length->numeric_value() : 0);
			
			break;
		}

		if (t->is_sequence())
		{
			RHBsequence_type *ts=t->is_sequence();

			r->write_long(tk_sequence);
			TYPECODE_HEADER

			write_TypeCode(ts->sequence_of,r,&newCtx);

			r->write_long(ts->length ? ts->length->numeric_value() : 0);
			
			break;
		}

		if (t->is_array())
		{
			RHBarray *ta=t->is_array();

			r->write_long(tk_array);
			TYPECODE_HEADER

			write_TypeCode(ta->array_of,r,&newCtx);

			r->write_long(ta->elements->numeric_value());
			
			break;
		}

		if (t->is_union())
		{
			RHBunion *tu=t->is_union();

			r->write_long(tk_union);
			TYPECODE_HEADER

			SOMIPC_ASSERT(tu->id);

			r->write_string(tu->id);

			write_TypeCode(tu->switch_type,r,&newCtx);

			long n=tu->elements.length();

			r->write_long(n);

			long i=0;

			while (i < n)
			{

				RHBunion_element *ue=
					tu->elements.get(i)->is_type()->is_union_element();

				if (i==tu->default_index)
				{
					r->write_long(2);
				}
				else
				{
					r->write_long(1);
					r->write_long(ue->switch_value->numeric_value());
				}

				r->write_string(ue->id);

				write_TypeCode(ue->element_type,r,&newCtx);

				i++;
			}

			break;
		}

		SOMIPC_ASSERT(!t);

		t=0;
	}
}

class enumInterface
{
public:
	virtual void found_interface(RHBir_file *file,long cnd,const char *name)=0;

	virtual ~enumInterface() {}
};

static long lookup_name(RHBir_file *file,long cnr,const char *name)
{
long retVal=0;

	if (file && cnr && name && name[0])
	{
		SOMIR_record r;

		r.load(file,cnr);

		SOMIPC_DEBUG(long len=)r.read_long();
		SOMIPC_DEBUG(long typ=)r.read_long();
		long nItems=r.read_long();

		SOMIPC_ASSERT(typ==kSOMIR_Container);

		long i=0;

		while (i < nItems)
		{
			long off=r.read_long();
			long ityp=r.read_long();
			const char *buf=r.read_string();

			if (((kSOMIR_ModuleDef==ityp)
				||(kSOMIR_InterfaceDef==ityp))
				&&(buf))
			{
				if (!strcmp(buf,name))
				{
					retVal=off;

					break;
				}
			}


			i++;
		}
	}

	return retVal;
}

static long lookup_id(RHBir_file *file,const char *id)
{
long offset=0;
long cnr=file->rootContainer;

/*	printf("lookup_id(%s) .... ",id);*/

	while (*id)
	{
		char buf[256];

		while (*id==':') id++;

		long len=0;

		while (id[len])
		{
			if (id[len]==':') break;
			len++;
		}

		if (!len) break;

		memcpy(buf,id,len);

		buf[len]=0;

		id+=len;

		long child=lookup_name(file,cnr,buf);

		if (!child) 
		{
			break;
		}

		if (!id[0])
		{
			offset=child;
			break;
		}

		SOMIR_record r;

		r.load(file,child);

		SOMIPC_DEBUG(long l2=)r.read_long();
		SOMIPC_DEBUG(long typ=)r.read_long();
		SOMIPC_ASSERT(typ==kSOMIR_ModuleDef)
		SOMIPC_DEBUG(const char *n=)r.read_string();
		SOMIPC_ASSERT(!strcmp(n,buf));
		SOMIPC_DEBUG(long zero=)r.read_long();
		SOMIPC_DEBUG(long defIn=)r.read_long();
		SOMIPC_DEBUG(long desc=)r.read_long();
		SOMIPC_DEBUG(long mod=)r.read_long();

		cnr=r.read_long();

		if (!cnr) break;
	}

/*	printf("%ld\n",offset); */

	SOMIPC_ASSERT(offset);

	return offset;
}

void enum_container(RHBir_file *file,long cnr,enumInterface *iface)
{
	if (file && cnr)
	{
		SOMIR_record r;

		r.load(file,cnr);

		SOMIPC_DEBUG(long len=)r.read_long();
		SOMIPC_DEBUG(long typ=)r.read_long();
		long nItems=r.read_long();

		SOMIPC_ASSERT(typ==kSOMIR_Container);

		long i=0;

		while (i < nItems)
		{
			long off=r.read_long();
			long ityp=r.read_long();
			const char *buf=r.read_string();

			if ( /*(ityp==kSOMIR_InterfaceDef) || */ 
				ityp==kSOMIR_ModuleDef)
			{
				SOMIR_record r2;
				long cnrOffset=0;

				r2.load(file,off);

				SOMIR_Contained cnd(&r2);

				switch (ityp)
				{
				case kSOMIR_InterfaceDef:
					cnrOffset=cnd.u.interfaceDef.contentsOffset;
					break;
				case kSOMIR_ModuleDef:
					cnrOffset=cnd.u.moduleDef.contentsOffset;
					break;
				}

				if (cnrOffset)
				{
					enum_container(file,cnrOffset,iface);
				}
			}

			if (ityp==kSOMIR_InterfaceDef)
			{
				iface->found_interface(file,off,buf);
			}

			i++;
		}
	}
}

class classList
{
public:
	classList *next;
	long offset;
	classList *children;

	classList(long o) : next(NULL),offset(o),children(NULL)
	{
	}

	void append(classList **h)
	{
		if (*h)
		{
			classList *p=*h;

			while (p->next) p=p->next;

			p->next=this;
		}
		else
		{
			*h=this;
		}
	}
};

class update_depends_enum : public enumInterface
{
	classList *classes;
public:
	update_depends_enum() : classes(NULL)
	{

	}

	virtual ~update_depends_enum()
	{
		while (classes)
		{
			classList *p=classes;
			classes=classes->next;
			classList *q=p->children;

			delete p;

			while (q)
			{
				p=q->next;
				delete q;
				q=p;
			}
		}
	}

	void found_interface(RHBir_file *file,long off,const char * /* name */)
	{
		SOMIR_record r2;

		r2.load(file,off);

		SOMIR_Contained cnd(&r2);

		SOMIPC_ASSERT(cnd.type==kSOMIR_InterfaceDef);

		if (cnd.modifiersOffset)
		{
			SOMIR_record r3;

			r3.load(file,cnd.modifiersOffset);

			SOMIPC_DEBUG(long len=)r3.read_long();
			SOMIPC_DEBUG(long typ=)r3.read_long();

			SOMIPC_ASSERT(typ==kSOMIR_Contained);

			SOMIPC_DEBUG(long owner=)r3.read_long();

			SOMIPC_ASSERT(owner==off);

			SOMIPC_DEBUG(long zero=)r3.read_long();

			long nMods=r3.read_long();

			while (nMods--)
			{
				SOMIPC_DEBUG(const char *name=)r3.read_string();
				SOMIPC_DEBUG(const char *val=)r3.read_string();
			}

			long nClasses=r3.read_long();

			while (nClasses--)
			{
				const char *p=r3.read_string();
				if (p)
				{
					long base=lookup_id(file,p);

					if (base)
					{
						classList *p=classes;

						while (p)
						{
							if (p->offset==base) break;

							p=p->next;
						}

						if (!p)
						{
							p=new classList(base);

							p->append(&classes);
						}

						classList *q=classes->children;

						while (q)
						{
							if (q->offset==off) break;

							q=q->next;
						}

						if (!q)
						{
							q=new classList(off);

							q->append(&p->children);
						}
					}
				}
			}
		}
	}

	void update(RHBir_file *file)
	{
		classList *parent=classes;

		while (parent)
		{
			SOMIR_record r;

			r.load(file,parent->offset);

			SOMIR_Contained cnd(&r);

/*			fprintf(stderr,"updating classes for %s, %ld\n",
				cnd.name,cnd.descendantsOffset);
*/

			classList *dep=parent->children;

			unsigned long len=12,num=0;

			while (dep)
			{
				len+=4;
				num++;

				dep=dep->next;
			}

			SOMIR_record original_descendants;

			long original_descendents_offset=cnd.descendantsOffset;

			if (cnd.descendantsOffset)
			{
				original_descendants.load(file,cnd.descendantsOffset);

				original_descendants._length=original_descendants._maximum;

/*				file->free(cnd.descendantsOffset,kSOMIR_Descendant);*/

				cnd.descendantsOffset=0;
			}

			if (num)
			{
				SOMIR_record r2;
				boolean doSave=1;

				r2._length=0;
				r2._maximum=len;
				r2.set_alloc(len);

				r2.write_long(len);
				r2.write_long(kSOMIR_Descendant);
				r2.write_long(num);

				dep=parent->children;

				while (dep)
				{
					r2.write_long(dep->offset);

					dep=dep->next;
				}

				if (r2._length==original_descendants._length)
				{
					cnd.descendantsOffset=original_descendants.fileOffset;
					original_descendants.fileOffset=0;
				}
				else
				{
					cnd.descendantsOffset=file->alloc(r2._length,__FILE__,__LINE__);
				}

				r2.fileOffset=cnd.descendantsOffset;

				if (original_descendants._length==r2._length)
				{
					if (!memcmp(original_descendants._buffer,
								r2._buffer,
								r2._length))
					{
						doSave=0;
					}
				}

				if (doSave)
				{
					r2.save(file);
				}
			}

			if (original_descendants.fileOffset)
			{
				file->free(original_descendants.fileOffset,kSOMIR_Descendant);

				original_descendants.fileOffset=0;
			}

			if (original_descendents_offset != cnd.descendantsOffset)
			{
				long delta=20+1+strlen(cnd.name);

				r.empty();

				r.write_long(cnd.descendantsOffset);

				file->seek(delta+parent->offset);
				file->write(r._buffer,r._length);
			}

#ifdef _DEBUG
			{
				r.load(file,parent->offset);

				SOMIR_Contained cnd2(&r);

				SOMIPC_ASSERT(cnd.descendantsOffset == cnd2.descendantsOffset);
			}
#endif
			parent=parent->next;
		}
	}
};

void SOMIR_Repository::update_depends()
{
	RHBir_file *file=emitter()->irFile;
	long offset=containerOffset();

	if (offset)
	{
		update_depends_enum ctx;

		enum_container(file,offset,&ctx);

		ctx.update(file);
	}
}

