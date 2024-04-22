/*
   defines for messages identification,
   shared along all os source

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   Bartosz Tomasik <bart2@asua.org.pl>
   JMA <jma@jma.se>
*/

#ifndef _ALL_MESSAGEIDS_H_
#define _ALL_MESSAGEIDS_H_

#if __386__
#include <basemid.h>
#else

#define MSG_FILE_NOT_FOUND              2
#define MSG_BAD_PARM1                   1001
#define MSG_BAD_PARM2                   1002
#define MSG_BAD_SYNTAX                  1003

#endif

#endif /* _ALL_MESSAGEIDS_H_ */
