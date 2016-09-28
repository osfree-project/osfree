/*DDK*************************************************************************/
/*                                                                           */
/* COPYRIGHT (C) IBM Corporation, 1992                                       */
/* COPYRIGHT (C) Microsoft Corporation, 1989                                 */
/* COPYRIGHT    Copyright (C) 1992 IBM Corporation                           */
/*                                                                           */
/*    The following IBM OS/2 source code is provided to you solely for       */
/*    the purpose of assisting you in your development of OS/2 device        */
/*    drivers. You may use this code in accordance with the IBM License      */
/*    Agreement provided in the IBM Developer Connection Device Driver       */
/*    Source Kit for OS/2. This Copyright statement may not be removed.      */
/*                                                                           */
/*****************************************************************************/
/*static char *SCCSID = "@(#)dskinit.h  6.3 92/03/06";*/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */

typedef struct _DDD_Parm_List { /* */

  USHORT        cache_parm_list;        /* addr of InitCache_Parameter List  */
  USHORT        disk_config_table;      /* addr of disk_configuration table  */
  USHORT        init_req_vec_table;     /* addr of IRQ_Vector_Table          */
  USHORT        cmd_line_args;          /* addr of Command Line Args         */
  USHORT        machine_config_table;   /* addr of Machine Config Info       */

} DDD_PARM_LIST, far *PDDD_PARM_LIST;

/*                                                                */
/*----------------------------------------------------------------*/
/* DiskCache Parameters                                           */
/*                                                                */
/* This structure is obsolete. It is included for compatibility   */
/* with 1.x base device drivers                                   */
/*----------------------------------------------------------------*/
/*                                                                */

typedef struct _Cacheinit_Input { /* */

  ULONG         cache_size;             /* size of caching memory           */
  ULONG         cache_addr;             /* phys addr of caching memory      */
  USHORT        cache_thresh;
  USHORT        cache_msg_id;           /* Msg ID returned by cache init    */
  USHORT        cache_handle;           /* handle for cache buffers         */

} CACHEINIT_INPUT, far *PCACHEINIT_INPUT;

/*                                                                */
/*----------------------------------------------------------------*/
/* Machine Configuration Info                                     */
/*                                                                */
/* This contains information gathered from System Initialization  */
/* about the machine OS/2 is running on.                          */
/*----------------------------------------------------------------*/
/*                                                                */

typedef struct _Machine_Config_Info { /* MCH */

  USHORT        Length;                 /* Length of table                  */
  USHORT        BusInfo;                /* Machine Bus Type                 */
  USHORT        CpuInfo;                /* Machine CPU Type                 */
  UCHAR         SubModel;               /* Machine Submodel ID              */
  UCHAR         Model;                  /* Machine Model ID                 */
  USHORT        ABIOSRevision;          /* Abios Revision Level             */
  USHORT        HardDriveCount;         /* # of BIOS Fixed Drives           */
  UCHAR         Reserved1[20];

} MACHINE_CONFIG_INFO, far *PMACHINE_CONFIG_INFO;


/*                 */
/*-----------------*/
/* CPU Info Values */
/*-----------------*/
#define CPUINFO_286     0
#define CPUINFO_386     1
#define CPUINFO_486     2
#define CPUINFO_PENTIUM 3

/*                 */
/*-----------------*/
/* Bus Info Values */
/*-----------------*/
#define BUSINFO_MCA             0x01
#define BUSINFO_EISA            0x02
#define BUSINFO_ABIOS_SUPPORTED 0x04  /* ABIOS is supported  */
#define BUSINFO_ABIOS_PRESENT   0x08  /* ABIOS is present    */
#define BUSINFO_PCI             0x10  /* PCI BIOS detected   */
#define BUSINFO_OEM_ABIOS       0x20  /* Mfr is OEM, valid if ABIOS supported */

/* We must define BUSINFO_ABIOS for compatibility */
#define BUSINFO_ABIOS            BUSINFO_ABIOS_SUPPORTED

#define BUSINFO_PENTIUM_CPU     0x40  /* Pentium CPU detected */
