/*    
	WinSockIF.c	1.21
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "windows.h"
#include "winsock.h"
#include "ddeml.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "dos.h"
#include "WinSockBin.h"

typedef struct tagIT_WSA
{
        LPSTR   lpGetServMemPtr;
	UINT	uGetServByName_Sync;
        LPSTR   lpGetProtoMemPtr;
	UINT	uGetProtoByName_Sync;
        LPSTR   lpGetHostMemPtr;
	UINT	uGetHostByName_Sync;
} IT_WSA;

static IT_WSA wsa;

static void servent_nat_to_bin(struct servent *,UINT *);
static void protoent_nat_to_bin(struct protoent *,UINT *);
static void hostent_nat_to_bin(struct hostent *,UINT *);

int Select ( int , fd_set *, fd_set *, fd_set *, const struct timeval *);

void
IT_GETSERVBYNAME(ENV *envp, LONGPROC f)
{
	struct servent FAR *PASCAL FAR lp;
   	DWORD	retcode;
	
	lp = Getservbyname((LPCSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8)),
                            (LPCSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4)));

	if(lp) {
	    servent_nat_to_bin(lp,&wsa.uGetServByName_Sync);
	    retcode = (DWORD)MAKELP(wsa.uGetServByName_Sync,0);
	}
	else
	    retcode = 0;

	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode); 	/* note: offset by link */
	envp->reg.dx = HIWORD(retcode);		/* the actual selector  */
}


static void servent_nat_to_bin(struct servent *lpse, UINT *lpSel)
{
    int nLen,nMemoryLen,nMemoryLenOld,nAliasCount;
    int i;
    LPSTR lpData;
    UINT sel = *lpSel;
    UINT offset = 0,alias_offset;

    for (i = 0, nLen = 0; lpse->s_aliases[i]; i++)
	nLen += strlen(lpse->s_aliases[i])+1;
    nAliasCount = i+1;
    if (lpse->s_name)
	nLen += strlen(lpse->s_name)+1;
    if (lpse->s_proto)
	nLen += strlen(lpse->s_proto)+1;

    nMemoryLen =  LP_86 + LP_86 + nAliasCount*LP_86 + INT_86 + LP_86 + nLen;
    if (sel == 0) {
	lpData = WinMalloc(nMemoryLen);
	*lpSel = AssignSelector((LPBYTE)lpData,0,TRANSFER_DATA,nMemoryLen);
	sel = *lpSel;
    }
    else {
	nMemoryLenOld = GetSelectorLimit(sel);
	lpData = (LPSTR)GetSelectorBase(sel);
	if (nMemoryLen > nMemoryLenOld) {
	    lpData = WinRealloc(lpData,nMemoryLen);
	    SetSelectorLimit(sel,nMemoryLen);
	    SetPhysicalAddress(sel,lpData);
	    DPMI_Notify(DN_MODIFY,sel);
	}
    }
    wsa.lpGetServMemPtr = lpData; 
    alias_offset = LP_86 + LP_86 + INT_86 + LP_86;
    offset = alias_offset + nAliasCount*LP_86;

    if (lpse->s_name) {
	strcpy(lpData + offset, lpse->s_name);
	PUTWORD(lpData, offset);
	PUTWORD(lpData+2,sel);
	offset += strlen(lpse->s_name)+1;
    }
    else
	PUTDWORD(lpData,0);

    PUTWORD(lpData+8,lpse->s_port);

    if (lpse->s_proto) {
	strcpy(lpData + offset, lpse->s_proto);
	PUTWORD(lpData+10, offset);
	PUTWORD(lpData+12,sel);
	offset += strlen(lpse->s_proto)+1;
    }
    else
	PUTDWORD(lpData+10,0);

    PUTWORD(lpData+4, alias_offset);
    PUTWORD(lpData+6,sel);

    for (i = 0; i < nAliasCount-1; i++) {
	strcpy(lpData + offset, lpse->s_aliases[i]);
	PUTWORD(lpData+alias_offset + i*LP_86, offset);
	PUTWORD(lpData+alias_offset + i*LP_86+2, sel);
	offset += strlen(lpse->s_aliases[i]) + 1;
    }
    PUTDWORD(lpData+alias_offset+i*LP_86,0);
}

/* IT_WSASTARTUP */
void 
IT_WSASTARTUP ( ENV *envp, LONGPROC f)
{

      DWORD	retcode;
      LPBYTE	lpStruct;
      WSADATA   WSDLLData;
      lpStruct = (LPBYTE)GetAddress
                              (GETWORD(SP+6),GETWORD(SP+4)); 
      retcode = WSAStartup(GETWORD(SP+8),&WSDLLData);

      PUTWORD(lpStruct,WSDLLData.wVersion);
      PUTWORD(lpStruct+2,WSDLLData.wHighVersion);
      strcpy((LPSTR)lpStruct+4,(LPSTR)&WSDLLData.szDescription);
      strcpy((LPSTR)lpStruct+261,(LPSTR)&WSDLLData.szSystemStatus);
      PUTWORD(lpStruct+390,(WORD)WSDLLData.iMaxSockets);
      PUTWORD(lpStruct+392,(WORD)WSDLLData.iMaxUdpDg);
      if ( WSDLLData.lpVendorInfo )
      strcpy((LPSTR)lpStruct+394,WSDLLData.lpVendorInfo);
      ResetNtoaSelList();
      envp->reg.sp += WORD_86 + LP_86 + RET_86;
      envp->reg.ax  = LOWORD(retcode);
      envp->reg.dx  = HIWORD(retcode);

}        

void IT_ACCEPT ( ENV *envp, LONGPROC f)
{
      DWORD	retcode;
      LPBYTE	lpStruct1;
      LPBYTE	lpStruct2;
      UINT      Socket;
      int       Sockaddr_len;  
      
      struct sockaddr_in Sockaddr_in;   

      memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));

      lpStruct1 = (LPBYTE)GetAddress
                                (GETWORD(SP+10),GETWORD(SP+8)); 
      Sockaddr_in.sin_family = (short)GETWORD(lpStruct1);
      Sockaddr_in.sin_port   = GETWORD(lpStruct1+2);
      Sockaddr_in.sin_addr.s_addr = GETDWORD(lpStruct1+4);
      lpStruct2 = (LPBYTE)GetAddress
                                (GETWORD(SP+6),GETWORD(SP+4)); 
      Sockaddr_len = (int)(short)GETWORD(lpStruct2); 
      Socket   = (UINT)GETWORD(SP+12);
      retcode = Accept ( Socket,(struct sockaddr FAR *)&Sockaddr_in,(int FAR *)&Sockaddr_len); 
      PUTWORD(lpStruct1,Sockaddr_in.sin_family);
      PUTWORD(lpStruct1+2,Sockaddr_in.sin_port);
      PUTDWORD(lpStruct1+4,Sockaddr_in.sin_addr.s_addr);
      /* fill in the length of the struct */ 
      PUTWORD((UINT)lpStruct2,Sockaddr_len); 
      envp->reg.sp += UINT_86 + LP_86 + LP_86 + RET_86;
      envp->reg.ax =  LOWORD(retcode);
      envp->reg.dx =  HIWORD(retcode);

}

void IT_BIND ( ENV *envp, LONGPROC f)
{

      DWORD	retcode;
      LPBYTE	lpStruct;
      UINT      Socket;
      
      struct sockaddr_in Sockaddr_in;   
      struct in_addr 	 In_addr;

      memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));

      lpStruct = (LPBYTE)GetAddress
                                (GETWORD(SP+8),GETWORD(SP+6)); 
      Sockaddr_in.sin_family = (short)GETWORD(lpStruct);
      Sockaddr_in.sin_port   = (unsigned short)GETWORD(lpStruct+2);
      In_addr.s_addr = GETDWORD(lpStruct+4);
      Socket         = (UINT)GETWORD(SP+10);
      Sockaddr_in.sin_addr.s_addr = In_addr.s_addr;
      retcode = Bind ( (UINT)Socket,(const struct sockaddr FAR *)&Sockaddr_in,GETWORD(SP+4)); 
      envp->reg.sp += UINT_86 + LP_86 + INT_86 + RET_86;
      envp->reg.ax =  LOWORD(retcode);
      envp->reg.dx =  HIWORD(retcode);
             
}


void IT_CONNECT( ENV *envp, LONGPROC f)
{
      DWORD	retcode;
      LPBYTE	lpStruct;
      UINT      Socket;
      int       NameLen;
      
      struct sockaddr_in Sockaddr_in;   
      struct in_addr 	 In_addr;

      memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));

      lpStruct = (LPBYTE)GetAddress
                                (GETWORD(SP+8),GETWORD(SP+6)); 
      Sockaddr_in.sin_family = (short)GETWORD(lpStruct);
      Sockaddr_in.sin_port   = (unsigned short)GETWORD(lpStruct+2);
      In_addr.s_addr = GETDWORD(lpStruct+4);
      Socket         = (UINT)GETWORD(SP+10);
      Sockaddr_in.sin_addr.s_addr = In_addr.s_addr;
      NameLen        = GETWORD(SP+4);
      retcode = Connect ( (UINT)Socket,(const struct sockaddr FAR *)&Sockaddr_in,NameLen); 
      envp->reg.sp += UINT_86 + LP_86 + INT_86 + RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);

}

void IT_CLOSESOCKET ( ENV *envp, LONGPROC f)
{
	DWORD retcode;

	retcode = Closesocket((UINT)(GETWORD(SP+4)));
	envp->reg.sp += UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);

}
void IT_GETPEERNAME ( ENV *envp, LONGPROC f)
{
      DWORD	retcode;
      LPBYTE	lpStruct1;
      LPBYTE	lpStruct2;
      UINT      Socket;
      int       Sockaddr_len;  
      
      struct sockaddr_in Sockaddr_in;   

      memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));

      lpStruct1 = (LPBYTE)GetAddress
                                (GETWORD(SP+10),GETWORD(SP+8)); 
      Sockaddr_in.sin_family = (short)GETWORD(lpStruct1);
      Sockaddr_in.sin_port   = GETWORD(lpStruct1+2);
      Sockaddr_in.sin_addr.s_addr = GETDWORD(lpStruct1+4);
      lpStruct2 = (LPBYTE)GetAddress
                                (GETWORD(SP+6),GETWORD(SP+4)); 
      Sockaddr_len = (int)(short)GETWORD(lpStruct2); 
      Socket   = (UINT)GETWORD(SP+12);
      retcode = Getpeername ( Socket,(struct sockaddr FAR *)&Sockaddr_in,(int FAR *)&Sockaddr_len); 
      PUTWORD(lpStruct1,Sockaddr_in.sin_family);
      PUTWORD(lpStruct1+2,Sockaddr_in.sin_port);
      PUTDWORD(lpStruct1+4,Sockaddr_in.sin_addr.s_addr);
      /* fill in the length of the struct */ 
      PUTWORD((UINT)lpStruct2,Sockaddr_len); 
      envp->reg.sp += UINT_86 + LP_86 + LP_86 + RET_86;
      envp->reg.ax =  LOWORD(retcode);
      envp->reg.dx =  HIWORD(retcode);

}

void IT_GETSOCKNAME ( ENV *envp, LONGPROC f)
{
      DWORD	retcode;
      LPBYTE	lpStruct1;
      LPBYTE	lpStruct2;
      UINT      Socket;
      int       Sockaddr_len;  
      
      struct sockaddr_in Sockaddr_in;   

      memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));
      lpStruct1 = (LPBYTE)GetAddress
                                (GETWORD(SP+10),GETWORD(SP+8)); 
      Sockaddr_in.sin_family = (short)GETWORD(lpStruct1);
      Sockaddr_in.sin_port   = GETWORD(lpStruct1+2);
      Sockaddr_in.sin_addr.s_addr = GETDWORD(lpStruct1+4);
      lpStruct2 = (LPBYTE)GetAddress
                                (GETWORD(SP+6),GETWORD(SP+4)); 
      Sockaddr_len = (int)(short)GETWORD(lpStruct2); 
      Socket   = (UINT)GETWORD(SP+12);
      retcode = Getsockname ( Socket,(struct sockaddr FAR *)&Sockaddr_in,(int FAR *)&Sockaddr_len); 
      PUTWORD(lpStruct1,Sockaddr_in.sin_family);
      PUTWORD(lpStruct1+2,Sockaddr_in.sin_port);
      PUTDWORD(lpStruct1+4,Sockaddr_in.sin_addr.s_addr);
      /* fill in the length of the struct */ 
      PUTWORD((UINT)lpStruct2,Sockaddr_len); 
      envp->reg.sp += UINT_86 + LP_86 + LP_86 + RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);
             
}


void IT_IOCTLSOCKET( ENV *envp, LONGPROC f)
{
        DWORD     retcode;
        DWORD     Cmd;
        LPBYTE    lpStruct;
        DWORD     Argp;
        WORD      Socket;

        lpStruct   = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
        Cmd        = GETDWORD(SP+8);
        Socket     = GETWORD(SP+12);

        Argp       = GETDWORD(lpStruct);
        retcode  = Ioctlsocket(Socket,Cmd,&Argp);

        envp->reg.sp += UINT_86 + DWORD_86 + LP_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode);
}

void IT_SOCKET( ENV *envp, LONGPROC f)
{
	DWORD retcode;
        DWORD dwParam1,dwParam2;

	dwParam1 = 	(int)((short)GETWORD(SP+8));
	dwParam2 = 	(int)((short)GETWORD(SP+6));

	/*******************************************
 	 * the binary interface has binary encoded 
	 * values for SOCK_XXX, which may be different
	 * in a source port. robf
	 ******************************************/
        if (dwParam2==1)
          dwParam2=SOCK_STREAM;
        else if (dwParam2==2)
          dwParam2=SOCK_DGRAM;

	retcode = Socket(dwParam1,
			 dwParam2,
			 (int)((short)GETWORD(SP+4)));

	envp->reg.sp += 3*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void IT_GETSOCKOPT ( ENV *envp, LONGPROC f)
{
        DWORD  retcode;
        UINT   Socket;
        WORD   Level,Optname;
        LPSTR  Optval;
        LPBYTE lpVal;
        LPBYTE lpStr;
        int    Optlen;
         
        Socket    = GETWORD(SP+16);
        Level     = GETWORD(SP+14); 
        Optname   = GETWORD(SP+12);
        lpStr     = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
        lpVal     = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
        Optval    = (LPSTR)GetAddress(GETWORD(lpStr+2),GETWORD(lpStr));
        Optlen    = (int)(short)GETWORD(lpVal);

/*   Here is a patch for now All Windows WinSock Applications
     send 0xffff for the level whereas it should be one of the valid responses ,
     So what we do now is to set it to Level 1 for Sockets which is SOL_SOCKET 
*/
        if ( Level & 0xffff ) Level =SOL_SOCKET;

        retcode = (int)Getsockopt(Socket,Level,Optname,Optval,(int FAR *)&Optlen);

        envp->reg.sp += UINT_86 + 2*LP_86 + 2*INT_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
	envp->reg.dx  = HIWORD(retcode);
         
        

}

void IT_HTONS      ( ENV *envp, LONGPROC f)
{

	DWORD retcode;
	retcode = Htons(GETWORD(SP+4));
        /*  retcode = GETWORD(ui); */
	envp->reg.sp += WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);

}

void IT_HTONL      ( ENV *envp, LONGPROC f)
{
 
	DWORD retcode;
        DWORD HostLong;
        HostLong = GETDWORD(SP+4);

	retcode = Htonl(HostLong);

         /* retcode = GETDWORD(&HostLong); */
	envp->reg.sp += DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void IT_NTOHS      ( ENV *envp, LONGPROC f)
{
	DWORD retcode;

	retcode = Ntohs(GETWORD(SP+4));
	envp->reg.sp += WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);

}

void IT_NTOHL      ( ENV *envp, LONGPROC f)
{
	DWORD retcode;
        DWORD NetLong;
        
        NetLong = GETDWORD(SP+4);
	retcode = Ntohl(NetLong);
	envp->reg.sp += DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

extern LPSTR Inet_ntoa(struct in_addr);

void IT_INET_NTOA  ( ENV *envp, LONGPROC f)
{
	LPSTR retcode;
	WORD wSelIndex;
        struct in_addr in;
        in.s_addr       = GETDWORD(SP+4); 
	retcode  = (LPSTR)Inet_ntoa(in);
	wSelIndex = FindFreeNTOASel();
	NtoaSelList[wSelIndex] = ASSIGNSEL(retcode,32); /* 32 for now */
	envp->reg.sp += DWORD_86 + RET_86;
	envp->reg.ax = 0;
	envp->reg.dx = NtoaSelList[wSelIndex]; 
}

void IT_INET_ADDR ( ENV *envp, LONGPROC f)
{
	DWORD retcode;
        retcode = Inet_addr((LPCSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4)));
        envp->reg.sp += LP_86 + RET_86 ;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);  
}


void IT_LISTEN     ( ENV *envp, LONGPROC f)
{
        DWORD retcode;
        UINT  Socket;
        WORD  BackLog;

        BackLog = (int)(short) GETWORD(SP+4);
        Socket = (UINT)GETWORD(SP+6);
        retcode = (int)Listen((UINT)Socket,(int)BackLog);
        envp->reg.sp += UINT_86 + INT_86 + RET_86;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode);
}

void IT_RECV       ( ENV *envp, LONGPROC f)
{
        DWORD  retcode;
        UINT   Socket;
        LPSTR  lpString;
        WORD   Flags, Len; 
        
        Flags = (int)(short)GETWORD(SP+4);
        Len   = (int)(short)GETWORD(SP+6);
        lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
        Socket   = (UINT)GETWORD(SP+12);
        retcode = Recv (Socket, lpString, Len, Flags);
        envp->reg.sp += UINT_86 + LP_86 + 2*INT_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode); 

}

void IT_RECVFROM   ( ENV *envp, LONGPROC f)
{
        DWORD  retcode;
        UINT   Socket;
        LPSTR  lpString;
        WORD   Flags, Len; 
        LPBYTE lpStruct;
        LPBYTE lpStruct_L;
        int    FromLen;
        struct sockaddr_in Sockaddr_in;

        memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));
         
        lpStruct_L = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
        FromLen    = (int)(short)GETWORD(lpStruct_L);
        lpStruct   = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
        Sockaddr_in.sin_family = (short)GETWORD(lpStruct);
        Sockaddr_in.sin_port   = GETWORD(lpStruct+2);
        Sockaddr_in.sin_addr.s_addr = GETDWORD(lpStruct+4);
         
        Flags = (int)(short)GETWORD(SP+12);
        Len   = (int)(short)GETWORD(SP+14);
        lpString = (LPSTR)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
        Socket   = (UINT)GETWORD(SP+20);
        retcode  = (int) Recvfrom( Socket,lpString,Len,Flags,(struct sockaddr *)&Sockaddr_in,(int FAR *)FromLen);
        /* fill in the length of the struct and the structure itself*/ 
        PUTWORD(lpStruct_L,FromLen);
        PUTWORD(lpStruct,Sockaddr_in.sin_family);
        PUTWORD(lpStruct+2,Sockaddr_in.sin_port);
        PUTDWORD(lpStruct+4,Sockaddr_in.sin_addr.s_addr);
        envp->reg.sp += UINT_86 + 3*LP_86 + 2*INT_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode); 
        
}
void IT_SELECT     ( ENV *envp, LONGPROC f)
{
        DWORD     retcode;
        WORD      Nfds;
        LPBYTE    lpStruct_rfds, lpStruct_wfds,lpStruct_exfds;  
        LPBYTE    lpStruct_Time;
        fd_set Read_fds, Write_fds, Except_fds;
        struct    timeval TimeOut; 
        WORD      Rfds_count,Wfds_count,Exfds_count,SockCnt;
        UINT      Sock;
        int       i;
        
        lpStruct_Time   = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
        lpStruct_exfds  = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
        lpStruct_wfds   = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
        lpStruct_rfds   = (LPBYTE)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
        Nfds            = GETWORD(SP+20);
        Rfds_count      = GETWORD(lpStruct_rfds);
        Wfds_count      = GETWORD(lpStruct_wfds); 
        Exfds_count     = GETWORD(lpStruct_exfds); 
        
        TimeOut.tv_sec = GETDWORD(lpStruct_Time);
        TimeOut.tv_usec = GETDWORD(lpStruct_Time+4);

        FD_ZERO(&Read_fds);
        FD_ZERO(&Write_fds);
        FD_ZERO(&Except_fds);
    
        for ( i=0; i< (int)Rfds_count; i++ ) {
              Sock = (UINT)GETWORD(lpStruct_rfds+i+2); 
              FD_SET(Sock,&Read_fds);
        }  
        for ( i=0; i< (int)Wfds_count; i++ ) {
              Sock = (UINT)GETWORD(lpStruct_wfds+i+2); 
              FD_SET(Sock,&Write_fds);
        } 
        for ( i=0; i< (int)Exfds_count; i++ ){
              Sock = (UINT)GETWORD(lpStruct_exfds+i+2); 
              FD_SET(Sock,&Except_fds);
        }
         retcode = (int)Select(Nfds,&Read_fds,&Write_fds,&Except_fds,&TimeOut);

        if ( (int)retcode > 0 )
          {
             for ( i = 0,SockCnt=0; i < (int)Rfds_count ; i++ )
                if (FD_ISSET(GETWORD(lpStruct_rfds+i+2),&Read_fds))
                    SockCnt++;

             if ( (int)SockCnt > 0 ){
                PUTWORD(lpStruct_rfds,SockCnt);
             }

             for ( i = 0,SockCnt=0; i < (int)Wfds_count ; i++ )
                if (FD_ISSET(GETWORD(lpStruct_wfds+i+2),&Write_fds))
                    SockCnt++;
             if ( SockCnt > 0 ) {
               PUTWORD(lpStruct_wfds,SockCnt);
             }

             for ( i = 0,SockCnt=0; i < (int)Exfds_count ; i++ )
                if (FD_ISSET(GETWORD(lpStruct_exfds+i+2),&Except_fds))
                    SockCnt++;
             if ( (int)SockCnt > 0 ) {
               PUTWORD(lpStruct_exfds,SockCnt);
             }
             if (lpStruct_Time) {
             PUTDWORD(lpStruct_Time,TimeOut.tv_sec); 
             PUTDWORD(lpStruct_Time+4,TimeOut.tv_usec);
             }

          }

        else if ( retcode == 0 )
          {
             if(lpStruct_rfds) *lpStruct_rfds = 0x0;
             if(lpStruct_wfds) *lpStruct_wfds = 0x0;
             if(lpStruct_exfds)*lpStruct_exfds = 0x0;
          }
          
        envp->reg.sp += INT_86 + 4*LP_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode); 
        
}

void IT_SEND     ( ENV *envp, LONGPROC f)
{
        DWORD  retcode;
        UINT   Socket;
        LPSTR  lpString;
        WORD   Flags, Len; 
        
        Flags = (int)(short)GETWORD(SP+4);
        Len   = (int)(short)GETWORD(SP+6);
        lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
        Socket   = (UINT)GETWORD(SP+12);
        retcode = Send (Socket, lpString, Len, Flags);
        envp->reg.sp += UINT_86 + LP_86 + 2*INT_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode); 

}

void IT_SHUTDOWN  ( ENV *envp, LONGPROC f)
{

        DWORD retcode;
        UINT  Socket;
        WORD  How;

        How    = (int)(short) GETWORD(SP+4);
        Socket = (UINT)GETWORD(SP+6);
        retcode = (int)Shutdown((UINT)Socket,(int)How);
        envp->reg.sp += UINT_86 + INT_86 + RET_86;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode);

}

void IT_SENDTO     ( ENV *envp, LONGPROC f)
{
        DWORD  retcode;
        UINT   Socket;
        LPSTR  lpString;
        WORD   Flags, Len; 
        LPBYTE lpStruct;
        int    ToLen;
        struct sockaddr_in Sockaddr_in;
         
        memset(&Sockaddr_in,0,sizeof(struct sockaddr_in));

        ToLen      = (int)(short)GETWORD(SP+4);
        lpStruct   = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
        Sockaddr_in.sin_family = (short)GETWORD(lpStruct);
        Sockaddr_in.sin_port   = GETWORD(lpStruct+2);
        Sockaddr_in.sin_addr.s_addr = GETDWORD(lpStruct+4);
         
        Flags = (int)(short)GETWORD(SP+10);
        Len   = (int)(short)GETWORD(SP+12);
        lpString = (LPSTR)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
        Socket   = (UINT)GETWORD(SP+18);
        retcode  = (int) Sendto( Socket,lpString,Len,Flags,(struct sockaddr *)&Sockaddr_in,ToLen);

        /* fill in the struct and the structure itself*/ 
        PUTWORD(lpStruct,Sockaddr_in.sin_family);
        PUTWORD(lpStruct+2,Sockaddr_in.sin_port);
        PUTDWORD(lpStruct+4,Sockaddr_in.sin_addr.s_addr);
        envp->reg.sp += UINT_86 + 2*LP_86 + 3*INT_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode); 

}
void IT_SETSOCKOPT(ENV *envp, LONGPROC f)
{
        DWORD  retcode;
        UINT   Socket;
        WORD   Level,Optname,Optlen;
        LPSTR Optval;
        LPBYTE lpStr;

        Optlen    = GETWORD(SP+4);
        
        lpStr     = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
        Optval    = (LPSTR)GetAddress(GETWORD(lpStr+2),GETWORD(lpStr));
        Optname   = GETWORD(SP+10);
        Level     = GETWORD(SP+12);

/*   Here is a patch for now All Windows WinSock Applications
     send 0xffff for the level whereas it should be one of the valid responses ,
     So what we do now is to set it to Level 1 for Sockets which is SOL_SOCKET 
*/
        if ( Level & 0xffff ) Level =SOL_SOCKET;
        Socket    = GETWORD(SP+14);

        retcode = (int)Setsockopt(Socket,Level,Optname,Optval,Optlen);

        envp->reg.sp += UINT_86 + LP_86 + 3*INT_86 + RET_86 ;
        envp->reg.ax  = LOWORD(retcode);
        envp->reg.dx  = HIWORD(retcode); 


}
void IT_GETHOSTBYADDR   ( ENV *envp, LONGPROC f)
{

	struct hostent FAR *PASCAL FAR lp;
	LPSTR	addr;
        LPBYTE  lpstr;
        WORD    len,type; 
	DWORD	retcode,dw;
	
	lpstr  = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
        dw     = GETDWORD(lpstr);
        len   = GETWORD(SP+6);
        type  = GETWORD(SP+4);
 	addr = (LPSTR)&dw; 
	lp = Gethostbyaddr((LPCSTR)addr,len,type);

	if(lp) {
	    hostent_nat_to_bin(lp,&wsa.uGetHostByName_Sync);
	    retcode = (DWORD)MAKELP(wsa.uGetHostByName_Sync,0);
	}
	else
	    retcode = 0;

	envp->reg.sp += LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode); 	/* note: offset by link */
	envp->reg.dx = HIWORD(retcode);		/* the actual selector  */

}

void IT_GETHOSTBYNAME   ( ENV *envp, LONGPROC f)
{
	struct hostent FAR *PASCAL FAR lp;
	DWORD	retcode;
	
	lp = Gethostbyname((LPCSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4)));

	if(lp) {
	    hostent_nat_to_bin(lp,&wsa.uGetHostByName_Sync);
	    retcode = (DWORD)MAKELP(wsa.uGetHostByName_Sync,0);
	}
	else
	    retcode = 0;

	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode); 	/* note: offset by link */
	envp->reg.dx = HIWORD(retcode);		/* the actual selector  */
}


static void hostent_nat_to_bin(struct hostent *lpse, UINT *lpSel)
{
    DWORD dw;
    int nLen,nMemoryLen,nMemoryLenOld,nAliasCount,nAddrCount;
    int i;
    LPSTR lpData;
    UINT sel = *lpSel;
    UINT offset = 0,alias_offset,addr_offset;

    for (i = 0, nLen = 0; lpse->h_aliases[i]; i++)
	nLen += strlen(lpse->h_aliases[i])+1;
    nAliasCount = i+1;

    for (i = 0; lpse->h_addr_list[i]; i++)
	nLen += DWORD_86;
    nAddrCount = i+1;

    if (lpse->h_name)
	nLen += strlen(lpse->h_name)+1;

    nMemoryLen =  LP_86 + LP_86 + LP_86 + nAliasCount*LP_86 + nAddrCount*LP_86 + INT_86 + INT_86 + nLen;
    if (sel == 0) {
	lpData = WinMalloc(nMemoryLen);
	*lpSel = AssignSelector((LPBYTE)lpData,0,TRANSFER_DATA,nMemoryLen);
	sel = *lpSel;
    }
    else {
	nMemoryLenOld = GetSelectorLimit(sel);
	lpData = (LPSTR)GetSelectorBase(sel);
	if (nMemoryLen > nMemoryLenOld) {
	    lpData = WinRealloc(lpData,nMemoryLen);
	    SetSelectorLimit(sel,nMemoryLen);
	    SetPhysicalAddress(sel,lpData);
	    DPMI_Notify(DN_MODIFY,sel);
	}
    }
    wsa.lpGetHostMemPtr = lpData; 
    alias_offset = LP_86 + LP_86 + INT_86 + INT_86 + LP_86;
    offset = alias_offset + nAliasCount*LP_86 + nAddrCount*LP_86;

    if (lpse->h_name) {
	strcpy(lpData + offset, lpse->h_name);
	PUTWORD(lpData, offset);
	PUTWORD(lpData+2,sel);
	offset += strlen(lpse->h_name)+1;
    }
    else
	PUTDWORD(lpData,0);

    PUTWORD(lpData+8,lpse->h_addrtype);
    PUTWORD(lpData+10,lpse->h_length);

    PUTWORD(lpData+4, alias_offset);
    PUTWORD(lpData+6,sel);

    for (i = 0; i < nAliasCount-1; i++) {
	strcpy(lpData + offset, lpse->h_aliases[i]);
	PUTWORD(lpData+alias_offset + i*LP_86, offset);
	PUTWORD(lpData+alias_offset + i*LP_86+2, sel);
	offset += strlen(lpse->h_aliases[i]) + 1;
    }
    PUTDWORD(lpData+alias_offset+i*LP_86,0);

    addr_offset = alias_offset + nAliasCount*LP_86;
    PUTWORD(lpData+12, addr_offset);
    PUTWORD(lpData+14,sel);

    for (i = 0; i < nAddrCount-1; i++) {
	dw = *((LPDWORD)lpse->h_addr_list[i]);
	PUTDWORD(lpData+offset,dw);
	PUTWORD(lpData+addr_offset + i*LP_86, offset);
	PUTWORD(lpData+addr_offset + i*LP_86+2, sel);
	offset += DWORD_86;
    }
    PUTDWORD(lpData+addr_offset+i*LP_86,0);
}

void IT_GETHOSTNAME   ( ENV *envp, LONGPROC f)
{
      DWORD	retcode;
      LPSTR     name;

      name = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6)); 
      retcode = Gethostname(name,GETWORD(SP+4));
      envp->reg.sp += LP_86 + INT_86 + RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);
}

void IT_GETSERVBYPORT   ( ENV *envp, LONGPROC f)
{

	struct servent FAR *PASCAL FAR lp;
        WORD    Port;
	DWORD	retcode;
	
	Port  = GETWORD(SP+8);

	lp = Getservbyport(Port,(LPCSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4)));

	if(lp) {
	    servent_nat_to_bin(lp,&wsa.uGetServByName_Sync);
	    retcode = (DWORD)MAKELP(wsa.uGetServByName_Sync,0);
	}
	else
	    retcode = 0;

	envp->reg.sp += INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode); 	/* note: offset is zero */
	envp->reg.dx = HIWORD(retcode);		/* the actual selector  */
}

void IT_GETPROTOBYNUMBER( ENV *envp, LONGPROC f)
{
	struct protoent FAR *PASCAL FAR lp;
	DWORD	retcode;
        int     Proto;
	
	Proto  = GETWORD(SP+4);

	lp = Getprotobynumber(Proto);

	if(lp) {
	    protoent_nat_to_bin(lp,&wsa.uGetProtoByName_Sync);
	    retcode = (DWORD)MAKELP(wsa.uGetProtoByName_Sync,0);
	}
	else
	    retcode = 0;

	envp->reg.sp += INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode); 	/* note: offset by link */
	envp->reg.dx = HIWORD(retcode);		/* the actual selector  */

}

void IT_GETPROTOBYNAME  ( ENV *envp, LONGPROC f)
{
	struct protoent FAR *PASCAL FAR lp;
	DWORD	retcode;
	
	lp = Getprotobyname((LPCSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4)));

	if(lp) {
	    protoent_nat_to_bin(lp,&wsa.uGetProtoByName_Sync);
	    retcode = (DWORD)MAKELP(wsa.uGetProtoByName_Sync,0);
	}
	else
	    retcode = 0;

	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode); 	/* note: offset by link */
	envp->reg.dx = HIWORD(retcode);		/* the actual selector  */

}

static void protoent_nat_to_bin(struct protoent *lpse, UINT *lpSel)
{
    int nLen,nMemoryLen,nMemoryLenOld,nAliasCount;
    int i;
    LPSTR lpData;
    UINT sel = *lpSel;
    UINT offset = 0,alias_offset;

    for (i = 0, nLen = 0; lpse->p_aliases[i]; i++)
	nLen += strlen(lpse->p_aliases[i])+1;
    nAliasCount = i+1;
    if (lpse->p_name)
	nLen += strlen(lpse->p_name)+1;

    nMemoryLen =  LP_86 + LP_86 + nAliasCount*LP_86 + INT_86 + nLen;
    if (sel == 0) {
	lpData = WinMalloc(nMemoryLen);
	*lpSel = AssignSelector((LPBYTE)lpData,0,TRANSFER_DATA,nMemoryLen);
	sel = *lpSel;
    }
    else {
	nMemoryLenOld = GetSelectorLimit(sel);
	lpData = (LPSTR)GetSelectorBase(sel);
	if (nMemoryLen > nMemoryLenOld) {
	    lpData = WinRealloc(lpData,nMemoryLen);
	    SetSelectorLimit(sel,nMemoryLen);
	    SetPhysicalAddress(sel,lpData);
	    DPMI_Notify(DN_MODIFY,sel);
	}
    }
    wsa.lpGetProtoMemPtr = lpData; 
    alias_offset = LP_86 + LP_86 + INT_86 ;
    offset = alias_offset + nAliasCount*LP_86;

    if (lpse->p_name) {
	strcpy(lpData + offset, lpse->p_name);
	PUTWORD(lpData, offset);
	PUTWORD(lpData+2,sel);
	offset += strlen(lpse->p_name)+1;
    }
    else
	PUTDWORD(lpData,0);

    PUTWORD(lpData+8,lpse->p_proto);

    PUTWORD(lpData+4, alias_offset);
    PUTWORD(lpData+6,sel);

    for (i = 0; i < nAliasCount-1; i++) {
	strcpy(lpData + offset, lpse->p_aliases[i]);
	PUTWORD(lpData+alias_offset + i*LP_86, offset);
	PUTWORD(lpData+alias_offset + i*LP_86+2, sel);
	offset += strlen(lpse->p_aliases[i]) + 1;
    }
    PUTDWORD(lpData+alias_offset+i*LP_86,0);
}
void IT_WSACLEANUP( ENV *envp, LONGPROC f)
{
       WSACleanup();
       FreeNtoaSelList();
       FreeSelector(wsa.uGetServByName_Sync);
       WinFree(wsa.lpGetServMemPtr);
       FreeSelector(wsa.uGetProtoByName_Sync);
       WinFree(wsa.lpGetProtoMemPtr);
       FreeSelector(wsa.uGetHostByName_Sync);
       WinFree(wsa.lpGetHostMemPtr);
       envp->reg.sp +=  RET_86;
       envp->reg.ax = 0;
       envp->reg.dx = 0;
}
 
void IT_WSASETLASTERROR    ( ENV *envp, LONGPROC f)
{
      int   Ierror;

      Ierror = (int)(short) GETWORD(SP+4);
      WSASetLastError(Ierror);
      envp->reg.sp += INT_86 + RET_86;
      envp->reg.ax = 0;
      envp->reg.dx = 0;
}
 
void IT_WSAGETLASTERROR    ( ENV *envp, LONGPROC f)
{
      DWORD retcode;

      retcode = (int) WSAGetLastError();
      envp->reg.sp += RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);

}
 
void IT_WSAISBLOCKING    ( ENV *envp, LONGPROC f)
{
      DWORD retcode;

      retcode = (BOOL) WSAIsBlocking();
      envp->reg.sp += RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);

}

void IT_WSAUNHOOKBLOCKINGHOOK    ( ENV *envp, LONGPROC f)
{

      DWORD retcode;

      retcode = (int) WSAUnhookBlockingHook();
      envp->reg.sp += RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);

}

void IT_WSASETBLOCKINGHOOK       ( ENV *envp, LONGPROC f)
{
        MessageBox((HWND)0,"Function WSASetBlockingHook() !",
                     "Unimplemented feature", MB_OK | MB_ICONSTOP);

}

void IT_WSACANCELBLOCKINGCALL    ( ENV *envp, LONGPROC f)
{
      DWORD retcode;

      retcode = (int) WSACancelBlockingCall();
      envp->reg.sp += RET_86;
      envp->reg.ax = LOWORD(retcode);
      envp->reg.dx = HIWORD(retcode);

}

void IT_WSAASYNCGETSERVBYNAME    ( ENV *envp, LONGPROC f)
{

   DWORD   retcode;
   UINT    Wmsg_in;
   LPSTR  Name;
   LPSTR  Proto;
   LPSTR   Buffer_in;
   WORD    Buf_len;
   HANDLE  Handle_in;

   struct  servent FAR * PASCAL FAR se;
       
   Buf_len    =  GETWORD(SP+4);
   Buffer_in  =  GetAddress(GETWORD(SP+8),GETWORD(SP+6));
   Proto      =  GetAddress(GETWORD(SP+12),GETWORD(SP+10));
   Name       =  GetAddress(GETWORD(SP+16),GETWORD(SP+14));
   Wmsg_in    =  GETWORD(SP+18);
   Handle_in  =  GETWORD(SP+20);      

    se = Getservbyname(Name,Proto);

	if (se) 
          {
	    servent_nat_to_bin(se,&wsa.uGetServByName_Sync);
            memcpy((LPSTR)Buffer_in,(LPSTR)wsa.lpGetServMemPtr,Buf_len); 
/* we need to address some more scenario's here as per the WinSock Spec's Later 
   especially the return values  and asynctask handles */
            retcode = 3;
            PostMessage(Handle_in,Wmsg_in,3,0);     
	  }
	else
	    retcode = 0;

   envp->reg.sp += HANDLE_86 + UINT_86 + 3*LP_86 + INT_86 + RET_86;
   envp->reg.ax = LOWORD(retcode);
   envp->reg.dx = HIWORD(retcode);


}

void IT_WSAASYNCGETSERVBYPORT    ( ENV *envp, LONGPROC f)
{
   DWORD   retcode;
   UINT    Wmsg_in;
   WORD    Port;
   LPSTR  Proto;
   LPSTR   Buffer_in;
   WORD    Buf_len;
   HANDLE  Handle_in;

   struct  servent FAR * PASCAL FAR se;
       
   Buf_len    =  GETWORD(SP+4);
   Buffer_in  =  GetAddress(GETWORD(SP+8),GETWORD(SP+6));
   Proto      =  GetAddress(GETWORD(SP+12),GETWORD(SP+10));
   Port       =  GETWORD(SP+14);
   Wmsg_in    =  GETWORD(SP+16);
   Handle_in  =  GETWORD(SP+18);      

    se = Getservbyport(Port,Proto);

	if (se) 
          {
	    servent_nat_to_bin(se,&wsa.uGetServByName_Sync);
            memcpy((LPSTR)Buffer_in,(LPSTR)wsa.lpGetServMemPtr,Buf_len); 
/* we need to address some more scenario's here as per the WinSock Spec's Later 
   especially the return values  and asynctask handles */
            retcode = 4;
            PostMessage(Handle_in,Wmsg_in,4,0);     
	  }
	else
	    retcode = 0;

   envp->reg.sp += HANDLE_86 + UINT_86 + INT_86 + 2*LP_86 + INT_86 + RET_86;
   envp->reg.ax = LOWORD(retcode);
   envp->reg.dx = HIWORD(retcode);


}

void IT_WSAASYNCGETPROTOBYNAME   ( ENV *envp, LONGPROC f)
{
   DWORD   retcode;
   UINT    Wmsg_in;
   LPSTR   ProtoName;
   LPSTR   Buffer_in;
   WORD    Buf_len;
   HANDLE  Handle_in;

   struct  protoent FAR * PASCAL FAR pe;
   
   Buf_len    =  GETWORD(SP+4);
   Buffer_in  =  GetAddress(GETWORD(SP+8),GETWORD(SP+6));
   ProtoName   =  GetAddress(GETWORD(SP+12),GETWORD(SP+10));
   Wmsg_in    =  GETWORD(SP+14);
   Handle_in  =  GETWORD(SP+16);      
 
    pe = Getprotobyname(ProtoName);

	if (pe) 
          {
	    protoent_nat_to_bin(pe,&wsa.uGetProtoByName_Sync);
            memcpy((LPSTR)Buffer_in,(LPSTR)wsa.lpGetProtoMemPtr,Buf_len); 
/* we need to address some more scenario's here as per the WinSock Spec's Later 
   especially the return values  and asynctask handles */
            retcode = 5;
            PostMessage(Handle_in,Wmsg_in,5,0);     
	  }
	else
	    retcode = 0;

   envp->reg.sp += HANDLE_86 + UINT_86 + 2*LP_86 + INT_86 + RET_86;
   envp->reg.ax = LOWORD(retcode);
   envp->reg.dx = HIWORD(retcode);

}


void IT_WSAASYNCGETPROTOBYNUMBER ( ENV *envp, LONGPROC f)
{
   DWORD   retcode;
   UINT    Wmsg_in;
   WORD    ProtoNum;
   LPSTR   Buffer_in;
   WORD    Buf_len;
   HANDLE  Handle_in;

   struct  protoent FAR * PASCAL FAR pe;
   
   Buf_len    =  GETWORD(SP+4);
   Buffer_in  =  GetAddress(GETWORD(SP+8),GETWORD(SP+6));
   ProtoNum   =  GETWORD(SP+10);
   Wmsg_in    =  GETWORD(SP+12);
   Handle_in  =  GETWORD(SP+14);      
 
    pe = Getprotobynumber(ProtoNum);

	if (pe) 
          {
	    protoent_nat_to_bin(pe,&wsa.uGetProtoByName_Sync);
            memcpy((LPSTR)Buffer_in,(LPSTR)wsa.lpGetProtoMemPtr,Buf_len); 
/* we need to address some more scenario's here as per the WinSock Spec's Later 
   especially the return values  and asynctask handles */
            retcode = 6;
            PostMessage(Handle_in,Wmsg_in,6,0);     
	  }
	else
	    retcode = 0;

   envp->reg.sp += HANDLE_86 + UINT_86 + INT_86 + LP_86 + INT_86 + RET_86;
   envp->reg.ax = LOWORD(retcode);
   envp->reg.dx = HIWORD(retcode);

}

void IT_WSAASYNCGETHOSTBYNAME    ( ENV *envp, LONGPROC f)
{
   DWORD   retcode;
   UINT    Wmsg_in;
   LPSTR  HostName;
   LPSTR   Buffer_in;
   WORD    Buf_len;
   HANDLE  Handle_in;

   struct  hostent FAR * PASCAL FAR he;
   
   Buf_len    =  GETWORD(SP+4);
   Buffer_in  =  GetAddress(GETWORD(SP+8),GETWORD(SP+6));
   HostName   =  GetAddress(GETWORD(SP+12),GETWORD(SP+10));
   Wmsg_in    =  GETWORD(SP+14);
   Handle_in  =  GETWORD(SP+16);      
 
    he = Gethostbyname(HostName);

	if (he) 
          {
	    hostent_nat_to_bin(he,&wsa.uGetHostByName_Sync);
            memcpy((LPSTR)Buffer_in,(LPSTR)wsa.lpGetHostMemPtr,Buf_len); 
/* we need to address some more scenario's here as per the WinSock Spec's Later 
   especially the return values  and asynctask handles */
            retcode = 2;
            PostMessage(Handle_in,Wmsg_in,2,0);     
	  }
	else
	    retcode = 0;

   envp->reg.sp += HANDLE_86 + UINT_86 + 2*LP_86 + INT_86 + RET_86;
   envp->reg.ax = LOWORD(retcode);
   envp->reg.dx = HIWORD(retcode);

}

void IT_WSAASYNCGETHOSTBYADDR    ( ENV *envp, LONGPROC f)
{
   DWORD   retcode,dw;
   UINT    Wmsg_in;
   LPDWORD Addr;
   LPBYTE  lpstr;
   LPSTR   Buffer_in;
   WORD    Buf_len,Len,Type;
   HANDLE  Handle_in;

   struct  hostent FAR * PASCAL FAR he;
   
   Buf_len    =  GETWORD(SP+4);
   Buffer_in  =  GetAddress(GETWORD(SP+8),GETWORD(SP+6));
   Type       =  GETWORD(SP+10);
   Len        =  (int)(short)GETWORD(SP+12);
   lpstr      =  (LPBYTE)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
   dw         =  GETDWORD(lpstr);
   Addr       =  &dw;
   Wmsg_in    =  GETWORD(SP+18);
   Handle_in  =  GETWORD(SP+20);      
 
    he = Gethostbyaddr((LPCSTR)Addr,Len,Type);

	if (he) 
          {
	    hostent_nat_to_bin(he,&wsa.uGetHostByName_Sync);
            memcpy((LPSTR)Buffer_in,(LPSTR)wsa.lpGetHostMemPtr,Buf_len); 
/* we need to address some more scenario's here as per the WinSock Spec's Later 
   especially the return values  and asynctask handles */
            retcode = 7;
            PostMessage(Handle_in,Wmsg_in,7,0);     
	  }
	else
	    retcode = 0;

   envp->reg.sp += HANDLE_86 + UINT_86 + LP_86 + INT_86 + INT_86 + LP_86 + INT_86 + RET_86;
   envp->reg.ax = LOWORD(retcode);
   envp->reg.dx = HIWORD(retcode);

}

void IT_WSACANCELASYNCREQUEST( ENV *envp, LONGPROC f)
{
	DWORD retcode;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+4));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	retcode = WSACancelAsyncRequest(h);
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void IT_WSAASYNCSELECT ( ENV *envp, LONGPROC f)
{
        DWORD     retcode;
        WORD      Socket; 
        UINT      Wmsg_in;
        HANDLE    Handle_in;
        DWORD     Levent; 
     
        Levent    = GETDWORD(SP+4);
        Wmsg_in   = GETWORD(SP+8);
        Handle_in = GETWORD(SP+10);
        Socket    = GETWORD(SP+12); 

        retcode = WSAAsyncSelect(Socket,Handle_in, Wmsg_in, Levent);       

        envp->reg.sp += UINT_86 + HANDLE_86 + UINT_86 + DWORD_86 + RET_86;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);
}

void IT__WSAFDISSET ( ENV *envp, LONGPROC f)
{

        DWORD     retcode;
        LPBYTE    lpStruct;
        WORD      Sock,SockCnt,Sock_in;
        int       i;
        fd_set Set;
        lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
        Sock_in  = GETWORD(SP+8);
        SockCnt  = GETWORD(lpStruct); 

        FD_ZERO(&Set);
    
        for ( i=0; i< (int)SockCnt; i++ ) {
              Sock = (UINT)GETWORD(lpStruct+i+2); 
              FD_SET(Sock,&Set);
        }  
           if (FD_ISSET(Sock_in,&Set))
             retcode  = 1 ;
           else
             retcode  = 0;

        envp->reg.sp += UINT_86 + LP_86 + RET_86;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);
         

}
WORD FindFreeNTOASel(void)
{
  if (NtoaSelCtr != 0 )
   NtoaSelCtr++;
  return NtoaSelCtr;
}
void ResetNtoaSelList(void)
{
  int NtoaIndex;
      NtoaSelCtr = 0;
      for (NtoaIndex=0;NtoaIndex<=100;NtoaIndex++)
             NtoaSelList[NtoaIndex] = 0;     

}
void FreeNtoaSelList(void)
{
  int NtoaIndex;
  for(NtoaIndex = 0;NtoaIndex <=100;NtoaIndex++)
    {
        if ( NtoaSelList[NtoaIndex] != 0 )
              FreeSelector(NtoaSelList[NtoaIndex]);
        else
          return; 
    }
}

