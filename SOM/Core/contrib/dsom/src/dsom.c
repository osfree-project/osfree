/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
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

/* program to drive a SOMDServerMgr object


  arguments


		dsom  cmd  (alias_1  [alias_2]  | -a )

  where -a means all registered implementations


	cmd can be one of

	start
	restart
	stop
	list		(list status)
	disable
	enable

  */

#include <rhbopt.h>
#include <rhbsomex.h>
#include <somd.h>

static void dump_ev(Environment *ev)
{
	somPrintf("%s\n",somExceptionId(ev));
}

static void f_start(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id)
{
	SOMDServerMgr_somdStartServer(mgr,ev,impl_id);
}

static void f_stop(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id)
{
	SOMDServerMgr_somdShutdownServer(mgr,ev,impl_id);
}

static void f_restart(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id)
{
	SOMDServerMgr_somdRestartServer(mgr,ev,impl_id);
}

static void f_list(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id)
{
	ORBStatus status=SOMDServerMgr_somdListServer(mgr,ev,impl_id);
	if (!ev->_major)
	{
		somPrintf("%ld\n",(long)status);
	}
}

static void f_enable(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id)
{
	SOMDServerMgr_somdEnableServer(mgr,ev,impl_id);
}

static void f_disable(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id)
{
	SOMDServerMgr_somdDisableServer(mgr,ev,impl_id);
}

struct
{
	char *cmd;
	void (*func)(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *impl_id);
} funcs[]=
{
	{"start",f_start},
	{"restart",f_restart},
	{"stop",f_stop},
	{"list",f_list},
	{"disable",f_disable},
	{"enable",f_enable},
	{NULL,NULL}
};

static void do_cmd(SOMDServerMgr SOMSTAR mgr,Environment *ev,char *cmd,char *impl_id)
{
	int i=0;

	while (!ev->_major)
	{
		if (funcs[i].cmd)
		{
			if (!strcmp(funcs[i].cmd,cmd))
			{
				funcs[i].func(mgr,ev,impl_id);
				return;
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadMethodName,NO);
			return;
		}

		i++;
	}
}

static int do_cmds(SOMDServerMgr SOMSTAR mgr,Environment *ev,int argc,char **argv)
{
	int retVal=1;
	_IDL_SEQUENCE_string seq={0,0,NULL};

	SOMDServerMgr_somDumpSelf(mgr,0);

	if (argc > 1)
	{
		char *cmd=argv[1];

		if (argc > 2)
		{
			if (!strcmp(argv[2],"-a"))
			{
				if (SOMD_ImplRepObject)
				{
					ImplRepository_find_all_aliases(SOMD_ImplRepObject,ev,&seq);
				}
				else
				{
					RHBOPT_throw_StExcep(ev,INITIALIZE,UnexpectedNULL,NO);
				}
			}
			else
			{
				unsigned int i=0;
				seq._maximum=argc-2;
				seq._length=seq._maximum;
				seq._buffer=SOMMalloc(seq._maximum*sizeof(seq._buffer[0]));

				while (i < seq._length)
				{
					char *p=argv[2+i];
					int j=(int)strlen(p)+1;
					char *q=SOMMalloc(j);
					memcpy(q,p,j);
					seq._buffer[i]=q;
					i++;
				}
			}

			if (!ev->_major)
			{
				unsigned int i=0;

				while ((i < seq._length)&&(!ev->_major))
				{
					do_cmd(mgr,ev,cmd,seq._buffer[i]);

					i++;
				}

				if (!ev->_major)
				{
					retVal=0;
				}
			}
		}
		else
		{
			somPrintf("usage: %s cmd impl_list... | -a\n",argv[0]);
			somPrintf("where cmd is one of start restart stop list disable enable\n");
		}
	}
	else
	{
		if (SOMD_ImplRepObject)
		{
			ImplRepository_find_all_aliases(SOMD_ImplRepObject,ev,&seq);

			if (!ev->_major)
			{
				retVal=0;

				if (seq._length)
				{
					unsigned int i=0;
					while (i < seq._length)
					{
						char *p=seq._buffer[i];
						
						somPrintf("%s\n",p);

						i++;
					}
				}
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,INITIALIZE,UnexpectedNULL,NO);
		}
	}

	if (seq._maximum)
	{
		unsigned long i=0;

		while (i < seq._length)
		{
			char *p=seq._buffer[i++];
			if (p) SOMFree(p);
		}

		SOMFree(seq._buffer);
	}
	return retVal;
}

int main(int argc,char **argv,char **envp)
{
	Environment ev;
	int err=1;
	SOMClassMgr SOMSTAR mgr;
	
	mgr=somEnvironmentNew();

	SOM_InitEnvironment(&ev);

	SOMD_Init(&ev);

	if (!ev._major)
	{
		somId id=somIdFromString("SOMDServerMgr");

		if (id)
		{
			SOMClass SOMSTAR cls=SOMClassMgr_somFindClass(mgr,id,
						SOMDServerMgr_MajorVersion,
						SOMDServerMgr_MinorVersion);

			SOMFree(id); id=NULL;

			if (cls)
			{
				SOMDServerMgr SOMSTAR smgr=SOMClass_somNew(cls);

				if (smgr)
				{
					if (SOMDServerMgr_somIsA(smgr,_SOMDServerMgr))
					{
						err=do_cmds(smgr,&ev,argc,argv);
					}

					SOMDServerMgr_somFree(smgr);
				}
			}
		}

		if (ev._major)
		{
			dump_ev(&ev);
			somdExceptionFree(&ev);
		}

		SOMD_Uninit(&ev);
	}
	else
	{
		dump_ev(&ev);
	}

	SOM_UninitEnvironment(&ev);

	somEnvironmentEnd();

	return err;
}
