#
# A Makefile for PMWP
# (c) osFree project,
#

DEST_DIR =$(%OS2TK)$(SEP)idl$(SEP)
SRC_DIR =$(MYDIR)..$(SEP)contrib$(SEP)idl$(SEP)wps$(SEP)

!include $(%ROOT)/mk/all.mk

TARGETS   = $(DEST_DIR)wpobject.idl &
			$(DEST_DIR)wpdisk.idl &
			$(DEST_DIR)wpkeybd.idl &
			$(DEST_DIR)wpmouse.idl &
			$(DEST_DIR)wppgm.idl &
			$(DEST_DIR)wpshadow.idl &
			$(DEST_DIR)wpsound.idl &
			$(DEST_DIR)wpsystem.idl &
			$(DEST_DIR)wptypes.idl
			
SOURCES =	$(SRC_DIR)wpobject.idl &
			$(SRC_DIR)wpdisk.idl &
			$(SRC_DIR)wpkeybd.idl &
			$(SRC_DIR)wpmouse.idl &
			$(SRC_DIR)wppgm.idl &
			$(SRC_DIR)wpshadow.idl &
			$(SRC_DIR)wpsound.idl &
			$(SRC_DIR)wpsystem.idl &
			$(SRC_DIR)wptypes.idl

$(TARGETS): $(SOURCES)
 @$(MDHIER) $(DEST_DIR)
 @$(CP) $[@ $^@ $(BLACKHOLE)

