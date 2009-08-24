#
#
#

!ifndef __appsos2_mk__
!define __appsos2_mk__

32_BITS = 1
PLATFORM = os2
CLEAN_ADD = *.inf *.cmd *.msg *.pl *.ru *.rsf *.c *.h
RCOPT = &
                      -i=$(%WATCOM)$(SEP)h &
                      -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared &


!ifeq DLL 1
ADD_COPT   =          $(ADD_COPT) -bd
!endif

ADD_COPT   =          $(ADD_COPT) -d__OS2__ &
                      -i=$(%WATCOM)$(SEP)h &
#                  until UniAPI modules will be ready &
#                      -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared &
                      -i=$(%ROOT)$(SEP)include &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -bt=os2
ADD_LINKOPT =         $(ADD_LINKOPT) OPTION REDEFSOK, internalrelocs lib all_shared.lib,cmd_shared.lib,sub32.lib

!ifndef DEST
DEST    = os2
!endif

SUF += .msg .rsf

!include $(%ROOT)/mk/all.mk

cplist = en pl ru

.rsf: $(PATH)

.rsf.msg:
 @$(MC) @$<

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
RCOPT    = -bt=os2 $(RCOPT)


!ifdef RESOURCE
deps = $(RESOURCE)
!endif

!ifdef OBJS16
OBJS = $(OBJS) $(OBJS16)
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS)
 @%create $^@
!ifdef WINDOWCOMPAT
 @%append $^@ SYSTEM os2v2_pm $(dllopts)
!else
 @%append $^@ SYSTEM os2v2 $(dllopts)
!endif
 @%append $^@ NAME $^*
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
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
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)

!endif
