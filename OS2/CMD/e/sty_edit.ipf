.**************************************************************************
.*
.*  File Name   : STY_EDIT.IPF
.*
.*  Description : Information Tag Language file for help on the Edit menu
.*
.*  Copyright (C) 1992 IBM Corporation
.*
.*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
.*      sample code created by IBM Corporation. This sample code is not
.*      part of any standard or IBM product and is provided to you solely
.*      for  the purpose of assisting you in the development of your
.*      applications.  The code is provided "AS IS", without
.*      warranty of any kind.  IBM shall not be liable for any damages
.*      arising out of your use of the sample code, even if they have been
.*      advised of the possibility of such damages.                                                    *
.*
.**************************************************************************

.*--------------------------------------------------------------*\
.*  Main Edit menu
.*      res = PANEL_EDIT
.*--------------------------------------------------------------*/
:h1 res=2310 name=PANEL_EDIT.Edit Menu
:i1 id=Edit.Edit Menu
:p.Enter text for the main Edit Menu help screen here.


.*--------------------------------------------------------------*\
.*  Edit menu Undo command help panel
.*      res = PANEL_EDITUNDO
.*--------------------------------------------------------------*/
:h1 res=2320 name=PANEL_EDITUNDO.Undo
:i2 refid=Edit.Undo
:p. Place information for the Edit Undo menu item here.


.*--------------------------------------------------------------*\
.*  Edit menu Cut command help panel
.*      res = PANEL_EDITCUT
.*--------------------------------------------------------------*/
:h1 res=2330 name=PANEL_EDITCUT.Cut
:i2 refid=Edit.Cut
:p.Place information for the Edit Cut menu item here.


.*--------------------------------------------------------------*\
.*  Edit menu Copy command help panel
.*      res = PANEL_EDITCOPY
.*--------------------------------------------------------------*/
:h1 res=2340 name=PANEL_EDITCOPY.Copy
:i2 refid=Edit.Copy
:p.Place information for the Edit Copy menu item here.


.*--------------------------------------------------------------*\
.*  Edit menu Paste command help panel
.*      res = PANEL_EDITPASTE
.*--------------------------------------------------------------*/
:h1 res=2350 name=PANEL_EDITPASTE.Paste
:i2 refid=Edit.Paste
:p.Place information for the Edit Paste menu item here.


.*--------------------------------------------------------------*\
.*  Edit menu Clear command help panel
.*      res = PANEL_EDITCLEAR
.*--------------------------------------------------------------*/
:h1 res=2360 name=PANEL_EDITCLEAR.Clear
:i2 refid=Edit.Clear
:p.Place information for the Edit Clear menu item here.
