#
# A Makefile for viocalls.dll
# (c) osFree project,
# author, date
#

PROJ     = viocalls
DESC     = Video API
srcfiles = $(p)wrap$(e) $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
UNI2H = 1

IMPORTS =   VioEndPopUp                 EMXWRAP.101, &
            VioGetPhysBuf               EMXWRAP.102, &
            VioGetAnsi                  EMXWRAP.103, &
            VioSetAnsi                  EMXWRAP.105, &
            VioWrtTTY                   EMXWRAP.119, &
            VioDeRegister               EMXWRAP.106, &
            VioScrollUp                 EMXWRAP.107, &
            VioPrtSc                    EMXWRAP.108, &
            VioGetCurPos                EMXWRAP.109, &
            VioWrtCellStr               EMXWRAP.110, &
            VioPopUp                    EMXWRAP.111, &
            VioScrollRt                 EMXWRAP.112, &
            VioWrtCharStr               EMXWRAP.113, &
            VioSetCurPos                EMXWRAP.115, &
            VioScrUnLock                EMXWRAP.118, &
            VioGetMode                  EMXWRAP.121, &
            VioSetMode                  EMXWRAP.122, &
            VioScrLock                  EMXWRAP.123, &
            VioReadCellStr              EMXWRAP.124, &
            VioSavRedrawWait            EMXWRAP.125, &
            VioWrtNAttr                 EMXWRAP.126, &
            VioGetCurType               EMXWRAP.127, &
            VioSavRedrawUndo            EMXWRAP.128, &
            VioGetFont                  EMXWRAP.129, &
            VioReadCharStr              EMXWRAP.130, &
            VioGetBuf                   EMXWRAP.131, &
            VioSetCurType               EMXWRAP.132, &
            VioSetFont                  EMXWRAP.133, &
            VioModeUndo                 EMXWRAP.135, &
            VioModeWait                 EMXWRAP.137, &
            VioGetCp                    EMXWRAP.140, &
            VioSetCp                    EMXWRAP.142, &
            VioShowBuf                  EMXWRAP.143, &
            VioScrollLf                 EMXWRAP.144, &
            VioRegister                 EMXWRAP.145, &
            VioGetConfig                EMXWRAP.146, &
            VioScrollDn                 EMXWRAP.147, &
            VioWrtCharStrAtt            EMXWRAP.148, &
            VioGetState                 EMXWRAP.149, &
            VioPrtScToggle              EMXWRAP.150, &
            VioSetState                 EMXWRAP.151, &
            VioWrtNCell                 EMXWRAP.152, &
            VioWrtNChar                 EMXWRAP.153, &
            VioAssociate                EMXWRAP.155, &
            VioCreatePS                 EMXWRAP.156, &
            VioDeleteSetId              EMXWRAP.157, &
            VioGetDeviceCellSize        EMXWRAP.158, &
            VioGetOrg                   EMXWRAP.159, &
            VioCreateLogFont            EMXWRAP.160, &
            VioDestroyPS                EMXWRAP.161, &
            VioQuerySetIds              EMXWRAP.162, &
            VioSetOrg                   EMXWRAP.163, &
            VioQueryFonts               EMXWRAP.164, &
            VioSetDeviceCellSize        EMXWRAP.165, &
            VioShowPS                   EMXWRAP.166, &
            VioGlobalReg                EMXWRAP.170, &
            VioCheckCharType            EMXWRAP.175

EXPORTS = VIOENDPOPUP.1,            &
          VIOGETPHYSBUF.2,          &
          VIOGETANSI.3,             &
#          VIOFREE.4,                &
          VIOSETANSI.5,             &
          VIODEREGISTER.6,          &
          VIOSCROLLUP.7,            &
          VIOPRTSC.8,               &
          VIOGETCURPOS.9,           &
          VIOWRTCELLSTR.10,         &
          VIOPOPUP.11,              &
          VIOSCROLLRT.12,           &
          VIOWRTCHARSTR.13,         &
#          AVS_PRTSC.14,             &
          VIOSETCURPOS.15,          &
#          VIOSRFUNBLOCK.16,         &
#          VIOSRFBLOCK.17,           &
          VIOSCRUNLOCK.18,          &
          VIOWRTTTY.19,             &
#          VIOSAVE.20,               &
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
#          VIOHETINIT.34,            &
          VIOMODEUNDO.35,           &
#          VIOSSWSWITCH.36,          &
          VIOMODEWAIT.37,           &
#          AVS_PRTSCTOGGLE.38,       &
          VIOGETCP.40,              &
#          VIORESTORE.41,            &
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
#          VIOSHELLINIT.54,          &
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
#          VIOGETPSADDRESS.67,       &
#          VIOQUERYCONSOLE.68,       &
#          VIOREDRAWSIZE.69,         &
          VIOGLOBALREG.70,          &
#          XVIOSETCASTATE.71,        &
#          XVIOCHECKCHARTYPE.72,     &
#          XVIODESTROYCA.73,         &
#          XVIOCREATECA.74,          &
          VIOCHECKCHARTYPE.75
#          XVIOGETCASTATE.76

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
