"001\MISC" readme
(W) Ulrich M”ller Dec 1, 1999

The "misc" directory contains additional files which are required for
XWorkplace NLS. The makefile in this directory will only copy the files
which have changed to XWPRUNNING, so if you update these files, do it
here in the source tree and have them copied to your XWorkplace installation
by the makefile.

These are the files:

crobj001.cmd        create default config folder (REXX script)

instl001.cmd        create other default install objects (REXX script)

sound001.cmd        add new system sounds to INI files

xfldr001.tmf        text message file (TMF) for various XWorkplace messages. This
                    is new with V0.9.0 and replaces the OS/2 MSG file which was
                    used previously. This is a plain-text file now.

drvrs001.txt        list of drivers used by "Drivers" page in "OS/2 Kernel".
                    This is also new with V0.9.0.

xfcls001.txt        WPS class descriptions for "WPS Class List" object.



