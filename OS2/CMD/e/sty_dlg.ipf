.**************************************************************************
.*
.*  File Name   : STY_DLG.IPF
.*
.*  Description : Information Tag Language file for help on the dialog boxes.
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
.*  Help for DEMOMSGBOX
.*      res = PANEL_DEMOMSGBOX
.*--------------------------------------------------------------*/
:h1 res=3100 name=PANEL_DEMOMSGBOX.Demo Message Box
:p.This dialog demonstrates the different attributes of a Message
Box created with the WinMessageBox() function.

.*--------------------------------------------------------------*\
.*  Demo Message Box Ok button
.*      res = PANEL_DEMOMSGBOX_OK
.*--------------------------------------------------------------*/
:h1 res=3110 name=PANEL_DEMOMSGBOX_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Demo Message Box Help button
.*      res = PANEL_DEMOMSGBOX_HELP
.*--------------------------------------------------------------*/
:h1 res=3120 name=PANEL_DEMOMSGBOX_HELP.Help
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for BUTTONSDLG
.*      res = PANEL_BUTTONSDLG
.*--------------------------------------------------------------*/
:h1 res=4100 name=PANEL_BUTTONSDLG.Button Demo Dialog
:p.This dialog demonstrates the differnt styles of button controls.

.*--------------------------------------------------------------*\
.*  Buttons Demo Dialog Radio buttons
.*      res = PANEL_BUTTONSDLG_RADIO
.*--------------------------------------------------------------*/
:h1 res=4110 name=PANEL_BUTTONSDLG_RADIO.Radio
:p.This button is an example of a radio button.  Only one radio
button in a group of buttons can be selected at one time.  When
this button is selected, all other radio buttons in the same
group are automatically unselected.

.*--------------------------------------------------------------*\
.*  Buttons Demo Dialog Check Box
.*      res = PANEL_BUTTONSDLG_CHECKBOX
.*--------------------------------------------------------------*/
:h1 res=4120 name=PANEL_BUTTONSDLG_CHECKBOX.Check Box
:p.A check box is a small square window that is empty when it is
unselected, and contains an "x" when selected.  Check box text is
displayed to the right of the check box.

.*--------------------------------------------------------------*\
.*  Buttons Demo Dialog Three State
.*      res = PANEL_BUTTONSDLG_THREESTATE
.*--------------------------------------------------------------*/
:h1 res=4130 name=PANEL_BUTTONSDLG_THREESTATE.Three State
:p.Similar to a check box, except that it toggles between selected,
unselected, and halftone states.

.*--------------------------------------------------------------*\
.*  Buttons Demo Dialog Push button
.*      res = PANEL_BUTTONSDLG_PUSHBUTTON
.*--------------------------------------------------------------*/
:h1 res=4140 name=PANEL_BUTTONSDLG_PUSHBUTTON.Push Button
:p.A push button.  The button posts a WM_COMMAND message to its
owner window when selected.

.*--------------------------------------------------------------*\
.*  Buttons Demo Dialog Ok button
.*      res = PANEL_BUTTONSDLG_OK
.*--------------------------------------------------------------*/
:h1 res=4150 name=PANEL_BUTTONSDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Buttons Demo Dialog Help button
.*      res = PANEL_BUTTONSDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=4160 name=PANEL_BUTTONSDLG_HELP.Help
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for LISTBOXDLG
.*      res = PANEL_LISTBOXDLG
.*--------------------------------------------------------------*/
:h1 res=5100 name=PANEL_LISTBOXDLG.List Box Demo Dialog
:p.This dialog demonstrates the different styles of list boxes.

.*--------------------------------------------------------------*\
.*  LISTBOX Demo Dialog Radio Single
.*      res = PANEL_LISTBOXDLG_SINGLE
.*--------------------------------------------------------------*/
:h1 res=5110 name=PANEL_LISTBOXDLG_SINGLE.Single Entry
:p.This listbox is an example of a single entry list box.  Only
one item at a time can be selected.

.*--------------------------------------------------------------*\
.*  LISTBOX Demo Dialog Multiple
.*      res = PANEL_LISTBOXDLG_MULTIPLE
.*--------------------------------------------------------------*/
:h1 res=5120 name=PANEL_LISTBOXDLG_MULTIPLE.Multiple Entry
:p.This listbox is an example of a multiple entry list box.  More
than one item can be selected at a time.

.*--------------------------------------------------------------*\
.*  LISTBOX Demo Dialog Ok button
.*      res = PANEL_LISTBOXDLG_OK
.*--------------------------------------------------------------*/
:h1 res=5130 name=PANEL_LISTBOXDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  LISTBOX Demo Dialog Help button
.*      res = PANEL_LISTBOXDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=5140 name=PANEL_LISTBOXDLG_HELP.Help
:p.This button calls the help scre`en for the selected item.

.*--------------------------------------------------------------*\
.*  Help for COMBOBOXDLG
.*      res = PANEL_COMBOBOXDLG
.*--------------------------------------------------------------*/
:h1 res=6100 name=PANEL_COMBOBOXDLG.ComboBox Demo Dialog
:p.This dialog demonstrates the different styles of combo boxes.

.*--------------------------------------------------------------*\
.*  Combo Box Demo Dialog Simple
.*      res = PANEL_COMBOBOXDLG_SIMPLE
.*--------------------------------------------------------------*/
:h1 res=6110 name=PANEL_COMBOBOXDLG_SIMPLE.Simple Combo Box
:p.This combo box is an example of a Simple combo box.  Simple
combo boxes are made up of a listbox control and an entry field
control that are visible at all times.

.*--------------------------------------------------------------*\
.*  Combo Box Demo Dialog Drop-down
.*      res = PANEL_COMBOBOXDLG_DROPDOWN
.*--------------------------------------------------------------*/
:h1 res=6120 name=PANEL_COMBOBOXDLG_DROPDOWN.Drop-down
:p.This combo box is an example of a drop-down combo box.  A drop-down
combo box is made up of an entry field control and a button.  When the
user selects the button, a listbox control appears.

.*--------------------------------------------------------------*\
.*  Combo Box Demo Dialog Drop-down List
.*      res = PANEL_COMBOBOXDLG_DROPDOWNLIST
.*--------------------------------------------------------------*/
:h1 res=6130 name=PANEL_COMBOBOXDLG_DROPDOWNLIST.Drop-down List
:p.This combo box is an example of a Drop-down List combo box.  A
Drop-down List combo box is similar to a Drop-down combo box, but
the user cannot enter or edit text in the entry field.

.*--------------------------------------------------------------*\
.*  Combo Box Demo Dialog Ok button
.*      res = PANEL_COMBOBOXDLG_OK
.*--------------------------------------------------------------*/
:h1 res=6140 name=PANEL_COMBOBOXDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Combo Box Demo Dialog Help button
.*      res = PANEL_COMBOBOXDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=6150 name=PANEL_COMBOBOXDLG_HELP.Help
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for ENTRYFIELDDLG
.*      res = PANEL_ENTRYFIELDDLG
.*--------------------------------------------------------------*/
:h1 res=7100 name=PANEL_ENTRYFIELDDLG.Entry Field Demo Dialog
:p.This dialog demonstrates the different styles of entry fields.

.*--------------------------------------------------------------*\
.*  Entry Field Demo Dialog Single line
.*      res = PANEL_ENTRYFIELDDLG_ENTRY
.*--------------------------------------------------------------*/
:h1 res=7110 name=PANEL_ENTRYFIELDDLG_SIMPLE.Entry Field
:p.This entry field is an example of a Single line entry field.
The entry field only accepts a single line of text but will scroll
horizontally should the text length be longer than the visible
area of the control.

.*--------------------------------------------------------------*\
.*  Entry Field Demo Dialog MLE
.*      res = PANEL_ENTRYFIELDDLG_MLE
.*--------------------------------------------------------------*/
:h1 res=7120 name=PANEL_ENTRYFIELDDLG_MLE.Multiple Line Entry Field
:p.This entry field is an example of a multiple line entry field (MLE).
An MLE is a very sophisticated control window that users use to view
and edit multiple lines of text.  An MLE plrovides all the text-
editing capability of a simple text editor, making these features
readily available to applications.

.*--------------------------------------------------------------*\
.*  Entry Field Demo Dialog Ok button
.*      res = PANEL_ENTRYFIELDDLG_OK
.*--------------------------------------------------------------*/
:h1 res=7130 name=PANEL_ENTRYFIELDDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Entry Field Demo Dialog Help button
.*      res = PANEL_ENTRYFIELDDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=7140 name=PANEL_ENTRYFIELDDLG_HELP.Help
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for STATICDLG
.*      res = PANEL_STATICDLG
.*--------------------------------------------------------------*/
:h1 res=8100 name=PANEL_STATICDLG.Static Demo Dialog
:p.This dialog demonstrates the different styles of static controls.

.*--------------------------------------------------------------*\
.*  Static Demo Dlg Ok button
.*      res = PANEL_STATICDLG_OK
.*--------------------------------------------------------------*/
:h1 res=8110 name=PANEL_STATICDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Static Demo Dlg Help button
.*      res = PANEL_STATICDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=8120 name=PANEL_STATICDLG_HELP.Help
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help Panels for PPDEMODLG
.*      res = PANEL_PPDEMODLG
.*--------------------------------------------------------------*/
:h1 res=9100 name=PANEL_PPDEMODLG.Pres. Param. Demo Dialog
:p.This dialog demonstrates how to use Presentation Parameters with
control windows.  Presentation Parameters are used to set display
attributes of a particular control window.  Such attributes include
foreground and background color as well as the font the control
uses to display its text.
:p.This dialog allows you to set the foreground color, background
color, and font for two controls.  The two controls used, a static
text box and a check box button, are on the left side of the center
dividing line of the dialog.  On the right side of the dialog are
three combo boxes, which are used to set the three attributes.
:p.To set an attribute for the controls, select the value desired
from the appropriate combo box.  For example, to set the foreground
color of the controls to blue, select blue from the Foreground
combo box.  Select the Ok button when you have set all the attributes
you want.  The foreground color, background color, and font
presentation parameters of both controls will be set to the values
you specified.
:p.When finished with the dialog, select the Apply button.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Foreground Color List
.*      res = PANEL_PPDEMODLG_FORECOLORLIST
.*--------------------------------------------------------------*/
:h1 res=9110 name=PANEL_PPDEMODLG_FORECOLORLIST.Foreground Color List
:p.This combo box allows you to set the foreground color of the
controls.  Simply select the foreground color desired and select the
Ok button.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Background Color List
.*      res = PANEL_PPDEMODLG_BACKCOLORLIST
.*--------------------------------------------------------------*/
:h1 res=9120 name=PANEL_PPDEMODLG_BACKCOLORLIST.Background Color List
:p.This combo box allows you to set the background color of the
controls.  Simply select the background color desired and select the
Ok button.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Font List
.*      res = PANEL_PPDEMODLG_FONTLIST
.*--------------------------------------------------------------*/
:h1 res=9130 name=PANEL_PPDEMODLG_FONTLIST.Font List
:p.This combo box allows you to set the font for the text of the
controls.  Simply select the font desired and select the Ok button.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Test button
.*      res = PANEL_PPDEMODLG_FONTLIST
.*--------------------------------------------------------------*/
:h1 res=9140 name=PANEL_PPDEMODLG_TESTBUTTON.Test Button
:p.This button is used to show the effect of changing the
presentation parameters of a check box button.  The button's
foreground color, background color, and font can be changed by
selecting the appropriate attribute and selecting the Ok button.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Apply button
.*      res = PANEL_PPDEMODLG_APPLY
.*--------------------------------------------------------------*/
:h1 res=9150 name=PANEL_PPDEMODLG_APPLY.Apply
:p.This button causes the presentation parameters of the two
test controls to be set to the values selected.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Cancel button
.*      res = PANEL_PPDEMODLG_CANCEL
.*--------------------------------------------------------------*/
:h1 res=9160 name=PANEL_PPDEMODLG_CANCEL.Cancel
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Pres. Param Demo Dialog Help button
.*      res = PANEL_PPDEMODLG_HELP
.*--------------------------------------------------------------*/
:h1 res=9170 name=PANEL_PPDEMODLG_HELP.Help
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for ABOUTBOX
.*      res = PANEL_ABOUTBOX
.*--------------------------------------------------------------*/
:h1 res=10100 name=PANEL_ABOUTBOX.About Box
:p.This dialog displays the version number, copyright notice,
and any other pertinent information for the application.

.*--------------------------------------------------------------*\
.*  About Box Dlg Ok button
.*      res = PANEL_ABOUTBOX_OK
.*--------------------------------------------------------------*/
:h1 res=10110 name=PANEL_ABOUTBOX_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Help for CONTAINERDLG
.*      res = PANEL_CONTAINERDLG_CONTAINER
.*--------------------------------------------------------------*/
:h1 res=11110 name=PANEL_CONTAINERDLG_CONTAINER.Container Demo Dialog
:p.This dialog demonstrates the container control. To display other
other views of the container, double click mouse button two and select
the desired view.
.*--------------------------------------------------------------*\
.*  Container Demo Dlg Ok button
.*      res = PANEL_CONTAINERDLG_OK
.*--------------------------------------------------------------*/
:h1 res=11120 name=PANEL_CONTAINERDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Container Demo Dlg Cancel button
.*      res = PANEL_CONTAINERDLG_CANCEL
.*--------------------------------------------------------------*/
:h1 res=11130 name=PANEL_CONTAINERDLG_CANCEL.CANCEL
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Container Demo Dlg Help button
.*      res = PANEL_CONTAINERDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=11140 name=PANEL_CONTAINERDLG_HELP.HELP
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for NOTEBOOKDLG
.*      res = PANEL_NOTEBOOKDLG_NOTEBOOK
.*--------------------------------------------------------------*/
:h1 res=12110 name=PANEL_NOTEBOOKDLG_NOTEBOOK.Notebook Demo Dialog
:p.This dialog demonstrates the notebook control. To change the page
displayed in the control,  select the arrow in the corner of the
notebook page.

.*--------------------------------------------------------------*\
.*  Notebook Demo Dlg Ok button
.*      res = PANEL_NOTEBOOKDLG_OK
.*--------------------------------------------------------------*/
:h1 res=12120 name=PANEL_NOTEBOOKDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Notebook Demo Dlg Cancel button
.*      res = PANEL_NOTEBOOKDLG_CANCEL
.*--------------------------------------------------------------*/
:h1 res=12130 name=PANEL_NOTEBOOKDLG_CANCEL.CANCEL
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Notebook Demo Dlg Help button
.*      res = PANEL_NOTEBOOKDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=12140 name=PANEL_NOTEBOOKDLG_HELP.HELP
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for Vertical Slider
.*      res = PANEL_SLIDERDLG_VSLIDER
.*--------------------------------------------------------------*/
:h1 res=13110 name=PANEL_SLIDERDLG_VSLIDER.Vertical Slider
:p.This is an example of a vertical slider control.
Move the slider arm or click on the up or down arrows to change the
output of the slider.

.*--------------------------------------------------------------*\
.*  Help for Horizontal Slider
.*      res = PANEL_SLIDERDLG_HSLIDER
.*--------------------------------------------------------------*/
:h1 res=13120 name=PANEL_SLIDERDLG_HSLIDER.Horizontal Slider
:p.This is an example of a horizontal slider control.
Move the slider arm or click on the up or down arrows to change the
output of the slider.

.*--------------------------------------------------------------*\
.*  Slider Demo Dlg Ok button
.*      res = PANEL_SLIDERDLG_OK
.*--------------------------------------------------------------*/
:h1 res=13130 name=PANEL_SLIDERDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Slider Demo Dlg Cancel button
.*      res = PANEL_SLIDERDLG_CANCEL
.*--------------------------------------------------------------*/
:h1 res=13140 name=PANEL_SLIDERDLG_CANCEL.CANCEL
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Slider Demo Dlg Help button
.*      res = PANEL_SLIDERDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=13150 name=PANEL_SLIDERDLG_HELP.HELP
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for VALUESETDLG
.*      res = PANEL_VALUESETDLG_VALUESET
.*--------------------------------------------------------------*/
:h1 res=14110 name=PANEL_VALUESETDLG_VALUESET.Valueset Demo Dialog
:p.This dialog demonstrates a valueset control.

.*--------------------------------------------------------------*\
.*  Valueset Demo Dlg Ok button
.*      res = PANEL_VALUESETDLG_OK
.*--------------------------------------------------------------*/
:h1 res=14120 name=PANEL_VALUESETDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Valueset Demo Dlg Cancel button
.*      res = PANEL_VALUESETDLG_CANCEL
.*--------------------------------------------------------------*/
:h1 res=14130 name=PANEL_VALUESETDLG_CANCEL.CANCEL
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Valueset Demo Dlg Help button
.*      res = PANEL_VALUESETDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=14140 name=PANEL_VALUESETDLG_HELP.HELP
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for Master Spinbutton
.*      res = PANEL_SPINBUTTONDLG_MASTER
.*--------------------------------------------------------------*/
:h1 res=15110 name=PANEL_SPINBUTTONDLG_MASTER.Master spinbutton
:p.This is an example of a master spinbutton control.
Values in the control may be changed by pressing the up or down
arror keys or by mouse selecting the up or down arrow buttons of
the spinbutton.

.*--------------------------------------------------------------*\
.*  Help for Servant Spinbuttong
.*      res = PANEL_SPINBUTTONDLG_SERVANT
.*--------------------------------------------------------------*/
:h1 res=15120 name=PANEL_SPINBUTTONDLG_SERVANT.Servant spinbutton
:p.This is an example of a servant spinbutton control.
Values in the servant control may be changed by pressing the up or down
arror keys or by mouse selecting the up or down arrow buttons of
the spinbutton

.*--------------------------------------------------------------*\
.*  Spinbutton Demo Dlg Ok button
.*      res = PANEL_SPINBUTTONDLG_OK
.*--------------------------------------------------------------*/
:h1 res=15130 name=PANEL_SPINBUTTONDLG_OK.OK
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Spinbutton Demo Dlg Cancel button
.*      res = PANEL_SPINBUTTONDLG_CANCEL
.*--------------------------------------------------------------*/
:h1 res=15140 name=PANEL_SPINBUTTONDLG_CANCEL.CANCEL
:p.This button closes the dialog.

.*--------------------------------------------------------------*\
.*  Spinbutton Demo Dlg Help button
.*      res = PANEL_SPINBUTTONDLG_HELP
.*--------------------------------------------------------------*/
:h1 res=15150 name=PANEL_SPINBUTTONDLG_HELP.HELP
:p.This button calls the help screen for the selected item.

.*--------------------------------------------------------------*\
.*  Help for the unknown dialog
.*      res = PANEL_UNKNOWNDLG
.*--------------------------------------------------------------*/
:h1 res=20100 name=PANEL_UNKNOWNDLG.Unknown Dialog Box
:p.This panel is displayed for any dialog box that does not have
a help panel created for it.  Make sure you have a help panel
created for this dialog.

.*--------------------------------------------------------------*\
.*  Help panel for an unknown dialog item
.*      res = PANEL_UNKNOWN
.*--------------------------------------------------------------*/
:h1 res=20110 name=PANEL_UNKNOWN.Unknown Dialog Item
:p.This panel is displayed for any dialog box item that does not
have a help panel created for it.  Make sure that you have a help
panel created for all dialog box items.
