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

SOM_Scope SOMClass_somOffsets  SOMLINK 
		somcls__get_somInstanceDataOffsets(
		SOMClass SOMSTAR somSelf)
{
	SOMClass_somOffsets ret={0,0,NULL};
	unsigned int i=0;
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somClassInfo info=&somThis->cimtabs->classInfo;
 
	ret._length=info->classes._length;
	ret._maximum=ret._length;

	ret._buffer=SOMCalloc(sizeof(*ret._buffer),ret._length);

	while (i < ret._length)
	{
		ret._buffer[i].cls=info->classes._buffer[i].cls->classObject;
		ret._buffer[i].offset=info->classes._buffer[i].data_offset;
		i++;
	}

    return ret;
}

#ifdef SOMClass__set_somDirectInitClasses
SOM_Scope void  SOMLINK somcls__set_somDirectInitClasses(
		SOMClass SOMSTAR somSelf, 
		SOMClass_SOMClassSequence* somDirectInitClasses)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

	somPrintf("SOMClass::_set_somDirectInitClasses not implemented\n");
}
#endif

/*static void do_nothing(void *pv) { }*/

SOM_Scope SOMObject SOMSTAR  SOMLINK somcls_somNew(SOMClass SOMSTAR somSelf)
{
	SOMClassData *somThis=SOMClassGetData(somSelf);
	if (somThis->cimtabs)
	{
#ifdef SOMObject_somDefaultInit
		somInitCtrl globalCtrl=somThis->cimtabs->classInfo.parents.initCtrl;
#endif
		long len=somThis->cimtabs->mtab.instanceSize;
		SOMObject SOMSTAR obj=SOMClass_somAllocate(somSelf,len);

		if (obj)
		{
			/* this seems bizarre, why it does not call somRenewNoInit() is
				beyond me, but that is what the somcls.idl says... */

			memset(obj,0,len);

			obj=SOMClass_somRenewNoInitNoZero(somSelf,(void *)obj);

	#ifdef SOMObject_somDefaultInit
			SOMObject_somDefaultInit(obj,&globalCtrl);
	#else
			SOMObject_somInit(obj);
	#endif

			return obj;
		}
	}

    return NULL;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somcls_somNewNoInit(
		SOMClass SOMSTAR somSelf)
{
	SOMClassData *somThis=SOMClassGetData(somSelf);
	if (somThis->cimtabs)
	{
		long len=somThis->cimtabs->mtab.instanceSize;
		SOMObject SOMSTAR obj=SOMClass_somAllocate(somSelf,len);

		if (obj)
		{
			memset(obj,0,len);

			return SOMClass_somRenewNoInitNoZero(somSelf,(void *)obj);
		}
	}

    return NULL;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somcls_somRenew(
	 SOMClass SOMSTAR somSelf, 
     void* obj)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);
#ifdef SOMObject_somDefaultInit
	somInitCtrl globalCtrl=somThis->cimtabs->classInfo.parents.initCtrl;
#endif

	memset(obj,0,somThis->cimtabs->mtab.instanceSize);

	((SOMObject SOMSTAR)obj)->mtab=&somThis->cimtabs->mtab;

#ifdef SOMClass_somDuplicateReference
	SOMClass_somDuplicateReference(somSelf);
#endif

#ifdef SOMObject_somDefaultInit
	SOMObject_somDefaultInit(((SOMObject SOMSTAR)obj),&globalCtrl);
#else
	SOMObject_somInit(((SOMObject SOMSTAR)obj));
#endif

    return (SOMObject SOMSTAR)obj;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somcls_somRenewNoInit(
		SOMClass SOMSTAR somSelf, 
        void* obj)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
#ifdef SOMClass_somDuplicateReference
	SOMClass_somDuplicateReference(somSelf);
#endif

	memset(obj,0,somThis->cimtabs->mtab.instanceSize);

	((SOMObject SOMSTAR)obj)->mtab=&somThis->cimtabs->mtab;

    return (SOMObject SOMSTAR)obj;
}

SOM_Scope void SOMLINK somcls_somDumpSelfInt(
		SOMClass SOMSTAR somSelf,long level)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	_IDL_SEQUENCE_SOMClass parents;
	somPrefixLevel(level);

	somPrintf("className=%s\n",somThis->cimtabs->mtab.className);

	parents=SOMClass_somGetParents(somSelf);
	if (parents._length)
	{
		unsigned int i=0;
		while (i < parents._length)
		{
			somPrefixLevel(level+1);
			somPrintf("parent[%d]=%s\n",i,SOMClass_somGetName(
					parents._buffer[i]));
			i++;
		}
	}
	if (parents._maximum)
	{
		SOMFree(parents._buffer);
	}

	SOMClass_parent_SOMObject_somDumpSelfInt(somSelf,level);
}


SOM_Scope SOMObject SOMSTAR SOMLINK somcls_somRenewNoZero(
		SOMClass SOMSTAR somSelf, 
		void* obj)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);
#ifdef SOMObject_somDefaultInit
	somInitCtrl globalCtrl=somThis->cimtabs->classInfo.parents.initCtrl;
#endif

#ifdef SOMClass_somDuplicateReference
	SOMClass_somDuplicateReference(somSelf);
#endif

	((SOMObject SOMSTAR)obj)->mtab=&somThis->cimtabs->mtab;

#ifdef SOMObject_somDefaultInit
	SOMObject_somDefaultInit(((SOMObject SOMSTAR)obj),&globalCtrl);
#else
	SOMObject_somInit(((SOMObject SOMSTAR)obj));
#endif

    return (SOMObject SOMSTAR)obj;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somcls_somRenewNoInitNoZero(
		SOMClass SOMSTAR somSelf, 
		void* obj)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

#ifdef SOMClass_somDuplicateReference
	SOMClass_somDuplicateReference(somSelf);
#endif

	((SOMObject SOMSTAR )obj)->mtab=&somThis->cimtabs->mtab;

    return ((SOMObject SOMSTAR)obj);
}

SOM_Scope somToken  SOMLINK somcls_somAllocate(
		SOMClass SOMSTAR somSelf, 
		long size)
{
	/* default implementation does not call SOMCalloc */

	SOM_IgnoreWarning(somSelf);

    return SOMMalloc(size);
}

SOM_Scope void  SOMLINK somcls_somDeallocate(SOMClass SOMSTAR somSelf,somToken memptr)
{
	((SOMObject SOMSTAR)memptr)->mtab=NULL;

	SOMFree(memptr);

#ifdef SOMClass_somRelease
	SOMClass_somRelease(somSelf);
#endif
	SOM_IgnoreWarning(somSelf);
}

SOM_Scope void  SOMLINK somcls_somInitClass(SOMClass SOMSTAR somSelf, 
                                                       string className, 
                                                       SOMClass SOMSTAR parentClass, 
                                                       long dataSize, 
                                                       long maxStaticMethods, 
                                                       long majorVersion, 
                                                       long minorVersion)
{
    _IDL_SEQUENCE_SOMClass seq={1,1,NULL};
	seq._buffer=&parentClass;
	SOMClass_somInitMIClass(somSelf,1,className,&seq,dataSize,1,
					maxStaticMethods,majorVersion,minorVersion);
}

static long SOMKERN_count_maxCtrlMask2(somMethodTabs tab,size_t i)
{
	long maxCtrlMask=(long)(i+1);

	while (i--)
	{
		somClassInfo ci=somClassInfoFromMtab(tab->mtab);

		RHBOPT_ASSERT(ci==tab->mtab->classInfo);

		if (ci->numParents)
		{
			maxCtrlMask+=SOMKERN_count_maxCtrlMask2(ci->parents.next,ci->numParents);
		}

		tab=tab->next;
	}

	return maxCtrlMask;
}

static long SOMKERN_count_maxCtrlMask(somMethodTabPtr *tabs,size_t i)
{
	long maxCtrlMask=(long)i;

	while (i--)
	{
		somClassInfo ci=somClassInfoFromMtab(tabs[0]);

		RHBOPT_ASSERT(ci==tabs[0]->classInfo);

		if (ci->numParents)
		{
			maxCtrlMask+=SOMKERN_count_maxCtrlMask2(ci->parents.next,ci->numParents);
		}

		tabs++;
	}

	return maxCtrlMask;
}

static somClassInfo SOMKERN_get_nth_parent(somClassInfo ci,unsigned long i)
{
	somMethodTabs tabs=ci->parents.next;
	struct somParentClassInfo *pci=ci->classes._buffer;

	RHBOPT_ASSERT(ci->numParents > i);

	while (i--)
	{
		tabs=tabs->next;
	}

	RHBOPT_ASSERT(tabs);

	i=ci->classes._length;

	while (i--)
	{
		if (pci->cls==tabs->mtab)
		{
			if (!pci->inherit_var) 
			{
				return NULL;
			}
		}

		pci++;
	}

	return somClassInfoFromMtab(tabs->mtab);
}

static somInitInfo *SOMKERN_get_parent_initInfo(somClassInfo ci,SOMClass SOMSTAR cls)
{
	somMethodTabPtr mtab=SOMClass_somGetClassMtab(cls);
	somClassInfo info=somClassInfoFromMtab(mtab);
	somInitInfo *pinfo=info->parents.initCtrl.info;

	RHBOPT_ASSERT(pinfo->cls==cls);
		
	return pinfo;
}

static somAssignInfo *SOMKERN_get_parent_assignInfo(somClassInfo ci,SOMClass SOMSTAR cls)
{
	somMethodTabPtr mtab=SOMClass_somGetClassMtab(cls);
	somClassInfo info=somClassInfoFromMtab(mtab);
	somAssignInfo *pinfo=info->parents.assignCtrl.info;

	RHBOPT_ASSERT(pinfo->cls==cls);
		
	return pinfo;
}

struct dts_init_info
{
	unsigned long dts_count;
};

#define kDestructCtrl	0
#define kInitCtrl		1
#define kAssignCtrl		2

static boolean SOMKERN_add_constructors(
				struct dts_init_info *dts_init_info,
				somClassInfo newClass,
				somClassInfo fromClass,
				octet magic,
				void *ctrl,
				void *original,
				long maxCtrlMask)
{
	unsigned long i=fromClass->numParents,j=0;
	union
	{
		somInitCtrl *initCtrl;
		somAssignCtrl *assignCtrl;
	} ctrl_u,original_u;
	union
	{
		octet *ptr;
		somInitInfo *initInfo;
		somAssignInfo *assignInfo;
	} info_u,search_u;
	somBooleanVector myMask=NULL;
	
	switch (magic)
	{
	case kInitCtrl:
		ctrl_u.initCtrl=ctrl;
		original_u.initCtrl=original;
		myMask=ctrl_u.initCtrl->mask;
		info_u.initInfo=ctrl_u.initCtrl->info;
		ctrl_u.initCtrl->mask+=i; /* move on past self */
		search_u.initInfo=original_u.initCtrl->info;
		RHBOPT_ASSERT(sizeof(ctrl_u.initCtrl->info[0])==
						ctrl_u.initCtrl->infoSize);
		break;
	case kAssignCtrl:
		ctrl_u.assignCtrl=ctrl;
		original_u.assignCtrl=original;
		myMask=ctrl_u.assignCtrl->mask;
		info_u.assignInfo=ctrl_u.assignCtrl->info;
		ctrl_u.assignCtrl->mask+=i; /* move on past self */
		search_u.assignInfo=original_u.assignCtrl->info;

		RHBOPT_ASSERT(sizeof(ctrl_u.assignCtrl->info[0])==
						ctrl_u.assignCtrl->infoSize);
		break;
	}

	RHBOPT_ASSERT(myMask);

	RHBOPT_ASSERT(fromClass->parents.classObject);
	RHBOPT_ASSERT(newClass->parents.classObject);

	/* always add self first */

	while (search_u.ptr)
	{
		SOMClass SOMSTAR cls=(magic==kInitCtrl) ?
			search_u.initInfo->cls :
			search_u.assignInfo->cls;

		if (!cls) 
		{
			break;
		}

		/* not sure if this should be before or after... */
		if (cls==fromClass->parents.classObject) 
		{
			/* already in list */
			return 0;
		}

		switch (magic)
		{
		case kInitCtrl:
			search_u.ptr+=ctrl_u.initCtrl->infoSize;
			break;
		case kAssignCtrl:
			search_u.ptr+=ctrl_u.assignCtrl->infoSize;
			break;
		}

#if 0
		/* not sure if this should be before or after... */
		if (cls==fromClass->parents.classObject) 
		{
			/* already in list */
			return 0;
		}
#endif
	}

	if (!i) 
	{
		RHBOPT_ASSERT(fromClass->parents.classObject==SOMObjectClassData.classObject);

		/* don't need constructor for SOMObject, eg where numParents==0 */

		return 0;
	}

	dts_init_info->dts_count++;

	switch (magic)
	{
	case kInitCtrl:
		{
			const somInitInfo *from=
				(newClass==fromClass) ? __somdts.SOMObject_initInfo :
				SOMKERN_get_parent_initInfo(newClass,fromClass->parents.classObject);

			info_u.initInfo->cls=fromClass->parents.classObject;
			ctrl_u.initCtrl->info=(void *)(info_u.ptr+ctrl_u.initCtrl->infoSize);

			RHBOPT_ASSERT(from)
			RHBOPT_ASSERT(from->defaultInit)
			RHBOPT_ASSERT(from->defaultCopyInit)
			RHBOPT_ASSERT(from->defaultConstCopyInit)
			RHBOPT_ASSERT(from->defaultNCArgCopyInit)

			info_u.initInfo->defaultInit=from->defaultInit;
			info_u.initInfo->defaultCopyInit=from->defaultCopyInit;
			info_u.initInfo->defaultConstCopyInit=from->defaultConstCopyInit;
			info_u.initInfo->defaultNCArgCopyInit=from->defaultNCArgCopyInit;
			info_u.initInfo->legacyInit=from->legacyInit;

/*			somPrintf("filling in for %s, defaultInit=%p\n",
				SOMClass_somGetName(info_u.initInfo->cls),
				from->defaultInit);*/
		}

		break;
	case kAssignCtrl:
		{
			const somAssignInfo *from=
				(newClass==fromClass) ? __somdts.SOMObject_assignInfo :
				SOMKERN_get_parent_assignInfo(newClass,fromClass->parents.classObject);

			info_u.assignInfo->cls=fromClass->parents.classObject;
			ctrl_u.assignCtrl->info=(void *)(info_u.ptr+ctrl_u.assignCtrl->infoSize);

			RHBOPT_ASSERT(from)
			RHBOPT_ASSERT(from->defaultAssign)
			RHBOPT_ASSERT(from->defaultConstAssign)
			RHBOPT_ASSERT(from->defaultNCArgAssign)
/*			RHBOPT_ASSERT(from->udaAssign)
			RHBOPT_ASSERT(from->udaConstAssign)
*/
			info_u.assignInfo->defaultAssign=from->defaultAssign;
			info_u.assignInfo->defaultConstAssign=from->defaultConstAssign;
			info_u.assignInfo->defaultNCArgAssign=from->defaultNCArgAssign;
			info_u.assignInfo->udaAssign=from->udaAssign;
			info_u.assignInfo->udaConstAssign=from->udaConstAssign;
		}

		break;
	}

	while (i--)
	{
		somClassInfo pci=SOMKERN_get_nth_parent(fromClass,j);

		RHBOPT_ASSERT((myMask+j) < (original_u.initCtrl->mask+maxCtrlMask));

		if (pci)
		{
			octet maskFlag=SOMKERN_add_constructors(dts_init_info,newClass,pci,magic,ctrl,original,maxCtrlMask);
			RHBOPT_ASSERT(!myMask[j]);
			myMask[j++]=maskFlag;

			RHBOPT_ASSERT(!original_u.initCtrl->mask[maxCtrlMask-1]);

			switch (magic)
			{
			case kInitCtrl:
				RHBOPT_ASSERT((myMask+j)<(newClass->parents.initCtrl.mask+maxCtrlMask));
				RHBOPT_ASSERT((myMask+j)>(newClass->parents.initCtrl.mask));
				break;
			case kAssignCtrl:
				RHBOPT_ASSERT((myMask+j)<(newClass->parents.assignCtrl.mask+maxCtrlMask));
				RHBOPT_ASSERT((myMask+j)>(newClass->parents.assignCtrl.mask));
				break;
			}
		}
		else
		{
			/* is not inherited so mask it out... */
			RHBOPT_ASSERT(!myMask[j]);
			myMask[j++]=0;
		}
	}

	return 1;
}


static void SOMKERN_add_destructor_usage(somClassInfo ci,somClassInfo parentClassInfo)
{
	unsigned long i=parentClassInfo->numParents;
	unsigned long x=ci->classes._length;

	while (x--)
	{
/*		20070830 - if substituted, this no longer holds true

		RHBOPT_ASSERT(parentClassInfo->parents.mtab==somMtabFromClassInfo(parentClassInfo));*/

		if (ci->classes._buffer[x].cls==somMtabFromClassInfo(parentClassInfo))
		{
			ci->classes._buffer[x].destructor_usage++;

			break;
		}
	}

	while (i--)
	{
		somClassInfo another=SOMKERN_get_nth_parent(parentClassInfo,i);

		if (another)
		{
			SOMKERN_add_destructor_usage(ci,another);
		}
	}
}

static somDestructInfo *SOMKERN_get_parent_destructInfo(
#if 0
		somClassInfo ci,
#endif
		struct somMethodTabStruct *mtab)
{
	return mtab->classInfo->parents.destructCtrl.info;
#if 0
	unsigned int i=0;

	while (i < ci->numParents)
	{
		somClassInfo parent=SOMKERN_get_nth_parent(ci,i);
		somDestructInfo *pinfo=parent->parents.destructCtrl.info;
		unsigned long size=parent->parents.destructCtrl.infoSize;
		unsigned k=parent->classes._length;

		RHBOPT_ASSERT(size==sizeof(pinfo[0]));

		while (k--)
		{
			if (pinfo->cls==cls) 
			{
				return pinfo;
			}

			if (!pinfo->cls)
			{
				/* come to end of list? */

				break;
			}

			pinfo=(somDestructInfo *)(((char *)pinfo)+size);
		}

		i++;
	}

	somPrintf("failed to find destructInfo for class %s\n",SOMClass_somGetName(cls));

	RHBOPT_ASSERT(!ci);
	
	return NULL;
#endif
}

static boolean SOMKERN_release_destructor(
			struct dts_init_info *dts_init_info,
			somClassInfo newClass,
			somDestructCtrl *ctrl,
			somClassInfo parentClassInfo,
			boolean doAdd,
			long maxCtrlMask)
{
	unsigned int i=newClass->classes._length;
	boolean result=0;

	while (i--)
	{
		/* 20070830 - no longer holds true if substituted

		RHBOPT_ASSERT(parentClassInfo->parents.mtab==somMtabFromClassInfo(parentClassInfo));
		*/

		if (newClass->classes._buffer[i].cls==somMtabFromClassInfo(parentClassInfo))
		{
			unsigned int k=parentClassInfo->numParents;
			somBooleanVector myMask=ctrl->mask;

			RHBOPT_ASSERT((newClass==parentClassInfo) ? (newClass->classes._buffer[i].destructor_usage==1) : 1);

			if (!--(newClass->classes._buffer[i].destructor_usage))
			{
				/* 20070830 */

				somDestructInfo *infoParent=(newClass==parentClassInfo) ? 
						NULL : SOMKERN_get_parent_destructInfo(/*newClass,*/
								somMtabFromClassInfo(parentClassInfo));

				/* assert no longer true due to substituting
				RHBOPT_ASSERT(parentClassInfo->parents.mtab==somMtabFromClassInfo(parentClassInfo));
				*/

/*				somPrintf("adding destructor for %s, %p, %s\n",
							SOMClass_somGetName(newClass->parents.classObject),
							infoParent,
							parent->parents.mtab->className);
*/
				RHBOPT_ASSERT(doAdd);

				RHBOPT_ASSERT(k==newClass->classes._buffer[i].cls->classInfo->numParents);

				if (k /*newClass->classes._buffer[i].cls->classInfo->numParents*/)
				{
					result=1;
				}
				else
				{
					RHBOPT_ASSERT(newClass->classes._buffer[i].cls->classObject==SOMObjectClassData.classObject);
				}

				if (result || !k)
				{
					ctrl->mask+=k;

					/* 20070830 assert no longer true due to substitute 

					RHBOPT_ASSERT(parentClassInfo->parents.mtab==somMtabFromClassInfo(parentClassInfo));
					*/

					ctrl->info->cls=somMtabFromClassInfo(parentClassInfo)->classObject;

					if (infoParent)
					{
						RHBOPT_ASSERT(newClass!=parentClassInfo);
						RHBOPT_ASSERT(infoParent->defaultDestruct);

						{
							RHBOPT_ASSERT(ctrl->info->cls==infoParent->cls);
/*							somPrintf("supposedly for \'%s\', actually have \'%s\'\n",
								SOMClass_somGetName(ctrl->info->cls),
								SOMClass_somGetName(infoParent->cls));*/
						}

#ifdef _DEBUG
						{
							somMethodPtr mptr=somClassResolve(ctrl->info->cls,
									SOMObjectClassData.somDestruct);
							RHBOPT_ASSERT(mptr==infoParent->defaultDestruct);
							if (mptr!=infoParent->defaultDestruct)
							{
								somPrintf("%p,%p\n",mptr,infoParent->defaultDestruct);
							}
						}
#endif

						ctrl->info->legacyUninit=infoParent->legacyUninit;
						ctrl->info->defaultDestruct=infoParent->defaultDestruct;
					}
					else
					{
						RHBOPT_ASSERT(newClass==parentClassInfo);

						ctrl->info->defaultDestruct=(somMethodPtr)somobj_somDestruct;
					}

					ctrl->info=(void *)(((char *)ctrl->info)+ctrl->infoSize);

					dts_init_info->dts_count++;
				}
				else
				{
					doAdd=0;
				}
			}
			else
			{
				doAdd=0;
			}

			while (k--)
			{
				somClassInfo superParent=SOMKERN_get_nth_parent(parentClassInfo,k);
				boolean flag=0;
				
				if (superParent)
				{
					flag=SOMKERN_release_destructor(dts_init_info,newClass,ctrl,superParent,doAdd,maxCtrlMask);
				}

/*somPrintf("maxCtrlMask=%ld,offset=%ld\n",maxCtrlMask,
			(myMask-newClass->parents.destructCtrl.mask));
*/
				RHBOPT_ASSERT((myMask) < (newClass->parents.destructCtrl.mask+maxCtrlMask));

				if (doAdd)
				{
					RHBOPT_ASSERT(!myMask[0])

					*myMask++=flag;

					RHBOPT_ASSERT((myMask)<(newClass->parents.destructCtrl.mask+maxCtrlMask));
					RHBOPT_ASSERT((myMask)>(newClass->parents.destructCtrl.mask));
				}
				else
				{
					RHBOPT_ASSERT(!flag);
				}
			}

			break;
		}
	}

	RHBOPT_ASSERT(i >= 0);

	return result;
}

struct override_info
{
	somMToken *tokenPtr;
	boolean (*handler)(SOMClass SOMSTAR,SOMClassData *,somMToken token,struct override_info *,somMethodPtr);
	long parents_offset;
	long info_offset;
	boolean legacy;
};

static boolean override_generic(SOMClass SOMSTAR somSelf,
							SOMClassData *somThis,
							somMToken token,
							struct override_info *info,
							somMethodPtr method)
{
	somParentMtabStruct *ci=&somThis->cimtabs->classInfo.parents;
	somDestructCtrl *ctrl=(void *)(((char *)ci)+info->parents_offset);
	somMethodPtr *p=(void *)(((char *)ctrl->info)+info->info_offset);

	RHBOPT_ASSERT(ci->destructCtrl.info->cls==somSelf);
	RHBOPT_ASSERT(ci->initCtrl.info->cls==somSelf);
	RHBOPT_ASSERT(ci->assignCtrl.info->cls==somSelf);

/*	somPrintf("override_generic(%s)\n",token->defined.somId_methodDescriptor[0]);*/

#ifdef _DEBUG
	if (token==SOMMTOKEN_DATA(SOMObjectClassData.somDestruct))
	{
		somDestructCtrl *ctl=&ci->destructCtrl;

		RHBOPT_ASSERT(ctl==ctrl);
		RHBOPT_ASSERT(p==&ctl->info->defaultDestruct);
	}
	if (token==SOMMTOKEN_DATA(SOMObjectClassData.somDefaultInit))
	{
		somInitCtrl *ctl=&ci->initCtrl;

		RHBOPT_ASSERT(ctl==(void *)ctrl);
		RHBOPT_ASSERT(p==&ctl->info->defaultInit);
	}
#endif

	*p=method;

	return info->legacy;
}

static struct override_info override_info_map[]=
{
	/* legacy entry points */

	{&SOMObjectClassData.somInit,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,initCtrl),
			SOMKERN_offsetof(somInitInfo,legacyInit),1},
	{&SOMObjectClassData.somUninit,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,destructCtrl),
			SOMKERN_offsetof(somDestructInfo,legacyUninit),1},

	/* destructor entry point */

	{&SOMObjectClassData.somDestruct,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,destructCtrl),
			SOMKERN_offsetof(somDestructInfo,defaultDestruct),0},

	/* init entry points */

	{&SOMObjectClassData.somDefaultInit,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,initCtrl),
			SOMKERN_offsetof(somInitInfo,defaultInit),0},

	{&SOMObjectClassData.somDefaultConstCopyInit,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,initCtrl),
			SOMKERN_offsetof(somInitInfo,defaultConstCopyInit),0},

	{&SOMObjectClassData.somDefaultCopyInit,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,initCtrl),
			SOMKERN_offsetof(somInitInfo,defaultNCArgCopyInit),0},

	/* assign entry points */

	{&SOMObjectClassData.somDefaultAssign,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,assignCtrl),
			SOMKERN_offsetof(somAssignInfo,defaultNCArgAssign),0},

	{&SOMObjectClassData.somDefaultConstAssign,override_generic,
			SOMKERN_offsetof(somParentMtabStruct,assignCtrl),
			SOMKERN_offsetof(somAssignInfo,defaultConstAssign),0}
};

static void SOMKERN_init_dts_restore_SOMObject(
	struct somClassInfoMethodTab *newClass)
{
	/* we need to restore the init/assign/destruct
		to the implementations from SOMObject until
		overridden by the new class,
		as always, drive this from a table 	
	*/

	unsigned short i=sizeof(override_info_map)/sizeof(override_info_map[0]);
	struct override_info *p=override_info_map;

#if 0
	somPrintf("resetting override map %s\n",newClass->mtab.className);
#endif

	while (i--)
	{
		if (!p->legacy)
		{
#ifdef SOM_METHOD_THUNKS
			somMToken token=SOMMTOKEN_DATA(p->tokenPtr[0]);
#else
			somMToken token=p->tokenPtr[0];
#endif

#if 0
			somPrintf("resetting specific %s,methodPtr=%p\n",
				token->defined.somId_methodDescriptor[0],
				token->defined.method);
#endif

			RHBOPT_ASSERT(SOMMTOKEN_DATA(token)==token);

			RHBOPT_ASSERT(token->index == token->index_total)

			newClass->mtab.entries[token->index]=token->defined.method;
		}

		p++;
	}
}

SOM_Scope somMToken  SOMLINK somcls_somAddStaticMethod(
		SOMClass SOMSTAR somSelf, 
		somId methodId, 
		somId methodDescriptor, 
		somMethodPtr method, 
		somMethodPtr redispatchStub, 
		somMethodPtr applyStub)
{
	SOMClassData *somThis=SOMClassGetData(somSelf);
	somMToken result=NULL;
	somMethodTabPtr mtab=&somThis->cimtabs->mtab;

	if (mtab)
	{
		somClassInfo info=&somThis->cimtabs->classInfo;

		if ((info->jump_table._length < info->jump_table._maximum)
			&&
			(info->added_methods._length < info->added_methods._maximum))
		{
			result=info->added_methods._buffer+info->added_methods._length;

			result->classInfoOwner=mtab;

			result->defined.type=2;
			result->defined.somId_methodId=(somConstId)methodId;
			result->defined.somId_methodDescriptor=(somConstId)methodDescriptor;
			result->defined.method=method;
			result->defined.redispatchStub=redispatchStub;
			result->defined.applyStub=applyStub;

			result->index=(SOMKERN_mtoken_index_t)info->added_methods._length;
			result->index_total=(SOMKERN_mtoken_index_t)info->jump_table._length;

#ifdef SOM_METHOD_THUNKS
			result->jumper.resolve.mToken=result;
			result->jumper.direct.mToken=result;
	#ifdef SOM_METHOD_STUBS
			result->jumper.apply.jump_instr=somMToken_call_dword_ptr;
			result->jumper.apply.jump_address=&somkern_jump_apply;
			result->jumper.redispatch.jump_instr=somMToken_call_dword_ptr;
			result->jumper.redispatch.jump_address=&somkern_jump_redispatch;
	#endif

			result->jumper.resolve.jump_thunk.jump_instr=somMToken_call_dword_ptr;
			result->jumper.resolve.jump_thunk.jump_address=&somkern_jump_standard;

			if (-1L == (long)redispatchStub)
			{
				somApRdInfo *abi=(void *)applyStub;

				if (abi && abi->stubInfo)
				{
					somMethodInfo *stubInfo=abi->stubInfo;

					if ( (stubInfo->callType & ~1) == SOMRdRetaggregate)
					{
						result->jumper.resolve.jump_thunk.jump_address=&somkern_jump_aggregate;
					}
				}
			}
	#ifdef somMToken_setjmp
			somMToken_setjmp(&(result->jumper.direct),method);
	#endif
#endif

			mtab->entries[result->index_total]=method;

			info->jump_table._length++;
			info->added_methods._length++;
		}
	}

    return result;
}

SOM_Scope void  SOMLINK somcls_somAddDynamicMethod(
		SOMClass SOMSTAR somSelf, 
		somId methodId, 
		somId methodDescriptor, 
		somMethodPtr method, 
		somMethodPtr applyStub)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somMToken m=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (m)
	{
		unsigned int i=SOMKERN_resolve_index(&somThis->cimtabs->mtab,m);
		somMethodPtr *je=somThis->cimtabs->mtab.entries+i;

#ifdef SOM_DEBUG_JUMP_TABLE
		je->method=method;
#else
		*je=method;
#endif
#ifdef USE_IS_OVERRIDDEN
		je->is_overridden=1;
#endif
	}
	else
	{
		somPrintf("SOMClass::somAddDynamicMethod(%s) failed\n",somStringFromId(methodId));

		SOM_IgnoreWarning(methodDescriptor);
		SOM_IgnoreWarning(applyStub);
	}
}

SOM_Scope void  SOMLINK somcls_somOverrideSMethod(
		SOMClass SOMSTAR somSelf, 
		somId methodId, 
		somMethodPtr method)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);
	somMToken m=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

/*	somPrintf("SOMClass_somOverrideSMethod(%s,%s,%p)\n",
				SOMClass_somGetName(somSelf),
				somStringFromId(methodId),
				method);
*/
	RHBOPT_ASSERT(m)

/*	if ((!method)||(!m))
	{
		somPrintf("fatal!\n");
		exit(1);
	}
*/
	if (m)
	{
		int i=sizeof(override_info_map)/sizeof(override_info_map[0]);
		struct override_info *p=override_info_map;

		while (i--)
		{
			if (SOMMTOKEN_DATA(p->tokenPtr[0])==m)
			{
/*				somPrintf("has special handler...\n");*/

				if (p->handler(somSelf,somThis,m,p,method))
				{
					/* this is a legacy handler */
					return;
				}

				break;
			}

			p++;
		}

		/* override the main entry points with the
			new implementation */

		RHBOPT_ASSERT(m!=SOMObjectClassData.somInit);
		RHBOPT_ASSERT(m!=SOMObjectClassData.somUninit);

		{
			unsigned int i=SOMKERN_resolve_index(&somThis->cimtabs->mtab,m);
			somThis->cimtabs->mtab.entries[i]=method;

#ifdef SOM_METHOD_THUNKS
			/* there is a small window of opportunity
				where as we are refixing the somMToken another
				thread is in the middle of the call/jmp instruction

				so we should do this fixup 
					(a) in a second somMToken which we then switch
						back into the ClassData structure

					(b) only if it has not been previously overridden
			*/

			/* special thread handling to cater for updating
			   the method token while a call is physically
			   going through the one reference in the ClassData,
			
			  eg while override Y::M, a call is going through the token
			  for X::M
			*/

			if (m->defined.classData)
			{
				if (m->defined.classData[0]==(somMToken)&(m->jumper.direct))
				{
					m->defined.classData[0]=(somMToken)&(m->jumper.resolve);
				}
			}
#endif
		}
	}
}

SOM_Scope void  SOMLINK somcls_somClassReady(
		SOMClass SOMSTAR somSelf)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);

	SOMKERN_post_process(somThis->cimtabs->classInfo.sci,&somThis->cimtabs->mtab);

	SOMClassMgr_somRegisterClass(SOMClassMgrObject,somSelf);*/

	SOMClassMgr SOMSTAR clsMgr=somEnvironmentNew();
	SOMClassMgr_somRegisterClass(clsMgr,somSelf);
}

SOM_Scope somClassDataStructure * SOMLINK somcls_somGetClassData(
	SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	if (somThis->cimtabs->classInfo.sci)
	{
		return somThis->cimtabs->classInfo.sci->cds;
	}

    return somThis->cds;
}

SOM_Scope void SOMLINK somcls_somSetClassData(
		SOMClass SOMSTAR somSelf, 
        somClassDataStructure* cds)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	RHBOPT_ASSERT(!somThis->cds);

	somThis->cds=cds;
}

SOM_Scope somMethodTab*  SOMLINK somcls_somGetClassMtab(
	SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

    return &somThis->cimtabs->mtab;
}

#ifdef SOMClass_somGetInstanceOffset
SOM_Scope long  SOMLINK somcls_somGetInstanceOffset(
		SOMClass SOMSTAR somSelf)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

    somPrintf("SOMClass::somGetInstanceOffset is obsolete\n");

    return 0;
}
#endif

SOM_Scope long  SOMLINK somcls_somGetInstancePartSize(
	  SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

    return somThis->cimtabs->classInfo.instancePartSize;
}

SOM_Scope long  SOMLINK somcls_somGetInstanceSize(
	SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

    return somThis->cimtabs->mtab.instanceSize;
}

SOM_Scope somDToken  SOMLINK somcls_somGetInstanceToken(
	SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

    return &(somThis->cimtabs->classInfo.data_token);
}

SOM_Scope somDToken  SOMLINK somcls_somGetMemberToken(
		SOMClass SOMSTAR somSelf, 
		long memberOffset, 
		somDToken instanceToken)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

	somPrintf("SOMClass::somGetMemberToken not implemented\n");

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(memberOffset);
	SOM_IgnoreWarning(instanceToken);

    return 0;
}

SOM_Scope boolean  SOMLINK somcls_somGetMethodData(
		SOMClass SOMSTAR somSelf, 
		somId methodId, 
		somMethodData* md)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);
	somMToken m=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (m)
	{
		md->id=(somId)(m->defined.somId_methodId);
		md->type=m->defined.type;
		md->descriptor=(somId)(m->defined.somId_methodDescriptor);
		md->mToken=m;
		md->method=SOMKERN_resolve(&somThis->cimtabs->mtab,m);
		md->shared=(-1 == (long)m->defined.redispatchStub) ? 
					(void*)m->defined.applyStub : NULL;

		return 1;
	}

    return 0;
}

SOM_Scope somMethodProc*  SOMLINK somcls_somGetRdStub(
		SOMClass SOMSTAR somSelf, 
        somId methodId)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somMToken m=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (m)
	{
		if (-1==(long)(m->defined.redispatchStub))
		{
			somApRdInfo *info=(void *)m->defined.applyStub;

			if (info)
			{
				if (info->rdStub) return info->rdStub;
			}

#ifdef SOM_METHOD_STUBS
			return (somMethodPtr)&(m->jumper.redispatch);
#else
			RHBOPT_ASSERT(!somSelf)
#endif
		}
		else
		{
			return m->defined.redispatchStub;
		}
	}

    return NULL;
}

SOM_Scope somId  SOMLINK somcls_somGetMethodDescriptor(
	SOMClass SOMSTAR somSelf, 
    somId methodId)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);
	somMToken m=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (m)
	{
		return (somId)(m->defined.somId_methodDescriptor);
	}

    return NULL;
}

SOM_Scope long  SOMLINK somcls_somGetMethodIndex(
		SOMClass SOMSTAR somSelf, 
		somId id)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);
	somMToken m=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,id);

	if (m)
	{
		return SOMKERN_resolve_index(&somThis->cimtabs->mtab,m);
	}

    return -1;
}

SOM_Scope somMToken  SOMLINK somcls_somGetMethodToken(
	SOMClass SOMSTAR somSelf, 
	somId methodId)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);

	return SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);
}

SOM_Scope string  SOMLINK somcls_somGetName(
	SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);

    return somThis->cimtabs->mtab.className;
}

SOM_Scope boolean  SOMLINK somcls_somGetNthMethodData(
		SOMClass SOMSTAR somSelf, 
		long n, 
		somMethodData* md)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);

	if (((unsigned long)n) < somThis->cimtabs->classInfo.jump_table._length)
	{
		somMToken m=SOMKERN_index_to_somMToken(&somThis->cimtabs->mtab,n);

		md->id=(somId)(m->defined.somId_methodId);
		md->type=m->defined.type;
		md->descriptor=(somId)(m->defined.somId_methodDescriptor);
		md->mToken=m;
		md->method=somThis->cimtabs->mtab.entries[n];
		md->shared=(-1 == (long)m->defined.redispatchStub) ? 
					(void*)m->defined.applyStub : NULL;

		return 1;				
	}

    return 0;
}

SOM_Scope somId  SOMLINK somcls_somGetNthMethodInfo(
		SOMClass SOMSTAR somSelf, 
		long n, 
		somId* descriptor)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	if (((unsigned long)n) < somThis->cimtabs->classInfo.jump_table._length)
	{
		somMToken m=SOMKERN_index_to_somMToken(&somThis->cimtabs->mtab,n);

		if (descriptor)
		{
			(*descriptor)=(somId)(m->defined.somId_methodDescriptor);
		}

		return (somId)(m->defined.somId_methodId);
	}

    return 0;
}

SOM_Scope long  SOMLINK somcls_somGetNumMethods(
		SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	return (long)somThis->cimtabs->classInfo.jump_table._length;
}

SOM_Scope long  SOMLINK somcls_somGetNumStaticMethods(
		SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	return (long)somThis->cimtabs->classInfo.jump_table._length;
}

#ifdef SOMClass_somGetParent
SOM_Scope SOMClass SOMSTAR  SOMLINK somcls_somGetParent(
		SOMClass SOMSTAR somSelf)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

	somPrintf("SOMClass::somGetParent not implemented, use somGetParents\n");

    return 0;
}
#endif

SOM_Scope SOMClass_SOMClassSequence SOMLINK somcls_somGetParents(
	SOMClass SOMSTAR somSelf)
{
	SOMClass_SOMClassSequence ret={0,0,NULL};
    SOMClassData *somThis=SOMClassGetData(somSelf);
	somClassInfo classInfo=&somThis->cimtabs->classInfo;

	ret._length=classInfo->numParents;
	ret._maximum=classInfo->numParents;

	if (ret._length)
	{
		somMethodTabs tabs=classInfo->parents.next;
		unsigned int i=0;

		ret._buffer=SOMMalloc(sizeof(SOMClass *)*ret._length);

		while (i < ret._length)
		{
			SOMClass SOMSTAR cls=tabs->mtab->classObject;
#ifdef SOMClass_somDuplicateReference
			cls=SOMClass_somDuplicateReference(cls);
#endif
			ret._buffer[i]=cls;

			tabs=tabs->next;

			i++;
		}
	}

    return ret;
}

SOM_Scope somMethodTabs  SOMLINK somcls_somGetPClsMtab(
		SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

    return (somMethodTabs)(void *)&somThis->cimtabs->classInfo.parents;
}

#ifdef SOMClass_somGetPClsMtabs
SOM_Scope somMethodTabs  SOMLINK somcls_somGetPClsMtabs(
		SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

    return (somMethodTabs)&somThis->cimtabs->classInfo.parents;
}
#endif

SOM_Scope void  SOMLINK somcls_somGetVersionNumbers(
		SOMClass SOMSTAR somSelf, 
		long* majorVersion, 
		long* minorVersion)
{
    SOMClassData *somThis=SOMClassGetData(somSelf);

	*majorVersion=somThis->cimtabs->classInfo.major_version;
	*minorVersion=somThis->cimtabs->classInfo.minor_version;
}

#ifdef SOMClass_somSetMethodDescriptor
SOM_Scope boolean  SOMLINK somcls_somSetMethodDescriptor(
	SOMClass SOMSTAR somSelf, 
	somId methodId, 
	somId descriptor)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	if (somThis) 
	{
		somPrintf("SOMClass::somSetMethodDescriptor not implemented\n");
	}

    return 0;
}
#endif

SOM_Scope boolean  SOMLINK somcls_somFindMethod(
	SOMClass SOMSTAR somSelf, 
	somId methodId, 
	somMethodPtr* m)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somMToken token=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (token)
	{
		*m=SOMKERN_resolve(&somThis->cimtabs->mtab,token);
		return 1;
	}

	*m=0;

    return 0;
}

SOM_Scope boolean  SOMLINK somcls_somFindMethodOk(
	SOMClass SOMSTAR somSelf, 
	somId methodId, 
	somMethodPtr* m)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somMToken token=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (token)
	{
		*m=SOMKERN_resolve(&somThis->cimtabs->mtab,token);
		return 1;
	}

	*m=NULL;

	somPrintf("SOMClass '%s' does not support method '%s'\n",
			somThis->cimtabs->mtab.className,
			somStringFromId(methodId));

    return 0;
}

SOM_Scope somMethodPtr  SOMLINK somcls_somFindSMethod(
		SOMClass SOMSTAR somSelf, 
        somId methodId)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somMToken token=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (token)
	{
		return SOMKERN_resolve(&somThis->cimtabs->mtab,token);
	}

    return 0;
}

SOM_Scope somMethodPtr  SOMLINK somcls_somFindSMethodOk(
		SOMClass SOMSTAR somSelf, 
		somId methodId)
{
	somMethodPtr ptr;

	ptr=SOMClass_somFindSMethod(somSelf,methodId);

	if (!ptr)
	{
		somPrintf("SOMClass '%s' does not support method '%s'\n",
			SOMClass_somGetName(somSelf),
			somStringFromId(methodId));
	}

    return ptr;
}

SOM_Scope somMethodPtr  SOMLINK somcls_somLookupMethod(
		SOMClass SOMSTAR somSelf, 
        somId methodId)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somMToken token=SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,methodId);

	if (token)
	{
		return SOMKERN_resolve(&somThis->cimtabs->mtab,token);
	}

	return 0;
}

SOM_Scope boolean  SOMLINK somcls_somCheckVersion(
		SOMClass SOMSTAR somSelf, 
		long majorVersion, 
		long minorVersion)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(majorVersion);
	SOM_IgnoreWarning(minorVersion);

    return 1;
}

SOM_Scope boolean  SOMLINK somcls_somDescendedFrom(
		SOMClass SOMSTAR somSelf, 
        SOMClass SOMSTAR aClassObj)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	somClassInfo info=&somThis->cimtabs->classInfo;
	unsigned int i=info->classes._length;

	while (i--)
	{
		if (aClassObj==info->classes._buffer[i].cls->classObject)
		{
			return 1;
		}
	}

    return 0;
}

SOM_Scope boolean  SOMLINK somcls_somSupportsMethod(
		SOMClass SOMSTAR somSelf, 
		somId mId)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);
	if (SOMKERN_somMToken_by_name(&somThis->cimtabs->classInfo,mId))
	{
		return 1;
	}

    return 0;
}

SOM_Scope somMethodPtr  SOMLINK somcls_somDefinedMethod(
		SOMClass SOMSTAR somSelf, 
        somMToken method)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	if (method)
	{
		if (SOMClass_somDescendedFrom(
			somSelf,
			method->classInfoOwner->classObject))
		{
			return SOMKERN_resolve(&somThis->cimtabs->mtab,method);
		}
	}

    return NULL;
}

SOM_Scope void  SOMLINK somcls_somOverrideMtab(
		SOMClass SOMSTAR somSelf)
{
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

	SOM_IgnoreWarning(somSelf);

	/* this should replace all methods except somDispatch with redirect stubs */

	somPrintf("SOMClass::somOverrideMtab not implemented\n");
}

/*SOMEXTERN void SOMClass__set_classMtab(SOMClass SOMSTAR somSelf,somMethodTabPtr m)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	somThis->mtabs=m;
}*/

SOM_Scope void SOMLINK somcls_somUninit(SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis=SOMClassGetData(somSelf); /* set in BeginDestructor */

	somStartCriticalSection();

	if (somThis->cimtabs)
	{
		SOMClass_unregistered(somSelf,&somThis->cimtabs->mtab);
	}

	somEndCriticalSection();

    SOMClass_parent_SOMObject_somUninit(somSelf);
}


static void SOMClass_unregistered(
		SOMClass SOMSTAR somSelf,
		somMethodTabPtr mtabs)
{
    SOMClassData *somThis=SOMClassGetData(somSelf); /* set in BeginDestructor */

	if (mtabs==&somThis->cimtabs->mtab)
	{
		unsigned int i=somThis->cimtabs->classInfo.numParents;;
		
/*		somPrintf("Destroying class %s\n",somThis->mtabs->className);*/

		if (somThis->cimtabs->classInfo.sci)
		{
			if (somThis->cimtabs->classInfo.sci->cds)
			{
				somThis->cimtabs->classInfo.sci->cds->classObject=NULL;
			}
		}

		/* need to unregister the parent classes
			and dissassociate any substitutions */

		

		if (i)
		{
			somMethodTabs tabs=somThis->cimtabs->classInfo.parents.next;

			while (i--)
			{
				somClassInfo parent_ci=somClassInfoFromMtab(tabs->mtab);

				if (parent_ci->substituted_mtab==mtabs)
				{
					struct somParentClassInfo *pci=somThis->cimtabs->classInfo.classes._buffer;
					somClassDataStructure *cds=NULL;

					while (pci)
					{
						if (pci->cls==tabs->mtab)
						{
							break;
						}

						pci++;
					}

					RHBOPT_ASSERT(pci->cls==somMtabFromClassInfo(parent_ci));

					/* restore previous values prior to substitution */

					parent_ci->substituted_mtab=pci->cls;

					cds=SOMClass_somGetClassData(pci->cls->classObject);

					if (cds)
					{
						RHBOPT_ASSERT(cds->classObject==somSelf);

						cds->classObject=pci->cls->classObject;
					}
				}

#ifdef SOMClass_somRelease
				SOMClass SOMSTAR cls;
				cls=tabs->mtab->classObject;
				SOMClass_somRelease(cls);
#endif
				tabs=tabs->next;
			}
		}

		if (somSelf->mtab != &somThis->cimtabs->mtab)
		{
			SOMKERN_clear_somMethodTabPtr(&somThis->cimtabs->mtab);
			SOMFreeEx(somThis->cimtabs);
		}

		somThis->cimtabs=NULL;
	}
}

/*
SOM_Scope somMethodProc * SOMLINK somcls__get_somClassDeallocate(SOMClass *somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	return somThis->somClassDeallocate;
}*/

/*
SOM_Scope somMethodProc * SOMLINK somcls__get_somClassAllocate(SOMClass *somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	return somThis->somClassAllocate;
}*/

SOM_Scope SOMClass_SOMClassSequence 
		SOMLINK somcls__get_somDirectInitClasses(
		SOMClass SOMSTAR somSelf)
{
	SOMClass_SOMClassSequence seq;
/*    SOMClassData *somThis = SOMClassGetData(somSelf);*/

	SOM_IgnoreWarning(somSelf);

	seq._length=0;
	seq._maximum=0;
	seq._buffer=0;
	return seq;
}

SOM_Scope long SOMLINK somcls__get_somDataAlignment(
	SOMClass SOMSTAR somSelf)
{
    SOMClassData *somThis = SOMClassGetData(somSelf);

	return somThis->cimtabs->mtab.dataAlignment;
}

#ifdef SOMClass_somMethodImplOwner
SOM_Scope SOMClass SOMSTAR SOMLINK somcls_somMethodImplOwner(
	SOMClass SOMSTAR somSelf,
	/* inout */ somMethodData *md)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(md);

	return NULL;
}
#endif

SOM_Scope somMethodPtr SOMLINK somcls_somGetApplyStub(
	SOMClass SOMSTAR somSelf,
	/* in */ somId methodId)
{
	somMethodData md;
	if (SOMClass_somGetMethodData(somSelf,methodId,&md))
	{
		if (-1L == (long)(md.mToken->defined.redispatchStub))
		{
			somApRdInfo *info=(void *)md.mToken->defined.applyStub;

			if (info)
			{
				if (info->apStub)
				{
					return info->apStub;						
				}

#ifdef SOM_METHOD_STUBS
				return (somMethodPtr)(&(md.mToken->jumper.apply));
#endif
			}
		}
		else
		{
			return md.mToken->defined.applyStub;
		}
	}

	return 0;
}

#ifdef SOMClass_somGetMarshalPlan
SOM_Scope somToken SOMLINK somcls_somGetMarshalPlan(
	SOMClass SOMSTAR somSelf,
	/* in */ somId methodId)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(methodId);

	return NULL;
}
#endif

#ifdef SOMClass_somEndow
SOM_Scope SOMClass SOMSTAR SOMLINK somcls_somEndow(
	SOMClass SOMSTAR somSelf,
	/* in */ SOMClass SOMSTAR parent,
	/* in */ string nameOfNewClass)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(parent);
	SOM_IgnoreWarning(nameOfNewClass);

	return NULL;
}
#endif

#ifdef SOMClass_somJoin
SOM_Scope SOMClass SOMSTAR SOMLINK somcls_somJoin(
	SOMClass SOMSTAR somSelf,
	/* in */ SOMClass SOMSTAR secondParent,
	/* in */ string nameOfNewClass)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(secondParent);
	SOM_IgnoreWarning(nameOfNewClass);

	return NULL;
}
#endif

static void somcm_merge(SOMClass_SOMClassSequence *seq,SOMClass SOMSTAR cls)
{
	if (cls)
	{
		unsigned int i=seq->_length;

		while (i--)
		{
			if (SOMClass_somDescendedFrom(seq->_buffer[i],cls)) return;
		}

		i=0;

		while (i < seq->_length)
		{
			if (SOMClass_somDescendedFrom(cls,seq->_buffer[i]))
			{
				seq->_buffer[i]=cls;

				return;
			}

			i++;
		}

		seq->_buffer[seq->_length++]=cls;
	}
}

struct somcls_somClassOfNewClassWithParents
{
	char *className;
	SOMClass SOMSTAR classObject;
};

RHBOPT_cleanup_begin(somcls_somClassOfNewClassWithParents_cleanup,pv)

	struct somcls_somClassOfNewClassWithParents *data=pv;
	if (data->className) SOMFree(data->className);
	if (data->classObject) SOMClass_somFree(data->classObject);

RHBOPT_cleanup_end

SOM_Scope SOMClass SOMSTAR SOMLINK somcls_somClassOfNewClassWithParents(
	/* in */ string newClassName,
	/* in */ SOMClass_SOMClassSequence *parents,
	/* in */ SOMClass SOMSTAR explicitMeta)
{
	if (explicitMeta)
	{
		boolean failed=0;

		if (parents)
		{
			if (parents->_length)
			{
				unsigned long i=parents->_length;

				while (i--)
				{
					if (!SOMClass_somDescendedFrom(explicitMeta,SOMObject_somGetClass(parents->_buffer[i])))
					{
						failed=1;

						break;
					}
				}
			}
		}

		if (!failed) return explicitMeta;
	}

	{
		SOMClass SOMSTAR _buffer[32];
		SOMClass_SOMClassSequence seq={sizeof(_buffer)/sizeof(_buffer[0]),0,NULL};
		size_t i=0;
		SOMClass SOMSTAR RHBOPT_volatile cls=NULL;

		seq._buffer=_buffer;

		if (explicitMeta) 
		{
			somcm_merge(&seq,explicitMeta);
		}

		while (i < parents->_length)
		{
			somcm_merge(&seq,SOMObject_somGetClass(parents->_buffer[i]));
			i++;
		}

		/* need at least one class object in there */

		somcm_merge(&seq,SOMObject_somGetClass(SOMClassClassData.classObject));

		switch (seq._length)
		{
		case 0:
			break;
		case 1:
			return seq._buffer[0];
		default:
			if (newClassName)
			{
				struct somcls_somClassOfNewClassWithParents data={NULL,NULL};
#define dynamic_prefix_length     2
#define dynamic_suffix_length     8
static const char dynamic_prefix[dynamic_prefix_length]="M_"; 
static const char dynamic_suffix[dynamic_suffix_length]="_Derived";

				RHBOPT_cleanup_push(somcls_somClassOfNewClassWithParents_cleanup,&data);

				i=strlen(newClassName);

				data.className=SOMMalloc(i+1+dynamic_prefix_length+dynamic_suffix_length);

				memcpy(data.className,dynamic_prefix,dynamic_prefix_length);
				memcpy(data.className+dynamic_prefix_length,newClassName,i);
				memcpy(data.className+dynamic_prefix_length+i,dynamic_suffix,dynamic_suffix_length);
				data.className[dynamic_prefix_length+i+dynamic_suffix_length]=0;

				data.classObject=cls=SOMClassNew();

				if (cls)
				{
					SOMClass_somInitMIClass(cls,
							(1 << seq._length)-1,
							data.className,
							&seq,
							0,
							0,
							0,
							SOMClass_MajorVersion,SOMClass_MinorVersion);

					SOMClass_somClassReady(cls);

					data.classObject=NULL;
				}

				RHBOPT_cleanup_pop();

				return cls;
			}
		}
	}

	return NULL;
}

SOM_Scope void SOMLINK somcls_somFree(SOMClass SOMSTAR somSelf)
{
	SOMClassData *somThis=SOMClassGetData(somSelf);
	if (somThis->cimtabs)
	{
		somPrintf("%s:%d, attempt to delete active class object \042%s\042\n",
				__FILE__,__LINE__,somThis->cimtabs->mtab.className);
		SOMError(-1,__FILE__,__LINE__);
	}
	else
	{
		SOMClass_parent_SOMObject_somFree(somSelf);
	}
}


#ifdef SOMClass__get_somClassAllocate
SOM_Scope somMethodPtr SOMLINK somcls__get_somClassAllocate(SOMClass SOMSTAR somSelf)
{
	return NULL;
}
#endif

#ifdef SOMClass__get_somClassDeallocate
SOM_Scope somMethodPtr SOMLINK somcls__get_somClassDeallocate(SOMClass SOMSTAR somSelf)
{
	return NULL;
}
#endif

#ifdef SOMClass__set_somClassAllocate
SOM_Scope void SOMLINK somcls__set_somClassAllocate(SOMClass SOMSTAR somSelf,somMethodPtr ptr)
{
}
#endif

#ifdef SOMClass__set_somClassDeallocate
SOM_Scope void SOMLINK somcls__set_somClassDeallocate(SOMClass SOMSTAR somSelf,somMethodPtr ptr)
{
}
#endif

SOM_Scope void SOMLINK somcls_somDefaultInit(SOMClass SOMSTAR somSelf,
											 somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMClassData *somThis;

	/* should not have to call SOMObject */

	SOMClass_BeginInitializer_somDefaultInit

	(void)somThis;

	RHBOPT_ASSERT(!(myMask[0] & 1));

/*	ctrl->info=(void *)(((char *)ctrl->info)+ctrl->infoSize);
	ctrl->mask++;*/

	SOMClass_Init_SOMObject_somDefaultInit(somSelf,ctrl);
}

SOM_Scope void SOMLINK somcls_somDestruct(SOMClass SOMSTAR somSelf,
										 boolean doFree,
										 somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	SOMClassData *somThis;
	somBooleanVector myMask;

	/* should not have to call SOMObject */

	RHBOPT_ASSERT(!(ctrl->mask[0] & 1));

	SOMClass_BeginDestructor;

	(void)somThis;

	RHBOPT_ASSERT(somThis==SOMClassGetData(somSelf));

	somcls_somUninit(somSelf);

	SOMClass_EndDestructor;
}

static void SOMKERN_init_dts_info(somClassInfo newClass,octet magic,void *ctrl,long maxCtrlMask)
{
	somDestructCtrl globalCtrl,*pinfo=ctrl;
	somDestructInfo *p=pinfo->info;
	struct dts_init_info dts_init_info={0};

	globalCtrl.info=pinfo->info;
	globalCtrl.infoSize=pinfo->infoSize;
	globalCtrl.mask=pinfo->mask;

	/* determine order of construction */

	if (magic)
	{
		SOMKERN_add_constructors(&dts_init_info,newClass,newClass,magic,&globalCtrl,ctrl,maxCtrlMask);
	}
	else
	{
		/* change to destructors */

		/* need to recurse through classes getting the
			parent classes and their usages */

		SOMKERN_add_destructor_usage(newClass,newClass);

		/* then go back through the recurse a second time
			returning true and adding to the list only
			when the usage count is dropped to zero */

		SOMKERN_release_destructor(&dts_init_info,newClass,&globalCtrl,newClass,1,maxCtrlMask);
	}

#ifdef _DEBUG
	{
		unsigned long classCount;
		classCount=(long)(((char *)globalCtrl.info)-((char *)pinfo->info));
		classCount/=pinfo->infoSize;

/*		somPrintf("%d info records added, classes._length=%d\n",
				classCount,
				newClass->classes._length);*/

		RHBOPT_ASSERT(dts_init_info.dts_count==classCount);

		RHBOPT_ASSERT(classCount<=newClass->classes._length);
	}
#endif
	/* get data offset for that class */

	while (dts_init_info.dts_count--)
	{
		SOMClass SOMSTAR clsPart=p->cls;
		unsigned long i=newClass->classes._length;

		RHBOPT_ASSERT(clsPart);

		while (i--)
		{
			struct somMethodTabStruct * cls=newClass->classes._buffer[i].cls;

			if (cls->classObject==clsPart)
			{
				long d=newClass->classes._buffer[i].data_offset;

				/* should only have functions for classes that
					are actually present in the class
					and not just inherited the interface
				*/

				RHBOPT_ASSERT(newClass->classes._buffer[i].inherit_var);

				switch (magic)
				{
				case kDestructCtrl:
					RHBOPT_ASSERT(p->defaultDestruct)
					p->dataOffset=d;
					break;
				case kInitCtrl:
					RHBOPT_ASSERT(((somInitInfo *)p)->defaultInit)
					((somInitInfo *)p)->dataOffset=d;
/*					somPrintf("%d,%p,kInitCtrl,defaultInit=%p for %s\n",
							i,
							p,
							(((somInitInfo *)p)->defaultInit),
							SOMClass_somGetName(p->cls));*/
					break;
				case kAssignCtrl:
					RHBOPT_ASSERT(((somAssignInfo *)p)->defaultAssign)
					((somAssignInfo *)p)->dataOffset=d;
					break;
				}

				break;
			}
		}

		RHBOPT_ASSERT(i>=0);

		p=(void *)(((char *)p)+pinfo->infoSize);
	}
}

SOM_Scope void  SOMLINK somcls_somInitMIClass(
	 SOMClass SOMSTAR somSelf, 
     unsigned long inherit_vars, 
     string className, 
     SOMClass_SOMClassSequence* parentClasses, 
     long dataSize, 
     long dataAlignment, 
     long maxStaticMethods, 
     long majorVersion, 
     long minorVersion)
{
	somMethodTabPtr tabs[sizeof(inherit_vars)<<3];
	unsigned int i=0;
	unsigned long m,c;
	boolean n=1;
	long maxCtrlMask;
    SOMClassData *somThis=SOMClassGetData(somSelf);
	struct somParentClassInfo *parents=NULL;
	
	/* need to work out how many methods appear that are not in the other
		classes */

	while (i < parentClasses->_length)
	{
		SOMClass SOMSTAR cls=parentClasses->_buffer[i];

#ifdef SOMClass_somDuplicateReference
		cls=SOMClass_somDuplicateReference(cls);
#endif

		tabs[i++]=SOMClass_somGetClassMtab(cls);
	}

	m=SOMKERN_count_unique_methods(tabs,i);

	c=SOMKERN_count_unique_classes(tabs,i);

	maxCtrlMask=1+SOMKERN_count_maxCtrlMask(tabs,i);

/*	somPrintf("maxCtrlMask allocated as %d\n",maxCtrlMask);
*/
	if (i==1)
	{
		if (!SOMKERN_any_mi(tabs[0]))
		{
			n=0;
		}
	}

	somThis->cimtabs=SOMKERN_allocate_somMethodTab(
			className,
			n,
			parentClasses->_length,
			c,
			m+maxStaticMethods,maxStaticMethods,
			0,
			majorVersion,
			minorVersion,
			dataSize,
			dataAlignment,
			maxCtrlMask);

	somThis->cimtabs->mtab.classObject=somSelf;

	i=0;

	while (i < parentClasses->_length)
	{
		SOMKERN_copy_somMethodTab(
			(boolean)(inherit_vars & 1),
			&somThis->cimtabs->mtab,tabs[i]);

		i++;
		inherit_vars>>=1;
		inherit_vars|=0x80000000;
	}

	somThis->cimtabs->classInfo.numParents=parentClasses->_length;

	SOMKERN_set_up_parents(&somThis->cimtabs->mtab,tabs,i);

	parents=SOMKERN_get_this_class(&somThis->cimtabs->mtab);

	SOMKERN_AllocateInstanceSize(somThis->cimtabs);

	SOMKERN_init_dts_info(&somThis->cimtabs->classInfo,kInitCtrl,
			&somThis->cimtabs->classInfo.parents.initCtrl,maxCtrlMask);
	SOMKERN_init_dts_info(&somThis->cimtabs->classInfo,kAssignCtrl,
			&somThis->cimtabs->classInfo.parents.assignCtrl,maxCtrlMask);
	SOMKERN_init_dts_info(&somThis->cimtabs->classInfo,kDestructCtrl,
			&somThis->cimtabs->classInfo.parents.destructCtrl,maxCtrlMask);

	SOMKERN_init_dts_restore_SOMObject(somThis->cimtabs);

	parents->jump_table_offset=(SOMKERN_mtoken_index_t)somThis->cimtabs->classInfo.jump_table._length;
}

