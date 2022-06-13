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

SOM_Scope void  SOMLINK somobj_somInit(SOMObject SOMSTAR somSelf)
{
#ifdef SOMObject_somDefaultInit
	somMethodTabPtr mtab=somMethodTabFromObject(somSelf);
	somClassInfo cinfo=somClassInfoFromMtab(mtab);
	somInitCtrl globalCtrl=cinfo->parents.initCtrl;
	SOMObject_somDefaultInit(somSelf,&globalCtrl);
#else
	somMethodTabPtr mtab=somMethodTabFromObject(somSelf);
	somClassInfo cinfo=somClassInfoFromMtab(mtab);
	struct somParentClassInfo *info=cinfo->classes._buffer;
	unsigned int i=cinfo->classes._length;

	while (i--)
	{
		if (info->inherit_var)
		{
			somMethodTabPtr m2=info->cls;
			somClassInfo ci2=somClassInfoFromMtab(m2);

			if (ci2->somInit)
			{
				ci2->somInit(somSelf);
			}
		}

		info++;
	}

	#ifdef SOM_DEBUG_MEMORY_SPRINTF
		somPrintf("%s_somInit(%p);\n",
			somSelf->mtab->className,somSelf);
	#endif
#endif
}

SOM_Scope void  SOMLINK somobj_somUninit(SOMObject SOMSTAR somSelf)
{
#ifdef SOMObject_somDestruct
	somMethodTabPtr mtab=somSelf->mtab;
	somClassInfo ci=somClassInfoFromMtab(mtab);
	somDestructCtrl globalCtrl=ci->parents.destructCtrl;
	SOMObject_somDestruct(somSelf,0,&globalCtrl);
#else
	somMethodTabPtr mtab=somMethodTabFromObject(somSelf);
	somClassInfo cinfo=somClassInfoFromMtab(mtab);
	unsigned int i=cinfo->classes._maximum;
	struct somParentClassInfo *info=cinfo->classes._buffer+i;

	while (i--)
	{
		info--;

		if (info->inherit_var)
		{
			somMethodTabPtr m2=info->cls;
			somClassInfo ci2=somClassInfoFromMtab(m2);

			if (ci2->somUninit)
			{
				ci2->somUninit(somSelf);
			}
		}
	}

	#ifdef SOM_DEBUG_MEMORY_SPRINTF
		somPrintf("%s_somUninit(%p);\n",
			somSelf->mtab->className,somSelf);
	#endif
#endif
}

SOM_Scope SOMClass SOMSTAR  SOMLINK somobj_somGetClass(SOMObject SOMSTAR somSelf)
{
#ifdef SOMClass_somDuplicateReference
	return SOMClass_somDuplicateReference(somMethodTabFromObject(somSelf)->classObject);
#else
	return somMethodTabFromObject(somSelf)->classObject;
#endif
}

SOM_Scope string  SOMLINK somobj_somGetClassName(SOMObject SOMSTAR somSelf)
{
    return somMethodTabFromObject(somSelf)->className;
}

SOM_Scope long  SOMLINK somobj_somGetSize(SOMObject SOMSTAR somSelf)
{
    return somMethodTabFromObject(somSelf)->instanceSize;
}

SOM_Scope boolean  SOMLINK somobj_somIsA(SOMObject SOMSTAR somSelf, 
                                                     SOMClass SOMSTAR aClassObj)
{
	somMethodTabPtr p=somMethodTabFromObject(somSelf);
	somClassInfo info=somClassInfoFromMtab(p);
	unsigned int i=info->classes._length;

	while (i--)
	{
		if (info->classes._buffer[i].cls->classObject==aClassObj)
		{
			return 1;
		}
	}

    return 0;
}

SOM_Scope boolean  SOMLINK somobj_somIsInstanceOf(
		SOMObject SOMSTAR somSelf, 
	    SOMClass SOMSTAR aClassObj)
{
	return (boolean)(somMethodTabFromObject(somSelf)->classObject == aClassObj);
}

SOM_Scope boolean  SOMLINK somobj_somRespondsTo(
		SOMObject SOMSTAR somSelf, 
        somId mId)
{
	if (SOMClass_somGetMethodDescriptor(
			somMethodTabFromObject(somSelf)->classObject,
			mId))
	{
		return 1;
	}

    return 0;
}

SOM_Scope boolean  SOMLINK somobj_somDispatch(SOMObject SOMSTAR somSelf, 
                                                          somToken* retValue, 
                                                          somId methodId, 
                                                          va_list ap)
{
	somMethodData md;
	boolean b=SOMClass_somGetMethodData(
			somMethodTabFromObject(somSelf)->classObject,methodId,&md);

	if (!b) 
	{
#ifdef _DEBUG
		somPrintf("SOMObject::somDispatch(%s,%s) failed\n",
				somMethodTabFromObject(somSelf)->className,
				*methodId);
#endif

		return 0;
	}

	/* need to check that the function we are
		applying on is not the same as the redispatch stub
		*/

#ifdef USE_SOM21
	if (md.method==md.mToken->defined.redispatchStub)
	{
		somPrintf("%s::somDispatch(%s) - aborting recursion\n",
				somSelf->mtab->className,
				somStringFromId(methodId));
		return 0;
	}
#else
	if (md.method==md.mToken->defined.redispatchStub)
	{
		somPrintf("%s::somDispatch(%s) - aborting recursion\n",
				somSelf->mtab->className,
				somStringFromId(methodId));
		return 0;
	}
#endif

	return somApply(somSelf,retValue,&md,ap);
}

SOM_Scope boolean  SOMLINK somobj_somClassDispatch(
		SOMObject SOMSTAR somSelf, 
		SOMClass SOMSTAR clsObj, 
		somToken* retValue, 
		somId methodId, 
		va_list ap)
{
	somPrintf("SOMObject::somClassDispatch not implemented\n");

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(clsObj);
	SOM_IgnoreWarning(retValue);
	SOM_IgnoreWarning(methodId);
	SOM_IgnoreWarning(ap);
    return 0;
}

SOM_Scope boolean SOMLINK somobj_somCastObj(
		SOMObject SOMSTAR somSelf, 
        SOMClass SOMSTAR cls)
{
	somMethodTabPtr p=somMethodTabFromObject(somSelf);
	somClassInfo info=somClassInfoFromMtab(p);
	unsigned int i=info->classes._length;

	while (i--)
	{
		if (info->classes._buffer[i].cls->classObject==cls)
		{
			return 1;
		}
	}

    return 0;
}

SOM_Scope boolean  SOMLINK somobj_somResetObj(
	SOMObject SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);

    return 1;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somobj_somPrintSelf(SOMObject SOMSTAR somSelf)
{
	 somPrintf("An instance of %s at address %p\n",
			somMethodTabFromObject(somSelf)->className,
			somSelf);

     return somSelf;
}

SOM_Scope void  SOMLINK somobj_somDumpSelf(SOMObject SOMSTAR somSelf, 
                                                       long level)
{
	 somPrefixLevel(level);
/*	 somPrintf("An instance of %s at address %p\n",
			somMethodTabFromObject(somSelf)->className,
			somSelf);*/
	 SOMObject_somPrintSelf(somSelf);
	 somPrefixLevel(level);
	 somPrintf("{\n");
	 SOMObject_somDumpSelfInt(somSelf,level+1);
	 somPrefixLevel(level);
	 somPrintf("}\n");
}

SOM_Scope void  SOMLINK somobj_somDumpSelfInt(
		SOMObject SOMSTAR somSelf, 
        long level)
{
#ifdef SOMObject_somRelease
	somPrefixLevel(level);
	somPrintf("count=%ld\n",(long)SOMObjectGetData(somSelf)->lUsage);
#else
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(level);
#endif
}

#ifdef SOMObject_somCanDelete
SOM_Scope boolean SOMLINK somobj_somCanDelete(
	SOMObject SOMSTAR somSelf)
{
	return 1;
}
#endif

SOM_Scope void  SOMLINK somobj_somFree(SOMObject SOMSTAR somSelf)
{
#ifdef SOMObject_somDestruct
	somMethodTabPtr mtab=somSelf->mtab;
	somClassInfo ci=somClassInfoFromMtab(mtab);
	somDestructCtrl globalCtrl=ci->parents.destructCtrl;
	SOMObject_somDestruct(somSelf,1,&globalCtrl);
#else
	SOMObject_somUninit(somSelf);
	SOMClass_somDeallocate(
		somMethodTabFromObject(somSelf)->classObject,
			(char *)somSelf);
#endif
}

/* VLAD type specific dispatchers, obsolete */

SOM_Scope void SOMLINK somobj_somDispatchV(SOMObject SOMSTAR somSelf,somId id,somId desc,va_list ap)
{
	somToken token=NULL;
	SOMObject_somDispatch(somSelf,&token,id ? id : desc,ap);
}

SOM_Scope somToken SOMLINK somobj_somDispatchA(SOMObject SOMSTAR somSelf,somId id,somId desc,va_list ap)
{
	somToken token=NULL;
	if (SOMObject_somDispatch(somSelf,&token,id ? id : desc,ap)) return token;
	return NULL;
}

SOM_Scope long SOMLINK somobj_somDispatchL(SOMObject SOMSTAR somSelf,somId id,somId desc,va_list ap)
{
	long l=0;
	if (SOMObject_somDispatch(somSelf,(somToken *)(void *)&l,id ? id : desc,ap)) return l;
	return 0;
}

SOM_Scope double SOMLINK somobj_somDispatchD(SOMObject SOMSTAR somSelf,somId id,somId desc,va_list ap)
{
	double dbl=0.0;
	if (SOMObject_somDispatch(somSelf,(somToken *)(void *)&dbl,id ? id : desc,ap)) return dbl;
	return 0.0;
}

/* the following are migrated from SOMDObject */
#ifdef SOMObject_release
SOM_Scope void SOMLINK somobj_release(SOMObject SOMSTAR somSelf,Environment *ev)
{
}
#endif

#ifdef SOMObject_duplicate
SOM_Scope SOMObject SOMSTAR SOMLINK somobj_duplicate(SOMObject SOMSTAR somSelf,Environment *ev)
{
	return somSelf;
}
#endif

#ifdef SOMObject_get_implementation
SOM_Scope ImplementationDef SOMSTAR SOMLINK somobj_get_implementation(SOMObject SOMSTAR somSelf,Environment *ev)
{
	return NULL;
}
#endif

#ifdef SOMObject_create_request
SOM_Scope ORBStatus SOMLINK somobj_create_request(
	SOMObject SOMSTAR somSelf,
	/* out */ Environment *ev,
	/* in */ Context SOMSTAR ctx,
	/* in */ Identifier operation,
	/* in */ NVList SOMSTAR arg_list,
	/* inout */ NamedValue *result,
	/* out */ Request SOMSTAR *request,
	/* in */ Flags req_flags)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}
#endif

#ifdef SOMObject_create_request_args
SOM_Scope ORBStatus SOMLINK somobj_create_request_args(
	SOMObject SOMSTAR somSelf,
	/* out */ Environment *ev,
	/* in */ Identifier operation,
	/* out */ NVList SOMSTAR *arg_list,
	/* out */ NamedValue *result)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}
#endif

#ifdef SOMObject_get_interface
struct somobj_get_interface
{
	somId idLookup;
	Repository SOMSTAR rep;
	char *clsId;
};

RHBOPT_cleanup_begin(somobj_get_interface_cleanup,pv)

struct somobj_get_interface *data=pv;

	if (data->clsId) SOMFree(data->clsId);
	if (data->idLookup) SOMFree(data->idLookup);
	if (data->rep)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		SOMObject_release(data->rep,&ev);
		SOM_UninitEnvironment(&ev);
	}

RHBOPT_cleanup_end

SOM_Scope InterfaceDef SOMSTAR SOMLINK somobj_get_interface(
	SOMObject SOMSTAR somSelf,
	/* out */ Environment *ev)
{
struct somobj_get_interface data={NULL,NULL,NULL};
InterfaceDef SOMSTAR ret=NULL;
char *p=SOMObject_somGetClassName(somSelf);
size_t len=strlen(p);

	RHBOPT_cleanup_push(somobj_get_interface_cleanup,&data);

	data.clsId=SOMMalloc(len+3);
	data.clsId[0]=':';
	data.clsId[1]=':';
	memcpy(data.clsId+2,p,len+1);

	data.idLookup=somIdFromString("lookup_id");

	data.rep=SOMClassMgr__get_somInterfaceRepository(SOMClassMgrObject);

	if (data.rep && (NO_EXCEPTION==ev->_major))
	{
		if (!somva_SOMObject_somDispatch(data.rep,
						(somToken *)(void *)&ret,
						data.idLookup,
						data.rep,ev,data.clsId))
		{
			RHBOPT_throw_StExcep(ev,BAD_OPERATION,DispatchError,NO);
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,INTF_REPOS,IRNotFound,NO);
	}

	RHBOPT_cleanup_pop();

	return ret;
}
#endif

#ifdef SOMObject_is_proxy
SOM_Scope boolean SOMLINK somobj_is_proxy(SOMObject SOMSTAR somSelf,
										  Environment *ev)
{
	return 0;
}
#endif

#ifdef _DEBUG
static struct somParentClassInfo *somobj_get_info_for(
		somMethodTabPtr mtab,somMethodTabPtr parent)
{
	somClassInfo info=somClassInfoFromMtab(mtab);
	unsigned int i=info->classes._length;
	struct somParentClassInfo *b=info->classes._buffer;

	while (i--)
	{
		if (b->cls==parent) 
		{
			return b;
		}

		b++;	
	}

	RHBOPT_ASSERT(!parent);

	return NULL;
}
#endif

static unsigned long somobj_get_num_parents(SOMObject SOMSTAR somSelf,
											SOMClass SOMSTAR cls)
{
	somMethodTabPtr mtab=somSelf->mtab;
	somClassInfo ci=somClassInfoFromMtab(mtab);
	unsigned long i=ci->classes._length;
	struct somParentClassInfo *info=ci->classes._buffer;
	while (i--)
	{
		if (info->cls->classObject==cls)
		{
			return somClassInfoFromMtab(info->cls)->numParents;
		}

		info++;
	}

	return 0;
}

SOM_Scope void SOMLINK somobj_somDestruct(SOMObject SOMSTAR somSelf,
										  boolean doFree,
										  somDestructCtrl *ctrl)
{
	somMethodTabPtr mtab=somSelf->mtab;
	somClassInfo ci=somClassInfoFromMtab(mtab);
	somDestructCtrl globalCtrl;
	unsigned long i=ci->numParents;

	if (!ctrl)
	{
		globalCtrl=ci->parents.destructCtrl;
/*		memcpy(&globalCtrl,&ci->parents.destructCtrl,sizeof(globalCtrl));*/
		ctrl=&globalCtrl;
	}

	i=somobj_get_num_parents(somSelf,ctrl->info->cls);

#ifdef _DEBUG_X
	somPrintf("somobj destructor i=%d for ::%s\n",i,SOMClass_somGetName(ctrl->info->cls));
#endif

	/* a good opportunity to call the legacy somUninit! */

	if (ctrl->info->legacyUninit)
	{
/*		somPrintf("legacyUninit(%s,~%s)\n",
			somSelf->mtab->className,
			SOMClass_somGetName(ctrl->info->cls));*/
		((somTD_SOMObject_somUninit)ctrl->info->legacyUninit)
			(somSelf);
	}

	if (i && ctrl->mask)
	{
		/* _BeginDestructor */
		const boolean * myMask=ctrl->mask;

		/* _EndDestructor */

		RHBOPT_ASSERT((!ctrl->info->dataOffset)||(((char *)somSelf)+ctrl->info->dataOffset)==
			(somDataResolve(somSelf,SOMClass_somGetInstanceToken(ctrl->info->cls))));

		ctrl->info=(somDestructInfo *)(((char *)ctrl->info)+ctrl->infoSize);
		ctrl->mask+=i;

		while (i--)
		{
			/* need to assert that we have not run off the end of
				the mask */

			RHBOPT_ASSERT(myMask < (ci->parents.destructCtrl.mask+ci->max_mask_length));
			RHBOPT_ASSERT(myMask >= (ci->parents.destructCtrl.mask));

			if ((*(myMask++)) & 1)
			{
				RHBOPT_ASSERT(somTestCls(somSelf,ctrl->info->cls,__FILE__,__LINE__));

/*				somPrintf("somDestruct[%d](%s::~%s)\n",
					i,
					somSelf->mtab->className,
					SOMClass_somGetName(ctrl->info->cls));

				somPrintf("ctrl=%p\n",ctrl);
				somPrintf("ctrl->info=%p\n",ctrl->info);
				somPrintf("ctrl->info->cls%p\n",ctrl->info->cls);
				somPrintf("ctrl->info->defaultDestruct=%p\n",ctrl->info->defaultDestruct);
*/

				RHBOPT_ASSERT(
					somobj_get_info_for(
						somSelf->mtab,
						SOMClass_somGetClassMtab(ctrl->info->cls))->inherit_var);

				RHBOPT_ASSERT(ctrl->info->defaultDestruct);

				((somTD_SOMObject_somDestruct)ctrl->info->defaultDestruct)(somSelf,0,ctrl);

/*				somPrintf("somDestruct[%d](%s::~%s) returned\n",
					i,
					somSelf->mtab->className,
					SOMClass_somGetName(ctrl->info->cls));
					*/
			}
		}
	}

/*	somPrintf("somDestruct, doFree=%d\n",doFree);*/

	if (doFree)
	{
		/* free with the original class of the object */
		SOMClass_somDeallocate(mtab->classObject,(void *)somSelf);
	}
}

#define walker_return_bit  1
#define walker_arg_bit     2

struct walker_info
{
	octet call_style;		/* ret/arg flags */
	size_t ctrl_offset;		/* offset to somAssignCtrl or somInitCtrl in somParentMtabStruct */
	size_t info_offset;		/* offset of function in somInitInfo/somAssignInfo */
	size_t legacy_offset;	/* somInit */
	somMToken *pToken;		/* method token if not using offset */
};

#define WALKER_CTRL(x)		  (size_t)&((somParentMtabStruct *)0)->x
#define WALKER_CALL(x,y)	  (size_t)&((x *)0)->y

static void * somobj_generic_walker(struct walker_info *winfo,
									SOMObject SOMSTAR somSelf,
									void *pvCtrl,
									void *arg)
{
	somInitCtrl *ctrl=pvCtrl;
	void *retVal=somSelf;
	somMethodTabPtr mtab=somSelf->mtab;
	somClassInfo ci=somClassInfoFromMtab(mtab);
	somInitCtrl globalCtrl;
	unsigned long i=ci->numParents;
	somInitInfo *info;
#if 0
	SOMClass SOMSTAR classLevel=ctrl ? ctrl->info->cls : somSelf->mtab->classObject;
#endif

	if (ctrl)
	{
		i=somobj_get_num_parents(somSelf,ctrl->info->cls);
	}
	else
	{
		somInitCtrl *ptr=(void *)(winfo->ctrl_offset+(char *)&ci->parents);

		RHBOPT_ASSERT((ptr==&ci->parents.initCtrl)||
					  (ptr==(void *)&ci->parents.assignCtrl));

		globalCtrl=*ptr;

		ctrl=&globalCtrl;
	}

	info=ctrl->info;

#if 0
	{
		char *p=SOMClass_somGetName(info->cls);
		somPrintf("somobj_generic_walker(%p,%s,%d)\n",somSelf,p,i);
		somPrintf(":: i=%d,mask=%p,size=%ld\n",i,ctrl->mask,(long)ctrl->infoSize);

		if (!strcmp(p,"Simple::SimpleBase"))
		{
			somPrintf("here we go...\n");
		}
	}
#endif

#if 0
	{
		const somInitCtrl *root=(void *)(winfo->ctrl_offset+(char *)&ci->parents);
		size_t i=ctrl->mask-root->mask;
		size_t j=info-root->info;

		somPrintf("root->mask=%p, ctrl->mask=%p,diff=%d\n",
				root->mask,
				ctrl->mask,
				(int)i);
		somPrintf("root->info=%p, ctrl->info=%p,diff=%d\n",
				root->info,
				info,
				(int)j);
	}
#endif

	if (i && ctrl->mask)
	{
		somBooleanVector myMask=ctrl->mask;

		RHBOPT_ASSERT((!ctrl->info->dataOffset)||(((char *)somSelf)+ctrl->info->dataOffset)==
					(somDataResolve(somSelf,SOMClass_somGetInstanceToken(ctrl->info->cls))));

		ctrl->mask+=i;
		ctrl->info=(somInitInfo *)(((char *)ctrl->info)+ctrl->infoSize);

		while (i--)
		{
#if 0
			somPrintf(":: exec(%d),myMask[0]=%d\n",(int)i,(int)myMask[0]);
#endif

			if (*myMask++ & 1)
			{
				somMethodPtr method=
					winfo->pToken 
					?
					somClassResolve(ctrl->info->cls,winfo->pToken[0])
					:
					*((somMethodPtr *)(void *)(winfo->info_offset+(char *)ctrl->info))
					;

#if 0
				{
					_IDL_SEQUENCE_SOMClass parents=SOMClass_somGetParents(classLevel);

					RHBOPT_ASSERT(parents._length)
					RHBOPT_ASSERT(i < parents._length)
					RHBOPT_ASSERT(ctrl->info->cls==
								parents._buffer[parents._length-1-i]);
					SOMFree(parents._buffer);
				}
#endif
				RHBOPT_ASSERT(somTestCls(somSelf,ctrl->info->cls,__FILE__,__LINE__));

				RHBOPT_ASSERT(
					somobj_get_info_for(
						somSelf->mtab,
						SOMClass_somGetClassMtab(ctrl->info->cls))->inherit_var);

#if 0
				somPrintf(":: call_style=%d,ctrl_offset=%d,info_offset=%d,legacy_offset=%d,method=%p\n",
						(int)winfo->call_style,
						(int)winfo->ctrl_offset,
						(int)winfo->info_offset,
						(int)winfo->legacy_offset,
						method);

				somPrintf("trying to call into parent class %s\n",
						SOMClass_somGetName(ctrl->info->cls));
#endif

				RHBOPT_ASSERT(method);

				switch (winfo->call_style)
				{
				case 0:
					((somTD_SOMObject_somDefaultInit)method)(somSelf,ctrl);
					break;
				case walker_arg_bit:
					((somTD_SOMObject_somDefaultConstCopyInit)method)(somSelf,ctrl,arg);
					break;
				case walker_arg_bit|walker_return_bit:
					retVal=((somTD_SOMObject_somDefaultAssign)method)(somSelf,(void *)ctrl,arg);
					break;
				default:
					RHBOPT_ASSERT(!somSelf);
					break;
				}
			}
		}
	}
	else
	{
		/* for all other cases the mask should cut this one off...,
			eg, creating a blank SOMObject is the only reason we should get here */
		RHBOPT_ASSERT(somSelf->mtab->classObject==SOMObjectClassData.classObject);

		ctrl->info=(somInitInfo *)(((char *)ctrl->info)+ctrl->infoSize);
	}

	if (winfo->legacy_offset)
	{
/*		somPrintf(":: legacyInit==%p\n",info->legacyInit);
*/

		RHBOPT_ASSERT((&info->legacyInit)==(void *)(winfo->legacy_offset+(char *)info));

		if (info->legacyInit)
		{
			((somTD_SOMObject_somInit)info->legacyInit)(somSelf);
		}
	}

	return retVal;
}

SOM_Scope void SOMLINK somobj_somDefaultInit(SOMObject SOMSTAR somSelf,
									 somInitCtrl *ctrl)
{
static struct walker_info walker={
		0,
		WALKER_CTRL(initCtrl),
		WALKER_CALL(somInitInfo,defaultInit),
		WALKER_CALL(somInitInfo,legacyInit),
		NULL
	};
	somobj_generic_walker(&walker,somSelf,ctrl,NULL);
}

SOM_Scope void SOMLINK somobj_somDefaultVCopyInit(
		SOMObject SOMSTAR somSelf,
		somInitCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_arg_bit,
		WALKER_CTRL(initCtrl),
		0,
		WALKER_CALL(somInitInfo,legacyInit),
		&SOMObjectClassData.somDefaultVCopyInit
	};
	somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope void SOMLINK somobj_somDefaultCopyInit(
		SOMObject SOMSTAR somSelf,
		somInitCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_arg_bit,
		WALKER_CTRL(initCtrl),
		WALKER_CALL(somInitInfo,defaultNCArgCopyInit),
		WALKER_CALL(somInitInfo,legacyInit),
		NULL
	};
	somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope void SOMLINK somobj_somDefaultConstCopyInit(
		SOMObject SOMSTAR somSelf,
		somInitCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_arg_bit,
		WALKER_CTRL(initCtrl),
		WALKER_CALL(somInitInfo,defaultConstCopyInit),
		WALKER_CALL(somInitInfo,legacyInit),
		NULL
	};
	somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope void SOMLINK somobj_somDefaultConstVCopyInit(
		SOMObject SOMSTAR somSelf,
		somInitCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_arg_bit,
		WALKER_CTRL(initCtrl),
		0,
		WALKER_CALL(somInitInfo,legacyInit),
		&SOMObjectClassData.somDefaultConstVCopyInit
	};
	somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somobj_somDefaultAssign(
		SOMObject SOMSTAR somSelf,
		somAssignCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_return_bit|walker_arg_bit,
		WALKER_CTRL(assignCtrl),
		WALKER_CALL(somAssignInfo,defaultNCArgAssign),
		0,
		NULL
	};
	return somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somobj_somDefaultConstAssign(
		SOMObject SOMSTAR somSelf,
		somAssignCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_return_bit|walker_arg_bit,
		WALKER_CTRL(assignCtrl),
		WALKER_CALL(somAssignInfo,defaultConstAssign),
		0,
		NULL
	};
	return somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somobj_somDefaultVAssign(
		SOMObject SOMSTAR somSelf,
		somAssignCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_return_bit|walker_arg_bit,
		WALKER_CTRL(assignCtrl),
		0,
		0,
		&SOMObjectClassData.somDefaultVAssign
	};
	return somobj_generic_walker(&walker,somSelf,ctrl,other);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somobj_somDefaultConstVAssign(
		SOMObject SOMSTAR somSelf,
		somAssignCtrl *ctrl,
		SOMObject SOMSTAR other)
{
static struct walker_info walker={
		walker_return_bit|walker_arg_bit,
		WALKER_CTRL(assignCtrl),
		0,
		0,
		&SOMObjectClassData.somDefaultConstVAssign
	};
	return somobj_generic_walker(&walker,somSelf,ctrl,other);
}

#ifdef SOMObject_is_nil
SOM_Scope boolean SOMLINK somobj_is_nil(SOMObject SOMSTAR somSelf,
										Environment *ev)
{
	RHBOPT_ASSERT(somSelf ? somIsObj(somSelf) : 1);

	return (boolean)(somSelf ? 0 : 1);
}
#endif

#ifdef SOMObject_is_nil_migrated
SOM_Scope boolean SOMLINK somobj_is_nil_migrated(SOMObject SOMSTAR somSelf,
										Environment *ev)
{
	RHBOPT_ASSERT(somSelf ? somIsObj(somSelf) : 1);

	return (boolean)(somSelf ? 0 : 1);
}
#endif

