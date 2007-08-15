/* WarpGL.h */
#ifndef __WarpGL_x11_h__
#define __WarpGL_x11_h__

   #include <GL\os2_x11.h>

#include <GL\gl.h>
#include <GL\glut.h>

#include <WarpWin.h>

#define CALLBACK

#define HGLRC GLXContext
   typedef void * GLXContext;

struct PixelFormatDescriptor
{ int nSize;    //    sizeof(struct PixelFormatDescriptor),  размер структуры
  int nVersion; // номер версии
  int dwFlags;
  int iPixelType;  // разрядов на цвет
int cColorBits;
int cAlphaBits;    // альфа параметр
int cRedBits;      // биты цвета
int cGreenBits;
int cBlueBits;

int cAccumBits;     // буфер аккумулятора
int cAccumRedBits;  // биты аккумулятора
int cAccumBlueBits;
int cAccumGreenBits;
int cAccumAlphaBits;

int cDepthBits;     // pазрядов буфер глубины
int cStencilBits;   // буфер трафарета

int cAuxBuffers;    // вспомогательный буфер
int bReserved;      // зарезервирован

  int buf[10]; //??
};

typedef struct PixelFormatDescriptor XVisualInfo;
typedef  XVisualInfo PIXELFORMATDESCRIPTOR;
#define XVisualInfoDEFINED

/* Type definitions (conversions) */
typedef HPAL Colormap;
typedef QMSG XEvent;

typedef int Visual;                    /* Win32 equivalent of X11 type */
typedef HWND Window;
typedef BOOL Bool;
typedef HDC Display;

typedef HPOINTER Cursor;

/* Win32 "equivalent" cursors - eventually, the X glyphs should be
   converted to Win32 cursors -- then they will look the same */
#define XC_arrow               SPTR_ARROW
#define XC_top_left_arrow      SPTR_ARROW
#define XC_hand1               SPTR_SIZE
#define XC_pirate              SPTR_ICONINFORMATION /* ?? */
#define XC_question_arrow      SPTR_ICONQUESTION
#define XC_exchange            SPTR_ICONWARNING
#define XC_spraycan            SPTR_SIZE
#define XC_watch               SPTR_WAIT
#define XC_xterm               SPTR_TEXT
#define XC_crosshair           SPTR_MOVE
#define XC_sb_v_double_arrow   SPTR_SIZENS
#define XC_sb_h_double_arrow   SPTR_SIZEWE
#define XC_top_side            SPTR_ARROW
#define XC_bottom_side         SPTR_SIZENS
#define XC_left_side           SPTR_SIZEWE
#define XC_right_side          SPTR_SIZEWE
#define XC_top_left_corner     SPTR_SIZENWSE
#define XC_top_right_corner    SPTR_SIZENESW
#define XC_bottom_right_corner SPTR_SIZENWSE
#define XC_bottom_left_corner  SPTR_SIZENESW


typedef int Atom;                      /* dummies */
typedef int XDevice;
typedef int Status;

#define True  TRUE                     /* Win32 equivalents of X11 booleans */
#define False FALSE

#define None                 0L        /* universal null resource or null atom */

/* Input Event Masks. Used as event-mask window attribute and as arguments
   to Grab requests.  Not to be confused with event names.  */

#define NoEventMask                    0L
#define KeyPressMask                   (1L<<0)
#define KeyReleaseMask                 (1L<<1)
#define ButtonPressMask                        (1L<<2)
#define ButtonReleaseMask              (1L<<3)
#define EnterWindowMask                        (1L<<4)
#define LeaveWindowMask                        (1L<<5)
#define PointerMotionMask              (1L<<6)
#define PointerMotionHintMask          (1L<<7)
#define Button1MotionMask              (1L<<8)
#define Button2MotionMask              (1L<<9)
#define Button3MotionMask              (1L<<10)
#define Button4MotionMask              (1L<<11)
#define Button5MotionMask              (1L<<12)
#define ButtonMotionMask               (1L<<13)
#define KeymapStateMask                        (1L<<14)
#define ExposureMask                   (1L<<15)
#define VisibilityChangeMask           (1L<<16)
#define StructureNotifyMask            (1L<<17)
#define ResizeRedirectMask             (1L<<18)
#define SubstructureNotifyMask         (1L<<19)
#define SubstructureRedirectMask       (1L<<20)
#define FocusChangeMask                        (1L<<21)
#define PropertyChangeMask             (1L<<22)
#define ColormapChangeMask             (1L<<23)
#define OwnerGrabButtonMask            (1L<<24)

/* Key masks. Used as modifiers to GrabButton and GrabKey, results of
   QueryPointer, state in various key-, mouse-, and button-related
   events. */

#define ShiftMask              (1<<0)
#define LockMask               (1<<1)
#define ControlMask            (1<<2)
#define Mod1Mask               (1<<3)
#define Mod2Mask               (1<<4)
#define Mod3Mask               (1<<5)
#define Mod4Mask               (1<<6)
#define Mod5Mask               (1<<7)

/* Window classes used by CreateWindow */
/* Note that CopyFromParent is already defined as 0 above */

#define InputOutput            1
#define InputOnly              2

/* Window attributes for CreateWindow and ChangeWindowAttributes */

#define CWBackPixmap           (1L<<0)
#define CWBackPixel            (1L<<1)
#define CWBorderPixmap         (1L<<2)
#define CWBorderPixel           (1L<<3)
#define CWBitGravity           (1L<<4)
#define CWWinGravity           (1L<<5)
#define CWBackingStore          (1L<<6)
#define CWBackingPlanes                (1L<<7)
#define CWBackingPixel         (1L<<8)
#define CWOverrideRedirect     (1L<<9)
#define CWSaveUnder            (1L<<10)
#define CWEventMask            (1L<<11)
#define CWDontPropagate                (1L<<12)
#define CWColormap             (1L<<13)
#define CWCursor               (1L<<14)

/* ConfigureWindow structure */

#define CWX                    (1<<0)
#define CWY                    (1<<1)
#define CWWidth                        (1<<2)
#define CWHeight               (1<<3)
#define CWBorderWidth          (1<<4)
#define CWSibling              (1<<5)
#define CWStackMode            (1<<6)


/* Used in GetWindowAttributes reply */

#define IsUnmapped             0
#define IsUnviewable           1
#define IsViewable             2

/* Window stacking method (in configureWindow) */

#define Above                   0
#define Below                   1
#define TopIf                   2
#define BottomIf                3
#define Opposite                4

/* For CreateColormap */

#define AllocNone              0       /* create map with no entries */
#define AllocAll               1       /* allocate entire map writeable */


/* Flags used in StoreNamedColor, StoreColors */

#define DoRed                  (1<<0)
#define DoGreen                        (1<<1)
#define DoBlue                 (1<<2)

/* For XStoreColor */
#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)

/*
 * Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue                0x0000
#define XValue         0x0001
#define YValue         0x0002
#define WidthValue     0x0004
#define HeightValue    0x0008
#define AllValues      0x000F
#define XNegative      0x0010
#define YNegative      0x0020

/* flags argument in size hints */
#define USPosition     (1L << 0) /* user specified x, y */
#define USSize         (1L << 1) /* user specified width, height */

/* definitions for initial window state */
#define WithdrawnState 0       /* for windows that are not mapped */
#define NormalState 1  /* most applications want to start this way */
#define IconicState 3  /* application wants to start as an icon */
#define GameModeState 4  /* Win32 GLUT only (not in Xlib!). */

/* Type definitions */

typedef struct {
    unsigned int background_pixmap;    /* background pixmap */
    unsigned long background_pixel;    /* background pixel */
    unsigned long border_pixel;        /* border pixel value */
    long event_mask;           /* set of events that should be saved */
    long do_not_propagate_mask;        /* set of events that should not propagate */
    Bool override_redirect;       /* boolean value for override-redirect */
    Colormap colormap;            /* color map to be associated with window */
} XSetWindowAttributes;

typedef struct {
  unsigned long pixel;
  unsigned short red, green, blue;
  char flags;  /* do_red, do_green, do_blue */
} XColor;

typedef struct {
  unsigned char *value;           /* same as Property routines */
  Atom encoding;          /* prop type */
  int format;             /* prop data format: 8, 16, or 32 */
  unsigned long nitems;           /* number of data items in value */
} XTextProperty;

typedef struct {
  long flags;          /* marks which fields in this structure are defined */
  int x, y;            /* obsolete for new window mgrs, but clients */
  int width, height;   /* should set so old wm's don't mess up */
} XSizeHints;

#define WPARAM MPARAM
#define LPARAM MPARAM

#define DisplayWidth(display, screen) \
  GetSystemMetrics(SM_CXSCREEN)

#define DisplayHeight(display, screen) \
  GetSystemMetrics(SM_CYSCREEN)

extern Colormap XCreateColormap(
  Display* display,
  Window root,
  Visual* visual,
  int alloc);

#define __glutStrdup  strdup


struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};


int gettimeofday(struct timeval* tp, void* tzp);


/* Type definitions (conversions). */

#define PFD_TYPE_COLORINDEX 1
#define PFD_DRAW_TO_WINDOW  2
#define PFD_SUPPORT_OPENGL  4
#define PFD_GENERIC_FORMAT  8
#define PFD_DOUBLEBUFFER    0x10
#define PFD_SWAP_COPY       0x20
#define PFD_TYPE_RGBA       0x40
#define PFD_STEREO          0x80
#define PFD_DRAW_TO_BITMAP  0x100
#define PFD_SUPPORT_GDI     0x200
#define PFD_DOUBLEBUFFER_DONTCARE  0x400
#define PFD_STEREO_DONTCARE        0x800
#define PFD_NEED_PALETTE          0x1000

#define GLX_USE_GL              1       /* support GLX rendering */
#define GLX_BUFFER_SIZE         2       /* depth of the color buffer */
#define GLX_LEVEL               3       /* level in plane stacking */
#define GLX_RGBA                4       /* true if RGBA mode */
#define GLX_DOUBLEBUFFER        5       /* double buffering supported */
#define GLX_STEREO              6       /* stereo buffering supported */
#define GLX_AUX_BUFFERS         7       /* number of aux buffers */
#define GLX_RED_SIZE            8       /* number of red component bits */
#define GLX_GREEN_SIZE          9       /* number of green component bits */
#define GLX_BLUE_SIZE           10      /* number of blue component bits */
#define GLX_ALPHA_SIZE          11      /* number of alpha component bits */
#define GLX_DEPTH_SIZE          12      /* number of depth bits */
#define GLX_STENCIL_SIZE        13      /* number of stencil bits */
#define GLX_ACCUM_RED_SIZE      14      /* number of red accum bits */
#define GLX_ACCUM_GREEN_SIZE    15      /* number of green accum bits */
#define GLX_ACCUM_BLUE_SIZE     16      /* number of blue accum bits */
#define GLX_ACCUM_ALPHA_SIZE    17      /* number of alpha accum bits */

#define GLX_BAD_ATTRIB  2
#define GLX_BAD_VISUAL  4

int XPending(Display* display);

GLXContext glXCreateContext(HPS hps, /* diff from Win/X */
  XVisualInfo* visinfo,  GLXContext share,  Bool direct);

#define XMapWindow(display, window) \
  WinShowWindow(window, TRUE)

#define XUnmapWindow(display, window) \
  WinShowWindow(window, FALSE)

#define XWithdrawWindow(display, window, screen) \
  WinShowWindow(window, FALSE)

#define XIconifyWindow(display, window, screen) \
  WinShowWindow(window, FALSE)

#define XFlush(display) \
  /* Nothing. */

#define glXDestroyContext(display, context) \
  /* Nothing. */

#define XDestroyWindow(display, window) \
  WinDestroyWindow(window)

#define XFree(data) \
  free(data)

#define XFreeColormap(display, colormap) \
     GpiDeletePalette(colormap)

#define glXSwapBuffers(display, window) \
     wglSwapBuffers()

#define glXMakeCurrent(display, window, context) \
  wglMakeCurrent(WinQueryWindowDC(window), context)

#define XCreateFontCursor(display, shape) \
  WinLoadPointer(HWND_DESKTOP, 0, shape)

#define XDefineCursor(display, window, cursor) \
  WinSetPointer(HWND_DESKTOP,cursor)


int SetLastError(int nerr);
int GetLastError(void);

XVisualInfo * glXChooseVisual(int mode);
int glXGetConfig(XVisualInfo* visual, int attrib, int* value);

void
XStoreColor(Display* display, Colormap colormap, XColor* color);
void
XSetWindowColormap(Display* display, Window window, Colormap colormap);
Bool
XTranslateCoordinates(Display *display, Window src, Window dst,
                     int src_x, int src_y,
                     int* dest_x_return, int* dest_y_return,
                     Window* child_return);
Status
XGetGeometry(Display* display, Window window, Window* root_return,
            int* x_return, int* y_return,
            unsigned int* width_return, unsigned int* height_return,
            unsigned int *border_width_return, unsigned int* depth_return);

int XParseGeometry(char *string, int *x, int *y, unsigned int *width, unsigned int *height);

int DisplayHeightMM(Display* display, int screen);
int DisplayWidthMM(Display* display, int screen);

#include "dd3Dfun.h"


#ifdef __cplusplus
extern "C" {
#endif

LONG *GetVideoConfig(HDC hdc);
void ScreenToClient( HWND hwnd,   POINTL *point);

HAB FPM_GetCurrentHAB(void);

#if defined(DINAMIC_LOAD)

// XVisualInfo *wglDescribePixelFormat(int iPixelFormat);
   FN_DD3DI_WGL_DESCRIBEPIXELFORMAT wglDescribePixelFormat;

//int   wglChoosePixelFormat(HDC hdc,  PIXELFORMATDESCRIPTOR *ppfd);
   FN_DD3DI_WGL_CHOOSEPIXELFORMAT   wglChoosePixelFormat;
//BOOL wglSetPixelFormat(HDC hdc,int iPixelFormat,XVisualInfo *ppfd);
   FN_DD3DI_WGL_SETPIXELFORMAT      wglSetPixelFormat;
//  int  wglGetPixelFormat(HDC hdc);
   FN_DD3DI_WGL_GETPIXELFORMAT      wglGetPixelFormat;
//BOOL  wglSwapBuffers(void);
   FN_DD3DI_WGL_SWAPBUFFERS         wglSwapBuffers;

#else

extern   PFN_DD3DI_WGL_DESCRIBEPIXELFORMAT wglDescribePixelFormat;
extern   PFN_DD3DI_WGL_CHOOSEPIXELFORMAT   wglChoosePixelFormat;
extern   PFN_DD3DI_WGL_SETPIXELFORMAT      wglSetPixelFormat;
extern   PFN_DD3DI_WGL_GETPIXELFORMAT      wglGetPixelFormat;
extern   PFN_DD3DI_WGL_SWAPBUFFERS	   wglSwapBuffers;
#endif



#ifdef __cplusplus
}
#endif


#endif /* __WarpGL_x11_h__ */

