#ifndef __FT2LIB_H__
#define __FT2LIB_H__

#define INCL_GPI
#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL APIENTRY ( *PFNENABLEFONTENGINE )( BOOL );
typedef BOOL APIENTRY ( *PFNSETCOLOR )( HPS, LONG );
typedef BOOL APIENTRY ( *PFNSETBACKCOLOR )( HPS, LONG );
typedef LONG APIENTRY ( *PFNCHARSTRINGPOSAT )( HPS, PPOINTL, PRECTL, ULONG, LONG, PCH, PLONG );
typedef BOOL APIENTRY ( *PFNQUERYCHARSTRINGPOSAT )( HPS, PPOINTL, ULONG, LONG, PCH, PLONG, PPOINTL );
typedef LONG APIENTRY ( *PFNCREATELOGFONT )( HPS, PSTR8, LONG, PFATTRS );
typedef BOOL APIENTRY ( *PFNSETCHARSET )( HPS, LONG );
typedef BOOL APIENTRY ( *PFNSETCHARBOX )( HPS, PSIZEF );
typedef BOOL APIENTRY ( *PFNQUERYFONTMETRICS )( HPS, LONG, PFONTMETRICS );
typedef HPS  APIENTRY ( *PFNBEGINPAINT )( HWND, HPS, PRECTL );
typedef BOOL APIENTRY ( *PFNENDPAINT )( HPS );
typedef HPS  APIENTRY ( *PFNGETPS )( HWND );
typedef HPS  APIENTRY ( *PFNGETSCREENPS )( HWND );
typedef BOOL APIENTRY ( *PFNRELEASEPS )( HPS );

BOOL loadFt2Lib( VOID );
VOID freeFt2Lib( VOID );

PFNENABLEFONTENGINE     getFt2EnableFontEngine( VOID );
PFNSETCOLOR             getFt2SetColor( VOID );
PFNSETBACKCOLOR         getFt2SetBackColor( VOID );
PFNCHARSTRINGPOSAT      getFt2CharStringPosAt( VOID );
PFNQUERYCHARSTRINGPOSAT getFt2QueryCharStringPosAt( VOID );
PFNCREATELOGFONT        getFt2CreateLogFont( VOID );
PFNSETCHARSET           getFt2SetCharSet( VOID );
PFNSETCHARBOX           getFt2SetCharBox( VOID );
PFNQUERYFONTMETRICS     getFt2QueryFontMetrics( VOID );
PFNBEGINPAINT           getFt2BeginPaint( VOID );
PFNENDPAINT             getFt2EndPaint( VOID );
PFNGETPS                getFt2GetPS( VOID );
PFNGETSCREENPS          getFt2GetScreenPS( VOID );
PFNRELEASEPS            getFt2ReleasePS( VOID );

#ifdef __cplusplus
}
#endif

#endif
