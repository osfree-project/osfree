/* os2mesa.h */

#ifndef WMESA_H
#define WMESA_H


#ifdef __cplusplus
extern "C" {
#endif


#include <GL\gl.h>

/* This is the WMesa context 'handle': */
typedef struct wmesa_context *WMesaContext;




/*
 * Destroy a rendering context as returned by WMesaCreateContext()
 */
/*extern void WMesaDestroyContext( WMesaContext ctx );*/
extern void WMesaDestroyContext( void );


/*
 * Make the specified context the current one.
 */
extern void WMesaMakeCurrent( WMesaContext ctx );


/*
 * Return a handle to the current context.
 */
extern WMesaContext WMesaGetCurrentContext( void );


/*
 * Swap the front and back buffers for the current context.  No action
 * taken if the context is not double buffered.
 */
extern void WMesaSwapBuffers(void);


/*
 * In indexed color mode we need to know when the palette changes.
 */
extern void WMesaPaletteChange(HPAL Pal);

extern void WMesaMove(void);

#if defined(DINAMIC_LOAD)
//HGLRC wglCreateContext(HDC hdc,HPS hpsCurrent, HAB hab);
  FN_DD3DI_WGL_CREATECONTEXT wglCreateContext;
//BOOL wglMakeCurrent(HDC hdc,HGLRC hglrc);   
  FN_DD3DI_WGL_MAKECURRENT wglMakeCurrent;

#else 
  extern PFN_DD3DI_WGL_CREATECONTEXT wglCreateContext;
  extern PFN_DD3DI_WGL_MAKECURRENT   wglMakeCurrent;
#endif

BOOL wglCopyContext(HGLRC hglrcSrc,HGLRC hglrcDst,UINT mask);


#ifdef __cplusplus
}
#endif


#endif

