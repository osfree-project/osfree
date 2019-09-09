/* This program (install.c) is Public Domain */

#define INCL_OS2MM
#define INCL_DOS
#include <os2.h>
#include <os2me.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

int deleteAssoc(char *ext) {
    MCI_SYSINFO_EXTENSION exts;
    MCI_SYSINFO_PARMS SysInfo;
    int i;
    
    SysInfo.hwndDummyCallback = 0;
    SysInfo.pszReturn = exts.szInstallName;
    SysInfo.ulRetSize = MAX_DEVICE_NAME;
    SysInfo.ulNumber = 0;
    SysInfo.usDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
    SysInfo.usReserved0 = 0;
    SysInfo.ulItem = 0;
    SysInfo.pSysInfoParm = 0;
	
    i = mciSendCommand(0, MCI_SYSINFO, 
		MCI_SYSINFO_INSTALLNAME | MCI_WAIT,
		(PVOID)(&SysInfo),0);
	if (i != 0) {
		return i;
    }

	exts.usNumExtensions = 0;
    SysInfo.pszReturn = 0;
    SysInfo.ulRetSize = 0;
    SysInfo.ulItem = MCI_SYSINFO_QUERY_EXTENSIONS;
    SysInfo.pSysInfoParm = &exts;
	
	i = mciSendCommand(0, MCI_SYSINFO, 
		MCI_SYSINFO_ITEM | MCI_WAIT,
		(PVOID)(&SysInfo),0);
	if (i != 0) {
		return i;
    }
#ifdef DEBUG
	printf("%s\n",ext);
	printf("%s\n",exts.szInstallName);
#endif
	
	{
    int found=FALSE;
	for (i = 0; i < exts.usNumExtensions; i++) {
#ifdef DEBUG
		printf("%s\n",exts.szExtension[i]);
#endif
 		if(stricmp(ext, exts.szExtension[i])==0) {
            int j;
	        printf("deleteing association %s with %s\n",exts.szExtension[i],exts.szInstallName);
	        found=TRUE;
            exts.usNumExtensions--;
            for (j=i; j < exts.usNumExtensions; j++) {
               strncpy(exts.szExtension[j],exts.szExtension[j+1],MAX_EXTENSION_NAME);
            } /* endfor */
        }
	}
	if (!found) {
        printf("No association with %s found for %s\n",exts.szInstallName,ext);
    }
    }
	
    SysInfo.ulItem = MCI_SYSINFO_SET_EXTENSIONS;
	i = mciSendCommand(0, MCI_SYSINFO, 
		MCI_SYSINFO_ITEM | MCI_WAIT,
		(PVOID)(&SysInfo),0);
		
	if (i != 0) {
		return i;
    }
    
    return 0;
}    

int deleteProc(char *Fourcc) {
    MMINIFILEINFO mmIniFileInfo;
    ULONG ulFlags = 0L;
    ULONG rc;

    memset( &mmIniFileInfo, '\0', sizeof(MMINIFILEINFO) );
    mmIniFileInfo.fccIOProc = mmioStringToFOURCC(Fourcc,0);
    ulFlags = MMIO_FINDPROC;
    rc = mmioIniFileHandler( &mmIniFileInfo,
                             ulFlags);
    if (rc) {
        printf("Couldn't find IOProc %s.  Code: %ld\n",Fourcc,rc);
        return rc;
    }

    ulFlags = MMIO_REMOVEPROC;
    rc = mmioIniFileHandler( &mmIniFileInfo,
                             ulFlags);
    if (rc) {
        printf("Couldn't remove IOProc %s.  Code: %ld\n",Fourcc,rc);
        return rc;
    } else {
        printf("Removed IOProc %s.\n",Fourcc);
    } /* endif */

    {
       char dllname[DLLNAME_SIZE] = "DLL\\";
       char fulldllname[DLLNAME_SIZE] = "";

       strncat(dllname, mmIniFileInfo.szDLLName,sizeof(dllname));
       strncat(dllname, ".DLL",sizeof(dllname));
       rc = DosSearchPath(2,"MMBASE",dllname,fulldllname,sizeof(fulldllname));
       if (rc) {
           printf("Couldn't find %s. code:%ld\n", dllname,rc);
           return rc;
       }

       rc = DosReplaceModule(fulldllname,NULL,NULL);
       if (rc) {
           printf("Couldn't unlock %s. code:%ld\n", fulldllname,rc);
       } else {
           printf("Unlocked %s.\n", fulldllname);
       } /* endif */

       rc = DosDelete(fulldllname);
       if (rc) {
           printf("Couldn't delete %s. code:%ld\n", fulldllname,rc);
           return rc;
       } else {
            printf("Deleted %s.\n", fulldllname);
       } /* endif */
    }

    return 0;
}

int main(int argc, char **argv) {
    if (3 != argc) {
       printf("Please run unInstall.cmd instead.");
       return 0;
    } /* endif */
    deleteAssoc(argv[2]);
    deleteProc(argv[1]);
    return 0;
}
