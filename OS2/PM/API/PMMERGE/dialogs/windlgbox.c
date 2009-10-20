#define INCL_PM
#define INCL_WIN
#include <os2.h>

ULONG APIENTRY WinDlgBox(const HWND hwndParent, const HWND hwndOwner, const PFNWP pfnDlgProc, const HMODULE hmod, const ULONG idDlg, const PVOID pCreateParams)
{
  HWND hwndDlg = WinLoadDlg(hwndParent, hwndOwner,
                            pfnDlgProc, hmod, idDlg, pCreateParams);

  ULONG ulReply = WinProcessDlg(hwndDlg);

  WinDestroyWindow(hwndDlg);

  return ulReply;
}
