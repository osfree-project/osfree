/*    
	Comm.c	2.20
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

/*
*   Implements the following functions:
*   -----------------------------------
*   NO	int     WINAPI BuildCommDCB(LPCSTR, DCB FAR*);
*
*   YES	int     WINAPI OpenComm(LPCSTR, UINT, UINT);
*   YES	int     WINAPI CloseComm(int);
*
*   YES	int     WINAPI ReadComm(int, void FAR*, int);
*   YES	int     WINAPI WriteComm(int, const void FAR*, int);
*
*   YES	int     WINAPI UngetCommChar(int, char);
*	int     WINAPI FlushComm(int, int);
*	int     WINAPI TransmitCommChar(int, char);
*
*   YES	int     WINAPI SetCommState(const DCB FAR*);
*   YES	int     WINAPI GetCommState(int, DCB FAR*);
*   YES	int     WINAPI GetCommError(int, COMSTAT FAR* );
*
*	int     WINAPI SetCommBreak(int);
*	int     WINAPI ClearCommBreak(int);
*
*	UINT FAR* WINAPI SetCommEventMask(int, UINT);
*	UINT    WINAPI GetCommEventMask(int, int);
*
*	LONG    WINAPI EscapeCommFunction(int, int);
*
*	BOOL    WINAPI EnableCommNotification(int, HWND, int, int);
*
********************************************************************/


#include "windows.h"
#include "windowsx.h"

#include "Comm.h"
#include "Log.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "WinConfig.h"

#include <ctype.h>

#define _SVID3
//#include <termios.h>
#undef _SVID3

#ifndef CRTSCTS
#define CRTSCTS 020000000000
#endif /* !CRTSCTS */

#include <errno.h>
#include <sys/stat.h>

#define LF_COMM		LF_LOG

#define Control(x)	(toupper(x) - 'A' + 1)

typedef struct DosDeviceStruct {
	struct DosDeviceStruct *dds_next; /* linked list of devices... */
	char *dds_dosname;	/* dos device name */
	char *dds_devicename;	/* device actual name */
	int   dds_fd;		/* device file descriptor */
	int   dds_status; 	/* status... */
	int   dds_error;	/* last error with this device  */
	int   dds_unget;	/* ungetc from UngetCommChar	*/
} DDS;

static struct {
	int	commerror;
	int	maxlpt;
	int	maxcomm;
	DDS    *list;
} pdevsw; 

static int DriverCommInit(DDS *,int);
static int DriverCommSetDCB(const DCB *);
static int DriverCommGetDCB(DCB *);

/*
 * common hook for all windows comm functions
 * set breakpoint here to trap any/all comm functions
 */
static void CommTrap(char* lpstr)
{
	if(lpstr)
		logstr(-1,"COMMTRAP: %s\n",lpstr);
}

/* 
 * get device pointer for given port id, or dos device name
 */
static DDS *
GetCommPort(int flag,int fd,LPCSTR port)
{
	DDS	*lpdds;
	for (lpdds = pdevsw.list;lpdds;lpdds = lpdds->dds_next) {
		if(flag && lpdds->dds_fd == fd)
			break;
		if(flag==0 && strcmp(lpdds->dds_dosname,port) == 0)
			break;
	}
	return lpdds;	
}

int WINAPI 
BuildCommDCB(LPCSTR device, DCB FAR *lpdcb)
{
	/* "COM1:9600,n,8,1"	*/
	/*  012345		*/

	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: BuildCommDCB: (%s), ptr %d\n", device, lpdcb));

	return 0;
}

/*
 * opencomm
 * open a comm port
 *	get mapped device name,
 *	open device,
 *	initialize device
 */
int 
OpenComm(LPCSTR device, UINT cbInQueue, UINT cbOutQueue)
{
	DDS	*lpdds;
	char	lpstr[128];
	char	portname[128];

	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: OpenComm: %s, %d, %d\n", device, cbInQueue, cbOutQueue));

	/* what device do we want */
	lpdds = GetCommPort(0,0,device);
	
	/* not found lets create it... */
	if(lpdds == 0) {
		/* allocate a structure */
		lpdds = (DDS *)WinMalloc(sizeof(struct DosDeviceStruct));
		memset(lpdds,0,sizeof(struct DosDeviceStruct));

		/* set the name(s) */
		lpdds->dds_dosname = (char *) WinMalloc(strlen(device)+1);
		strcpy(lpdds->dds_dosname,device);

		strcpy(portname,device);
		if(strchr(portname,':') == 0)
			strcat(portname,":");

		/* get the native alias name */
		GetPrivateProfileString("PortAliases", portname, "/dev/modem", 
			lpstr, sizeof(lpstr), GetTwinFilename() );

		LOGSTR((LF_COMM,"COMM: device %s = %s\n",device,lpstr));

		lpdds->dds_devicename  = (char *) WinMalloc(strlen(lpstr));
		strcpy(lpdds->dds_devicename,lpstr);
		pdevsw.list = lpdds;
	}

	/* already opened? */
	if(lpdds->dds_fd) {
		/* device already opened */
		lpdds->dds_error = IE_OPEN;
		return -1;
	}

	/* initialize the device */
	return DriverCommInit(lpdds,0); 
}

/*
 * closecomm
 * 	close an opened device 
 */
int     WINAPI 
CloseComm(int fd)
{
	DDS	*lpdds;
	
	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: closecomm"));
	lpdds = GetCommPort(1,fd,0);
	if(lpdds && lpdds->dds_fd) {
		DriverCommInit(lpdds,1);
		close(lpdds->dds_fd);
		lpdds->dds_error = 0;
		lpdds->dds_fd = 0;
	}
	return 0;
}

int WINAPI 
ReadComm(int idComDev, void FAR *lpvBuf, int cbRead)
{
	int	cnt;
	DDS	*lpdds;

	CommTrap(0);
	if((lpdds = GetCommPort(1,idComDev,0))) {
		if(lpdds->dds_unget) {
			*(char *) lpvBuf = lpdds->dds_unget;
			lpdds->dds_unget = 0;
			return 1;
		}
		cnt = read(lpdds->dds_fd,lpvBuf,cbRead);
	} else 
		cnt = IE_BADID;
	return cnt;
}


int WINAPI 
WriteComm(int idComDev, const void *lpvBuf, int cbWrite)
{
	int	cnt;
	DDS	*lpdds;

	CommTrap(0);
	if((lpdds = GetCommPort(1,idComDev,0))) {
		cnt = write(lpdds->dds_fd,lpvBuf,cbWrite);
	} else cnt = IE_BADID;
	return cnt;
}

int UngetCommChar(int idComDev, char chUnget)
{
	DDS	*lpdds;

	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: UngetCommChar"));
	if((lpdds = GetCommPort(1,idComDev,0))) {
		if(lpdds->dds_unget)
			return 0;
		lpdds->dds_unget = chUnget;
	}
	return 0;
}

int WINAPI
FlushComm(int idComDev, int fnQueue)
{
	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: FlushComm"));
	return 0;
}  

int WINAPI
TransmitCommChar(int fd, char chTransmit)
{
	CommTrap("transmitcommchar");
	return 0;
}

int WINAPI 
SetCommState(const DCB FAR *lpdcb)
{
	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: setcommstate"));
	DriverCommSetDCB(lpdcb);
	return 0;
}

int WINAPI
GetCommState(int idComDev, DCB FAR *lpdcb)
{
	DDS	*lpdds;

	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: getcommstate"));

	/* initialize the structure */
	memset(lpdcb,0,sizeof(DCB));

	if((lpdds = GetCommPort(1,idComDev,0))) {

		lpdcb->Id = lpdds->dds_fd;
		DriverCommGetDCB(lpdcb);
	}
	return 0;
}

int WINAPI
GetCommError(int idComDev, COMSTAT FAR *lpStat)
{
	int	rc;
	DDS	*lpdds;

	CommTrap(0);
	LOGSTR((LF_COMM,"COMM: getcommerror"));

	/* initialize the structure */
	memset(lpStat,0,sizeof(COMSTAT));

	rc = 0;
	if((lpdds = GetCommPort(1,idComDev,0))) {
		rc = lpdds->dds_error;
		if(lpStat)
		   lpStat->status = rc;
		lpdds->dds_error = 0;
	}
	return rc;
}

int WINAPI SetCommBreak(int fd)
{
	CommTrap("setcommbreak");
	return 0;
}

int WINAPI ClearCommBreak(int fd)
{
	CommTrap("clearcommbreak");
	return 0;
}

UINT FAR* 
SetCommEventMask(int idComDev, UINT fuEvtMask)
{
	CommTrap("SetCommEventMask");
	return 0;
}

UINT 
GetCommEventMask(int fd, int fnEvtClear)
{
	CommTrap("getcommeventmask");
	return 0;
}

LONG 
EscapeCommFunction(int idComDev, int nFunction)
{
	CommTrap(0);

	LOGSTR((LF_LOG,"COMM: EscapeCommFunction(%d,%x)\n",
		idComDev,nFunction));
	switch(nFunction) {
	case SETXOFF:
		break;
	case SETXON:
		break;
	case SETRTS:
		break;
	case CLRRTS:
		break;
	case SETDTR:
		break;
	case CLRDTR:
		break;
	case RESETDEV:
		break;
	default:
		break;
	}

	return 0;
}

BOOL
EnableCommNotification(int idComDev, HWND hWnd, int cbWriteNotify, int cbOutQue)
{
        CommTrap("EnableCommNotification");
	return FALSE;
}

/*********************************************************************/

int
trmcom(int cid)
{
	CommTrap("trmcom");
	return -2;
}


int
cclrbrk(int cid)
{
	CommTrap("cclrbrk");
	return -1;
}


int
setque(int cid, qdb *lpqdb)
{
	CommTrap("setque");
	return -1;
}


WORD
setcom(DCB *lpdcb)
{
	CommTrap("setcom");
	return -1;
}


int
csetbrk(int cid)
{
	CommTrap("csetbrk");
	return -1;
}


void
SuspendOpenCommPorts(void)
{
	CommTrap("SuspendOpenCommPorts");
}


int
EnableNotification(int cid, HWND hWnd, WORD winTrigger, WORD wOutTrigger)
{
	CommTrap("EnableNotification");
	return FALSE;
}


WORD
ctx(int cid, int ch)
{
	CommTrap("ctx");
	return 0x4000;
}


LONG
cextfcn(int cid, int fcn)
{
	CommTrap("cextfcn");
	return -1;
}


LPWORD
cevt(int cid, int evtmask)
{
	CommTrap("cevt");
	return 0;
}


WORD
cevtGet(int cid, int evtmask)
{
	CommTrap("cevtGet");
	return 0;
}


int
reccom(int cid)
{
	CommTrap("reccom");
	return -1;
}


WORD
stacom(int cid, COMSTAT *cs)
{
	CommTrap("stacom");
	return -1;
}


WORD
sndcom(int cid, int ch)
{
	CommTrap("sndcom");
	return -1;
}


LPDCB
getdcb(int cid)
{
	CommTrap("getdcb");
	return 0;
}


WORD
cflush(int cid, int q)
{
	CommTrap("cflush");
	return -1;
}


void
ReactivateOpenCommPorts(void)
{
	CommTrap("ReactivateOpenCommPorts");
}


WORD
inicom(DCB *lpdcb)
{
	CommTrap("inicom");
	return -1;
}


void
ReadCommString(void)
{
	CommTrap("ReadCommString");
}

int
CommWriteString(int cid, LPSTR pbuf, int size)
{
	return WriteComm(cid,pbuf,size);
}

/**************************************************************************
 *
 * The following should be moved to the kernel driver.
 * This is the platform specific code for the comm driver.
 *
 * The functions are:
 *	1) DriverCommInit
 *		open the device in lpdds, and initialize it
 *	2) DriverCommSetDCB
 *		set comm port to values in the DCB
 *	2) DriverCommGetDCB
 *		get comm port values into the DCB
 *
 **************************************************************************/

static int DriverCommInit(DDS *lpdds,int Reset)
{
    static struct termios	old;
    struct termios		new;

    /* open the device ... */
    if(lpdds->dds_fd == 0) {
    	lpdds->dds_fd = open(lpdds->dds_devicename,2);
    }

    /* open success */
    if(lpdds->dds_fd < 0) {
	LOGSTR((-1,"COMM: ERROR: open failed on device %s (%s)\n",
		lpdds->dds_dosname,lpdds->dds_devicename));
	lpdds->dds_fd = 0;
	lpdds->dds_error = IE_NOPEN;
	return -1;
    }

    if (Reset == 0) {
	(void)tcgetattr(lpdds->dds_fd, &old);
	new = old;
	new.c_cc[VINTR] = -1;
	new.c_cc[VQUIT] = -1;
	new.c_lflag &= ~(ECHO | ICANON);
	new.c_iflag &= ~(ISTRIP | INPCK);
	new.c_iflag &= ~(ICRNL);
	new.c_cc[VMIN] = 0;
	new.c_cc[VTIME] = 0;
	(void)tcsetattr(lpdds->dds_fd, TCSANOW, &new);
    }
    else
	(void)tcsetattr(lpdds->dds_fd, TCSANOW, &old);
    return lpdds->dds_fd;
}

static int DriverCommSetDCB(const DCB *lpdcb)
{
	struct termios port;

	if (tcgetattr(lpdcb->Id, &port) == -1) {
		return -1;
	}

	port.c_cc[VMIN] = 0;
	port.c_cc[VTIME] = 0;

	LOGSTR((LF_COMM,"COMM: SetCommState: baudrate %d\n",lpdcb->BaudRate));
	port.c_cflag &= ~CBAUD;
	switch (lpdcb->BaudRate) {
		case 110:
		case CBR_110:
			port.c_cflag |= B110;
			break;		
		case 300:
		case CBR_300:
			port.c_cflag |= B300;
			break;		
		case 600:
		case CBR_600:
			port.c_cflag |= B600;
			break;		
		case 1200:
		case CBR_1200:
			port.c_cflag |= B1200;
			break;		
		case 2400:
		case CBR_2400:
			port.c_cflag |= B2400;
			break;		
		case 4800:
		case CBR_4800:
			port.c_cflag |= B4800;
			break;		
		case 9600:
		case CBR_9600:
			port.c_cflag |= B9600;
			break;		
		case 19200:
		case CBR_19200:
			port.c_cflag |= B19200;
			break;		
		case 38400:
		case CBR_38400:
			port.c_cflag |= B38400;
			break;		
		default:
			pdevsw.commerror = IE_BAUDRATE;
			return -1;
	}

	LOGSTR((LF_COMM,"COMM: SetCommState: bytesize %d\n",lpdcb->ByteSize));
	port.c_cflag &= ~CSIZE;
	switch (lpdcb->ByteSize) {
		case 5:
			port.c_cflag |= CS5;
			break;
		case 6:
			port.c_cflag |= CS6;
			break;
		case 7:
			port.c_cflag |= CS7;
			break;
		case 8:
			port.c_cflag |= CS8;
			break;
		default:
			pdevsw.commerror = IE_BYTESIZE;
			return -1;
	}

	LOGSTR((LF_COMM,"COMM: SetCommState: parity %d\n",lpdcb->Parity));
	port.c_cflag &= ~(PARENB | PARODD);
	if (lpdcb->fParity)
		switch (lpdcb->Parity) {
			case NOPARITY:
				port.c_iflag &= ~INPCK;
				break;
			case ODDPARITY:
				port.c_cflag |= (PARENB | PARODD);
				port.c_iflag |= INPCK;
				break;
			case EVENPARITY:
				port.c_cflag |= PARENB;
				port.c_iflag |= INPCK;
				break;
			default:
				pdevsw.commerror = IE_BYTESIZE;
				return -1;
		}
	

	LOGSTR((LF_COMM,"COMM: SetCommState: stopbits %d\n",lpdcb->StopBits));
	switch (lpdcb->StopBits) {
		case ONESTOPBIT:
				port.c_cflag &= ~CSTOPB;
				break;
		case TWOSTOPBITS:
				port.c_cflag |= CSTOPB;
				break;
		default:
			pdevsw.commerror = IE_BYTESIZE;
			return -1;
	}

	if (lpdcb->fDtrflow || lpdcb->fRtsflow || lpdcb->fOutxCtsFlow)
		port.c_cflag |= CRTSCTS;

	if (lpdcb->fDtrDisable) 
		port.c_cflag &= ~CRTSCTS;
	
	if (lpdcb->fInX)
		port.c_iflag |= IXON;
	if (lpdcb->fOutX)
		port.c_iflag |= IXOFF;

	if (tcsetattr(lpdcb->Id, TCSADRAIN, &port) == -1) {
		return -1;
	} else {
		pdevsw.commerror = 0;
		return 0;
	}
	return 0;
}

static int DriverCommGetDCB(DCB *lpdcb)
{
	struct termios port;

	if (tcgetattr(lpdcb->Id, &port) == -1) {
		return -1;
	}

	switch (port.c_cflag & CBAUD) {
		case B110:
			lpdcb->BaudRate = 110;
			break;
		case B300:
			lpdcb->BaudRate = 300;
			break;
		case B600:
			lpdcb->BaudRate = 600;
			break;
		case B1200:
			lpdcb->BaudRate = 1200;
			break;
		case B2400:
			lpdcb->BaudRate = 2400;
			break;
		case B4800:
			lpdcb->BaudRate = 4800;
			break;
		case B9600:
			lpdcb->BaudRate = 9600;
			break;
		case B19200:
			lpdcb->BaudRate = 19200;
			break;
		case B38400:
			lpdcb->BaudRate = 38400;
			break;
	}
	LOGSTR((LF_COMM,"COMM: getcommstate: baudrate = %d\n",lpdcb->BaudRate));

	switch (port.c_cflag & CSIZE) {
		case CS5:
			lpdcb->ByteSize = 5;
			break;
		case CS6:
			lpdcb->ByteSize = 6;
			break;
		case CS7:
			lpdcb->ByteSize = 7;
			break;
		case CS8:
			lpdcb->ByteSize = 8;
			break;
	}	
	LOGSTR((LF_COMM,"COMM: getcommstate: bytesize = %d\n",lpdcb->ByteSize));
	
	
	switch (port.c_cflag & ~(PARENB | PARODD)) {
		case 0:
			lpdcb->fParity = NOPARITY;
			break;
		case PARENB:
			lpdcb->fParity = EVENPARITY;
			break;
		case (PARENB | PARODD):
			lpdcb->fParity = ODDPARITY;		
			break;
	}
	LOGSTR((LF_COMM,"COMM: getcommstate: parity = %d\n",lpdcb->fParity));

	if (port.c_cflag & CSTOPB)
		lpdcb->StopBits = TWOSTOPBITS;
	else
		lpdcb->StopBits = ONESTOPBIT;
	LOGSTR((LF_COMM,"COMM: getcommstate: stopbits = %d\n",lpdcb->StopBits));

	lpdcb->RlsTimeout = 50;
	lpdcb->CtsTimeout = 50; 
	lpdcb->DsrTimeout = 50;
	lpdcb->fNull = 0;
	lpdcb->fChEvt = 0;
	lpdcb->fBinary = 1;

	lpdcb->fDtrDisable = 0;
	if (port.c_cflag & CRTSCTS) {
		lpdcb->fDtrflow = 1;
		lpdcb->fRtsflow = 1;
		lpdcb->fOutxCtsFlow = 1;
		lpdcb->fOutxDsrFlow = 1;
	} else 
		lpdcb->fDtrDisable = 1;

	if (port.c_iflag & IXON)
		lpdcb->fInX = 1;
	else
		lpdcb->fInX = 0;

	if (port.c_iflag & IXOFF)
		lpdcb->fOutX = 1;
	else
		lpdcb->fOutX = 0;

	lpdcb->XonChar =  Control('Q');
	lpdcb->XoffChar =  Control('S');

	lpdcb->XonLim = 10;
	lpdcb->XoffLim = 10;

	return 0;
}
