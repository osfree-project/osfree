#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#include <os2.h>

typedef ULONG MSGID;

#define MSG_INSERTMAXLEN 255

BOOL MSG_Init (PSZ MsgFile);
VOID MSG_Print (MSGID MessageID);
VOID MSG_fPrint (HFILE FileHandle, MSGID MessageID);
BOOL MSG_Get (PCHAR DestPtr, ULONG DestMaxLength, MSGID MessageID);
BOOL MSG_FillInsert (ULONG InsertNo, MSGID MessageID);
BOOL MSG_SetInsertViaPSZ (ULONG InsertNo, PSZ String);
BOOL MSG_SetInsertViaString (ULONG InsertNo, PCHAR StringPtr, ULONG StringLen);
BOOL MSG_SetInsertFileLocation (ULONG InsertNo, PSZ FileName, ULONG LineNo);
