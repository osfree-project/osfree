APIRET APIENTRY BVSMain(ULONG fn)
      switch (fn)  
      {  
        case FN_GETPHYSBUF:
          break;
        case FN_GETBUF:
          break;
        case FN_SHOWBUF:
          break;
        case FN_GETCURPOS:
          break;
        case FN_GETCURTYPE:
          break;
        case FN_GETMODE:
          break;
        case FN_SETCURPOS:
          break;
        case FN_SETCURTYPE:
          break;
        case FN_SETMODE:
          break;
        case FN_READCHARSTR:
          break;
        case FN_READCELLSTR:
          break;
        case FN_WRTNCHAR:
          break;
        case FN_WRTNATTR:
          break;
        case FN_WRTNCELL:
          break;
        case FN_WRTCHARSTR:
          break;
        case FN_WRTCHARSTRATT:
          break;
        case FN_WRTCELLSTR:
          break;
        case FN_WRTTTY:
          break;
        case FN_SCROLLUP:
          break;
        case FN_SCROLLDN:
          break;
        case FN_SCROLLLF:
          break;
        case FN_SCROLLRT:
          break;
        case FN_SETANSI:
          break;
        case FN_GETANSI:
          break;
        case FN_PRTSC:
          break;
        case FN_SCRLOCK:
          break;
        case FN_SCRUNLOCK:
          break;
        case FN_SAVREDRAWWAIT:
          break;
        case FN_SAVREDRAWUNDO:
          break;
        case FN_POPUP:
          break;
        case FN_ENDPOPUP:
          break;
        case FN_PRTSCTOGGLE:
          break;
        case FN_MODEWAIT:
          break;
        case FN_MODEUNDO:
          break;
        case FN_GETFONT:
          break;
        case FN_GETCONFIG:
          break;
        case FN_SETCP:
          break;
        case FN_GETCP:
          break;
        case FN_SETFONT:
          break;
        case FN_GETSTATE:
          break;
        case FN_SETSTATE:
          break;
        case FN_REGISTER:
          break;
        case FN_DEREGISTER:
          break;
        case FN_GLOBALREG:
          break;
        case FN_CREATECA:
          break;
        case FN_GETCASTATE:
          break;
        case FN_SETCASTATE:
          break;
        case FN_DESTROYCA:
          break;
        case FN_CHECKCHARTYPE:
          break;
        case FN_SAVE:
          break;
        case FN_RESTORE:
          break;
        case FN_FREE:
          break;
        case FN_SHELLINIT:
          break;
        default: 
          BVSError();
      }  
}