/*static char *SCCSID = "@(#)devcmd.h   6.2 92/05/08";*/

/*
 *      Copyright (c) IBM Corporation 1987, 1989
 *      Copyright (c) Microsoft Corporation 1987, 1989
 *
 *      All Rights Reserved
 */

/*
   device commands
*/

#define CMDInit                 0       /* INIT command */
#define CMDMedChk               1       /* Media Check */
#define CMDBldBPB               2       /* build BPB */
#define CMDIOCTLR               3       /* reserved for 3.x compatability */
#define CMDINPUT                4       /* read data from device */
#define CMDNDR                  5       /* non-destructive read */
#define CMDInputS               6       /* input status */
#define CMDInputF               7       /* input flush */
#define CMDOUTPUT               8       /* write data to device */
#define CMDOUTPUTV              9       /* write data and verify */
#define CMDOutputS              10      /* output status */
#define CMDOutputF              11      /* output flush */
#define CMDIOCTLW               12      /* reserved for 3.x compatability */
#define CMDOpen                 13      /* device open */
#define CMDClose                14      /* device close */
#define CMDRemMed               15      /* is media removable */
#define CMDGenIOCTL             16      /* Generic IOCTL */
#define CMDResMed               17      /* reset media uncertain */
#define CMDGetLogMap            18
#define CMDSetLogMap            19
#define CMDDeInstall            20      /* De-Install driver */
#define CMDPartfixeddisks       22      /* Partitionable Fixed Disks */
#define CMDGetfd_logunitsmap    23      /* Get Fixed Disk/Logical Unit Map */
#define CMDInputBypass          24      /* cache bypass read data */
#define CMDOutputBypass         25      /* cache bypass write data */
#define CMDOutputBypassV        26      /* cache bypass write data and verify */
#define CMDInitBase             27      /* INIT command for base DDs */
#define CMDShutdown             28
#define CMDGetDevSupport        29      /* query for extended capability */
#define CMDInitComplete         31      /* Init complete for all DD's    */
#define CMDSaveRestore          32
#define CMDAddOnPrep            97      /* Prepare for add on */
#define CMDStar                 98      /* start console output */
#define CMDStop                 99      /* stop console output */
