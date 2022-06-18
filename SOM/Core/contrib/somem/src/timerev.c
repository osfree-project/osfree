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
#include <rhbemani.h>

#ifndef SOM_Module_timerev_Source
#define SOM_Module_timerev_Source
#endif
#define SOMETimerEvent_Class_Source

#include "timerev.ih"

SOM_Scope long  SOMLINK timerev_somevGetEventInterval(
		SOMETimerEvent SOMSTAR somSelf, 
		Environment *ev)
{
    SOMETimerEventData *somThis = SOMETimerEventGetData(somSelf);

    return somThis->timer_interval;
}

SOM_Scope void  SOMLINK timerev_somevSetEventInterval(
		SOMETimerEvent SOMSTAR somSelf, 
		Environment *ev, 
		long interval)
{
    SOMETimerEventData *somThis = SOMETimerEventGetData(somSelf);

	somThis->timer_interval=interval;
}

SOM_Scope void  SOMLINK timerev_somInit(
		SOMETimerEvent SOMSTAR somSelf)
{
    SOMETimerEventData *somThis = SOMETimerEventGetData(somSelf);

	somThis->timer_interval=0;

    SOMETimerEvent_parent_SOMEEvent_somInit(somSelf);
}


SOM_Scope void SOMLINK timerev_somDumpSelfInt(SOMETimerEvent SOMSTAR somSelf,long level)
{
    SOMETimerEventData *somThis = SOMETimerEventGetData(somSelf);
	somPrefixLevel(level);
	somPrintf("timer_interval : %ld\n",(long)somThis->timer_interval);
}

