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

#include <somd.xh>        
#include <somp.xh>        

#include "week.h"
#include "workitem.xh"       
#include "day.xh"            
#include "caldir.xh"        
#include "perserv.xh"

class DayPlannerWindow : public IFrameWindow,
                         public ICommandHandler,
                         public ISelectHandler
{
 public:
   DayPlannerWindow (unsigned long windowId, char *name);
   ~DayPlannerWindow();

 protected:
   virtual Boolean command(ICommandEvent& cmdevt);
   virtual Boolean selected(IControlEvent& evt);

 private:
   setupClient();
   setupData();                
   short findProxy();
   short findPersist();    
   externalizeProxy();
   showItems(short day);
   book(char *start, char *end, char *desc);

   IEntryField  *descT;
   IStaticText  *weekday, *start, *end, *desc;
   ISpinButton  *startT, *endT;

   IMultiCellCanvas *mc;
   IMenuBar         *menuBar;
   short            menuFlag;
   short            createFile;

   IListBox         *listBox;
   IListBox::Cursor *cursor;

   // DSOM related variables
   PersistentServer *server;            // use Persistent Server
   char             *serverName;
   Environment      *ev;
   CalendarDir      *curDirEntry;
   Day              *curDay;

   // PSOM related variables
   SOMPPersistentStorageMgr *psm;
   char                     *pidString;     // persistent ID string
}; 

#endif
