/******************************************************************************/
/* Main Menu defines                                                          */
/******************************************************************************/
                         
#define  RXHOST_WIN       0x1100
#define  RXIOM_EDIT       0x106
#define  RXIOA_COPY       0x107
#define  RXIOA_PASTE      0x108
#define  RXIOA_DELETE     0x109
#define  RXIOA_ALL        0x10a
#define  RXIOA_TRACE      0x10d
#define  RXIOA_FONTS      0x10e
#define  RXIOM_ACTIONS    0x10f
#define  RXIOA_HALT       0x110
#define  RXIOA_TRCSTEP    0x111
#define  RXIOA_TRCLAST    0x112
#define  RXIOA_TRCOFF     0x113
#define  RXIOM_HELP       0x114
#define  RXIOM_HELPHELP   0x115
#define  RXIOM_EXTHELP    0x116
#define  RXIOM_KEYSHELP   0x117
#define  RXIOM_INDEXHELP  0x118
#define  RXIO_RUNPROC     0x122
#define  DLG_BOX          0x123
#define  RXIOA_CUT        0x124
#define  RXIOA_DALL       0x125
#define  RXIOA_SEARCH     0x126
#define  RXIOA_PROPERTIES 0x127
#define  RXIOA_RESULTS    0x128
#define  RXIOM_USER       0x200
#define  RXIOWM_USER      0x300
#define  ID_OK            0x001
#define  ID_CANCEL        0x002
#define  PMREXX_CUALOGO   0x126
#define  MLE_WNDW         0x127
#define  STDIN_WNDW       0x128
#define  RH_EF1           0x129
#define  ID_EF1           0x12a
#define  RH_EFH           0x12b
#define  ID_EFH           0x12c
#define  RXD_SAVEQUIT     0x12d
#define  ID_BMP           0x12e

#define  REXX_STARTPROC       WM_USER+1
#define  REXX_ENDPROC         WM_USER+2
#define  REXX_MAKE_VISIBLE    WM_USER+3
#define  REXX_MAKE_INVISIBLE  WM_USER+4
#define  PMRXA_FLAGCHANGE     WM_USER+5
#define  PMRXA_RESULTCHANGE   WM_USER+6

#define  PMRXIO_EXIT     "PMREXXIO"    /* I/O exit name to use                */

/* PMREXXIO dialog control messages                                           */

                                       /* prototype for dialog functions      */
HWND RexxCreateIOWindow(HWND, PHWND);
VOID RexxDestroyIOWindow(HWND);

/*   Search dialog declarations */
MRESULT EXPENTRY SearchDlgProc(HWND, ULONG, MPARAM, MPARAM);

/* Search dialog defines */

#define PMREXX_SEARCH      130
#define RX_SEARCH_TEXT     131
#define RX_SEARCH_FIELD    132
#define RX_CASEBOX         133
#define PB_OK              134
#define PB_CANCEL          135
#define PB_HELP            136
