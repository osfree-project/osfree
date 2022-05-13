#
# A Makefile for ANSI
# (c) osFree project,
#

PROJ  = winfile1
PROJ1 = winfile
TRGT = $(PROJ1).exe
DESC = Windows File Manager
srcfiles = $(p)splitpath$(e) $(p)winefile$(e)
# defines additional options for C compiler
ADD_COPT = -zW -sg -zw -bw -bg -d3 -db -hw
#DEBUG    = watcom
IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@
