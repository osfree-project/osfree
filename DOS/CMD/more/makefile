#
# A Makefile for MORE
# (c) osFree project,
# author, date
#

PROJ = more
DESC = Show text file page by page
#defines object file names in format objname.$(O)
srcfiles = $(p)find95$(e) $(p)kitten$(e) $(p)l2s95$(e) $(p)lfnapi$(e) $(p)more$(e) &
           $(p)prf$(e) $(p)talloc$(e) $(p)tcdummy$(e) #$(p)tcc2wat$(e)

# defines additional options for linker
STACKSIZE = 40000

!include $(%ROOT)/mk/appsdos.mk
