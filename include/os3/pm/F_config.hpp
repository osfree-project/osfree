/* 
 $Id: F_config.hpp,v 1.5 2003/06/18 22:51:49 evgen2 Exp $ 
*/
/* F_config.hpp */
/* defines */
/* ver 0.02 21.08.2002 */

#ifndef FREEPMCONFIG
 #define FREEPMCONFIG

#include "F_PresPar.hpp"
#ifdef __cplusplus
      extern "C" {
#endif

/*+---------------------------------+*/
/*| defines for config FreePM       |*/
/*+---------------------------------+*/
#define FREEPM_VERSION         "0.0.0.1"
#define FREEPM_SERVER_APPLICATION_NAME  "FreePMserver"
#define FREEPM_CLIENT_APPLICATION_NAME  "FreePMclient"

#define USE_CIRCLE_QUEUES   	1
#define MAX_PIPE_BUF 		4096
#define FREEPMS_MAX_NUM_THREADS 32
/* #define THREAD_STACK_SIZE	32000 */
#define THREAD_STACK_SIZE	65536
#define USE_SOCKETS	   	0

//переодичность проверки конфига на изменения не чаще чем, в мсекундах
#define CHECK_CONFIG_TIME  1000

/* Ahtung!  the name of mutex smaphore shuld not be the same as VAC3
   project name !!!
*/
#define FREEPM_MUTEX_NAME            "\\SEM32\\_FREE_PM" /* Semaphore name */

struct GUI_WindowOptions
{   int nx,ny;          /* Window size */
    int bytesPerPixel;  /* байт на пиксел */
    int nColors;        /* colors per pixel: 2,16,256, 32k, 64k ... */
    struct WinPresParam pp; /* Presentation parameters */
};

struct FreePM_Config
{

    struct {
       char *log;
/*     char *access; */
       int rotateNumber;
    } Log;
//    char *debugOptions;

    struct {
       int  buffered_logs;
       int  UseANSI_stdout;
       int  UseBeepAtFatalError;
    } onoff;

    double Timeout_connection; /* client connection timeout */
/* GUI options */
    struct GUI_WindowOptions desktop; /*  default desktop options */
    struct GUI_WindowOptions window;  /*  default window  options */
};

#ifdef __cplusplus
      }
#endif

#endif
   /* FREEPMCONFIG */
