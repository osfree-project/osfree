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

#include <rhbopt.h>
/* code generater for sombacls.idl */
#include <som.h>

SOM_SEQUENCE_TYPEDEF(somMethodData);
typedef somMethodData somMethodDataStruct;
SOM_SEQUENCE_TYPEDEF(somMethodDataStruct);

#define SOMMBeforeAfter_VA_EXTERN
#define SOM_Module_sombacls_Source
#define SOMMBeforeAfter_Class_Source
#include <sombacls.ih>

SOM_Scope boolean SOMLINK SOMMBeforeAfter_dispatcher(
			SOMObject SOMSTAR somSelf,
			somToken *token,
			somId id,
			va_list ap);

/* overridden methods for ::SOMMBeforeAfter */
/* overridden method ::SOMClass::somInitMIClass */
SOM_Scope void SOMLINK sombacls_somClassReady(SOMMBeforeAfter SOMSTAR somSelf)
{
	long num=SOMClass_somGetNumMethods(somSelf);
	SOMMBeforeAfterData *somThis=SOMMBeforeAfterGetData(somSelf);

	somThis->jump_table._length=num;
	somThis->jump_table._maximum=num;
	somThis->jump_table._buffer=SOMMalloc(sizeof(somThis->jump_table._buffer[0])*num);

	while (num--)
	{
		somMethodData *md=&somThis->jump_table._buffer[num];
		SOMClass_somGetNthMethodData(somSelf,num,md);

		if (strcmp(somStringFromId(md->descriptor),"::SOMObject::somDispatch"))
		{
			somMethodPtr rd=SOMClass_somGetRdStub(somSelf,md->id);
			SOMClass_somOverrideSMethod(somSelf,md->id,rd);
		}
		else
		{
			SOMClass_somOverrideSMethod(somSelf,md->id,(somMethodPtr)SOMMBeforeAfter_dispatcher);
		}
	}

	SOMMBeforeAfter_parent_SOMClass_somClassReady(somSelf);

	somPrintf("class %s is before-aftered\n",SOMClass_somGetName(somSelf));
}
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK sombacls_somInit(
	SOMMBeforeAfter SOMSTAR somSelf)
{
	SOMMBeforeAfter_parent_SOMClass_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK sombacls_somUninit(
	SOMMBeforeAfter SOMSTAR somSelf)
{
	SOMMBeforeAfterData *somThis=SOMMBeforeAfterGetData(somSelf);
	if (somThis->jump_table._buffer)
	{
		SOMFree(somThis->jump_table._buffer);
		somThis->jump_table._buffer=0;
	}
	SOMMBeforeAfter_parent_SOMClass_somUninit(somSelf);
}
/* introduced methods for ::SOMMBeforeAfter */
/* introduced method ::SOMMBeforeAfter::sommBeforeMethod */
SOM_Scope boolean SOMLINK sombacls_sommBeforeMethod(
	SOMMBeforeAfter SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR object,
	/* in */ somId methodId,
	/* in */ va_list ap)
{
	return 0;
}
/* introduced method ::SOMMBeforeAfter::sommAfterMethod */
SOM_Scope void SOMLINK sombacls_sommAfterMethod(
	SOMMBeforeAfter SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR object,
	/* in */ somId methodId,
	/* in */ somToken returnedvalue,
	/* in */ va_list ap)
{
}

SOM_Scope boolean SOMLINK SOMMBeforeAfter_dispatcher(
			SOMObject SOMSTAR somSelf,
			somToken *retVal,
			somId id,
			va_list ap)
{
	unsigned int i;
	SOMMBeforeAfterData *somThis=SOMMBeforeAfterGetData(somSelf->mtab->classObject);

	i=somThis->jump_table._length;

	while (i--)
	{
		if (somCompareIds(somThis->jump_table._buffer[i].id,id))
		{
			return somApply(somSelf,retVal,&somThis->jump_table._buffer[i],ap);
		}
	}

	return 0;
}

