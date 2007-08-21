# $Id: makefile,v 1.3 2004/08/16 04:51:10 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1
DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = cmd
DESC = Command processor
OBJS = main.$(O) parser.$(O) os2init.$(O) cmds.$(O) expand.$(O) eval.$(O) &
           ea.$(O) dircmds.$(O) filecmds.$(O) syscmds.$(O) os2cmds.$(O)   &
           os2calls.$(O) misc.$(O) env.$(O) batch.$(O) screenio.$(O) msg.$(O) &
           error.$(O) window.$(O) listall.$(O) lines.$(O) kparse.$(O)        &
           iofmt32.$(O) select.$(O) os2tty.$(O) iparse.$(O)     &
           bcd32.$(O) os32util.$(O) ..$(SEP)shared$(SEP)istring.$(O)
ADD_COPT = -i=..$(SEP)include -ox
ADD_LIB = MATH387S,OS2,OS2386

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe # $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).exe: $(OBJS) 4os2.rc
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBPATH $(LIBPATH)
 @%append $^&.lnk LIBRARY $(LINKLIB)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)
 @$(RC) 4os2.rc $@ $(LOG)

# Clean target
.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
