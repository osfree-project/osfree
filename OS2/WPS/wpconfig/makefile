PROJ     = wpconfig
TRGT     = $(PROJ).dll
DESC     = WPS Config
srcfiles = $(p)wpkeybd$(e) $(p)wpmouse$(e) $(p)wpsound$(e) $(p)wpsystem$(e)
ADD_SCOPT = -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)idl
ADD_COPT = -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)h
ADD_LINKOPT  = segment   type DATA nonshared lib $(%OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact, start=dll_initterm
#UNI2H = 1

DEST_DIR =$(%OS2TK)$(SEP)idl
DESTH_DIR =$(%OS2TK)$(SEP)h
SRC_DIR =$(MYDIR)..$(SEP)contrib$(SEP)idl$(SEP)wps

IDLS = 			$(DEST_DIR)$(SEP)wpkeybd.idl &
			$(DEST_DIR)$(SEP)wpmouse.idl &
			$(DEST_DIR)$(SEP)wpsound.idl &
			$(DEST_DIR)$(SEP)wpsystem.idl

HEADERS=			$(DESTH_DIR)$(SEP)wpkeybd.h &
			$(DESTH_DIR)$(SEP)wpkeybd.xh &
			$(DESTH_DIR)$(SEP)wpmouse.h &
			$(DESTH_DIR)$(SEP)wpmouse.xh &
			$(DESTH_DIR)$(SEP)wpsound.h &
			$(DESTH_DIR)$(SEP)wpsound.xh &
			$(DESTH_DIR)$(SEP)wpsystem.h &
			$(DESTH_DIR)$(SEP)wpsystem.xh


IHEADERS=			$(PATH)$(SEP)wpkeybd.ih &
			$(PATH)$(SEP)wpkeybd.xih &
			$(PATH)$(SEP)wpmouse.ih &
			$(PATH)$(SEP)wpmouse.xih &
			$(PATH)$(SEP)wpsound.ih &
			$(PATH)$(SEP)wpsound.xih &
			$(PATH)$(SEP)wpsystem.ih &
			$(PATH)$(SEP)wpsystem.xih

#EXPORTS      = &
#		ShlStartWorkplace .211, &
#		WPObjectCClassData.22, &

DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_pm.mk

#TARGETS   = $(IDLS) $(IDLS2) $(HEADERS) $(IHEADERS) $(OBJS)
			
SOURCES =	$(SRC_DIR)$(SEP)wpkeybd.idl &
			$(SRC_DIR)$(SEP)wpmouse.idl &
			$(SRC_DIR)$(SEP)wpsound.idl &
			$(SRC_DIR)$(SEP)wpsystem.idl


$(HEADERS): $(IDLS)

$(IHEADERS): $(IDLS)

$(IDLS): $(SOURCES)
 @$(MDHIER) $(DEST_DIR)
 $(verbose)$(CP) $(SRC_DIR)$(SEP)$^. $^@ $(BLACKHOLE)


.idl: $(SRC_DIR)
.h: $(DEST_DIR)
.xh: $(DEST_DIR)
.ih: $(PATH)
#.xih: $(PATH)
