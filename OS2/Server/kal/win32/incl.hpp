#define APIENTRY _System /*will cause a warning*/
#define os2APIENTRY _System
#define BOOL os2BOOL
#define BYTE os2BYTE
#define PBOOL os2PBOOL
#define PBYTE os2PBYTE
#define PUINT os2PUINT
#define HWND os2HWND
#define HBITMAP os2HBITMAP
#define HDC os2HDC
#define HMODULE os2HMODULE
#define HRGN os2HRGN
#define HFILE os2HFILE
#define _RECTL os2_RECTL
#define RECTL os2RECTL
#define PRECTL os2PRECTL
#define _POINTL os2_POINTL
#define POINTL os2POINTL
#define PPOINTL os2PPOINTL
#define _POINTS os2_POINTS
#define POINTS os2POINTS
#define PPOINTS os2PPOINTS
#define _ICONINFO os2_ICONINFO
#define ICONINFO os2ICONINFO
#define PICONINFO os2PICONINFO

#define INCL_NOPMAPI
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_NOCOMMON
#define INCL_DOSSEMAPHORES
#define INCL_DOSMEMMGR
#define INCL_DOSNMPIPES
#define INCL_DOSDATETIME
#include <os2>

#undef APIENTRY
#undef FAR
#undef NEAR
#undef BOOL
#undef BYTE
#undef PBOOL
#undef PBYTE
#undef PUINT
#undef MAKELONG
#undef LOBYTE
#undef HIBYTE
#undef HWND
#undef INT
#undef HBITMAP
#undef HDC
#undef HMODULE
#undef HRGN
#undef HFILE
#undef _RECTL
#undef RECTL
#undef PRECTL
#undef _POINTL
#undef POINTL
#undef PPOINTL
#undef _POINTS
#undef POINTS
#undef PPOINTS
#undef _ICONINFO
#undef ICONINFO
#undef PICONINFO

#undef FILE_BEGIN
#undef FILE_CURRENT
#undef FILE_END
#undef MAKEERRORID
#undef ERRORIDERROR
#undef ERRORIDSEV
#undef SEVERITY_NOERROR
#undef SEVERITY_WARNING
#undef SEVERITY_ERROR
#undef SEVERITY_SEVERE
#undef SEVERITY_UNRECOVERABLE
#undef CREATE_SUSPENDED

#define WIN32_LEAN_AND_MEAN
#define STRICT
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCLTMGR
#define NODRAWTEXT
#define NOGDI
#define NOMETAFILE
#define NOMINMAX
#define NOSERVICE
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define WINVER 0x0351
#include <windows.h>

