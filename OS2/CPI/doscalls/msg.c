/*!

   @file msg.c

   @brief MSG API implementation.

   @author Yuri Prokushev <prokushev@freemail.ru>
   @author Valery Sedletski <_valerius@mail.ru>
   
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

/*!
   @brief Outputs a message to file

   @param hfile              file handle
   @param pBuf               message buffer
   @param cbMsg              message length

   @return
     NO_ERROR                successful return

   API
     DosWrite
*/

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
     ERROR_MR_UN_PERFORM     can't perform an action
     ERROR_MR_MSG_TOO_LONG   message exceeds CCHMAXPATH
     ERROR_INVALID_PARAMETER invalid parameter
     ERROR_MR_INV_IVCOUNT    invalid subst. vars. count

   API
     none
*/

APIRET APIENTRY DosInsertMessage(const PCHAR *pTable, ULONG cTable,
                                 PCSZ pszMsg, ULONG cbMsg, PCHAR pBuf,
                                 ULONG cbBuf, PULONG pcbMsg)
{
  // Check arguments
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
    int   srclen;
    int   dstlen;
    int   len, rest, maxlen = 0;
    int   ivcount = 0;
    int   i;

    src    = (char *)pszMsg;
    dst    = pBuf;
    srclen = cbMsg;
    maxlen = srclen;
    dstlen = 0;

    // add params lenths (without zeroes)
    for (i = 0; i < cTable; i++)
      maxlen += strnlen(pTable[i], cbBuf) - 1;

    srclen = maxlen;

    for (;;)
    {
      if (*src == '%')
      {
        src++;
        srclen--;
        ivcount++;

        if (ivcount > 9)
          return ERROR_MR_INV_IVCOUNT;

        switch (*src)
        {
          case '%': // %%
            *dst++ = *src;
            srclen--;
            dstlen++;
            break;
          case '0': // %0
            srclen--;
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
            len = strnlen(pTable[*src - '1'], cbBuf);
            strncpy(dst, pTable[*src - '1'],  len);
            dst    += len;
            dstlen += len;
            srclen -= len;
            break;
          default:  // Can't perfom action?
            if (srclen <= 0)
              break;

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

      if (srclen <= 0)
        break;

      // if no bytes remaining for terminating zero, return an error
      if (dstlen > maxlen)
      {
        *pcbMsg = cbBuf;
        pBuf[cbBuf - 1] = '\0';
        return ERROR_MR_MSG_TOO_LONG;
      }
    }

    pBuf[dstlen++] = '\0';
    *pcbMsg = dstlen;

    return NO_ERROR;
  }
}


APIRET APIENTRY      PvtLoadMsgFile(PSZ pszFile, PVOID *buf, PULONG pcbFile)
{
  FILESTATUS3 fileinfo;
  HFILE    hf;
  ULONG    ulAction;
  LONGLONG ll;
  char     fn[CCHMAXPATH];
  ULONG    fisize;
  ULONG    ulActual;
  APIRET   rc;

  ll.ulLo = 0;
  ll.ulHi = 0;

  if (!pszFile || !*pszFile)
    return ERROR_INVALID_PARAMETER;

  if (strnlen(pszFile, CCHMAXPATH) == CCHMAXPATH)
    return ERROR_FILENAME_EXCED_RANGE;

  // try opening the file from the root dir/as is
  rc = DosOpenL(pszFile,                    // File name
                &hf,                        // File handle
                &ulAction,                  // Action
                ll,                         // Initial file size
                0,                          // Attributes
                OPEN_ACTION_FAIL_IF_NEW |   // Open type
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYNONE |       // Open mode
                OPEN_ACCESS_READONLY,
                NULL);                      // EA

  if (rc && rc != ERROR_FILE_NOT_FOUND && rc != ERROR_OPEN_FAILED)
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
                       CCHMAXPATH);

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
  }

  if (rc)
    return rc;

  // file is found, so get file size
  rc = DosQueryPathInfo(fn,
                        FIL_STANDARD,
                        &fileinfo,
                        sizeof(FILESTATUS3));

  if (rc)
    return rc;

  // allocate a buffer for the file
  rc = DosAllocMem(buf, fileinfo.cbFile,
                   PAG_READ | PAG_WRITE | PAG_COMMIT);

  if (rc)
    return rc;

  // read the file into memory
  rc = DosRead(hf,
               *buf,
               fileinfo.cbFile,
               &ulActual);

  if (rc)
    return rc;

  // close file
  DosClose(hf);
  *pcbFile = fileinfo.cbFile;

  return rc;
}

APIRET APIENTRY      PvtChkMsgFileFmt(void *msgSeg)
{

  if (strncmp(msgSeg, HDR_MAGIC, 8))
    return ERROR_MR_INV_MSGF_FORMAT; // invalid message format
  // additional checks, if any
  // ...

  return NO_ERROR;
}

/*!
     @brief Queries message file codepage data

     @param pb            user supplied buffer for CP data
     @param cb            buffer length
     @param pszFile       message file specification
     @param cbBuf         returned data actual size
     @param msgSeg        bound message segment address, if any

     @return
       NO_ERROR           successful return
       ERROR_FILENAME_EXCED_RANGE   filename too long
       ERROR_OPEN_FAILED            open failed
       ERROR_FILE_NOT_FOUND         file not found
       ERROR_MR_UN_ACC_MSGF         message file unaccessible
       ERROR_BUFFER_OVERFLOW        buffer too small
       ERROR_MR_INV_MSGF_FORMAT     invalid message format
       ERROR_MR_UN_PERFORM          can't perform an action

     API
       DosOpenL
       DosRead
       DosClose
       DosAllocMem
       DosFreeMem
       DosSearchPath
       DosQueryPathInfo

*/

APIRET APIENTRY      DosIQueryMessageCP(PCHAR pb, ULONG cb,
                                        PSZ pszFile,
                                        PULONG cbBuf, void *msgSeg)
{
  APIRET   rc;
  ULONG    cbFile;
  int      cp_cnt, i;
  void     *buf;
  char     *msg, *p = pb;
  msghdr_t *hdr;
  ctry_block_t *ctry;

  if (!pb || !cb || !pszFile || !*pszFile)
    return ERROR_INVALID_PARAMETER;

  if (msgSeg)
  {
    if (PvtChkMsgFileFmt(msgSeg))
      // file format is invalid
      msgSeg = 0;
  }

  if (!msgSeg)
  {
    // try opening file from DASD
    rc = PvtLoadMsgFile(pszFile, &buf, &cbFile);
    msgSeg = buf;
  }

  if (!msgSeg || rc)
    return ERROR_MR_UN_ACC_MSGF; // Unable to access message file

  if (PvtChkMsgFileFmt(msgSeg))
    return ERROR_MR_INV_MSGF_FORMAT; // invalid message format

  // from this point, the file/msg seg is loaded at msgSeg address
  msg = (char *)msgSeg;  // message pointer
  hdr = (msghdr_t *)msg; // message header

  // country info
  ctry = (ctry_block_t *)(msg + hdr->ctry_info_ofs);
  // codepages count
  cp_cnt = ctry->codepages_no;

  if (!cp_cnt)
    return ERROR_MR_UN_PERFORM;

  if (cp_cnt > 16)
    return ERROR_INVALID_PARAMETER;

  if (6 + 2 * cp_cnt > cb)
    return ERROR_BUFFER_OVERFLOW;

  *((USHORT *)p) = cp_cnt;
  p += 2;

  for (i = 0; i < cp_cnt; i++, p += 2)
    *((USHORT *)p) = ctry->codepages[i];

  *((USHORT *)p) = ctry->lang_family_id;
  p += 2;
  *((USHORT *)p) = ctry->lang_dialect_id;
  p += 2;

  // returned data size
  *cbBuf =  p - pb;

  // finally, free file buffer
  DosFreeMem(buf);


  return NO_ERROR;
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
       NO_ERROR                     a successful return from the API
       ERROR_FILENAME_EXCED_RANGE   filename too long
       ERROR_OPEN_FAILED            open failed
       ERROR_FILE_NOT_FOUND         file not found
       ERROR_MR_MSG_TOO_LONG        message exceeds CCHMAXPATH
       ERROR_INVALID_PARAMETER      invalid parameter
       ERROR_MR_INV_IVCOUNT         invalid subst. vars. count
       ERROR_MR_UN_ACC_MSGF         unable to access message file
       ERROR_MR_INV_MSGF_FORMAT     invalid message format
       ERROR_MR_UN_PERFORM          can't perform an action

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
  ULONG  cbFile;
  void   *buf;
  char   *msg;
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
    if (PvtChkMsgFileFmt(msgSeg))
      // file format is invalid
      msgSeg = 0;
  }

  if (!msgSeg)
  {
    // try opening file from DASD
    rc = PvtLoadMsgFile(pszFile, &buf, &cbFile);
    msgSeg = buf;
  }

  if (!msgSeg || rc)
    return ERROR_MR_UN_ACC_MSGF; // Unable to access message file

  if (PvtChkMsgFileFmt(msgSeg))
    return ERROR_MR_INV_MSGF_FORMAT; // invalid message format

  // from this point, the file/msg seg is loaded at msgSeg address
  msg = (char *)msgSeg;  // message pointer
  hdr = (msghdr_t *)msg; // message header
  msgnumber -= hdr->firstmsgno;

  // get message offset
  if (hdr->is_offs_16bits) // if offset is 16 bits
    msgoff = (int)(*(unsigned short *)(msg + hdr->idx_ofs + 2 * msgnumber));
  else // it is 32 bits
    msgoff = (int)(*(unsigned long *)(msg + hdr->idx_ofs + 4 * msgnumber));

  if (msgnumber + 1 == hdr->msgs_no) // last message
    msgend = hdr->next_ctry_info;
  else
  {
    // get next message offset
    if (hdr->is_offs_16bits) // if offset is 16 bits
      msgend = (int)(*(unsigned short *)(msg + hdr->idx_ofs + 2 * (msgnumber + 1)));
    else // it is 32 bits
      msgend = (int)(*(unsigned long *)(msg + hdr->idx_ofs + 4 * (msgnumber + 1)));
  }

  if (msgoff > cbFile || msgend > cbFile)
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
      // prepend the Warning/Error ID (like SYS3175: )
      printf("%s%04u: ", id, msgnumber + 1);
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
