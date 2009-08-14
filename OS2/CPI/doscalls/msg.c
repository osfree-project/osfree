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

APIRET APIENTRY  DosPutMessage(HFILE hfile,
                                  ULONG cbMsg,
                                  PCHAR pBuf)
{
  ULONG ulActual;
  return DosWrite(hfile, pBuf, cbMsg, &ulActual);
}


/*!
   @brief Inserts arguments on places of %n (Similar to CLib sprintf function but much simple)

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

APIRET APIENTRY DosInsertMessage(const PCHAR *  pTable, ULONG cTable, PCSZ pszMsg, ULONG cbMsg, PCHAR pBuf, ULONG cbBuf, PULONG pcbMsg)
{
  // Check arguments
  if (!pcbMsg) return ERROR_INVALID_PARAMETER;               // No result size variable
  if (!pszMsg) return ERROR_INVALID_PARAMETER;               // Nothing to proceed
  if (!pBuf) return ERROR_INVALID_PARAMETER;                 // No target buffer
  if ((cTable) && (!pTable)) return ERROR_INVALID_PARAMETER; // No inserting strings array
  if (cbMsg>cbBuf) return ERROR_MR_MSG_TOO_LONG;             // Target buffer too small

  // If nothing to insert then just copy message to buffer
  if (!cTable)
  {
    strcpy(pszMsg, pBuf);
    return NO_ERROR;
  } else { // Produce output string
    PCHAR src;
    PCHAR dst;
    ULONG srclen;
    ULONG dstlen;

    src=pszMsg;
    dst=pBuf;
    srclen=cbMsg;
    dstlen=0;

    while (srclen!=0)
    {
      if (*src=='%')
      {
        src++;
        switch (*src)
        {
          case '%': { *dst=*src; break; } // %%
          case '1': // %1
          {
            break;
          }
          case '2': // %2
          {
            break;
          }
          case '3': // %3
          {
            break;
          }
          case '4': // %4
          {
            break;
          }
          case '5': // %5
          {
            break;
          }
          case '6': // %6
          {
            break;
          }
          case '7': // %7
          {
            break;
          }
          case '8': // %8
          {
            break;
          }
          case '9': // %9
          {
            break;
          }
          default: return ERROR_MR_UN_PERFORM; // Can't perfom action?
        }
      } else {
        *dst=*src;
      }
      src++;
      dst++;
      srclen--;
      dstlen++;
    }
    return NO_ERROR;
  }
}

APIRET APIENTRY      DosTrueGetMessage(void)
{
  printf(__FUNCTION__ " not implemented yet\n");
  return 0;
}
