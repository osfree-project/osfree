//旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//� XTRCDSK - Setup, setup/delete input file data areas
//� Author:  Alan B. Arnett
//� Copyright 1997, 1998 by Alan B. Arnett
//� Last modified: 4 Jan 98
//� 24 May 98 - add crc checking to dsk file setup
//쳐컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//�
//� This file is part of the source code to dskxtrct and is distributed under
//� the terms of the GNU General Public Licence.
//�
//읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴


#ifndef MAXSTRLEN
#define MAXSTRLEN 2048
#endif

#ifndef STDSECTORLEN
#define STDSECTORLEN 512
#endif

#ifndef MAXSECTORS
#define MAXSECTORS 2880*2   // max allowed for 2.88 floppy
#endif

#ifndef STDDRIENTRYLEN
#define STDDIRENTRYLEN 32
#endif

#ifndef CLUSTERHIGH
#define CLUSTERHIGH 4088    // max data cluster number on floppy
#endif

#define INCL_DOSFILEMGR                        /* File Manager values */
#define INCL_DOSERRORS                 /* DOS error values    */
#include <os2.h>

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\utime.h>
#include <sys\types.h>
#include <time.h>
#include <direct.h>
#include <string.h>

// #include "pringmsg.c"

#define Dsk_no_more_files FALSE
#define Dsk_more_files TRUE
#define DskDirFileEmpty '\x00'
#define DskDirFileErased '\xE5'
#define DskDirFileAltE5 '\x05'
#define DskDirFileDir '.'
#define DskDirAttrVLabel '\x08'
#define DskDirAttrSubDir '\x10'


enum
{
       DskAddrTypeUndefined, DskAddrTypeCluster
};

struct DskDirEntryLayout
{
       char name[8];
       char ext[3];
       char attrib;
       char reserved[10];
       char time[2];
       char date[2];
       char start[2];
       char size[4];
};

struct DskCtl
{
       char *pBuf;
       int BufSize;                            // size of header + disk sectors
       int DiskStartOffset;            // start of disk sectors
       int DiskSize;                           // total size of disk sectors
       int SectorSize;                         // length of each sector
       int NumberRootDirEntries;       // number dir entries in root directory
       int SectorsPerCluster;          // sectors in each cluster
       int ClusterSize;                        // length of each cluster
       int NumberSubDirEntries;        // number dir entries in a subdir cluster
       unsigned int CrcField;          // CRC field
       char *pDisk;                            // mem addr of start of diskette image
       char *pFat;                                     // mem addr of start of fat
       char *pDir;                                     // mem addr of start of root directory
       char *pFiles;                           // mem addr of start of file data area
       char *pDiskEnd;                         // mem addr of end of disk area
       char *pClusterEnd;                      // mem addr of start of last cluster
       BOOL FAT12;
};

struct DskDirCtl
{
       DskDirEntryLayout *pPos;
       char Count;
       char *pName;
       DskDirCtl *pPrevious;
       int Cluster;
};

struct DskFileCtl
{
       char *pDir;
       char *pName;
       char *pFullName;
       int Size;
       int Date;
       int Time;
       int Attributes;
       char *pData;
       int DataSize;
       int DataLeftToRead;
       int Cluster;
       DskCtl *pDskCB;
       DskDirCtl *pDirCB;
};

unsigned int mtoi(char *word, int bytes)
{
       unsigned int x;

       if (bytes < 1 || bytes > 4)
               return 0;
       x = word[bytes - 1];
       for (int i = bytes - 2; i >= 0; i--)
               x = x * 256 + word[i];
       return x;
}

struct utimbuf *DskBuildDateTime(utimbuf *ubuf, DskFileCtl *pFileCB)
{
       struct tm tbuf;

       tbuf.tm_sec = (pFileCB->Time & 0x1F) * 2;
       tbuf.tm_min = (pFileCB->Time >> 5) & 0x3F;
       tbuf.tm_hour = (pFileCB->Time >> 11) & 0x1F;

       tbuf.tm_mday = (pFileCB->Date & 0x1F);
       tbuf.tm_mon = ((pFileCB->Date >> 5) & 0x0F) - 1;
       tbuf.tm_year = (pFileCB->Date >> 9) + 80;

       tbuf.tm_wday = 0;
       tbuf.tm_yday = 0;
       tbuf.tm_isdst = 0;
       if (-1 == (ubuf->modtime = mktime(&tbuf)))
               ubuf->modtime = 0;
       return ubuf;
}

char *DskBuildName(char *ndir, char *sdir, char *sname)
{
       strcpy(ndir, sdir);
       if (strlen(sdir) != 0 && strlen(sname) != 0)
               strcat(ndir, "\\");
       strcat(ndir, sname);
       return ndir;
}

char *DskBuildFileName(char *nname, char *sname, char *sext)
{
       int i, slen;

       if (nname == NULL || sname == NULL || sext == NULL)
       {
               printf("DskBuildFileName error, an input parameter is NULL");
               exit(EXIT_FAILURE);
       }
       for (i = 7; i >= 0 && sname[i] == ' '; i--);
       strncpy(nname, sname, i + 1);
       slen = i + 1;
       for (i = 2; i >= 0 && sext[i] == ' '; i--);
       if (i >= 0)
       {
               nname[slen++] = '.';
               strncpy(nname + slen, sext, i + 1);
               slen += i + 1;
       }
       nname[slen] = '\0';
       return nname;
}

int DskBufOffset(DskCtl * pDskCB, char *BufAddr)
{
       char xMsg[MAXSTRLEN];

       if (BufAddr < pDskCB->pBuf)
       {
               sprintf(xMsg, "DskBufOffset, invalid buffer address, below buffer: %x", BufAddr);
               PrintMsg(MsgFail, xMsg);
       }
       if (BufAddr > pDskCB->pDiskEnd)
       {
               sprintf(xMsg, "DskBufOffset, invalid buffer address, past buffer: %x", BufAddr);
               PrintMsg(MsgFail, xMsg);
       }
       return (BufAddr - pDskCB->pBuf);
}

void DskCheckDiskAddrErr(char *Msg, char *Addr1, char *Addr2)
{
       char xMsg[MAXSTRLEN];

       sprintf(xMsg, Msg, Addr1, Addr2);
       PrintMsg(MsgFail, xMsg);
}

char *DskCheckDiskAddr(DskCtl * pDskCB, char *pTest, int AddressType)
{
       char *pLimit;

       if (AddressType == DskAddrTypeCluster)
       {
               if (pTest < pDskCB->pFiles)
                       DskCheckDiskAddrErr("Cluster address low: %X, test: %X",
                                                               pTest, pDskCB->pFiles);
       }
       else if (pTest < pDskCB->pDisk)
               DskCheckDiskAddrErr("Memory address low: %X, test: %X",
                                                       pTest, pDskCB->pDisk);

       if (AddressType == DskAddrTypeCluster)
       {
               if (pTest > pDskCB->pClusterEnd)
                       DskCheckDiskAddrErr("Cluster address high: %X, test: %X",
                                                               pTest, pDskCB->pClusterEnd);
       }
       else if (pTest > pDskCB->pDiskEnd)
               DskCheckDiskAddrErr("Memory address high: %X, test: %X",
                                                       pTest, pDskCB->pDiskEnd);

       return (pTest);
}

char *DskAddrOfCluster(DskCtl * pDskCB, int Cluster)
{
       char *Taddr;

       if (Cluster < CLUSTERHIGH)
       {
               Taddr = (Cluster - 2) * pDskCB->ClusterSize + pDskCB->pFiles;
               DskCheckDiskAddr(pDskCB, Taddr, DskAddrTypeCluster);
       }
       else
               Taddr = NULL;
       return (Taddr);
}

int DskNextCluster(DskCtl * pDskCB, int Cluster)
{
       char xMsg[MAXSTRLEN];
       unsigned int wcl1, wcl2, ClNext;

       wcl2 = Cluster >> 1;            // get 1/2
       wcl1 = Cluster + wcl2;          // get cluster slot
       ClNext = mtoi(pDskCB->pFat + wcl1, 2);  // get the cluster data
       if ((wcl2 << 1) == Cluster)     // was cluster number even (vs odd)
               ClNext &= 4095;
       else
               ClNext >>= 4;
       sprintf(xMsg, "DskNextCluster, current: '%x', next: '%x'", Cluster, ClNext);
       PrintMsg(MsgDebug2, xMsg);
       return (ClNext);
}

void DskReadNext(DskFileCtl * pFileCB)
{
       DskCtl *pDskCB = pFileCB->pDskCB;

       pFileCB->Cluster = DskNextCluster(pDskCB, pFileCB->Cluster);
       pFileCB->pData = DskAddrOfCluster(pDskCB, pFileCB->Cluster);
       if (pFileCB->pData == NULL)
       {
               pFileCB->DataSize = 0;
               pFileCB->DataLeftToRead = 0;
       }
       else if (pFileCB->DataLeftToRead > pFileCB->DataSize)
               pFileCB->DataLeftToRead -= pFileCB->DataSize;
       else
       {
               pFileCB->DataSize = pFileCB->DataLeftToRead;
               pFileCB->DataLeftToRead = 0;
       }
}

void DskFillFileCtl(DskFileCtl * pFileCB)
{
       DskCtl *pDskCB = pFileCB->pDskCB;
       DskDirCtl *pDirCB = pFileCB->pDirCB;
       DskDirEntryLayout *pDirEntry = pDirCB->pPos;

       pFileCB->pDir = pDirCB->pName;
       DskBuildFileName(pFileCB->pName, pDirEntry->name, pDirEntry->ext);
       DskBuildName(pFileCB->pFullName, pFileCB->pDir, pFileCB->pName);
       pFileCB->Size = mtoi(pDirEntry->size, 4);
       pFileCB->Date = mtoi(pDirEntry->date, 2);
       pFileCB->Time = mtoi(pDirEntry->time, 2);
       pFileCB->Attributes = pDirEntry->attrib;
       pFileCB->Cluster = mtoi(pDirEntry->start, 2);
       pFileCB->pData = DskAddrOfCluster(pDskCB, pFileCB->Cluster);
       pFileCB->DataSize = pDskCB->ClusterSize;
       if (pFileCB->Size > pFileCB->DataSize)
               pFileCB->DataLeftToRead = pFileCB->Size - pFileCB->DataSize;
       else
       {
               pFileCB->DataSize = pFileCB->Size;
               pFileCB->DataLeftToRead = 0;
       }
}

DskDirCtl *DskDropDirCtl(DskDirCtl * pDirCB)
{
       DskDirCtl *pOldCB = pDirCB->pPrevious;

       free(pDirCB->pName);
       free(pDirCB);
       return (pOldCB);
}

DskDirCtl *DskSetupDirCtl(DskDirCtl * pDirCB, DskCtl * pDskCB, DskFileCtl * pFileCB)
{
       char work[MAXSTRLEN], xMsg[MAXSTRLEN];
       DskDirCtl *pNewCB = (DskDirCtl *) malloc(sizeof(DskDirCtl));

       memset(pNewCB, '\0', sizeof(DskDirCtl));
       if (pDirCB == NULL)
       {
               pNewCB->pPos = (DskDirEntryLayout *) pDskCB->pDir;
               pNewCB->Count = pDskCB->NumberRootDirEntries;
               pNewCB->pName = strdup("");
               pNewCB->Cluster = 0;
       }
       else
       {
               pNewCB->pPos = (DskDirEntryLayout *) pFileCB->pData;
               pNewCB->Count = pDskCB->NumberSubDirEntries;
               pNewCB->pName = strdup(pFileCB->pFullName);
               pNewCB->Cluster = pFileCB->Cluster;
       }
       pNewCB->pPrevious = pDirCB;
       sprintf(xMsg, "Starting dir located at: '%x'", pNewCB->pPos);
       PrintMsg(MsgDebug2, xMsg);
       return (pNewCB);
}

BOOL DskPositionNext(DskFileCtl * pFileCB)
{
       int rc;
       DskDirCtl *pDirCB = pFileCB->pDirCB;
       DskCtl *pDskCB = pFileCB->pDskCB;

       rc = Dsk_no_more_files;
       while (rc != Dsk_more_files)
       {
               if (pDirCB->Count < 1 || pDirCB->pPos->name[0] == DskDirFileEmpty)
               {
                       if (pDirCB->pPrevious == NULL)
                               return (rc);
                       if ((pDirCB->Cluster = DskNextCluster(pDskCB, pDirCB->Cluster)) < CLUSTERHIGH)
                       {
                               pDirCB->pPos = (DskDirEntryLayout *) DskAddrOfCluster(pDskCB, pDirCB->Cluster);
                               pDirCB->Count = pDskCB->NumberSubDirEntries;
                       }
                       else
                       {
                               pDirCB = DskDropDirCtl(pDirCB);
                               pDirCB->Count--;
                               pDirCB->pPos++;
                               pFileCB->pDirCB = pDirCB;
                       }
               }
               else
               {
                       /* skip over unusable dir entries  */
                       if ((pDirCB->pPos->name[0] == DskDirFileErased
                                || pDirCB->pPos->name[0] == DskDirFileDir)
                               || pDirCB->pPos->attrib & DskDirAttrVLabel)
                       {
                               pDirCB->Count--;
                               pDirCB->pPos++;
                       }
                       else
                               rc = Dsk_more_files;
               }
       }
       DskFillFileCtl(pFileCB);
       return (rc);
}

BOOL DskFileNext(DskFileCtl * pFileCB)
{
       BOOL rc;
       DskDirCtl *pDirCB = pFileCB->pDirCB;
       DskCtl *pDskCB = pFileCB->pDskCB;

       if (pFileCB->Attributes & DskDirAttrSubDir)
               pFileCB->pDirCB = DskSetupDirCtl(pDirCB, pDskCB, pFileCB);
       else
       {
               pDirCB->Count--;
               pDirCB->pPos++;
       }
       rc = DskPositionNext(pFileCB);
       return (rc);
}

BOOL DskFileFirst(DskCtl * pDskCB, DskFileCtl * *ppFileCB)
{
       BOOL rc;
       DskDirCtl *pDirCB = DskSetupDirCtl(NULL, pDskCB, NULL);
       DskFileCtl *pFileCB = (DskFileCtl *) malloc(sizeof(DskFileCtl));

       *ppFileCB = pFileCB;
       memset(pFileCB, '\0', sizeof(DskFileCtl));
       pFileCB->pDskCB = pDskCB;
       pFileCB->pDirCB = pDirCB;
       pFileCB->pName = (char *) malloc(MAXSTRLEN);
       pFileCB->pFullName = (char *) malloc(MAXSTRLEN);
       rc = DskPositionNext(pFileCB);
       return (rc);
}

int DskCrcCheckFile(DskCtl * pDskCB)
{
       /* check CRC of dsk file                              */
       /* return TRUE if CRC is correct or if CRC-ignore selected */
       /* else return FALSE                         */
       char *DataBytes;
       unsigned int CrcValue = 0;
       char xMsg[MAXSTRLEN];

       for (DataBytes = pDskCB->pDisk; DataBytes < pDskCB->pDiskEnd; DataBytes += 2)
               CrcValue += mtoi(DataBytes, 2);
       if ((CrcValue != pDskCB->CrcField) && Ctl.HasLeader)
       {
               Ctl.CrcCount++;
               sprintf(xMsg, "CRC error, expected: %x, found: %x", pDskCB->CrcField, CrcValue);
               if (!Ctl.CrcIgnore)
               {
                       PrintMsg(MsgError, xMsg);
                       sprintf(xMsg, "Bypassing processing of this file");
               }
               else
               {
                       PrintMsg(MsgWarning, xMsg);
                       sprintf(xMsg, "CRC-ignore specified, continuing to process this file");
               }
               PrintMsg(MsgMust, xMsg);
       }
       else
       {
               sprintf(xMsg, "Calculated CRC value: %x", CrcValue);
               PrintMsg(MsgDebug, xMsg);
       }
       return (Ctl.CrcIgnore || (CrcValue == pDskCB->CrcField) || !Ctl.HasLeader);
}

BOOL DskSetControls(DskCtl * pDskCB)
{
       const int HeaderLen = 40;
       char xMsg[MAXSTRLEN];

       if (strncmp(pDskCB->pBuf +3,"IBM 10.2",sizeof("IBM 10.2"))==0)
       {
               Ctl.HasLeader = FALSE;
               pDskCB->DiskStartOffset = 0;
       }
       else
       {
               Ctl.HasLeader = TRUE;
               /* set up disk area data */
               pDskCB->DiskStartOffset = mtoi(pDskCB->pBuf + 38, 2);
       }
       sprintf(xMsg, "Offset to start of disk data: %x", pDskCB->DiskStartOffset);
       PrintMsg(MsgDebug, xMsg);
       pDskCB->pDisk = pDskCB->pBuf + pDskCB->DiskStartOffset;
       pDskCB->pDiskEnd = pDskCB->pBuf + pDskCB->BufSize;
       pDskCB->DiskSize = pDskCB->pDiskEnd - pDskCB->pDisk;
       sprintf(xMsg, "Disk size: %d", pDskCB->DiskSize);
       PrintMsg(MsgDebug, xMsg);
       sprintf(xMsg, "Disk end offset in buffer: %x", DskBufOffset(pDskCB, pDskCB->pDiskEnd));
       PrintMsg(MsgDebug, xMsg);
       if (Ctl.HasLeader)
       {
               pDskCB->CrcField = mtoi(pDskCB->pBuf + 20, 4);
               sprintf(xMsg, "CRC value: %x", pDskCB->CrcField);
               PrintMsg(MsgDebug, xMsg);
       }

       /* grab diskette control information */
       pDskCB->SectorSize = mtoi(pDskCB->pDisk + 11, 2);
       sprintf(xMsg, "Sectors size: %d", pDskCB->SectorSize);
       PrintMsg(MsgDebug, xMsg);
       pDskCB->SectorsPerCluster = pDskCB->pDisk[13];
       sprintf(xMsg, "Sectors in each Cluster: %d", pDskCB->SectorsPerCluster);
       PrintMsg(MsgDebug, xMsg);
       int ReservedSectors = mtoi(pDskCB->pDisk + 14, 2);

       sprintf(xMsg, "Reserved sectors: %d", ReservedSectors);
       PrintMsg(MsgDebug, xMsg);
       if (ReservedSectors > MAXSECTORS)
               PrintMsg(MsgError, "Reserved sectors exceeds maximum allowed");
       int NumberFATs = pDskCB->pDisk[16];

       sprintf(xMsg, "Number of FATs: %d", NumberFATs);
       PrintMsg(MsgDebug, xMsg);
       pDskCB->NumberRootDirEntries = mtoi(pDskCB->pDisk + 17, 2);
       sprintf(xMsg, "Number of root directory entries: %d", pDskCB->NumberRootDirEntries);
       PrintMsg(MsgDebug, xMsg);
       int SectorsFAT = mtoi(pDskCB->pDisk + 22, 2);

       sprintf(xMsg, "Sectors in each FAT: %d", SectorsFAT);
       PrintMsg(MsgDebug, xMsg);

       /* calculate various values */
       pDskCB->ClusterSize = pDskCB->SectorSize * pDskCB->SectorsPerCluster;
       sprintf(xMsg, "Cluster size: %d", pDskCB->ClusterSize);
       PrintMsg(MsgDebug, xMsg);
       pDskCB->FAT12 = TRUE;
       PrintMsg(MsgDebug, "FAT processing forced to use 12 bit FAT");
       pDskCB->NumberSubDirEntries = pDskCB->ClusterSize / sizeof(DskDirEntryLayout);

       /* calculate various pointers */
       pDskCB->pFat = pDskCB->pDisk + ReservedSectors * pDskCB->SectorSize;
       pDskCB->pDir = pDskCB->pFat + NumberFATs * SectorsFAT * pDskCB->SectorSize;
       pDskCB->pFiles = pDskCB->pDir + pDskCB->NumberRootDirEntries * STDDIRENTRYLEN;
       pDskCB->pClusterEnd = pDskCB->pDiskEnd - pDskCB->ClusterSize;
       sprintf(xMsg, "FAT offset in buffer: %x", DskBufOffset(pDskCB, pDskCB->pFat));
       PrintMsg(MsgDebug, xMsg);
       sprintf(xMsg, "Directory offset in buffer: %x", DskBufOffset(pDskCB, pDskCB->pDir));
       PrintMsg(MsgDebug, xMsg);
       sprintf(xMsg, "Files offset in buffer: %x", DskBufOffset(pDskCB, pDskCB->pFiles));
       PrintMsg(MsgDebug, xMsg);
       sprintf(xMsg, "Last valid cluster offset in buffer: %x",
                       DskBufOffset(pDskCB, pDskCB->pClusterEnd));
       PrintMsg(MsgDebug, xMsg);

       if (Ctl.HasLeader)
       {
               /* validate disk size calculated from header with what was read */
               /* ************  do something about diskend. compare sizes?? ********** */
               int rsectors = mtoi(pDskCB->pBuf + 34, 2);

               sprintf(xMsg, "Sector count stored in header: %d", rsectors);
               PrintMsg(MsgDebug, xMsg);
               if (rsectors > MAXSECTORS)
                        PrintMsg(MsgFail, "Number of sectors is out of range, probable disk format error");
               int CalcDiskSize = rsectors * STDSECTORLEN;

               sprintf(xMsg, "Calculated Disk size: %d", CalcDiskSize);
               char *CalcDiskEnd = pDskCB->pDisk + CalcDiskSize;

               if (CalcDiskEnd != pDskCB->pDiskEnd)
               {
                     sprintf(xMsg, "Actual and calculated disk size are different, actual: %d, calculated: %d",
                                    pDskCB->pDiskEnd - pDskCB->pDisk, CalcDiskEnd);
                     PrintMsg(MsgWarning, xMsg);
               }
       }
       return (TRUE);
}

BOOL DskLoadFile(char *SrcFile, DskCtl * pDskCB)
{
       struct stat StatBuf;
       char xMsg[MAXSTRLEN];
       int rhandle, rbytes, KeepOn;

       if (!(KeepOn = (rhandle = open(SrcFile, O_RDONLY | O_BINARY)) >= 0))
       {
               sprintf(xMsg, "Error opening input file: %s", SrcFile);
               PrintMsg(MsgError, xMsg);
       }
       if (KeepOn)
               if (!(KeepOn = fstat(rhandle, &StatBuf) == 0))
               {
                       sprintf(xMsg, "Error getting input file size: %s", SrcFile);
                       PrintMsg(MsgError, xMsg);
               }
       if (KeepOn)
       {
               pDskCB->BufSize = StatBuf.st_size;
               sprintf(xMsg, "Buffer size: %x", pDskCB->BufSize);
               PrintMsg(MsgDebug, xMsg);
               if (!(KeepOn = (pDskCB->pBuf = (char *) malloc(pDskCB->BufSize)) != 0))
               {
                       sprintf(xMsg, "Unable to allocate DSK image buffer: %s", SrcFile);
                       PrintMsg(MsgError, xMsg);
               }
       }
       if (KeepOn)
               if (!(KeepOn = (rbytes = read(rhandle, pDskCB->pBuf, pDskCB->BufSize)) >= 0))
               {
                       sprintf(xMsg, "Error reading input file: %s", SrcFile);
                       PrintMsg(MsgError, xMsg);
               }
       if (KeepOn)
               if (!(KeepOn = rbytes == pDskCB->BufSize))
               {
                       sprintf(xMsg, "Incorrect read, expected %d bytes, read %d", pDskCB->BufSize, rbytes);
                       PrintMsg(MsgError, xMsg);
               }
       if (rhandle > 0)
               if (close(rhandle) != 0)
               {
                       sprintf(xMsg, "Error closing input file: %s", SrcFile);
                       PrintMsg(MsgError, xMsg);
                       KeepOn = FALSE;
               }
       return (KeepOn);
}

BOOL DskFindFirst(char *SrcFile, DskFileCtl * *ppFileCB)
{
       /* initialize DskFileCtl and call routines to setup other data */
       /* if any errors, return same flag as if no files found        */
       /* return TRUE if files to process, FALSE if not to process    */
       char xMsg[MAXSTRLEN];
       BOOL MoreFiles;
       DskFileCtl *pFileCB = 0;
       DskCtl *pDskCB = (DskCtl *) malloc(sizeof(DskCtl));

       memset(pDskCB, '\0', sizeof(DskCtl));
       MoreFiles = DskLoadFile(SrcFile, pDskCB);
       if (MoreFiles)
               MoreFiles = DskSetControls(pDskCB);
       if (MoreFiles)
               MoreFiles = DskCrcCheckFile(pDskCB);
       if (MoreFiles)
               MoreFiles = DskFileFirst(pDskCB, &pFileCB);
       if (MoreFiles)
               *ppFileCB = pFileCB;
       else if (pFileCB == 0 && pDskCB != 0)
               free(pDskCB);
       return MoreFiles;
}


void DskClose(DskFileCtl * FileCB)
{
       /* free all data/buffers allocated in DskFileCtl  */
       if (FileCB != 0)
       {
               if (FileCB->pDskCB != 0)
               {
                       if (FileCB->pDskCB->pBuf != 0)
                               free(FileCB->pDskCB->pBuf);
                       free(FileCB->pDskCB);
               }
               if (FileCB->pName != 0)
                       free(FileCB->pName);
               if (FileCB->pFullName != 0)
                       free(FileCB->pFullName);
               free(FileCB);
       }
}
