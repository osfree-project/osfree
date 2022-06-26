/*
 * StHealth.H
 * Autor:               Stefan Milcke
 * Erstellt am:         14.12.1998
 * Letzte Aenderung am: 03.02.2001
 *
*/

#ifndef STHEALTH_H_INCLUDED
#define STHEALTH_H_INCLUDED

/* defines for OS/2 include */
#define INCL_DOSSEMAPHORES

/* Standard headers */

/* UI Class Library headers */

/* Application headers */

/* OS/2 Headers */
#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STHEALTH_VERSION                        (4)
#define STHEALTH_NOT_PRESENT_ERROR              (-255)
#define STHEALTH_CORRECTION_QUERY               (-65535)

#define STHEALTH_EVT_MAX_STRLEN                 (260)

#define STHEALTH_ERROR_NO_DAEMON                (-1)
#define STHEALTH_ERROR_DAEMON_NOT_REGISTERED    (-2)
#define STHEALTH_ERROR_INVALID_PARAMETER        (-3)
#define STHEALTH_ERROR_STRUCT_SIZE              (-4)
#define STHEALTH_ERROR_TOO_MANY_EVENTS          (-5)
#define STHEALTH_ERROR_MISSING_EVENT_NAME       (-6)
#define STHEALTH_ERROR_NAME_ALREADY_EXIST       (-7)
#define STHEALTH_ERROR_EVENTTABLE_DEADLOCK      (-8)
#define STHEALTH_ERROR_INVALID_EVENT_HANDLE     (-9)
#define STHEALTH_ERROR_MORE_EVENTS              (-10)

#define STHEALTH_EVT_SOURCE_TEMP                (1)
#define STHEALTH_EVT_SOURCE_FAN                 (2)
#define STHEALTH_EVT_SOURCE_VOLTAGE             (3)

#define STHEALTH_EVT_COMPARE_EQUAL              (0)
#define STHEALTH_EVT_COMPARE_GREATEROREQUAL     (1)
#define STHEALTH_EVT_COMPARE_GREATER            (2)
#define STHEALTH_EVT_COMPARE_LESSOREQUAL        (3)
#define STHEALTH_EVT_COMPARE_LESS               (4)

#define STHEALTH_EVT_ACTION_DISABLED            (1)
#define STHEALTH_EVT_ACTION_ALERT               (2)
#define STHEALTH_EVT_ACTION_ALERT_REPEAT        (4)
#define STHEALTH_EVT_ACTION_SHOW_MESSAGE        (8)
#define STHEALTH_EVT_ACTION_EXECUTE_PGM         (16)
#define STHEALTH_EVT_ACTION_APMPOWEROFF         (32)

#define STHEALTH_EVT_STATE_ISACTIVE             (128)
#define STHEALTH_EVT_STATE_ALERTED              (2)
#define STHEALTH_EVT_STATE_MESSAGE_SHOWN        (8)
#define STHEALTH_EVT_STATE_EXECUTED             (16)

#define STHEALTH_EVT_CHECK_ASDEFINED            (1)
#define STHEALTH_EVT_CHECK_CHECKONLY            (2)

#define STHEALTH_EVT_FILTER_LEVEL_1             (1)
#define STHEALTH_EVT_FILTER_LEVEL_2             (2)
#define STHEALTH_EVT_FILTER_LEVEL_3             (3)
#define STHEALTH_EVT_FILTER_LEVEL_4             (4)
#define STHEALTH_EVT_FILTER_LEVEL_5             (5)
#define STHEALTH_EVT_FILTER_LEVEL_101           (101)
#define STHEALTH_EVT_FILTER_LEVEL_102           (102)
#define STHEALTH_EVT_FILTER_LEVEL_103           (103)

#ifndef NO_STHEALTH_PRAGMA_LIBRARY
 #pragma library("StHealth.LIB")
#endif

extern unsigned long StHealthBuildLevel;

#pragma pack(1)
/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/
/* ------------------------------ event storage ------------------------------*/
typedef struct _STHEALTH_EVENT
{
 ULONG cbLen;                              /* Length of structure */
 ULONG hevt;                               /* Handle of event */
 UCHAR source;                             /* Event source */
 UCHAR sourceNumber;                       /* Event source number */
 UCHAR compareOperation;                   /* Compare operation */
 LONG compareValue;                        /* Compare value multiplied with 100 */
 ULONG seconds;                            /* Time */
 ULONG actionFlags;                        /* What to do */
 ULONG actionState;                        /* Internal buffer for some flags */
 ULONG lastCheckpoint;                     /* Internal buffer */
 ULONG activeSince;                        /* Internal buffer */
 HEV semaphore;                            /* Semaphore (don't use directly !!!) */
 CHAR name[STHEALTH_EVT_MAX_STRLEN];       /* Event name */
 CHAR waveFile[STHEALTH_EVT_MAX_STRLEN];   /* Wave file */
 CHAR message[STHEALTH_EVT_MAX_STRLEN];    /* Message text */
 CHAR program[STHEALTH_EVT_MAX_STRLEN];    /* Path of programm to execute */
} STHEALTH_EVENT,*PSTHEALTH_EVENT;

/* -------------------------------- filtering --------------------------------*/
typedef struct _STHEALTH_EVT_FILTER_1
{
 ULONG cbLen;
 UCHAR source;
} STHEALTH_EVT_FILTER_1,*PSTHEALTH_EVT_FILTER_1;

typedef struct _STHEALTH_EVT_FILTER_2
{
 ULONG cbLen;
 UCHAR sourceNumber;
} STHEALTH_EVT_FILTER_2,*PSTHEALTH_EVT_FILTER_2;

typedef struct _STHEALTH_EVT_FILTER_3
{
 ULONG cbLen;
 UCHAR source;
 UCHAR sourceNumber;
} STHEALTH_EVT_FILTER_3,*PSTHEALTH_EVT_FILTER_3;

typedef struct _STHEALTH_EVT_FILTER_4
{
 ULONG cbLen;
 ULONG actionFlags;
} STHEALTH_EVT_FILTER_4,*PSTHEALTH_EVT_FILTER_4;

typedef struct _STHEALTH_EVT_FILTER_5
{
 ULONG cbLen;
 ULONG actionState;
} STHEALTH_EVT_FILTER_5,*PSTHEALTH_EVT_FILTER_5;

typedef struct _STHEALTH_EVT_FILTER_101
{
 ULONG cbLen;
 UCHAR source;
 UCHAR sourceNumber;
 ULONG actionFlags;
} STHEALTH_EVT_FILTER_101,*PSTHEALTH_EVT_FILTER_101;

typedef struct _STHEALTH_EVT_FILTER_102
{
 ULONG cbLen;
 UCHAR source;
 UCHAR sourceNumber;
 ULONG actionState;
} STHEALTH_EVT_FILTER_102,*PSTHEALTH_EVT_FILTER_102;

typedef struct _STHEALTH_EVT_FILTER_103
{
 ULONG cbLen;
 UCHAR source;
 UCHAR sourceNumber;
 ULONG actionFlags;
 ULONG actionState;
} STHEALTH_EVT_FILTER_103,*PSTHEALTH_EVT_FILTER_103;

/******************************************************************************/
/* Function definitions                                                       */
/******************************************************************************/
/* version */
int _System StHealthDLLVersion(void);

/* presence check */
BOOL _System StHealthIsPresent(void);

/* detect chips */
BOOL _System StHealthDetectChip(void);

/* chip type string */
char * _System StHealthChipType(char *buffer,unsigned long bufferLen);
char * _System StHealthLibAutor(char *b1,unsigned long b1Len
                       ,char *b2,unsigned long b2Len);

/* chip id */
int _System StHealthChipId(void);

/* temperatures */
double _System StHealthTemp(int nr,BOOL forceUpdate);

/* fan speeds */
long _System StHealthFan(int nr,BOOL forceUpdate);

/* voltage querying */
double _System StHealthVoltage(int nr,BOOL forceUpdate);

/* misc functions */
int _System StHealthVendorId(void);

/* Corrections */
/* If the parameter cor is equal to STHELATH_CORRECTION_QUERY no value is set */
/* Only the current correction value is returned */
/* Note ! */
/* All correction values are stored in StHealth.INI located in the */
/* OS2 directory on the boot drive */

double _System StHealthSetTempCorrection(int nr,float cor);
long _System StHealthSetFanCorrection(int nr,int cor);
double _System StHealthSetVoltageCorrection(int nr,float cor);

/* Some helper functions */
/* Note: reserved must be set to zero !! */
int _System StHealthPowerOff(unsigned long reserved);

/* Event setup and handling */
int _System StHealthAddEvent(PSTHEALTH_EVENT pEvent,PULONG eventHandle);
int _System StHealthModifyEvent(ULONG eventHandle,PSTHEALTH_EVENT pEvent);
int _System StHealthRemoveEvent(ULONG eventHandle);
int _System StHealthEnumEventHandles(PULONG buffer,ULONG pBufferLen
                                     ,PULONG numEvents);
int _System StHealthFilterEvents(PULONG buffer,ULONG pBufferLen
                                 ,PULONG numEvents
                                 ,ULONG filterLevel
                                 ,PVOID pFilter);
int _System StHealthGetEvent(ULONG eventHandle,PSTHEALTH_EVENT pEvent);
ULONG _System StHealthEventHandleForName(PSZ name);
int _System StHealthCheckEvent(ULONG eventHandle,ULONG checkMode
                                         ,PULONG seconds);
int _System StHealthCheckAllEvents(void);
int _System StHealthWaitForEvent(ULONG eventHandle,ULONG ulTimeout);
int _System StHealthSetEventState(ULONG eventHandle,ULONG stateFlags,BOOL set);

/* Daemon registering */
int _System StHealthRegisterDaemon(BOOL force);
int _System StHealthUnregisterDaemon(void);

#pragma pack()
#ifdef __cplusplus
};
#endif

#endif /* STHEALTH_H_INCLUDED */
