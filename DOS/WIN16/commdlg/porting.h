#define GET_WM_CTLCOLOR_MSG(type)		    (UINT)(WM_CTLCOLOR)
#define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
        (WPARAM)(hdc), MAKELONG(hwnd, type)
#define GET_WM_COMMAND_ID(wp, lp)                   (wp)
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)(id), MAKELONG(hwnd, cmd)
#define GET_WM_COMMAND_CMD(wp, lp)                  HIWORD(lp)
#define GET_WM_COMMAND_HWND(wp, lp)                 (HWND)LOWORD(lp)

/* strings used to obtain unique window message for communication
 * between dialog and caller
 */
#define LBSELCHSTRING  "commdlg_LBSelChangedNotify"
#define SHAREVISTRING  "commdlg_ShareViolation"
#define FILEOKSTRING   "commdlg_FileNameOK"
#define COLOROKSTRING  "commdlg_ColorOK"
#define SETRGBSTRING   "commdlg_SetRGBColor"
#define FINDMSGSTRING  "commdlg_FindReplace"
#define HELPMSGSTRING  "commdlg_help"
