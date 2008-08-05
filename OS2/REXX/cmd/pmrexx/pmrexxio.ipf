.*
.* PMREXXIO.IPF
.*
.* This file contains the help text for PMREXX.
.*
.* 11/14/94 -- Completed major revisions for 32-bit version
.*             done in Endicott.  Mike Baryla
.*
:userdoc.
:prolog.
:title.
:tline.PMREXX HELP
:etitle.
:eprolog.
:body.
.********************************************************************
.* General Help from the help pull-down menu on PMREXX action bar.
.* The general help topic is "Help for PMREXX."  It links several
.* other topics of general interest.
.********************************************************************
:h1 res=01.Help for PMREXX
:i1.PMREXX
:i1.About PMREXX
:p.Use PMREXX to run and debug REXX procedures.
:p.
PMREXX is a windowed
application that runs a REXX procedure.
Any results displayed by the REXX procedure
are displayed in the PMREXX window.
You
can use the scroll bar to view any previous output that
has not been cleared.
:p.PMREXX also provides a single line input field so that you can supply
input to the REXX procedure or to any commands called by the REXX procedure.
For more information about using REXX procedures, see :hp1.REXX
Information:ehp1..
:p.
For more information about starting and using PMREXX, select these topics:
:ul compact.
:li.:link reftype=hd RES=38 group=40.Using PMREXX:elink.
:li.:link reftype=hd RES=40 group=40.Developing Procedures with PMREXX:elink.
:li.:link reftype=hd RES=39 group=40.Tracing REXX Procedures:elink.
:eul.
.*
.* Additional topic that is linked by the "Help for PMREXX" topic
.*
:h1 res=38.Using PMREXX
:i1.using PMREXX
:i1.starting PMREXX
:i1.arguments, specifying
:i1.specifying arguments
:p.
To start PMREXX, enter PMREXX from an OS/2 command prompt followed
by the name of a REXX procedure to run:
:xmp.
pmrexx myprog
:exmp.
:p.
You can also supply arguments for the REXX procedure.
In this example, PMREXX runs the procedure MYPROG and
passes MYPROG the argument c&colon.\test\test.data&colon.
:xmp.
pmrexx myprog c&colon.\test\test.data
:exmp.
:p.
PMREXX runs the REXX procedure and displays any output generated
by it in a scrollable output box that is part of the
PMREXX window.
If the procedure prompts you for input, the prompt
is displayed in the output box, but you type the response
in the smaller input box near the top of the PMREXX window.
:p.
To try it, start another OS/2 session and create this REXX procedure:
:xmp.
/* DIRTEST.CMD -- displays directory contents */
do forever
   say 'Enter the name of a directory'
   parse upper pull response
   if response='QUIT' then leave
   'dir' response
end
exit
:exmp.
:p.
Then start PMREXX again for the DIRTEST procedure:
:xmp.
pmrexx dirtest
:exmp.
:p.
When the PMREXX window is displayed, you will see the prompt in
the output box.
Position the cursor in the input box, type the name of a
directory, and press Enter.
The contents of the directory will be listed in the output
box, and another prompt will be displayed.
:p.
When amount of displayed output exceeds the size of the output
box, a slider box appears in the scroll bar.
:p.
To end the DIRTEST program, enter QUIT in the input box.
:p.
.*
.* Additional topic that is linked by the "Help for PMREXX" topic
.*
:h1 res=40.Developing Procedures with PMREXX
:i1.developing procedures with PMREXX
:i1.REXX procedures, developing
:i1.restarting a REXX procedure
:i1.REXX procedures, restarting
:p.
To use PMREXX as a miniature development environment for your
procedures, run PMREXX in one OS/2 session and an editor in another
session.
:p.
Use the editor to change and save the procedure being developed.
Then switch to the PMREXX session to run the modified procedure.
:p.
You can restart a procedure from PMREXX by
selecting :hp2.Trace:ehp2. from the action bar in the PMREXX window.
If the procedure is running, select :hp2.Halt procedure:ehp2. to stop
it,
then
select :hp2.Restart:ehp2. from the menu.
Otherwise, just select :hp2.Restart:ehp2..
The latest version of the procedure will run again.
.*
.* Additional topic that is linked by the "Help for PMREXX" topic
.*
:h1 res=39.Tracing REXX Procedures
:p.
:i1.tracing REXX procedures
:i1.debugging REXX procedures
:i1.REXX procedures, debugging
:i1.REXX procedures, tracing
:i1.interactive trace
PMREXX includes several functions that are useful for debugging
your procedures.
You can, for instance, start an interactive trace from PMREXX without
having to add a TRACE instruction to your procedure.
Select :hp2.Trace:ehp2. on the action bar, and then select
:hp2.Interactive trace on:ehp2..
A check mark on the menu shows that interactive tracing is on.
To stop the interactive trace, just select :hp2.Interactive trace on again:ehp2..
One advantage of using the interactive trace from PMREXX is that you can turn
the trace on and off while the program is running.
:p.
Once tracing is active, you can step through your procedure one clause at
at time, re-do a clause that was just processed, or enter a line of REXX
clauses for immediate processing.
The ability to enter REXX clauses is especially useful if you want to
try a fix to a problem interactively or if you want to test
instruction paths that are otherwise difficult to trigger.
For example, you might want to test some error handling instructions,
but cannot easily create the condition that would cause the error.
By using the interactive trace, you can add REXX instructions
at the right moment to fake the conditions that would cause the error
handling instructions to be processed.
:p.
To process a line of one or more REXX clauses, type the line in the
PMREXX input box when tracing is active and press ENTER.
For example, you could enter:
:xmp.
do i=1 to 10; say 'hello' i; end
:exmp.
:p.
The line is processed before the next REXX clause in the program is processed.
If you simply want to step ahead to the next clause, press Enter
without typing anything in the input area.
You can also step ahead by selecting :hp2.Trace next clause:ehp2.
from the :hp2.Trace:ehp2. menu.
If you want to process the last REXX clause again,
select
:hp2.Re-do last clause:ehp2. from the :hp2.Trace:ehp2.menu.
:p.
To stop tracing, select
:hp2.Trace off:ehp2. from the :hp2.Trace:ehp2..
This item is not selectable
when the REXX procedure is waiting for user input.
In this case select :hp2.Interactive trace on:ehp2. again to stop the
trace.
:p.
.********************************************************************
.*
.* End of general Help topics.
.*
.********************************************************************
.*
.* Start of Help topics for action bar choices.  Each action bar
.* choice is followed by the helps for the pull-down menus
.*
.*********************************************************************
.* Help for File       (Action Bar)
 
:h1 res=02 nosearch.Help for File
:p.Use the choices on the :hp2.File:ehp2. pull-down to save the contents
of the output window, to set properties of the output window, and to exit PMREXX.
.********************************************************************
 
.*Help for Save  (PD)
:h1 res=03 nosearch.Help for Save
:i1.saving the output window
:i1.output window, saving
:p.Use :hp2.Save:ehp2. on the :hp2.File:ehp2. pull down menu to save the text displayed in the PMREXX
window.
If you have previously saved the output window, the text
is written to that same file.
Otherwise, the :hp2.Save as:ehp2. dialog box is displayed
so that you can select a file.
.***********************************************************************
 
.*Help for Save as    (PD)
 
:h1 res=04 nosearch.Help for Save As
:p.Use :hp2.Save as:ehp2. to name and save a new file or to save an existing
file under a different name, in a different directory, or on a different disk.
When you choose :hp2.Save as:ehp2., a pop-up is displayed.
 
.* Help for Properties (PD)
 
:h1 res=35.Help for Properties
:i1.properties, setting
:p.Use :hp2.Properties:ehp2. to set PMREXX fonts and to
indicate whether output from REXX procedures should be appended to any existing
text in the output window.
 
.*Help for font
:h1 res=18.Help for Font
:p.Use :hp2.Font:ehp2.
to display the text in the output window in a different font.  When you
choose :hp2.Font:ehp2., a pop-up is displayed.
.*
 
:h1 res=36.Help for Append Results
:p.Use :hp2.Append Results:ehp2.
if you want the results of REXX procedures
to be appended to the output window.
If :hp2.Append Results:ehp2. is not selected,
the output window is cleared each time you run the REXX procedure.
.**********************************************************************
.*Help for Exit   (PD)
:h1 res=09 nosearch.Help for Exit
:p.Use :hp2.Exit:ehp2.
to close PMREXX.
.*******************************************************************
 
.*Help for Edit  (AB)
:h1 res=10 nosearch.Help for Edit
:p.Use the choices on the :hp2.Edit:ehp2. pull-down to cut, copy, paste, delete, or search
the contents of the output window.
.*************************************************************************
 
:h1 res=32.Help for Cut
:p.Use :hp2.Cut:ehp2.
to move or delete
:link reftype=hd RES=4864 group=41 dependent.selected text:elink. from the
output window.  A
copy of the text is moved to the
:link reftype=hd RES=4752 group=40 dependent.Clipboard:elink..
:p.You can use the :hp2.Paste:ehp2. choice to insert the text from
the Clipboard into the input window.
:p.To select or mark text&colon.
:ol compact.
:li.Position the mouse pointer to the left of the first character
to be selected.
:li.Hold down mouse button 1; then use the mouse to drag (move)
the cursor to the right of the last character to be selected.
:li.Release mouse button 1.
:eol.
:p.When the cursor comes to a window border while you are marking text,
PMREXX scrolls past the border.
This makes it possible to select more text than can be displayed in the window.
:p.
To deselect text or remove the highlighting, click once within the text
area of the window.
.*Help for Copy
:h1 res=11 nosearch.Help for Copy
:p.Use :hp2.Copy:ehp2.
to duplicate
:link reftype=hd RES=4864 group=41 dependent.selected text:elink. from the
output window.  The
text remains in the output window, but a copy of the text is moved to the
:link reftype=hd RES=4752 group=40 dependent.Clipboard:elink..
:p.You can use the :hp2.Paste:ehp2. choice to insert the text from
the Clipboard into the input window.
:p.To select or mark text&colon.
:ol compact.
:li.Position the mouse pointer to the left of the first character
to be selected.
:li.Hold down mouse button 1; then use the mouse to drag (move)
the cursor to the right of the last character to be selected.
:li.Release mouse button 1.
:eol.
:p.When the cursor comes to a window border while you are marking text,
PMREXX scrolls past the border.
This makes it possible to select more text than can be displayed in the window.
:p.
To deselect text or remove the highlighting, click once within the text
area of the window.
.*************************************************************************
 
.*Definition Selected Text
:h1 hide res=4864 x=25% y=20% width=70% height=50% scroll=vertical rules=sizeborder titlebar=sysmenu group=41 id=A4864.Selected Text&colon.definition
:i1.Selected text, definition
:p.To select text means to highlight the text that you want to delete
or move.  You delete or move selected text by using the :hp2.Cut:ehp2., :hp2.Delete:ehp2.,
:hp2.Copy:ehp2., or :hp2.Paste:ehp2. choices on the
:hp2.Edit:ehp2. pull-down.
.************************************************************************
 
 
.*Definition Clipboard
:h1 hide res=4752 x=25% y=20% width=70% height=50% scroll=vertical rules=sizeborder titlebar=sysmenu group=40 id=A4752.Clipboard&colon.definition
.*:i1.Clipboard, definition
:p.
A clipboard is an area of storage that can temporarily hold data
that is available to other programs.
.*************************************************************************
 
 
:h1 res=12 nosearch.Help for Paste
:p.Use :hp2.Paste:ehp2.
to insert the text from
the :link reftype=hd RES=4752 group=40 dependent.Clipboard:elink.
into the input window.  Press Enter to pass the text you have pasted in
the input window into the REXX program.
:p.If text is :link reftype=hd res=4864 group=41 dependent.selected:elink.,
the text from the Clipboard will replace the selected text.
:nt.Use
:hp2.Copy:ehp2. or :hp2.Cut:ehp2. to put text on the
Clipboard.
:ent.
.*************************************************************************
 
.*Help for Delete
:h1 res=13 nosearch.Help for Delete
:p.Use :hp2.Delete:ehp2.
to delete
:link reftype=hd RES=4864 dependent.selected text:elink.
from the output window.
:p.To delete selected text&colon.
:ol compact.
:li.Select the text to be deleted.
:li.Select :hp2.Edit:ehp2..
:li.Select :hp2.Delete:ehp2..
:eol.
.**********************************************************************
 
 
.*Help for Select All (PD)
:h1 res=14 nosearch.Help for Select All
:p.Use :hp2.Select all:ehp2.
to
:link reftype=hd RES=4864.select:elink.
all of the text in the output window.  You can then perform some action on this
text.  (For example, you could copy all the text to the clipboard.)
.**************************************************************************
:h1 res=34.Help for Deselect All
:p.Use :hp2.Deselect All:ehp2. to deselect all selected
text in the output window.
 
:h1 res=37.Help for Search
:p.Use :hp2.Search:ehp2. to search for a string in the PMREXX output window.
 
:h1 res=24.Help for Trace
:p.Use the choices on the :hp2.Trace:ehp2. pull-down menu to set tracing on or off and to
halt and restart REXX procedures.
 
.*Help for Restart
:h1 res=16 nosearch.Help for Restart
:p.Use :hp2.Restart:ehp2. to start a PMREXX procedure again.
.**********************************************************************
 
.*Help for Interactive trace on
:h1 res=17.Help for Interactive trace on
:p.
Use :hp2.Interactive trace on:ehp2. to start an interactive trace.
Interactive Trace is used for debugging your program.
.***********************************************************************
 
 
 
.*Help for Halt Procedure
:h1 res=25 nosearch.Help for Halt Procedure
:p.Use :hp2.Halt procedure:ehp2. to stop a PMREXX program.
.************************************************************************
 
 
.*Help for Trace next clause
:h1 res=26 nosearch.Help for Trace next clause
:p.
Use :hp2.Trace next clause:ehp2. to step to the next clause in the PMREXX
procedure.  Using this choice is the same as entering a
blank line in the input field.
.**********************************************************************
 
 
.*Help for Re-do last clause
:h1 res=27 nosearch.Help for Re-do last clause
:p.
Use :hp2.Re-do last clause:ehp2. to repeat the last PMREXX clause procedure.
Using this choice is the same as entering an
equal sign (=) in the input field.
.************************************************************************
 
 
.*Help for Trace off
:h1 res=28 nosearch.Help for Trace off
:p.
Use :hp2.Trace off:ehp2. to turn off the debugging procedure in your
program session.
.*********************************************************************
.* Help for Dialog Boxes
.* -- font selection
.*********************************************************************
.*
.* Help for Save As Dialog box (D)
.*
:h1 hide res=56 scroll=none titlebar=yes group=100 id=A9004.Help for Save as
:link reftype=hd res=26500 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26600 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26500 nosearch noprint.Help for Save as
:ol compact.
:li.Select the down arrow to the right of the :hp2.Drive:ehp2. list to display
all the drives on your system.
:li.Select a drive from the :hp2.Drive:ehp2. list.
:li.Select a directory from the :hp2.Directory:ehp2. list.
:li.Type the name of the file in which you want to save the contents of
the output window in the :hp2.Save as file name:ehp2. field
and select the :hp2.Ok:ehp2. pushbutton.
:eol.
:p.For specific help, select a topic below.
 
:h1 hide res=26600 nosearch noprint.List of Fields
:dl compact tsize=23.
:dt.:link reftype=hd res=58 group=200.Save as filename:elink.
:dd.:link reftype=hd res=60 group=210.Save file as type:elink.
:dt.:link reftype=hd res=59 group=220.Drive:elink.
:dd.:link reftype=hd res=62 group=230.File:elink.
:dt.:link reftype=hd res=61 group=240.Directory:elink.
:dd.:link reftype=hd res=57 group=250.Ok pushbutton:elink.
:dt.:link reftype=hd res=55 group=251.Cancel pushbutton:elink.
:dd.
:edl.
.*
.* Help for "Save as file name" field on Save As dialog box
.*
:h1 hide res=58 scroll=none titlebar=yes group=260 nosearch noprint.Help for Save as filename
:link reftype=hd res=26550 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26550 nosearch.Help for Save as filename
:p.Type the name of the file in which you want to place the contents
of the output window in the
:hp2.Save as filename:ehp2. field
and select the :hp2.Ok:ehp2. pushbutton.
:p.For more help, select a topic below.
 
:h1 hide res=26650 nosearch noprint.List of Fields
:dl compact tsize=23.
:dt.:link reftype=hd res=58 group=270.Save as filename:elink.
:dd.:link reftype=hd res=60 group=280.Save file as type:elink.
:dt.:link reftype=hd res=59 group=290.Drive:elink.
:dd.:link reftype=hd res=62 group=300.File:elink.
:dt.:link reftype=hd res=61 group=310.Directory:elink.
:dd.:link reftype=hd res=57 group=320.Ok pushbutton:elink.
:dt.:link reftype=hd res=55 group=321.Cancel pushbutton:elink.
:dd.:link reftype=hd res=56  group=330.General help:elink.
:edl.
.************************************************************************
 
.* Help for "Save file as type" field in Save As dialog box
 
:h1 hide res=60 scroll=none titlebar=yes group=380 nosearch noprint.Help for Save file as type
:link reftype=hd res=26552 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26552 group=390 nosearch.Help for Save file as type
:p.Select the down arrow to the right of the :hp2.Save file as type:ehp2. field to
display the available file types.  PMREXX is set for all file types.
:p.For more help, select a topic below.
.***********************************************************************
 
.* Help for Drive field on the Save As dialog box
 
:h1 hide res=59 scroll=none titlebar=yes group=110 nosearch noprint.Help for Drive
:link reftype=hd res=26553 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26553 group=400 nosearch.Help for Drive
:p.The :hp2.Drive:ehp2. list displays the drives on your system.  Select the
drive that contains the file in which you want to place the contents
of the output window.
:p.For more help, select a topic below.
.***************************************************************************
 
.* Help for File field on the Save As dialog box
 
:h1 hide res=62 scroll=none titlebar=yes group=120 nosearch noprint.Help for File
:link reftype=hd res=26554 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26554 group=420 nosearch.Help for File
:p.The :hp2.File:ehp2. list displays all the files in the directory
you selected from the :hp2.Directory:ehp2. list.  Select the file in which
you want to place the contents of the output window.
:p.For more help, select a topic below.
.***************************************************************************
 
.* Help for Directory field on the Save As dialog box
 
:h1 hide res=61 scroll=none titlebar=yes group=11 nosearch noprint.Help for Directory
:link reftype=hd res=26555 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26555 group=430 nosearch.Help for Directory
:p.The :hp2.Directory:ehp2. list displays the directories on the selected
drive.  Select a directory to display the list of files from that directory
in the :hp2.File:ehp2. list box.
:p.For more help, select a topic below.
.*************************************************************************
 
.* Help for Ok pushbutton on the Save As dialog box
 
:h1 hide res=57 scroll=none titlebar=yes group=170 nosearch noprint.Help for Ok
:link reftype=hd res=26556 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
:h1 hide res=26556 group=440 nosearch.Help for Ok
:p.Select the :hp2.Ok:ehp2. pushbutton to save the contents of the output window
to the file, drive, and directory you selected and with the file name
you specified.
:p.For more help, select a topic below.
.********************************************************************
 
:h1 hide res=55 scroll=none titlebar=yes group=110 nosearch noprint.Help for Cancel
:link reftype=hd res=26557 vpx=left vpy=top vpcx=100% vpcy=70% scroll=vertical rules=border titlebar=none child auto.
:link reftype=hd res=26650 vpx=left vpy=bottom vpcx=100% vpcy=30% scroll=vertical rules=none titlebar=none child auto.
 
.* Help for Cancel push button on the Save As dialog box
.*
:h1 res=26557.Help for Cancel
:p.
Select :hp2.Cancel:ehp2. to close the dialog box without
saving the contents of PMREXX output window in a file.
 
.********************************************************************
.*
.* Help for PMREXX Font Selection dialog box
.*
:h1 res=400.Help for PMREXX Font Selection
:p.
Use the PMREXX Font Selection dialog box to change the font PMREXX uses for displaying
or printing text.
To change fonts:
:ol compact.
:li.Select the device for which you are changing the font:
:hp2.Display:ehp2. or :hp2.Printer:ehp2..
:li.Select the font :hp2.Name:ehp2..
:li.Select the font :hp2.Style:ehp2..
:li.Select the font :hp2.Size:ehp2..
:li.Select any desired attributes under :hp2.Emphasis:ehp2..
:li.Select :hp2.Ok:ehp2..
:eol.
:p.
As you make selections, :hp2.Sample:ehp2. will change to
show you what the font looks like.
:p.
For a detailed explanation of each field, select from the list below:
:ul compact.
:li.:link reftype=hd RES=404 group=40.Name:elink.
:li.:link reftype=hd RES=405 group=40.Style:elink.
:li.:link reftype=hd RES=406 group=40.Size:elink.
:li.:link reftype=hd RES=407 group=40.Outline:elink.
:li.:link reftype=hd RES=408 group=40.Underline:elink.
:li.:link reftype=hd RES=409 group=40.Strikeout:elink.
:li.:link reftype=hd RES=401 group=40.Cancel:elink. push button.
:eul.
.*
.* Help for cancel button on the font selection dialog
.*
:h1 res=401.Help for Cancel
:p.
When :hp2.Cancel:ehp2. is selected,
the dialog box is closed.
No changes are made to the fonts.
.*
.* Help for OK button on the font selection dialog
.*
:h1 res=402.Help for Ok
:p.
When :hp2.Ok:ehp2. is selected, the
fonts are changed according to your selections.
.*
.* Help for Name field in the font selection dialog
.*
:h1 res=404.Help for Name
:p.
To select a font name, point to the
arrow to the right of the :hp2.Name:ehp2. field and click mouse button 1.
Then select the desired font from the displayed list.
.*
.* Help for Style field on the font selection dialog
.*
:h1 res=405.Help for Style
:p.
To select a font style, point to the
arrow to the right of the :hp2.Style:ehp2. field and click mouse button 1.
Then select the desired style from the displayed list.
.*
.* Help for the size field on the font selection dialog
.*
:h1 res=406.Help for Size
:p.
To select a font size, point to the
arrow to the right of the :hp2.Size:ehp2. field and click mouse button 1.
Then select the desired size from the displayed list.
.*
.* Help for the Outline check box on the font selection dialog
.*
:h1 res=407.Help for Outline
:p.
Select :hp2.Outline:ehp2. if you want only the outline
of the characters in the font to be displayed.
.*
.* Help for the Underline check box on the font selection dialog
.*
:h1 res=408.Help for Underline
:p.
Select :hp2.Underline:ehp2. if you want all
characters to be underlined.
.*
.* Help for the Strikeout check box on the font selection dialog
.*
:h1 res=409.Help for Strikeout
:p.
Select :hp2.Strikeout:ehp2. if you want a horizontal line
to be drawn through all characters.
.*
:h1 res=410.Help for Display
:p.Select the :hp2.Display:ehp2. check box if you
want to change the font used for the output window display.
:h1 res=411.Help for Print
:p.Select the :hp2.Print:ehp2. check box if you want to
change the font used when the output window is printed.
.********************************************************************
.*
.* Help for Search dialog
.*
.********************************************************************
.* I think that 604 is never called and that 601 is used instead
:h1 res=604.Help for Search
:p.Use the Search dialog to find text
in the PMREXX output window.
:h1 res=603.Help for Case Sensitive
:p.Select the :hp2.Case sensitive:ehp2. check box if you
want PMREXX to consider capital and small letters in its search.
.*
:h1 res=602.Help for Enter search text
:p.Type the string of text to be found in the :hp2.Enter search text:ehp2. field.
.*
:h1 res=601.Help for Search 
:p.Use the :hp2.Search:ehp2. dialog box to search
for strings of text in the output window:
:ol.
:li.Type the text to be found in the :hp2.Enter search text field:ehp2..
:li.Select the :hp2.Case sensitive:ehp2. check box if you want the
capital and small letters to be significant.
:li.Select :hp2.Ok:ehp2..
:eol.
.*
.*
:h1 res=605.Help for Ok
:p.Select the :hp2.Ok:ehp2. push button to start the search.
.*
:h1 res=606.Help for Cancel
:p.Select the :hp2.Cancel:ehp2. push button to close the :hp2.Search:ehp2. dialog
box without searching for text.
.********************************************************************
.*
.* THE HELP KEY DEFINITIONS
.*
.********************************************************************
.****** HID_KEYS_HELP_PANEL
:h1 res=9001 group=250.Help for Key Assignments
:p.
Select the key group you want help for:
:ul.
:li.
:link reftype=hd res=6375 group=251.HELP KEYS:elink. to get help
and manage help information
:li.
:link reftype=hd res=6376 group=252.SYSTEM KEYS:elink. to switch
among open programs, groups, and online documents
:li.
.*:link reftype=hd res=6377 group=253.PROGRAM WINDOW KEYS:elink.
:link reftype=hd res=6377 group=253.WINDOW MOVEMENT KEYS:elink.
to move around in a program, group, or help window, or to move
or size a window
:li.
:link reftype=hd res=6378 group=254.POP-UP KEYS:elink.
to move around in or complete a pop-up.
:eul.
:p.
When two key names are joined by a plus sign (+), use these two keys
together.  Hold down the first key and press the second key.
 
:h1 hide res=6375 group=251 id=A6375 nosearch.Help for Help Keys
:p.
From any active window:
:dl tsize=5 break=all compact.
:dt.:hp2.
Alt+F6:ehp2.
:dd.
Switch between the help window and the associated program window
:dt.:hp2.
F1:ehp2.
:dd.
Get help for the active area
:edl.
:p.
From any active window that has a :hp2.Help:ehp2. action bar choice:
:dl tsize=5 break=all compact.
:dt.:hp2.
F2:ehp2.
:dd.
Display general information about the program
:dt.:hp2.
F9:ehp2.
:dd.
Display a list of keys for the program
:dt.:hp2.
F11 or Shift+F1:ehp2.
:dd.
Display the help index for the program
:dt.:hp2.
Shift+F10:ehp2.
:dd.
Get help about the help facility
:edl.
:p.
From any active help window:
:dl tsize=5 break=all compact.
:dt.:hp2.
Alt+F4:ehp2.
:dd.
Close the help window
:dt.:hp2.
Esc:ehp2.
:dd.
Redisplay the previous help
.*Following added per Mark Benge 6/8am -- there will be PTR.
.*topic
topic, if there is one; if not, remove the help window
.*End change
:dt.:hp2.
Ctrl+A:ehp2.
:dd.
Copy the displayed topic and add it to the end of a temporary file
:dt.:hp2.
Ctrl+C:ehp2.
:dd.
Display the help table of contents
:dt.:hp2.
Ctrl+F:ehp2.
:dd.
Copy the displayed topic to a temporary file
:dt.:hp2.
Ctrl+H:ehp2.
:dd.
Display a list of all the topics you have viewed
:dt.:hp2.
Ctrl+L:ehp2.
:dd.
Display a list of libraries that contain help topics for one or
more programs
:dt.:hp2.
Ctrl+N:ehp2.
:dd.
Open a new help window to display any topic
.*
.*you can select from a list
.*of topics or index entries, or sometimes for topics related to words or
.*phrases that have special highlighting
that is displayed in a window where the minimize and maximize choices
in the System Menu pull-down are displayed and available
.*
:dt.:hp2.
Ctrl+S:ehp2.
:dd.
Search for a word or phrase in help topics
:dt.:hp2.
Ctrl+Ins:ehp2.
:dd.
Copy the displayed topic to the clipboard
:edl.
:p.
From any active Contents window that has multiple levels of help topics:
:dl tsize=5 break=all compact.
:dt.:hp2.
Ctrl+* (Asterisk):ehp2.
:dd.
Display all levels of the table of contents
:dt.:hp2.
Ctrl+- (Minus):ehp2.
:dd.
Remove all subtopics from the table of contents
:dt.:hp2.
+ (Plus):ehp2.
:dd.
Display one more level of topics in the table of contents
:dt.:hp2.
* (Asterisk):ehp2.
:dd.
Display all subtopics for one topic in the table of contents
:dt.:hp2.
- (Minus):ehp2.
:dd.
Remove all subtopics from one topic in the table of contents
:dt.:hp2.
Tab:ehp2.
:dd.
Move to the next word or phrase that has special highlighting
:edl.
 
:h1 hide res=6376 group=252 id=A6376 nosearch.Help for System Keys
:dl tsize=5 break=all compact.
:dt.:hp2.
Alt+Esc or Alt+Tab:ehp2.
:dd.
Switch to the next program (windowed or full-screen), group,
or online document
:dt.:hp2.
Ctrl+Esc:ehp2.
:dd.
Switch to the Task List
:edl.
 
:h1 hide res=6377 group=253 id=A6377 nosearch.Help for Window Movement Keys
:dl tsize=5 break=all compact.
:dt.:hp2.
F3:ehp2.
:dd.
Close the active program window
:dt.:hp2.
F10 or Alt:ehp2.
:dd.
Switch to or from the action bar
:dt.:hp2.
Arrow keys:ehp2.
:dd.
Move among choices
:dt.:hp2.
End:ehp2.
:dd.
Move to the last choice in a pull-down
:dt.:hp2.
Esc:ehp2.
:dd.
Remove an action bar pull-down or the System Menu pull-down but keep
the action bar choice or the System Menu icon selected
:dt.:hp2.
First letter:ehp2.
:dd.
Select from a list the next choice that starts with the
.*typed letter
letter you type
:dt.:hp2.
Home:ehp2.
:dd.
Move to the first choice in a pull-down
:dt.:hp2.
PgUp:ehp2.
:dd.
Scroll the contents of a window up one page
:dt.:hp2.
PgDn:ehp2.
:dd.
Scroll the contents of a window down one page
:dt.:hp2.
Underlined letter:ehp2.
:dd.
Select a choice on the action bar or a pull-down
:dt.:hp2.
Alt+F4:ehp2.
:dd.
Close the active program window
:dt.:hp2.
Alt+F5:ehp2.
:dd.
Restore the active program window
:dt.:hp2.
Alt+F7:ehp2.
:dd.
Move the active program window
:dt.:hp2.
Alt+F8:ehp2.
:dd.
Size the active program window
:dt.:hp2.
Alt+F9:ehp2.
:dd.
Minimize the active program window
:dt.:hp2.
Alt+F10:ehp2.
:dd.
Maximize the active program window
:dt.:hp2.
Ctrl+PgDn:ehp2.
:dd.
Scroll the contents of a window right one page
:dt.:hp2.
Ctrl+PgUp:ehp2.
:dd.
Scroll the contents of a window left one page
:dt.:hp2.
Shift+Esc or Alt+Spacebar:ehp2.
:dd.
Switch to or from the System Menu pull-down
:dt.
:hp2.Shift+Esc or Alt:ehp2.
:dd.
Switch to or from the System Menu pull-down of an OS/2 or a DOS window
:edl.
 
:h1 hide res=6378 group=254 id=A6378 nosearch.Help for Pop-Up Keys
:dl tsize=5 break=all compact.
:dt.:hp2.
Alt+F6:ehp2.
:dd.
Switch between the pop-up and the help window
:dt.:hp2.
Enter:ehp2.
:dd.
Send any changes to the program
:dt.:hp2.
Esc:ehp2.
:dd.
Remove the pop-up without sending any changes to the program
:dt.:hp2.
F1:ehp2.
:dd.
Get help for the active area
:dt.:hp2.
Spacebar:ehp2.
:dd.
Select a check box choice
or perform the task described in the selected pushbutton
:dt.:hp2.
Tab:ehp2.
:dd.
Move to the next field (entry field, check box, list, spin button,
first radio button, or first pushbutton)
:edl.
:p.
In an entry field:
:dl tsize=5 break=all compact.
:dt.:hp2.
Backspace
:ehp2.
:dd.
Delete the character to the left of the cursor
:dt.:hp2.
Del:ehp2.
:dd.
Delete the character to the right of the cursor
:dt.:hp2.
End or Ctrl+Right Arrow key:ehp2.
:dd.
Move to the end of a field
:dt.:hp2.
Home or Ctrl+Left Arrow key:ehp2.
:dd.
Move to the beginning of a field
:edl.
 
:euserdoc.
