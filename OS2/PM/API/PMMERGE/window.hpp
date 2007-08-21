/*  int dev_class;   device class */

int _FreePM_id_index   = 0;

#define FPM_W_DEV_MEMORY    0
#define FPM_W_DEV_VIDEO     1
#define FPM_W_DEV_PRINTER   2

/* int dev_type; device type */

#define FPM_W_DEVT_MEMORY       0

#define FPM_W_DEVT_PM           1
#define FPM_W_DEVT_GENGRADD     2
#define FPM_W_DEVT_SDDNUCLEUS   3

struct WinPresParam
{
   int ForegroundColor; /*  PP_FOREGROUNDCOLOR  Foreground color (in RGB) attribute. */
   int BackgroungColor; /*  PP_BACKGROUNDCOLOR  Background color (in RGB) attribute. */
   int HiliteForegroundColor;/* PP_HILITEFOREGROUNDCOLOR Highlighted foreground color (in RGB) attribute, for example for selected menu items.  */
   int HiliteBackgroungColor;/* PP_HILITEBACKGROUNDCOLOR Highlighted background color (in RGB) attribute. */
   int DisabledForegroundColor; /* PP_DISABLEDFOREGROUNDCOLOR Disabled foreground color (in RGB) attribute. */
   int DisabledBackgroungColor; /* PP_DISABLEDBACKGROUNDCOLOR Disabled background color (in RGB) attribute. */
   int BorderColor;      /* PP_BORDERCOLOR  Border color (in RGB) attribute.   */
   char *pFontNameSize;  /* PP_FONTNAMESIZE  Font name and size attribute. */
   int  ActiveColor;     /* Active color value of data type RGB. */
   int  InactiveColor;   /* Inactive color value of data type RGB. */
   int  ActiveTextFGNDColor; /* PP_ACTIVETEXTFGNDCOLOR Active text foreground color value of data type RGB. */
   int  ActiveTextBGNDColor; /* PP_ACTIVETEXTBGNDCOLOR Active text background color value of data type RGB. */
   int  InactiveTextFGNDColor; /* PP_INACTIVETEXTFGNDCOLOR Inactive text foreground color value of data type RGB. */
   int  InactiveTextBGNDColor; /* PP_INACTIVETEXTBGNDCOLOR Inactive text background color value of data type RGB. */
   int  ShadowColor;           /* PP_SHADOW  Changes the color used for drop shadows on certain controls. */
   int  user0;                 /* PP_USER This is a user-defined presentation parameter. */
};

/* Window Class */
class FPM_Window
{  int id; /* идентификатор */
protected:
   HWND handle;     /*  Window handle. */
   HWND hwndOwner;  /*  Owner-window handle. */
   HWND hwndParent; /*  Parent-window handle. */
   int dev_class;   /* класс устройства: 0-память, 1-дисплей, 2 - принтер */
   int dev_type;    /* тип устройства: 0-память, 1-PM Window, 2 - GENGRADD, 3 - SDDNUCLEUS */
   int x,y,z; /* координаты начала - левого нижнего угла */
   int nx; /* размер по X в pixel'ах */
   int ny; /* размер по Y в pixel'ах */
   int bytesPerPixel;  /* байт на пиксел */
   int bitsPerPixel;   /* бит на пиксел */
   int nColors;        /* число цветов на пиксел: 2,16,256, 32k, 64k ... */
   int class_type; /* класс окна     */
   PCSZ  pszClass;   /*  Registered-class name. */
   PCSZ  pszName;    /*  Window text. */
   ULONG flStyle;    /*  Window style. */
   ULONG rid;        /*  Window resource identifier. */
   struct WinPresParam pp;

public:
   FPM_Window
                  (HWND hwndParent,  /*  Parent-window handle. */
                   PCSZ  _pszClass,   /*  Registered-class name. */
                   PCSZ  _pszName,    /*  Window text. */
                   ULONG _flStyle,    /*  Window style. */
                   LONG _x,          /*  x-coordinate of window position. */
                   LONG _y,          /*  y-coordinate of window position. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   HWND hwndOwner,   /*  Owner-window handle. */
                   HWND hwndInsertBehind, /*  Sibling-window handle. */
                   ULONG _rid,       /*  Window resource identifier. */
                   PVOID pCtlData,   /*  Pointer to control data. */
                   PVOID pPresParams);/*  Presentation parameters. */
   FPM_Window(void)
   {
      dev_type  = FPM_W_DEVT_MEMORY;
      dev_class = FPM_W_DEV_MEMORY;
      nx = ny = 0;
      bytesPerPixel=bitsPerPixel=0;
      nColors = 0;
      id =_FreePM_id_index++;
      flStyle = 0;
      class_type = 0;
      pszClass = NULL;
      pszName  = NULL;
      rid = 0;
      hwndOwner = hwndParent = handle = 0;
   };

   int CreateFPM_Window(HWND hwndParent,  /*  Parent-window handle. */
                   PCSZ  _pszClass,   /*  Registered-class name. */
                   PCSZ  _pszName,    /*  Window text. */
                   ULONG _flStyle,    /*  Window style. */
                   LONG _x,          /*  x-coordinate of window position. */
                   LONG _y,          /*  y-coordinate of window position. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   HWND hwndOwner,   /*  Owner-window handle. */
                   HWND hwndInsertBehind, /*  Sibling-window handle. */
                   ULONG _rid,       /*  Window resource identifier. */
                   PVOID pCtlData,   /*  Pointer to control data. */
                   PVOID pPresParams);/*  Presentation parameters. */
   void SetHandle(int _hwnd)
   { handle = _hwnd;
   }
   HWND GetHandle(void)
   { return handle;
   }

//   int proc( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 );  /*  Window procedure pointer  (?) */
 virtual int proc(PQMSG pqmsg);
 int proc( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 ); /* From F_Window.cpp */
// int GetPS(F_PS *pps);
 HPS  GetPS(void);
 BOOL ReleasePS(HPS hps);

 virtual int Draw(HPS hps);
 int SendMsg_to_proc(ULONG umsg, MPARAM mp1, MPARAM mp2);
 int WinSetWindowPos(HWND hwndInsertBehind, LONG x, LONG y, LONG cx, LONG cy, ULONG fl);

};

/* Frame Window Class                */
class FRAME_Window:public FPM_Window
{
 public:
/* child windows */
//todo: those should be other window classes
   FPM_Window *pClient;
   FPM_Window *pTitleBar;
   FPM_Window *pSysMenu;
   FPM_Window *pMinMaxMenu;
   FPM_Window *pMenu;
   FPM_Window *pVertScroll;
   FPM_Window *pHorizScroll;
   FRAME_Window(void):FPM_Window()
   { pClient   = NULL;
     pTitleBar = NULL;
     pSysMenu  = NULL;
     pMinMaxMenu = NULL;
     pMenu       = NULL;
     pVertScroll  = NULL;
     pHorizScroll = NULL;
   }
   ~FRAME_Window(void)
   { if(pClient)
         delete(pClient);
     if(pTitleBar)
         delete(pTitleBar);
     if(pSysMenu)
         delete(pSysMenu);
     if(pMinMaxMenu)
         delete(pMinMaxMenu);
     if(pMenu)
         delete(pMenu);
     if(pVertScroll)
         delete(pVertScroll);
     if(pHorizScroll)
         delete(pHorizScroll);
   }
   int proc(PQMSG pqmsg);
};

/* Standard Window Class                */
/*  handle is FRAME Window handle;      */
/*   maybe it should be FRAME window ?? */
/* tothink: what is the diff between Standard Window and FRAME ? */
class STD_Window:public FRAME_Window
{
 public:
/* child windows */
//   FPM_Window Client;
//   FPM_Window TitleBar;
//   FPM_Window SysMenu;
//   FPM_Window Menu;
//   FPM_Window VertScroll;
//   FPM_Window HorizScroll;

//   STD_Window(void):FPM_Window(void)
//   {
//   }

   int CreateSTD_Window(HWND hwndParent,
               ULONG flStyle,
               PULONG pflCreateFlags,
               PCSZ pszClientClass,
               PCSZ pszTitle,
               ULONG styleClient,
               HMODULE hmod,
               ULONG idResources,
               PHWND phwndClient);

};
