# Makefile for common shared libs

PROJ = gbm
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include -dENABLE_PNG -dENABLE_IJG -dENABLE_TIF -dENABLE_J2K
srcfiles = $(p)gbm$(e)    $(p)gbmpbm$(e)  $(p)gbmpgm$(e) $(p)gbmppm$(e) $(p)gbmpnm$(e)  &
           $(p)gbmbmp$(e) $(p)gbmtga$(e)  $(p)gbmkps$(e) $(p)gbmiax$(e) $(p)gbmpcx$(e)  &
           $(p)gbmtif$(e) $(p)gbmlbm$(e)  $(p)gbmvid$(e) $(p)gbmgif$(e) $(p)gbmxbm$(e)  &
           $(p)gbmspr$(e) $(p)gbmpsg$(e)  $(p)gbmgem$(e) $(p)gbmcvp$(e) $(p)gbmjpg$(e)  &
           $(p)gbmpng$(e) $(p)gbmxpm$(e)  $(p)gbmxpmcn$(e) $(p)gbmhelp$(e) $(p)gbmmap$(e) &
           $(p)gbmmem$(e) $(p)gbmj2k$(e)  $(p)gbmtrunc$(e) $(p)gbmbpp$(e)

          
!include $(%ROOT)tools/mk/libsos2.mk
!include $(MYDIR)gbm.mk

TARGETS  = $(PATH)$(PROJ).lib



#.c: $(PORT_BASE)gbmos2pm_gbm_gbmplugins.src$(SEP)gbm

#.h: $(PORT_BASE)gbmos2pm_gbm_gbmplugins.src$(SEP)gbm

.c: $(PORT_BASE)gbm

.h: $(PORT_BASE)gbm

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
