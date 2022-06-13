/**************************************************************************
 *
 *  Copyright 1994, Christina Lau
 *
 *  This file is part of Object-Orientated Programming using SOM and DSOM.
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

#include "planhdr.hpp"
#include <stdio.h>

Boolean PlannerHandler :: dispatchHandlerEvent(IEvent &evt)
{
   if (evt.eventId() == WM_REFRESH                ||
       evt.eventId() == WM_STAND_ALONE            ||
       evt.eventId() == WM_CONNECTION_LOST        ||
       evt.eventId() == WM_CONNECTION_REESTABLISH ||
       evt.eventId() == WM_LOST_RECOVER)
   {
      //************************************************
      // events that are of interest to DayPlannerWindow
      //************************************************
      return plannerMessage(evt);
   } 
   return false;
}

Boolean PlannerHandler :: plannerMessage(IEvent& evt)
{
   // Provides default return
   return false;
}
