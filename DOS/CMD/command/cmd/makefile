#
# A Makefile for COMMAND.COM
# (c) osFree project,
# author, date
#

32_BITS = 0
PROJ      = cmd
TRGT      = $(PROJ).lib
DESC      = DOS command interpreter
#defines object file names in format objname.$(O)
srcfiles = $(p)alias$(e) $(p)beep$(e) $(p)break$(e) $(p)call$(e) $(p)cdd$(e) $(p)chcp$(e) $(p)chdir$(e) &
           $(p)cls$(e) $(p)copy$(e) $(p)ctty$(e) $(p)date$(e) $(p)del$(e) $(p)dir$(e) $(p)dirs$(e) $(p)doskey$(e) &
           $(p)echo$(e) $(p)exit$(e) $(p)exit2$(e) $(p)fddebug$(e) $(p)for$(e) $(p)goto$(e) $(p)history$(e) &
           $(p)if$(e) $(p)memory$(e) $(p)mkdir$(e) $(p)path$(e) $(p)pause$(e) $(p)popd$(e) $(p)prompt$(e) &
           $(p)pushd$(e) $(p)rem$(e) $(p)ren$(e) $(p)rmdir$(e) $(p)set$(e) $(p)shift$(e) $(p)time$(e) &
           $(p)truename$(e) $(p)type$(e) $(p)verify$(e) $(p)which$(e)
# defines additional options for C compiler

!include $(%ROOT)/mk/libsdos.mk

ADD_COPT = -ml -d2 -db  -i=$(MYDIR) -i=$(MYDIR).. -i=$(PATH) -i=$(PATH)..$(SEP)criter$(SEP)criter1 -i=$(MYDIR)..$(SEP)suppl -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)doscmd_shared.lib
TARGETS = $(PATH)cmd.lib

$(TARGETS): $(OBJS) $(PATH)..$(SEP)criter$(SEP)context.def
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
