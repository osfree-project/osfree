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

#ifdef HAVE_SIGNAL_H
#	include <signal.h>
#endif

#include <rhbsc.h>

static boolean default_emitflag;

class modifier_arg
{
public:
	char *name;
	char *value;
	modifier_arg *next;
	modifier_arg(const char *p) : name(NULL),value(NULL),next(NULL)
	{
		if (p)
		{
			size_t n=1+strlen(p);

			name=new char[n];
			memcpy(name,p,n);

			n=0;

			while (name[n])
			{
				if (name[n]=='=')
				{
					name[n]=0;
					value=name+n;
					break;
				}

				n++;
			}
		}
	}

	~modifier_arg()
	{
		if (name) delete []name;
	}
};

static int do_emit(
	modifier_arg * /* mod_list */ ,
	const char *emitter_name,
	const char *output_filename,
	int argc,
	char **argv)
{
	int rc=1;
	static const char *valid_emitter[]={
		"xh","xih","h","ih","api","kih","ir","tc"
	};
	int i=sizeof(valid_emitter)/sizeof(valid_emitter[0]);

	while (i--)
	{
		if (!strcmp(emitter_name,valid_emitter[i]))
		{
			break;
		}
	}

	if (i < 0)
	{
		fprintf(stderr,"Unknown emitter %s\n",emitter_name);
		return 1;
	}

	RHBPreprocessor preprocessor;
	const char *root_idl=NULL;
	FILE *fp=stdin;

	if (argc) fp=NULL;

	RHBFile f(argc,argv,fp);

	if (f.failed())
	{
		return 1;
	}

	preprocessor.push_stream(&f);

	{
		RHBPreprocessor *p=&preprocessor;
		RHBrepository r(p,"");

		r.build(p);

		if (p->Errors())
		{
			fprintf(stderr,"Errors were detected\n");

			return 1;
		}

		if (p->includes)
		{
			RHBelement *e=p->includes->children();

			while (e)
			{
				RHBinclude_file *f=e->is_include_file();

				if (f && f->id)
				{
					const char *p=f->id;
					int k=strlen(p);
					while (k--)
					{
						if (p[k]=='.')
						{
							if (!strcmp(p+k,".idl"))
							{
								root_idl=p;
							}

							break;
						}
					}

					if (root_idl) 
					{
						break;
					}
				}

				e=e->next();
			}
		}

		if (!root_idl)
		{
			fprintf(stderr,"unable to determine root IDL filename\n");
		}
		else
		{
			rc=0;

			if (!strcmp(emitter_name,"ir"))
			{
				boolean old_emit=default_emitflag;
				default_emitflag=1;
				RHBir_file out(output_filename);
		/*		printf("This should be the IR generator\n",buf);*/
				RHBir_emitter emitter(&r,&out);
				emitter.generate(&out,root_idl);
#ifdef _PLATFORM_MACINTOSH_
				fsetfileinfo(buf,SOMIR_CREATOR,SOMIR_TYPE);
#endif
				default_emitflag=old_emit;
			}
			else
			{
				RHBtextfile out(output_filename);

				if (!strcmp(emitter_name,"tc"))
				{
					RHBtc_emitter emitter(&r);

					emitter.generate(&out,root_idl);
				}
				else
				{
					if (!strcmp(emitter_name,"kih"))
					{
						RHBkernel_emitter emitter(&r);

						emitter.generate(&out,root_idl);
					}
					else
					{
						if (!strcmp(emitter_name,"api"))
						{
							RHBapi_emitter emitter(&r);

							emitter.generate(&out,root_idl);
						}
						else
						{
							RHBheader_emitter emitter(&r);

							emitter.cplusplus=0;
							emitter.internal=0;
							emitter.testonly=0;

							if (0==strcmp(emitter_name,"ih"))
							{
								emitter.internal=1;
							}
				
							if (0==strcmp(emitter_name,"xih"))
							{
								emitter.internal=1;
								emitter.cplusplus=1;
							}

							if (0==strcmp(emitter_name,"xh"))
							{
								emitter.cplusplus=1;
							}

							emitter.generate(&out,root_idl);
						}
					}
				}
			}
		}

		r.destroy();
		r.delete_all(&r);
	}

	return rc;
}

void bomb(const char *p)
{
	if (!p) p="unknown error";
#ifdef _WIN32
	printf("SC: fatal: %s\n",p);
	fflush(stdout);
	fflush(stderr);
#	ifdef _DEBUG
#		ifdef _M_IX86
			__asm int 3;
#		else
			((int *)0)[0]=0;
#		endif
#	else
		exit(1);
#	endif
#else
#ifdef _PLATFORM_MACINTOSH_xxx
	if (p)
	{
		unsigned char buf[256];
		buf[0]=strlen(p);
		if (*p) memcpy(&buf[1],p,buf[0]);
		DebugStr(buf);
	}
	else
	{
		DebugStr("\p");
	}
#else
	printf("SC: fatal: %s\n",p);
	fflush(stdout);
	fflush(stderr);
	exit(1);
#endif
#endif
}

boolean RHBshould_emit_on(RHBelement *)
{
	return 1;
}

boolean RHBshould_emit_off(RHBelement *)
{
	return 0;
}

boolean RHBshould_emit_default(RHBelement * /*t */)
{
	return default_emitflag;
}

#ifndef HAVE_VSNPRINTF
int vsnprintf(char *tgt,size_t /*tgtlen*/,const char *fmt,va_list ap)
{
	/* SunOS returns a char *, ANSI returns an int */

	vsprintf(tgt,fmt,ap);

	return strlen(tgt);
}
#endif

#ifndef HAVE_SNPRINTF
int snprintf(char *tgt,size_t len,const char *fmt,...)
{
	va_list ap;
	int rc=-1;

	va_start(ap,fmt);

	rc=vsnprintf(tgt,len,fmt,ap);

	va_end(ap);

	return rc;
}
#endif

class args
{
public:
	modifier_arg *mods;
	char **argv;
	args(int argc,char **d) : argv(new char *[argc+1])
	{
		mods=NULL;
		argv[argc]=NULL;
		while (argc--)
		{
			argv[argc]=d[argc];
		}
	}
	~args()
	{
		delete [] argv;

		while (mods)
		{
			modifier_arg *p=mods;
			mods=mods->next;
			delete p;
		}
	}

	void add_modifier(const char *m)
	{
		modifier_arg *p=new modifier_arg(m);

		if (mods)
		{
			modifier_arg *q=mods;
			while (q->next) q=q->next;
			q->next=p;
		}
		else
		{
			mods=p;
		}
	}
};

int main(int argc,char **argv)
{
	const char *appname=argv[0];
	int i=0;
	const char *emitter_name=NULL;
	const char *output_filename=NULL;
#if defined(HAVE_SIGNAL_H) && defined(SIG_BLOCK)
	sigset_t sigs;
	sigfillset(&sigs);
	sigprocmask(SIG_BLOCK,&sigs,NULL);
#endif

	if (!argc) 
	{
		fprintf(stderr,"no argument list\n");

		return 1;
	}

	argc--;
	argv++;

	{
		args a(argc,argv);

		argv=a.argv;

		while (i < argc)
		{
			int drop=0;
			const char *p=argv[i];

			if (
#ifdef _WIN32
				(*p=='/')||
#endif
				(*p=='-'))
			{
				if ((p[1]=='o')
	#ifdef _WIN32
					||(p[1]=='O')
	#endif
					)
				{
					if (output_filename)
					{
						fprintf(stderr,"%s: output file already specified\n",appname);
						return 1;
					}

					output_filename=argv[i+1];
					drop=2;
				}
				else
				{
					if ((p[1]=='e')
	#ifdef _WIN32
						||(p[1]=='E')
						||(p[1]=='S')
	#endif
						||(p[1]=='s')
						)
					{
						if (emitter_name)
						{
							fprintf(stderr,"%s: emitter already specified\n",appname);
							return 1;
						}

						if (p[2])
						{
							emitter_name=p+2;
							drop=1;
						}
						else
						{
							emitter_name=argv[i+1];
							drop=2;
						}
					}
					else
					{
						if ((p[1]=='m')
	#ifdef _WIN32
							||(p[1]=='M')
	#endif

							)
						{
							if (p[2])
							{
								a.add_modifier(p);
								drop=1;
							}
							else
							{
								a.add_modifier(argv[i+1]);
								drop=2;
							}
						}
						else
						{
							fprintf(stderr,"%s: Unknown switch \'%s\'\n",appname,p);
							return 1;
						}
					}
				}
			}

			if (drop)
			{
				int k=i;

				while ((k+drop) < argc)
				{
					argv[k]=argv[k+drop];
					k++;
				}

				argc-=drop;

				argv[argc]=NULL;
			}
			else
			{
				i++;
			}
		}

		if (!emitter_name)
		{
			fprintf(stderr,"%s: no emitter specified\n",appname);

			return 1;
		}

		i=do_emit(a.mods,emitter_name,output_filename,argc,argv);
	}

#if defined(_WIN32) && defined(_DEBUG) && (_MSC_VER >= 1200) && !defined(_WIN32_WCE) && !defined(_WIN64)
	if (_CrtDumpMemoryLeaks())
	{
		__asm int 3;

		_CrtDumpMemoryLeaks();

		__asm int 3;
	}
#endif

	return i;
}
