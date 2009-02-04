.**************************************************************************
.*
.*  File Name   : STY_MENU.IPF
.*
.*  Description : Information Tag Language file for help on menu items
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
.*  Main Options menu
.*      res = PANEL_OPTIONS
.*--------------------------------------------------------------*/
:h1 res=2410 name=PANEL_OPTIONS.Options Menu
:i1 id=Options.Options Menu
:p.The Options menu allows you to change the appearance of the text
in the MLE.  The following commands appear in the Options menu:
:parml tsize=15 break=none.
:pt.Foreground Color
:pd.Sets the text color
:pt.Background Color
:pd.Sets the color of the window background
:pt.Font
:pd.Sets the font used for the text
:eparml.

.*--------------------------------------------------------------*\
.*  Options menu Foreground Color command help panel
.*      res = PANEL_OPTIONSFORECOLOR
.*--------------------------------------------------------------*/
:h1 res=2420 name=PANEL_OPTIONSFORECOLOR.Foreground Color
:i2 refid=Options.Foreground Color
:p. You can change the color of the text in the Style application
window by using the Foreground Color command. To change the text color,
do the following:
:ul.
:li.Select the Options menu and choose the Foreground Color command.
:eul.
:p.A dialog box appears, allowing you to select a new text color.

.*--------------------------------------------------------------*\
.*  Options menu Background Color menu help panel
.*      res = PANEL_OPTIONSBACKCOLOR
.*--------------------------------------------------------------*/
:h1 res=2430 name=PANEL_OPTIONSBACKCOLOR.Background Color
:i2 refid=Options.Background Color
:p.The Background Color menu allows you to change the color of the
window background of the Style application.  The following commands
appear in the Background Color menu:
:parml tsize=15 break=none.
:pt.Yellow
:pd.Sets the window color to yellow
:pt.Pink
:pd.Sets the window color to pink
:pt.Cyan
:pd.Sets the window color to cyan
:pt.Default
:pd.Sets the window color to the default window color
:eparml.


.*--------------------------------------------------------------*\
.*  Options menu Background Color Yellow command help panel
.*      res = PANEL_OPTIONSBACKCOLORYELLOW
.*--------------------------------------------------------------*/
:h1 res=2431 name=PANEL_OPTIONSBACKCOLORYELLOW.Background Color Yellow
.*:i2 refid=Options.Background Color Yellow
:p. You can change the color of the window background of the Style
application to yellow by using the Background Color Yellow command.
To change the background color to yellow, do the following:
:ul.
:li.Select the Options menu and choose the Background Color menu.
:li.Select the Yellow command.
:eul.

.*--------------------------------------------------------------*\
.*  Options menu Background Color Pink command help panel
.*      res = PANEL_OPTIONSBACKCOLORPINK
.*--------------------------------------------------------------*/
:h1 res=2432 name=PANEL_OPTIONSBACKCOLORPINK.Background Color Pink
.*:i2 refid=Options.Background Color Pink
:p. You can change the color of the window background of the Style
application to pink by using the Background Color Pink command.
To change the background color to pink, do the following:
:ul.
:li.Select the Options menu and choose the Background Color menu.
:li.Select the Pink command.
:eul.


.*--------------------------------------------------------------*\
.*  Options menu Background Color Cyan command help panel
.*      res = PANEL_OPTIONSBACKCOLORCYAN
.*--------------------------------------------------------------*/
:h1 res=2433 name=PANEL_OPTIONSBACKCOLORCYAN.Background Color Cyan
.*:i2 refid=Options.Background Color Cyan
:p. You can change the color of the window background of the Style
application to cyan by using the Background Color Cyan command.
To change the background color to cyan, do the following:
:ul.
:li.Select the Options menu and choose the Background Color menu.
:li.Select the Cyan command.
:eul.

.*--------------------------------------------------------------*\
.*  Options menu Background Color Default command help panel
.*      res = PANEL_OPTIONSBACKCOLORDEFAULT
.*--------------------------------------------------------------*/
:h1 res=2434 name=PANEL_OPTIONSBACKCOLORDEFAULT.Background Color Default
.*:i2 refid=Options.Background Color Default
:p. You can change the color of the window background of the Style
application to the default window color by using the Background Color
Default command.  To change the background color to the default,
do the following:
:ul.
:li.Select the Options menu and choose the Background Color menu.
:li.Select the Default command.
:eul.

.*--------------------------------------------------------------*\
.*  Options menu Font command help panel
.*      res = PANEL_OPTIONSFONT
.*--------------------------------------------------------------*/
:h1 res=2440 name=PANEL_OPTIONSFONT.Font
.*:i2 refid=Options.Font
:p. You can change the font of the text in the Style application
window by using the Font command. To change the font, do the
following:
:ul.
:li.Select the Options menu and choose the Font command.
:eul.
:p.A dialog box appears, allowing you to select a new font.





.*--------------------------------------------------------------*\
.*  Main Demo menu
.*      res = PANEL_DEMO
.*--------------------------------------------------------------*/
:h1 res=2510 name=PANEL_DEMO.Demo Menu
:i1 id=Demo.Demo Menu
:p.The Demo menu contains some dialog boxes that demonstrates the
various controls available in Presentation Manager, as well as
samples of the various types of Message Boxes.  The following
commands appear in the Demo menu:
:parml tsize=15 break=none.
:pt.Dialog Controls
:pd.Dialog boxes that demonstrate the dialog controls
:pt.Message Boxes
:pd.Samples of the different types of Message Boxes
:eparml.


.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls menu help panel
.*      res = PANEL_DEMODLG
.*--------------------------------------------------------------*/
:h1 res=2520 name=PANEL_DEMODLG.Dialog Controls
:i2 refid=Demo.Dialog Controls
:p.The Dialog Controls menu allows you to view a dialog which
demonstrates some of the dialog control windows.  The following
commands appear in the Dialog Controls menu:
:parml tsize=15 break=none.
:pt.Buttons
:pd.Demonstrates the various types of button controls
:pt.List Boxes
:pd.Demonstrates the various types of list box controls
:pt.Combo Boxes
:pd.Demonstrates the various types of combo box controls
:pt.Entry Fields
:pd.Demonstrates the various types of entry field controls
:pt.Static
:pd.Demonstrates the various types of static controls
:pt.Presentation Parameters
:pd.Demonstrates how to set the presentation parameters of a control
:pt.Container
:pd.Demonstrates a container control
:pt.Valueset
:pd.Demonstrates a valueset control
:pt.Sliders
:pd.Demonstrates a horizontal and vertical slider controls
:pt.Spinbuttons
:pd.Demonstrates a master and servant spin button controls
:pt.Notebook
:pd.Demonstrates a notebook control
:eparml.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Buttons command help panel
.*      res = PANEL_DEMODLGBUTTONS
.*--------------------------------------------------------------*/
:h1 res=2521 name=PANEL_DEMODLGBUTTONS.Buttons
.*:i2 refid=Demo.Buttons Dialog
:p.This command displays a dialog demonstrating the various types
of button controls available in Presentation Manager.  To view the
button dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Buttons command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls List Boxes command help panel
.*      res = PANEL_DEMODLGLISTBOXES
.*--------------------------------------------------------------*/
:h1 res=2522 name=PANEL_DEMODLGLISTBOXES.ListBoxes
.*:i2 refid=Demo.List Box Dialog
:p.This command displays a dialog demonstrating the various types
of list-box controls available in Presentation Manager.  To view the
list-box dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the List Boxes command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Combo Boxes command help panel
.*      res = PANEL_DEMODLGCOMBOBOXES
.*--------------------------------------------------------------*/
:h1 res=2523 name=PANEL_DEMODLGCOMBOBOXES.ComboBoxes
.*:i2 refid=Demo.Combo Box Dialog
:p.This command displays a dialog demonstrating the various types
of combo-box controls available in Presentation Manager.  To view the
combo-box dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Combo Boxes command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Entry Fields command help panel
.*      res = PANEL_DEMODLGENTRYFIELDS
.*--------------------------------------------------------------*/
:h1 res=2524 name=PANEL_DEMODLGENTRYFIELDS.EntryFields
.*:i2 refid=Demo.Entry Field Dialog
:p.This command displays a dialog demonstrating the various types
of entry field controls available in Presentation Manager.  To view the
entry field dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Entry Fields command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Static command help panel
.*      res = PANEL_DEMODLGSTATIC
.*--------------------------------------------------------------*/
:h1 res=2525 name=PANEL_DEMODLGSTATIC.Static
.*:i2 refid=Demo.Static Dialog
:p.This command displays a dialog demonstrating the various types
of static controls available in Presentation Manager.  To view the
static dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Static command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls PP Demo command help panel
.*      res = PANEL_DEMODLGPP
.*--------------------------------------------------------------*/
:h1 res=2526 name=PANEL_DEMODLGPP.Presentation Parameter Demo
.*:i2 refid=Demo.Pres. Param. Dialog
:p.This command displays a dialog demonstrating the method for
setting presentation parameters on some dialog controls.  To view the
presentation parameter dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Presentation Parameters command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Container command help panel
.*      res = PANEL_DEMODLGCONTAINER
.*--------------------------------------------------------------*/
:h1 res=2527 name=PANEL_DEMODLGCONTAINER.Container
.*:i2 refid=Demo.Container Dialog
:p.This command displays a dialog demonstrating the container
control available in Presentation Manager.  To view the
container dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Container command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Notebook command help panel
.*      res = PANEL_DEMODLGNOTEBOOK
.*--------------------------------------------------------------*/
:h1 res=2528 name=PANEL_DEMODLGNOTEBOOK.Notebook
.*:i2 refid=Demo.Notebook Dialog
:p.This command displays a dialog demonstrating the notebook
control available in Presentation Manager.  To view the
notebook dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Notebook command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Slider command help panel
.*      res = PANEL_DEMODLGSLIDER
.*--------------------------------------------------------------*/
:h1 res=2529 name=PANEL_DEMODLGSLIDER.Slider
.*:i2 refid=Demo.Slider Dialog
:p.This command displays a dialog demonstrating the slider
control available in Presentation Manager.  To view the
slider dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Slider command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Valueset command help panel
.*      res = PANEL_DEMODLGVALUESET
.*--------------------------------------------------------------*/
:h1 res=2560 name=PANEL_DEMODLGVALUESET.Valueset
.*:i2 refid=Demo.valueset Dialog
:p.This command displays a dialog demonstrating the valueset
control available in Presentation Manager.  To view the
valueset dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Valueset command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Dialog Controls Spinbutton command help panel
.*      res = PANEL_DEMODLGSPINBUTTON
.*--------------------------------------------------------------*/
:h1 res=2561 name=PANEL_DEMODLGSPINBUTTON.Spinbutton
.*:i2 refid=Demo.Spinbutton Dialog
:p.This command displays a dialog demonstrating the spinbutton
control available in Presentation Manager.  To view the
spinbutton dialog, do the following:
:ul.
:li.Select the Dialog Controls menu and choose the Spinbutton command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes menu help panel
.*      res = PANEL_DEMOMB
.*--------------------------------------------------------------*/
:h1 res=2530 name=PANEL_DEMOMB.Message Boxes
:i2 refid=Demo.Demo Message Boxes
:p.The Message Boxes menu allows you to view a examples of each
type of Message Box available.  The following commands appear
in the Dialog Controls menu:
:parml tsize=15 break=none.
:pt.Ok Button
:pd.Example of a Message Box with only an Ok button
:pt.Ok/Cancel Buttons
:pd.Example of a Message Box with Ok and Cancel buttons
:pt.Yes/No Buttons
:pd.Example of a Message Box with Yes and No buttons
:pt.Yes/No/Cancel Buttons
:pd.Example of a Message Box with Yes, No, and Cancel buttons
:pt.Retry/Cancel Buttons
:pd.Example of a Message Box with Retry and Cancel buttons
:pt.Abort/Retry/Ignore Buttons
:pd.Example of a Message Box with Abort, Retry, and Ignore buttons
:pt.Enter Button
:pd.Example of a Message Box with only an Enter button
:pt.Enter/Cancel Buttons
:pd.Example of a Message Box with Enter and Cancel buttons
:pt.Query Icon
:pd.Example of a Message Box with a Query icon
:pt.Warning Icon
:pd.Example of a Message Box with a Warning icon
:pt.Information Icon
:pd.Example of a Message Box with a Information icon
:pt.Critical Icon
:pd.Example of a Message Box with a Critical icon
:pt.Application Modal
:pd.Example of a Message Box that is application modal
:pt.System Modal
:pd.Example of a Message Box that is system modal
:pt.Moveable
:pd.Example of a Message Box that is moveable
:pt.Help
:pd.Example of a Message Box that has a help button
:eparml.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Ok Button command help panel
.*      res = PANEL_DEMOMBOK
.*--------------------------------------------------------------*/
:h1 res=2531 name=PANEL_DEMOMBOK.Ok Button
.*:i2 refid=Demo.Ok Button
:p.This command displays a Message Box which contains only an Ok
button.  To view the Ok Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Ok Button command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Ok/Cancel command help panel
.*      res = PANEL_DEMOMBOKCANCEL
.*--------------------------------------------------------------*/
:h1 res=2532 name=PANEL_DEMOMBOKCANCEL.Ok/Cancel Buttons
.*:i2 refid=Demo.Ok/Cancel Buttons
:p.This command displays a Message Box which contains Ok and Cancel
buttons.  To view the Ok/Cancel Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Ok/Cancel Buttons command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Yes/No command help panel
.*      res = PANEL_DEMOMBYESNO
.*--------------------------------------------------------------*/
:h1 res=2533 name=PANEL_DEMOMBYESNO.Yes/No Buttons
.*:i2 refid=Demo.Yes/No Buttons
:p.This command displays a Message Box which contains Yes and No
buttons.  To view the Yes/No Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Yes/No Buttons command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Yes/No/Cancel command help panel
.*      res = PANEL_DEMOMBYESNOCANCEL
.*--------------------------------------------------------------*/
:h1 res=2534 name=PANEL_DEMOMBYESNOCANCEL.Yes/No/Cancel Buttons
.*:i2 refid=Demo.Yes/No/Cancel Buttons
:p.This command displays a Message Box which contains Yes, No, and Cancel
buttons.  To view the Yes/No/Cancel Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Yes/No/Cancel
Buttons command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Retry/Cancel command help panel
.*      res = PANEL_DEMOMBRETRY
.*--------------------------------------------------------------*/
:h1 res=2535 name=PANEL_DEMOMBRETRY.Retry/Cancel Buttons
.*:i2 refid=Demo.Retry/Cancel Buttons
:p.This command displays a Message Box which contains Retry and Cancel
buttons.  To view the Retry/Cancel Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Retry/Cancel Buttons command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Abort/Retry/Ignore cmd help panel
.*      res = PANEL_DEMOMBABORT
.*--------------------------------------------------------------*/
:h1 res=2536 name=PANEL_DEMOMBABORT.Abort/Retry/Ignore Buttons
.*:i2 refid=Demo.Abort/Retry/Ignore Buttons
:p.This command displays a Message Box which contains Abort, Retry,
and Ignore buttons.  To view the Abort/Retry/Ignore Button Message
Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Abort/Retry/Ignore
Buttons command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Enter Button command help panel
.*      res = PANEL_DEMOMBENTER
.*--------------------------------------------------------------*/
:h1 res=2537 name=PANEL_DEMOMBENTER.Enter Button
.*:i2 refid=Demo.Enter Button
:p.This command displays a Message Box which contains only an Enter
button.  To view the Enter Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Enter Button command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Enter/Cancel command help panel
.*      res = PANEL_DEMOMBENTERCANCEL
.*--------------------------------------------------------------*/
:h1 res=2538 name=PANEL_DEMOMBENTERCANCEL.Enter/Cancel Buttons
.*:i2 refid=Demo.Enter/Cancel Buttons
:p.This command displays a Message Box which contains Enter and Cancel
buttons.  To view the Enter/Cancel Button Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Enter/Cancel Buttons
command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Query Icon command help panel
.*      res = PANEL_DEMOMBQUERY
.*--------------------------------------------------------------*/
:h1 res=2541 name=PANEL_DEMOMBQUERY.Query Icon
.*:i2 refid=Demo.Query Icon
:p.This command displays a Message Box which contains a Query icon.
To view the Query Icon Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Query Icon command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Warning Icon command help panel
.*      res = PANEL_DEMOMBWARNING
.*--------------------------------------------------------------*/
:h1 res=2542 name=PANEL_DEMOMBWARNING.Warning Icon
.*:i2 refid=Demo.Warning Icon
:p.This command displays a Message Box which contains a Warning icon.
To view the Warning Icon Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Warning Icon command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Information Icon command help panel
.*      res = PANEL_DEMOMBINFORMATION
.*--------------------------------------------------------------*/
:h1 res=2543 name=PANEL_DEMOMBINFORMATION.Information Icon
.*:i2 refid=Demo.Information Icon
:p.This command displays a Message Box which contains a Information
icon.  To view the Information Icon Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Information Icon
command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Critical Icon command help panel
.*      res = PANEL_DEMOMBCRITICAL
.*--------------------------------------------------------------*/
:h1 res=2544 name=PANEL_DEMOMBCRITICAL.Critical Icon
.*:i2 refid=Demo.Critical Icon
:p.This command displays a Message Box which contains a Critical
icon.  To view the Critical Icon Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Critical Icon
command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes App Modal command help panel
.*      res = PANEL_DEMOMBAPPMODAL
.*--------------------------------------------------------------*/
:h1 res=2551 name=PANEL_DEMOMBAPPMODAL.Application Modal
.*:i2 refid=Demo.Application Modal
:p.This command displays a Message Box which is modal to the
application.  To view the Application Modal Message Box, do
the following:
:ul.
:li.Select the Message Boxes menu and choose the Application
Modal command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Sys Modal command help panel
.*      res = PANEL_DEMOMBSYSMODAL
.*--------------------------------------------------------------*/
:h1 res=2552 name=PANEL_DEMOMBSYSMODAL.System Modal
.*:i2 refid=Demo.System Modal
:p.This command displays a Message Box which is modal to the
system.  To view the Application Modal Message Box, do the
following:
:ul.
:li.Select the Message Boxes menu and choose the System Modal
command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Moveable command help panel
.*      res = PANEL_DEMOMBMOVEABLE
.*--------------------------------------------------------------*/
:h1 res=2553 name=PANEL_DEMOMBMOVEABLE.Moveable
.*:i2 refid=Demo.Moveable
:p.This command displays a Message Box which is moveable.  To view
the Moveable Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Moveable command.
:eul.

.*--------------------------------------------------------------*\
.*  Demo menu Message Boxes Help command help panel
.*      res = PANEL_DEMOMBHELP
.*--------------------------------------------------------------*/
:h1 res=2554 name=PANEL_DEMOMBHELP.Help Message Box
.*:i2 refid=Demo.Help Button
:p.This command displays a Message Box which has a Help button.  To
view the Help Message Box, do the following:
:ul.
:li.Select the Message Boxes menu and choose the Help command.
:eul.
