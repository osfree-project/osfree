#
# A Makefile for lzexpand.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

#32_BITS  = 1
DIR      = $(FILESDIR)$(SEP)dos
PROJ     = lzexpand
DESC     = Windows file expansion library
OBJS     = lzexpand.$(O)
#ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT  = lib windows.lib lib clibs.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    windows dll
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk export
 @%append $^&.lnk       LZCOPY          .1,
 @%append $^&.lnk       LZOPENFILE      .2,
 @%append $^&.lnk       LZINIT          .3,
 @%append $^&.lnk       LZSEEK          .4,
 @%append $^&.lnk       LZREAD          .5,
 @%append $^&.lnk       LZCLOSE         .6,
 @%append $^&.lnk       LZSTART         .7,
 @%append $^&.lnk       COPYLZFILE      .8,
 @%append $^&.lnk       LZDONE          .9,
 @%append $^&.lnk       GETEXPANDEDNAME .10
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(DIR)
