!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = diskcomp
DESC = Diskette compare tool
#defines object file names in format objname.$(O)
OBJS = diskcomp.$(O) misc.$(O) md5.$(O) drive.$(O)
# defines additional options for C compiler
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe # $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

