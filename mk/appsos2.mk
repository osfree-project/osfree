#
#
#

!ifndef __appsos2_mk__
!define __appsos2_mk__

32_BITS = 1
PLATFORM = os2
CLEAN_ADD = *.inf *.cmd *.msg *.pl *.ru *.rsf *.c *.h

!include $(%ROOT)/mk/dirs.mk

ADD_COPT   =         -d__OS2__ -i=$(%WATCOM)$(SEP)h $(ADD_COPT)

!ifneq C s
# stack calling convention
C = r
!endif

!ifeq UNI2H 1

# generated uni2h headers

ADD_COPT   +=         -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared

#ADD_COPT    +=         -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)sub32 &
#                       -i=$(%WATCOM)$(SEP)h$(SEP)os2

!ifneq NOLIBS 1
ADD_LINKOPT += option nod lib $(%WATCOM)$(SEP)lib386$(SEP)math387$(C).lib, &
               $(%WATCOM)$(SEP)lib386$(SEP)os2$(SEP)emu387.lib, &
               $(BLD)lib$(SEP)clibext$(C).lib,$(BLD)lib$(SEP)sub32.lib, &
!ifeq CXX 1
               $(%WATCOM)$(SEP)lib386$(SEP)os2$(SEP)plib3$(C).lib, &
!endif
               $(%WATCOM)$(SEP)lib386$(SEP)os2$(SEP)clib3$(C).lib, &
               $(BLD)lib$(SEP)os2386.lib, &
               rexx.lib,$(BLD)lib$(SEP)pdcurses.lib

!endif

!else

# use Watcom headers

ADD_COPT    +=         -i=$(%WATCOM)$(SEP)h$(SEP)os2

!endif

ADD_RCOPT    =         -bt=os2 $(ADD_RCOPT)

!ifndef DEST
DEST    = os2
!endif

SUF += .msg .rsf

!include $(%ROOT)/mk/all.mk

ADD_COPT    +=        -i=$(%ROOT)$(SEP)include &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3

ADD_COPT   +=         -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)ojpeg &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GL &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)acpitk &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lvmtk &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)xwphelpers

###
!ifeq NOLIBS 1
ADD_LINKOPT +=        # OPTION REDEFSOK
!else
ADD_LINKOPT +=        lib $(BLD)lib$(SEP)cmd_shared.lib, &
		      $(BLD)lib$(SEP)all_shared.lib # op  internalrelocs  OPTION REDEFSOK
!endif


###

.rc.res: .AUTODEPEND
 @$(SAY) RESCMP   $^.
 $(verbose)$(RC) -r $(RCOPT) $[@ -fo=$^@

cplist = en pl ru

.rsf: $(PATH)

.rsf.msg:
 @$(SAY) MKMSGF   $^. $(LOG)
 $(verbose)$(MC) @$< $(LOG2)

rsf: .SYMBOLIC .PROCEDURE
 @%create $(PATH)$(T)
 #@for %i in ($(cplist)) do @%append $@ $(MYDIR)en$(SEP)oso001.txt $(PATH)oso001.%i
 @%append $(PATH)$(T) $(MYDIR)en$(SEP)$(T:.rsf=.txt) $(PATH)$(T:.rsf=.msg) /P 850 /L 1,1
 @%append $(PATH)$(T) $(MYDIR)pl$(SEP)$(T:.rsf=.txt) $(PATH)$(T:.rsf=.pl)  /P 852 /L 48,1
 @%append $(PATH)$(T) $(MYDIR)ru$(SEP)$(T:.rsf=.txt) $(PATH)$(T:.rsf=.ru)  /P 866 /L 25,1

!ifeq DLL 1
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
dllopts = dll
!ifdef DLLOPT
dllopts += $(DLLOPT)
!endif
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
dllopts =
!endif

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

!ifdef OBJS16
OBJS = $(OBJS) $(OBJS16)
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS) $(MYDIR)makefile
 @%create $^@
!ifdef WINDOWCOMPAT
 @%append $^@ SYSTEM os2v2_pm $(dllopts)
!else
 @%append $^@ SYSTEM os2v2 $(dllopts)
!endif
 @%append $^@ NAME $^*
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
!ifdef NODEFAULTLIBS
 @%append $^@ OPTION NODEFAULTLIBS
!endif
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
!ifdef HEAPSIZE
 @%append $^@ OPTION HEAP=$(HEAPSIZE)
!endif
!ifdef RESOURCE
 @%append $^@ OPTION RESOURCE=$(RESOURCE)
!endif
!ifdef IMPORTS
 @%append $^@ IMPORT $(IMPORTS)
!endif
!ifdef ALIASES
 @%append $^@ ALIAS  $(ALIASES)
!endif
!ifdef EXPORTS
 @%append $^@ EXPORT $(EXPORTS)
!endif
!ifdef OPTIONS
 @%append $^@ OPTION $(OPTIONS)
!endif
!ifdef DEBUG
 @%append $^@ DEBUG $(DEBUG)
!endif
!ifdef STUB
 @%append $^@ OPTION STUB=$(STUB)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

!ifeq DLL 1
$(PATH)$(PROJ).dll: $(PATH)$(PROJ).lnk
!else
$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk
!endif
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

!endif
