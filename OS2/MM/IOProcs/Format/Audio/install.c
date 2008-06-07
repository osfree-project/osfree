/* This program (install.c) is Public Domain */

#define INCL_OS2MM
#include <os2.h>
#include <os2me.h>
#include <string.h>

//#include <minstall.h>
//If you don't have minstall.h, this is all that is needed
typedef struct _INSTMCISENDCOMMAND
   {
   USHORT wDeviceID;
   USHORT wMessage;
   ULONG  dwParam1;
   ULONG  dwParam2;
   USHORT wUserParm;
   } INSTMCISENDCOMMAND;

typedef INSTMCISENDCOMMAND *PINSTMCISENDCOMMAND;
#define IM_MCI_SEND_COMMAND         0x057C
#define IM_MCI_EXTENDED_SYSINFO     0x057E
#define IM_LOG_ERROR       0x0573

ULONG APIENTRY InstallExtension (HWND hwndOwnerHandle,
                                 PSZ pszSourcePath,
                                 PSZ pszTargetDrive,
                                 PSZ pszMyParams,
                                 HWND hwndMinstallHandle,
                                 PSZ pszResponseFile) {
    MCI_SYSINFO_EXTENSION exts;
    int i;

	{
		INSTMCISENDCOMMAND cmd;
        MCI_SYSINFO_PARMS SysInfo;
    
	    SysInfo.hwndDummyCallback = 0;
	    SysInfo.pszReturn = exts.szInstallName;
	    SysInfo.ulRetSize = MAX_DEVICE_NAME;
	    SysInfo.ulNumber = 0;
	    SysInfo.usDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
	    SysInfo.usReserved0 = 0;
	    SysInfo.ulItem = 0;
	    SysInfo.pSysInfoParm = 0;

		cmd.wDeviceID = 0;
    	cmd.wMessage = MCI_SYSINFO;             
    	cmd.dwParam1 = MCI_SYSINFO_INSTALLNAME;
     	cmd.dwParam2 = (ULONG)(&SysInfo);
     	cmd.wUserParm = 0;
     	
		WinSendMsg(hwndMinstallHandle, IM_MCI_SEND_COMMAND, 
			0,
			MPFROMP(&cmd));
	}

	exts.usNumExtensions = 0;
	WinSendMsg(hwndMinstallHandle, IM_MCI_EXTENDED_SYSINFO, 
		MPFROMLONG(MCI_SYSINFO_QUERY_EXTENSIONS),
		MPFROMP(&exts));
#ifdef DEBUG	
	WinSendMsg(hwndMinstallHandle, IM_LOG_ERROR, pszMyParams,0);
	WinSendMsg(hwndMinstallHandle, IM_LOG_ERROR, exts.szInstallName,0);
#endif
	if (exts.usNumExtensions >= MAX_EXTENSIONS) return 0;	
	for (i = 0; i < exts.usNumExtensions; i++) {
#ifdef DEBUG	
	WinSendMsg(hwndMinstallHandle, IM_LOG_ERROR, exts.szExtension[i],0);
#endif
 		if(stricmp(pszMyParams, exts.szExtension[i])==0) return 0;
    }
    strncpy(exts.szExtension[exts.usNumExtensions], pszMyParams, MAX_EXTENSION_NAME);
    exts.usNumExtensions++;
	WinSendMsg(hwndMinstallHandle, IM_MCI_EXTENDED_SYSINFO, 
		MPFROMLONG(MCI_SYSINFO_SET_EXTENSIONS),
		MPFROMP(&exts));
	return 0;
}
