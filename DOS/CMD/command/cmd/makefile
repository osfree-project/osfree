#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ      = command
DESC      = Control file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)tcc2wat$(e) $(p)alias$(e) $(p)beep$(e) $(p)break$(e) $(p)call$(e) $(p)cdd$(e) $(p)chcp$(e) $(p)chdir$(e) &
           $(p)cls$(e) $(p)copy$(e) $(p)ctty$(e) $(p)date$(e) $(p)del$(e) $(p)dir$(e) $(p)dirs$(e) $(p)doskey$(e) &
           $(p)echo$(e) $(p)exit$(e) $(p)exit2$(e) $(p)fddebug$(e) $(p)for$(e) $(p)goto$(e) $(p)history$(e) &
           $(p)if$(e) $(p)memory$(e) $(p)mkdir$(e) $(p)path$(e) $(p)pause$(e) $(p)popd$(e) $(p)prompt$(e) &
           $(p)pushd$(e) $(p)rem$(e) $(p)ren$(e) $(p)rmdir$(e) $(p)set$(e) $(p)shift$(e) $(p)time$(e) &
           $(p)truename$(e) $(p)type.$(e) $(p)verify.$(e) $(p)which$(e) $(p)tcc2wat$(e)
# defines additional options for C compiler

!include $(%ROOT)/mk/dosapp.mk

ADD_COPT = -i=$(MYDIR)..$(SEP)suppl
TARGETS = $(PATH)cmd.lib

$(PATH)cmd.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)
