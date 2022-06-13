/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/* 
 * $Id$
 */

#ifdef __cplusplus
#	ifdef HAVE_CONFIG_HPP
#		include <config.hpp>
#	else
#		ifdef HAVE_CONFIG_H
#			include <config.h>
#		endif
#	endif
#	define MAINLINE 				int main(int argc,char **argv)
#else
#	ifdef HAVE_CONFIG_H
#		include <config.h>
#	endif
#	define MAINLINE 				int main(argc,argv) int argc; char **argv;
#endif

#define COMPLETE_PROGRAM 			MAINLINE { return argv && !argc; }

#ifdef LIST_OPTIONS
HAVE_OS2_H							"Looking for <os2.h>"
HAVE_PTHREAD_H						"Looking for <pthread.h>"
HAVE_SYS_TYPES_H					"Looking for <sys/types.h>"
HAVE_LIMITS_H						"Looking for <limits.h>"
HAVE_SIGNAL_H						"Looking for <signal.h>"
HAVE_POSIX_SIGWAIT					"Looking for sigwait()"
_POSIX_PTHREAD_SEMANTICS			"Looking for _POSIX_PTHREAD_SEMANTICS"
HAVE_STDIO_H						"Looking for <stdio.h>"
HAVE_STDARG_H						"Looking for <stdarg.h>"
HAVE_STDINT_H						"Looking for <stdint.h>"
HAVE_WCHAR_H						"Looking for <wchar.h>"
HAVE_DLFCN_H						"Looking for <dlfcn.h>"
HAVE_DL_H							"Looking for <dl.h>"
HAVE_SYS_LDR_H						"Looking for <sys/ldr.h>"
HAVE_MACHO_DYLD_H					"Looking for <mach-o/dyld.h>"
#ifdef _WIN32
HAVE_WINSOCK_H						"Looking for <winsock.h>"
HAVE_WINSOCK2_H						"Looking for <winsock2.h>"
HAVE_WININET_H						"Looking for <wininet.h>"
HAVE_WS2IP6_H						"Looking for <ws2ip6.h>"
HAVE_WS2TCPIP_H						"Looking for <ws2tcpip.h>"
HAVE_WSPIAPI_H						"Looking for <wspiapi.h>"
HAVE_WSIPX_H						"Looking for <wsipx.h>"
HAVE_ATALKWSH_H						"Looking for <atalkwsh.h>"
HAVE_NB30_H							"Looking for <nb30.h>"
#else
HAVE_WINTYPES_H						"Looking for <wintypes.h>"
#endif
HAVE_UNISTD_H						"Looking for <unistd.h>"
HAVE_ERRNO_H						"Looking for <errno.h>"
HAVE_POLL_H							"Looking for <poll.h>"
HAVE_GRP_H							"Looking for <grp.h>"
HAVE_FCNTL_H						"Looking for <fcntl.h>"
HAVE_MEMORY_H						"Looking for <memory.h>"
HAVE_SETJMP_H						"Looking for <setjmp.h>"
HAVE_SETJMPEX_H						"Looking for <setjmpex.h>"
HAVE_STRING_H						"Looking for <string.h>"
HAVE_STRINGS_H						"Looking for <strings.h>"
HAVE_SIGINFO_H						"Looking for <siginfo.h>"
HAVE_SYS_POLL_H						"Looking for <sys/poll.h>"
HAVE_SYS_SELECT_H					"Looking for <sys/select.h>"
HAVE_SYS_TIME_H						"Looking for <sys/time.h>"
HAVE_SYS_SOCKET_H					"Looking for <sys/socket.h>"
HAVE_SYS_IOCTL_H					"Looking for <sys/ioctl.h>"
HAVE_SYS_FILE_H						"Looking for <sys/file.h>"
HAVE_SYS_FCNTL_H					"Looking for <sys/fcntl.h>"
HAVE_SYS_SOCKIO_H					"Looking for <sys/sockio.h>"
HAVE_SYS_SYSTEMINFO_H				"Looking for <sys/systeminfo.h>"
HAVE_SYS_UUID_H						"Looking for <sys/uuid.h>"
HAVE_SYS_ETHERNET_H					"Looking for <sys/ethernet.h>"
HAVE_SYS_UIO_H						"Looking for <sys/uio.h>"
HAVE_UUID_UUID_H					"Looking for <uuid/uuid.h>"
HAVE_UUID_H							"Looking for <uuid.h>"
HAVE_NET_IF_H						"Looking for <net/if.h>"
HAVE_NET_IF_ARP_H					"Looking for <net/if_arp.h>"
HAVE_NET_IF_DL_H					"Looking for <net/if_dl.h>"
HAVE_NET_SLIP_H						"Looking for <net/slip.h>"
HAVE_NET_ROUTE_H					"Looking for <net/route.h>"
HAVE_NETINET_IN_H					"Looking for <netinet/in.h>"
HAVE_NETINET_IF_ETHER_H				"Looking for <netinet/if_ether.h>"
HAVE_ARPA_INET_H					"Looking for <arpa/inet.h>"
HAVE_NETAT_APPLETALK_H				"Looking for <netat/appletalk.h>"
HAVE_NETAT_AT_VAR_H					"Looking for <netat/at_var.h>"
HAVE_APPLETALK_AT_PROTO_H			"Looking for <AppleTalk/at_proto.h>"
HAVE_AT_APPLETALK_H					"Looking for <at/appletalk.h>"
HAVE_NETATALK_AT_H					"Looking for <netatalk/at.h>"
HAVE_ATALK_NBP_H					"Looking for <atalk/nbp.h>"
HAVE_NETIPX_IPX_H					"Looking for <netipx/ipx.h>"
HAVE_NETDB_H						"Looking for <netdb.h>"
HAVE_TERMIOS_H						"Looking for <termios.h>"
HAVE_NL_TYPES_H						"Looking for <nl_types.h>"
HAVE_LANGINFO_H						"Looking for <langinfo.h>"
HAVE_LANGINFO_CODESET				"Looking for CODESET"
HAVE_USB_H							"Looking for <usb.h>"
HAVE_PCSCLITE_H						"Looking for <pcsclite.h>"
HAVE_WINSCARD_H						"Looking for <winscard.h>"
HAVE_PCSC_WINSCARD_H				"Looking for <PCSC/winscard.h>"
HAVE_READER_H						"Looking for <reader.h>"
HAVE_X11_XLIB_H						"Looking for <X11/Xlib.h>"
HAVE_X11_EXTENSIONS_PRINT_H			"Looking for <X11/extensions/Print.h>"
HAVE_X11_EXTENSIONS_SHAPE_H			"Looking for <X11/extensions/shape.h>"
HAVE_X11_XPM_H						"Looking for <X11/xpm.h>"
HAVE_X11_SGISCHEME_H				"Looking for <X11/SGIScheme.h>"
HAVE_XM_XM_H						"Looking for <Xm/Xm.h>"
HAVE_XM_XMALL_H						"Looking for <Xm/XmAll.h>"
HAVE_COREFOUNDATION_CFSTRING_H		"Looking for <CoreFoundation/CFString.h>"
HAVE_COREFOUNDATION_CFUUID_H		"Looking for <CoreFoundation/CFUUID.h>"
HAVE_COREFOUNDATION_CFPLUGIN_H		"Looking for <CoreFoundation/CFPlugIn.h>"
HAVE_COREFOUNDATION_CFPLUGINCOM_H	"Looking for <CoreFoundation/CFPlugInCOM.h>"
HAVE_SEH_TRY_EXCEPT					"Looking for __try/__except"
HAVE_SEH_TRY_FINALLY				"Looking for __try/__finally"
HAVE_DECLSPEC_DLLEXPORT				"Looking for __declspec(dllexport)"
HAVE_DECLSPEC_DLLIMPORT				"Looking for __declspec(dllimport)"
HAVE_LONG_DOUBLE					"Looking for long double"
HAVE_SOCKADDR_SA_LEN				"Looking for sockaddr.sa_len"
HAVE_SOCKLEN_T						"Looking for socklen_t"
HAVE_IN_ADDR_T						"Looking for in_addr_t"
HAVE_IN6_ADDR						"Looking inf in6_addr"
HAVE_IN_ADDR6						"Looking inf in_addr6"
HAVE_SOCKADDR_IN6					"Looking for sockaddr_in6"
HAVE_SOCKADDR_AT					"Looking for sockaddr_at"
HAVE_SOCKADDR_SAT_TYPE				"Looking for sockaddr_at.sat_type"
HAVE_SOCKADDR_IPX					"Looking for sockaddr_ipx"
HAVE_SOCKADDR_SIPX_NETWORK			"Looking for sockaddr_ipx.sipx_network"
HAVE_SOCKADDR_SIPX_NODE				"Looking for sockaddr_ipx.sipx_node"
HAVE_SOCKADDR_SIPX_TYPE				"Looking for sockaddr_ipx.sipx_type"
HAVE_SOCKADDR_SIPX_PORT				"Looking for sockaddr_ipx.sipx_port"
HAVE_SOCKADDR_SIPX_ADDR_X_NET_C_NET "Looking for sockaddr_ipx.sipx_addr.x_net.c_net"
HAVE_SOCKADDR_SIPX_ADDR_X_HOST_C_HOST "Looking for sockaddr_ipx.sipx_addr.x_host.c_host"
HAVE_MSGHDR_MSG_ACCRIGHTS			"Looking for msghdr.msg_accrights"
HAVE_MSGHDR_MSG_CONTROL				"Looking for msg.msg_control"
HAVE_HOSTENT_DATA					"Looking for hostent_data"
HAVE_FPOS_T_SCALAR					"Looking for (long)fpos_t"
HAVE_VA_COPY						"Looking for va_copy()"
HAVE_VA_COPY_VOLATILE				"Looking for va_copy(volatile)"
HAVE_VA_LIST_ASSIGN					"Looking for va_list assign"
HAVE_VA_LIST_SCALAR					"Looking for va_list scalar"
HAVE_VA_LIST_OVERFLOW_ARG_AREA		"Looking for va_list.overflow_arg_area"
HAVE_VA_LIST_REG_SAVE_AREA			"Looking for va_list.reg_save_area"
HAVE_VA_LIST_GPR					"Looking for va_list.gpr"
HAVE_VA_LIST_FPR					"Looking for va_list.fpr"
HAVE_VA_LIST_GP_OFFSET				"Looking for va_list.gp_offset"
HAVE_VA_LIST_FP_OFFSET				"Looking for va_list.fp_offset"
HAVE_VA_LIST__OFFSET				"Looking for va_list._offset"
HAVE_VA_LIST__A0					"Looking for va_list._a0"
HAVE_VA_LIST___AP					"Looking for va_list.__ap"
HAVE_VA_LIST___VA_GP_OFFSET			"Looking for va_list.__va_gp_offset"
HAVE_VA_LIST___VA_FP_OFFSET			"Looking for va_list.__va_fp_offset"
HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA	"Looking for va_list.__va_overflow_arg_area"
HAVE_VA_LIST___VA_REG_SVE_AREA		"Looking for va_list.__va_reg_sve_area"
HAVE___VA_REGSAVE_T					"Looking for typedef struct __va_regsave_t"
HAVE_VA_END_EMPTY					"Looking for va_end(ignored)"
HAVE_VA_ARG_ASSIGN					"Looking for va_arg assign"
HAVE_VA_ARG_VOLATILE				"Looking for va_arg volatile"
HAVE_MEMMOVE						"Looking for memmove()"
HAVE_STRCASECMP						"Looking for strcasecmp()"
HAVE_STRICMP						"Looking for stricmp()"
HAVE__STRICMP						"Looking for _stricmp()"
HAVE_SNPRINTF						"Looking for snprintf()"
HAVE__SNPRINTF						"Looking for _snprintf()"
HAVE_VSNPRINTF						"Looking for vsnprintf()"
HAVE__VSNPRINTF						"Looking for _vsnprintf()"
HAVE_INT_SPRINTF					"Looking for int sprintf()"
HAVE_INT_VSPRINTF					"Looking for int vsprintf()"
HAVE_PTR_SPRINTF					"Looking for char *sprintf()"
HAVE_PTR_VSPRINTF					"Looking for char *vsprintf()"
HAVE_SWPRINTF_ISO					"Looking for ISO style swprintf()"
HAVE_SWPRINTF						"Looking for swprintf()"
HAVE_TMPFILE						"Looking for tmpfile()"
HAVE_RAISE							"Looking for raise()"
HAVE_SIGINTERRUPT					"Looking for siginterrupt()"
HAVE_PSIGNAL						"Looking for psignal()"
HAVE_SA_HANDLER_DEFAULT_EMPTY		"Looking for (*sa_handler)()"
HAVE_SA_HANDLER_DEFAULT_INT			"Looking for (*sa_handler)(int)"
HAVE_SA_HANDLER_DEFAULT_KANDR		"Looking for (*sa_handler)(int) K&R"
HAVE_SA_HANDLER_INT_INT				"Looking for int (*sa_handler)(int)"
HAVE_SA_HANDLER_VOID_EMPTY			"Looking for void (*sa_handler)()"
HAVE_SA_HANDLER_VOID_VOID			"Looking for void (*sa_handler)(void)"
HAVE_SA_HANDLER_VOID_INT			"Looking for void (*sa_handler)(int)"
HAVE_SA_HANDLER_VOID_KANDR			"Looking for void (*sa_handler)(int) K&R"
HAVE_ATEXIT_VOID					"Looking for atexit(void (*)(void))"
HAVE_ATEXIT_INT						"Looking for atexit((*)())"
HAVE_ATEXIT							"Looking for atexit()"
HAVE_STRERROR						"Looking for strerror()"
HAVE_PERROR							"Looking for perror()"
HAVE_VFORK							"Looking for vfork()"
HAVE_FLOCK							"Looking for flock()"
HAVE_IOCTL_FIONBIO					"Looking for ioctl(FIONBIO)"
HAVE_FCNTL_F_SETFL_FNDELAY			"Looking for fcntl(F_SETFL,FNDELAY)"
HAVE_FCNTL_F_SETFL_O_NDELAY			"Looking for fcntl(F_SETFL,O_NDELAY)"
HAVE_READLINK						"Looking for readlink()"
HAVE_REALPATH						"Looking for realpath()"
HAVE_CFMAKERAW						"Looking for cfmakeraw()"
HAVE_UUID_CREATE					"Looking for uuid_create()"
HAVE_UUID_GENERATE					"Looking for uuid_generate()"
HAVE_SYSINFO_SVR4					"Looking for SVR4 style sysinfo"
HAVE_SETPGRP_SYSV					"Looking for System V style setpgrp()"
HAVE_SETPGRP_BSD					"Looking for BSD style setpgrp()"
HAVE_PTSNAME						"Looking for ptsname()"
HAVE__GETPTY						"Looking for _getpty()"
HAVE_POSIX_OPENPT					"Looking for posix_openpt()"
HAVE_DLOPEN							"Looking for dlopen()"
HAVE_GETPWUID_R						"Looking for getpwuid_r()"
HAVE_READDIR_R						"Looking for readdir_r()"
HAVE_RECVMSG						"Looking for recvmsg()"
HAVE_SENDMSG						"Looking for sendmsg()"
HAVE_ETHER_HOSTTON					"Looking for ether_hostton()"
HAVE_GETIPNODEBYNAME				"Looking for getipnodebyname()"
HAVE_GETIPNODEBYADDR				"Looking for getipnodebyaddr()"
HAVE_FREEHOSTENT					"Looking for freehostent()"
HAVE_GETHOSTBYNAME_R_LINUX			"Looking for Linux style gethostbyname_r()"
HAVE_GETHOSTBYADDR_R_LINUX			"Looking for Linux style gethostbyaddr_r()"
HAVE_GETHOSTBYNAME_R_SOLARIS		"Looking for Solaris style gethostbyname_r()"
HAVE_GETHOSTBYADDR_R_SOLARIS		"Looking for Solaris style gethostbyaddr_r()"
HAVE_GETNAMEINFO					"Looking for getnameinfo()"
HAVE_GETADDRINFO					"Looking for getaddrinfo()"
HAVE_FREEADDRINFO					"Looking for freeaddrinfo()"
HAVE_NBP_LOOKUP						"Looking for nbp_lookup()"
HAVE_PTHREAD_ATFORK					"Looking for pthread_atfork()"
HAVE_PTHREAD_CANCEL					"Looking for pthread_cancel()"
HAVE_PTHREAD_KILL					"Looking for pthread_kill()"
HAVE_PTHREAD_T_SCALAR				"Looking for scalar pthread_t"
HAVE_PTHREAD_ADDR_T					"Looking for pthread_addr_t"
HAVE_PTHREAD_KEY_CREATE				"Looking for pthread_key_create()"
HAVE_PTHREAD_KEY_DELETE				"Looking for pthread_key_delete()"
HAVE_PTHREAD_KEYCREATE				"Looking for pthread_keycreate()"
HAVE_PTHREAD_GETSPECIFIC_STD		"Looking for pthread_getspecific() - Standard"
HAVE_PTHREAD_GETSPECIFIC_D4			"Looking for pthread_getspecific() - Draft4"
HAVE_PTHREAD_ATTR_DEFAULT			"Looking for pthread_attr_default"
HAVE_PTHREAD_CONDATTR_DEFAULT		"Looking for pthread_condattr_default"
HAVE_PTHREAD_MUTEXATTR_DEFAULT		"Looking for pthread_mutexattr_default"
HAVE_CFRELEASE						"Looking for CFRelease()"
HAVE_SCARDESTABLISHCONTEXT			"Looking for SCardEstablishContext()"
HAVE_SCARDLISTREADERGROUPS			"Looking for SCardListReaderGroups()"
HAVE_WINSCARD_UNSIGNED_LONG			"Looking for SCardListReaders(unsigned long)"
HAVE_INTERLOCKEDINCREMENT_VOLATILE	"Looking for InterlockedIncrement(volatile...)"
HAVE_INTERLOCKEDEXCHANGE			"Looking for InterlockedExchange()"
HAVE_WAITFORSINGLEOBJECTEX			"Looking for WaitForSingleObjectEx()"
HAVE_D2I_X509_CONST					"Looking for d2i_x509(...const...)"
HAVE_XTDEFAULTAPPCONTEXT			"Looking for _XtDefaultAppContext()"
HAVE_XTSETLANGUAGEPROC				"Looking for XtSetLanguageProc()"
HAVE_XTAPPSETEXITFLAG				"Looking for XtAppSetExitFlag()"
HAVE_XEDITRESCHECKMESSAGES			"Looking for _XEditResCheckMessages()"
HAVE_XTDISPATCHEVENTTOWIDGET		"Looking for XtDispatchEventToWidget()"
HAVE_XPCREATECONTEXT				"Looking for XpCreateContext()"
HAVE_IIDFROMSTRING_LPCOLESTR		"Looking for IIDFromString(LPCOLESTR,IID*)"
HAVE_UNREGISTERTYPELIB				"Looking for UnRegisterTypeLib()"
USE_PTHREAD_ONCE_INIT_BRACES0		"Looking for PTHREAD_ONCE_INIT"
USE_PTHREAD_ONCE_INIT_BRACES1		"Looking for {PTHREAD_ONCE_INIT}"
USE_PTHREAD_ONCE_INIT_BRACES2		"Looking for {{PTHREAD_ONCE_INIT}}"
USE_PTHREAD_MUTEX_INITIALIZER_BRACES0		"Looking for PTHREAD_MUTEX_INITIALIZER"
USE_PTHREAD_MUTEX_INITIALIZER_BRACES1		"Looking for {PTHREAD_MUTEX_INITIALIZER}"
USE_PTHREAD_MUTEX_INITIALIZER_BRACES2		"Looking for {{PTHREAD_MUTEX_INITIALIZER}}"
USE_PTHREAD_COND_INITIALIZER_BRACES0		"Looking for PTHREAD_COND_INITIALIZER"
USE_PTHREAD_COND_INITIALIZER_BRACES1		"Looking for {PTHREAD_COND_INITIALIZER}"
USE_PTHREAD_COND_INITIALIZER_BRACES2		"Looking for {{PTHREAD_COND_INITIALIZER}}"
USE_IN6ADDR_ANY_INIT_BRACES0		"Looking for IN6ADDR_ANY_INIT"
USE_IN6ADDR_ANY_INIT_BRACES1		"Looking for {IN6ADDR_ANY_INIT}"
USE_IN6ADDR_ANY_INIT_BRACES2		"Looking for {{IN6ADDR_ANY_INIT}}"
#elif defined(TRY_HAVE_SYS_TIME_H)
#	include <sys/time.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_SOCKET_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_UUID_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/uuid.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_UUID_UUID_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <uuid/uuid.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_UUID_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <uuid.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_ETHERNET_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/ethernet.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_UIO_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/uio.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NL_TYPES_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <nl_types.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_LANGINFO_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_NL_TYPES_H
#		include <nl_types.h>
#	endif
#	include <langinfo.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_LANGINFO_CODESET)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_NL_TYPES_H
#		include <nl_types.h>
#	endif
#	ifdef HAVE_LANGINFO_H
#		include <langinfo.h>
#	endif
	MAINLINE
	{ nl_langinfo(CODESET); return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_GRP_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <grp.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_DLFCN_H)
#	ifdef BUILD_STATIC
		#error BUILD_STATIC
#	else
#		ifdef HAVE_SYS_TYPES_H
#			include <sys/types.h>
#		endif
#		include <dlfcn.h>
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_HAVE_MACHO_DYLD_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <mach-o/dyld.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_DL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <dl.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_LDR_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/ldr.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_POLL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <poll.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_MEMORY_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <memory.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SETJMP_H)
#	include <setjmp.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SETJMPEX_H)
#	include <setjmpex.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_FCNTL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <fcntl.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_TYPES_H)
#	include <sys/types.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SIGNAL_H)
#	include <signal.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SIGINFO_H)
#	ifdef HAVE_SIGNAL_H
#		include <signal.h>
#	endif
#	include <siginfo.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_STDARG_H)
#	include <stdarg.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_STDINT_H)
#	include <stdint.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_LIMITS_H)
#	include <limits.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_STDIO_H)
#	include <stdio.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WCHAR_H)
#	include <wchar.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_ERRNO_H)
#	include <errno.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_UNISTD_H)
#	include <unistd.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_STRING_H)
#	include <string.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_STRINGS_H)
#	include <strings.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_POLL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/poll.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_SELECT_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/select.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_SYSTEMINFO_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/systeminfo.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_IOCTL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/ioctl.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_FILE_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/file.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_FCNTL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/fcntl.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SYS_SOCKIO_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <sys/sockio.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETINET_IN_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netinet/in.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETINET_IF_ETHER_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	ifdef HAVE_NET_IF_H
#	include <net/if.h>
#	endif
#	include <netinet/in.h>
#	include <netinet/if_ether.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETDB_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_USB_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <usb.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_PCSCLITE_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <pcsclite.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WINSCARD_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <winscard.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_PCSC_WINSCARD_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <PCSC/winscard.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_READER_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <reader.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WINTYPES_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <wintypes.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_ARPA_INET_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NET_IF_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <net/if.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NET_IF_ARP_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <net/if.h>
#	include <net/if_arp.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NET_IF_DL_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <net/if.h>
#	include <net/if_dl.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NET_SLIP_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <net/slip.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NET_ROUTE_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <net/route.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETATALK_AT_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netatalk/at.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETIPX_IPX_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_TERMIOS_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SYS_IOCTL_H
#		include <sys/ioctl.h>
#	endif
#	include <termios.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_PTHREAD_H)
#	include <pthread.h>
	MAINLINE
	{ void *pv;	pthread_t t=pthread_self(); 
		return argc && argv && pthread_join(t,&pv);	}
#elif defined(TRY_HAVE_X11_XLIB_H)
#	include <X11/Xlib.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_X11_EXTENSIONS_PRINT_H)
#	include <X11/extensions/Print.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_X11_EXTENSIONS_SHAPE_H)
#	include <X11/Intrinsic.h>
#	include <X11/extensions/shape.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_X11_XPM_H)
#	include <X11/xpm.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_X11_SGISCHEME_H)
#	include <X11/Xresource.h>
#	include <X11/SGIScheme.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_XM_XM_H)
#	include <Xm/Xm.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_XM_XMALL_H)
#	include <Xm/XmAll.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_COREFOUNDATION_CFSTRING_H)
#	include <CoreFoundation/CFString.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_COREFOUNDATION_CFUUID_H)
#	include <CoreFoundation/CFUUID.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_COREFOUNDATION_CFPLUGIN_H)
#	include <CoreFoundation/CFPlugIn.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_COREFOUNDATION_CFPLUGINCOM_H)
#	include <CoreFoundation/CFPlugInCOM.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_CFRELEASE)
#	include <CoreFoundation/CFString.h>
	MAINLINE
	{   void *pv=NULL; CFRelease(pv);
		return argc && argv && pv; }
#elif defined(TRY_HAVE_VFORK)
#	include <unistd.h>
	MAINLINE
	{ return argc && argv && vfork(); }
#elif defined(TRY_HAVE_READLINK)
#	include <unistd.h>
	MAINLINE
	{ char buf[256]; return readlink(argv[1],buf,sizeof(buf)); }
#elif defined(TRY_HAVE_REALPATH)
#ifdef HAVE_LIMITS_H
#	include <limits.h>
#endif
#	include <stdlib.h>
	MAINLINE
	{ char buf[PATH_MAX]; 
	  const char *p=realpath(argv[1],buf); return p ? 0 : 1;}
#elif defined(TRY_HAVE_STRERROR)
#	ifdef HAVE_STRING_H
#		include <string.h>
#	endif
#	include <errno.h>
	MAINLINE
	{   const char *p=strerror(errno);
		return argc && argv && p; }
#elif defined(TRY_HAVE_PERROR)
#	include <stdio.h>
	MAINLINE
	{   perror(argv[0]);
		return argc; }
#elif defined(TRY_HAVE_FLOCK)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SYS_FILE_H
#		include <sys/file.h>
#	endif
#	include <unistd.h>
	MAINLINE
	{ return argc && argv && flock(0,LOCK_NB); }
#elif defined(TRY_HAVE_IOCTL_FIONBIO)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SYS_IOCTL_H
#		include <sys/ioctl.h>
#	endif
#	ifdef HAVE_UNISTD_H
#		include <unistd.h>
#	endif
	MAINLINE
	{ int ul=argc; return argv && ioctl(ul,FIONBIO,(char *)&ul); }
#elif defined(TRY_HAVE_FCNTL_F_SETFL_FNDELAY)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_UNISTD_H
#		include <unistd.h>
#	endif
#	ifdef HAVE_FCNTL_H
#		include <fcntl.h>
#	endif
	MAINLINE
	{ return argc && argv && fcntl(0,F_SETFL,FNDELAY); }
#elif defined(TRY_HAVE_FCNTL_F_SETFL_O_NDELAY)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_UNISTD_H
#		include <unistd.h>
#	endif
#	ifdef HAVE_FCNTL_H
#		include <fcntl.h>
#	endif
	MAINLINE
	{ return argc && argv && fcntl(0,F_SETFL,O_NDELAY); }
#elif defined(TRY_HAVE_ATEXIT_VOID)
#	include <stdlib.h>
	static void myexit(void) {}
	MAINLINE
	{ atexit(myexit); return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_ATEXIT_INT)
#	include <stdlib.h>
	static myexit() {}
	MAINLINE
	{ atexit(myexit); return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_ATEXIT)
#	if defined(HAVE_ATEXIT_INT)||defined(HAVE_ATEXIT_VOID)
		COMPLETE_PROGRAM
#	else
		#error no recognized atexit
#	endif
#elif defined(TRY_HAVE_SEH_TRY_EXCEPT)
	MAINLINE
	{ __try {} __except(1) {} return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SEH_TRY_FINALLY)
	MAINLINE
	{ __try {} __finally {} return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_DECLSPEC_DLLEXPORT)
#	ifdef BUILD_STATIC
		#error BUILD_STATIC
#	endif
	__declspec(dllexport) int myfunc(int x) { return x; }
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_DECLSPEC_DLLIMPORT)
#	ifdef BUILD_STATIC
		#error BUILD_STATIC
#	endif
	__declspec(dllimport) int myfunc(int);
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_LONG_DOUBLE)
	long double my_ld=1.2;
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_SOCKADDR_SA_LEN)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
	MAINLINE
	{ static struct sockaddr addr;
	  return (sizeof(addr.sa_len) && (!addr.sa_len) && argc && argv) ? 0 : 1;	}
#elif defined(TRY_HAVE_SOCKLEN_T)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SYS_SOCKET_H
#		include <sys/socket.h>
#	else
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			ifdef HAVE_WINSOCK_H
#				include <winsock.h>
#			endif
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#	endif
	MAINLINE
	{ static struct sockaddr addr; socklen_t len=sizeof(addr);
	  return getsockname(0,&addr,&len) && argc && argv;	}
#elif defined(TRY_HAVE_IN_ADDR_T)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SYS_SOCKET_H
#		include <sys/socket.h>
#	else
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			ifdef HAVE_WINSOCK_H
#				include <winsock.h>
#			endif
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#	endif
#	ifdef HAVE_NETINET_IN_H
#		include <netinet/in.h>
#	endif
	MAINLINE
	{ in_addr_t addr=0x7F000001;
	  return (addr==0x7F000001) && argc && argv;	}
#elif defined(TRY_HAVE_SCARDESTABLISHCONTEXT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdio.h>
#	ifdef HAVE_PCSC_WINSCARD_H
#		include <PCSC/winscard.h>
#	else
#		include <winscard.h>
#	endif
#	ifdef _WIN32
#		pragma comment(lib,"winscard.lib")
#	endif
	MAINLINE
	{ 
		return (argc && argv) ? SCardEstablishContext(0,0,0,NULL) : 1; }
#elif defined(TRY_HAVE_SCARDLISTREADERGROUPS)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdio.h>
#	ifdef HAVE_PCSC_WINSCARD_H
#		include <PCSC/winscard.h>
#	else
#		include <winscard.h>
#	endif
#	ifdef _WIN32
#		pragma comment(lib,"winscard.lib")
#	endif
	MAINLINE
	{ 
		return (argc && argv) ? SCardListReaderGroups(0,NULL,NULL) : 1; }
#elif defined(TRY_HAVE_WINSCARD_UNSIGNED_LONG)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdio.h>
#	ifdef HAVE_PCSC_WINSCARD_H
#		include <PCSC/winscard.h>
#	else
#		include <winscard.h>
#	endif
#	ifdef _WIN32
#		pragma comment(lib,"winscard.lib")
#	endif
	MAINLINE
	{ 
		unsigned long cch=0;
		return (argc && argv) ? SCardListReaders(0,NULL,NULL,&cch) : 1; }
#elif defined(TRY_HAVE_INTERLOCKEDINCREMENT_VOLATILE)
#	include <windows.h>
	MAINLINE
	{ 
LONG (__stdcall *myProc)(volatile LONG  *pv)=InterlockedIncrement;
static volatile LONG l=-2;
	myProc(&l);
	return InterlockedIncrement(&l); }
#elif defined(TRY_HAVE_INTERLOCKEDEXCHANGE)
#	include <windows.h>
	MAINLINE
	{ LONG l=0;
	return InterlockedExchange(&l,1); }
#elif defined(TRY_HAVE_WAITFORSINGLEOBJECTEX)
#	include <windows.h>
	MAINLINE
	{ return WaitForSingleObjectEx((HANDLE)0,INFINITE,TRUE); }
#elif defined(TRY_HAVE_TMPFILE)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdio.h>
	MAINLINE
	{ FILE *fp=tmpfile(); return (argc && argv && fp) ? 0 : 1; }
#elif defined(TRY_HAVE_RAISE)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	MAINLINE
	{ return (argc && argv) ? raise(SIGINT) : 1; }
#elif defined(TRY_HAVE_PSIGNAL)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SIGNAL_H
#		include <signal.h>
#	endif
#	ifdef HAVE_SIGINFO_H
#		include <siginfo.h>
#	endif
	MAINLINE
	{ 	psignal(argc,argv[0]);
		return (argc && argv); }
#elif defined(TRY_HAVE_SIGINTERRUPT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_SIGNAL_H
#		include <signal.h>
#	endif
	MAINLINE
	{ 	siginterrupt(argc,SIGINT);
		return (argc && argv); }
#elif defined(TRY_HAVE_SA_HANDLER_DEFAULT_EMPTY)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static handler() {}
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (sa.sa_handler && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_DEFAULT_INT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static handler(int num) { if (num) { } }
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (sa.sa_handler && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_DEFAULT_KANDR)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static handler(num) int num; { if (num) { } }
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (sa.sa_handler && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_VOID_KANDR)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static void handler(num) int num; { if (num) { } }
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (sa.sa_handler && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_INT_INT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static int handler(int num) { return num; }
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (sa.sa_handler && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_VOID_EMPTY)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static void handler() {}
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (sa.sa_handler && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_VOID_VOID)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static void handler(void) {}
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (argc && argv && sa.sa_handler) ? 0 : 1; }
#elif defined(TRY_HAVE_SA_HANDLER_VOID_INT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	static void handler(int num) { if (num) {} }
	MAINLINE
	{ struct sigaction sa; sa.sa_handler=handler;
	return (argc && argv && sa.sa_handler) ? 0 : 1; }
#elif defined(TRY_HAVE_MEMMOVE)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdlib.h>
#	include <string.h>
	MAINLINE
	{ static const char src[]={'A'}; char dst[sizeof(src)]; 
	memmove(dst,src,sizeof(dst)); return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_STRCASECMP)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdlib.h>
#	ifdef HAVE_STRING_H
#		include <string.h>
#	endif
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif
	MAINLINE
	{ return (argc && argv) ? strcasecmp("A","a") : 1; }
#elif defined(TRY_HAVE_STRICMP)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdlib.h>
#	ifdef HAVE_STRING_H
#		include <string.h>
#	endif
	MAINLINE
	{ return (argc && argv) ? stricmp("A","a") : 1; }
#elif defined(TRY_HAVE__STRICMP)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdlib.h>
#	ifdef HAVE_STRING_H
#		include <string.h>
#	endif
	MAINLINE
	{ return (argc && argv) ? _stricmp("A","a") : 1; }
#elif defined(TRY_HAVE_INT_SPRINTF)
#	include <stdio.h>
	MAINLINE
	{ char buf[256]; int i=sprintf(buf,"%d,%p",argc,argv); return !i; }
#elif defined(TRY_HAVE_INT_VSPRINTF)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
#	include <stdio.h>
	static int func(char *buf,char *f,...)
	{ int rc; va_list ap; va_start(ap,f); 
	  rc=vsprintf(buf,f,ap); va_end(ap); return rc; }
	MAINLINE
	{ char buf[256]; int i=func(buf,"%d,%p",argc,argv); return !i; }
#elif defined(TRY_HAVE_PTR_SPRINTF)
#	include <stdio.h>
	MAINLINE
	{ char buf[256]; char *p=sprintf(buf,"%d,%p",argc,argv); return p ? 0 : 1; }
#elif defined(TRY_HAVE_PTR_VSPRINTF)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
#	include <stdio.h>
	static char * func(char *buf,char *f,...)
	{ char *rc; va_list ap; va_start(ap,f); 
	  rc=vsprintf(buf,f,ap); va_end(ap); return rc; }
	MAINLINE
	{ char buf[256]; char *p=func(buf,"%d,%p",argc,argv); return p ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_IN6)
#	if defined(HAVE_WINSOCK2_H) || defined(HAVE_WINSOCK_H)
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			include <winsock.h>
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		include <netinet/in.h>
#	endif
	MAINLINE
	{ static struct sockaddr_in6 addr; 
	  addr.sin6_family=AF_INET6;
	  return (sizeof(addr) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_IN6_ADDR)
#	if defined(HAVE_WINSOCK2_H) || defined(HAVE_WINSOCK_H)
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			include <winsock.h>
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		include <netinet/in.h>
#	endif
	MAINLINE
	{ static struct in6_addr addr; 
	  return (sizeof(addr) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_IN_ADDR6)
#	if defined(HAVE_WINSOCK2_H) || defined(HAVE_WINSOCK_H)
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			include <winsock.h>
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		include <netinet/in.h>
#	endif
	MAINLINE
	{ static struct in_addr6 addr; char *p=(void *)&addr;
	  return (sizeof(addr) && argc && argv && !p[0]) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_IPX)
#	ifdef HAVE_WSIPX_H
#		include <winsock.h>
#		include <wsipx.h>
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		include <netipx/ipx.h>
#	endif
	MAINLINE
	{ static struct sockaddr_ipx addr;
	  return (sizeof(addr) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_AT)
#	ifdef HAVE_ATALKWSH_H
#		include <winsock.h>
#		include <atalkwsh.h>
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		ifdef HAVE_NETATALK_AT_H
#			include <netatalk/at.h>
#		endif
#		ifdef HAVE_NETAT_APPLETALK_H
#			include <netat/appletalk.h>
#		endif
#		ifdef HAVE_AT_APPLETALK_H
#			include <at/appletalk.h>
#		endif
#	endif
	MAINLINE
	{ static struct sockaddr_at addr;
	  return (sizeof(addr) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SAT_TYPE)
#	ifdef HAVE_ATALKWSH_H
#		include <winsock.h>
#		include <atalkwsh.h>
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		ifdef HAVE_NETATALK_AT_H
#			include <netatalk/at.h>
#		endif
#		ifdef HAVE_NETAT_APPLETALK_H
#			include <netat/appletalk.h>
#		endif
#		ifdef HAVE_AT_APPLETALK_H
#			include <at/appletalk.h>
#		endif
#	endif
	MAINLINE
	{ static struct sockaddr_at addr;
	  return (sizeof(addr.sat_type) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SIPX_TYPE)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	MAINLINE
	{ struct sockaddr_ipx addr;
	  return (sizeof(addr.sipx_type) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SIPX_PORT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	MAINLINE
	{ struct sockaddr_ipx addr;
	  return (sizeof(addr.sipx_port) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SIPX_NODE)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	MAINLINE
	{ struct sockaddr_ipx addr;
	  return (sizeof(addr.sipx_node) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SIPX_NETWORK)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	MAINLINE
	{ struct sockaddr_ipx addr;
	  return (sizeof(addr.sipx_network) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SIPX_ADDR_X_NET_C_NET)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	MAINLINE
	{ struct sockaddr_ipx addr;
	  return (sizeof(addr.sipx_addr.x_net.c_net) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_SOCKADDR_SIPX_ADDR_X_HOST_C_HOST)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netipx/ipx.h>
	MAINLINE
	{ struct sockaddr_ipx addr;
	  return (sizeof(addr.sipx_addr.x_host.c_host) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_HOSTENT_DATA)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
	MAINLINE
	{ static struct hostent_data data;
	  return (sizeof(data) && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_DLOPEN)
#	ifdef BUILD_STATIC
		#error BUILD_STATIC
#	else
#		ifdef HAVE_SYS_TYPES_H
#			include <sys/types.h>
#		endif
#		include <dlfcn.h>
		MAINLINE
		{ return dlopen(argv[0],argc) ? 0 : 1; }
#	endif
#elif defined(TRY_HAVE_WINSOCK_H)
#	include <winsock.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WINSOCK2_H)
#	include <winsock2.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WININET_H)
#	include <windows.h>
#	include <wininet.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NB30_H)
#	include <windows.h>
#	include <nb30.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WS2IP6_H)
#	include <winsock2.h>
#	include <ws2ip6.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WS2TCPIP_H)
#	include <winsock2.h>
#	ifdef HAVE_WS2IP6_H
#	include <ws2ip6.h>
#	endif
#	include <ws2tcpip.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WSPIAPI_H)
#	include <winsock2.h>
#	ifdef HAVE_WS2IP6_H
#		include <ws2ip6.h>
#	endif
#	ifdef HAVE_WS2TCPIP_H
#		include <ws2tcpip.h>
#	endif
#	include <wspiapi.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_WSIPX_H)
#	include <winsock.h>
#	include <wsipx.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_ATALKWSH_H)
#	include <winsock.h>
#	include <atalkwsh.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_OS2_H)
#	include <os2.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETAT_APPLETALK_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netat/appletalk.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_NETAT_AT_VAR_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <net/if.h>
#	include <netat/appletalk.h>
#	include <netat/at_var.h>
	at_entity_t x;
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_APPLETALK_AT_PROTO_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netat/appletalk.h>
#	include <AppleTalk/at_proto.h>
	at_entity_t x;
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_AT_APPLETALK_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <at/appletalk.h>
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_ATALK_NBP_H)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netatalk/at.h>
#	include <atalk/nbp.h>
	int x=NBPOP_LKUP;
	COMPLETE_PROGRAM
#elif defined(TRY_HAVE_ETHER_HOSTTON)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/time.h>
#	include <sys/socket.h>
#	ifdef HAVE_NET_IF_H
#	include <net/if.h>
#	endif
#	ifdef HAVE_NETINET_IN_H
#	include <netinet/in.h>
#	endif
#	ifdef HAVE_NETINET_IF_ETHER_H
#	include <netinet/if_ether.h>
#	endif
	MAINLINE
	{ struct ether_addr addr;
	int i=ether_hostton(argv[0],&addr);
	return i; }
#elif defined(TRY_HAVE_MSGHDR_MSG_ACCRIGHTS)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	ifdef HAVE_SYS_UIO_H
#		include <sys/uio.h>
#	endif
	MAINLINE
	{ static struct msghdr a; 
		return a.msg_accrights ? 1 : 0; }
#elif defined(TRY_HAVE_MSGHDR_MSG_CONTROL)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	ifdef HAVE_SYS_UIO_H
#		include <sys/uio.h>
#	endif
	MAINLINE
	{ static struct msghdr a; 
		return a.msg_control ? 1 : 0; }
#elif defined(TRY_HAVE_RECVMSG)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	ifdef HAVE_SYS_UIO_H
#		include <sys/uio.h>
#	endif
	MAINLINE
	{ static struct msghdr a; 
		return recvmsg(0,&a,0) ? 0 : 1; }
#elif defined(TRY_HAVE_SENDMSG)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	ifdef HAVE_SYS_UIO_H
#		include <sys/uio.h>
#	endif
	MAINLINE
	{ static struct msghdr a; 
		return sendmsg(0,&a,0) ? 0 : 1; }
#elif defined(TRY_HAVE_GETIPNODEBYNAME)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netdb.h>
	MAINLINE
	{ struct hostent *hp=getipnodebyname(
			argv[1],AF_INET6,AI_DEFAULT,&argc); 
	return hp ? 0 : 1; }
#elif defined(TRY_HAVE_GETIPNODEBYADDR)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netdb.h>
	MAINLINE
	{ struct hostent *hp=getipnodebyaddr(
		argv[1],16,AF_INET6,&argc); 
	return hp ? 0 : 1; }
#elif defined(TRY_HAVE_FREEHOSTENT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	include <netdb.h>
	MAINLINE
	{  struct hostent *hp=0; freehostent(hp); return (argc && argv) ? 0 : 1;  }
#elif defined(TRY_HAVE_GETHOSTBYNAME_R_LINUX)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
#	include <netdb.h>
	MAINLINE
	{ struct hostent he,*hp; char buf[1];
	int er; int i=gethostbyname_r(argv[0],&he,buf,1,&hp,&er);
	return i; }
#elif defined(TRY_HAVE_GETHOSTBYNAME_R_SOLARIS)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
#	include <netdb.h>
	MAINLINE
	{ struct hostent he,*hp; char buf[1];
	int er; hp=gethostbyname_r(argv[0],&he,buf,1,&er);
	return er; }
#elif defined(TRY_HAVE_GETHOSTBYADDR_R_LINUX)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
#	include <netdb.h>
	MAINLINE
	{ struct hostent he,*hp; char buf[1];
	int er; int i=gethostbyaddr_r(argv[0],1,2,&he,buf,1,&hp,&er);
	return i; }
#elif defined(TRY_HAVE_GETHOSTBYADDR_R_SOLARIS)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
#	include <netdb.h>
	MAINLINE
	{ struct hostent he,*hp; char buf[1];
	int er; hp=gethostbyaddr_r(argv[0],1,2,&he,buf,1,&er);
	return er; }
#elif defined(TRY_HAVE_GETNAMEINFO)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef _WIN32
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			include <winsock.h>
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#		ifdef HAVE_WSPIAPI_H
#			include <wspiapi.h>
#		endif
#	else
#		include <sys/socket.h>
#		include <netdb.h>
#	endif
	MAINLINE
	{ char b[1]; struct sockaddr addr; getnameinfo(&addr,1,b,1,b,1,1); 
	  return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_GETADDRINFO)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef _WIN32
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			include <winsock.h>
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#		ifdef HAVE_WSPIAPI_H
#			include <wspiapi.h>
#		endif
#	else
#		include <sys/socket.h>
#		include <netdb.h>
#	endif
	MAINLINE
	{ char b[1]; static struct addrinfo *res; getaddrinfo(b,b,res,&res); 
	  return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_FREEADDRINFO)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef _WIN32
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#		else
#			include <winsock.h>
#		endif
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#		ifdef HAVE_WSPIAPI_H
#			include <wspiapi.h>
#		endif
#	else
#		include <sys/socket.h>
#		include <netdb.h>
#	endif
	MAINLINE
	{ static struct addrinfo *ai; freeaddrinfo(ai); 
	  return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_NBP_LOOKUP)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/socket.h>
#	ifdef HAVE_NETAT_APPLETALK_H
#		include <netat/appletalk.h>
#	endif
#	ifdef HAVE_AT_APPLETALK_H
#		include <at/appletalk.h>
#		include <at/ddp.h>
#		include <at/nbp.h>
#		include <at/zip.h>
#	endif
#	ifdef HAVE_APPLETALK_AT_PROTO_H
#		include <AppleTalk/at_proto.h>
#	endif
	MAINLINE
	{ static at_entity_t e; 
      static at_nbptuple_t b;
	  static at_retry_t r;
	  int i=nbp_lookup(&e,&b,1,&r); 
	  return (argc && argv && i) ? 0 : 1; }
#elif defined(TRY_HAVE_FPOS_T_SCALAR)
#	include <stdio.h>
	MAINLINE
	{ static fpos_t fpt; long val=(long)fpt; return val; }
#elif defined(TRY_HAVE_SNPRINTF)
#	include <stdio.h>
	MAINLINE
	{ char buf[256];
	  int i=snprintf(buf,sizeof(buf),"%d,%p",argc,argv);
	  return i ? 0 : 1;	}
#elif defined(TRY_HAVE_SWPRINTF_ISO)
#	include <stdio.h>
#	ifdef HAVE_WCHAR_H
#		include <wchar.h>
#	endif
	MAINLINE
	{ wchar_t buf[256];
	  static const wchar_t fmt[]={0};
	  int i=swprintf(buf,(sizeof(buf)/sizeof(buf[0])),fmt,argc,argv);
	  return i ? 0 : 1;	}
#elif defined(TRY_HAVE_SWPRINTF)
#	include <stdio.h>
#	ifdef HAVE_WCHAR_H
#		include <wchar.h>
#	endif
	MAINLINE
	{ wchar_t buf[256];
	  static const wchar_t fmt[]={0};
#ifdef HAVE_SWPRINTF_ISO
	  int i=swprintf(buf,(sizeof(buf)/sizeof(buf[0])),fmt,argc,argv);
#else
	  int i=swprintf(buf,fmt,argc,argv);
#endif
	  return i ? 0 : 1;	}
#elif defined(TRY_HAVE__SNPRINTF)
#	include <stdio.h>
	MAINLINE
	{ char buf[256];
	  int i=_snprintf(buf,sizeof(buf),"%d,%p",argc,argv);
	  return i ? 0 : 1;	}
#elif defined(TRY_HAVE_VSNPRINTF)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
#	include <stdio.h>
	static int func(va_list ap)
	{
		char buf[256];
		int i=vsnprintf(buf,sizeof(buf),"%d",ap);
		return i;
	}
	static int func2(int argc,...)
	{
	  int i;
	  va_list ap;
	  va_start(ap,argc);
	  i=func(ap);
	  va_end(ap);
	  return i;
	}
	MAINLINE
	{ func2(argc,argv);
	  return 0; }
#elif defined(TRY_HAVE__VSNPRINTF)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
#	include <stdio.h>
	static int func(va_list ap)
	{
		char buf[256];
		int i=_vsnprintf(buf,sizeof(buf),"%d",ap);
		return i;
	}
	static int func2(int argc,...)
	{
	  int i;
	  va_list ap;
	  va_start(ap,argc);
	  i=func(ap);
	  va_end(ap);
	  return i;
	}
	MAINLINE
	{ func2(argc,argv);
	  return 0; }
#elif defined(TRY_HAVE_VA_END_EMPTY)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ va_end(ignored-argument); return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_VA_COPY)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	static int func1(int f,va_list ap1)
	{ va_list ap2; int i;
	va_copy(ap2,ap1);
	i=va_arg(ap2,int);
	va_end(ap2);
	return i+f; }
	static int func2(int f,...)
	{ va_list ap1;int i;
	va_start(ap1,f);
	i=func1(f,ap1);
	va_end(ap1);
	return i; }
	MAINLINE
	{ return func2(argc,argv); }
#elif defined(TRY_HAVE_VA_COPY_VOLATILE)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	static int func1(int f,va_list ap1)
	{ va_list volatile ap2; int i;
	va_copy(ap2,ap1);
	i=va_arg(ap2,int);
	va_end(ap2);
	return i+f; }
	static int func2(int f,...)
	{ va_list ap1;int i;
	va_start(ap1,f);
	i=func1(f,ap1);
	va_end(ap1);
	return i; }
	MAINLINE
	{ return func2(argc,argv); }
#elif defined(TRY_HAVE_VA_LIST_ASSIGN)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	static int func1(int f,va_list ap)
	{ va_list ap1=ap; return (f+va_arg(ap1,int)) ? 0: 1; }
	static int func2(int f,...)
	{ va_list ap1;int i;
	va_start(ap1,f);
	i=func1(f,ap1);
	va_end(ap1);
	return i; }
	MAINLINE
	{ return func2(argc,argv); }
#elif defined(TRY_HAVE_VA_ARG_ASSIGN)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	static int func2(int f,...)
	{ 	va_list ap;
		int i=0;
		char *p=(char *)0;
		double d=0.0;
		va_start(ap,f);
		va_arg(ap,int)=i;
		va_arg(ap,char *)=p;
		va_arg(ap,double)=d;
		va_end(ap);
	return 0; }
	MAINLINE
	{ return func2(argc,argv); }
#elif defined(TRY_HAVE_VA_ARG_VOLATILE)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	static int func2(va_list ap)
	{
#ifdef HAVE_VA_COPY_VOLATILE
		va_list volatile ap2;
		va_copy(ap2,ap);
#else
		va_list volatile ap2=ap;
#endif
		return va_arg(ap2,int);
	}
	static int func(int x,...)
	{
		va_list ap;
		va_start(ap,x);
		x=func2(ap);
		va_end(ap);
		return x;
	}

	MAINLINE
	{ return func(argc,argv); }
#elif defined(TRY_HAVE_VA_LIST_SCALAR)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ va_list ap=(va_list)0; return ap ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_VA_LIST_OVERFLOW_ARG_AREA)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].overflow_arg_area) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST_REG_SAVE_AREA)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].reg_save_area) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST_GPR)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].gpr) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST_FPR)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].fpr) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST_GP_OFFSET)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].gp_offset) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST_FP_OFFSET)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].fp_offset) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST__OFFSET)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap._offset) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST__A0)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap._a0) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST___AP)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap.__ap) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST___VA_GP_OFFSET)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].__va_gp_offset) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST___VA_FP_OFFSET)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].__va_fp_offset) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].__va_overflow_arg_area) ? 1 : 0;}
#elif defined(TRY_HAVE_VA_LIST___VA_REG_SVE_AREA)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static va_list ap; return (argc && argv && ap[0].__va_reg_sve_area) ? 1 : 0;}
#elif defined(TRY_HAVE___VA_REGSAVE_T)
#	ifdef HAVE_STDARG_H
#		include <stdarg.h>
#	else
#		include <varargs.h>
#	endif
	MAINLINE
	{ static __va_regsave_t s; return (argc && argv && sizeof(s)) ? 1 : 0;}
#elif defined(TRY_HAVE_PTHREAD_ATFORK)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
	static void wibble(void) {  }
	MAINLINE
	{ pthread_atfork(wibble,wibble,wibble); return 0; }
#elif defined(TRY_HAVE_PTHREAD_CANCEL)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
	MAINLINE
	{ pthread_cancel(pthread_self());
	pthread_testcancel(); return 0; }
#elif defined(TRY_HAVE_PTHREAD_KILL)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <signal.h>
	MAINLINE
	{ pthread_kill(pthread_self(),SIGINT); return 0; }
#elif defined(TRY_HAVE_PTHREAD_T_SCALAR)
#	include <pthread.h>
	MAINLINE
	{ pthread_t tid=(pthread_t)0; return tid ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_ADDR_T)
#	include <pthread.h>
	static struct wibble { int x; } value;
	MAINLINE
	{ pthread_addr_t tid=(pthread_addr_t)(void *)&value; return tid ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_KEY_CREATE)
#	include <pthread.h>
	static void destr(void *pv) { if (pv) {} }
	static pthread_key_t key;
	MAINLINE
	{ return pthread_key_create(&key,destr) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_KEY_DELETE)
#	include <pthread.h>
	static pthread_key_t key;
	MAINLINE
	{ return pthread_key_delete(key) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_KEYCREATE)
#	include <pthread.h>
	static void destr(void *pv) { if (pv) {} }
	static pthread_key_t key;
	MAINLINE
	{ return pthread_keycreate(&key,destr) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_GETSPECIFIC_STD)
#	include <pthread.h>
	MAINLINE
	{	static pthread_key_t key;
		return pthread_getspecific(key) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_GETSPECIFIC_D4)
#	include <pthread.h>
	MAINLINE 
	{   static pthread_addr_t v;
		static pthread_key_t key;
		return pthread_getspecific(key,&v) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_ATTR_DEFAULT)
#	include <pthread.h>
	static void * mythread(void *pv) { return pv; }
	static pthread_t tid;
	MAINLINE
	{ return pthread_create(&tid,pthread_attr_default,mythread,argv) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_MUTEXATTR_DEFAULT)
#	include <pthread.h>
	static pthread_mutex_t mutex;
	MAINLINE
	{ return pthread_mutex_init(&mutex,pthread_mutexattr_default) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_PTHREAD_CONDATTR_DEFAULT)
#	include <pthread.h>
	static pthread_cond_t cond;
	MAINLINE
	{ return pthread_cond_init(&cond,pthread_condattr_default) ? 1 : ((argc&&argv)?0:1);}
#elif defined(TRY_HAVE_D2I_X509_CONST)
#	include <sys/types.h>
#	include <stdio.h>
#	include <openssl/x509.h>
#	ifdef _WIN32
#		pragma comment(lib,"libeay32")
#	endif
	MAINLINE
	{ const unsigned char *p=NULL;
	  X509 *cert=d2i_X509(NULL,&p,0);
		return (cert&&argc&&argv)?0:1;}
#elif defined(TRY_HAVE_XTDEFAULTAPPCONTEXT)
#	include <X11/Intrinsic.h>
#ifdef __cplusplus
	extern "C"
#else
	extern
#endif
	XtAppContext _XtDefaultAppContext(void);
	MAINLINE
	{ XtAppContext app=_XtDefaultAppContext();
	return (app && argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_XTSETLANGUAGEPROC)
#	include <X11/Intrinsic.h>
	MAINLINE
	{ XtSetLanguageProc(NULL,(XtLanguageProc)NULL,NULL);
	return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_XTAPPSETEXITFLAG)
#	include <X11/Intrinsic.h>
	MAINLINE
	{ static XtAppContext app; XtAppSetExitFlag(app);
	return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_XTDISPATCHEVENTTOWIDGET)
#	include <X11/Intrinsic.h>
	MAINLINE
	{ static Widget w; static XEvent e;
	  int x=XtDispatchEventToWidget(w,&e);
	return (argc && argv && x) ? 0 : 1; }
#elif defined(TRY_HAVE_XPCREATECONTEXT)
#	include <X11/Xlib.h>
#	include <X11/extensions/Print.h>
	MAINLINE
	{   Display *disp=XOpenDisplay(argv[1]);
	    XPContext context=XpCreateContext(disp,argv[2]);
		return (argc && argv && context) ? 0 : 1; }
#elif defined(TRY_HAVE_XEDITRESCHECKMESSAGES)
#	include <X11/Intrinsic.h>
#ifdef __cplusplus
	extern "C"
#else
	extern
#endif
	void _XEditResCheckMessages(Widget,void *,XEvent *,Boolean *);
	MAINLINE
	{ Widget parent=NULL;
	XtAddEventHandler(parent,(EventMask)0,True,_XEditResCheckMessages,NULL);
	return (parent && argc && argv) ? 0 : 1; }
#elif defined(TRY_USE_PTHREAD_ONCE_INIT_BRACES0)
#	include <pthread.h>
	pthread_once_t x=PTHREAD_ONCE_INIT;
	COMPLETE_PROGRAM
#elif defined(TRY_USE_PTHREAD_ONCE_INIT_BRACES1)
#	ifdef USE_PTHREAD_ONCE_INIT_BRACES0
		#error USE_PTHREAD_ONCE_INIT_BRACES0 already defined
#	else
#		include <pthread.h>
		pthread_once_t x={PTHREAD_ONCE_INIT};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_PTHREAD_ONCE_INIT_BRACES2)
#	ifdef USE_PTHREAD_ONCE_INIT_BRACES0
		#error USE_PTHREAD_ONCE_INIT_BRACES0 already defined
#	elif defined(USE_PTHREAD_ONCE_INIT_BRACES1)
		#error USE_PTHREAD_ONCE_INIT_BRACES1 already defined
#	else
#		include <pthread.h>
		pthread_once_t x={{PTHREAD_ONCE_INIT}};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_PTHREAD_MUTEX_INITIALIZER_BRACES0)
#	include <pthread.h>
	pthread_mutex_t x=PTHREAD_MUTEX_INITIALIZER;
	COMPLETE_PROGRAM
#elif defined(TRY_USE_PTHREAD_MUTEX_INITIALIZER_BRACES1)
#	ifdef USE_PTHREAD_MUTEX_INITIALIZER_BRACES0
		#error USE_PTHREAD_MUTEX_INITIALIZER_BRACES0 already defined
#	else
#		include <pthread.h>
		pthread_mutex_t x={PTHREAD_MUTEX_INITIALIZER};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_PTHREAD_MUTEX_INITIALIZER_BRACES2)
#	ifdef USE_PTHREAD_MUTEX_INITIALIZER_BRACES0
		#error USE_PTHREAD_MUTEX_INITIALIZER_BRACES0 already defined
#	elif defined(USE_PTHREAD_MUTEX_INITIALIZER_BRACES1)
		#error USE_PTHREAD_MUTEX_INITIALIZER_BRACES1 already defined
#	else
#		include <pthread.h>
		pthread_mutex_t x={{PTHREAD_MUTEX_INITIALIZER}};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_PTHREAD_COND_INITIALIZER_BRACES0)
#	include <pthread.h>
	pthread_cond_t x=PTHREAD_COND_INITIALIZER;
	COMPLETE_PROGRAM
#elif defined(TRY_USE_PTHREAD_COND_INITIALIZER_BRACES1)
#	ifdef USE_PTHREAD_COND_INITIALIZER_BRACES0
		#error USE_PTHREAD_COND_INITIALIZER_BRACES0 already defined
#	else
#		include <pthread.h>
		pthread_cond_t x={PTHREAD_COND_INITIALIZER};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_PTHREAD_COND_INITIALIZER_BRACES2)
#	ifdef USE_PTHREAD_COND_INITIALIZER_BRACES0
		#error USE_PTHREAD_COND_INITIALIZER_BRACES0 already defined
#	elif defined(USE_PTHREAD_COND_INITIALIZER_BRACES1)
		#error USE_PTHREAD_COND_INITIALIZER_BRACES1 already defined
#	else
#		include <pthread.h>
		pthread_cond_t x={{PTHREAD_COND_INITIALIZER}};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_IN6ADDR_ANY_INIT_BRACES0)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	ifdef HAVE_WINSOCK2_H
#		include <winsock2.h>
#		ifdef HAVE_WS2IP6_H
#			include <ws2ip6.h>
#		endif
#		ifdef HAVE_WS2TCPIP_H
#			include <ws2tcpip.h>
#		endif
#	else
#		include <sys/socket.h>
#		include <netinet/in.h>
#	endif
struct in6_addr addrInit=IN6ADDR_ANY_INIT;
	COMPLETE_PROGRAM
#elif defined(TRY_USE_IN6ADDR_ANY_INIT_BRACES1)
#	ifdef USE_IN6ADDR_ANY_INIT_BRACES0
		#error USE_IN6ADDR_ANY_INIT_BRACES0
#	else
#		ifdef HAVE_SYS_TYPES_H
#			include <sys/types.h>
#		endif
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#			ifdef HAVE_WS2IP6_H
#				include <ws2ip6.h>
#			endif
#			ifdef HAVE_WS2TCPIP_H
#				include <ws2tcpip.h>
#			endif
#		else
#			include <sys/socket.h>
#			include <netinet/in.h>
#		endif
	struct in6_addr addrInit={IN6ADDR_ANY_INIT};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_USE_IN6ADDR_ANY_INIT_BRACES2)
#	if defined(USE_IN6ADDR_ANY_INIT_BRACES0) || defined(USE_IN6ADDR_ANY_INIT_BRACES1)
		#error USE_IN6ADDR_ANY_INIT_BRACES0 or USE_IN6ADDR_ANY_INIT_BRACES1
#	else
#		ifdef HAVE_SYS_TYPES_H
#			include <sys/types.h>
#		endif
#		ifdef HAVE_WINSOCK2_H
#			include <winsock2.h>
#			ifdef HAVE_WS2IP6_H
#				include <ws2ip6.h>
#			endif
#			ifdef HAVE_WS2TCPIP_H
#				include <ws2tcpip.h>
#			endif
#		else
#			include <sys/socket.h>
#			include <netinet/in.h>
#		endif
	struct in6_addr addrInit={{IN6ADDR_ANY_INIT}};
		COMPLETE_PROGRAM
#	endif
#elif defined(TRY_HAVE_POSIX_SIGWAIT)
#	include <signal.h>
	MAINLINE
	{ sigset_t s; int i; sigemptyset(&s); argc=sigwait(&s,&i);
	return (argc && argv) ? 0 : 1;}
#elif defined(TRY_HAVE_SYSINFO_SVR4)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <sys/systeminfo.h>
	MAINLINE
	{ char buf[256]; long i=sysinfo(SI_HW_SERIAL,buf,sizeof(buf));
	return (argc && argv && (i > 0)) ? 0 : 1;}
#elif defined(TRY_HAVE_SETPGRP_SYSV)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
	MAINLINE
	{ return (argc && argv && !setpgrp()) ? 0 : 1; }
#elif defined(TRY_HAVE_SETPGRP_BSD)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
	MAINLINE
	{ return (argc && argv && !setpgrp(getpid(),getpid())) ? 0 : 1; }
#elif defined(TRY_HAVE__GETPTY)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <unistd.h>
#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif
	MAINLINE
	{ int fd=-1; char *pty=_getpty(&fd,O_RDWR,0600,0);
	 return (argc && argv && pty && (fd!=-1)) ? 0 : 1; }
#elif defined(TRY_HAVE_PTSNAME)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdlib.h>
	MAINLINE
	{ int fd=-1; char *pty=ptsname(fd);
	 return (argc && argv && pty) ? 0 : 1; }
#elif defined(TRY_HAVE_POSIX_OPENPT)
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <stdlib.h>
#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif
	MAINLINE
	{ int fd=posix_openpt(O_RDWR);
	 return (argc && argv && (fd!=-1)) ? 0 : 1; }
#elif defined(TRY__POSIX_PTHREAD_SEMANTICS)
#	ifdef HAVE_POSIX_SIGWAIT
		#error HAVE_POSIX_SIGWAIT already defined
#	else
#		define _POSIX_PTHREAD_SEMANTICS
#		include <signal.h>
		MAINLINE
		{ sigset_t s; int i; sigemptyset(&s); argc=sigwait(&s,&i);
		return (argc && argv) ? 0 : 1;}
#	endif
#elif defined(TRY_HAVE_GETPWUID_R)
#	include <pthread.h>
#	include <pwd.h>
	MAINLINE
	{ struct passwd pwd; char buf[8192];
	  struct passwd *result=&pwd;
	  int i=getpwuid_r(0,&pwd,buf,sizeof(buf),&result);
	  return (argc && argv && result && !i) ? 0 : 1; }
#elif defined(TRY_HAVE_UUID_CREATE)
#ifdef HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif
#ifdef HAVE_SYS_UUID_H
#	include <sys/uuid.h>
#endif
#ifdef HAVE_UUID_H
#	include <uuid.h>
#endif
#ifdef HAVE_UUID_UUID_H
#	include <uuid/uuid.h>
#endif
	MAINLINE
	{ uuid_t u; unsigned int i; uuid_create(&u,&i); 
      return (argc && argv && !i) ? 0 : 1; }
#elif defined(TRY_HAVE_UUID_GENERATE)
#ifdef HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif
#ifdef HAVE_SYS_UUID_H
#	include <sys/uuid.h>
#endif
#ifdef HAVE_UUID_UUID_H
#	include <uuid/uuid.h>
#endif
	MAINLINE
	{ uuid_t u; uuid_generate(u); 
      return (argc && argv) ? 0 : 1; }
#elif defined(TRY_HAVE_READDIR_R)
#	include <pthread.h>
#	ifdef HAVE_SYS_TYPES_H
#		include <sys/types.h>
#	endif
#	include <dirent.h>
	MAINLINE
	{
		/* note some defs of dirent have a 
			d_name as 1 character, including Solaris... */
		DIR *dirp=opendir("/");
		struct {
			struct dirent entry;
			char buf[8192];
		} s;
		struct dirent *result=&s.entry;
		int i=readdir_r(dirp,&s.entry,&result);
		closedir(dirp);
		return (argc && argv && !i) ? 0 : 1; }
#elif defined(TRY_HAVE_CFMAKERAW)
#	include <termios.h>
	MAINLINE
	{ static struct termios t; cfmakeraw(&t); return argc && argv; }
#elif defined(TRY_HAVE_IIDFROMSTRING_LPCOLESTR)
#	include <ole2.h>
#	ifdef _WIN32
#		pragma comment(lib,"ole32")
#	endif
	MAINLINE
	{ HRESULT ( STDAPICALLTYPE * fn)(LPCOLESTR,IID*)=IIDFromString; return argc && argv && fn; }
#elif defined(TRY_HAVE_UNREGISTERTYPELIB)
#	include <ole2.h>
#	ifdef _WIN32
#		pragma comment(lib,"ole32")
#		pragma comment(lib,"oleaut32")
#	endif
	MAINLINE
	{ static TLIBATTR lpAttr;
 	  return UnRegisterTypeLib(&(lpAttr.guid),
							lpAttr.wMajorVerNum,
							lpAttr.wMinorVerNum,
							lpAttr.lcid,
							lpAttr.syskind); }
#elif defined(BUILD_CONFIG)
	static int xyz_config;
#else
	#error INVALID_OPTION
#endif
