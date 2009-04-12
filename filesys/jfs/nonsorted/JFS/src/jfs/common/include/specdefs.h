
/* Defines not found in the DDK headers */

#pragma pack(1)
/*----------------------------------------------------------*/
/* Category 8, function 69H, OS2LVM IOCtl                   */
/*----------------------------------------------------------*/

typedef struct _DDI_OS2LVM_param
{
  UCHAR       Command;
  UCHAR       DriveUnit;
  USHORT      TableNumber;
  ULONG       LSN;
  ULONG       Feature_ID;
  ULONG       Flags;
  ULONG       CurrentInstance;
  ULONG       StartingLSN;
  ULONG       EndingLSN;
} DDI_OS2LVM_param, FAR *PDDI_OS2LVM_param;

typedef struct _DDI_OS2LVM_data
{
  UCHAR        ReturnData;
  void*        UserBuffer;
} DDI_OS2LVM_data, FAR *PDDI_OS2LVM_data;

// Parameter Packet Flags field bit definitions
#define First_Visit_To_Feature          0x00000001

// Command values ...
#define IDENTIFY_VOLUME  0
#define ENABLE_BBR       0x01
#define DISABLE_BBR      0x02
#define GET_BBR_INFO     0x03
#define GET_TBL_SIZE     0x04
#define GET_SECT_LIST    0x05
#define GET_RELO_DATA    0x06
#define REMOVE_TBL_ENTRY 0x07
#define CLEAR_TBL        0x08
#define GET_DRIVE_NAME   0x09

#define ENABLE_VERIFY    0x0A
#define DISABLE_VERIFY   0x0B
#define QUERY_VERIFY     0x0C

#define GET_OS2LVM_VIEW  0x0D
#define SET_PRM_REDISCOVERY_STATE  0x0E
#define GENERIC_FEATURE_COMMAND    0x0F


// ReturnData values ...
#define  BBR_SUCCESS     0x00     // Successful completion
#define  BBR_FAILURE     0x01     // Failed completion
#define  BBR_VERIFY_OFF  0x00     // Write Verify Inactive
#define  BBR_VERIFY_ON   0x01     // Write Verify Active


// Returned data for OS2LVM IOCtl Cat 8 Func 69
// Command 3 = Get Bad Block Information
typedef struct _BadBlockInfo
{
   USHORT         TotalRelocations;      // Total relocations active for this partition/volume.
   USHORT         TotalTables;           // Total tables in use for this partition/volume.
} BadBlockInfo, FAR *PBadBlockInfo;

// Returned data for OS2LVM IOCtl Cat 8 Func 69
// Command 4 = Get Table Size
typedef struct _BadBlockTableInfo
{
   USHORT         ActiveRelocations;      // Total relocations active for this partition.
   USHORT         MaxRelocationsAllowed;  // Max relocations allowed for this partition.
} BadBlockTableInfo, FAR *PBadBlockTableInfo;

#define IODC_LV  0x69             /* Logical Volume Management */
#define IODC_DD  0x6A             /* Dynamic Drive Delete      */

#pragma pack()
