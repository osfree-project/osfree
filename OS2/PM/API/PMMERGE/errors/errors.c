/*
WinGetLastError
WinGetLastError2
 This function returns the error code set by the failure of a Presentation Manager function.

 In multiple thread applications where there are multiple anchor blocks, errors are stored in the anchor
 block created by the WinInitialize function of the thread invoking a call. The last error for the process and
 thread on which this function call is made will be returned.

 Returns
 -1 = hab not exist or not used, or
  the last nonzero error code, and sets the error code to zero.
*/
ERRORID     APIENTRY    WinGetLastError(HAB iHAB)
{   int rc;
    rc = _hab.QueryHABexist(iHAB);
    if(rc != 1)
       return -1;
    rc = _hab.hab[iHAB].lastError;
    _hab.hab[iHAB].lastError = 0;
    return rc;

}
#if 0
ERRORID     APIENTRY    WinGetLastError2(void)
{   int ordinal,tid,rc, ind;
    ordinal = QueryThreadOrdinal(tid);
    ind = _hab.QueryOrdinalUsed(ordinal,Q_ORDINAL_INDEX);
    if(ind == -1)
       return -1;

    rc = _hab.hab[ind].lastError;
    _hab.hab[ind].lastError = 0;
    return rc;
}
#endif

/*
 This function allocates a single private segment to contain the ERRINFO structure. All the pointers to string
 fields within the ERRINFO structure are offsets to memory within that segment.

 The memory allocated by this function is not released until the returned pointer is passed to the
 WinFreeErrorInfo function.

 Returns:

    NULL     No error information available
    Other    pointer to structure contains information about the previous error code for the current thread:

   { ERRINFO,       sizeof(ERRINFO)
     char *pstr,    sizeof(char *)
     NULL,          sizeof(char *)
     char str[]     strlen(errorinfo)+1
   }
*/
//todo: handle "thread not initialized to FreePM (PMERR_INVALID_HAB)"

PERRINFO    APIENTRY    WinGetErrorInfo(HAB iHAB)
{  int rc, errid,l;
   ERRINFO *perrInf;
   char *perrstr, *perrInfStr, * *ptr;

   errid = WinGetLastError(iHAB);
   if(errid == -1) return NULL;
   perrstr = GetFreePMErrorMessage(errid);
   l = strlen(perrstr);

   perrInf = (PERRINFO) calloc(sizeof(ERRINFO)+ l+1 + 2 * sizeof(char *)+1, 1);
   perrInf->cbFixedErrInfo = sizeof(ERRINFO);
   perrInf->idError =  errid;
   perrInf->offaoffszMsg = sizeof(ERRINFO);
   perrInf->offBinaryData = 0;
   ptr = (char * *)( ((char *)perrInf) + sizeof(ERRINFO));
   perrInfStr = ((char *)perrInf) + sizeof(ERRINFO) + 2 * sizeof(char *);
   strcpy(perrInfStr,perrstr);
   *ptr =  perrInfStr;
   return perrInf;
}

BOOL        APIENTRY    WinFreeErrorInfo(PERRINFO perrinfo)
{  if(perrinfo == NULL) return FALSE;
   free(perrinfo);
   return TRUE;
}


