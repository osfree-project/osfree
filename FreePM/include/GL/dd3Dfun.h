/* dd3Dfun.h */
/*****************************************************************************
 * SOURCE FILE NAME = dd3Dfun.h
 * DESCRIPTIVE NAME = Functions & Function pointers prototypes for the DD3DI
 *
 * Copyright : Evgen
 *
 * ==========================================================================
 *
 * VERSION = 0.01
 *
 * DESCRIPTION
 *      typedefs for interface  between the loadable low level video 3D
 *      device driver and Mesa3d for OS/2
 *
 * NOTES
 *      may be placed at GL\
 ****************************************************************************/

#ifndef  DD3DFUN_INCLUDED
  #define DD3DFUN_INCLUDED

#include "dd3Di.h" //??

/*********************************************/
/*          Function prototypes              */
/*********************************************/
#ifndef DD3D_APIENTRY
#define DD3D_APIENTRY  _System
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Init driver */
int _InitLoadDriver( unsigned long ulFlag );

/* Init core device driver functions */
typedef int (DD3D_APIENTRY FN_DD3DI_IVOID) (void);
typedef FN_DD3DI_IVOID *PFN_DD3DI_IVOID;
/* rc = 0 - Ok    */
/*    = 1 - Error */
//FN_DD3DI_INIT  _DD3i_Init;
//int DD3D_APIENTRY  _DD3i_Init(void);
//int DD3D_APIENTRY  _DD3i_Close(void);


/* Test videocard hardware */
typedef int (DD3D_APIENTRY FN_DD3DI_TESTHARDWARE) (void);
typedef FN_DD3DI_TESTHARDWARE *PFN_DD3DI_TESTHARDWARE;
/* rc = 0 - Ok, supported */
/*    = 1 - Error, not supported */
//int DD3D_APIENTRY _DD3i_TestHardware(void);

/* Query current mode  */
//  PGDDMODEINFO  defined in GRADD.H,
//  PDD3_GDDMODEINFO    - in dd3Di.h they are equal, but should change to 3D modes
typedef int (DD3D_APIENTRY FN_DD3DI_QUERYCURRENTMODE) (PDD3_GDDMODEINFO pMode);
typedef FN_DD3DI_QUERYCURRENTMODE *PFN_DD3DI_QUERYCURRENTMODE;
/* rc = 0 - Ok    */
/*    = 1 - Error */
//int DD3D_APIENTRY _DD3i_QueryCurrentMode(PGDDMODEINFO pMode);


typedef int (DD3D_APIENTRY FN_DD3DI_SETDEFAULTCONFIG) (PVISUALCONFIG pConfig);
typedef FN_DD3DI_SETDEFAULTCONFIG *PFN_DD3DI_SETDEFAULTCONFIG;
//_DD3i_SetDefaultConfig

#ifndef TYPES_H /* Definition for GLcontext */
#define GLcontext void
#endif

/* Get driver string */
typedef const GLubyte * (DD3D_APIENTRY FN_DD3DI_GET_STRING) ( GLcontext *ctx, GLenum name );
typedef FN_DD3DI_GET_STRING *PFN_DD3DI_GET_STRING;
//const GLubyte  * DD3D_APIENTRY _DD3i_get_string ( GLcontext *ctx, GLenum name );

//analog for QUERYCURRENTMODE ???
typedef LONG * (DD3D_APIENTRY FN_DD3DI_GETVIDEOCONFIG) (HDC hdc);
typedef FN_DD3DI_GETVIDEOCONFIG *PFN_DD3DI_GETVIDEOCONFIG;
//

/* Init hab */
typedef void (DD3D_APIENTRY FN_DD3DI_MESAINITHAB) (HAB  proghab,int param);
typedef FN_DD3DI_MESAINITHAB *PFN_DD3DI_MESAINITHAB;
//

/******************************************************/
/**********        Functions for GLUT         *********/
/******************************************************/
/* wglMakeCurrentContext */
/*  HGLRC is (void *) */
typedef BOOL  (DD3D_APIENTRY FN_DD3DI_WGL_MAKECURRENT) (HDC hdc,void * hglrc);
typedef FN_DD3DI_WGL_MAKECURRENT *PFN_DD3DI_WGL_MAKECURRENT;
//BOOL wglMakeCurrent(HDC hdc,HGLRC hglrc)

/* wglDescribePixelFormat */
#ifndef  XVisualInfoDEFINED
struct XVisualInfo;
#define PIXELFORMATDESCRIPTOR void
#endif
typedef XVisualInfo *(DD3D_APIENTRY FN_DD3DI_WGL_DESCRIBEPIXELFORMAT) (int iPixelFormat);
typedef FN_DD3DI_WGL_DESCRIBEPIXELFORMAT *PFN_DD3DI_WGL_DESCRIBEPIXELFORMAT;
//typedef struct PixelFormatDescriptor XVisualInfo;
//XVisualInfo *wglDescribePixelFormat(int iPixelFormat)

/* wglCreateContext */
#ifndef HGLRC
typedef void *HGLRC;
#endif
typedef HGLRC (DD3D_APIENTRY FN_DD3DI_WGL_CREATECONTEXT)(HDC hdc,HPS hpsCurrent, HAB hab);
typedef FN_DD3DI_WGL_CREATECONTEXT *PFN_DD3DI_WGL_CREATECONTEXT;
//WGLAPI HGLRC GLAPIENTRY wglCreateContext(HDC);

/* wglGetProcAddress */
typedef void *(DD3D_APIENTRY FN_DD3DI_WGL_GETPROCADDRESS) (const char* lpszProc);
typedef FN_DD3DI_WGL_GETPROCADDRESS *PFN_DD3DI_WGL_GETPROCADDRESS;
//WGLAPI void *  GLAPIENTRY wglGetProcAddress(const char* lpszProc);

/* wglGetPixelFormat */
typedef int (DD3D_APIENTRY FN_DD3DI_WGL_GETPIXELFORMAT) (HDC hdc);
typedef FN_DD3DI_WGL_GETPIXELFORMAT *PFN_DD3DI_WGL_GETPIXELFORMAT;
//int  wglGetPixelFormat(HDC hdc);

/* wglSetPixelFormat */
typedef BOOL (DD3D_APIENTRY FN_DD3DI_WGL_SETPIXELFORMAT)
                   (HDC hdc, int iPixelFormat,PIXELFORMATDESCRIPTOR  *ppfd);
typedef FN_DD3DI_WGL_SETPIXELFORMAT *PFN_DD3DI_WGL_SETPIXELFORMAT;
//BOOL wglSetPixelFormat(HDC hdc,int iPixelFormat,XVisualInfo *ppfd);

/* wglChoosePixelFormat */
typedef int (DD3D_APIENTRY FN_DD3DI_WGL_CHOOSEPIXELFORMAT)
                   (HDC hdc, PIXELFORMATDESCRIPTOR *ppfd);
typedef FN_DD3DI_WGL_CHOOSEPIXELFORMAT *PFN_DD3DI_WGL_CHOOSEPIXELFORMAT;
//int   wglChoosePixelFormat(HDC hdc,  PIXELFORMATDESCRIPTOR *ppfd);

/* wglSwapBuffers */
typedef BOOL  (DD3D_APIENTRY FN_DD3DI_WGL_SWAPBUFFERS) (void);
typedef FN_DD3DI_WGL_SWAPBUFFERS *PFN_DD3DI_WGL_SWAPBUFFERS;
//BOOL  wglSwapBuffers(void);

#ifdef __cplusplus
}
#endif


#endif
       /* DD3DFUN_INCLUDED */
