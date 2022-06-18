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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#ifdef _WIN32
	#ifdef HAVE_WINSOCK2_H
		#include <winsock2.h>
	#else
		#include <winsock.h>
	#endif
	#ifdef HAVE_WS2IP6_H
		#include <ws2ip6.h>
	#endif
	#ifdef HAVE_WS2TCPIP_H
		#include <ws2tcpip.h>
	#endif
	#include <time.h>
#else
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#ifdef HAVE_ARPA_INET_H
		#include <arpa/inet.h>
	#endif
	#include <unistd.h>
	#include <strings.h>
	#include <time.h>
	#define closesocket(fd)			close(fd)
	typedef int SOCKET;
#	define INVALID_SOCKET		-1
#endif

#include <stdio.h>
#include <string.h>

#ifndef HAVE_SOCKLEN_T
	#define HAVE_SOCKLEN_T
	typedef int socklen_t;
#endif

#ifndef CPU_VENDOR_OS
#	ifdef _WIN32
#		ifdef _WIN64
#			define CPU_VENDOR_OS "x64-unknown-win64"
#		elif defined(_M_IX86)
#			define CPU_VENDOR_OS "i386-unknown-win32"
#		elif defined(_WIN32_WCE) && defined(_ARM_)
#			define CPU_VENDOR_OS "arm-unknown-wince"
#		endif
#	endif
#endif

int main(int argc,char **argv)
{
	FILE *fp=NULL;
	char *filename=NULL;
	char *p=CPU_VENDOR_OS;
#ifdef _WIN32
	WSADATA wsd;
	int k=WSAStartup(0x101,&wsd);

	if (k) return k;
#endif

	if (argc > 1)
	{
		filename=argv[1];
		fp=fopen(filename,"w");

		if (!fp) 
		{
#ifdef _WIN32_WCE
			fprintf(stderr,"fopen error %s\n",filename);
#else
			perror(filename);
#endif
			return 1;
		}
	}
	else
	{
		fp=stdout;
	}

	fprintf(fp,"/* Generated on CPU_VENDOR_OS=%s */\n",p);

#ifndef _WIN32_WCE	
	{
		time_t t;
		int i=0;

		time(&t);
		p=ctime(&t);

		if (p)
		{
			while (p[i]>=' ')
			{
				i++;
			}

			fprintf(fp,"/* ");
			fwrite(p,i,1,fp);
			fprintf(fp," */\n");
		}
	}
#endif

#if defined(HAVE_SOCKADDR_IN6) && defined(AF_INET6)
	do 
	{
#ifdef _WIN32
		SOCKET 
#else
		#define INVALID_SOCKET -1
		int 
#endif
			fd6=socket(AF_INET6,SOCK_STREAM,0),
			fd4=socket(AF_INET,SOCK_STREAM,0);
		struct sockaddr_in6 addr6;
		struct sockaddr_in addr4;
		socklen_t addrlen6=sizeof(addr6);
		socklen_t addrlen4=sizeof(addr4);

		if (fd6==INVALID_SOCKET)
		{
			break;
		}

		memset(&addr6,0,sizeof(addr6));
		memset(&addr4,0,sizeof(addr4));

		addr6.sin6_family=AF_INET6;

		if (bind(fd6,(struct sockaddr *)&addr6,addrlen6) ||
			listen(fd6,5) ||
			getsockname(fd6,(struct sockaddr *)&addr6,&addrlen6))
		{
			closesocket(fd6);

			break;
		}

		fprintf(fp,"#define IPV6_WILL_BIND %d\n",ntohs(addr6.sin6_port));

		addr4.sin_family=AF_INET;
		addr4.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
		addr4.sin_port=addr6.sin6_port;

		if (connect(fd4,(struct sockaddr *)&addr4,addrlen4))
		{
			fprintf(fp,"/* connect(AF_INET4,127.0.0.1:%d) failed */\n",
				ntohs(addr4.sin_port));
		}
		else
		{
			fd_set fds;
			struct timeval tv={0,0};
			FD_ZERO(&fds);
			FD_SET(fd6,&fds);
			tv.tv_sec=5;

			if (select(((int)fd6)+1,&fds,NULL,NULL,&tv)>0)
			{
				if (FD_ISSET(fd6,&fds))
				{
					SOCKET fd=accept(fd6,(struct sockaddr *)&addr6,&addrlen6);
					if (fd!=INVALID_SOCKET)
					{
						char buf[256];
						size_t s=sizeof(addr6.sin6_addr);
						unsigned char *p=(unsigned char *)&addr6.sin6_addr;
						unsigned char *q=(unsigned char *)buf;

						while (s--)
						{
							q+=
#ifdef HAVE_SNPRINTF
							snprintf
#else
							sprintf
#endif
								((char *)q,
#ifdef HAVE_SNPRINTF
								((buf+sizeof(buf))-(char *)q),
#endif
								"%02X",*p++);
						}

						fprintf(fp,"#define IPV6_WILL_ACCEPT_IPV4 \"%s\"\n",buf);

						closesocket(fd);
					}
				}
			}
			else
			{
				fprintf(fp,"/* select failed */\n");
			}
		}

		closesocket(fd4);
		closesocket(fd6);

		{
			int i=64;
			while (i--)
			{
				SOCKET fd6=socket(AF_INET6,SOCK_STREAM,0);
				SOCKET fd4=socket(AF_INET,SOCK_STREAM,0);
				struct sockaddr_in6 addr6;
				struct sockaddr_in addr4;
				socklen_t addrlen6=sizeof(addr6);
				socklen_t addrlen4=sizeof(addr4);

				memset(&addr6,0,sizeof(addr6));
				memset(&addr4,0,sizeof(addr4));

				addr6.sin6_family=AF_INET6;
				addr4.sin_family=AF_INET;

				if (!bind(fd6,(struct sockaddr *)&addr6,addrlen6) &&
					!getsockname(fd6,(struct sockaddr *)&addr6,&addrlen6))
				{
					addr4.sin_port=addr6.sin6_port;

					if (!bind(fd4,(struct sockaddr *)&addr4,addrlen4) &&
						!getsockname(fd4,(struct sockaddr *)&addr4,&addrlen4))
					{
						if (addr4.sin_port==addr6.sin6_port)
						{
							fprintf(fp,"#define IPV6_AND_IPV4_BIND %d\n",ntohs(addr4.sin_port));
							break;
						}
					}
				}

				closesocket(fd6);
				closesocket(fd4);
			}
		}
	} while (0);
#endif

	if (fp && (fp!=stdout))
	{
		fclose(fp);
	}

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
