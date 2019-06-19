/*
 *      MsgCode.h
 *
 *      @(#)msgcode.h	1.1 5/3/96 16:39:17 /users/sccs/src/samples/msgdemo/s.msgcode.h
 *
 *      typedefs and function declarrations for MsgCode.c
 *
*/

typedef struct  {
        int     msgid;
        char   *msgstr;
        int     msgcnt;
        int     msgop;
} MSGCODE;

MSGCODE * GetMsgCode(int);
