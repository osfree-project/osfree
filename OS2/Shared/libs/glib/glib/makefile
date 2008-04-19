# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk



all: $(ROOT)$(SEP)lib$(SEP)glib.lib .SYMBOLIC

OBJS =  garray.obj              &
        gasyncqueue.obj         &
        gatomic.obj     &
#        gbacktrace.obj          &
        gbase64.obj     &
        gbookmarkfile.obj       &
        gcache.obj              &
        gchecksum.obj   &
        gcompletion.obj         &
#        gconvert.obj            &
        gdataset.obj            &
        gdate.obj               &
#        gdir.obj        &
        gerror.obj              &
        gfileutils.obj          &
        ghash.obj               &
        ghook.obj               &
        giochannel.obj          &
        gkeyfile.obj            &
        glist.obj               &
#        gmain.obj               &
        gmappedfile.obj &
        gmarkup.obj             &
        gmem.obj                &
        gmessages.obj           &
        gnode.obj               &
        goption.obj     &
        gpattern.obj    &
        gprimes.obj             &
        gprintf.obj     &
#        gqsort.obj              &
        gqueue.obj              &
        grand.obj               &
        gregex.obj              &
        grel.obj                &
        gscanner.obj            &
        gsequence.obj   &
        gshell.obj              &
#        gslice.obj      &
        gslist.obj              &
#        gspawn-win32.obj        &
#        gstdio.obj      &
        gstrfuncs.obj           &
        gstring.obj             &
        gtestutils.obj  &
        gthread.obj             &
        gthreadpool.obj         &
        gtimer.obj              &
        gtree.obj               &
        gunibreak.obj           &
        gunicollate.obj         &
        gunidecomp.obj          &
        guniprop.obj            &
        gurifuncs.obj   &
        gutf8.obj
#        gutils.obj              &
#        localcharset.obj




$(ROOT)$(SEP)lib$(SEP)glib.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
