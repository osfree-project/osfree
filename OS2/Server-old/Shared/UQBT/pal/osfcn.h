
#ifndef OSFCN_H
#define OSFCN_H 1

#ifndef WIN32
#include <std.h>
#endif
#include <time.h>
#include <sys/types.h>
#if _G_HAVE_SYS_SOCKET
#include <sys/socket.h>
#endif
#if _G_HAVE_SYS_RESOURCE
#include <sys/time.h>
#include <sys/resource.h>
#endif


#endif
