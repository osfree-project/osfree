/*!
   @addtogroup ansi

   @brief ansi command - Frontend for ansi.sys. Allows user to turn ANSI
                         processing on or off.

   @par Purpose

   Change or view file and subdirectory attributes.

   @par Format

   <tt>ATTRIB [/A:[[-]rhsda] /D /E /P /Q /S] [+|-[AHRS]] file ...</tt>

:pt.:pd.:hp3.file :ehp3.&colon.  A file, directory, or list of files or directories on which to operate.
:eparml.:lm margin=13.:parml compact tsize=32 break=none.
:pt.:hp2./A&colon.:ehp2. (Attribute select):pd.:hp2./P:ehp2.(ause)
:pt.:hp2./D:ehp2.(irectories):pd.:hp2./Q:ehp2.(uiet)
:pt.:hp2./E:ehp2. (No error messages):pd.:hp2./S:ehp2.(ubdirectories)
:eparml.:p.
Attribute flags&colon.
:parml compact tsize=8 break=fit.
:pt.:hp2.+A:ehp2.:pd.Set the archive attribute
:pt.:hp2.-A:ehp2.:pd.Clear the archive attribute
:pt.:hp2.+H:ehp2.:pd.Set the hidden attribute
:pt.:hp2.-H:ehp2.:pd.Clear the hidden attribute
:pt.:hp2.+R:ehp2.:pd.Set the read-only attribute
:pt.:hp2.-R:ehp2.:pd.Clear the read-only attribute
:pt.:hp2.+S:ehp2.:pd.Set the system attribute
:pt.:hp2.-S:ehp2.:pd.Clear the system attribute
:eparml.:lm margin=1.

  @par File Selection

Supports extended :link reftype=hd refid=wildcards.wildcards:elink., :link reftype=hd refid=ranges.ranges:elink.,
:link reftype=hd refid=MultFiles.multiple file names:elink., and
:link reftype=hd refid=InclList.include lists:elink..

  @par Usage

Every file and subdirectory has 4 attributes that can be turned on
(set) or turned off (cleared)&colon.  :hp2.Archive:ehp2., :hp2.Hidden:ehp2., :hp2.Read-
only:ehp2., and :hp2.System:ehp2..
:p.
The ATTRIB command lets you view, set, or clear attributes for any file,
group of files, or subdirectory.  You can view file attributes by
entering ATTRIB without specifying new attributes (:hp1.i.e.:ehp1.,
without the :hp2.[+&splitvbar.-[AHRS]]:ehp2. part of the format), or with the :hp2.DIR
/T:ehp2. command.
:p.
You can view file attributes by entering ATTRIB without specifying new
attributes (:hp1.i.e.:ehp1., without the :hp2.[+|-[AHRS]]:ehp2. part of the
format).  (You can also view file attributes with the :link reftype=hd
refid=DIR.DIR:elink. /T command.)
:p.
The primary use of ATTRIB is to set attributes.  For example, you can set
the read-only and hidden attributes for the file :hp1.MEMO:ehp1.&colon.
:xmp.
        [c&colon.\] attrib +rh memo
:exmp.:p.
Attribute options apply to the file(s) that follow the options on
the ATTRIB command line.  The example below shows how to set
different attributes on different files with a single command.  It
sets the archive attribute for all :hp1..TXT:ehp1. files, then sets the
system attribute and clears the archive attribute for :hp1.TEST.COM:ehp1. &colon.
:xmp.
        [c&colon.\] attrib +a *.txt +s -a test.com
:exmp.:p.
When you use ATTRIB on an HPFS drive, you must quote any file
names which contain whitespace or special characters.  See :link reftype=hd
refid=FileNames.File Names:elink. for additional details.
:p.
To change directory attributes, use the :hp2./D:ehp2. switch.  If you give
ATTRIB a directory name instead of a file name, and omit :hp2./D:ehp2., it
will append "\*.*" to the end of the name and act on all files in that
directory, rather than acting on the directory itself.
:p.
Your operating system also supports "D" (subdirectory) and "V"
(volume label) attributes.  These attributes cannot be altered with
ATTRIB; they are designed to be controlled only by the operating
system itself.
:p.
ATTRIB will ignore underlines in the new attribute (the
:hp2.[+|-[AHRS]]:ehp2. part of the command).  For example, ATTRIB sees these
two commands as identical&colon.
:xmp.
        [c&colon.\] attrib +a filename
        [c&colon.\] attrib +__A_ filename
:exmp.:p.
This allows you to use a string of attributes from either the @ATTRIB
variable function or from ATTRIB itself (both of which use underscores to
represent attributes that are not set) and send that string back to ATTRIB
to set attributes for other files.  For example, to clear the attributes of
:hp1.FILE2:ehp1. and then set its attributes to match those of
:hp1.FILE1:ehp1. (enter this on one line)&colon.
:xmp.
        [c&colon.\] attrib -arhs file2 &amp. attrib +%@attrib[file1] file2
:exmp.:p.
:hp2.Options:ehp2.
:lm margin=4.:parml tsize=8 break=fit.
:pt.:hp2./A&colon.:ehp2.&colon.:pd.(Attribute select) Select only those files that
have the specified attribute(s) set.  Preceding the attribute character with a
hyphen [:hp2.-:ehp2.] will select files that do not have that attribute
set.  The colon [:hp2.&colon.:ehp2.] after :hp2./A:ehp2. is required.  The
attributes are&colon.
:parml tsize=8 compact break=fit.
:pt.:pd.:hp2.    R:ehp2.  Read-only
:pt.:pd.:hp2.    H:ehp2.  Hidden
:pt.:pd.:hp2.    S:ehp2.  System
:pt.:pd.:hp2.    D:ehp2.  Subdirectory
:pt.:pd.:hp2.    A:ehp2.  Archive
:eparml.:p.
If no attributes are listed at all (:hp1.e.g.:ehp1., :hp2.ATTRIB
/A&colon. ...:ehp2.), ATTRIB will select all
files and subdirectories including hidden and system files.  If
attributes are combined, all the specified attributes must match for a
file to be selected. For example, /A&colon.RHS will select only those files
with all three attributes set.
:p.
The :hp2./A&colon.:ehp2. switch specifies which files to select,
:hp2.not:ehp2. which attributes to set.  For example, to remove the archive
attribute from all hidden files, you could use this command&colon.
:xmp.
        [c&colon.\] attrib /a&colon.h -a *.*
:exmp.:p.
:pt.:hp2./D:ehp2.&colon.:pd.(Directories) If you use the :hp2./D:ehp2.
option, ATTRIB will modify the attributes of subdirectories in addition to
files (yes, you can have a hidden subdirectory)&colon.
:xmp.
        [c&colon.\] attrib /d +h c&colon.\mydir
:exmp.:p.
If you use a directory name instead of a file name, and omit :hp2./D:ehp2.,
ATTRIB will append "\*.*" to the end of the name and act on all files in
that directory, rather than acting on the directory itself.
:pt.:hp2./E:ehp2.&colon.:pd.(No error messages) Suppress all non-fatal error
messages, such as "File Not Found."  Fatal error messages, such as "Drive
not ready," will still be displayed.  This option is most useful in batch
files and aliases.
:pt.:hp2./P:ehp2.&colon.:pd.(Pause) Wait for a key to be pressed after each screen page
before continuing the display.  Your options at the prompt are explained in
detail under :link reftype=hd refid=Prompts.Page and File Prompts:elink..
:pt.:hp2./Q:ehp2.&colon.:pd.(Quiet) This option turns off ATTRIB's normal screen output.
It is most useful in batch files.
:pt.:hp2./S:ehp2.&colon.:pd.(Subdirectories) If you use the :hp2./S:ehp2.
option, the ATTRIB command will be applied to all matching files in the
current or named directory and all of its subdirectories.
:eparml.:lm margin=1.

*/