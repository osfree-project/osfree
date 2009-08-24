.**************************************************************************
.*
.*  File Name   : STY_HELP.IPF
.*
.*  Description : Information Tag Language file for help on the help menu
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
.*  Main Help menu
.*      res = PANEL_HELP
.*--------------------------------------------------------------*/
:h1 res=2910 name=PANEL_HELP.Help Menu
:i1 id=Help.Help Menu
:p.Insert standard Help menu text here.

.*--------------------------------------------------------------*\
.*  Help menu New command help panel
.*      res = PANEL_HELPHELPFORHELP
.*--------------------------------------------------------------*/
:h1 res=2920 name=PANEL_HELPHELPFORHELP.Help For Help
:i2 refid=Help.Help for Help
:p. Place information for the Help Help for Help menu here.


.*--------------------------------------------------------------*\
.*  Help menu Open command help panel
.*      res = PANEL_HELPEXTENDED
.*--------------------------------------------------------------*/
:h1 res=2930 name=PANEL_HELPEXTENDED.Extended
:i2 refid=Help.Extended
:p.Place information for the Help Extended menu item here.
:note.This screen will usually be handled by the default.


.*--------------------------------------------------------------*\
.*  Help menu Save command help panel
.*      res = PANEL_HELPKEYS
.*--------------------------------------------------------------*/
:h1 res=2940 name=PANEL_HELPKEYS.Keys
:i2 refid=Help.Keys
:p.Place information for the Help Keys menu item here.


.*--------------------------------------------------------------*\
.*  Help menu Save As command help panel
.*      res = PANEL_HELPINDEX
.*--------------------------------------------------------------*/
:h1 res=2950 name=PANEL_HELPINDEX.Index
:i2 refid=Help.Index
:p.Place information for the Help Index menu item here.


.*--------------------------------------------------------------*\
.*  Help menu About command help panel
.*      res = PANEL_HELPABOUT
.*--------------------------------------------------------------*/
:h1 res=2960 name=PANEL_HELPABOUT.About
:i2 refid=Help.About
:p.Place information for the Help About menu item here.
