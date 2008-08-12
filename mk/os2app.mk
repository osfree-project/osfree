#
#
#

32_BITS = 1
#ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=$(%ROOT)$(SEP)include

!ifndef DEST
DEST    = os2
!endif

!include $(%ROOT)/mk/all.mk

cplist = en pl ru

SUF = $(SUF) .msg .rsf

.SUFFIXES:
.SUFFIXES: $(SUF)

.rsf: $(PATH)

.rsf.msg:
 @$(MC) @$<

rsf: .SYMBOLIC .PROCEDURE
 @%create $(PATH)$(T)
 #@for %i in ($(cplist)) do @%append $@ $(MYDIR)en$(SEP)oso001.txt $(PATH)oso001.%i
 @%append $(PATH)$(T) $(MYDIR)en$(SEP)$(T:.rsf=.txt) $(PATH)$(T:.rsf=.msg) /P 850 /L 1,1
 @%append $(PATH)$(T) $(MYDIR)pl$(SEP)$(T:.rsf=.txt) $(PATH)$(T:.rsf=.pl)  /P 852 /L 48,1
 @%append $(PATH)$(T) $(MYDIR)ru$(SEP)$(T:.rsf=.txt) $(PATH)$(T:.rsf=.ru)  /P 866 /L 25,1
