
/*
   PM123 Plugin Definitions
   Copyright (C) 1998 Taneli Lepp„ <rosmo@kalja.com>
                      Samuel Audet <guardia@cam.org>
*/

#define PLUGIN_NULL    0x000
#define PLUGIN_VISUAL  0x001
#define PLUGIN_FILTER  0x002
#define PLUGIN_DECODER 0x004
#define PLUGIN_OUTPUT  0x008

/* see decoder_plug.h and output_plug.h for more information
   on some of these functions */
typedef struct {
  ULONG (* _System output_playing_samples)(FORMAT_INFO *info, char *buf, int len);
  BOOL  (* _System decoder_playing)();
  ULONG (* _System output_playing_pos)();
  ULONG (* _System decoder_status)();
  ULONG (* _System decoder_command)(ULONG msg, DECODER_PARAMS *params);
  /* name is the DLL filename of the decoder that can play that file */
  ULONG (* _System decoder_fileinfo)(char *filename, DECODER_INFO *info, char *name);

  int (*_System specana_init)(int setnumsamples);
  /* int specana_init(int setnumsamples);
     setnumsamples must be a power of 2
     Returns the number of bands in return (setnumsamples/2+1).
  */
  int (*_System specana_dobands)(float bands[]);
  /*
     int specana_dobands(float bands[]);
     Returns the max value.
  */

  int (*_System pm123_getstring)(int index, int subindex, size_t bufsize, char *buf);
  void (* _System pm123_control)(int index, void *param);

  /* name is the DLL filename of the decoder that can play that track */
  ULONG (* _System decoder_trackinfo)(char *drive, int track, DECODER_INFO *info, char *name);
  ULONG (* _System decoder_cdinfo)(char *drive, DECODER_CDINFO *info);
  ULONG (* _System decoder_length)();

} PLUGIN_PROCS, *PPLUGIN_PROCS;

/*
  int pm123_getstring(int index, int subindex, int bufsize, char *buf)

    index    - which string (see STR_* defines below)
    subindex - not currently used
    bufsize  - bytes in buf
    buf      - buffer for the string
*/

#define STR_NULL         0
#define STR_VERSION      1 /* PM123 version          */
#define STR_DISPLAY_TEXT 2 /* Display text           */
#define STR_FILENAME     3 /* Currently loaded file  */

/*
  int pm123_control(int index, void *param);

    index - operation
    param - parameter for the operation
*/
#define CONTROL_NEXTMODE 1  /* Next display mode */

typedef struct {
  int           x, y, cx, cy;   /* Input        */
  HWND          hwnd;           /* Input/Output */
  PPLUGIN_PROCS procs;          /* Input        */
  int           id;             /* Input        */
  char          *param;         /* Input        */
  HAB           hab;            /* Input        */
} VISPLUGININIT, *PVISPLUGININIT;

typedef struct
{
 int  type;         /* null, visual, filter, input. values can be ORred */
 char *author;      /* Author of the plugin                             */
 char *desc;        /* Description of the plugin                        */
 int  configurable; /* Is the plugin configurable                       */
} PLUGIN_QUERYPARAM, *PPLUGIN_QUERYPARAM;


/* Plugin notify message */
#define WM_PLUGIN_CONTROL (WM_USER + 666)
/*
   WM_PLUGIN_CONTROL
     LONGFROMMP(mp1) = notify code
     LONGFROMMP(mp2) = additional information
*/
/* Notify codes */
#define PN_TEXTCHANGED  1   /* Display text changed */
