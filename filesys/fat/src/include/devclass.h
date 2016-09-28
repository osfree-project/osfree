/*static char *SCCSID = "@(#)devclass.h	6.2 92/05/08";*/

/*
 *      Copyright (c) IBM Corporation 1987, 1989
 *
 *      All Rights Reserved
 */

/*
 * Device Class Structure -  returned by dh_GetDOSVar when
 * AL=DHGETDOSV_DEVICECLASSTABLE and CX = device_class
 *
 */


#define  MAXDEVCLASSNAMELEN 16     /*   Maximum length of the DevClass Name */
#define  MAXDEVCLASSTABLES 2       /*   Maximum number of DevClass tables   */

#define  MAXDISKDCENTRIES 32   /*  Maximum number of entries in DISK table */
#define  MAXMOUSEDCENTRIES 3   /*  Maximum number of entries in Mouse table */

/* structures for the DeviceClassTable  */

struct DevClassTableEntry {
        USHORT   DCOffset;
        USHORT   DCSelector;
        USHORT   DCFlags;
        UCHAR    DCName[MAXDEVCLASSNAMELEN];
};


struct DevClassTableStruc {
   USHORT                     DCCount;
   USHORT                     DCMaxCount;
   struct DevClassTableEntry  DCTableEntries[1];
};




