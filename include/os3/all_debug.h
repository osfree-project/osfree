/*
   defines for messages identification,
   shared along all os source

   (c) osFree Project 2009, <http://www.osFree.org>

   Yuri Prokushev <yuri.prokushev@gmail.com>
*/

#ifndef _ALL_DEBUG_H_
#define _ALL_DEBUG_H_

all_DebugLog();

#define DEBUGMSG             printf("%s: %s: %d ",__FILE__, __FUNCTION__, __LINE__);printf
#define all_FunctionEnter()  printf("%s: %d: %s() ->" NEWLINE,__FILE_, __LINE__, __FUNCTION__)
#define all_FunctionExit(rc) printf("%s() <- rc=%u" NEWLINE, __FUNCTION__, rc)


#endif /* _ALL_DEBUG_H_ */
