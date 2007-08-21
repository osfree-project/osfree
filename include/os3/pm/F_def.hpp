/*
 $Id: F_def.hpp,v 1.3 2003/06/18 22:51:49 evgen2 Exp $
*/
/* F_def.hpp */
/* defines */
/* ver 0.02 20.08.2002 */

#ifndef FREEPMDEF
  #define FREEPMDEF
#include "F_basedef.hpp"

#ifdef __cplusplus
      extern "C" {
#endif

/*+---------------------------------+*/
/*| defines for class FPM_Window    |*/
/*+---------------------------------+*/
/*  int dev_class;   device class */

#define FPM_W_DEV_MEMORY    0
#define FPM_W_DEV_VIDEO     1
#define FPM_W_DEV_PRINTER   2

/* int dev_type; device type */

#define FPM_W_DEVT_MEMORY       0

#define FPM_W_DEVT_PM           1
#define FPM_W_DEVT_GENGRADD     2
#define FPM_W_DEVT_SDDNUCLEUS   3



/*** Predefined window handles ********/
#define HWND_DESKTOP               (HWND)-1
#define HWND_TOP                   (HWND)-3
#define HWND_BOTTOM                (HWND)-4

/*** Predefined window handles in PM !!! ********/

   #define PM_HWND_DESKTOP               (HWND)1
   #define PM_HWND_OBJECT                (HWND)2
   #define PM_HWND_TOP                   (HWND)3
   #define PM_HWND_BOTTOM                (HWND)4
   #define PM_HWND_THREADCAPTURE         (HWND)5
/* Utility for conversion FreePM to PM predefined window handles */
/* Should be used called from PM_api.cpp  */
HWND  F_convertHandleToPM(HWND hwnd);

/* Debug messages & info */
#define MAX_DEBUG_SECTIONS         64
#define debug(SECTION, LEVEL) \
        ((_FreePM_db_level = (LEVEL)) > _FreePM_debugLevels[SECTION]) ? (void) 0 : _db_print
extern void _db_print(const char *,...);
extern void _db_init(const char *logfile, const char *options);
extern void _db_rotate_log(void);
extern const char * xstdio_strerror(void);

void fatalf(const char *fmt,...);

/* independent version of strerror: XXX VAC strerror - work only with VAC installed */
extern const char *StdStrError(int ierr);
char * GetOS2ErrorMessage(int ierr);
char * GetFreePMErrorMessage(int ierr);


/*  ANSI Console Debug color's */
#define FATAL_COLOR        "[1;31;40m"
#define WARNING_COLOR      "[1;33;40m"
#define IGNORING_COLOR     "[1;35;40m"
#define NORMAL_COLOR       "[0;37;40m"
#define DISABLE_COLOR      "[2;37;40m"

/* handle type */
#define  FD_NONE       0
#define  FD_LOG                1
#define  FD_FILE       2
#define  FD_SOCKET     3
#define  FD_PIPE       4
#define  FD_PIPE_READ  5
#define  FD_PIPE_WRITE 6
#define  FD_UNKNOWN    7


#ifdef __cplusplus
        }
#endif

#endif
   /* FREEPMDEF */
