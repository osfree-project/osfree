/*!

   @file msg.c

   @brief MSG API implementation.

   @author Yuri Prokushev <prokushev@freemail.ru>
   @author Valery Sedletski <_valerius@mail.ru>

*/

#include "kal.h"

#include <stdio.h>
#include <string.h>

// Safe functions
#include <strlcpy.h>
#include <strnlen.h>

#include "msg.h"

void log(const char *fmt, ...);

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
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("hfile=0x%x\n", hfile);
  log("cbMsg=%lu\n", cbMsg);
  log("pBuf=%s\n", pBuf);
  rc = DosWrite(hfile, pBuf, cbMsg, &ulActual);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
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

APIRET APIENTRY DosInsertMessage(PCHAR *pTable, ULONG cTable,
                                 PCSZ pszMsg, ULONG cbMsg, PCHAR pBuf,
                                 ULONG cbBuf, PULONG pcbMsg)
{
  APIRET rc;
  int i;

  log("%s enter\n", __FUNCTION__);

  // output args to log
  log("cbMsg=%u", cbMsg);
  log("pszMsg=");

  for (i = 0; i < cbMsg; i++)
    log("%c", pszMsg[i]);

  log("\n");

  log("cTable=%u\n", cTable);

  if (pTable)
  {
    for (i = 0; i < cTable; i++)
      log("pTable[%u]=%s\n", i, pTable[i]);
  }
  else
    log("pTable=0\n");

  log("pBuf=0x%x\n", pBuf);
  log("cbBuf=%lu\n", cbBuf);

  // Check arguments
  if (! pszMsg)
  {
    rc = ERROR_INVALID_PARAMETER;                   // Nothing to proceed
    goto DOSINSERTMESSAGE_EXIT;
  }

  if (! pBuf)
  {
    rc = ERROR_INVALID_PARAMETER;                   // No target buffer
    goto DOSINSERTMESSAGE_EXIT;
  }

  if ((cTable) && (!pTable))
  {
    rc = ERROR_INVALID_PARAMETER;                   // No inserting strings array
    goto DOSINSERTMESSAGE_EXIT;
  }

  if (cbMsg > cbBuf)
  {
    rc = ERROR_MR_MSG_TOO_LONG;                     // Target buffer too small
    goto DOSINSERTMESSAGE_EXIT;
  }

  // If nothing to insert then just copy message to buffer
  if (!cTable)
  {

    strlcpy(pBuf, pszMsg, cbMsg);
    rc = NO_ERROR;
    goto DOSINSERTMESSAGE_EXIT;

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

    log("srclen=%u\n", srclen);

    // add params lenths (without zeroes)
    for (i = 0; i < cTable; i++)
      maxlen += strnlen(pTable[i], cbBuf) - 1;

    for (;;)
    {
      if (*src == '%')
      {
        src++;
        srclen--;
        ivcount++;

        if (ivcount > 9)
        {
          rc = ERROR_MR_INV_IVCOUNT;
          goto DOSINSERTMESSAGE_EXIT;
        }

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
            //srclen -= len;
            break;
          default:  // Can't perfom action?
            if (srclen <= 0)
              break;

            rc = ERROR_MR_UN_PERFORM;
            goto DOSINSERTMESSAGE_EXIT;
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
        rc = ERROR_MR_MSG_TOO_LONG;
        goto DOSINSERTMESSAGE_EXIT;
      }
    }

    pBuf[dstlen++] = '\0';
    *pcbMsg = dstlen;
    log("*pcbMsg=%lu\n", *pcbMsg);

    rc = NO_ERROR;
  }

DOSINSERTMESSAGE_EXIT:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET APIENTRY      PvtLoadMsgFile(PSZ pszFile, PVOID *buf, PULONG pcbFile)
{
  FILESTATUS3 fileinfo;
  HFILE    hf;
  ULONG    ulAction;
  LONGLONG ll;
  char     fn[CCHMAXPATH] = "";
  ULONG    len;
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

  if (rc && rc != ERROR_FILE_NOT_FOUND &&
            rc != ERROR_OPEN_FAILED    &&
            rc != ERROR_PATH_NOT_FOUND)
    return rc;

  if (rc) // file not found
  {
    // otherwise, try searching in the currentdir and on path
    rc = DosSearchPath(SEARCH_IGNORENETERRS |
                       SEARCH_ENVIRONMENT   |
                       SEARCH_CUR_DIRECTORY,
                       "DPATH",
                       pszFile,
                       fn,
                       CCHMAXPATH); // returns .\OSO001.MSG, which is incorrect

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

  if (! fn[0])
    strcpy(fn, pszFile);

  // file is found, so get file size
  rc = DosQueryPathInfo(fn,
                        FIL_STANDARD,
                        &fileinfo,
                        sizeof(FILESTATUS3));

  if (rc)
    return rc;

  if (!fileinfo.cbFile)
  {
    log("DosQueryPathInfo returned zero .msg file size!\n");
    return ERROR_INVALID_PARAMETER;
  }
  else
    log("fileinfo.cbFile=%x\n", fileinfo.cbFile);

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
  APIRET   rc = NO_ERROR;
  ULONG    cbFile;
  int      cp_cnt, i;
  void     *buf;
  char     *msg, *p = pb;
  msghdr_t *hdr;
  ctry_block_t *ctry;

  log("%s enter\n", __FUNCTION__);

  log("pb=0x%lx\n", pb);
  log("cb=%lu\n", cb);

  if (!pszFile && !*pszFile)
    log("pszFile=%s\n", pszFile);

  log("msgSeg=0x%lx\n", msgSeg);

  if (! pb || ! cb || ! pszFile || ! *pszFile)
  {
    rc = ERROR_INVALID_PARAMETER;
    goto DOSIQUERYMESSAGECP;
  }

  if (msgSeg)
  {
    if (PvtChkMsgFileFmt(msgSeg))
      // file format is invalid
      msgSeg = 0;
  }

  if (! msgSeg)
  {
    // try opening file from DASD
    rc = PvtLoadMsgFile(pszFile, &buf, &cbFile);
    msgSeg = buf;
  }

  if (! msgSeg || rc)
  {
    rc = ERROR_MR_UN_ACC_MSGF; // Unable to access message file
    goto DOSIQUERYMESSAGECP;
  }

  if (PvtChkMsgFileFmt(msgSeg))
  {
    rc = ERROR_MR_INV_MSGF_FORMAT; // invalid message format
    goto DOSIQUERYMESSAGECP;
  }

  // from this point, the file/msg seg is loaded at msgSeg address
  msg = (char *)msgSeg;  // message pointer
  hdr = (msghdr_t *)msg; // message header

  // country info
  ctry = (ctry_block_t *)(msg + hdr->ctry_info_ofs);
  // codepages count
  cp_cnt = ctry->codepages_no;

  if (! cp_cnt)
  {
    rc = ERROR_MR_UN_PERFORM;
    goto DOSIQUERYMESSAGECP;
  }

  if (cp_cnt > 16)
  {
    rc = ERROR_INVALID_PARAMETER;
    goto DOSIQUERYMESSAGECP;
  }

  if (6 + 2 * cp_cnt > cb)
  {
    rc = ERROR_BUFFER_OVERFLOW;
    goto DOSIQUERYMESSAGECP;
  }

  *((USHORT *)p) = cp_cnt;
  log("codepage cnt=%u\n", cp_cnt);
  p += 2;

  for (i = 0; i < cp_cnt; i++, p += 2)
  {
    *((USHORT *)p) = ctry->codepages[i];
    log("codepage%u=%u\n", i, ctry->codepages[i]);
  }

  *((USHORT *)p) = ctry->lang_family_id;
  log("language family id=%u\n", ctry->lang_family_id);
  p += 2;
  *((USHORT *)p) = ctry->lang_dialect_id;
  log("language dialect id=%u\n", ctry->lang_dialect_id);
  p += 2;

  // returned data size
  *cbBuf =  p - pb;
  log("*cbBuf=%lu\n", *cbBuf);

  // finally, free file buffer
  DosFreeMem(buf);

DOSIQUERYMESSAGECP:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
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
  APIRET rc = NO_ERROR;
  ULONG  cbFile;
  void   *buf;
  char   *msg;
  char   id[4];
  char   str[CCHMAXPATH];
  msghdr_t *hdr = (msghdr_t *)msgSeg;
  int    msgoff, msgend, msglen, i;

  ULONG  len;

  log("%s enter\n", __FUNCTION__);
  log("msgSeg=0x%lx\n", msgSeg);

  // output args to log
  log("cTable=%u\n", cTable);

  for (i = 0; i < cTable; i++)
  {
    log("pTable[%u]=%s\n", i, pTable[i]);
  }

  log("pBuf=0x%x\n", pBuf);
  log("cbBuf=%lu\n", cbBuf);
  log("msgnumber=%lu\n", msgnumber);
  log("pszFile=%s\n", pszFile);

  /* Check arguments */
  if (cTable > 9)
  {
    rc = ERROR_MR_INV_IVCOUNT;
    goto DOSTRUEGETMESSAGE_EXIT;
  }

  if (! pBuf || ! cbBuf)
  {
    rc = ERROR_INVALID_PARAMETER;
    goto DOSTRUEGETMESSAGE_EXIT;
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

  if (! msgSeg || rc)
  {
    rc = ERROR_MR_UN_ACC_MSGF; // Unable to access message file
    goto DOSTRUEGETMESSAGE_EXIT;
  }

  if (PvtChkMsgFileFmt(msgSeg))
  {
    rc = ERROR_MR_INV_MSGF_FORMAT; // invalid message format
    goto DOSTRUEGETMESSAGE_EXIT;
  }

  // from this point, the file/msg seg is loaded at msgSeg address
  msg = (char *)msgSeg;  // message pointer
  hdr = (msghdr_t *)msg; // message header
  msgnumber -= hdr->firstmsgno;

  if (msgnumber > hdr->msgs_no)
  {
    rc = ERROR_MR_MID_NOT_FOUND; // ???
    goto DOSTRUEGETMESSAGE_EXIT;
  }

  // get message offset
  if (hdr->is_offs_16bits) // if offset is 16 bits
    msgoff = (int)(*(unsigned short *)(msg + hdr->idx_ofs + 2 * msgnumber));
  else // it is 32 bits
    msgoff = (int)(*(unsigned long *)(msg + hdr->idx_ofs + 4 * msgnumber));

  if (msgnumber + 1 == hdr->msgs_no) // last message
  {
    if (hdr->next_ctry_info)
      msgend = hdr->next_ctry_info;
    else
      msgend = cbFile; // EOF
  }
  else
  {
    // get next message offset
    if (hdr->is_offs_16bits) // if offset is 16 bits
      msgend = (int)(*(unsigned short *)(msg + hdr->idx_ofs + 2 * (msgnumber + 1)));
    else // it is 32 bits
      msgend = (int)(*(unsigned long *)(msg + hdr->idx_ofs + 4 * (msgnumber + 1)));
  }

  log("msgoff=0x%lx\n", msgoff);
  log("msgend=0x%lx\n", msgend);

  if (msgoff > cbFile || msgend > cbFile)
  {
    rc = ERROR_MR_MSG_TOO_LONG;
    goto DOSTRUEGETMESSAGE_EXIT;
  }

  // message length
  msglen = msgend - msgoff - 1;

  // msg now points to the desired message
  msg += msgoff;

  // OS/2 actually does not check for prefixes
  //if (*msg != 'E' && *msg != 'W' &&
  //    *msg != 'P' && *msg != 'I' &&
  //    *msg != 'H' && *msg != '?')
  //  {
  //    rc = ERROR_MR_INV_MSGF_FORMAT;
  //    goto DOSTRUEGETMESSAGE_EXIT;
  //  }

  // message file ID
  strncpy(id, hdr->id, 3);
  id[3] = '\0';

  switch (*msg)
  {
    case 'E': // Error
    case 'W': // Warning
    case 'I': // Info
      // prepend the Warning/Error ID (like SYS3175: )
      sprintf(str, "%s%04u: ", id, msgnumber);
      DosPutMessage(1, strlen(str), str);
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

  // finally, free file buffer
  DosFreeMem(buf);

DOSTRUEGETMESSAGE_EXIT:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
