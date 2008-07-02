# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk



all: $(ROOT)$(SEP)lib$(SEP)glib.lib .SYMBOLIC

OBJS =  gerror.obj              &
        ghash.obj               &
        glist.obj               &
        gmem.obj                &
        gmessages.obj           &
        gnode.obj               &
        gqueue.obj              &
        grand.obj               &
        grel.obj                &
        gscanner.obj            &
        gsequence.obj   &
        gslist.obj              &
        gstrfuncs.obj           &
        gstring.obj             &
        gslice.obj      &
        gtree.obj &
        ghook.obj               &
        gfileutils.obj          &
        giochannel.obj          &
        gkeyfile.obj            &
        gmappedfile.obj &
        gmarkup.obj             &
        garray.obj              &
        gasyncqueue.obj         &
        gatomic.obj     &
        gbase64.obj     &
        gbookmarkfile.obj       &
        gcache.obj              &
        gchecksum.obj   &
        gcompletion.obj         &
        gdataset.obj            &
        gdate.obj               &
        gpattern.obj    &
        gprimes.obj             &
        gprintf.obj     &
        gqsort.obj              &
        gregex.obj              &
        gshell.obj              &
        gtestutils.obj  &
        gthread.obj             &
        gthreadpool.obj         &
        gtimer.obj              &
        gunibreak.obj           &
        gunicollate.obj         &
        gunidecomp.obj          &
        guniprop.obj            &
        gurifuncs.obj   &
        gstdio.obj      &
        gutf8.obj       &
        waitpid.obj       &
        galiasdef.obj       &
        gmain.obj            &
        gutils.obj          &
        localcharset.obj      &
        goption.obj     &
        gdir.obj        &
        gbacktrace.obj          &
        gconvert.obj




$(ROOT)$(SEP)lib$(SEP)glib.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
