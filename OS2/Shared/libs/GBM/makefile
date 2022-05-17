# Makefile for common shared libs

PROJ = gbm
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include -dENABLE_IJG -dENABLE_PNG -dENABLE_TIF -dENABLE_J2K
srcfiles = $(p)gbm$(e)    $(p)gbmpbm$(e)  $(p)gbmpgm$(e) $(p)gbmppm$(e) $(p)gbmpnm$(e)  &
           $(p)gbmbmp$(e) $(p)gbmtga$(e)  $(p)gbmkps$(e) $(p)gbmiax$(e) $(p)gbmpcx$(e)  &
           $(p)gbmtif$(e) $(p)gbmlbm$(e)  $(p)gbmvid$(e) $(p)gbmgif$(e) $(p)gbmxbm$(e)  &
           $(p)gbmspr$(e) $(p)gbmpsg$(e)  $(p)gbmgem$(e) $(p)gbmcvp$(e) $(p)gbmjpg$(e)  &
           $(p)gbmpng$(e) $(p)gbmxpm$(e)  $(p)gbmxpmcn$(e) $(p)gbmhelp$(e) $(p)gbmmap$(e) &
           $(p)gbmmem$(e) $(p)gbmj2k$(e)  $(p)gbmtrunc$(e) $(p)gbmbpp$(e)

#          gbms.obj  gbmpbm.obj gbmpgm.obj gbmppm.obj gbmpnm.obj \
#          gbmbmp.obj gbmtga.obj gbmkps.obj gbmiax.obj gbmpcx.obj \
#          gbmtif.obj gbmlbm.obj gbmvid.obj gbmgif.obj gbmxbm.obj \
#          gbmspr.obj gbmpsg.obj gbmgem.obj gbmcvp.obj gbmjbg.obj gbmjpg.obj \
#          gbmj2k.obj gbmpng.obj gbmxpm.obj gbmxpmcn.obj gbmraw.obj gbmhelp.obj gbmmap.obj 
#          gbmmem.obj
          
!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
