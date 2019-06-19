/***********************************************************************

    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

***********************************************************************/
#ifndef WNDDATA_H
#define WNDDATA_H


/*********************************** GENERAL DEFINES ************************************/

/* 
** A handle to a global memory
** block is stored in the WNDEXTRABYTES
** of a window.  This block of memory
** is locked and used when window data is
** stored/retrieved.
*/
typedef HGLOBAL HWNDDATA;



/************************** CHILD WINDOW'S WINDOW DATA DEFINES *******************************/

/* 
** Allocate memory space for the child window's data. 
*/
BOOL __far __pascal AllocChildWndData( HWND hWnd );

/* 
** Free memory space for the child window's data. 
*/
void __far __pascal FreeChildWndData( HWND hWnd );



/************************** PARENT WINDOW'S WINDOW DATA DEFINES *******************************/

/* 
** For now the parent window, secondary window and child window 
** extrabytes alloc/free functions are the same. 
*/
#define AllocParentWndData  AllocChildWndData
#define FreeParentWndData   FreeChildWndData


/************************** MESSAGES TO ACCESS PARENT AND CHILD WINDOW'S **********************
***************************   WINDOW DATA USING THE WNDDATA() FUNCTION   *********************/

/* 
** Messages that can be sent in the to the WNDDATAMSG
** parameter of the WndData() function.
**
** Depending on the message, the WndData() function will
** set/retrieve window data stored in the WNDDATA
** structure. 
*/
typedef enum 
{
/************************** CHILD WINDOW'S WINDOW DATA MESSAGES *******************************/


  /**************************
  **
  **  Data: Position of window in client area.
  **
  **  Structure Element: ChildWndData.ClientPosRect
  **
  **  Messages:  
  **
  **    WDM_SETCLIENTPOS
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  RECT __far * - Position of window in client area.
  **      Returns:       NU        
  **  
  **    WDM_GETCLIENTPOS
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  RECT __far * - Position of window in client area.
  **      Returns:       NU       
  **  
  ***************************/
  WDM_SETCLIENTPOS,    
  WDM_GETCLIENTPOS,    
  

  /**************************
  **
  **  Data: Handle of the window's parent window. 
  **
  **  Structure Element: ChildWndData.hParentWnd
  **
  **  Messages:  
  **
  **    WDM_SETPARENTHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       BOOL - Handle of the window's parent window.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETPARENTHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       BOOL - Handle of the window's parent window.       
  **  
  ***************************/
  WDM_SETPARENTHWND,    
  WDM_GETPARENTHWND,    
  

  /**************************
  **
  **  Data: Handle to the non-scrollable topic window. 
  **
  **  Structure Element: ChildWndData.hNoScrollWnd
  **
  **  Messages:  
  **
  **    WDM_SETNOSCROLLHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       BOOL - Handle to the non-scrollable topic window.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETNOSCROLLHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       BOOL - Handle to the non-scrollable topic window.       
  **  
  ***************************/
  WDM_SETNOSCROLLHWND,    
  WDM_GETNOSCROLLHWND,    
  

  /**************************
  **
  **  Data: Am I painting the window currently. 
  **
  **  Structure Element: ChildWndData.bAmPainting
  **
  **  Messages:  
  **
  **    WDM_SETHOTSPOTPOS
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       BOOL - Am I painting the window currently. 
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETHOTSPOTPOS
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU 
  **      fpDataBuffer:  NU
  **      Returns:       BOOL - Am I painting the window currently.     
  **  
  ***************************/
  WDM_SETAMPAINTING,    
  WDM_GETAMPAINTING,    
  

  /**************************
  **
  **  Data: Main window's HWND
  **
  **  Structure Element: ChildWndData.HotSpotPosRec
  **
  **  Messages:  
  **
  **    WDM_SETHOTSPOTPOS
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  RECT __far * - Rectangle containg position.
  **      Returns:       NU        
  **  
  **    WDM_GETHOTSPOTPOS
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  RECT __far * - Rectangle containg position.
  **      Returns:       NU       
  **  
  ***************************/
  WDM_SETHOTSPOTPOS,    
  WDM_GETHOTSPOTPOS,    
  

  /**************************
  **
  **  Data: Handle to the window that keeps the data.
  **
  **  Structure Element: ChildWndData.hDataWnd
  **
  **  Messages:  
  **
  **    WDM_SETDATAHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HWND - Handle to the window that keeps the data.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETDATAHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HWND - Handle to the window that keeps the data.       
  **  
  ***************************/
  WDM_SETDATAHWND,    
  WDM_GETDATAHWND,    
  

  /**************************
  **
  **  Data: The display data for window. 
  **
  **  Structure Element: ChildWndData.hDisplayInfo
  **
  **  Messages:  
  **
  **    WDM_SETDISPLAYINFO
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HGLOBAL - The display data for window.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETDISPLAYINFO
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HGLOBAL - The display data for window.        
  **  
  ***************************/
  WDM_SETDISPLAYINFO,    
  WDM_GETDISPLAYINFO,    
  



/************************** PARENT WINDOW'S WINDOW DATA MESSAGES *******************************/

  /**************************
  **
  **  Data: Handle to accelerator support system.
  **
  **  Structure Element: MainWndData.hAccelSys
  **
  **  Messages:  
  **
  **    WDM_SETACCELSYS
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HACCELSYS - Handle to accelerator support system. 
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETACCELSYS
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HACCELSYS - Handle to accelerator support system.        
  **  
  ***************************/
  WDM_SETACCELSYS,    
  WDM_GETACCELSYS,              
  
  
  /**************************
  **
  **  Data: Handle to the window that keeps the global data. 
  **
  **  Structure Element: MainWndData.hDataWnd
  **
  **  Messages:  
  **
  **    WDM_SETMAINDATAHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HWND - Handle to the window that keeps the data.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETMAINDATAHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HWND - Handle to the window that keeps the data.       
  **  
  ***************************/
  WDM_SETMAINDATAHWND,    
  WDM_GETMAINDATAHWND,              
  
  
  /**************************
  **
  **  Data: Is the Search dialog already being displayed.
  **
  **  Structure Element: MainWndData.bSearchDlgDisplayed
  **
  **  Messages:  
  **
  **    WDM_SETSEARCHDLGDISPLAYED
  **     
  **      Purpose:       Set value of variable.
  **      dwParam:       BOOL - Is the Search dialog already being displayed.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETSEARCHDLGDISPLAYED
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       BOOL - Is the Search dialog already being displayed.       
  **  
  ***************************/
  WDM_GETSEARCHDLGDISPLAYED,    
  WDM_SETSEARCHDLGDISPLAYED,    



  /**************************
  **
  **  Data: Topic data for main topic window.
  **
  **  Structure Element: MainWndData.hTopicData
  **
  **  Messages:  
  **
  **    WDM_SETMAINTOPICDATA
  **     
  **      Purpose:       Set value of variable.
  **      dwParam:       HTOPICDATA - Topic data for main topic window.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETMAINTOPICDATA
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HTOPICDATA - Topic data for main topic window.       
  **  
  ***************************/
  WDM_SETMAINTOPICDATA,    
  WDM_GETMAINTOPICDATA,    


  /**************************
  **
  **  Data: Macro engine instance handle.
  **
  **  Structure Element: MainWndData.hMacroEngine
  **
  **  Messages:  
  **
  **    WDM_SETSECWNDDATA
  **     
  **      Purpose:       Set value of variable.
  **      dwParam:       HMACROENGINE - Macro engine instance handle.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETSECWNDDATA
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HMACROENGINE - Macro engine instance handle.       
  **  
  ***************************/
  WDM_SETMACROENGINE,    
  WDM_GETMACROENGINE,    


  /**************************
  **
  **  Data: Active secondary windows list.
  **
  **  Structure Element: MainWndData.hSecWndList
  **
  **  Messages:  
  **
  **    WDM_SETSECWNDDATA
  **     
  **      Purpose:       Set value of variable.
  **      dwParam:       HGLOBAL - Handle to the secondary windows list.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETSECWNDDATA
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HGLOBAL - Handle to the secondary windows list.       
  **  
  ***************************/
  WDM_SETSECWNDDATA,    
  WDM_GETSECWNDDATA,    


  /**************************
  **
  **  Data: Browse previous topic offset.
  **
  **  Structure Element: MainWndData.dwPrevTopicCharOffset
  **
  **  Messages:  
  **
  **    WDM_SETBROWSEPREV
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       long int - Browse previous topic offset.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETBROWSEPREV
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       long int - Browse previous topic offset.        
  **  
  ***************************/
  WDM_SETBROWSEPREVDATA,    
  WDM_GETBROWSEPREVDATA,    


  /**************************
  **
  **  Data: Browse next topic offset.
  **
  **  Structure Element: MainWndData.dwNextTopicCharOffset
  **
  **  Messages:  
  **
  **    WDM_SETBROWSENEXT
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       DWORD - Browse next topic offset.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETBROWSENEXT
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       DWORD - Browse next topic offset.        
  **  
  ***************************/
  WDM_SETBROWSENEXTDATA,    
  WDM_GETBROWSENEXTDATA,    


  /**************************
  **
  **  Data: Handle to the "Back" button data.
  **
  **  Structure Element: MainWndData.hBackBtnData
  **
  **  Messages:  
  **
  **    WDM_SETBACKDATA
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HGLOBAL - Handle to the "Back" button data.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETBACKDATA
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HGLOBAL - Handle to the "Back" button data.        
  **  
  ***************************/
  WDM_SETBACKDATA,    
  WDM_GETBACKDATA,    
  

  /**************************
  **
  **  Data: Handle of help file information buffer.
  **
  **  Structure Element: MainWndData.hHelpFileBuffer
  **
  **  Messages:  
  **
  **    WDM_SETFILEBUFFER
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HGLOBAL - Handle of help file information buffer.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETFILEBUFFER
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HGLOBAL - Handle of help file information buffer.        
  **  
  ***************************/
  WDM_SETFILEBUFFER,    
  WDM_GETFILEBUFFER,    
  

  /**************************
  **
  **  Data: Main topic window's HWND
  **
  **  Structure Element: MainWndData.hTopicWnd
  **
  **  Messages:  
  **
  **    WDM_SETMAINTOPICHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HWND - Main topic window's HWND
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETMAINTOPICHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HWND - Main topic window's HWND        
  **  
  ***************************/
  WDM_SETMAINTOPICHWND,    
  WDM_GETMAINTOPICHWND,    
  

  /**************************
  **
  **  Data: History window's HWND.
  **
  **  Structure Element: MainWndData.hHistoryWnd
  **
  **  Messages:  
  **
  **    WDM_SETMAINTOPICHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HWND - History window's HWND.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETMAINTOPICHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HWND - History window's HWND.        
  **  
  ***************************/
  WDM_SETHISTORYHWND,    
  WDM_GETHISTORYHWND,    
  

  /**************************
  **
  **  Data: Button bar window's HWND
  **
  **  Structure Element: MainWndData.hButtonBarWnd
  **
  **  Messages:  
  **
  **    WDM_SETBUTTONBARHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HWND - Button bar window's HWND
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETBUTTONBARHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HWND - Button bar window's HWND        
  **  
  ***************************/
  WDM_SETBUTTONBARHWND,    
  WDM_GETBUTTONBARHWND,    
  

  /**************************
  **
  **  Data: Window to notify when quitting.
  **
  **  Structure Element: MainWndData.hQuitWnd
  **
  **  Messages:  
  **
  **    WDM_SETPARENTHWND
  **     
  **      Purpose:       Sets value of variable.
  **      dwParam:       HWND - HWND of window to notify when quitting.
  **      fpDataBuffer:  NU
  **      Returns:       NU        
  **  
  **    WDM_GETPARENTHWND
  **
  **      Purpose:       Retrieves value of variable.
  **      dwParam:       NU
  **      fpDataBuffer:  NU
  **      Returns:       HWND - HWND of window to notify when quitting.        
  **  
  ***************************/
  WDM_SETQUITHWND,    
  WDM_GETQUITHWND    

}
WNDDATAMSG;


/* 
** WndData() function to set/retrieve
** window data in the WNDDATA structure. 
*/
DWORD __far __pascal WndData( HWND hWnd, WNDDATAMSG Msg, DWORD dwParam, void __far * fpDataBuffer );


#endif
