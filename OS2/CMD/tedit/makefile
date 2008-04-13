# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = TEDIT
DESC = Tiny Editor
OBJ1 = box.$(O) colour.$(O) comm1.$(O) comm2.$(O) comm3.$(O) comm4.$(O) comm5.$(O) &
	commset1.$(O) commset2.$(O) commsos.$(O) cursor.$(O) default.$(O) &
	edit.$(O) error.$(O) execute.$(O) linked.$(O) column.obj mouse.obj memory.obj &
	nonansi.$(O) prefix.$(O) parser.$(O) reserved.$(O) scroll.$(O) show.$(O) sort.$(O) &
	target.$(O) the.$(O) util.$(O) print.$(O) regex.obj alloca.obj
OBJ2 = commutil.$(O) #trace.obj
OBJ3 = getch.$(O)
OBJ4 = query.$(O)
OBJ5 = thematch.$(O)
OBJ6 = directry.$(O) file.$(O)
OBJ7 = rexx.$(O)
OBJ8 = os2eas.$(O) mygetopt.$(O)
OBJX = $(XTRAOBJ)
OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJX)


ADD_COPT = -i=..$(SEP)include -dOS2 -dUSE_OS2REXX # -dSTDC_HEADERS -dPROTO -dOS2 -zz -oneatx -ei -zq -mf
ADD_LINKOPT = LIB rexx,pdcurses

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe

$(DIR)$(SEP)$(PROJ).exe: $(OBJS) tedit.hlp
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
#helpfile
 $(CP) tedit.hlp $(DIR)
#macros
 $(CP) *.the $(DIR)

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
