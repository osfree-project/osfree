:userdoc.

:h1 name=External.External Commands

:p.

To Do...

:p.The lists below categorize the available
commands by topic and will help you find the ones that you need.
:p.
System configuration&colon.
:lm margin=4.:table rules=none frame=box cols='12 12 12 12 12'.
:row.
:c.:link reftype=hd refid=ANSI.ANSI:elink.
:c.:link reftype=hd refid=LABEL.LABEL:elink.
:c.:link reftype=hd refid=LVM.LVM:elink.
:c.:link reftype=hd refid=MAKEINI.MAKEINI:elink.
:etable.
:p.
File and directory management&colon.
:lm margin=4.:table rules=none frame=box cols='12 12 12 12 12'.
:row.
:c.:link reftype=hd refid=BLDLEVEL.BLDLEVEL:elink.
:c.:link reftype=hd refid=BACKUP.BACKUP:elink.
:c.:link reftype=hd refid=RESTORE.RESTORE:elink.
:c.:link reftype=hd refid=UNDELETE.UNDELETE:elink.
:etable.
:p.

:ul.
:li.CHKDSK
:li.COMP
:li.DISKCOMP
:li.DISKCOPY
:li.DTRACE
:li.EAUTIL
:li.FDISK
:li.FIND
:li.FORMAT
:li.FSACCESS
:li.HELPMSG
:li.KEYB
:li.MKMSGF
:li.MODE
:li.MORE
:li.PATCH
:li.PDUMPSYS
:li.PDUMPUSR
:li.PRINT
:li.PROCDUMP
:li.PSTAT
:li.RECOVER
:li.REPLACE
:li.SORT
:li.SPOOL
:li.STRACE
:li.SYSLEVEL
:li.TRACE
:li.TRAPDUMP
:li.TRSPOOL
:li.XCOPY
:li.VMDISK
:eul.

:h2 name=VER.VER - Display the &4OS2. and OS/2 version
:parml compact tsize=12 break=fit.
:pt.:hp3.Purpose&colon.:ehp3.:pd.Display the current command processor and operating
system versions.
.br
:pt.:hp3.Format&colon.:ehp3.:pd.:hp2.VER [/R]:ehp2.
:eparml.:lm margin=13.:parml compact tsize=32 break=none.
:pt.:hp2./R:ehp2.(evision level):pd.
:eparml.:lm margin=1.
.br
:hp2.Usage:ehp2.
:p.
Version numbers consist of a one-digit major version number, a
period, and a one- or two-digit minor version number.  Ver uses the default
decimal separator defined by the current country information.  The VER
command displays both version numbers&colon.
:xmp.
        [c&colon.\] ver

        &4OS2. &4verrev.   OS/2 Version is 4.50
:exmp.:p.
:hp2.Options:ehp2.
:lm margin=4.:parml tsize=8 break=fit.
:pt.:hp2./R:ehp2.&colon.:pd.(Revision level) Display the &4OS2. and OS/2
internal revision levels (if any), plus your &4OS2. serial number and
registered name.
:eparml.:lm margin=1.

:userdoc.
