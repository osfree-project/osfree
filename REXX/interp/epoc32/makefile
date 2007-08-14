#
# Makefile for EPOC32 stuff
#
# Usage:
#    nmake 
#
# generated files without giving a target on the command line:
# rexx.exe    statically bounded REXX interpreter
# rexx.lib    static link library of the REXX interpreter, place anywhere
#             in a directory of your LIB-variable.
# regina.lib  link file for regina.dll, place anywhere in a directory of
#             your LIB-variable.
# regina.dll  dynamic link library of the REXX interpreter, place anywhere
#             in a directory of your PATH.
# regina.exe  dynamically bound REXX interpreter
#
# rxstack.exe daemon process that provides external queues
#
# rxqueue.exe program to access external queues
#
#
# Do not use default suffixes rule
#
.SUFFIXES:

all : REXX_EXE RECREXX_EXE MSGCMP_EXE Regina.hlp MTB_FILES
	makesis rexx_marm.pkg
	-del rexx.sis
	ren rexx_marm.sis rexx.sis
	makesis rexx_wins.pkg
	-del rexxW.sis
	ren rexx_wins.sis rexxW.sis
	makesis rexxstart_marm.pkg
	-del rexxstart.sis
	ren rexxstart_marm.sis rexxstart.sis
	makesis rexxstart_wins.pkg
	-del rexxstartW.sis
	ren rexxstart_wins.sis rexxstartW.sis

REXX_EXE: REXX.WINS REXX.MARM
	nmake -f REXX.WINS
	nmake -f REXX.MARM

RECREXX_EXE: RECREXX.WINS RECREXX.MARM
	nmake -f RECREXX.WINS
	nmake -f RECREXX.MARM

MSGCMP_EXE: MSGCMP.WINS MSGCMP.MARM
	nmake -f MSGCMP.WINS
	nmake -f MSGCMP.MARM

REXX.WINS : rexx.mmp
	makmake rexx.mmp wins

RECREXX.WINS : recrexx.mmp
	makmake recrexx.mmp wins

MSGCMP.WINS : msgcmp.mmp
	makmake msgcmp.mmp wins

REXX.MARM : rexx.mmp
	makmake rexx.mmp marm

RECREXX.MARM : recrexx.mmp
	makmake recrexx.mmp marm

MSGCMP.MARM : msgcmp.mmp
	makmake msgcmp.mmp marm

Regina.hlp: regina.rtf
	aleppo -epoc -compress regina.alp
	attrib -r regina.hlp
	del regina.hlp
	ren regina.hlp.c regina.hlp

MTB_FILES:
	nmake -f ..\makefile.win32.vc mtb_files
