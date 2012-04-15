/* F_debug.cpp */
/* ver 0.00 20.08.2002       */
/* DEBUG: section 0     Debug Routines */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "FreePM.hpp"

#include "F_globals.hpp"
#include <pmclient.h>

#define BUFSIZE 256

extern class NPipe *pF_pipe;

void fatalf(const char *fmt,...);
const char * xstrerror_type(int type);

void fatalf(const char *fmt,...);
const char * xstrerror_type(int type);

static char *debug_log_file = NULL;
static int Ctx_Lock = 0;
static const char *debugLogTime(time_t);
static void ctx_print(void);
#if HAVE_SYSLOG
static void _db_print_syslog(const char *format, va_list args);
#endif
static void _db_print_stderr(const char *format, va_list args);
static void _db_print_file(const char *format, va_list args);

#include <time.h>

int DebugCount=0;

extern "C" APIRET APIENTRY  _InitServerConnection(char *remotemachineName, ULONG *obj);
extern "C" APIRET APIENTRY  _CloseServerConnection(void);
extern "C" APIRET APIENTRY  _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size);
extern "C" APIRET APIENTRY  _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data);
extern "C" APIRET APIENTRY  _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen);
extern "C" APIRET APIENTRY  _F_SendDataToServer(void *recvobj, void *data, int len);
extern "C" APIRET APIENTRY  _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen);
extern "C" APIRET APIENTRY  _F_SendGenCmdToServer(void *recvobj, int cmd, int par);
extern "C" APIRET APIENTRY  _F_SendCmdToServer(void *recvobj, int ncmd, int data);
extern "C" void APIENTRY    _db_print(const char *format,...);

extern "C" void APIENTRY
_db_print(const char *format,...)
{
  char f[BUFSIZE];
  va_list args;

  memset(&f, 0, BUFSIZE);
  va_start(args, format);
  vsprintf(f, format, args);

  if (pF_pipe) 
  {
    if (!_F_SendCmdToServer(pF_pipe, F_CMD_DB_PRINT, 0))
      _F_SendDataToServer(pF_pipe, (void *)f, BUFSIZE);
  }
  
  va_end(args);
}

extern "C" void
_db_init(const char *logfile, const char *options) {}

void
_db_rotate_log(void) {}

void
xassert(const char *msg, const char *file, int line)
{
    debug(0, 0) ("assertion failed: %s:%d: \"%s\"\n", file, line, msg);
    //if (!_FreePM_shutting_down)
    abort();
}

/************************************/
/*   fatal* functions family        */
/************************************/
void fatalvf(const char *fmt, va_list args);
extern "C" void _fatal_common(const char *message);

void fatalf(const char *fmt,...);

extern "C" void
_fatal_common(const char *message)
{
  if(_FreePM_FatalInProgress) return;
  _FreePM_FatalInProgress = 1;

  if (pF_pipe) 
  {
    if (!_F_SendCmdToServer(pF_pipe, F_CMD_FATAL_COMMON, 0))
      _F_SendDataToServer(pF_pipe, (void *)message, strlen(message));
  }
}

/* fatal */
void APIENTRY _fatal(const char *message)
{
//todo
// for future usage with sockets
//    releaseServerSockets();
//todo
//    KillAllChilds();

    _fatal_common(message);
    //if (_FreePM_shutting_down)
    //   exit(0);
    //else
    abort();
}

/* printf-style interface for fatal */
void
fatalf(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);

    fatalvf(fmt, args);
    va_end(args);
}


/* used by fatalf */
void
fatalvf(const char *fmt, va_list args)
{
    static char fatal_str[BUFSIZE];
    vsnprintf(fatal_str, sizeof(fatal_str), fmt, args);
    _fatal(fatal_str);
}
