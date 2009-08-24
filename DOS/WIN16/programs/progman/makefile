#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = progman
DESC = Windows Program Manager
#defines object file names in format objname.$(O)
srcfiles = $(p)dialog$(e) $(p)group$(e) $(p)grpfile$(e) $(p)main$(e) $(p)program$(e) $(p)string$(e)

# defines additional options for C compiler
ADD_COPT = -zW -sg -zw -bw -bg -d3 -db -hw
#DEBUG    = watcom
IMPORTS  = SHELLABOUT SHELL.22, &
           GETOPENFILENAME COMMDLG.1, &
           GETCURRENTDIRECTORY WINSMSG.20, &
           EXTRACTICON SHELL.34
RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)/mk/appsw16.mk
