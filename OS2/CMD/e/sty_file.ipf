.**************************************************************************
.*
.*  File Name   : STY_FILE.IPF
.*
.*  Description : Information Tag Language file for help on the file menu
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
.*  Main file menu
.*      res = PANEL_FILE
.*--------------------------------------------------------------*/
:h1 res=2210 name=PANEL_FILE.File Menu
:i1 id=File.File Menu
:p.The File menu contains commands that you use to create, open,
save, and print data files. In addition, it also contains the
command that you use to exit the Sample application. The following
commands appear in the File menu:
:parml tsize=15 break=none.
:pt.New
:pd.Creates a new untitled file
:pt.Open
:pd.Opens an existing file
:pt.Save
:pd.Saves any changes to current file
:pt.Save As
:pd.Saves the current file using a new name
:pt.Print
:pd.Prints a file
:pt.Page Setup
:pd.Displays various page-design options
:pt.Print Setup
:pd.Helps you set up your printer for use with the Sample application
:pt.Exit
:pd.Quits the Sample application
:eparml.

.*--------------------------------------------------------------*\
.*  File menu New command help panel
.*      res = PANEL_FILENEW
.*--------------------------------------------------------------*/
:h1 res=2220 name=PANEL_FILENEW.New
:i2 refid=File.New
:p. You can create a new file in the Sample application window by
using the New command. To create a new file, do the following:
:ul.
:li.Select the File menu and choose the New command.
:eul.
:p. The word "Untitled" appears in the title bar of the new file.


.*--------------------------------------------------------------*\
.*  File menu Open command help panel
.*      res = PANEL_FILEOPEN
.*--------------------------------------------------------------*/
:h1 res=2230 name=PANEL_FILEOPEN.Open
:i2 refid=File.Open
:p.You can open a file that exists on any drive or in any directory
by using the Open command. To open a file, do the following:
:ul.
:li.Select the File menu and choose the Open command.
:eul.
:p.A dialog box appears, showing you a list of files in the current
directory.


.*--------------------------------------------------------------*\
.*  File menu Save command help panel
.*      res = PANEL_FILESAVE
.*--------------------------------------------------------------*/
:h1 res=2240 name=PANEL_FILESAVE.Save
:i2 refid=File.Save
:p.You use the Save command to save changes you've made to your file.
To save changes to the current file, do the following:
:ul.
:li.Select the File menu and choose the Save command.
:eul.



.*--------------------------------------------------------------*\
.*  File menu Save As command help panel
.*      res = PANEL_FILESAVEAS
.*--------------------------------------------------------------*/
:h1 res=2250 name=PANEL_FILESAVEAS.Save As
:i2 refid=File.Save As
:p.You use the Save As command to name and save a new file.
:p.Follow these steps to save a new file:
:ol.
:li.Select the File menu and choose the Save As command.
:eol.
:p.A dialog box appears, prompting you for a new filename.


.*--------------------------------------------------------------*\
.*  File menu Print command help panel
.*      res = PANEL_FILEPRINT
.*--------------------------------------------------------------*/
:h1 res=2260 name=PANEL_FILEPRINT.Print
:i2 refid=File.Print
:p.You use the Print command to print a Sample application file.
:p.Follow these steps to print a file:
:ol.
:li.Select the File menu and choose the Print command.
:li.
:eol.

.*--------------------------------------------------------------*\
.*  File menu Page Setup command help panel
.*      res = PANEL_FILEPAGESETUP
.*--------------------------------------------------------------*/
:h1 res=2270 name=PANEL_FILEPAGESETUP.PageSetup
:i2 refid=File.Page Setup
:p.You use the Page Setup command to format the page layout used when
a file is printed.  Such formats include:
:ol.
:li.Paper margins.
:li.Maximum paper margins.
:li.Header and footer text.
:eol.
:p.Follow these steps to select the page setup for a file:
:ol.
:li.Select the File menu and choose the Page Setup command.
:eol.



.*--------------------------------------------------------------*\
.*  File menu Print Setup command help panel
.*      res = PANEL_FILEPRINTSETUP
.*--------------------------------------------------------------*/
:h1 res=2280 name=PANEL_FILEPRINTSETUP.PrintSetup
:i2 refid=File.Print Setup
:p.You use the Print Setup command to set options that control how a
file is printed.  Such options include:
:ol.
:li.Specifying which pages to print.
:li.Number of copies to print.
:eol.
:p.Follow these steps to select print options for a file:
:ol.
:li.Select the File menu and choose the Print Setup command.
:eol.

.*--------------------------------------------------------------*\
.*  File menu Exit command help panel
.*      res = PANEL_FILEEXIT
.*--------------------------------------------------------------*/
:h1 res=2290 name=PANEL_FILEEXIT.Exit
:i2 refid=File.Exit
:p.You quit the Sample application by using the Exit command. To
quit the Sample application, do the following:
:ol.
:li.Select the File menu and choose the Exit command.
:eol.
