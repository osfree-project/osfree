/*
 *      MsgCode.c
 *
 *      @(#)msgcode.c	1.1 5/3/96 16:39:10 /users/sccs/src/samples/msgdemo/s.msgcode.c
 *
 * 
 *      converts ordinal numbers of messages into message strings
 * 
 * msgcode structure
 *      includes the msg number
 *      the message string
 *      the message opcode describes how to decode wparam,lparam
 *      an optional message counter
 */


#include <stdio.h>
#include "msgcode.h"


MSGCODE msgcode[] = {
     { 0x0000,  "WM_NULL", 0,0 },
     { 0x0001,  "WM_CREATE", 0,0 },
     { 0x0002,  "WM_DESTROY", 0,0 },
     { 0x0003,  "WM_MOVE", 0,0 },
     { 0x0004,  "WM_MOUSELEFTUP", 0,0 },
     { 0x0005,  "WM_SIZE", 0,0 },
     { 0x0006,  "WM_ACTIVATE", 0,0 },
     { 0x0007,  "WM_SETFOCUS", 0,0 },
     { 0x0008,  "WM_KILLFOCUS", 0,0 },
     { 0x000A,  "WM_ENABLE", 0,0 },
     { 0x000B,  "WM_SETREDRAW", 0,0 },
     { 0x000C,  "WM_SETTEXT", 0,0 },
     { 0x000D,  "WM_GETTEXT", 0,0 },
     { 0x000E,  "WM_GETTEXTLENGTH", 0,0 },
     { 0x000F,  "WM_PAINT", 0,0 },
     { 0x0010,  "WM_CLOSE", 0,0 },
     { 0x0011,  "WM_QUERYENDSESSION", 0,0 },
     { 0x0012,  "WM_QUIT", 0,0 },
     { 0x0013,  "WM_QUERYOPEN", 0,0 },
     { 0x0014,  "WM_ERASEBKGND", 0,0 },
     { 0x0015,  "WM_SYSCOLORCHANGE", 0,0 },
     { 0x0016,  "WM_ENDSESSION", 0,0 },
     { 0x0017,  "WM_SYSTEMERROR", 0,0 },
     { 0x0018,  "WM_SHOWWINDOW", 0,0 },
     { 0x0019,  "WM_CTLCOLOR", 0,0 },
     { 0x001A,  "WM_WININICHANGE", 0,0 },
     { 0x001B,  "WM_DEVMODECHANGE", 0,0 },
     { 0x001C,  "WM_ACTIVATEAPP", 0,0 },
     { 0x001D,  "WM_FONTCHANGE", 0,0 },
     { 0x001E,  "WM_TIMECHANGE", 0,0 },
     { 0x001F,  "WM_CANCELMODE", 0,0 },
     { 0x0020,  "WM_SETCURSOR", 0,0 },
     { 0x0021,  "WM_MOUSEACTIVATE", 0,0 },
     { 0x0022,  "WM_CHILDACTIVATE", 0,0 },
     { 0x0023,  "WM_QUEUESYNC", 0,0 },
     { 0x0024,  "WM_GETMINMAXINFO", 0,0 },
     { 0x0026,  "WM_PAINTICON", 0,0 },
     { 0x0027,  "WM_ICONERASEBKGND", 0,0 },
     { 0x0028,  "WM_NEXTDLGCTL", 0,0 },
     { 0x002A,  "WM_SPOOLERSTATUS", 0,0 },
     { 0x002B,  "WM_DRAWITEM", 0,0 },
     { 0x002C,  "WM_MEASUREITEM", 0,0 },
     { 0x002D,  "WM_DELETEITEM", 0,0 },
     { 0x002E,  "WM_VKEYTOITEM", 0,0 },
     { 0x002F,  "WM_CHARTOITEM", 0,0 },
     { 0x0030,  "WM_SETFONT", 0,0 },
     { 0x0031,  "WM_GETFONT", 0,0 },
     { 0x0035,  "WM_ISACTIVEICON", 0,0 },
     { 0x0037,  "WM_QUERYDRAGICON", 0,0 },
     { 0x0039,  "WM_COMPAREITEM", 0,0 },
     { 0x0041,  "WM_COMPACTING", 0,0 },
     { 0x0044,  "WM_COMMNOTIFY", 0,0 },
     { 0x0046,  "WM_WINDOWPOSCHANGING", 0,0 },
     { 0x0047,  "WM_WINDOWPOSCHANGED", 0,0 },
     { 0x0048,  "WM_POWER", 0,0 },
     { 0x0081,  "WM_NCCREATE", 0,0 },
     { 0x0082,  "WM_NCDESTROY", 0,0 },
     { 0x0083,  "WM_NCCALCSIZE", 0,0 },
     { 0x0084,  "WM_NCHITTEST", 0,0 },
     { 0x0085,  "WM_NCPAINT", 0,0 },
     { 0x0086,  "WM_NCACTIVATE", 0,0 },
     { 0x0087,  "WM_GETDLGCODE", 0,0 },
     { 0x0088,  "WM_SYNCPAINT", 0,0 },
     { 0x00A0,  "WM_NCMOUSEMOVE", 0,0 },
     { 0x00A1,  "WM_NCLBUTTONDOWN", 0,0 },
     { 0x00A2,  "WM_NCLBUTTONUP", 0,0 },
     { 0x00A3,  "WM_NCLBUTTONDBLCLK", 0,0 },
     { 0x00A4,  "WM_NCRBUTTONDOWN", 0,0 },
     { 0x00A5,  "WM_NCRBUTTONUP", 0,0 },
     { 0x00A6,  "WM_NCRBUTTONDBLCLK", 0,0 },
     { 0x00A7,  "WM_NCMBUTTONDOWN", 0,0 },
     { 0x00A8,  "WM_NCMBUTTONUP", 0,0 },
     { 0x00A9,  "WM_NCMBUTTONDBLCLK", 0,0 },
     { 0x0100,  "WM_KEYDOWN", 0,0 },
     { 0x0101,  "WM_KEYUP", 0,0 },
     { 0x0102,  "WM_CHAR", 0,0 },
     { 0x0103,  "WM_DEADCHAR", 0,0 },
     { 0x0104,  "WM_SYSKEYDOWN", 0,0 },
     { 0x0105,  "WM_SYSKEYUP", 0,0 },
     { 0x0106,  "WM_SYSCHAR", 0,0 },
     { 0x0107,  "WM_SYSDEADCHAR", 0,0 },
     { 0x0108,  "WM_KEYLAST", 0,0 },
     { 0x0110,  "WM_INITDIALOG", 0,0 },
     { 0x0111,  "WM_COMMAND", 0,0 },
     { 0x0112,  "WM_SYSCOMMAND", 0,0 },
     { 0x0113,  "WM_TIMER", 0,0 },
     { 0x0114,  "WM_HSCROLL", 0,0 },
     { 0x0115,  "WM_VSCROLL", 0,0 },
     { 0x0116,  "WM_INITMENU", 0,0 },
     { 0x0117,  "WM_INITMENUPOPUP", 0,0 },
     { 0x0118,  "WM_SYSTIMER", 0,0 },
     { 0x011F,  "WM_MENUSELECT", 0,0 },
     { 0x0120,  "WM_MENUCHAR", 0,0 },
     { 0x0121,  "WM_ENTERIDLE", 0,0 },
     { 0x0200,  "WM_MOUSEMOVE", 0,0 },
     { 0x0201,  "WM_LBUTTONDOWN", 0,0 },
     { 0x0202,  "WM_LBUTTONUP", 0,0 },
     { 0x0203,  "WM_LBUTTONDBLCLK", 0,0 },
     { 0x0204,  "WM_RBUTTONDOWN", 0,0 },
     { 0x0205,  "WM_RBUTTONUP", 0,0 },
     { 0x0206,  "WM_RBUTTONDBLCLK", 0,0 },
     { 0x0207,  "WM_MBUTTONDOWN", 0,0 },
     { 0x0208,  "WM_MBUTTONUP", 0,0 },
     { 0x0209,  "WM_MBUTTONDBLCLK", 0,0 },
     { 0x0210,  "WM_PARENTNOTIFY", 0,0 },
     { 0x0220,  "WM_MDICREATE", 0,0 },
     { 0x0221,  "WM_MDIDESTROY", 0,0 },
     { 0x0222,  "WM_MDIACTIVATE", 0,0 },
     { 0x0223,  "WM_MDIRESTORE", 0,0 },
     { 0x0224,  "WM_MDINEXT", 0,0 },
     { 0x0225,  "WM_MDIMAXIMIZE", 0,0 },
     { 0x0226,  "WM_MDITILE", 0,0 },
     { 0x0227,  "WM_MDICASCADE", 0,0 },
     { 0x0228,  "WM_MDIICONARRANGE", 0,0 },
     { 0x0229,  "WM_MDIGETACTIVE", 0,0 },
     { 0x0230,  "WM_MDISETMENU", 0,0 },
     { 0x0233,  "WM_DROPFILES", 0,0 },
     { 0x0300,  "WM_CUT", 0,0 },
     { 0x0301,  "WM_COPY", 0,0 },
     { 0x0302,  "WM_PASTE", 0,0 },
     { 0x0303,  "WM_CLEAR", 0,0 },
     { 0x0304,  "WM_UNDO", 0,0 },
     { 0x0305,  "WM_RENDERFORMAT", 0,0 },
     { 0x0306,  "WM_RENDERALLFORMATS", 0,0 },
     { 0x0307,  "WM_DESTROYCLIPBOARD", 0,0 },
     { 0x0308,  "WM_DRAWCLIPBOARD", 0,0 },
     { 0x0309,  "WM_PAINTCLIPBOARD", 0,0 },
     { 0x030A,  "WM_VSCROLLCLIPBOARD", 0,0 },
     { 0x030B,  "WM_SIZECLIPBOARD", 0,0 },
     { 0x030C,  "WM_ASKCBFORMATNAME", 0,0 },
     { 0x030D,  "WM_CHANGECBCHAIN", 0,0 },
     { 0x030E,  "WM_HSCROLLCLIPBOARD", 0,0 },
     { 0x030F,  "WM_QUERYNEWPALETTE", 0,0 },
     { 0x0310,  "WM_PALETTEISCHANGING", 0,0 },
     { 0x0311,  "WM_PALETTECHANGED", 0,0 },
     { 0x0380,  "WM_PENWINFIRST", 0,0 },
     { 0x038F,  "WM_PENWINLAST", 0,0 },
     { 0x0390,  "WM_COALESCE_FIRST", 0,0 },
     { 0x039F,  "WM_COALESCE_LAST", 0,0 },
        { 0,0,0,0 }
};

MSGCODE *
GetTwinMsgCode(int id)
{
        static char msgstr[16];
        static MSGCODE msgdemo;
        MSGCODE *msgp;

        for(msgp=msgcode;msgp->msgstr;msgp++) {
                if(msgp->msgid == id)
                        return msgp;
        }

        sprintf(msgstr,"0x%x",id);

        msgdemo.msgid  = id;
        msgdemo.msgstr = msgstr;
        msgdemo.msgcnt = 0;
        msgdemo.msgop  = 0;

        return &msgdemo;
}
