#ifndef __CDEFTYPES2_H__
#define __CDEFTYPES2_H__
#define CDEFTYPES2_INCLUDED

/* types tricks */

typedef CHAR     * _Seg16 PCHAR16;
typedef UCHAR    * _Seg16 PUCHAR16;

typedef VOID     * _Seg16 PVOID16;


#ifndef LONGLONG_INCLUDED
#define LONGLONG_INCLUDED 1

#ifndef INCL_LONGLONG
typedef struct _LONGLONG {
    ULONG ulLo;
    LONG  ulHi;
} LONGLONG, *PLONGLONG;

typedef struct _ULONGLONG {
    ULONG ulLo;
    ULONG ulHi;
} ULONGLONG, *PULONGLONG;
#else
typedef long long LONGLONG, *PLONGLONG;
typedef unsigned long long ULONGLONG, *PULONGLONG;
#endif
#endif

#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))
#define MAKEUSHORT(l, h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

#define MAKEP( sel,off )   ((void */(void * _Seg16)( (sel) << 16 | (off) ))
#define MAKE16P( sel,off ) ((void * _Seg16)( (sel) << 16 | (off) ))

#define SELECTOROF(ptr)    ((((ULONG)(ptr))>>13)|7)
#define OFFSETOF(p)        (((PUSHORT)&(p))[0])

#define MAKETYPE(v, type)  /*((type */&v))

#define FIELDOFFSET(type, field)   ((SHORT)&(((type */0)->field))

#define LOBYTE(w)   LOUCHAR(w)
#define HIBYTE(w)   HIUCHAR(w)
#define LOUCHAR(w)  ((UCHAR)(w))
#define HIUCHAR(w)  ((UCHAR)(((USHORT)(w) >> 8) & 0xff))
#define LOUSHORT(l) ((USHORT)((ULONG)l))
#define HIUSHORT(l) ((USHORT)(((ULONG)(l) >> 16) & 0xffff))

#define MAKEERRORID(sev, error) (ERRORID)(MAKEULONG((error), (sev)))
#define ERRORIDERROR(errid)     (LOUSHORT(errid))
#define ERRORIDSEV(errid)       (HIUSHORT(errid))

#define APIENTRY16 _Far16 _Pascal
#define PASCAL16   _Far16 _Pascal
#define CDECL16    _Far16 _Cdecl


#define  MOUSEMSG(pmsg) \
   ((PMSEMSG)((PBYTE)pmsg + sizeof(MPARAM) ))

#define  CHARMSG(pmsg) \
   ((PCHRMSG)((PBYTE)pmsg + sizeof(MPARAM) ))


#define MAKERXSTRING(r,p,l) \
    ((r).strptr = (PCH)p, (r).strlength = (ULONG)l)

   /* Useful macros */

   /***************************************************************************
   * ULONG/USHORT WinCheckButton(HWND hwndDlg, USHORT id, USHORT checkState)
   *    sets the check state of button #id.
   *    returns the previous check state.
   ***************************************************************************/
#define WinCheckButton(hwndDlg, id, usCheckState) \
   ((ULONG)WinSendDlgItemMsg(hwndDlg, id, BM_SETCHECK, \
   MPFROMSHORT(usCheckState), (MPARAM)NULL))

   /***************************************************************************
   * ULONG/USHORT WinQueryButtonCheckstate(HWND hwndDlg, USHORT id)
   *    returns the check state of buttin #id.
   *
   ***************************************************************************/
#define WinQueryButtonCheckstate(hwndDlg, id) \
   ((ULONG)WinSendDlgItemMsg(hwndDlg, id, BM_QUERYCHECK, \
   (MPARAM)NULL, (MPARAM)NULL))

   /***************************************************************************
   * BOOL WinEnableControl(HWND hwndDlg, USHORT id, BOOL fEnable)
   *    sets the enable state of button #id.
   *    returns fSuccess.
   ***************************************************************************/
#define WinEnableControl(hwndDlg, id, fEnable) \
   WinEnableWindow(WinWindowFromID(hwndDlg, id), fEnable)

   /***************************************************************************
   * BOOL WinShowControl(HWND hwndDlg, USHORT id, BOOL fShow)
   *    sets the show state of control #id.
   *    returns fSuccess.
   ***************************************************************************/
#define WinShowControl(hwndDlg, id, fShow) \
   WinShowWindow(WinWindowFromID(hwndDlg, id), fShow)

   /***************************************************************************
   * BOOL WinIsControlEnabled(HWND hwndDlg, ULONG id)
   *    returns the enable state of control #id.
   *
   ***************************************************************************/
#define WinIsControlEnabled(hwndDlg, id) \
   ((BOOL)WinIsWindowEnabled(WinWindowFromID(hwndDlg, id)))

/*** Macros to make an MPARAM from standard types. *************************/

#define MPVOID                     ((MPARAM)0L)
#define MPFROMP(p)                 ((MPARAM)((ULONG)(p)))
#define MPFROMHWND(hwnd)           ((MPARAM)(HWND)(hwnd))
#define MPFROMCHAR(ch)             ((MPARAM)(USHORT)(ch))
#define MPFROMSHORT(s)             ((MPARAM)(USHORT)(s))
#define MPFROM2SHORT(s1, s2)       ((MPARAM)MAKELONG(s1, s2))
#define MPFROMSH2CH(s, uch1, uch2) ((MPARAM)MAKELONG(s, MAKESHORT(uch1, uch2)))
#define MPFROMLONG(l)              ((MPARAM)(ULONG)(l))

/*** Macros to extract standard types from an MPARAM ***********************/

#define PVOIDFROMMP(mp)            ((VOID *)(mp))
#define HWNDFROMMP(mp)             ((HWND)(mp))
#define CHAR1FROMMP(mp)            ((UCHAR)((ULONG)mp))
#define CHAR2FROMMP(mp)            ((UCHAR)((ULONG)mp >> 8))
#define CHAR3FROMMP(mp)            ((UCHAR)((ULONG)mp >> 16))
#define CHAR4FROMMP(mp)            ((UCHAR)((ULONG)mp >> 24))
#define SHORT1FROMMP(mp)           ((USHORT)(ULONG)(mp))
#define SHORT2FROMMP(mp)           ((USHORT)((ULONG)mp >> 16))
#define LONGFROMMP(mp)             ((ULONG)(mp))

/*** Macros to make an MRESULT from standard types. ************************/

#define MRFROMP(p)                 ((MRESULT)(VOID *)(p))
#define MRFROMSHORT(s)             ((MRESULT)(USHORT)(s))
#define MRFROM2SHORT(s1, s2)       ((MRESULT)MAKELONG(s1, s2))
#define MRFROMLONG(l)              ((MRESULT)(ULONG)(l))

/*** Macros to extract standard types from an MRESULT **********************/

#define PVOIDFROMMR(mr)            ((VOID *)(mr))
#define SHORT1FROMMR(mr)           ((USHORT)((ULONG)mr))
#define SHORT2FROMMR(mr)           ((USHORT)((ULONG)mr >> 16))
#define LONGFROMMR(mr)             ((ULONG)(mr))

   /* Useful macros */

   /***************************************************************************
   * LONG/SHORT WinDeleteLboxItem(HWND hwndLbox, LONG/SHORT index)
   *    deletes item index from the listbox.
   *    returns the number of items left.
   ***************************************************************************/
   #define WinDeleteLboxItem(hwndLbox, index) \
   ((LONG)WinSendMsg(hwndLbox, LM_DELETEITEM, MPFROMLONG(index), \
   (MPARAM)NULL))

   /***************************************************************************
   * LONG/SHORT WinInsertLboxItem(HWND hwndLbox, LONG/SHORT index, PSZ psz)
   *    inserts psz into a listbox at index.  Index may be an LIT_ constant.
   *    returns the actual index where it was inserted.
   ***************************************************************************/
   #define WinInsertLboxItem(hwndLbox, index, psz) \
   ((LONG)WinSendMsg(hwndLbox, LM_INSERTITEM, MPFROMLONG(index), \
   MPFROMP(psz)))

   /***************************************************************************
   * LONG/SHORT WinQueryLboxCount(HWND hwndLbox)
   *    returns the number of items in the listbox.
   *
   ***************************************************************************/
   #define WinQueryLboxCount(hwndLbox) \
   ((LONG)WinSendMsg(hwndLbox, LM_QUERYITEMCOUNT, (MPARAM)NULL, \
   (MPARAM)NULL))

   /***************************************************************************
   * LONG/SHORT WinQueryLboxItemText(HWND hwndLbox, LONG/SHORT index, PSZ psz, SHORT cchMax)
   *    fills psz with the text if item index.
   *    returns the text length copied.
   ***************************************************************************/
   #define WinQueryLboxItemText(hwndLbox, index, psz, cchMax) \
   ((LONG)WinSendMsg(hwndLbox, LM_QUERYITEMTEXT, \
   MPFROM2SHORT((index), (cchMax)), MPFROMP(psz)))

   /***************************************************************************
   * LONG/SHORT WinQueryLboxItemTextLength(HWND hwndLbox, LONG/SHORT index)
   *    returns the length of item index.
   *
   ***************************************************************************/
   #define WinQueryLboxItemTextLength(hwndLbox, index)         \
   ((SHORT)WinSendMsg(hwndLbox, LM_QUERYITEMTEXTLENGTH,    \
   MPFROMSHORT(index), (MPARAM)NULL))

   /***************************************************************************
   * BOOL WinSetLboxItemText(HWND hwndLbox, LONG/SHORT index, PSZ psz)
   *    sets the text of item index to psz.
   *    returns fSuccess.
   ***************************************************************************/
   #define WinSetLboxItemText(hwndLbox, index, psz) \
   ((BOOL)WinSendMsg(hwndLbox, LM_SETITEMTEXT, \
   MPFROMLONG(index), MPFROMP(psz)))

   /***************************************************************************
   * LONG/SHORT WinQueryLboxSelectedItem(HWND hwndLbox)
   *    returns the item index selected. (for single selection only)
   *
   ***************************************************************************/
   #define WinQueryLboxSelectedItem(hwndLbox) \
   ((LONG)WinSendMsg(hwndLbox, LM_QUERYSELECTION, MPFROMLONG(LIT_FIRST), \
   (MPARAM)NULL))

/* make FIXED number from SHORT integer part and USHORT fractional part */
#define MAKEFIXED(intpart,fractpart) MAKELONG(fractpart,intpart)
/* extract fractional part from a fixed quantity */
#define FIXEDFRAC(fx)                (LOUSHORT(fx))
/* extract integer part from a fixed quantity */
#define FIXEDINT(fx)                 ((SHORT)HIUSHORT(fx))

#endif
