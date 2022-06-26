PROJ     = pmwp
TRGT     = $(PROJ).dll
DESC     = WPS
#DIRS     = rectangles habmgr messagemgr framemgr windowmgr input load dialogs errors
srcfiles = $(p)init$(e) # $(p)wpobject$(e) 
ADD_SCOPT = -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)idl
# ADD_LINKOPT  = segment   type DATA nonshared lib os2386.lib lib clib3r.lib
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact, start=dll_initterm
UNI2H = 1

DEST_DIR =$(%OS2TK)$(SEP)idl
DESTH_DIR =$(%OS2TK)$(SEP)h
SRC_DIR =$(MYDIR)..$(SEP)contrib$(SEP)idl$(SEP)wps

IDLS = $(DEST_DIR)$(SEP)wpobject.idl &
			$(DEST_DIR)$(SEP)wpdisk.idl &
			$(DEST_DIR)$(SEP)wpkeybd.idl &
			$(DEST_DIR)$(SEP)wpmouse.idl &
			$(DEST_DIR)$(SEP)wppgm.idl &
			$(DEST_DIR)$(SEP)wpshadow.idl &
			$(DEST_DIR)$(SEP)wpsound.idl &
			$(DEST_DIR)$(SEP)wpsystem.idl &
			$(DEST_DIR)$(SEP)wptypes.idl

IDLS2 = $(DEST_DIR)$(SEP)wpabs.idl

HEADERS=$(DESTH_DIR)$(SEP)wpobject.h &
			$(DESTH_DIR)$(SEP)wpobject.xh &
			$(DESTH_DIR)$(SEP)wpdisk.h &
			$(DESTH_DIR)$(SEP)wpdisk.xh &
			$(DESTH_DIR)$(SEP)wpkeybd.h &
			$(DESTH_DIR)$(SEP)wpkeybd.xh &
			$(DESTH_DIR)$(SEP)wpmouse.h &
			$(DESTH_DIR)$(SEP)wpmouse.xh &
			$(DESTH_DIR)$(SEP)wppgm.h &
			$(DESTH_DIR)$(SEP)wppgm.xh &
			$(DESTH_DIR)$(SEP)wpshadow.h &
			$(DESTH_DIR)$(SEP)wpshadow.xh &
			$(DESTH_DIR)$(SEP)wpsound.h &
			$(DESTH_DIR)$(SEP)wpsound.xh &
			$(DESTH_DIR)$(SEP)wpsystem.h &
			$(DESTH_DIR)$(SEP)wpsystem.xh &
			$(DESTH_DIR)$(SEP)wptypes.h &
			$(DESTH_DIR)$(SEP)wptypes.xh &
			$(DESTH_DIR)$(SEP)wpabs.h &
			$(DESTH_DIR)$(SEP)wpabs.xh


DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_pm.mk

TARGETS   = $(IDLS) $(IDLS2) $(HEADERS) $(OBJS)
			
SOURCES =	$(SRC_DIR)$(SEP)wpobject.idl &
			$(SRC_DIR)$(SEP)wpdisk.idl &
			$(SRC_DIR)$(SEP)wpkeybd.idl &
			$(SRC_DIR)$(SEP)wpmouse.idl &
			$(SRC_DIR)$(SEP)wppgm.idl &
			$(SRC_DIR)$(SEP)wpshadow.idl &
			$(SRC_DIR)$(SEP)wpsound.idl &
			$(SRC_DIR)$(SEP)wpsystem.idl &
			$(SRC_DIR)$(SEP)wptypes.idl

SOURCES2 =	$(MYDIR)$(SEP)wpabs.idl

$(HEADERS): $(IDLS) $(IDLS2)

$(IDLS): $(SOURCES)
 @$(MDHIER) $(DEST_DIR)
 $(verbose)$(CP) $(SRC_DIR)$(SEP)$^. $^@ $(BLACKHOLE)

$(IDLS2): $(SOURCES2)
 @$(MDHIER) $(DEST_DIR)
 $(verbose)$(CP) $(MYDIR)$(SEP)$^. $^@ $(BLACKHOLE)


.idl: $(SRC_DIR)
.h: $(DEST_DIR)
.xh: $(DEST_DIR)
