/*!

   @file msg.c

   @brief MSG API implementation.

   @author Yuri Prokushev <prokushev@freemail.ru>

*/

#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <string.h>

// Safe functions
#include <strlcpy.h>
#include <strnlen.h>

#include "msg.h"

void log(const char *fmt, ...);
APIRET unimplemented(char *func);

APIRET APIENTRY  DosPutMessage(HFILE hfile,
                               ULONG cbMsg,
                               PCHAR pBuf)
{
  ULONG ulActual;
  return DosWrite(hfile, pBuf, cbMsg, &ulActual);
}


/*!
   @brief Inserts arguments on places of %n (Similar to CLib sprintf function but much simpler)

   @param pTable             pointer table of arguments
   @param cTable             number of variable insertion text strings
   @param pszMsg             address of the input message
   @param cbMsg              length, in bytes, of the input message
   @param pBuf               address of the caller's buffer area where the system returns the requested message
   @param cbBuf              length, in bytes, of the caller's buffer area
   @param pcbMsg             length, in bytes, of the updated message returned

   @return
     NO_ERROR                message processed succesfully

   API

*/

APIRET APIENTRY DosInsertMessage(const PCHAR *pTable, ULONG cTable,
                                 PCSZ pszMsg, ULONG cbMsg, PCHAR pBuf,
                                 ULONG cbBuf, PULONG pcbMsg)
{
  // Check arguments
  //if (!pcbMsg) return ERROR_INVALID_PARAMETER;               // No result size variable
  if (!pszMsg) return ERROR_INVALID_PARAMETER;                 // Nothing to proceed
  if (!pBuf) return ERROR_INVALID_PARAMETER;                   // No target buffer
  if ((cTable) && (!pTable)) return ERROR_INVALID_PARAMETER;   // No inserting strings array
  if (cbMsg > cbBuf) return ERROR_MR_MSG_TOO_LONG;             // Target buffer too small

  // If nothing to insert then just copy message to buffer
  if (!cTable)
  {
    strlcpy(pBuf, pszMsg, cbBuf);
    return NO_ERROR;
  } else { // Produce output string
    PCHAR src;
    PCHAR dst;
    ULONG srclen;
    ULONG dstlen;
    ULONG len;

    src    = pszMsg;
    dst    = pBuf;
    srclen = cbMsg;
    dstlen = 0;

    while (srclen)
    {
      if (*src == '%')
      {
        src++;
        dstlen++;

        switch (*src)
        {
          case '%': // %%
            *dst = *src;
            break;
          case '0': // %0
            break;
          case '1': // %1
          case '2': // %2
          case '3': // %3
          case '4': // %4
          case '5': // %5
          case '6': // %6
          case '7': // %7
          case '8': // %8
          case '9': // %9
            strlcpy(dst, pTable[*src], CCHMAXPATH);
            len    =  strnlen(pTable[*src], CCHMAXPATH);
            dst    += len;
            dstlen += len;
            break;
          default:  // Can't perfom action?
            return ERROR_MR_UN_PERFORM;
        }

        src++;
      }
      else
      {
        *dst++ = *src++;
        srclen--;
        dstlen++;
      }

      *pcbMsg = dstlen;

      if (dstlen > CCHMAXPATH)
        return ERROR_MR_MSG_TOO_LONG;
    }

    return NO_ERROR;
  }
}

APIRET APIENTRY      DosIQueryMessageCP(PCHAR pb, ULONG cb,
                                        PSZ pszFile,
                                        PULONG cbBuf, void *msgSeg)
{
  return unimplemented(__FUNCTION__);
}

/*!  @brief Searches for a message in a message file, with a given message number and
            returns it with a number of string substituted to %i placeholders.

     @param msgSeg        a message segment address (if it's contained in an executable)
     @param pTable        an array of substitution strings
     @param cTable        a substitution strings number
     @param pBuf          a user buffer
     @param cbBuf         a user buffer size
     @param msgnumber     a message number
     @param pszFile       a message file name
     @param pcbMsg        an actual returned message length

     @return
       NO_ERROR           a successful return from the API

     API
       DosOpenL
       DosRead
       DosClose
       DosSearchPath
       DosQueryPathInfo
       DosAllocMem
       DosFreeMem
       DosInsertMessage
 */

APIRET APIENTRY DosTrueGetMessage(void *msgSeg, PCHAR *pTable, ULONG cTable, PCHAR pBuf,
                                  ULONG cbBuf, ULONG msgnumber,
                                  PSZ pszFile, PULONG pcbMsg)
{
  APIRET rc;
  HFILE  hf;
  ULONG  ulAction;
  char   fn[CCHMAXPATH];
  FILESTATUS3 fileinfo;
  LONGLONG ll;
  ULONG  fisize;
  ULONG  ulActual;
  char   *buf, *msg;
  char   id[4];
  msghdr_t *hdr = (msghdr_t *)msgSeg;
  int    msgoff, msgend, msglen;

  ULONG  len;

  log("msgSeg=%lx\n", msgSeg);
  log("*pTable=%lx\n", *pTable);
  log("cTable=%lu\n", cTable);
  log("pBuf=%lx\n", pBuf);
  log("cbBuf=%lu\n", cbBuf);
  log("msgnumber=%lu\n", msgnumber);
  log("pszFile=%s\n", pszFile);
  log("*pcbMsg=%lu\n", *pcbMsg);

  ll.ulLo = 0;
  ll.ulHi = 0;

  /* Check arguments */
  if (cTable > 9)
    return ERROR_MR_INV_IVCOUNT;

  if (!pBuf || !cbBuf)
    return ERROR_INVALID_PARAMETER;

  if (!pTable)
  {
    pTable[0] = "";
    cTable = 1;
  }

  if (msgSeg)
  {
    if (strncmp(msgSeg, HDR_MAGIC, 8))
      // header is invalid
      msgSeg = 0;
    // additional checks, if any
  }

  // try opening file from DASD
  if (!msgSeg)
  {
    // do some checks first
    if (!pszFile || !*pszFile)
      return ERROR_INVALID_PARAMETER;

    // try opening the file from the root dir/as is
    rc = DosOpenL(pszFile,                    // File name
                  &hf,                        // File handle
                  &ulAction,                  // Action
                  ll,                          // Initial file size
                  0,                          // Attributes
                  OPEN_ACTION_FAIL_IF_NEW |   // Open type
                  OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_SHARE_DENYNONE |       // Open mode
                  OPEN_ACCESS_READONLY,
                  NULL);                      // EA

    if (rc && rc != ERROR_FILE_NOT_FOUND)
      return rc;

    if (rc) // file not found
    {
      // if filename is fully qualified, return an error
      if (pszFile[1] ==':' && pszFile[2] == '\\')
        return rc;

      // otherwise, try searchin in the currentdir and on path
      rc = DosSearchPath(SEARCH_IGNORENETERRS |
                         SEARCH_ENVIRONMENT   |
                         SEARCH_CUR_DIRECTORY,
                         "DPATH",
                         pszFile,
                         fn,
                         strlen(fn) + 1);

      if (rc)
        return rc;

      // file is found, so get file size
      rc = DosQueryPathInfo(fn,
                            FIL_STANDARD,
                            &fileinfo,
                            sizeof(FILESTATUS3));

      if (rc)
        return rc;

      // open it
      rc = DosOpenL(fn,
                    &hf,
                    &ulAction,
                    ll,
                    0,
                    OPEN_ACTION_FAIL_IF_NEW |
                    OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_SHARE_DENYNONE |
                    OPEN_ACCESS_READONLY,
                    NULL);

      if (rc)
        return rc;
    }

    // allocate a buffer for the file
    rc = DosAllocMem((void **)&buf, fileinfo.cbFile,
                     PAG_READ | PAG_WRITE | PAG_COMMIT);

    if (rc)
      return rc;

    // read the file into memory
    rc = DosRead(hf,
                 buf,
                 fileinfo.cbFile,
                 &ulActual);

    if (rc)
      return rc;

    // close file
    DosClose(hf);

    msgSeg = buf;
  }

  // from this point, the file/msg seg is loaded at msgSeg address
  msg = (char *)msgSeg; // message pointer
  msgnumber -= hdr->firstmsgno;

  // get message offset
  if (hdr->is_offs_16bits) // if offset is 16 bits
    msgoff = (int)(*(unsigned short *)(hdr->idx_ofs + 2 * msgnumber));
  else // it is 32 bits
    msgoff = (int)(*(unsigned long *)(hdr->idx_ofs + 4 * msgnumber));

  if (msgnumber + 1 == hdr->msgs_no) // last message
    msgend = hdr->next_ctry_info;
  else
  {
    // get next message offset
    if (hdr->is_offs_16bits) // if offset is 16 bits
      msgend = (int)(*(unsigned short *)(hdr->idx_ofs + 2 * (msgnumber + 1)));
    else // it is 32 bits
      msgend = (int)(*(unsigned long *)(hdr->idx_ofs + 4 * (msgnumber + 1)));
  }

  if (msgoff > fileinfo.cbFile || msgend > fileinfo.cbFile)
    return ERROR_MR_MSG_TOO_LONG;

  // message length
  msglen = msgend - msgoff - 1;

  // msg now points to the desired message
  msg += msgoff;

  // OS/2 actually does not check for prefixes
  //if (*msg != 'E' && *msg != 'W' &&
  //    *msg != 'P' && *msg != 'I' &&
  //    *msg != 'H' && *msg != '?')
  //  rc = ERROR_MR_INV_MSGF_FORMAT;

  // message file ID
  strncpy(id, hdr->id, 3);

  switch (*msg)
  {
    case 'E': // Error
    case 'W': // Warning
      printf("%s%u: ", id, msgnumber + 1);
      break;
    default:
      break;
  }

  // skip message type letter
  msg++;

  // substitute %? to the actual parameters
  rc = DosInsertMessage(pTable, cTable,
                        msg, msglen,
                        pBuf, cbBuf,
                        pcbMsg);

  // display the actual message
  printf("%s\n", pBuf);

  // finally, free file buffer
  DosFreeMem(buf);


  return rc;
}
