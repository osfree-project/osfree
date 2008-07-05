#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

#32_BITS  = 1

#defines object file names in format objname.$(O)
OBJS = tcc2wat.$(O) alias.$(O) beep.$(O) break.$(O) call.$(O) cdd.$(O) chcp.$(O) chdir.$(O) &
        cls.$(O) copy.$(O) ctty.$(O) date.$(O) del.$(O) dir.$(O) dirs.$(O) doskey.$(O) &
        echo.$(O) exit.$(O) exit2.$(O) fddebug.$(O) for.$(O) goto.$(O) history.$(O) &
        if.$(O) memory.$(O) mkdir.$(O) path.$(O) pause.$(O) popd.$(O) prompt.$(O) &
        pushd.$(O) rem.$(O) ren.$(O) rmdir.$(O) set.$(O) shift.$(O) time.$(O) &
        truename.$(O) type.$(O) verify.$(O) which.$(O)
# defines additional options for C compiler

ADD_COPT = -i=..$(SEP)suppl

!include $(%ROOT)/mk/all.mk


all: cmd.lib

cmd.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)dos
