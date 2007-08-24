#pragma	title("List Box Control  --  Version 1.1 -- (LBSnd.C)")
#pragma	subtitle("  MMPM/2 Support - Interface Definitions")

/* Program name: Listbox.Dll	Title: A List Box Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  May	'94, page 12                    */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	first of a series.	*/
/*									*/
/* Program Requirements:  OS/2 2.x					*/
/*			  IBM C	Set++					*/
/*			  WATCOM C 386/9.0				*/
/*			  Borland C++ for OS/2				*/
/*			  OS/2 Toolkit					*/
/*			  MMPM/2 Toolkit				*/

/* Copyright ¸ International Business Machines Corp. 1991-1994		*/
/* Copyright ¸ 1989-1994  Prominare Inc.  All Rights Reserved.		*/

/************************************************************************/
/************************************************************************/
/*		       DISCLAIMER OF WARRANTIES.			*/
/************************************************************************/
/************************************************************************/
/*     The following [enclosed]	code is	source code created by the	*/
/*     authors.	 This source code is  provided to you solely		*/
/*     for the purpose of assisting you	in the development of your	*/
/*     applications.  The code is provided "AS IS", without		*/
/*     warranty	of any kind.  The authors shall	not be liable		*/
/*     for any damages arising out of your use of the source code,	*/
/*     even if they have been advised of the possibility of such	*/
/*     damages.	 It is provided	purely for instructional and		*/
/*     illustrative purposes.						*/
/************************************************************************/
/************************************************************************/

#pragma	info(noext)
#pragma	strings(readonly)

#define	INCL_DOS		   /* Include OS/2 DOS Kernal		*/
#define	INCL_OS2MM		   /* Include OS/2 MultiMedia Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

static char *MODID = "@(#)lbsnd.c:1.00";

#include <os2.h>
#include <os2me.h>
#include <string.h>

/* This	module contains	the routines that handle the sound management	*/
/* for the list	box.							*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoLBSnd LBSnd.C			*/

/* Filename:   LBSnd.C							*/

/*  Version:   2.00							*/
/*  Created:   1994-02-27						*/
/*  Revised:   1994-04-21						*/

/* Routines:   static VOID ShowMCIError(HWND hWnd, ULONG ulError);	*/
/*	       static VOID _System PlayWaveThread(ULONG	hSound);	*/
/*	       ULONG EXPENTRY LoadWaveFile(HWND	hWnd, PSZ pszWaveFile);	*/
/*	       VOID EXPENTRY UnloadWave(HWND hWnd, ULONG hSound);	*/
/*	       VOID EXPENTRY PlayWave(ULONG hSound);			*/


/* --------------------------------------------------------------------	*/

static VOID ShowMCIError(HWND hWnd, ULONG ulError);

#if defined(__IBMC__) || defined(__IBMCPP__)

static VOID _System PlayWaveThread(ULONG hSound);

#else
#if defined(__WATCOMC__)

static VOID __syscall PlayWaveThread(ULONG hSound);

#else
#if defined(__BCPLUSPLUS__) || defined(__BORLANDC__)

static VOID _syscall PlayWaveThread(ULONG hSound);

#endif
#endif
#endif

#if !defined(STACK_COMMITTED)

#define	STACK_COMMITTED	2UL

#endif

ULONG EXPENTRY LoadWaveFile(HWND hWnd, PSZ pszWaveFile);
VOID EXPENTRY UnloadWave(HWND hWnd, ULONG hSound);
VOID EXPENTRY PlayWave(ULONG hSound);

/************************************************************************/
/*									*/
/*     Private Functions						*/
/*									*/
/************************************************************************/

#pragma	subtitle("   MMPM/2 Support - mci Error Display Routine")
#pragma	page ( )

/* --- ShowMCIError -----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	display	a mci* error message.		*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND  hWnd;    =	Calling	Window Handle				*/
/*     ULONG ulError; =	mci Error Value					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID ShowMCIError(HWND hWnd, ULONG ulError)

{
CHAR szMessage[128];		   /* Error Message Buffer		*/

switch ( mciGetErrorString(ulError, szMessage, 128) )
   {
   case	MCIERR_SUCCESS :
       WinMessageBox(HWND_DESKTOP, hWnd,
		     szMessage,	"mci Error Message",
		     0UL, MB_OK	| MB_ICONHAND |	MB_MOVEABLE);
       break;

   case	MCIERR_INVALID_DEVICE_ID :
   case	MCIERR_OUTOFRANGE :
   case	MCIERR_INVALID_BUFFER :
   default :
       WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
		     "Unknown error!", "mci Error Message",
		     0UL, MB_OK	| MB_ICONHAND |	MB_MOVEABLE);
       break;
   }
}
#pragma	subtitle("   MMPM/2 Support - Control Window Procedure")
#pragma	page ( )

/* --- PlayWaveThread ---------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	play a loaded wave file	within a	*/
/*     separate	thread to ensure responsiveness.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     ULONG hSound; = Sound File Handle				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

#if defined(__IBMC__) || defined(__IBMCPP__)

static VOID _System PlayWaveThread(ULONG hSound)

#else
#if defined(__WATCOMC__)

static VOID __syscall PlayWaveThread(ULONG hSound)

#else
#if defined(__BCPLUSPLUS__) || defined(__BORLANDC__)

static VOID _syscall PlayWaveThread(ULONG hSound)

#endif
#endif
#endif

{
HAB		  habThread;	   /* Thread Anchor Block		*/
HMQ		  hmqThread;	   /* Thread Message Queue Handle	*/
MCI_GENERIC_PARMS mciGenericParms; /* Generic Parameters		*/
MCI_PLAY_PARMS	  mciPlayParms;	   /* Play Parameters			*/
MCI_SEEK_PARMS	  mciSeekParms;	   /* Seek Parameters			*/
ULONG		  ulError;	   /* Error Holder			*/

		       /* Register the thread with OS/2	PM		*/

habThread = WinInitialize(0UL);

		       /* Acquire the wave device first			*/

mciGenericParms.hwndCallback = (HWND)NULL;
if ( (ulError =	mciSendCommand((USHORT)hSound, MCI_ACQUIREDEVICE, MCI_WAIT,
			       (PVOID)&mciGenericParms,	0UL)) == MCIERR_SUCCESS	)
   {
		       /* Initialize the play parameters structure	*/

   mciPlayParms.hwndCallback = (HWND)NULL;

		       /* Try to play the loaded wave file		*/

   if (	(ulError = mciSendCommand((USHORT)hSound, MCI_PLAY, MCI_WAIT,
				   (PVOID)&mciPlayParms, 0UL)) != 0UL )
       {
		       /* Error	occurred, create a message queue so as	*/
		       /* to be	able to	display	the error message	*/
		       /* within a message box				*/

       hmqThread = WinCreateMsgQueue(habThread,	0L);
       ShowMCIError(HWND_DESKTOP, ulError);
       WinDestroyMsgQueue(hmqThread);

		       /* Stop the attempted playing of	the wave file	*/

       mciGenericParms.hwndCallback = (HWND)NULL;
       mciSendCommand((USHORT)hSound, MCI_STOP,	MCI_WAIT,
		      (PVOID)&mciGenericParms, 0UL);
       }
   else
       {
		       /* Initialize the seek parameters structure	*/

       memset(&mciSeekParms, 0,	sizeof(MCI_SEEK_PARMS));

		       /* Rewind the sound back	to the beginning to	*/
		       /* allow	it to be played	from the beginning	*/
		       /* a subsequent time				*/

       mciSendCommand((USHORT)hSound, MCI_SEEK,	MCI_WAIT | MCI_TO_START,
		      (PVOID)&mciSeekParms, 0UL);
       }
   }
else
   {
		       /* Error	occurred, create a message queue so as	*/
		       /* to be	able to	display	the error message	*/
		       /* within a message box				*/

   hmqThread = WinCreateMsgQueue(habThread, 0L);
   ShowMCIError(HWND_DESKTOP, ulError);
   WinDestroyMsgQueue(hmqThread);
   }
		       /* De-register the thread with OS/2 PM since the	*/
		       /* creation of the bitmap is complete		*/
WinTerminate(habThread);
		       /* Exit the thread				*/
DosExit(EXIT_THREAD, 0L);
}

/************************************************************************/
/************************************************************************/
/*									*/
/************************************************************************/
/************************************************************************/
/*									*/
/*     Public Functions							*/
/*									*/
/************************************************************************/
/************************************************************************/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   MMPM/2 Support - Wave File Load Function")
#pragma	page ( )

/* --- LoadWaveFile ------------------------------------ [ Public ] ---	*/
/*									*/
/*     This function is	used to	load a wave file specified and return	*/
/*     back to the caller a handle that	can be used to play the	file	*/
/*     as required.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND hWnd;	 = Window Handle				*/
/*     PSZ  pszWaveFile; = Wave	File to	Use				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     LoadWaveFile =  0 : Error Return					*/
/*		    = >0 : Wave	File Handle				*/
/*									*/
/* --------------------------------------------------------------------	*/

ULONG EXPENTRY LoadWaveFile(HWND hWnd, PSZ pszWaveFile)

{
BYTE			  szMMPMBase[CCHMAXPATH];  /* MMPM/2 Base Path	*/
BYTE			  szWavePath[CCHMAXPATH];  /* Wave File	Path	*/
FILESTATUS3		  fsts;			   /* File Status	*/
MCI_OPEN_PARMS		  mciOpenParms;		   /* Open Parameters	*/
MCI_SYSINFO_DEFAULTDEVICE mciSysInfoDefaultDevice; /* Default Device	*/
MCI_SYSINFO_PARMS	  mciSysInfoParms;	   /* Info Parameters	*/
MCI_SYSINFO_QUERY_NAME	  mciSysInfoQueryName;	   /* Query Name	*/
PSZ			  pszMMPMBase;		   /* Environment Ptr	*/
ULONG			  ulError;		   /* Error Holder	*/
register UINT n;		   /* Index				*/

		       /* Check	to see if the wave file	name valid and	*/
		       /* if it	isn't return with the null handle       */
		       /* to indicate the wave file was	not loaded	*/
if ( !pszWaveFile )
   return(0UL);
else
   if (	!pszWaveFile[0]	)
       return(0UL);
		       /* Initialize the system	information parameters	*/
		       /* structure					*/

memset(&mciSysInfoParms,	 0, sizeof(MCI_SYSINFO_PARMS));
memset(&mciSysInfoDefaultDevice, 0, sizeof(MCI_SYSINFO_DEFAULTDEVICE));

mciSysInfoParms.ulItem	     = MCI_SYSINFO_QUERY_DEFAULT;
mciSysInfoParms.pSysInfoParm = &mciSysInfoDefaultDevice;

mciSysInfoDefaultDevice.usDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;

		       /* Get the system information for the wave	*/
		       /* device to allow it to	be properly opended	*/
		       /* with the desired wave	file			*/

if ( (ulError =	mciSendCommand(0, MCI_SYSINFO, MCI_WAIT	| MCI_SYSINFO_ITEM,
			       (PVOID)&mciSysInfoParms,	0UL)) == 0UL )
   {
		       /* Initialize the system	information parameters	*/
		       /* again, to get	more information about the	*/
		       /* wave device					*/
   memset(&mciSysInfoParms,	0, sizeof(MCI_SYSINFO_PARMS));
   memset(&mciSysInfoQueryName,	0, sizeof(MCI_SYSINFO_QUERY_NAME));

   mciSysInfoParms.ulItem	= MCI_SYSINFO_QUERY_NAMES;
   mciSysInfoParms.pSysInfoParm	= &mciSysInfoQueryName;

   memmove(&mciSysInfoQueryName.szInstallName,
	   &mciSysInfoDefaultDevice.szInstallName,
	   sizeof(mciSysInfoQueryName.szInstallName));

		       /* Get the system information for the wave	*/
		       /* device to allow it to	be properly opended	*/
		       /* with the desired wave	file			*/

   if (	(ulError = mciSendCommand(0, MCI_SYSINFO, MCI_WAIT | MCI_SYSINFO_ITEM,
				  (PVOID)&mciSysInfoParms, 0UL)) == 0UL	)
       {
		       /* Check	to see that the	wave file specified	*/
		       /* does actually	exist.	Also, find the true	*/
		       /* location of the file since a fully qualified	*/
		       /* filename is required.				*/

	   if (	!DosQueryPathInfo(pszWaveFile, FIL_STANDARD, (PBYTE)&fsts,
				  sizeof(FILESTATUS3)) )

		       /* Fully	qualified path give, use the filename	*/
		       /* as is						*/

	       strcpy(szWavePath, pszWaveFile);
	   else
		       /* Wave file probably in	the MMPM/2 SOUNDS	*/
		       /* directory, find the location where MMPM/2 is	*/
		       /* located by looking for the base environment	*/
		       /* variable					*/

	       if ( !DosScanEnv("MMBASE", &pszMMPMBase)	)
		   {

		       /* Have found the base location for the MMPM/2,	*/
		       /* parse	out the	path portion before trying to	*/
		       /* construct the	wave file path.	 First make	*/
		       /* sure that any	trailing semi-colons are	*/
		       /* removed.					*/

		   if (	szMMPMBase[n = strlen(strcpy(szMMPMBase, pszMMPMBase)) - 1] == ';' )
		       szMMPMBase[n--] = 0;

		       /* Next check to	see if a back-slash present at	*/
		       /* the end of the path and if not the case, add	*/
		       /* one						*/

		   if (	szMMPMBase[n] != '\\' )
		       strcat(szMMPMBase, "\\");

		       /* Complete the path and	search for the file	*/

		   if (	DosSearchPath(SEARCH_CUR_DIRECTORY | SEARCH_IGNORENETERRS,
				      strcat(szMMPMBase, "SOUNDS"), pszWaveFile, szWavePath, CCHMAXPATH) )

		       /* File not found, use the existing wave	file	*/
		       /* name						*/

		       strcpy(szWavePath, pszWaveFile);
		   }
	       else
		       /* MMBASE environment variable not found, use	*/
		       /* the given wave file name			*/

		   strcpy(szWavePath, pszWaveFile);

		       /* Initialize the open parameters structure	*/

       memset(&mciOpenParms, 0,	sizeof(MCI_OPEN_PARMS));
       mciOpenParms.hwndCallback = (HWND)NULL;
       mciOpenParms.pszElementName = szWavePath;
       mciOpenParms.pszDeviceType = (PSZ)MAKEULONG(MCI_DEVTYPE_WAVEFORM_AUDIO,
						   mciSysInfoQueryName.usDeviceOrd);

		       /* Open the wave	device				*/

       if ( (ulError = mciSendCommand(0, MCI_OPEN,
				      MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE | MCI_OPEN_ELEMENT,
				      (PVOID)&mciOpenParms, 0UL)) == 0UL )

		       /* Wave file successfully loaded, return	the	*/
		       /* device ID as the handle for the wave file	*/

	       return((ULONG)mciOpenParms.usDeviceID);
       else
		       /* Wave device could not	be opened, display	*/
		       /* returned error				*/

	   ShowMCIError(hWnd, ulError);
       }
   else
		       /* Error	in retrieving information for the wave	*/
		       /* device, display returned error		*/

       ShowMCIError(hWnd, ulError);
   }
else
		       /* Error	in retrieving information to determine	*/
		       /* the name of the wave device, display		*/
		       /* returned error				*/

   ShowMCIError(hWnd, ulError);

		       /* Return null handle indicating	that the wave	*/
		       /* file was not successfully loaded		*/
return(0UL);
}
#pragma	subtitle("   MMPM/2 Support - Wave File Unload Function")
#pragma	page ( )

/* --- UnloadWaveFile ---------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	unload a loaded	wave file and to	*/
/*     release the associated handle.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND  hWnd;   = Window Handle					*/
/*     ULONG hSound; = Sound File Handle				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID EXPENTRY UnloadWave(HWND hWnd, ULONG hSound)

{
MCI_GENERIC_PARMS mciGenericParms; /* Generic Parameters Structure	*/
ULONG		  ulError;	   /* Error Value			*/

		       /* Check	to make	sure that a valid handle has	*/
		       /* passed.  A null handle indicates that	the	*/
		       /* wave file was	not loaded.			*/
if ( hSound )
   {
		       /* Initialize the generic parameters		*/

   mciGenericParms.hwndCallback	= (HWND)NULL;

		       /* Close	the wave file and the wave device	*/

   if (	(ulError = mciSendCommand((USHORT)hSound, MCI_CLOSE, MCI_WAIT,
				   (PVOID)&mciGenericParms, 0UL)) != 0UL )

		       /* Error	in closing, display the	error value	*/

       ShowMCIError(hWnd, ulError);
   }
}
#pragma	subtitle("   MMPM/2 Support - Ansync Sound Play Procedure")
#pragma	page ( )

/* --- PlayWave	---------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	load and play a	wave file submitted.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     ULONG hSound; = Sound File Handle				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID EXPENTRY PlayWave(ULONG hSound)

{
TID tid;			   /* Thread ID				*/

		       /* Check	to make	sure that a valid handle has	*/
		       /* passed.  A null handle indicates that	the	*/
		       /* wave file was	not loaded.			*/
if ( hSound )
		       /* Start	the play thread	and pass the sound	*/
		       /* handle as the	parameter block	to the thread	*/

   DosCreateThread(&tid, (PFNTHREAD)PlayWaveThread,
		   hSound, STACK_COMMITTED, 16384UL);
}
