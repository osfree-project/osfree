=============================
AE - Text Editor to replace E
=============================

AE is a simple text editor, designed to replace IBM's original OS/2 E.EXE. It mostly behaves the same, except for where behaviour has become annoying, irrelevant or inconsistent with other systems/programs.


License
-------

AE is open source software under the GNU GPL License.

Include credit to the original author:
Aaron Lawrence (aaronl at consultant dot com)

AE remains Copyright 2004 Aaron Lawrence.


Features
--------

Fixes these Stupid Things About E:
- "Type of file" dialog.
- EOF byte (Hex 1A, Dec 26) added at end of files
- Shift + Page up/down does not select
- Ctrl + Page up/down does something useless
- Refusal to save file if it thinks it isn't changed.
- Requirement for Extended Attribute support when saving
- No File - Exit
- Find combined with replace
- Word wrap switch is clumsy
- No shortcuts for file open etc
- No display of line, column (however, see limitations) or mode
- Task list entry does not show filename
- Vertical scrollbar blank space at end
- Silly horizontal scrollbar when wrapping
- Unix-format files converted to DOS format when saved

Enhancements (intended to be minimal!):
- Timed autosave as well as "number of changes"
- Print
- If file is read-only, text is (initially) read-only

Install
-------

You probably want to do this:

put ae.exe into x:\os2
put ae.hlp into x:\os2\help

or perhaps, on eCS

put ae.exe into x:\ecs\bin
put ae.hlp into x:\ecs\help

If you want to replace e.exe (yes, you do :) then do this:

COPY e.exe to olde.exe to make a backup.
Do not rename it, or OS/2 will detect and change desktop icons to use the backup!
Then copy ae.exe to e.exe.
Don't rename the help file, leave it as ae.hlp.


Known Problems
--------------

This is beta software. I recommend caution! May set fire to your house and kill your family without notice.

*** WARNING ***
If you intend to use AE for editing large or important documents, then test it thoroughly before you start. Nobody has done such testing yet. I have tested it with various different sizes of file, but not exhaustively. If you do thorough testing then let me know and I will remove this warning.

AE does NOT backup your previous file (yet).
***************

DragText integration: it works, but you must use the "re-enable" feature in DragText.
 - open DT's Options notebook to Setup->Reenable
 - enter AE.EXE in the 'Program' field
 - check both 'Drag' and 'Drop' for these window types:
     Multi-line EFs, Entry Fields, and List Boxes
 - press the 'Add' button

Limitations

AE uses the standard OS/2 "multi-line edit control" (MLE), and inherits most of it's limitations:
- Line/column number display doesn't work when wrapping.
- Cut/copy/paste is limited to 64kB as with old E.
- Undo information is discarded when color or wrapping is changed
- Scrolling doesn't update until button released
- Cannot search backwards
A few of the old problems have been worked around indirectly, but this can't be done for everything. To fix all these issues needs a new edit control, which is not something I personally plan to do.

If you get a "usage" screen when launching AE from the desktop, the WPS may be adding the folder name to the parameters. Try adding a % character to the end of the parameters e.g.
Parameters:    c:\config.sys %
This will prevent the folder name from being added.

Thanks
------

Dieter Wachsmuth, Peter Koller, Veit Kannegieser, Rich Walsh, Michal Pohorelsky, Jan Magne Landsvik, Guillame Gay, Gabriele Gamba, Simon Gronlund, Salvador Parra Camacho, D.J. van Enckevort, Frank Vos, Kris Steenhaut, Paul Ratcliffe, Yuri Proniakin, Bart van Leeuwen, Jacques van Leeuwen, Alberto Gabrielli, Peter Weilbacher, Stan Goodman


History
-------

1.9.8
  Fix for repeated DBCS characters 
   Thanks to "Average" & Teijo Kakkinen from Japan 
  Added help section on supported file formats.
  Handles Unix-format text files (LF only).
  AE can be killed without seeing an error message.
  Status bar and dialogs scale to suit system fonts.
  Status messages cleared when file edited.
  Status shown as "new" until first save.
  Updated translations from:
   Guillaume Gay (French), Jacques van Leeuwen (Dutch), 
   Bj”rn S”derstr”m (Swedish), 
   Dieter Wachsmuth and the Dresden OS/2 User Group (German)

1.9.2 [Beta test]
  Change current code page to anything supported by system.
    This feature has been withdrawn since it cannot be supported properly.
  Thanks to Veit Kannegieser and Rich Walsh for info on CPs.
  Large files warning dialog is translateable.
  Focus goes back to find window when no match found.
  Undo doesn't change color back to white on startup!
  Buttons on save/discard/cancel dialog enlarged for translation

1.8.5
  Keep existing extended attributes when saving (e.g custom icons)
  
1.8.3
  Find/Replace dialogs now non-modal like old E.
  First time position is screen center.
  Own help file can now be language-specific, as for interface.
  Also look in %OSDIR%\lang and %ULSPATH% for language files.

1.6.10
  Fix save dialog to show existing files.
  Replace "from top" option works.
  Optional warning for opening large files.
  Hourglass cursor while loading.
  Space for translators to put a message 
  (e.g. name) in the Product Information Dialog
  Michal Pohorelsky - Czech
  Jan Magne Landsvik - Norwegian
  Updated French from Guillame Gay
  Keyboard handling improvements in dialogs etc.

1.6.5
  Set font and colors from within program
  Fixed some errors loading translations
  Some updates to translations

1.5.5
  Allow tabs to be entered
  Context menu for editing
  Updated help

1.5.1
  NOTE! Translation files are now named ae_<language>.lng
  Added translations, thanks everyone
    Guillame Gay (French)
    Gabriele Gamba (Italian)
    Simon Gronlund (Swedish)
    Salvador Parra Camacho (Spanish)
    Dieter Wachsmuth (German)
    D.J. van Enckevort (Dutch)
      (also Frank Vos and Kris Steenhaut)     
  Fixed bug saving files > 64kB
  Fixed specifying new file from command line
  Display status changes (eg. clear modified after save)
  Normal EXE size!

1.4.4
  Fixed page up/down
  Implemented /read command line parameter 
  Find own help file in application directory

1.4.2
  Multi-lingual support like NewView (see bottom of file)
  Fixed saving of word wrap setting

1.3.0
  Fixed saving files with no extension - don't add ..txt
  Option to print selection
  Fixed long printer list
  Drop file on window to open

1.2.3
  Removed 64kB limitations (truncation of file!)
  Autosave
  Internal restructure for better performance with large files
  Added "From top" option to find and replace
  Show "modified" indicator
  Disable cut/paste/replace if read-only
  Extra keys
    F2 = Save (like EPM)
    F3 = Exit
         default - can be changed to Find Again
    Ctrl+D Save as
    Ctrl+W Word wrap on/off
    Ctrl+A Select all
  Fixed a memory corruption bug
  More help

1.0.7
  Get rid of the default blank space at the bottom of text
    (stupid vertical scroll behaviour)
  Standard eCS Product Information dialog
  Standard menu font
  Don't display (wrong) line/column if wrapping
  BldLevel version number
 
1.0.6
  Fixed lockup printing
  If wrap is on, no horizontal scrollbar
  Can load files without extension

1.0.5 
  Public release
  Or maybe it was another version.

Multi-lingual Support
---------------------

AE can now load different languages for its user interface. 

It looks for a file which is called 'ae_XXX.lng'. where XXX is based. The particular file loaded is based on the LANG environment variable. For example, on US systems LANG=EN_US so it will look for ae_en_us.lng. If it cannot find that, it tries looking for the major part of the LANG variable, for example ae_en.lng. If it cannot find that, the default is US English (which is built into the .EXE - you have to restart to get it back).

Normally, translations are not for a particular region, so the files that come with AE are just (for example) ae_es.lng, not ae_es_es.lng.

- Where Language Files Live

Language files are searched for in (in order):
  - %OSDIR%\lang  (eCS 1.1+)
  - %ULSPATH%
  - Application directory

- Creating a new translation

You can easily create a translation file for AE. Use Options - Save/Update Language to create a new .lng file. This .lng file will contain all the text items that the program uses. Translate the text and save it.

You can call the .lng file whatever you like, but follow the rules above if you want it to be loaded automatically. 

If you send me your translations, I'll include them with the next release. To save wasting your time, check with me before starting a translation, in case someone has already done one.

Notes:
~ (tilde) in the text indicates that the next letter that should be underlined and used as a hot key. (You may also see & (ampersand) which does the same thing, but does not work quite as well - please tell me if you see one.)

\t in the text indicates a tab character, and is used to separate a menu item from its "accelerator" or key combination. However, you CANNOT change the accelerator e.g. F3 for Exit by this means (or any means). Please leave the part after \t as it is.

- Updating an existing translation

If you already have a file, that needs some updates - perhaps for a new version - then just save to the file again. The new items needing translation will be added, and items that are no longer required removed.

Unneeded items are left at the end of the file for you to check; remove them after you've done so, to save space. Note that sometimes items might be renamed, in this case you need to copy the old value to the new name. (Sorry!)

New items needing translation will be marked with *** 

Note: unfortunately any comments etc in the file will be discarded when you save. 

Multi-lingual Help Files
------------------------

Using similar logic, AE can load a language-specific version of it's own help file. It looks for ae_XXX.hlp, where XXX is derived from LANG as for .lng files above.

For a starting point, you can take the included AE.ipf. Note that I do NOT use this as the source; this file is generated by VyperHelp. This file is not installed, it's just included in the zip file.

-- end --