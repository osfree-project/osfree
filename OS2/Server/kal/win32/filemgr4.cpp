//filesystem/volume APIs
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

extern "C" {

APIRET os2APIENTRY DosQueryFSAttach(PCSZ   pszDeviceName,
                                    ULONG  /*ulOrdinal*/,
                                    ULONG  ulFSAInfoLevel,
                                    PFSQBUFFER2 pfsqb,
                                    PULONG /*pcbBuffLength*/)
{
        //FixMe: handle devices too
        if(ulFSAInfoLevel!=FSAIL_QUERYNAME)
                return 124; //invalid level
        UINT dt = GetDriveType(pszDeviceName);
        switch(dt) {
                case DRIVE_REMOVABLE:
                        pfsqb->iType = FSAT_LOCALDRV; break;
                case DRIVE_FIXED:
                        pfsqb->iType = FSAT_LOCALDRV; break;
                case DRIVE_REMOTE:
                        pfsqb->iType = FSAT_REMOTEDRV; break;
                case DRIVE_CDROM:
                        pfsqb->iType = FSAT_LOCALDRV; break;
                case DRIVE_RAMDISK:
                        pfsqb->iType = FSAT_LOCALDRV; break;
                default:
                        return 15; //invalid drive
        }

        char volumeNameBuffer[256];
        DWORD mcl;
        DWORD fsf;
        char fsNameBuffer[256];
        if(!GetVolumeInformation(pszDeviceName,
                                 volumeNameBuffer,
                                 256,
                                 0,
                                 &mcl,
                                 &fsf,
                                 fsNameBuffer,
                                 256
                                ))
                return (APIRET)GetLastError();

        pfsqb->cbName = (USHORT)strlen(fsNameBuffer);
        strcpy((char*)pfsqb->szName,fsNameBuffer);
        pfsqb->cbFSDName = (USHORT)strlen(fsNameBuffer);
        strcpy((char*)pfsqb->szName+pfsqb->cbName+1,fsNameBuffer);
        pfsqb->cbFSAData = 0;

        return 0;
}

APIRET os2APIENTRY DosQueryFSInfo(ULONG disknum,
                                  ULONG infolevel,
                                  PVOID pBuf,
                                  ULONG /*cbBuf*/)
{
        if(infolevel!=FSIL_ALLOC && infolevel!=FSIL_VOLSER)
                return 87; //invalid parameter

        char root[MAX_PATH];
        if(disknum==0) {
                GetCurrentDirectory(MAX_PATH,root);
                if(root[0]=='\\' || root[1]!=':')
                        return 15; //invalid drive
                root[3]='\0';
        } else {
                root[0]=(char)(disknum+'A'-1);
                root[1]=':';
                root[2]='\\';
                root[4]='\0';
        }
        if(infolevel==FSIL_ALLOC) {
                DWORD sectorsPerCluster;
                DWORD bytesPerSector;
                DWORD freeClusters;
                DWORD clusters;
                if(!GetDiskFreeSpace(root,&sectorsPerCluster,&bytesPerSector,&freeClusters,&clusters))
                        return (APIRET)GetLastError();

                FSALLOCATE *pfsa=(FSALLOCATE*)pBuf;
                pfsa->idFileSystem = 0;
                pfsa->cSectorUnit = (ULONG)sectorsPerCluster;
                pfsa->cUnit = (ULONG)clusters;
                pfsa->cUnitAvail = (ULONG)freeClusters;
                pfsa->cbSector = (USHORT)bytesPerSector;
        } else {
                char volumeNameBuffer[256];
                DWORD volumeSerialNumber, maxComponentLength,fsf;
                char fsName[256];
                if(!GetVolumeInformation(root,
                                         volumeNameBuffer,256,
                                         &volumeSerialNumber,
                                         &maxComponentLength,
                                         &fsf,
                                         fsName,256
                                        ))
                        return (APIRET)GetLastError();

                FSINFO *pfsinfo = (FSINFO*)pBuf;
                //the toolkit doc is wrong
                //pfsinfo->ulVSN = (ULONG)volumeSerialNumber;
                *((DWORD*)pfsinfo) = volumeSerialNumber;
                pfsinfo->vol.cch = (os2BYTE)strlen(volumeNameBuffer);
                strcpy(pfsinfo->vol.szVolLabel,volumeNameBuffer);
        }
        return 0;
        
}

APIRET os2APIENTRY DosSetFSInfo(ULONG disknum,
                                ULONG infolevel,
                                PVOID pBuf,
                                ULONG /*cbBuf*/)
{
        if(infolevel!=FSIL_VOLSER)
                return 87; //invalid parameter

        char root[MAX_PATH];
        if(disknum==0) {
                GetCurrentDirectory(MAX_PATH,root);
                if(root[0]=='\\' || root[1]!=':')
                        return 15; //invalid drive
                root[3]='\0';
        } else {
                root[0]=(char)(disknum+'A'-1);
                root[1]=':';
                root[2]='\\';
                root[4]='\0';
        }

        //FixMe: volume serial number is NOT set
        FSINFO *pfsinfo = (FSINFO*)pBuf;
        if(!SetVolumeLabel(root,pfsinfo->vol.szVolLabel))
                return (APIRET)GetLastError();
        return 0;
}

};

