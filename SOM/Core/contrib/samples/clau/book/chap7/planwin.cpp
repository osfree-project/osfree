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

#define INCL_DOS                   
#define INCL_ERRORS                
#define INCL_DOSPROCESS            
#define INCL_DOSSEMAPHORES         
                                   
#include <os2.h>                   
#include <stdio.h>                 
#include <stdlib.h>                
#include <ctype.h>                 
#include <errno.h>                 
                                   
#include <eventmsk.h>              

#include <ifont.hpp>
#include <ititle.hpp>
#include <iostream.h>       
#include <fstream.h>        
#include <imsgbox.hpp>     

#include "planwin.hpp"
#include "dayview.xh"

HMTX  keyboard_sem;       

void main(int argc, char *argv[], char *envp[])
{
  DayPlannerWindow mainWindow(WND_MAIN);
  IApplication::current().run(); 
}

DayPlannerWindow :: DayPlannerWindow(unsigned long windowId)
  : IFrameWindow (IFrameWindow::defaultStyle(), windowId)
{
   IString title("Weekly Calendar");

   ICommandHandler::handleEventsFor(this);

   menuBar = new IMenuBar(WND_MAIN,this);
   menuBar->setAutoDeleteObject();
   menuBar->checkItem(MI_BASE);
   menuFlag = MI_BASE;

   setupClient();
   setupData();

   if ( IWindow::desktopWindow()->size() > ISize(1000,700) )
   {
      sizeTo(ISize(460,330));
   }
   else  // VGA
   {
      sizeTo(ISize(360,240));
   } 

   setFocus();

   // Enable PlannerHandler to handle user-defined events
   PlannerHandler::handleEventsFor(this);
   show();
}

DayPlannerWindow :: setupClient()
{
  mc = new IMultiCellCanvas(WND_CANVAS, this, this);
  mc->setAutoDeleteObject();
  setClient(mc);

  weekday = new IStaticText(WND_TEXT, mc, mc);
  weekday->setAutoDeleteObject();
  weekday->setText("Monday");

  start = new IStaticText(WND_TEXT, mc, mc);
  start->setAutoDeleteObject();
  start->setText("Start  ");

  startT = new ISpinButton(WND_START,mc,mc);
  startT->setAutoDeleteObject();
  startT->setInputType(ISpinButton::numeric);
  startT->setRange(IRange(1,24));
  startT->setCurrent(8);
  startT->setLimit(2);
  startT->readOnly;

  end = new IStaticText(WND_TEXT, mc, mc);
  end->setAutoDeleteObject();
  end->setText("End    ");

  endT = new ISpinButton(WND_END,mc,mc);
  endT->setAutoDeleteObject();
  endT->setInputType(ISpinButton::numeric);
  endT->setRange(IRange(1,24));
  endT->setCurrent(6);
  endT->setLimit(2);
  endT->readOnly;

  desc = new IStaticText(WND_TEXT, mc, mc);
  desc->setAutoDeleteObject();
  desc->setText("Description");

  descT = new IEntryField(WND_DESC, mc, mc);
  descT->setAutoDeleteObject();

  listBox = new IListBox(WND_LISTBOX,mc,mc,
                         IRectangle(),
                         IListBox::defaultStyle() |
                         IControl::tabStop);
  listBox->setAutoDeleteObject();

  cursor = new IListBox::Cursor(*listBox);
  ISelectHandler::handleEventsFor(listBox);

  mc->addToCell(weekday,2,2);
  mc->addToCell(start,  2,4);
  mc->addToCell(startT, 2,5);
  mc->addToCell(end,    4,4);
  mc->addToCell(endT,   4,5);
  mc->addToCell(desc,   6,4);
  mc->addToCell(descT,  6,5);
  mc->addToCell(listBox,2,7,5,1);

  mc->setRowHeight(3,2,true);
  mc->setRowHeight(6,2,true);
  mc->setRowHeight(8,2,true);
  mc->setColumnWidth(3,2,true);
  mc->setColumnWidth(5,2,true);
  mc->setColumnWidth(7,2,true);
}

DayPlannerWindow :: setupData()
{
  short i, found;
  Day   *day;
  char  *dayName[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  ULONG semattr;
  long  rc;

  semattr = DC_SEM_SHARED;                                                            
  rc = DosCreateMutexSem((PSZ) NULL, 
                         (PHMTX)&keyboard_sem, semattr, 0);
  if (rc != 0)
  {
    FILE *fp;
    fp = fopen("debug.dat", "a");
    fprintf(fp, "Error Allocating shared semaphore: rc = %d \n",rc);
    fclose(fp);
    exit(1);    
  }

  ev = SOM_CreateLocalEnvironment();            
  repobj = new SOMR();
  emgr = new SOMEEMan();

  curDirEntry = new CalendarDir();            
  for (i=0; i < 7 ; i++ )
  {
     day = new Day();                        
     day->_set_date(ev,i);
     day->initReplica(ev, dayName[i]);      // set replica name
     curDirEntry->addDay(ev,i,day);         
     registerView(day);                     // register view
  }

  //************************************************************
  // Set current day to Monday and show any existing activities
  //************************************************************
  curDay = curDirEntry->getDay(ev,1);
  showItems(1);

  //***************************************************
  // Start a new thread for SOM event manager
  //***************************************************
  eventMgrThread *emgrFn = new eventMgrThread( *this );

  //*************************************************** 
  // Dispatch thread to run function
  //***************************************************
  thread.start( emgrFn );
}

//************************************************* 
// Register a DayView object with the Day object
//*************************************************
DayPlannerWindow :: registerView(Day *day)
{
  DayView *dv;

  dv = new DayView;
  dv->_set_owner(ev, (void *)this);

  day->registerView(ev, dv, "notifyDayPlanner");
}

DayPlannerWindow :: ~DayPlannerWindow()
{
  short i,j;                    
  _IDL_SEQUENCE_WorkItem alist;

  for (i=0; i < 7; i++ ) 
  {
     curDay = curDirEntry->getDay(ev,i);
     alist = curDay->_get_workList(ev);

     //*********************************************
     // Destroy each WorkItem from the Day object
     //*********************************************
     for (j=0; j < sequenceLength(alist) ; j++)
     {
        delete sequenceElement(alist,j);
     }

     //********************************
     // Destroy DayView object
     //********************************
     delete (curDay->getViewObject(ev));

     delete curDay;
  }

  delete emgr;
  delete repobj;


  SOM_DestroyLocalEnvironment(ev);
}

Boolean DayPlannerWindow :: command(ICommandEvent & cmdEvent)
{                                                                           
  IMessageBox msgbox(this);       

  switch (cmdEvent.commandId()) 
  {
    case MI_ADD:
      if ( !(descT->text().size()) ) 
      {
         msgbox.show("Enter a description", IMessageBox::okButton);
      } 
      else
      {
        IString str;                            
        IString pad("0");
        IString trial(":00 ");
        IString blank(" ");
        IString sstr(startT->value());
        IString estr(endT->value());

        if ( startT->value() < 10 )
        {
          sstr = pad + sstr;
        }
        if ( endT->value() < 10 )
        {
          estr = pad + estr;
        }
 
        sstr = sstr + trial;
        estr = estr + trial;

        str = sstr + estr + descT->text();
        listBox->addAscending(str);

        book( sstr, estr, descT->text() );
      }
      return true;
      break;                          
      
    case MI_DEL:
      if ( cursor->isValid() )
      {
        IString item, startTime, endTime, task;

        item = listBox->elementAt(*cursor);

        startTime = item.subString(1,6);
        endTime   = item.subString(7,6);
        task      = item.subString(13);

        remove(startTime, endTime, task);

        listBox->removeAt(*cursor);
      }
      return true;
      break;
     
    case MI_SUN:
      weekday->setText("Sunday");
      showItems(0);
      return true;

    case MI_MON:
      weekday->setText("Monday");
      showItems(1);
      return true;

    case MI_TUE: 
      weekday->setText("Tuesday");
      showItems(2);
      return true;
                   
    case MI_WED:
      weekday->setText("Wednesday");
      showItems(3);
      return true;

    case MI_THU:
      weekday->setText("Thursday");
      showItems(4);
      return true;
                        
    case MI_FRI:
      weekday->setText("Friday");
      showItems(5);
      return true;
                        
    case MI_SAT:
      weekday->setText("Saturday");
      showItems(6);
      return true;

    case MI_QUIT:
      close();
      return true;
      break;
  }

  return false;
}

//****************************************************
// Handler for user-defined messages
//****************************************************
Boolean DayPlannerWindow :: plannerMessage(IEvent& evt)
{
  IMessageBox msgbox(this);      

  if (evt.eventId() == WM_REFRESH)
  {
        refreshListBox();
        return true;
  }
  if (evt.eventId() == WM_STAND_ALONE)
  {
        msgbox.show("stand alone", IMessageBox::okButton);
        return true;
  }
  if (evt.eventId() == WM_CONNECTION_LOST)
  {
        msgbox.show("Connection lost", IMessageBox::okButton);
        return true;
  }
  if (evt.eventId() == WM_CONNECTION_REESTABLISH)
  {
      msgbox.show("Connection reestablished", IMessageBox::okButton);
      return true;                                           
  }
  if (evt.eventId() == WM_LOST_RECOVER)
  {
      msgbox.show("Lost recoverability", IMessageBox::okButton);
      return true;
  }

  return false;
}

Boolean DayPlannerWindow :: selected(IControlEvent & evt)
{
  cursor->setToFirst();
  return true;
}

DayPlannerWindow :: showItems(short day)
{
  menuBar->uncheckItem(menuFlag);   // uncheck previous day
  menuBar->checkItem(MI_BASE+day);  // check selected day
  menuFlag = MI_BASE + day;

  curDay = curDirEntry->getDay(ev,day);
  refreshListBox();
}

DayPlannerWindow :: refreshListBox()
{
  _IDL_SEQUENCE_WorkItem alist;      
  short                  i;
  IString                str;

  alist = curDay->_get_workList(ev); 

  listBox->removeAll();

  for (i=0; i < sequenceLength(alist); i++)
  {
    str = "";
    str = str +
          sequenceElement(alist,i)->_get_startTime(ev) +
          sequenceElement(alist,i)->_get_endTime(ev) +
          sequenceElement(alist,i)->_get_task(ev);

    listBox->addAscending(str);
  }
}

DayPlannerWindow :: book(char *start, char * end, char *task)
{
   while  (DosRequestMutexSem(keyboard_sem, SEM_INDEFINITE_WAIT) != 0);
    
   curDay->book2(ev, start, end, task);

   DosReleaseMutexSem(keyboard_sem);   
}

DayPlannerWindow :: remove(char *start, char * end, char *task)
{                                                                        
   while  (DosRequestMutexSem(keyboard_sem, SEM_INDEFINITE_WAIT) != 0);  
                                                                         
   curDay->removeItem(ev, start, end, task);
                                                                         
   DosReleaseMutexSem(keyboard_sem);                                     
}                                                                        

//*********************************************************
// SOM Event Manager loop. It runs on its own thread
//*********************************************************
void DayPlannerWindow :: checkEvents()
{                                                               
  short         rc;                                             
  IMessageBox msgbox(this);                                     

  while (1)
  {
     // Wait for 500 milliseconds
     DosSleep(500);       
    
     // Wait on and take the mutual exclusion semaphore
     while  (DosRequestMutexSem(keyboard_sem, SEM_INDEFINITE_WAIT) != 0);

     // When control is obtained, process events
     emgr->someProcessEvent(ev, EMProcessTimerEvent | EMProcessSinkEvent);
                                                                           
     // Release the semaphore
     DosReleaseMutexSem(keyboard_sem);
  }
}
