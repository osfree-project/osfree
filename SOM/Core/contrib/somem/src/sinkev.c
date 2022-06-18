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

#ifndef SOM_Module_sinkev_Source
#define SOM_Module_sinkev_Source
#endif
#define SOMESinkEvent_Class_Source

#include "sinkev.ih"

SOM_Scope long  SOMLINK sinkev_somevGetEventSink(
		SOMESinkEvent SOMSTAR somSelf, 
		Environment *ev)
{
    SOMESinkEventData *somThis = SOMESinkEventGetData(somSelf);

    return somThis->sink_fd;
}

SOM_Scope void  SOMLINK sinkev_somevSetEventSink(
		SOMESinkEvent SOMSTAR somSelf, 
		Environment *ev, 
		long sink)
{
    SOMESinkEventData *somThis = SOMESinkEventGetData(somSelf);

	somThis->sink_fd=sink;
}

SOM_Scope void  SOMLINK sinkev_somInit(SOMESinkEvent SOMSTAR somSelf)
{
    SOMESinkEventData *somThis = SOMESinkEventGetData(somSelf);

	somThis->sink_fd=-1;

    SOMESinkEvent_parent_SOMEEvent_somInit(somSelf);
}

SOM_Scope void SOMLINK sinkev_somDumpSelfInt(SOMESinkEvent SOMSTAR somSelf,long level)
{
    SOMESinkEventData *somThis = SOMESinkEventGetData(somSelf);
	somPrefixLevel(level);
	somPrintf("sink_fd : %d\n",(int)somThis->sink_fd);
}
