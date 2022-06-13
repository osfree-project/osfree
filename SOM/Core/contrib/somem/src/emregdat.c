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

#define SOMEEMRegisterData_VA_STUBS
#define SOMEEMRegisterData_Class_Source

#include <rhbemani.h>
#include <emtypes.h>
#include <event.h>
#include <eventmsk.h>
#include <eman.h>
#ifndef SOM_Module_emregdat_Source
#define SOM_Module_emregdat_Source
#endif

#include <emregdat.ih>

#include <rhbeman.h>

SOM_Scope void  SOMLINK emregdat_someClearRegData(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	if (somThis->client_type) SOMFree(somThis->client_type);

	somThis->timer_interval=0;
	somThis->timer_count=0;
	somThis->client_type=0;
	somThis->event_mask=0;
	somThis->sink_mask=0;
	somThis->sink_fd=-1;
}

SOM_Scope void  SOMLINK emregdat_someSetRegDataClientType(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev, 
		corbastring clientType)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);
    SOMEEMRegisterDataMethodDebug("SOMEEMRegisterData","emregdat_someSetRegDataClientType");

	if (somThis->client_type)
	{
		SOMFree(somThis->client_type);
	}

	somThis->client_type=SOMEEMRegisterData_dupl_string(ev,clientType);
}

SOM_Scope void  SOMLINK emregdat_someSetRegDataEventMask(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev, 
		long eventType, 
		va_list ap)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	somThis->event_mask=eventType;
}

SOM_Scope void  SOMLINK emregdat_someSetRegDataSink(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev, 
		long sink)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	somThis->sink_fd=sink;
}

SOM_Scope void  SOMLINK emregdat_someSetRegDataSinkMask(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev, 
		unsigned long sinkMask)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	somThis->sink_mask=sinkMask;
}

SOM_Scope void  SOMLINK emregdat_someSetRegDataTimerCount(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev, 
		long count)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	somThis->timer_count=count;
}

SOM_Scope void  SOMLINK emregdat_someSetRegDataTimerInterval(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev, 
		long interval)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	somThis->timer_interval=interval;
}

SOM_Scope void  SOMLINK emregdat_somInit(
		SOMEEMRegisterData SOMSTAR somSelf)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	somThis->timer_interval=0;
	somThis->timer_count=0;
	somThis->client_type=0;
	somThis->event_mask=0;
	somThis->sink_mask=0;
	somThis->sink_fd=-1;

#ifdef SOMEEMRegisterData_parent_SOMRefObject_somInit
    SOMEEMRegisterData_parent_SOMRefObject_somInit(somSelf);
#else
    SOMEEMRegisterData_parent_SOMObject_somInit(somSelf);
#endif
}

SOM_Scope void  SOMLINK emregdat_somUninit(
		SOMEEMRegisterData SOMSTAR somSelf)
{
    SOMEEMRegisterDataData *somThis = SOMEEMRegisterDataGetData(somSelf);

	if (somThis->client_type)
	{
		SOMFree(somThis->client_type);
		somThis->client_type=0;
	}

#ifdef SOMEEMRegisterData_parent_SOMRefObject_somUninit
    SOMEEMRegisterData_parent_SOMRefObject_somUninit(somSelf);
#else
    SOMEEMRegisterData_parent_SOMObject_somUninit(somSelf);
#endif
}


SOM_Scope long SOMLINK emregdat_someGetRegDataTimerInterval(
					SOMEEMRegisterData SOMSTAR data,
					Environment *ev)
{
    SOMEEMRegisterDataData *somThis=
			SOMEEMRegisterDataGetData(data);

	return somThis->timer_interval;
}

SOM_Scope long SOMLINK emregdat_someGetRegDataTimerCount(
					SOMEEMRegisterData SOMSTAR data,
					Environment *ev)
{
    SOMEEMRegisterDataData *somThis=
			SOMEEMRegisterDataGetData(data);

	return somThis->timer_count;
}

SOM_Scope corbastring SOMLINK emregdat_someGetRegDataClientType(
					SOMEEMRegisterData SOMSTAR data,
					Environment *ev)
{
    SOMEEMRegisterDataData *somThis=
			SOMEEMRegisterDataGetData(data);

	return somThis->client_type;
}

SOM_Scope long SOMLINK emregdat_someGetRegDataEventMask(
					SOMEEMRegisterData SOMSTAR data,
					Environment *ev)
{
    SOMEEMRegisterDataData *somThis=
			SOMEEMRegisterDataGetData(data);

	return somThis->event_mask;
}

SOM_Scope long SOMLINK emregdat_someGetRegDataSink(
					SOMEEMRegisterData SOMSTAR data,
					Environment *ev)
{
    SOMEEMRegisterDataData *somThis=
			SOMEEMRegisterDataGetData(data);

	return somThis->sink_fd;
}

SOM_Scope unsigned long SOMLINK emregdat_someGetRegDataSinkMask(
					SOMEEMRegisterData SOMSTAR data,
					Environment *ev)
{
    SOMEEMRegisterDataData *somThis=
			SOMEEMRegisterDataGetData(data);

	return somThis->sink_mask;
}

#if 0
SOMEXTERN void  SOMLINK somva_SOMEEMRegisterData_someSetRegDataEventMask(
		SOMEEMRegisterData SOMSTAR somSelf, 
		Environment *ev,
		long eventType,
		...)
{
    va_list ap;
    va_start(ap, eventType);
    SOMEEMRegisterData_someSetRegDataEventMask(somSelf,ev,eventType,ap);
    va_end(ap);
}    
#endif

SOM_Scope char *SOMLINK emregdat_dupl_string(Environment *ev,char *p)
{
	char *q=NULL;

	if (p && ev && !ev->_major)
	{
		size_t len=strlen(p)+1;
		q=SOMMalloc(len);
		if (q) memcpy(q,p,len);
	}

	return q;
}

