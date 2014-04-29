/*static char *SCCSID = "@(#)devhdr.h   6.5 91/11/09";*/

/*
 * Device Table Record
 *
 * Devices are described by a chain of these records
 */

 /* If someone ever changes the following define of DEV_CBNAME, the
  * length of the string in the ASM block within the SysDev struct
  * should be changed to the same length.
  */
#define DEV_CBNAME      8

struct SysDev {
    unsigned long SDevNext;     /* Pointer to next device header */
    unsigned short SDevAtt;     /* Attributes of the device */
    unsigned short SDevStrat;   /* Strategy entry point */
    unsigned short SDevInt;     /* Interrupt entry point */
/* XLATOFF */
    unsigned char  SDevName[DEV_CBNAME]; /* name (block uses only 1st byte) */
/* XLATON */
/* ASM
SDevName        DB      "        "
*/
    unsigned short SDevProtCS;  /* Protect-mode CS of strategy entry pt */
    unsigned short SDevProtDS;  /* Protect-mode DS */
    unsigned short SDevRealCS;  /* Real-mode CS of strategy entry pt */
    unsigned short SDevRealDS;  /* Real-mode DS */
};

/*
 * Device driver header for OS/2 2.0 drivers ; these are at level 3 or above
 * These drivers have an extra DWORD (bitmap) of capabilities
 * Pre-OS/2 2.0 drivers are assumed to have a bitmap of value 0
 */
struct  SysDev3 {
    struct SysDev SysDevBef3;
    unsigned long  SDevCaps;    /* bit map of DD /MM restrictions */
};

/*
 * SDevCaps bitmap definitions
 */
#define DEV_IOCTL2       0x0001 /* DD can handle dev ioctl2           */
#define DEV_16MB         0x0002 /* DD can handle phys.addresses >16MB */
#define DEV_PARALLEL     0x0004 /* DD handles parallel port           */
#define DEV_ADAPTER_DD   0x0008 /* DD supports Adapter Dev Driver Intf*/
#define DEV_INITCOMPLETE 0x0010 /* DD can handle CMDInitComplete      */

#define DEV_SAVERESTORE  0x0020

/*
 * Device Driver Type defininitions
 */

#define DEV_CIN         0x0001  /*  0  2 5 Device is console in */
#define DEV_COUT        0x0002  /*  1  2 5 Device is console out */
#define DEV_NULL        0x0004  /*  2  2 5 Device is the Null device */
#define DEV_CLOCK       0x0008  /*  3  2 5 Device is the clock device */
#define DEV_SPEC        0x0010  /*  4  2   Devices can support INT 29h */
#define DEV_ADD_ON      0x0020  /*  5      Device is add-on driver (BWS) */
#define DEV_GIOCTL      0x0040  /*  6  3   Device supports generic ioctl */
#define DEV_FCNLEV      0x0380  /*  9-7  5 Device function level */
/*                      0x0400  /* 10 */
#define DEV_30          0x0800  /* 11  2 5 Accepts Open/Close/Removable Media */
#define DEV_SHARE       0x1000  /* 12      Device wants FS sharing checking */
#define DEV_NON_IBM     0x2000  /* 13  2 5 Device is a non IBM device. */
#define DEV_IOCTL       0x4000  /* 14  2   Device accepts IOCTL request */
#define DEV_CHAR_DEV    0x8000  /* 15  2 5 Device is a character device */


/*
 * ***** BUGBUG *****
 *
 * DOS 3.2 Screws us as it uses bit 6 to indicate generic ioctl support
 * for the time being those device drivers are SOL.
 */

/* Level definitions for devices */

#define DEVLEV_0    0x0000      /* DOS 3.0 and before (NEEDS TO BE FIXED) */
#define DEVLEV_1    0x0080      /* DOS 5.0 */
#define DEVLEV_2    0x0100      /* OS/2 v1.2 (new gen ioctl iface) */
#define DEVLEV_3    0x0180      /* OS/2 v2.0 (support of memory above 16MB) */

/***    Dev_FuncLev - Compare device driver level to specified level
 *
 *      Dev_FuncLev masks off extra bits of the device driver attribute word
 *      and compares the driver level to the specified level
 *
 *      ENTRY   ReqLev  = Requested Level (register or value)
 *              DevAttr = Register with Device Driver Attribute word
 *
 *      EXIT    'C' if level is less than the specified level
 *                  (DevAttr < ReqLev)
 *
 *              'NC'if driver level is equal or greater to specified level
 *                  (DevAttr >= ReqLev)
 *
 *              (Use JC  or JB  to jump if DevAttr <  ReqLev)
 *              (Use JNC or JAE to jump if DevAttr >= ReqLev)
 */

/* ASM
Dev_FuncLev     macro   ReqLev, DevAttr
        and     DevAttr,DEV_FCNLEV          ; mask off extra bits of dev attributes
        cmp     DevAttr,ReqLev              ; compare with specified level
                                            ; carry -> level < specified
endm
*/
