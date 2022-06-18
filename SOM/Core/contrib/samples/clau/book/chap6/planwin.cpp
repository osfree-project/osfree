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

#include <ifont.hpp>
#include <ititle.hpp>
#include <iostream.h>       
#include <fstream.h>        
#include <imsgbox.hpp>   

#include <stdio.h>
#include "planwin.hpp"

void main(int argc, char *argv[], char *envp[])
{
  IString filename;
  if (argc == 2)
  {
     filename = argv[1];
  }
  else
  {
     filename = "pChris";   // default to my persistent calendar
  }
  DayPlannerWindow mainWindow(WND_MAIN, filename);
  IApplication::current().run(); 
}

DayPlannerWindow :: DayPlannerWindow(unsigned long windowId, 
                                     char *name)
  : IFrameWindow (IFrameWindow::defaultStyle(), windowId)
  , serverName(name)
  , createFile(0)
{
   IString title(" Weekly Calendar");

   ICommandHandler::handleEventsFor(this);

   title = serverName + title;
   ITitle(this, title);

   menuBar = new IMenuBar(WND_MAIN,this);
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
 
  ev = SOM_CreateLocalEnvironment();            
  SOMD_Init(ev);

  PersistentServerNewClass(0,0);
  SOMPPersistentStorageMgrNewClass(0,0);

  WorkItemNewClass(0,0);
  DayNewClass(0,0);
  CalendarDirNewClass(0,0);

  server = (PersistentServer *) SOMD_ObjectMgr->somdFindServerByName(ev,serverName);
  psm = (SOMPPersistentStorageMgr *) server->getPerStorMgr(ev);

  found = findProxy();

  if (!found) // no current client yet
  {
    found = findPersist();
  }

  if (!found)
  {
    //************************************************************
    // Create a new CalendarDir object and add 7 Days to the list.
    // Pass pidString as "hints".
    //************************************************************
    curDirEntry = (CalendarDir *) server->somdCreateObj(ev,
                                                        "CalendarDir",
                                                        pidString);
    externalizeProxy();
                                                
    for (i=0; i<7; i++ )
    {
       day = (Day *) server->somdCreateObj(ev, "Day", NULL);
       day->_set_date(ev,i);
       curDirEntry->addDay(ev, i, day);
    }
  }

  // Set current day to Monday and show any existing activities
  curDay = curDirEntry->getDay(ev,1);
  showItems(1);
}

DayPlannerWindow :: externalizeProxy()
{
  string objectId;      

  objectId = SOMD_ObjectMgr->somdGetIdFromObject(ev,curDirEntry);
                                                                 
  ofstream  outfile(serverName);
  outfile << objectId;
  outfile.close();
 
  createFile = 1;     // remember this client created the file
}

short DayPlannerWindow :: findProxy()
{                                                            
  ifstream infile(serverName);
  string   objectId;
  char     buffer[256];
  short    found = 0;
                                                             
  if (infile)    // proxy exist
  {                                                          
    //***************************************
    // restore proxy from its string form
    //***************************************
    objectId = (string) buffer;                              
    infile >> objectId;          
    curDirEntry = (CalendarDir *) SOMD_ObjectMgr->somdGetObjectFromId(ev, 
                                                 objectId);           
    return 1;
  }
  return 0;
}

short DayPlannerWindow :: findPersist()
{
  //*************************************************************
  // Create string for persistent ID. We will store the 
  // persistent objects in a file using the following convention:
  //   SOMPAscii:.\\<serverName>.dat:0
  //*************************************************************
  pidString = new char[strlen(serverName) +
                       strlen("SOMPAscii:.\\.dat:0") + 1];
  sprintf(pidString, "SOMPAscii:.\\%s.dat:0", serverName);

  if ( server->objectExists(ev, pidString) )
  {
    curDirEntry = (CalendarDir *) server->restoreObjectFromId(ev,
                                                              pidString);
    externalizeProxy();
    return 1;
  }
  else
  {
    return 0;
  }
}

DayPlannerWindow :: ~DayPlannerWindow()
{
  short i,j;
  _IDL_SEQUENCE_WorkItem alist;

  // Store all objects
  psm->sompStoreObject(ev, curDirEntry);
 
  if (createFile)     // this client writes proxy to file
  {
     //**************************************
     // perform clean up: delete the file
     //**************************************
     IString buffer("erase ");
     buffer = buffer + serverName;
     system(buffer);

     //**************************************                                 
     // Destroy each Day object
     //**************************************
     for (i=0; i<7; i++ )
     {
       curDay = curDirEntry->getDay(ev,i);
       alist = curDay->_get_workList(ev);
       if (sequenceLength(alist) > 0 && sequenceLength(alist) < 50)
       {
         //*********************************************                    
         // Destroy each WorkItem from the Day object                       
         //*********************************************                    
         for (j=0; j < sequenceLength(alist) ; j++)                         
         {                                                                  
            SOMD_ObjectMgr->somdDestroyObject(ev,                           
                                              sequenceElement(alist,j));    
         }                                                                  
       }
       SOMD_ObjectMgr->somdDestroyObject(ev,curDay);
     }
                                                                            
     // Destroy CalendarDir object
     SOMD_ObjectMgr->somdDestroyObject(ev, curDirEntry);
  }
  else
  {
    //**************************************
    // Release the proxy for each Day object
    //**************************************
    for (i=0; i<7; i++ )
    {
      curDay = curDirEntry->getDay(ev,i);                                   
      alist = curDay->_get_workList(ev);                                    
      if (sequenceLength(alist) > 0 && sequenceLength(alist) < 50)          
      {                                                                     
         //**************************************************               
         // Release proxy for each WorkItem from Day object                 
         //*****************************************************            
         for (j=0; j < sequenceLength(alist) ; j++)                         
         {                                                                  
           SOMD_ObjectMgr->somdReleaseObject(ev,                            
                                             sequenceElement(alist,j));     
         }                                                                  
      }
      SOMD_ObjectMgr->somdReleaseObject(ev,curDay);
    }

    // Release CalendarDir proxy
    SOMD_ObjectMgr->somdReleaseObject(ev,curDirEntry);
  }
                                                                            
  // release server proxy
  SOMD_ObjectMgr->somdReleaseObject(ev, server);
                                                                            
  SOMD_Uninit(ev);
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
        curDay->removeItem(ev, startTime, endTime, task);

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

Boolean DayPlannerWindow :: selected(IControlEvent & evt)
{
  cursor->setToFirst();
  return true;
}

DayPlannerWindow :: showItems(short day)
{
  _IDL_SEQUENCE_WorkItem alist;
  short                  i;
  IString                str;
  long                   date;

  listBox->removeAll();             // clear list
  
  menuBar->uncheckItem(menuFlag);   // uncheck previous day
  menuBar->checkItem(MI_BASE+day);  // check selected day
  menuFlag = MI_BASE + day;

  curDay = curDirEntry->getDay(ev,day);
  alist = curDay->_get_workList(ev);

  if (sequenceLength(alist) > 0 && sequenceLength(alist) < 50) 
  {
    for (i=0; i < sequenceLength(alist) ; i++)
    {
      str = "";
      str = str +
            sequenceElement(alist,i)->_get_startTime(ev) +
            sequenceElement(alist,i)->_get_endTime(ev) +
            sequenceElement(alist,i)->_get_task(ev);

      listBox->addAscending(str);
    }
  }
}

DayPlannerWindow :: book(char *start, char * end, char *task)
{
   WorkItem *item;

   item = (WorkItem *) server->somdCreateObj(ev,
                                             "WorkItem",
                                             NULL);
   item->mkEntry(ev, start, end, task);
   curDay->book(ev, item);
}
