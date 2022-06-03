/*!

  @mainpage

  @section Content Content

  @li @ref page1
  @li @ref page2
  @li @ref page3

*/

/*!
  @page page1 About Command reference

  Text by osFree team, Hardin Brothers, Tom Rawson, and Rex Conn

  Help Text Copyright 2003, osFree team.

  Help Text Copyright 1993 - 2002, JP Software Inc., All Rights Reserved.

  Software Copyright 1988 - 2002, Rex Conn and JP Software Inc., All Rights
  Reserved.

  4DOS, 4OS2, and 4NT are JP Software Inc.'s trademarks for its family of
  character-mode command processors.  4DOS and Take Command are registered
  trademarks of JP Software Inc.  JP Software, jpsoft.com, and all JP
  Software designs and logos are also trademarks of JP Software Inc.  Other
  product and company names are trademarks of their respective owners.

*/

/*! @page page2 The Command Line

CMD.EXE displays a [c:\] prompt when it is waiting for you to enter a
command.  (The actual text depends on the current drive and
directory as well as your PROMPT settings.)  This is called the
command line and the prompt is asking you to enter a command, an
alias or batch file name, or the instructions necessary to begin an
application program.

This section explains the features that will help you while you are
typing in commands, and how keystrokes are interpreted when you
enter them at the command line.  The keystrokes discussed here are
the ones normally used by CMD.EXE.  If you prefer using different
keystrokes to perform these functions, you can assign new ones with
:link reftype=hd refid=INIKeyMap.key mapping directives:elink. in
:hp1.&4OS2INI.:ehp1..

The command line features documented in this section are&colon.
:lm margin=8.:parml compact tsize=4 break=fit.
:pt.:pd.:link reftype=hd refid=Editing.Command-Line Editing:elink.
:pt.:pd.:link reftype=hd refid=CmdHist.Command History and Recall:elink.
:pt.:pd.:link reftype=hd refid=HistWin.Command History Window:elink.
:pt.:pd.:link reftype=hd refid=CmdNames.Command Names and Parameters:elink.
:pt.:pd.:link reftype=hd refid=FileComp.Filename Completion:elink.
:pt.:pd.:link reftype=hd refid=AutoDir.Automatic Directory Changes:elink.
:pt.:pd.:link reftype=hd refid=DirHist.Directory History Window:elink.
:pt.:pd.:link reftype=hd refid=MultCmds.Multiple Commands:elink.
:pt.:pd.:link reftype=hd refid=ExpandAlias.Expanding and Disabling Aliases:elink.
:pt.:pd.:link reftype=hd refid=CmdLength.Command-Line Length Limits:elink.
:eparml.:lm margin=1.
.br

Additional command-line features are documented under
:link reftype=hd refid=FileSel.File Selection:elink. and :link reftype=hd
refid=TOC_Other.Other Features:elink..

*/

/*! @page page3 Command-Line Editing

The command line works like a single-line word processor, allowing you to
edit any part of the command at any time before you press :hp2.Enter:ehp2.
to execute it, or :hp2.Esc:ehp2. to erase it.  The command line you enter
can be up to 1023 characters long.
:p.
You can use the following editing keys when you are typing a command (the
words :hp2.Ctrl:ehp2. and :hp2.Shift:ehp2. mean to press the Ctrl or Shift
key together with the other key named)&colon.
:p.
:hp2.Cursor Movement Keys&colon.:ehp2.
:lm margin=8.:parml compact tsize=16 break=fit.
:pt.:hp2.:ehp2.:pd.Move the cursor left one character.
:pt.:hp2.:ehp2.:pd.Move the cursor right one character.
:pt.:hp2.Ctrl :ehp2.:pd.Move the cursor left one word.
:pt.:hp2.Ctrl :ehp2.:pd.Move the cursor right one word.
:pt.:hp2.Home:ehp2.:pd.Move the cursor to the beginning of the line.
:pt.:hp2.End:ehp2.:pd.Move the cursor to the end of the line.
:eparml.:lm margin=1.
.br
:hp2.Insert and Delete&colon.:ehp2.
:lm margin=8.:parml compact tsize=16 break=fit.
:pt.:hp2.Ins:ehp2.:pd.Toggle between insert and overtype mode.
:pt.:hp2.Del:ehp2.:pd.Delete the character at the cursor.
:pt.:hp2.Backspace:ehp2.:pd.Delete the character to the left of the cursor.
:pt.:hp2.Ctrl-L:ehp2.:pd.Delete the word or partial word to the left of the cursor.
:pt.:hp2.Ctrl-R:ehp2.:pd.or :hp2.Ctrl-Bksp:ehp2.   Delete the word or partial word to the right of the cursor.
:pt.:hp2.Ctrl-Home:ehp2.:pd.Delete from the beginning of the line to the cursor.
:pt.:hp2.Ctrl-End:ehp2.:pd.Delete from the cursor to the end of the line.
:pt.:hp2.Esc:ehp2.:pd.Delete the entire line.
:eparml.:lm margin=1.
.br
:hp2.Execution&colon.:ehp2.
:lm margin=8.:parml compact tsize=16 break=fit.
:pt.:hp2.Ctrl-C:ehp2.:pd.or :hp2.Ctrl-Break:ehp2.  Cancel the command line.
:pt.:hp2.Enter:ehp2.:pd.Execute the command line.
:eparml.:lm margin=1.
.br
Most of the command-line editing capabilities are also available
when a &4OS2. command prompts you for a line of input.  For example,
you can use the command-line editing keys when
:link reftype=hd refid=DESCRIBE.DESCRIBE:elink. prompts for a file
description, when :link reftype=hd refid=INPUT.INPUT:elink. prompts for
input from an alias or batch file, or when :link reftype=hd
refid=LIST.LIST:elink. prompts you for a search string.
:p.
If you want your input at the command line to be in a different
color from &4OS2.'s prompts or output, you can use the Display page of the
:link reftype=hd refid=OPTION.OPTION:elink. dialogs, or the
:link reftype=hd refid=INI_InputColors.InputColors:elink. directive in
:hp1.&4OS2INI.:ehp1..
:p.
&4OS2. will prompt for additional command-line text when you include
the :link reftype=hd refid=Escape.escape character:elink. as the very
last character of a typed command line.  The default escape character
is the caret [:hp2.^:ehp2.].  For example&colon.
:xmp.
        [c&colon.\] echo The quick brown fox jumped over the lazy^
        More? sleeping dog. > alphabet
:exmp.:p.
Sometimes you may want to enter one of the command line editing keystrokes
on the command line, instead of performing the key's usual action.  For
example, suppose you have a program that requires a Ctrl-R
character on its command line.  Normally you couldn't type this
keystroke at the prompt, because it would be interpreted as a
"Delete word right" command.
:p.
To get around this problem, use the special keystroke :hp2.Alt-255:ehp2..
You enter Alt-255 by holding down the :hp2.Alt:ehp2. key while you type
:hp2.255:ehp2. on the numeric keypad, then releasing the :hp2.Alt:ehp2. key (you
must use the number keys on the numeric pad; the row of keys at the
top of your keyboard won't work).  This forces &4OS2. to interpret
the next keystroke literally and places it on the command line,
ignoring any special meaning it would normally have as a command-line
editing or history keystroke.  You can use Alt-255 to suppress
the normal meaning of command-line editing keystrokes even if they
have been reassigned with :link reftype=hd refid=INIKeyMap.key mapping directives:elink. in
the :hp1..INI:ehp1. file, and Alt-255 itself can be reassigned with the
:link reftype=hd refid=INI_CommandEscape.CommandEscape:elink. directive.

*/
