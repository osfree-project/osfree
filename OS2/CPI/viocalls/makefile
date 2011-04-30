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
IMPORTS =   VioEndPopUp                 SUB32.101, &
            VioGetPhysBuf               SUB32.102, &
            VioGetAnsi                  SUB32.103, &
            VioSetAnsi                  SUB32.105, &
            VioWrtTTY                   SUB32.119, &
            VioDeRegister               SUB32.106, &
            VioScrollUp                 SUB32.107, &
            VioPrtSc                    SUB32.108, &
            VioGetCurPos                SUB32.109, &
            VioWrtCellStr               SUB32.110, &
            VioPopUp                    SUB32.111, &
            VioScrollRt                 SUB32.112, &
            VioWrtCharStr               SUB32.113, &
            VioSetCurPos                SUB32.115, &
            VioScrUnLock                SUB32.118, &
            VioGetMode                  SUB32.121, &
            VioSetMode                  SUB32.122, &
            VioScrLock                  SUB32.123, &
            VioReadCellStr              SUB32.124, &
            VioSavRedrawWait            SUB32.125, &
            VioWrtNAttr                 SUB32.126, &
            VioGetCurType               SUB32.127, &
            VioSavRedrawUndo            SUB32.128, &
            VioGetFont                  SUB32.129, &
            VioReadCharStr              SUB32.130, &
            VioGetBuf                   SUB32.131, &
            VioSetCurType               SUB32.132, &
            VioSetFont                  SUB32.133, &
            VioModeUndo                 SUB32.135, &
            VioModeWait                 SUB32.137, &
            VioGetCp                    SUB32.140, &
            VioSetCp                    SUB32.142, &
            VioShowBuf                  SUB32.143, &
            VioScrollLf                 SUB32.144, &
            VioRegister                 SUB32.145, &
            VioGetConfig                SUB32.146, &
            VioScrollDn                 SUB32.147, &
            VioWrtCharStrAtt            SUB32.148, &
            VioGetState                 SUB32.149, &
            VioPrtScToggle              SUB32.150, &
            VioSetState                 SUB32.151, &
            VioWrtNCell                 SUB32.152, &
            VioWrtNChar                 SUB32.153, &
            VioAssociate                SUB32.155, &
            VioCreatePS                 SUB32.156, &
            VioDeleteSetId              SUB32.157, &
            VioGetDeviceCellSize        SUB32.158, &
            VioGetOrg                   SUB32.159, &
            VioCreateLogFont            SUB32.160, &
            VioDestroyPS                SUB32.161, &
            VioQuerySetIds              SUB32.162, &
            VioSetOrg                   SUB32.163, &
            VioQueryFonts               SUB32.164, &
            VioSetDeviceCellSize        SUB32.165, &
            VioShowPS                   SUB32.166, &
            VioGlobalReg                SUB32.170, &
            VioCheckCharType            SUB32.175

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

!include $(%ROOT)/mk/appsos2.mk
