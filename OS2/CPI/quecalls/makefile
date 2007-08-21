#
# A Makefile for quecalls.dll
# (c) osFree project,
# prokushev, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = quecalls
DESC     = Queries API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk import
 @%append $^&.lnk   DOSREADQUEUE    DOSCALLS.674,
 @%append $^&.lnk   DOSPURGEQUEUE   DOSCALLS.675,
 @%append $^&.lnk   DOSCLOSEQUEUE   DOSCALLS.676,
 @%append $^&.lnk   DOSQUERYQUEUE   DOSCALLS.677,
 @%append $^&.lnk   DOSPEEKQUEUE    DOSCALLS.678,
 @%append $^&.lnk   DOSWRITEQUEUE   DOSCALLS.679,
 @%append $^&.lnk   DOSOPENQUEUE    DOSCALLS.680,
 @%append $^&.lnk   DOSCREATEQUEUE  DOSCALLS.681,
 @%append $^&.lnk   DosReadQueue2   DOSCALLS.661,
 @%append $^&.lnk   DosPurgeQueue2  DOSCALLS.662,
 @%append $^&.lnk   DosCloseQueue2  DOSCALLS.663,
 @%append $^&.lnk   DosQueryQueue2  DOSCALLS.664,
 @%append $^&.lnk   DosPeekQueue2   DOSCALLS.665,
 @%append $^&.lnk   DosWriteQueue2  DOSCALLS.666,
 @%append $^&.lnk   DosOpenQueue2   DOSCALLS.667,
 @%append $^&.lnk   DosCreateQueue2 DOSCALLS.668
 @%append $^&.lnk export
 @%append $^&.lnk   DOSREADQUEUE.1,
 @%append $^&.lnk   DOSPURGEQUEUE.2,
 @%append $^&.lnk   DOSCLOSEQUEUE.3,
 @%append $^&.lnk   DOSQUERYQUEUE.4,
 @%append $^&.lnk   DOSPEEKQUEUE.5,
 @%append $^&.lnk   DOSWRITEQUEUE.6,
 @%append $^&.lnk   DOSOPENQUEUE.7,
 @%append $^&.lnk   DOSCREATEQUEUE.8,
 @%append $^&.lnk   DosReadQueue2.9,
 @%append $^&.lnk   DosPurgeQueue2.10,
 @%append $^&.lnk   DosCloseQueue2.11,
 @%append $^&.lnk   DosQueryQueue2.12,
 @%append $^&.lnk   DosPeekQueue2.13,
 @%append $^&.lnk   DosWriteQueue2.14,
 @%append $^&.lnk   DosOpenQueue2.15,
 @%append $^&.lnk   DosCreateQueue2.16
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
