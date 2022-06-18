#ifndef PLANWIN_HPP
#define PLANWIN_HPP

#include <iframe.hpp>            
#include <icmdhdr.hpp>           
#include <imcelcv.hpp>
#include <ilistbox.hpp>
#include <iselhdr.hpp>
#include <istring.hpp>
#include <istattxt.hpp>
#include <imenubar.hpp>
#include <ispinbt.hpp>
#include <ientryfd.hpp>
#include <ithread.hpp>

#include <somr.xh>
#include <eman.xh>

#include "week.h"
#include "workitem.xh"       
#include "day.xh"            
#include "caldir.xh"  
       
#include "planhdr.hpp"

class DayPlannerWindow : public IFrameWindow,
                         public ICommandHandler,
                         public ISelectHandler,
                         public PlannerHandler
{
 public:
   DayPlannerWindow (unsigned long windowId);
   ~DayPlannerWindow();
   void checkEvents();

 protected:
   Boolean command(ICommandEvent& cmdevt);
   Boolean selected(IControlEvent& evt);
   Boolean plannerMessage(IEvent& evt);

 private:
   setupClient();
   setupData();
   showItems(short day);
   book(char *start, char *end, char *desc);
   remove(char *start, char *end, char *desc);
   refreshListBox();
   registerView(Day *day);

   IEntryField  *descT;
   IStaticText  *weekday, *start, *end, *desc;
   ISpinButton  *startT, *endT;

   IMultiCellCanvas *mc;
   IMenuBar         *menuBar;
   short            menuFlag;

   IListBox         *listBox;
   IListBox::Cursor *cursor;

   IThread     thread;
   SOMEEMan    *emgr;
   SOMR        *repobj;
   Environment *ev;
   CalendarDir *curDirEntry;
   Day         *curDay;
}; 

class eventMgrThread : public IThreadFn
{
   public:
     eventMgrThread(DayPlannerWindow &obj)
        : planObj(obj)
        {;}
 
     void run() { planObj.checkEvents(); }

   private:
     DayPlannerWindow &planObj;
};

#endif
