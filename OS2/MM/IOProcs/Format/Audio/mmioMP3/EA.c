#define INCL_WIN
#define INCL_DOSERRORS
#include <os2.h>
#include <os2me.h>

BOOL cwSaveStringToEA(void * wpFSObject,  PSZ pKey, PSZ pValue)
{
  LONG rc;                             /* Ret code                   */
  PVOID       fealist;                 /* fealist buffer             */
  EAOP2       eaop;                    /* eaop structure             */
  PFEA2       pfea;                    /* pfea structure             */
  HFILE       handle;                  /* file handle                */
  ULONG       act;                     /* open action                */
  ULONG ulSize;
  USHORT    usSizeEA;

  char chrFileName[CCHMAXPATH];
  
  if((fealist=_wpAllocMem(wpFSObject, strlen(pKey)+strlen(pValue)+sizeof(FEA2)+0x100 /* Add some space */,
                          &ulSize))==NULLHANDLE)
    return FALSE;

  ulSize=sizeof(chrFileName);
  _wpQueryRealName(wpFSObject, chrFileName,&ulSize,TRUE);
  if ((rc = DosOpen(chrFileName, &handle, &act,
      0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
      OPEN_ACCESS_READWRITE + OPEN_SHARE_DENYWRITE +
      OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_WRITE_THROUGH, NULL))!=NO_ERROR) {
    _wpFreeMem(wpFSObject,fealist);     /* Free the memory            */
    return FALSE;
  }


  eaop.fpFEA2List = (PFEA2LIST)fealist;/* Set memory for the FEA     */
  eaop.fpGEA2List = NULL;              /* GEA is unused              */
  pfea = &eaop.fpFEA2List->list[0];    /* point to first FEA         */
  pfea->fEA = '\0';                    /* set the flags              */
                                       /* Size of FEA name field     */
  pfea->cbName = (BYTE)strlen(pKey);
                                       /* Size of Value for this one */
  pfea->cbValue = (SHORT)strlen(pValue)+4;
                                       /* Set the name of this FEA   */
  strcpy((PSZ)pfea->szName, pKey);
                                       /* Set the EA value           */
  usSizeEA=0xFFFD;
  memcpy((PSZ)pfea->szName+(pfea->cbName+1), &usSizeEA,
      2);/* It's an ASCII value */
  usSizeEA=strlen(pValue);
  memcpy((PSZ)pfea->szName+(pfea->cbName+3), &usSizeEA,
      sizeof(USHORT));/* It's an ASCII value */

  memcpy((PSZ)pfea->szName+(pfea->cbName+5), pValue,
      usSizeEA);
  pfea->oNextEntryOffset = 0;          /* no next entry              */
  eaop.fpFEA2List->cbList =            /* Set the total size var     */
      sizeof(ULONG) + sizeof(FEA2) + pfea->cbName + pfea->cbValue;

                                       /* set the file info          */
  rc = DosSetFileInfo(handle, 2, (PSZ)&eaop, sizeof(EAOP2));
  DosClose(handle);                    /* Close the File             */
  _wpFreeMem(wpFSObject,fealist);                 /* Free the memory            */
  return TRUE;
}


BOOL cwGetStringFromEA(void* wpFSObject,  PSZ pKey, char * chrBuffer, ULONG 
ulBuffSize) {
  LONG rc;                             /* Ret code                   */
  UCHAR       geabuff[300];            /* buffer for GEA             */
  PVOID       fealist;                 /* fealist buffer             */
  EAOP2       eaop;                    /* eaop structure             */
  PGEA2       pgea;                    /* pgea structure             */
  PFEA2       pfea;                    /* pfea structure             */
  HFILE       handle;                  /* file handle                */
  ULONG       act;                     /* open action                */
  ULONG ulSize;
  USHORT     *ptrUs;
  USHORT us;
  char chrFileName[CCHMAXPATH];

  if((fealist=_wpAllocMem(wpFSObject, 0x00010000L /* Add some space */,
                          &ulSize))==NULLHANDLE)
    return FALSE;

  ulSize=sizeof(chrFileName);
  _wpQueryRealName(wpFSObject, chrFileName,&ulSize,TRUE);

  if ((rc = DosOpen(chrFileName, &handle, &act,
      0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
      OPEN_ACCESS_READONLY + OPEN_SHARE_DENYREADWRITE +
      OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_WRITE_THROUGH,NULL))!=NO_ERROR) {
    _wpFreeMem(wpFSObject,fealist);    /* Free the memory            */
    return FALSE;
  }                                    /* get the file status info   */


                                       /* FEA and GEA lists          */
  eaop.fpGEA2List = (PGEA2LIST)geabuff;
  eaop.fpFEA2List = (PFEA2LIST)fealist;
  eaop.oError = 0;                     /* no error occurred yet      */
  pgea = &eaop.fpGEA2List->list[0];    /* point to first GEA         */
  eaop.fpGEA2List->cbList = sizeof(ULONG) + sizeof(GEA2) +
    strlen(pKey);
  eaop.fpFEA2List->cbList = (ULONG)0xffff;

                                       /* fill in the EA name length */
  pgea->cbName = (BYTE)strlen(pKey);
  strcpy(pgea->szName, pKey);          /* fill in the name           */
  pgea->oNextEntryOffset = 0;          /* fill in the next offset    */
                                       /* read the extended attribute*/
  rc = DosQueryFileInfo(handle, 3, (PSZ)&eaop, sizeof(EAOP2));
  DosClose(handle);                    /* close the file             */
  if (eaop.fpFEA2List->cbList <= sizeof(ULONG))
    rc = ERROR_EAS_NOT_SUPPORTED;      /* this is error also         */

  if (rc) {                            /* failure?                   */
    _wpFreeMem(wpFSObject,fealist);    /* Free the memory            */
    return FALSE;
  }

  pfea = &(eaop.fpFEA2List->list[0]);  /* point to the first FEA     */

#if 0
  ldp.shvb.shvname.strptr = ldp.varname;
  ldp.shvb.shvname.strlength = ldp.stemlen;
  ldp.shvb.shvnamelen = ldp.stemlen;
  ldp.shvb.shvvalue.strptr = ((PSZ)pfea->szName+(pfea->cbName+1));
  ldp.shvb.shvvalue.strlength = pfea->cbValue;
  ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
#endif

  ptrUs=(USHORT*)((PSZ)pfea->szName+(pfea->cbName+1));
  if(*ptrUs!=0xfffd) {
    /* Not an ASCII EA */
    _wpFreeMem(wpFSObject,fealist);    /* Free the memory            */
    return FALSE;
  }
  ptrUs++; /* Move to length */
  ulSize=(ULONG)*ptrUs;

  ptrUs++; /* Move to value */
  if(ulSize==0) {
    /* Not an ASCII EA */
    _wpFreeMem(wpFSObject,fealist);    /* Free the memory            */
    return FALSE;
  }
  //  HlpWriteToTrapLog("Length: %x, Value: %s\n", ulSize, (char*)ptrUs);
  memset(chrBuffer,0,ulBuffSize);
  memcpy(chrBuffer, (char*)ptrUs, (ulSize>=ulBuffSize ? ulBuffSize-1 : ulSize));
  _wpFreeMem(wpFSObject,fealist);    /* Free the memory            */
  return TRUE;
}
