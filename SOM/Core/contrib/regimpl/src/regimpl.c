#include <rhbopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <rhbsomex.h>
#include <somir.h>
#include <somd.h>
#include <implrep.h>
#include <somdom.h>
#include <somdserv.h>
#include <somtc.h>
#include <somtcnst.h>
#include <somuutil.h>
#include <somestio.h>
#include <regior.h>

#ifdef _PLATFORM_MACINTOSH_
	#ifdef USE_THREADS
		#include <rhbpkern.h>
	#else
		#include <LibraryManagerUtilities.h>
		#include <somexp.h>
	#endif
	#include <QuickDraw.h>
	#include <MacWindows.h>
	#include <Processes.h>
	#include <SegLoad.h>
	#include <AppleEvents.h>
	#include <somterm.h>
#else
	#include <signal.h>
	#include <fcntl.h>
	#include <errno.h>
#endif

#if defined(_WIN32) && ( (!defined(_CONSOLE)) || defined(USE_THREADS) )
	#ifndef USE_PTHREADS
		#include <windows.h>
	#endif
#endif 

/*

  To enter interactive mode:
        regimpl
  Add implementation:
        regimpl -A -i <str> [-p <str>] [-v <str>]
                [-m {on|off}] [-z <str>]
                [-e <str> {<param> ...}] [-s {on|off}] [-k {on|off}]
                [-t <str> {<param> ...} [-t ...]] [-g <str>]
  Update implementation:
        regimpl -U -i <str> [-p <str>] [-v <str>]
                [-m {on|off}]
                [-e <str> {<param> ...}] [-s {on|off}] [-k {on|off}]
                [-t <str> {<param> ...} [-t ...]] [-g <str>]
  Delete implementation:
        regimpl -D -i <str> [-i ...]
  List implementation(s):
        regimpl -L [-i <str> [-i ...]]
  List aliases:
        regimpl -S
  Add class(es):
        regimpl -a -c <str> [-c ...] -i <str> [-i ...]
  Delete class(es):
        regimpl -d -c <str> [-c ...] [-i <str> [-i ...]]
  List classes associated with implementation(s):
        regimpl -l [-i <str> [-i ...]]
  where:
    -i <str>    = Implementation alias name
    -p <str>    = Server program name  (default: somdsvr.exe)
    -v <str>    = Server-class name  (default: SOMDServer)
    -m {on|off} = Enable multi-threaded server  (optional)
    -z <str>    = Implementation ID
    -c <str>    = Class name
    -e <str>    = ImplDef Class name (default: ImplementationDef)
       <param>  = Values for additional attributes needed by subclass of ImplementationDef
    -s {on|off} = Enable secure server (optional)
    -t <str>    = Transport protocol
       <param>  = Option can be zero or more strings, delimited by spaces.
    -g <str>    = Configuration File name (optional)

  */

static int regimpl_running=1;

#define scanf dont_use

static char *dupl_string(char *a)
{
	if (a)
	{
		int i=(int)strlen(a)+1;
		char *p=SOMMalloc(i);
		memcpy(p,a,i);
		return p;
	}
	return 0;
}

static boolean regimpl_get_line(char *p,int len)
{
	if (regimpl_running)
	{
#ifdef _PLATFORM_MACINTOSH_
		len=SOMTerminal_Read(p,len);
		p[len]=0;
		return 1;
#else
		*p=0;

		errno=0;

#if defined(F_GETFL) && defined(F_SETFL) && defined(O_NONBLOCK)
		/* strangely, NetBSD 1.6.1 with pth manages to flip
			the O_NONBLOCK bit */

		while (1)
		{
			int i=fcntl(0,F_GETFL,0);
/*			printf("fcntl(0,F_GETFL)=%lx\n",(long)i);*/
			if (i == -1) break;
			if (!(i & O_NONBLOCK)) break;
			i&=~O_NONBLOCK;
			i=fcntl(0,F_SETFL,i);
/*			printf("fcntl(0,F_SETFL)=%lx\n",(long)i);*/
			if (i == -1) break;			
		}
#endif

		if (fgets(p,len,stdin))
		{
			while (*p)
			{
				if (*p < ' ') 
				{
					*p=0;
				}
				else
				{
					p++;
				}
			}		

			return 1;
		}
		else
		{
			int e=errno;
			perror("fgets");
			printf("errno=%d\n",e);
		}
#endif
	}

	return 0;
}

static char *regimpl_id_from_alias(Environment *ev,char *alias)
{
	if (ev->_major) return 0;

	if (alias)
	{
		if (*alias)
		{
			ImplementationDef SOMSTAR impl=ImplRepository_find_impldef_by_alias(SOMD_ImplRepObject,ev,alias);

			if (!ev->_major)
			{
				if (impl)
				{
					char *p;

					p=ImplementationDef__get_impl_id(impl,ev);

					somReleaseObjectReference(impl);

					if (p) return p;
				}
			}
		}
	}

	RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,NO);

	return 0;
}

static void regimpl_command_add_class(Environment *ev,char *impl_id,char *classname)
{
	if (impl_id)
	{
		impl_id=regimpl_id_from_alias(ev,impl_id);

		if (ev->_major) return;

		ImplRepository_add_class_to_impldef(SOMD_ImplRepObject,ev,impl_id,classname);

		SOMFree(impl_id);
	}
	else
	{
		ImplRepository_add_class_to_all(SOMD_ImplRepObject,ev,classname);
	}
}

static void class_add_all(Environment *ev)
{
	char cls[256];

	somPrintf("Enter class : ");

	if (regimpl_get_line(cls,sizeof(cls)))
	{
		regimpl_command_add_class(ev,NULL,cls);
	}
}

static void regimpl_command_remove_class(Environment *ev,char *impl_id,char *classname)
{
	if (impl_id)
	{
		impl_id=regimpl_id_from_alias(ev,impl_id);

		if (ev->_major) return;

		ImplRepository_remove_class_from_impldef(SOMD_ImplRepObject,ev,impl_id,classname);

		SOMFree(impl_id);
	}
	else
	{
		ImplRepository_remove_class_from_all(SOMD_ImplRepObject,ev,classname);
	}
}

static void class_delete_all(Environment *ev)
{
	char cls[256];

	somPrintf("Enter class : ");

	if (regimpl_get_line(cls,sizeof(cls)))
	{
		regimpl_command_remove_class(ev,NULL,cls);
	}
}

static void print_copyright(void)
{
static int copydone;

	if (!copydone)
	{
		copydone=1;
		somPrintf("Implementation Registration Utility\n");
		somPrintf("(C) Copyright Roger Brown 1998\n");
	}
}

static void print_main_menu(void)
{
	print_copyright();

	somPrintf("\n");
	somPrintf("Server Operations:\n");
	somPrintf(" 1.  Add          2.  Delete       3.  Change\n");
	somPrintf(" 4.  Show one     5.  Show all     6.  List aliases\n");
	somPrintf("\n");
	somPrintf("Class operations:\n");
	somPrintf(" 7.  Add          8.  Delete       9.  Delete from all\n");
	somPrintf(" 10. List classes 11. Add to all\n");
	somPrintf("\n");
	somPrintf(" 12. Exit\n");
}

static void show_impl(ImplementationDef SOMSTAR impl,Environment *ev)
{
	char *p;
	if (ev->_major) return;
	p=ImplementationDef__get_impl_alias(impl,ev);
	if (ev->_major) return;
	if (p)
	{
		somPrintf("impl_alias         : %s\n",p); 
		SOMFree(p);
	}
	p=ImplementationDef__get_impl_id(impl,ev);
	if (ev->_major) return;
	if (p)
	{
		somPrintf("impl_id            : %s\n",p); 
		SOMFree(p);
	}
	p=ImplementationDef__get_impl_hostname(impl,ev);
	if (ev->_major) return;
	if (p)
	{
		somPrintf("impl_hostname      : %s\n",p); 
		SOMFree(p);
	}
	p=ImplementationDef__get_impl_program(impl,ev);
	if (ev->_major) return;
	if (p)
	{
		somPrintf("impl_program       : %s\n",p); 
		SOMFree(p);
	}
	p=ImplementationDef__get_impl_server_class(impl,ev);
	if (ev->_major) return;
	if (p)
	{
		somPrintf("impl_server_class  : %s\n",p); 
		SOMFree(p);
	}
	if (ev->_major) return;
	{
		_IDL_SEQUENCE_octet *seqp;

		seqp=ImplementationDef__get_svr_objref(impl,ev);

		if (ev->_major) return;

		if (seqp)
		{
		somPrintf("svr_objref         : ");
			RHB_print_ior(seqp,ev,1);
		}
	}
}

static void regimpl_command_list_all_servers(Environment *ev)
{
_IDL_SEQUENCE_ImplementationDef impls={0,0,NULL};

	ImplRepository_find_all_impldefs(SOMD_ImplRepObject,ev,&impls);

	if (impls._buffer && !ev->_major)
	{
		unsigned int i=0;
		int any_shown=0;

		while (i < impls._length)
		{
			if (impls._buffer[i])
			{
				if (!ev->_major)
				{
					if (any_shown) somPrintf("\n");
					any_shown=1;
					show_impl(impls._buffer[i],ev);
				}
				somReleaseObjectReference(impls._buffer[i]);
				impls._buffer[i]=0;
			}

			i++;
		}

		SOMFree(impls._buffer);
	}
}

static void regimpl_command_list_server(Environment *ev,char *p)
{
	ImplementationDef SOMSTAR impl=ImplRepository_find_impldef_by_alias(SOMD_ImplRepObject,ev,p);

	if (impl && !ev->_major)
	{
		show_impl(impl,ev);

		somReleaseObjectReference(impl);
	}
}

static void regimpl_command_list_all_aliases(Environment *ev)
{
	_IDL_SEQUENCE_string aliases={0,0,NULL};

	ImplRepository_find_all_aliases(
			SOMD_ImplRepObject,ev,&aliases);

	if (aliases._length)
	{
		unsigned int i=0;

		while (i < aliases._length)
		{
			char *p=aliases._buffer[i];
			somPrintf("%s\n",p);
			SOMFree(p);
			i++;
		}

		SOMFree(aliases._buffer);
	}
}

static void regimpl_command_delete_server(Environment *ev,char *p)
{
	ImplementationDef SOMSTAR impl=ImplRepository_find_impldef_by_alias(SOMD_ImplRepObject,ev,p);

	if (impl && !ev->_major)
	{
		p=ImplementationDef__get_impl_id(impl,ev);

		somReleaseObjectReference(impl);

		if (p)
		{
			ImplRepository_delete_impldef(SOMD_ImplRepObject,ev,p);
			SOMFree(p);
		}
	}
}

static void impldef_copy_from(
		ImplementationDef SOMSTAR where,
		Environment *ev,
		ImplementationDef SOMSTAR def,
		SOMDServer SOMSTAR impl_server)
{
	SOMObject SOMSTAR stream=NULL;
	SOMObject SOMSTAR local_stream=
#ifdef somNewObject
		somNewObject(somStream_MemoryStreamIO);
#else
		somStream_MemoryStreamIONew();
#endif
	char *stream_class=SOMObject_somGetClassName(local_stream);

	somId id=somIdFromString("somdCreateObj");
	boolean b=somva_SOMObject_somDispatch(
			impl_server,
			(somToken *)(void *)&stream,
			id,
			impl_server,ev,stream_class,NULL);
	SOMFree(id);

	if ((!b) || (ev->_major) || (!stream))
	{
		if (local_stream) somReleaseObjectReference(local_stream);

		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,NO);
		return;
	}

/*	SOMObject_somPrintSelf(stream);*/

	if (local_stream)
	{
/*		SOMObject_somPrintSelf(local_stream);*/

		ImplementationDef_externalize_to_stream(def,ev,local_stream);

		if (!ev->_major)
		{
			_IDL_SEQUENCE_octet buf={0,0,0};

			buf=somStream_MemoryStreamIO_get_buffer(local_stream,ev);

			if (!ev->_major)
			{
				somId id=somIdFromString("set_buffer");
				somToken ret=0;

				somva_SOMObject_somDispatch(stream,&ret,id,
						stream,ev,&buf);

				SOMFree(id);
			}

			if (buf._buffer) SOMFree(buf._buffer);
		}
	}

	if (!ev->_major)
	{
		ImplementationDef_internalize_from_stream(where,ev,stream,0);
	}

	if (stream) somReleaseObjectReference(stream);
	if (local_stream) somReleaseObjectReference(local_stream);
}

static ImplementationDef SOMSTAR make_remote_def(
		ImplRepository SOMSTAR rep,
		Environment *ev,
		ImplementationDef SOMSTAR def)
{
	ImplementationDef SOMSTAR remote=NULL;
	char *p;

	/* need to make remote def on same machine as other */
	/* solution is to try and get a SOMDServer for the remote machine */

	SOMDServer SOMSTAR server=SOMDObjectMgr_somdFindAnyServerByClass(SOMD_ObjectMgr,ev,
		"ImplementationDef");

	if (!server)
	{
		return 0;
	}

	if (ev->_major)
	{
		somReleaseObjectReference(server);

		return 0;
	}

	p=SOMObject_somGetClassName(def);

	if (p)
	{
		somId somdCreateObj=somIdFromString("somdCreateObj");
		somva_SOMObject_somDispatch(server,(somToken *)(void *)&remote,somdCreateObj,server,ev,p,0);
		SOMFree(somdCreateObj);
	}

	if (!ev->_major)
	{
		impldef_copy_from(remote,ev,def,server);

		somReleaseObjectReference(server);

		return remote;
	}

	somReleaseObjectReference(server);

	return NULL;
}

static void regimpl_command_add_server(Environment *ev,
				char *impl_alias,
				char *program_name,
				char *server_class,
				char *impl_id,
				_IDL_SEQUENCE_octet **iorp)
{
	ImplementationDef SOMSTAR impl;

#ifdef somNewObject
	impl=somNewObject(ImplementationDef);
#else
	impl=ImplementationDefNew();
#endif

	if (impl_id)
	{
		ImplementationDef__set_impl_id(impl,ev,impl_id);
	}

	if (impl_alias)
	{
		ImplementationDef__set_impl_alias(impl,ev,impl_alias);
	}

	if (program_name)
	{
		ImplementationDef__set_impl_program(impl,ev,program_name);
	}

	if (server_class)
	{
		ImplementationDef__set_impl_server_class(impl,ev,server_class);
	}

	if (iorp && !ev->_major)
	{
		if (*iorp)
		{
			ImplementationDef__set_svr_objref(impl,ev,*iorp);
			*iorp=NULL;
		}
	}

	if (!ev->_major)
	{
		ImplementationDef SOMSTAR remote=make_remote_def(SOMD_ImplRepObject,ev,impl);

		if (remote && !ev->_major)
		{
			ImplRepository_add_impldef(SOMD_ImplRepObject,ev,remote);

			/* remote is invalid if exception occurred */
			if (remote) somReleaseObjectReference(remote);
		}
	}

	somReleaseObjectReference(impl);
}

static char *regimpl_find_arg(int argc,char **argv,char *cmd)
{
	while (argc--)
	{
		char *p=*argv++;
		if (!strcmp(cmd,p))
		{
			return *argv;
		}
	}

	return NULL;
}

static void regimpl_command_list_classes(Environment *ev,char *impl_id)
{
	_IDL_SEQUENCE_string seq={0,0,NULL};

	impl_id=regimpl_id_from_alias(ev,impl_id);

	if (ev->_major) return;

	seq=ImplRepository_find_classes_by_impldef(SOMD_ImplRepObject,ev,impl_id);

	if (!ev->_major)
	{
		if (seq._length)
		{
			unsigned int i;

			i=0;

			while (i < seq._length)
			{
				char *p;
				p=seq._buffer[i];

				if (p)
				{
					somPrintf("%s\n",p);
					SOMFree(p);
				}
				i++;
			}
		}

		if (seq._maximum)
		{
			SOMFree(seq._buffer);
		}
	}

	SOMFree(impl_id);
}

static SOMDServer SOMSTAR get_remote_somdd(Environment *ev,char *hostname)
{
	ReferenceData object_key={0,0,NULL};
	Repository SOMSTAR rep=NULL;
	char *somdserver_type_id=NULL;
	_IDL_SEQUENCE_octet seq={0,0,NULL};
	char *ior=NULL;
	SOMDServer SOMSTAR server=NULL;

	rep=ORB_resolve_initial_references(SOMD_ORBObject,ev,"InterfaceRepository");

	if (ev->_major) return NULL;

	if (!ev->_major && rep)
	{
		somId id=NULL;
		InterfaceDef SOMSTAR iface=NULL;

		id=somIdFromString("lookup_id");

		somva_SOMObject_somDispatch(rep,(somToken *)(void *)&iface,id,
				rep,ev,"::SOMDServer");

		SOMFree(id);

		id=NULL;

		if (!ev->_major && iface)
		{
			id=somIdFromString("_get_id");

			somva_SOMObject_somDispatch(iface,(somToken *)(void *)&somdserver_type_id,id,iface,ev);

			SOMFree(id);

			somReleaseObjectReference(iface);
		}

		somReleaseObjectReference(rep);
	}

	if (!somdserver_type_id) 
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,NO);

		return NULL;
	}

	object_key=somdGetDefaultObjectKey(ev,NULL);

	RHB_create_ior(&seq,ev,somdserver_type_id,hostname,
			RHBOPT_SOMDPORT,&object_key);

	SOMFree(object_key._buffer);
	SOMFree(somdserver_type_id);

	if (ev->_major) return NULL;

	ior=RHB_ior_string_from_octets(&seq);

	SOMFree(seq._buffer);

	server=ORB_string_to_object(SOMD_ORBObject,ev,ior);

	SOMFree(ior);

	return server;
}

static char *regimpl_get_remote_impl_id(Environment *ev,char *hostname,char *alias,ImplementationDef_octet_seqP *iorp)
{
	char *impl_id=NULL;
	SOMDServer SOMSTAR somdd=NULL;
	ImplRepository SOMSTAR irep=NULL;

	if (ev->_major) return NULL;

	/* dont try this at home kids, only works with remote server with
		this implementation of 'somdd' */

	somdd=get_remote_somdd(ev,hostname);

	if (ev->_major) return NULL;

	irep=SOMDServer_somdCreateObj(somdd,ev,"ImplRepository",0);

	if (!ev->_major && irep)
	{
		ImplementationDef SOMSTAR impl=NULL;

		impl=ImplRepository_find_impldef_by_alias(irep,ev,alias);

		somReleaseObjectReference(irep);

		if (impl && !ev->_major)
		{
			impl_id=ImplementationDef__get_impl_id(impl,ev);
	
			somReleaseObjectReference(impl);
		}
		else
		{
			somPrintf("Error, failed to find implementation '%s' from '%s'\n",alias,hostname);
		}
	}
	else
	{
		somPrintf("Error, failed to acquire remote Implementation Repository from '%s'\n",hostname);
	}

	/* should also get a reference to the server from
		the 'SOMDObjectMgr' object on the remote server,
		from this we can decode the svr_objref */

	if (iorp && !ev->_major)
	{
		SOMDObjectMgr SOMSTAR mgr=NULL;

		mgr=SOMDServer_somdCreateObj(somdd,ev,"SOMDObjectMgr",0);

		if (!ev->_major && mgr)
		{
			SOMObject SOMSTAR ref=NULL;

			somId id=somIdFromString("somdFindServer");

/*			SOMObject_somDumpSelf(mgr,0);*/

			somva_SOMObject_somDispatch(mgr,
					(somToken *)(void *)&ref,id,
					mgr,ev,impl_id);

			SOMFree(id);

			somReleaseObjectReference(mgr);

			if (!ev->_major && ref)
			{
				char *ior=ORB_object_to_string(SOMD_ORBObject,ev,ref);

/*				SOMObject_somDumpSelf(ref,0);*/
				somReleaseObjectReference(ref);

				if (ior)
				{
					_IDL_SEQUENCE_octet *iop;

					iop=SOMMalloc(sizeof(*iop));

					*iop=RHB_octets_from_ior_string(ior);

					*iorp=iop;

					SOMFree(ior);
				}
			}
		}

		if (!*iorp)
		{
			RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,NO);
		}
	}

	somReleaseObjectReference(somdd);

	return impl_id;
}

static void regimpl_command(Environment *ev,int argc,char **argv)
{
	while (argc)
	{
		char *p;

		p=*argv;
		argc--;
		argv++;

		if (*p != '-')
		{
			print_copyright();
			somPrintf("%s does not lead with a '-'\n",p);
			return;
		}

		switch (p[1])
		{
		case 'L':
			if (!argc)
			{
				regimpl_command_list_all_servers(ev);
			}

			while (argc)
			{
				p=*argv;
				argc--;
				argv++;

				if (strcmp(p,"-i"))
				{
					print_copyright();
					somPrintf("%s is not '-i'\n",p);
					return;
				}

				if (!argc)
				{
					print_copyright();
					somPrintf("run out of parameters\n");
					return;
				}

				p=*argv;
				argc--;
				argv++;

				regimpl_command_list_server(ev,p);
			}

			return;

		case 'S':
			regimpl_command_list_all_aliases(ev);
			return;

		case 'D':
			while (argc)
			{
				char *p;
				argc--;
				p=*argv++;
				if (strcmp(p,"-i"))
				{
					print_copyright();
					somPrintf("%s was supposed to be '-i'\n");
					return;
				}
				if (!argc)
				{
					somPrintf("run out of arguments\n");
					return;
				}
				argc--;
				p=*argv++;
				regimpl_command_delete_server(ev,p);
			}

			return;

		case 'A':
			if (argc)
			{
				char *alias;
				char *program;
				char *server;
				char *impl;

				alias=0;
				program=0;
				server=0;
				impl=0;

				while (argc--)
				{
					char *p;
					p=*argv++;
					if (p[0]!='-')
					{
						print_copyright();
						somPrintf("%s is not an option\n",p);
						return;
					}

					if (!argc)
					{
						print_copyright();
						somPrintf("run out of arguments after %s\n",p);
						return;
					}

					argc--;
					switch (p[1])
					{
						case 'i': alias=*argv++; break;
						case 'p': program=*argv++; break;
						case 'v': server=*argv++; break;
						case 'z': impl=*argv++; break;
						default:
							print_copyright();
							somPrintf("%s is not an recognized option\n",p);
							return;
					}
				}

				regimpl_command_add_server(ev,
						alias,
						program,
						server,
						impl,
						0);
			}
			return;

		case 'a': /* add classes to impl */
			{
				char *cls=regimpl_find_arg(argc,argv,"-c");
				char *impl=regimpl_find_arg(argc,argv,"-i");

				regimpl_command_add_class(ev,impl,cls);
			}

			return;
		case 'd': /* delete classes from impl */
			{
				char *cls=regimpl_find_arg(argc,argv,"-c");
				char *impl=regimpl_find_arg(argc,argv,"-i");

				regimpl_command_remove_class(ev,impl,cls);
			}
			return;
		case 'l': /* list classes with impl */
			{
				char *impl=regimpl_find_arg(argc,argv,"-i");

				regimpl_command_list_classes(ev,impl);
			}
			return;

		default:
			print_copyright();
			somPrintf("unknown command %s\n",p);
			return;
		}
	}
}

static void server_add(Environment *ev)
{
	char buf[256];
	char *alias_name=NULL;
	char *program_name=NULL;
	char *server_class=NULL;
	char *host_name=NULL;
	char *impl_id=NULL;
	_IDL_SEQUENCE_octet *ior=NULL;

	somPrintf("Enter alias : ");

	if (!regimpl_get_line(buf,sizeof(buf))) return;

	if (!buf[0]) return;

	alias_name=dupl_string(buf);

	somPrintf("Enter hostname : ");

	if (regimpl_get_line(buf,sizeof(buf)))
	{
		if (buf[0])
		{
			host_name=dupl_string(buf);
		}
	}

	if (host_name)
	{
		/* this means add a remote implementation to this pool */

		impl_id=regimpl_get_remote_impl_id(ev,host_name,alias_name,&ior);

		if (!impl_id)
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,NO);
		}
	}
	else
	{
		somPrintf("Enter server program : ");

		if (regimpl_get_line(buf,sizeof(buf)))
		{
			if (buf[0])
			{
				program_name=dupl_string(buf);
			}
		}

		somPrintf("Enter server class : ");

		if (regimpl_get_line(buf,sizeof(buf)))
		{
			if (buf[0])
			{
				server_class=dupl_string(buf);
			}
		}
	}

	if (!regimpl_running)
	{
		RHBOPT_throw_StExcep(ev,UNKNOWN,UnknownError,MAYBE);
	}

	if (ev->_major==NO_EXCEPTION)
	{
		regimpl_command_add_server(ev,alias_name,program_name,server_class,impl_id,&ior);
	}

	if (alias_name) SOMFree(alias_name);
	if (program_name) SOMFree(program_name);
	if (server_class) SOMFree(server_class);
	if (host_name) SOMFree(host_name);
	if (impl_id) SOMFree(impl_id);
	if (ior)
	{
		if (ior->_buffer)
		{
			SOMFree(ior->_buffer);
		}

		SOMFree(ior);
	}
}

static void server_delete(Environment *ev)
{
	char buf[256];

	somPrintf("Enter alias : ");

	if (regimpl_get_line(buf,sizeof(buf)))
	{
		if (buf[0])
		{
			regimpl_command_delete_server(ev,buf);
		}
	}
}

static void server_change(Environment *ev)
{
	somPrintf("change server\n");
}

static void server_show_one(Environment *ev)
{
	char buf[256];

	somPrintf("Enter alias : ");

	if (regimpl_get_line(buf,sizeof(buf)))
	{
		if (buf[0])
		{
			ImplementationDef SOMSTAR impl=
				ImplRepository_find_impldef_by_alias(SOMD_ImplRepObject,ev,buf);

			if (impl && !ev->_major)
			{
				show_impl(impl,ev);
				somReleaseObjectReference(impl);
			}
		}
	}
}

static void class_list(Environment *ev)
{
	char impl[256];

	somPrintf("Enter alias : ");

	if (regimpl_get_line(impl,sizeof(impl)))
	{
		somPrintf("\n");

		if (impl[0])
		{
			regimpl_command_list_classes(ev,impl);
		}
	}
}

static void class_add(Environment *ev)
{
	char impl[256];

	somPrintf("Enter alias : ");

	if (regimpl_get_line(impl,sizeof(impl)))
	{
		if (impl[0])
		{
			char cls[256];

			somPrintf("Enter class : ");

			if (regimpl_get_line(cls,sizeof(cls)))
			{
				regimpl_command_add_class(ev,impl,cls);
			}
		}
	}
}

static void class_delete(Environment *ev)
{
	char impl[256];

	somPrintf("Enter alias : ");

	if (regimpl_get_line(impl,sizeof(impl)))
	{
		if (impl[0])
		{
			char cls[256];

			somPrintf("Enter class : ");

			if (regimpl_get_line(cls,sizeof(cls)))
			{
				regimpl_command_remove_class(ev,impl,cls);
			}
		}
	}
}

static void regimpl_menu(Environment *ev)
{
	while (regimpl_running)
	{
		int i;
		char buf[256];

		print_main_menu();

		somPrintf("\nEnter an operation : ");

		if (regimpl_get_line(buf,sizeof(buf)))
		{
			somPrintf("\n");

			i=0;

			if (sscanf(buf,"%d",&i))
			{
				switch (i)
				{
				case 1:
					server_add(ev);
					break;
				case 2:
					server_delete(ev); 
					break;
				case 3:
					server_change(ev); 
					break;
				case 4:
					server_show_one(ev); 
					break;
				case 5: 
					regimpl_command_list_all_servers(ev); 
					break;
				case 6: 
					regimpl_command_list_all_aliases(ev); 
					break;
				case 7:
					class_add(ev);
					break;
				case 8:
					class_delete(ev);
					break;
				case 9:
					class_delete_all(ev);
					break;
				case 10:
					class_list(ev);
					break;
				case 11:
					class_add_all(ev);
					break;
				case 12:
					regimpl_running=0;
					break;
				default:
					somPrintf("Unknown option %d\n",i);
					break;
				}
			}
		}
		else
		{
			somPrintf("Setting regimpl_running to zero....!!!!\n");
			regimpl_running=0;
		}

		if (ev->_major)
		{
			char *p=somExceptionId(ev);

			somPrintf("%s\n",p);

			somExceptionFree(ev);
		}
	}
}

#ifdef USE_THREADS
struct regimpl_menu_threaded_t
{
#ifdef USE_PTHREADS
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	boolean running;
#endif
	Environment *ev;
};
static 
#if defined(USE_PTHREADS)
void *
#else
unsigned long __stdcall
#endif
regimpl_menu_thread(void *pv)
{
	struct regimpl_menu_threaded_t *thunk=pv;

	regimpl_menu(thunk->ev);
#ifdef USE_PTHREADS
	pthread_mutex_lock(&thunk->mutex);
	thunk->running=0;
	pthread_cond_signal(&thunk->cond);
	pthread_mutex_unlock(&thunk->mutex);
	return NULL;
#else
	return 0;
#endif
}

static void regimpl_menu_threaded(Environment *ev)
{
	struct regimpl_menu_threaded_t thunk;
#ifdef USE_PTHREADS
	pthread_t thr;
	void *pv=NULL;
	pthread_cond_init(&thunk.cond,RHBOPT_pthread_condattr_default);
	pthread_mutex_init(&thunk.mutex,RHBOPT_pthread_mutexattr_default);
	thunk.running=1;
#else
	HANDLE hThread;
	DWORD dw;
#endif
	thunk.ev=ev;


#ifdef USE_PTHREADS
	pthread_mutex_lock(&thunk.mutex);

	pthread_create(&thr,RHBOPT_pthread_attr_default,
			regimpl_menu_thread,
			&thunk);
			
	while (thunk.running)
	{
#ifdef _PLATFORM_MACINTOSH_
		pthread_mutex_unlock(&thunk.mutex);
		{
			EventRecord event;
			WaitNextEvent(-1,&event,500,NULL);
			SOMTerminal_HandleEvent(&event);
		}
		pthread_mutex_lock(&thunk.mutex);
#else
		pthread_cond_wait(&thunk.cond,&thunk.mutex);
#endif
	}
	pthread_mutex_unlock(&thunk.mutex);
	pthread_join(thr,&pv);
	pthread_cond_destroy(&thunk.cond);
	pthread_mutex_destroy(&thunk.mutex);
#else
	hThread=CreateThread(NULL,0,regimpl_menu_thread,&thunk,0,&dw);
	WaitForSingleObject(hThread,INFINITE);
	CloseHandle(hThread);
#endif
}
#endif

#ifdef _PLATFORM_MACINTOSH_
#define STACK_SIZE   32768
QDGlobals qd;

#ifdef USE_THREADS
static void regimpl_callback(rhbpkern_thread_struct *thr,void *refCon,long del)
{
	if (del <=0) del=0;
	if (del > 500) del=500;
		
	if (SOMTerminal_Exists())
	{
		EventRecord event;
		RgnHandle h;
		Point pt;
		Rect r;
		GrafPtr port;
		
		GetWMgrPort(&port);
		
		h=NewRgn();
		
		SetPort(port);
		GetMouse(&pt);
		
		r.left=pt.h; r.top=pt.v;
		r.right=r.left+1; r.bottom=r.top+1;
		RectRgn(h,&r);
		
		WaitNextEvent(-1,&event,del,h);

		DisposeRgn(h);

		SOMTerminal_HandleEvent(&event);
	}
	else
	{
		EventRecord event;
		WaitNextEvent(-1,&event,del,0);
		SOMTerminal_HandleEvent(&event);
	}
}

struct SOMD_Uninit_r
{
	int running;
	Environment *ev;
};
static void SOMD_Uninit_cleanup(void *pv)
{
struct SOMD_Uninit_r *data=pv;
	data->running=0;
}
static void *SOMD_Uninit_start(void *pv)
{
struct SOMD_Uninit_r *data=pv;
	RHBOPT_cleanup_push(SOMD_Uninit_cleanup,pv);
	SOMD_Uninit(data->ev);
	RHBOPT_cleanup_pop();
	return pv;
}
static void SOMD_Uninit_r(Environment *ev)
{
struct SOMD_Uninit_r data={1,NULL};
pthread_t tid;
	data.ev=ev;

	if (!pthread_create(&tid,NULL,SOMD_Uninit_start,&data))
	{
		void *pv;
		while (data.running)
		{
			somPrintf(".");
			regimpl_callback(NULL,NULL,500);
		}
		
		pthread_join(tid,&pv);
	}
	else
	{
		SOMD_Uninit_start(&data);
	}
}
#endif

int main()
{
Environment ev;

	SetApplLimit((Ptr)(((unsigned long)GetApplLimit())-STACK_SIZE));
	
	MaxApplZone();

	InitGraf(&qd.thePort);
		
	if (SOMTerminal_Open(&qd))
	{
		return 0;
	}

	somEnvironmentNew();

#ifdef USE_THREADS
/*	rhbpkern_set_callback(regimpl_callback,0); */
#endif

	somSetOutChar(SOMTerminal_OutChar);

	SOM_InitEnvironment(&ev);

#ifdef __ASLM__
	TRY
#endif

	SOMD_Init(&ev);
	
#ifdef __ASLM__
	CATCH_ALL
	
	somPrintf("exception %d occurred\n",except.fError);
	RHBOPT_throw_StExcep(&ev,UNKNOWN,NoSOMDInit,NO);
	
	ENDTRY
#endif

	if (!ev._major)
	{
#ifdef USE_THREADS
		regimpl_menu_threaded(&ev);
#else
		regimpl_menu(&ev);
#endif

		if (ev._major)
		{
			char *p=somExceptionId(&ev);
			somPrintf("regimpl finished with %s\n",p);
			somExceptionFree(&ev);
		}

#if defined(USE_THREADS) && defined(_PLATFORM_MACINTOSH_)
		SOMD_Uninit_r(&ev);
#else
		SOMD_Uninit(&ev);
#endif
	}
	else
	{
		if (ev._major)
		{
			char *p=somExceptionId(&ev);
			somPrintf("regimpl finished with %s\n",p);
		}
	}
	
	SOM_UninitEnvironment(&ev);
	
	somEnvironmentEnd();

	return 0;
}
#else
static void do_quit(int s)
{
	regimpl_running=0;
}

int main(int argc,char **argv)
{
Environment ev;
int rc=1;

	signal(SIGINT,do_quit);
	signal(SIGTERM,do_quit);

	somEnvironmentNew();
	
	SOM_InitEnvironment(&ev);

	SOMD_Init(&ev);

	if (ev._major==NO_EXCEPTION)
	{
		if (argc > 1)
		{
			regimpl_command(&ev,argc-1,&argv[1]);
		}
		else
		{
#ifdef USE_THREADS
			regimpl_menu_threaded(&ev);
#else
			regimpl_menu(&ev);
#endif
		}

		if (ev._major)
		{
			char *p=somExceptionId(&ev);
			somPrintf("regimpl finished with %s\n",p);
			somExceptionFree(&ev);
		}

		SOMD_Uninit(&ev);

		rc=0;
	}
	else
	{
		somPrintf("%s\n",somExceptionId(&ev));
	}

	SOM_UninitEnvironment(&ev);

	somEnvironmentEnd();

	return rc;
}
#endif


#ifdef _PLATFORM_MACINTOSH_
void doQuit(void)
{
	somPrintf("quit\n");
	regimpl_running=0;
}

OSErr doScript(const char *text,Environment *ev)
{
	somPrintf("script='%s'\n",text);
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	return 0;
}
#endif

#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev,LPSTR lpszCmdLine,INT nCmdShow)
{
	char buf[256];
	if (GetModuleFileName(hInstance,buf,sizeof(buf)))
	{
		char *argv[2]={buf,NULL};
		int argc=1;
		return main(argc,argv);
	}
	return 1;
}
#endif
