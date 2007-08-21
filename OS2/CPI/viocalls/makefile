#
# A Makefile for viocalls.dll
# (c) osFree project,
# prokushev, 2007
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = viocalls
DESC     = Video API
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
 @%append $^&.lnk   VIOENDPOPUP          DOSCALLS.770,
 @%append $^&.lnk   VIOGETPHYSBUF        DOSCALLS.771,
 @%append $^&.lnk   VIOGETANSI           DOSCALLS.772,
 @%append $^&.lnk   VIOFREE              DOSCALLS.773,
 @%append $^&.lnk   VIOSETANSI           DOSCALLS.774,
 @%append $^&.lnk   VIODEREGISTER        DOSCALLS.775,
 @%append $^&.lnk   VIOSCROLLUP          DOSCALLS.776,
 @%append $^&.lnk   VIOPRTSC             DOSCALLS.777,
 @%append $^&.lnk   VIOGETCURPOS         DOSCALLS.778,
 @%append $^&.lnk   VIOWRTCELLSTR        DOSCALLS.779,
 @%append $^&.lnk   VIOPOPUP             DOSCALLS.780,
 @%append $^&.lnk   VIOSCROLLRT          DOSCALLS.781,
 @%append $^&.lnk   VIOWRTCHARSTR        DOSCALLS.782,
 @%append $^&.lnk   AVS_PRTSC            DOSCALLS.783,
 @%append $^&.lnk   VIOSETCURPOS         DOSCALLS.784,
 @%append $^&.lnk   VIOSRFUNBLOCK        DOSCALLS.785,
 @%append $^&.lnk   VIOSRFBLOCK          DOSCALLS.786,
 @%append $^&.lnk   VIOSCRUNLOCK         DOSCALLS.787,
 @%append $^&.lnk   VIOWRTTTY            DOSCALLS.788,
 @%append $^&.lnk   VIOSAVE              DOSCALLS.789,
 @%append $^&.lnk   VIOGETMODE           DOSCALLS.790,
 @%append $^&.lnk   VIOSETMODE           DOSCALLS.791,
 @%append $^&.lnk   VIOSCRLOCK           DOSCALLS.792,
 @%append $^&.lnk   VIOREADCELLSTR       DOSCALLS.793,
 @%append $^&.lnk   VIOSAVREDRAWWAIT     DOSCALLS.794,
 @%append $^&.lnk   VIOWRTNATTR          DOSCALLS.795,
 @%append $^&.lnk   VIOGETCURTYPE        DOSCALLS.796,
 @%append $^&.lnk   VIOSAVREDRAWUNDO     DOSCALLS.797,
 @%append $^&.lnk   VIOGETFONT           DOSCALLS.798,
 @%append $^&.lnk   VIOREADCHARSTR       DOSCALLS.799,
 @%append $^&.lnk   VIOGETBUF            DOSCALLS.800,
 @%append $^&.lnk   VIOSETCURTYPE        DOSCALLS.801,
 @%append $^&.lnk   VIOSETFONT           DOSCALLS.802,
 @%append $^&.lnk   VIOHETINIT           DOSCALLS.803,
 @%append $^&.lnk   VIOMODEUNDO          DOSCALLS.804,
 @%append $^&.lnk   VIOSSWSWITCH         DOSCALLS.805,
 @%append $^&.lnk   VIOMODEWAIT          DOSCALLS.806,
 @%append $^&.lnk   AVS_PRTSCTOGGLE      DOSCALLS.807,
 @%append $^&.lnk   VIOGETCP             DOSCALLS.808,
 @%append $^&.lnk   VIORESTORE           DOSCALLS.809,
 @%append $^&.lnk   VIOSETCP             DOSCALLS.810,
 @%append $^&.lnk   VIOSHOWBUF           DOSCALLS.811,
 @%append $^&.lnk   VIOSCROLLLF          DOSCALLS.812,
 @%append $^&.lnk   VIOREGISTER          DOSCALLS.813,
 @%append $^&.lnk   VIOGETCONFIG         DOSCALLS.814,
 @%append $^&.lnk   VIOSCROLLDN          DOSCALLS.815,
 @%append $^&.lnk   VIOWRTCHARSTRATT     DOSCALLS.816,
 @%append $^&.lnk   VIOGETSTATE          DOSCALLS.817,
 @%append $^&.lnk   VIOPRTSCTOGGLE       DOSCALLS.818,
 @%append $^&.lnk   VIOSETSTATE          DOSCALLS.819,
 @%append $^&.lnk   VIOWRTNCELL          DOSCALLS.820,
 @%append $^&.lnk   VIOWRTNCHAR          DOSCALLS.821,
 @%append $^&.lnk   VIOSHELLINIT         DOSCALLS.822,
 @%append $^&.lnk   VIOASSOCIATE         DOSCALLS.823,
 @%append $^&.lnk   VIOCREATEPS          DOSCALLS.824,
 @%append $^&.lnk   VIODELETESETID       DOSCALLS.825,
 @%append $^&.lnk   VIOGETDEVICECELLSIZE DOSCALLS.826,
 @%append $^&.lnk   VIOGETORG            DOSCALLS.827,
 @%append $^&.lnk   VIOCREATELOGFONT     DOSCALLS.828,
 @%append $^&.lnk   VIODESTROYPS         DOSCALLS.829,
 @%append $^&.lnk   VIOQUERYSETIDS       DOSCALLS.830,
 @%append $^&.lnk   VIOSETORG            DOSCALLS.831,
 @%append $^&.lnk   VIOQUERYFONTS        DOSCALLS.832,
 @%append $^&.lnk   VIOSETDEVICECELLSIZE DOSCALLS.833,
 @%append $^&.lnk   VIOSHOWPS            DOSCALLS.834,
 @%append $^&.lnk   VIOGETPSADDRESS      DOSCALLS.835,
 @%append $^&.lnk   VIOQUERYCONSOLE      DOSCALLS.836,
 @%append $^&.lnk   VIOREDRAWSIZE        DOSCALLS.837,
 @%append $^&.lnk   VIOGLOBALREG         DOSCALLS.838,
 @%append $^&.lnk   XVIOSETCASTATE       DOSCALLS.839,
 @%append $^&.lnk   XVIOCHECKCHARTYPE    DOSCALLS.840,
 @%append $^&.lnk   XVIODESTROYCA        DOSCALLS.841,
 @%append $^&.lnk   XVIOCREATECA         DOSCALLS.842,
 @%append $^&.lnk   VIOCHECKCHARTYPE     DOSCALLS.843,
 @%append $^&.lnk   XVIOGETCASTATE       DOSCALLS.844
 @%append $^&.lnk export
 @%append $^&.lnk   VIOENDPOPUP.1,
 @%append $^&.lnk   VIOGETPHYSBUF.2,
 @%append $^&.lnk   VIOGETANSI.3,
 @%append $^&.lnk   VIOFREE.4,
 @%append $^&.lnk   VIOSETANSI.5,
 @%append $^&.lnk   VIODEREGISTER.6,
 @%append $^&.lnk   VIOSCROLLUP.7,
 @%append $^&.lnk   VIOPRTSC.8,
 @%append $^&.lnk   VIOGETCURPOS.9,
 @%append $^&.lnk   VIOWRTCELLSTR.10,
 @%append $^&.lnk   VIOPOPUP.11,
 @%append $^&.lnk   VIOSCROLLRT.12,
 @%append $^&.lnk   VIOWRTCHARSTR.13,
 @%append $^&.lnk   AVS_PRTSC.14,
 @%append $^&.lnk   VIOSETCURPOS.15,
 @%append $^&.lnk   VIOSRFUNBLOCK.16,
 @%append $^&.lnk   VIOSRFBLOCK.17,
 @%append $^&.lnk   VIOSCRUNLOCK.18,
 @%append $^&.lnk   VIOWRTTTY.19,
 @%append $^&.lnk   VIOSAVE.20,
 @%append $^&.lnk   VIOGETMODE.21,
 @%append $^&.lnk   VIOSETMODE.22,
 @%append $^&.lnk   VIOSCRLOCK.23,
 @%append $^&.lnk   VIOREADCELLSTR.24,
 @%append $^&.lnk   VIOSAVREDRAWWAIT.25,
 @%append $^&.lnk   VIOWRTNATTR.26,
 @%append $^&.lnk   VIOGETCURTYPE.27,
 @%append $^&.lnk   VIOSAVREDRAWUNDO.28,
 @%append $^&.lnk   VIOGETFONT.29,
 @%append $^&.lnk   VIOREADCHARSTR.30,
 @%append $^&.lnk   VIOGETBUF.31,
 @%append $^&.lnk   VIOSETCURTYPE.32,
 @%append $^&.lnk   VIOSETFONT.33,
 @%append $^&.lnk   VIOHETINIT.34,
 @%append $^&.lnk   VIOMODEUNDO.35,
 @%append $^&.lnk   VIOSSWSWITCH.36,
 @%append $^&.lnk   VIOMODEWAIT.37,
 @%append $^&.lnk   AVS_PRTSCTOGGLE.38,
 @%append $^&.lnk   VIOGETCP.40,
 @%append $^&.lnk   VIORESTORE.41,
 @%append $^&.lnk   VIOSETCP.42,
 @%append $^&.lnk   VIOSHOWBUF.43,
 @%append $^&.lnk   VIOSCROLLLF.44,
 @%append $^&.lnk   VIOREGISTER.45,
 @%append $^&.lnk   VIOGETCONFIG.46,
 @%append $^&.lnk   VIOSCROLLDN.47,
 @%append $^&.lnk   VIOWRTCHARSTRATT.48,
 @%append $^&.lnk   VIOGETSTATE.49,
 @%append $^&.lnk   VIOPRTSCTOGGLE.50,
 @%append $^&.lnk   VIOSETSTATE.51,
 @%append $^&.lnk   VIOWRTNCELL.52,
 @%append $^&.lnk   VIOWRTNCHAR.53,
 @%append $^&.lnk   VIOSHELLINIT.54,
 @%append $^&.lnk   VIOASSOCIATE.55,
 @%append $^&.lnk   VIOCREATEPS.56,
 @%append $^&.lnk   VIODELETESETID.57,
 @%append $^&.lnk   VIOGETDEVICECELLSIZE.58,
 @%append $^&.lnk   VIOGETORG.59,
 @%append $^&.lnk   VIOCREATELOGFONT.60,
 @%append $^&.lnk   VIODESTROYPS.61,
 @%append $^&.lnk   VIOQUERYSETIDS.62,
 @%append $^&.lnk   VIOSETORG.63,
 @%append $^&.lnk   VIOQUERYFONTS.64,
 @%append $^&.lnk   VIOSETDEVICECELLSIZE.65,
 @%append $^&.lnk   VIOSHOWPS.66,
 @%append $^&.lnk   VIOGETPSADDRESS.67,
 @%append $^&.lnk   VIOQUERYCONSOLE.68,
 @%append $^&.lnk   VIOREDRAWSIZE.69,
 @%append $^&.lnk   VIOGLOBALREG.70,
 @%append $^&.lnk   XVIOSETCASTATE.71,
 @%append $^&.lnk   XVIOCHECKCHARTYPE.72,
 @%append $^&.lnk   XVIODESTROYCA.73,
 @%append $^&.lnk   XVIOCREATECA.74,
 @%append $^&.lnk   VIOCHECKCHARTYPE.75,
 @%append $^&.lnk   XVIOGETCASTATE.76
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
