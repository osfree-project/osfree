#define INCL_DOS
#define INCL_GPI    // for ft2lib.h
#include <os2.h>

#include "ft2lib.h"

static HMODULE m_hmodFt2Lib = NULLHANDLE;

static PFNENABLEFONTENGINE m_pfnFt2EnableFontEngine = NULL;
static PFNSETCOLOR m_pfnFt2SetColor = NULL;
static PFNSETBACKCOLOR m_pfnFt2SetBackColor = NULL;
static PFNCHARSTRINGPOSAT m_pfnFt2CharStringPosAt = NULL;
static PFNQUERYCHARSTRINGPOSAT m_pfnFt2QueryCharStringPosAt = NULL;
static PFNCREATELOGFONT m_pfnFt2CreateLogFont = NULL;
static PFNSETCHARSET m_pfnFt2SetCharSet = NULL;
static PFNSETCHARBOX m_pfnFt2SetCharBox = NULL;
static PFNQUERYFONTMETRICS m_pfnFt2QueryFontMetrics = NULL;
static PFNBEGINPAINT m_pfnFt2BeginPaint = NULL;
static PFNENDPAINT m_pfnFt2EndPaint = NULL;
static PFNGETPS m_pfnFt2GetPS = NULL;
static PFNGETSCREENPS m_pfnFt2GetScreenPS = NULL;
static PFNRELEASEPS m_pfnFt2ReleasePS = NULL;

BOOL loadFt2Lib( VOID )
{
    UCHAR szFailedName[ 256 ];

    if( DosLoadModule( szFailedName, sizeof( szFailedName ), "FT2LIB.DLL", &m_hmodFt2Lib ))
        return FALSE;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2EnableFontEngine", ( PFN * )&m_pfnFt2EnableFontEngine ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2SetColor", ( PFN * )&m_pfnFt2SetColor ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2SetBackColor", ( PFN * )&m_pfnFt2SetBackColor ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2CharStringPosAt", ( PFN * )&m_pfnFt2CharStringPosAt ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2QueryCharStringPosAt", ( PFN * )&m_pfnFt2QueryCharStringPosAt ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2CreateLogFont", ( PFN * )&m_pfnFt2CreateLogFont ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2SetCharSet", ( PFN * )&m_pfnFt2SetCharSet ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2SetCharBox", ( PFN * )&m_pfnFt2SetCharBox ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2QueryFontMetrics", ( PFN * )&m_pfnFt2QueryFontMetrics ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2BeginPaint", ( PFN * )&m_pfnFt2BeginPaint ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2EndPaint", ( PFN * )&m_pfnFt2EndPaint ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2GetPS", ( PFN * )&m_pfnFt2GetPS ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2GetScreenPS", ( PFN * )&m_pfnFt2GetScreenPS ))
        goto exit_error;

    if( DosQueryProcAddr( m_hmodFt2Lib, 0, "Ft2ReleasePS", ( PFN * )&m_pfnFt2ReleasePS ))
        goto exit_error;

    return TRUE;

exit_error :

    freeFt2Lib();
}

VOID freeFt2Lib( VOID )
{
    DosFreeModule( m_hmodFt2Lib );

    m_hmodFt2Lib = NULLHANDLE;
}

PFNENABLEFONTENGINE getFt2EnableFontEngine( VOID )
{
    return m_pfnFt2EnableFontEngine;
}

PFNSETCOLOR getFt2SetColor( VOID )
{
    return m_pfnFt2SetColor;
}

PFNSETBACKCOLOR getFt2SetBackColor( VOID )
{
    return m_pfnFt2SetBackColor;
}

PFNCHARSTRINGPOSAT getFt2CharStringPosAt( VOID )
{
    return m_pfnFt2CharStringPosAt;
}

PFNQUERYCHARSTRINGPOSAT getFt2QueryCharStringPosAt( VOID )
{
    return m_pfnFt2QueryCharStringPosAt;
}

PFNCREATELOGFONT getFt2CreateLogFont( VOID )
{
    return m_pfnFt2CreateLogFont;
}

PFNSETCHARSET getFt2SetCharSet( VOID )
{
    return m_pfnFt2SetCharSet;
}

PFNSETCHARBOX getFt2SetCharBox( VOID )
{
    return m_pfnFt2SetCharBox;
}

PFNQUERYFONTMETRICS getFt2QueryFontMetrics( VOID )
{
    return m_pfnFt2QueryFontMetrics;
}

PFNBEGINPAINT getFt2BeginPaint( VOID )
{
    return m_pfnFt2BeginPaint;
}

PFNENDPAINT getFt2EndPaint( VOID )
{
    return m_pfnFt2EndPaint;
}

PFNGETPS getFt2GetPS( VOID )
{
    return m_pfnFt2GetPS;
}

PFNGETSCREENPS getFt2GetScreenPS( VOID )
{
    return m_pfnFt2GetScreenPS;
}

PFNRELEASEPS getFt2ReleasePS( VOID )
{
    return m_pfnFt2ReleasePS;
}


