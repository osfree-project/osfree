#
# A Makefile for viocalls.dll
# (c) osFree project,
# prokushev, 2007
#

PROJ     = viocalls
TRGT     = $(PROJ).dll
DESC     = Video API
OBJS     = $(PATH)..$(SEP)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib
IMPORTS = VIOENDPOPUP          DOSCALLS.770, &
          VIOGETPHYSBUF        DOSCALLS.771, &
          VIOGETANSI           DOSCALLS.772, &
          VIOFREE              DOSCALLS.773, &
          VIOSETANSI           DOSCALLS.774, &
          VIODEREGISTER        DOSCALLS.775, &
          VIOSCROLLUP          DOSCALLS.776, &
          VIOPRTSC             DOSCALLS.777, &
          VIOGETCURPOS         DOSCALLS.778, &
          VIOWRTCELLSTR        DOSCALLS.779, &
          VIOPOPUP             DOSCALLS.780, &
          VIOSCROLLRT          DOSCALLS.781, &
          VIOWRTCHARSTR        DOSCALLS.782, &
          AVS_PRTSC            DOSCALLS.783, &
          VIOSETCURPOS         DOSCALLS.784, &
          VIOSRFUNBLOCK        DOSCALLS.785, &
          VIOSRFBLOCK          DOSCALLS.786, &
          VIOSCRUNLOCK         DOSCALLS.787, &
          VIOWRTTTY            DOSCALLS.788, &
          VIOSAVE              DOSCALLS.789, &
          VIOGETMODE           DOSCALLS.790, &
          VIOSETMODE           DOSCALLS.791, &
          VIOSCRLOCK           DOSCALLS.792, &
          VIOREADCELLSTR       DOSCALLS.793, &
          VIOSAVREDRAWWAIT     DOSCALLS.794, &
          VIOWRTNATTR          DOSCALLS.795, &
          VIOGETCURTYPE        DOSCALLS.796, &
          VIOSAVREDRAWUNDO     DOSCALLS.797, &
          VIOGETFONT           DOSCALLS.798, &
          VIOREADCHARSTR       DOSCALLS.799, &
          VIOGETBUF            DOSCALLS.800, &
          VIOSETCURTYPE        DOSCALLS.801, &
          VIOSETFONT           DOSCALLS.802, &
          VIOHETINIT           DOSCALLS.803, &
          VIOMODEUNDO          DOSCALLS.804, &
          VIOSSWSWITCH         DOSCALLS.805, &
          VIOMODEWAIT          DOSCALLS.806, &
          AVS_PRTSCTOGGLE      DOSCALLS.807, &
          VIOGETCP             DOSCALLS.808, &
          VIORESTORE           DOSCALLS.809, &
          VIOSETCP             DOSCALLS.810, &
          VIOSHOWBUF           DOSCALLS.811, &
          VIOSCROLLLF          DOSCALLS.812, &
          VIOREGISTER          DOSCALLS.813, &
          VIOGETCONFIG         DOSCALLS.814, &
          VIOSCROLLDN          DOSCALLS.815, &
          VIOWRTCHARSTRATT     DOSCALLS.816, &
          VIOGETSTATE          DOSCALLS.817, &
          VIOPRTSCTOGGLE       DOSCALLS.818, &
          VIOSETSTATE          DOSCALLS.819, &
          VIOWRTNCELL          DOSCALLS.820, &
          VIOWRTNCHAR          DOSCALLS.821, &
          VIOSHELLINIT         DOSCALLS.822, &
          VIOASSOCIATE         DOSCALLS.823, &
          VIOCREATEPS          DOSCALLS.824, &
          VIODELETESETID       DOSCALLS.825, &
          VIOGETDEVICECELLSIZE DOSCALLS.826, &
          VIOGETORG            DOSCALLS.827, &
          VIOCREATELOGFONT     DOSCALLS.828, &
          VIODESTROYPS         DOSCALLS.829, &
          VIOQUERYSETIDS       DOSCALLS.830, &
          VIOSETORG            DOSCALLS.831, &
          VIOQUERYFONTS        DOSCALLS.832, &
          VIOSETDEVICECELLSIZE DOSCALLS.833, &
          VIOSHOWPS            DOSCALLS.834, &
          VIOGETPSADDRESS      DOSCALLS.835, &
          VIOQUERYCONSOLE      DOSCALLS.836, &
          VIOREDRAWSIZE        DOSCALLS.837, &
          VIOGLOBALREG         DOSCALLS.838, &
          XVIOSETCASTATE       DOSCALLS.839, &
          XVIOCHECKCHARTYPE    DOSCALLS.840, &
          XVIODESTROYCA        DOSCALLS.841, &
          XVIOCREATECA         DOSCALLS.842, &
          VIOCHECKCHARTYPE     DOSCALLS.843, &
          XVIOGETCASTATE       DOSCALLS.844
EXPORTS = VIOENDPOPUP.1,            &
          VIOGETPHYSBUF.2,          &
          VIOGETANSI.3,             &
          VIOFREE.4,                &
          VIOSETANSI.5,             &
          VIODEREGISTER.6,          &
          VIOSCROLLUP.7,            &
          VIOPRTSC.8,               &
          VIOGETCURPOS.9,           &
          VIOWRTCELLSTR.10,         &
          VIOPOPUP.11,              &
          VIOSCROLLRT.12,           &
          VIOWRTCHARSTR.13,         &
          AVS_PRTSC.14,             &
          VIOSETCURPOS.15,          &
          VIOSRFUNBLOCK.16,         &
          VIOSRFBLOCK.17,           &
          VIOSCRUNLOCK.18,          &
          VIOWRTTTY.19,             &
          VIOSAVE.20,               &
          VIOGETMODE.21,            &
          VIOSETMODE.22,            &
          VIOSCRLOCK.23,            &
          VIOREADCELLSTR.24,        &
          VIOSAVREDRAWWAIT.25,      &
          VIOWRTNATTR.26,           &
          VIOGETCURTYPE.27,         &
          VIOSAVREDRAWUNDO.28,      &
          VIOGETFONT.29,            &
          VIOREADCHARSTR.30,        &
          VIOGETBUF.31,             &
          VIOSETCURTYPE.32,         &
          VIOSETFONT.33,            &
          VIOHETINIT.34,            &
          VIOMODEUNDO.35,           &
          VIOSSWSWITCH.36,          &
          VIOMODEWAIT.37,           &
          AVS_PRTSCTOGGLE.38,       &
          VIOGETCP.40,              &
          VIORESTORE.41,            &
          VIOSETCP.42,              &
          VIOSHOWBUF.43,            &
          VIOSCROLLLF.44,           &
          VIOREGISTER.45,           &
          VIOGETCONFIG.46,          &
          VIOSCROLLDN.47,           &
          VIOWRTCHARSTRATT.48,      &
          VIOGETSTATE.49,           &
          VIOPRTSCTOGGLE.50,        &
          VIOSETSTATE.51,           &
          VIOWRTNCELL.52,           &
          VIOWRTNCHAR.53,           &
          VIOSHELLINIT.54,          &
          VIOASSOCIATE.55,          &
          VIOCREATEPS.56,           &
          VIODELETESETID.57,        &
          VIOGETDEVICECELLSIZE.58,  &
          VIOGETORG.59,             &
          VIOCREATELOGFONT.60,      &
          VIODESTROYPS.61,          &
          VIOQUERYSETIDS.62,        &
          VIOSETORG.63,             &
          VIOQUERYFONTS.64,         &
          VIOSETDEVICECELLSIZE.65,  &
          VIOSHOWPS.66,             &
          VIOGETPSADDRESS.67,       &
          VIOQUERYCONSOLE.68,       &
          VIOREDRAWSIZE.69,         &
          VIOGLOBALREG.70,          &
          XVIOSETCASTATE.71,        &
          XVIOCHECKCHARTYPE.72,     &
          XVIODESTROYCA.73,         &
          XVIOCREATECA.74,          &
          VIOCHECKCHARTYPE.75,      &
          XVIOGETCASTATE.76
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
