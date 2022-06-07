/***************************************************************************
 *      Created 2005  eCo Software                                         *
 *                                                                         *
 *      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is        *
 *      sample code created by eCo Software. This sample code is not part  *
 *      of any standard or eCo Software product and is provided to you     *
 *      solely for the purpose of assisting you in the development of your *
 *      applications.  The code is provided "AS IS", without               *
 *      warranty of any kind. eCo Software shall not be liable for any     *
 *      damages arising out of your use of the sample code, even if they   *
 *      have been advised of the possibility of such damages.              *
 *-------------------------------------------------------------------------*/
#define INCL_DOSPROCESS
#include <os2.h>
#include "acpi.h"
#include "acpiapi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct Hid_Code_Info
{  char *pHid; char *Info;
};

struct Hid_Code_Info HidCodeInfo[] =
{

"CSC0000", "Crystal Semiconductor CS423x sound -- SB/WSS/OPL3 emulation",
"CSC0010", "Crystal Semiconductor CS423x sound -- control",
"CSC0001", "Crystal Semiconductor CS423x sound -- joystick",
"CSC0003", "Crystal Semiconductor CS423x sound -- MPU401",
"IBM3780", "IBM pointing device",
"IBM0071", "IBM infrared communications device",
"IBM3760", "IBM DSP",
"NSC6001", "National Semiconductor Serial Port with Fast IR",
"PNP0000", "AT Interrupt Controller",
"PNP0001", "EISA Interrupt Controller",
"PNP0002", "MCA Interrupt Controller",
"PNP0003", "APIC",
"PNP0004", "Cyrix SLiC MP Interrupt Controller",
"PNP0100", "AT Timer",
"PNP0101", "EISA Timer",
"PNP0102", "MCA Timer",
"PNP0200", "AT DMA Controller",
"PNP0201", "EISA DMA Controller",
"PNP0202", "MCA DMA Controller",
"PNP0300", "IBM PC/XT keyboard controller (83-key)",
"PNP0301", "IBM PC/AT keyboard controller (86-key)",
"PNP0302", "IBM PC/XT keyboard controller (84-key)",
"PNP0303", "IBM Enhanced (101/102-key, PS/2 mouse support)",
"PNP0304", "Olivetti Keyboard (83-key)",
"PNP0305", "Olivetti Keyboard (102-key)",
"PNP0306", "Olivetti Keyboard (86-key)",
"PNP0307", "Microsoft Windows(R) Keyboard",
"PNP0308", "General Input Device Emulation Interface (GIDEI) legacy",
"PNP0309", "Olivetti Keyboard (A101/102 key)",
"PNP030A", "AT&T 302 Keyboard",
"PNP0320", "Japanese 106-key keyboard A01",
"PNP0321", "Japanese 101-key keyboard",
"PNP0322", "Japanese AX keyboard",
"PNP0323", "Japanese 106-key keyboard 002/003",
"PNP0324", "Japanese 106-key keyboard 001",
"PNP0325", "Japanese Toshiba Desktop keyboard",
"PNP0326", "Japanese Toshiba Laptop keyboard",
"PNP0327", "Japanese Toshiba Notebook keyboard",
"PNP0340", "Korean 84-key keyboard",
"PNP0341", "Korean 86-key keyboard",
"PNP0342", "Korean Enhanced keyboard",
"PNP0343", "Korean Enhanced keyboard 101b",
"PNP0343", "Korean Enhanced keyboard 101c",
"PNP0344", "Korean Enhanced keyboard 103",
"PNP0400", "Standard LPT printer port",
"PNP0401", "ECP printer port",
"PNP0500", "Standard PC COM port",
"PNP0501", "16550A-compatible COM port",
"PNP0502", "Multiport serial device (non-intelligent 16550)",
"PNP0510", "Generic IRDA-compatible device",
"PNP0511", "Generic IRDA-compatible device",
"PNP0600", "Generic ESDI/IDE/ATA compatible hard disk controller",
"PNP0601", "Plus Hardcard II",
"PNP0602", "Plus Hardcard IIXL/EZ",
"PNP0603", "Generic IDE supporting Microsoft Device Bay Specification",
"PNP0700", "PC standard floppy disk controller",
"PNP0701", "Standard floppy controller supporting MS Device Bay Spec",
"PNP0802", "Microsoft Sound System or Compatible Device (obsolete)",
"PNP0900", "VGA Compatible",
"PNP0901", "Video Seven VRAM/VRAM II/1024i",
"PNP0902", "8514/A Compatible",
"PNP0903", "Trident VGA",
"PNP0904", "Cirrus Logic Laptop VGA",
"PNP0905", "Cirrus Logic VGA",
"PNP0906", "Tseng ET4000",
"PNP0907", "Western Digital VGA",
"PNP0908", "Western Digital Laptop VGA",
"PNP0909", "S3 Inc. 911/924",
"PNP090A", "ATI Ultra Pro/Plus (Mach 32)",
"PNP090B", "ATI Ultra (Mach 8)",
"PNP090C", "XGA Compatible",
"PNP090D", "ATI VGA Wonder",
"PNP090E", "Weitek P9000 Graphics Adapter",
"PNP090F", "Oak Technology VGA",
"PNP0910", "Compaq QVision",
"PNP0911", "XGA/2",
"PNP0912", "Tseng Labs W32/W32i/W32p",
"PNP0913", "S3 Inc. 801/928/964",
"PNP0914", "Cirrus Logic 5429/5434 (memory mapped)",
"PNP0915", "Compaq Advanced VGA (AVGA)",
"PNP0916", "ATI Ultra Pro Turbo (Mach64)",
"PNP0917", "Reserved by Microsoft",
"PNP0918", "Matrox MGA",
"PNP0919", "Compaq QVision 2000",
"PNP091A", "Tseng W128",
"PNP0930", "Chips & Technologies Super VGA",
"PNP0931", "Chips & Technologies Accelerator",
"PNP0940", "NCR 77c22e Super VGA",
"PNP0941", "NCR 77c32blt",
"PNP09FF", "Plug and Play Monitors (VESA DDC)",
"PNP0A00", "ISA Bus",
"PNP0A01", "EISA Bus",
"PNP0A02", "MCA Bus",
"PNP0A03", "PCI/PCI-X Host Bridge",
"PNP0A04", "VESA/VL Bus",
"PNP0A05", "Generic Container Device",
"PNP0A06", "Generic Container Device",
"PNP0A08", "PCI Express/PCI-X Mode-2 Host Bridge",
"PNP0800", "AT-style speaker sound",
"PNP0B00", "AT Real-Time Clock",
"PNP0C00", "Plug and Play BIOS (only created by the root enumerator)",
"PNP0C01", "System Board",
"PNP0C02", "Reserved Motherboard Resources",
"PNP0C03", "Plug and Play BIOS Event Notification Interrupt",
"PNP0C04", "Math Coprocessor",
"PNP0C05", "APM BIOS (Version independent)",
"PNP0C06", "Reserved for identification of early Plug and Play BIOS implementation.",
"PNP0C07", "Reserved for identification of early Plug and Play BIOS implementation.",
"PNP0C08", "ACPI system board hardware",
"PNP0C09", "ACPI Embedded Controller",
"PNP0C0A", "ACPI Control Method Battery",
"PNP0C0B", "ACPI Fan",
"PNP0c0c", "ACPI power button device",
"PNP0c0d", "ACPI lid device",
"PNP0c0e", "ACPI sleep button device",
"PNP0c0f", "PCI interrupt link device",
"PNP0c10", "ACPI system indicator device",
"PNP0c11", "ACPI thermal zone",
"PNP0c12", "Device Bay Controller",
"PNP0c13", "Plug and Play BIOS (used when ACPI mode cannot be used)",
"PNP0c80", "Memory Device",
"PNP0e00", "Intel 82365-Compatible PCMCIA Controller",
"PNP0e01", "Cirrus Logic CL-PD6720 PCMCIA Controller",
"PNP0e02", "VLSI VL82C146 PCMCIA Controller",
"PNP0e03", "Intel 82365-compatible CardBus controller",
"PNP0f00", "Microsoft Bus Mouse",
"PNP0f01", "Microsoft Serial Mouse",
"PNP0f02", "Microsoft InPort Mouse",
"PNP0f03", "Microsoft PS/2-style Mouse",
"PNP0f04", "Mouse Systems Mouse",
"PNP0f05", "Mouse Systems 3-Button Mouse (COM2)",
"PNP0f06", "Genius Mouse (COM1)",
"PNP0f07", "Genius Mouse (COM2)",
"PNP0f08", "Logitech Serial Mouse",
"PNP0f09", "Microsoft BallPoint Serial Mouse",
"PNP0f0a", "Microsoft Plug and Play Mouse",
"PNP0f0b", "Microsoft Plug and Play BallPoint Mouse",
"PNP0f0c", "Microsoft-compatible Serial Mouse",
"PNP0f0d", "Microsoft-compatible InPort-compatible Mouse",
"PNP0f0e", "Microsoft-compatible PS/2-style Mouse",
"PNP0f0f", "Microsoft-compatible Serial BallPoint-compatible Mouse",
"PNP0f10", "Texas Instruments QuickPort Mouse",
"PNP0f11", "Microsoft-compatible Bus Mouse",
"PNP0f12", "Logitech PS/2-style Mouse",
"PNP0f13", "PS/2 Port for PS/2-style Mice",
"PNP0f14", "Microsoft Kids Mouse",
"PNP0f15", "Logitech bus mouse",
"PNP0f16", "Logitech SWIFT device",
"PNP0f17", "Logitech-compatible serial mouse",
"PNP0f18", "Logitech-compatible bus mouse",
"PNP0f19", "Logitech-compatible PS/2-style Mouse",
"PNP0f1a", "Logitech-compatible SWIFT Device",
"PNP0f1b", "HP Omnibook Mouse",
"PNP0f1c", "Compaq LTE Trackball PS/2-style Mouse",
"PNP0f1d", "Compaq LTE Trackball Serial Mouse",
"PNP0f1e", "Microsoft Kids Trackball Mouse",
"PNP8001", "Novell/Anthem NE3200",
"PNP8004", "Compaq NE3200",
"PNP8006", "Intel EtherExpress/32",
"PNP8008", "HP EtherTwist EISA LAN Adapter/32 (HP27248A)",
"PNP8065", "Ungermann-Bass NIUps or NIUps/EOTP",
"PNP8072", "DEC (DE211) EtherWorks MC/TP",
"PNP8073", "DEC (DE212) EtherWorks MC/TP_BNC",
"PNP8078", "DCA 10 Mb MCA",
"PNP8074", "HP MC LAN Adapter/16 TP (PC27246)",
"PNP80c9", "IBM Token Ring",
"PNP80ca", "IBM Token Ring II",
"PNP80cb", "IBM Token Ring II/Short",
"PNP80cc", "IBM Token Ring 4/16Mbs",
"PNP80d3", "Novell/Anthem NE1000",
"PNP80d4", "Novell/Anthem NE2000",
"PNP80d5", "NE1000 Compatible",
"PNP80d6", "NE2000 Compatible",
"PNP80d7", "Novell/Anthem NE1500T",
"PNP80d8", "Novell/Anthem NE2100",
"PNP80dd", "SMC ARCNETPC",
"PNP80de", "SMC ARCNET PC100, PC200",
"PNP80df", "SMC ARCNET PC110, PC210, PC250",
"PNP80e0", "SMC ARCNET PC130/E",
"PNP80e1", "SMC ARCNET PC120, PC220, PC260",
"PNP80e2", "SMC ARCNET PC270/E",
"PNP80e5", "SMC ARCNET PC600W, PC650W",
"PNP80e7", "DEC DEPCA",
"PNP80e8", "DEC (DE100) EtherWorks LC",
"PNP80e9", "DEC (DE200) EtherWorks Turbo",
"PNP80ea", "DEC (DE101) EtherWorks LC/TP",
"PNP80eb", "DEC (DE201) EtherWorks Turbo/TP",
"PNP80ec", "DEC (DE202) EtherWorks Turbo/TP_BNC",
"PNP80ed", "DEC (DE102) EtherWorks LC/TP_BNC",
"PNP80ee", "DEC EE101 (Built-In)",
"PNP80ef", "DECpc 433 WS (Built-In)",
"PNP80f1", "3Com EtherLink Plus",
"PNP80f3", "3Com EtherLink II or IITP (8 or 16-bit)",
"PNP80f4", "3Com TokenLink",
"PNP80f6", "3Com EtherLink 16",
"PNP80f7", "3Com EtherLink III",
"PNP80f8", "3Com Generic Etherlink Plug and Play Device",
"PNP80fb", "Thomas Conrad TC6045",
"PNP80fc", "Thomas Conrad TC6042",
"PNP80fd", "Thomas Conrad TC6142",
"PNP80fe", "Thomas Conrad TC6145",
"PNP80ff", "Thomas Conrad TC6242",
"PNP8100", "Thomas Conrad TC6245",
"PNP8105", "DCA 10 MB",
"PNP8106", "DCA 10 MB Fiber Optic",
"PNP8107", "DCA 10 MB Twisted Pair",
"PNP8113", "Racal NI6510",
"PNP811c", "Ungermann-Bass NIUpc",
"PNP8120", "Ungermann-Bass NIUpc/EOTP",
"PNP8123", "SMC StarCard PLUS (WD/8003S)",
"PNP8124", "SMC StarCard PLUS With On Board Hub (WD/8003SH)",
"PNP8125", "SMC EtherCard PLUS (WD/8003E)",
"PNP8126", "SMC EtherCard PLUS With Boot ROM Socket (WD/8003EBT)",
"PNP8127", "SMC EtherCard PLUS With Boot ROM Socket (WD/8003EB)",
"PNP8128", "SMC EtherCard PLUS TP (WD/8003WT)",
"PNP812a", "SMC EtherCard PLUS 16 With Boot ROM Socket (WD/8013EBT)",
"PNP812d", "Intel EtherExpress 16 or 16TP",
"PNP812f", "Intel TokenExpress 16/4",
"PNP8130", "Intel TokenExpress MCA 16/4",
"PNP8132", "Intel EtherExpress 16 (MCA)",
"PNP8137", "Artisoft AE-1",
"PNP8138", "Artisoft AE-2 or AE-3",
"PNP8141", "Amplicard AC 210/XT",
"PNP8142", "Amplicard AC 210/AT",
"PNP814b", "Everex SpeedLink /PC16 (EV2027)",
"PNP8155", "HP PC LAN Adapter/8 TP (HP27245)",
"PNP8156", "HP PC LAN Adapter/16 TP (HP27247A)",
"PNP8157", "HP PC LAN Adapter/8 TL (HP27250)",
"PNP8158", "HP PC LAN Adapter/16 TP Plus (HP27247B)",
"PNP8159", "HP PC LAN Adapter/16 TL Plus (HP27252)",
"PNP815f", "National Semiconductor Ethernode *16AT",
"PNP8160", "National Semiconductor AT/LANTIC EtherNODE 16-AT3",
"PNP816a", "NCR Token-Ring 4 Mbs ISA",
"PNP816d", "NCR Token-Ring 16/4 Mbs ISA",
"PNP8191", "Olicom 16/4 Token-Ring Adapter",
"PNP81c3", "SMC EtherCard PLUS Elite (WD/8003EP)",
"PNP81c4", "SMC EtherCard PLUS 10T (WD/8003W)",
"PNP81c5", "SMC EtherCard PLUS Elite 16 (WD/8013EP)",
"PNP81c6", "SMC EtherCard PLUS Elite 16T (WD/8013W)",
"PNP81c7", "SMC EtherCard PLUS Elite 16 Combo (WD/8013EW or 8013EWC)",
"PNP81c8", "SMC EtherElite Ultra 16",
"PNP81e4", "Pure Data PDI9025-32 (Token Ring)",
"PNP81e6", "Pure Data PDI508+ (ArcNet)",
"PNP81e7", "Pure Data PDI516+ (ArcNet)",
"PNP81eb", "Proteon Token Ring (P1390)",
"PNP81ec", "Proteon Token Ring (P1392)",
"PNP81ed", "Proteon ISA Token Ring (1340)",
"PNP81ee", "Proteon ISA Token Ring (1342)",
"PNP81ef", "Proteon ISA Token Ring (1346)",
"PNP81f0", "Proteon ISA Token Ring (1347)",
"PNP81ff", "Cabletron E2000 Series DNI",
"PNP8200", "Cabletron E2100 Series DNI",
"PNP8209", "Zenith Data Systems Z-Note",
"PNP820a", "Zenith Data Systems NE2000-Compatible",
"PNP8213", "Xircom Pocket Ethernet II",
"PNP8214", "Xircom Pocket Ethernet I",
"PNP821d", "RadiSys EXM-10",
"PNP8227", "SMC 3000 Series",
"PNP8228", "SMC 91C2 controller",
"PNP8231", "Advanced Micro Devices AM2100/AM1500T",
"PNP8263", "Tulip NCC-16",
"PNP8277", "Exos 105",
"PNP828a", "Intel '595 based Ethernet",
"PNP828b", "TI2000-style Token Ring",
"PNP828c", "AMD PCNet Family cards",
"PNP828d", "AMD PCNet32 (VL version)",
"PNP8294", "IrDA Infrared NDIS driver (Microsoft-supplied)",
"PNP82bd", "IBM PCMCIA-NIC",
"PNP82c2", "Xircom CE10",
"PNP82c3", "Xircom CEM2",
"PNP8321", "DEC Ethernet (All Types)",
"PNP8323", "SMC EtherCard (All Types except 8013/A)",
"PNP8324", "ARCNET Compatible",
"PNP8326", "Thomas Conrad (All Arcnet Types)",
"PNP8327", "IBM Token Ring (All Types)",
"PNP8385", "Remote Network Access Driver",
"PNP8387", "RNA Point-to-point Protocol Driver",
"PNP8388", "Reserved for Microsoft Networking components",
"PNP8389", "Peer IrLAN infrared driver (Microsoft-supplied)",
"PNP8390", "Generic network adapter",
"PNPa002", "Future Domain 16-700 compatible controller",
"PNPa003", "Panasonic proprietary CD-ROM adapter (SBPro/SB16)",
"PNPa01b", "Trantor 128 SCSI Controller",
"PNPa01d", "Trantor T160 SCSI Controller",
"PNPa01e", "Trantor T338 Parallel SCSI controller",
"PNPa01f", "Trantor T348 Parallel SCSI controller",
"PNPa020", "Trantor Media Vision SCSI controller",
"PNPa022", "Always IN-2000 SCSI controller",
"PNPa02b", "Sony proprietary CD-ROM controller",
"PNPa02d", "Trantor T13b 8-bit SCSI controller",
"PNPa02f", "Trantor T358 Parallel SCSI controller",
"PNPa030", "Mitsumi LU-005 Single Speed CD-ROM controller + drive",
"PNPa031", "Mitsumi FX-001 Single Speed CD-ROM controller + drive",
"PNPa032", "Mitsumi FX-001 Double Speed CD-ROM controller + drive",
"PNPb000", "Sound Blaster 1.5 sound device",
"PNPb001", "Sound Blaster 2.0 sound device",
"PNPb002", "Sound Blaster Pro sound device",
"PNPb003", "Sound Blaster 16 sound device",
"PNPb004", "Thunderboard-compatible sound device",
"PNPb005", "Adlib-compatible FM synthesizer device",
"PNPb006", "MPU401 compatible",
"PNPb007", "Microsoft Windows Sound System-compatible sound device",
"PNPb008", "Compaq Business Audio",
"PNPb009", "Plug and Play Microsoft Windows Sound System Device",
"PNPb00a", "MediaVision Pro Audio Spectrum (Trantor SCSI enabled, Thunder Chip Disabled)",
"PNPb00b", "MediaVision Pro Audio 3D",
"PNPb00c", "MusicQuest MQX-32M",
"PNPb00d", "MediaVision Pro Audio Spectrum Basic (No Trantor SCSI, Thunder Chip Enabled)",
"PNPb00e", "MediaVision Pro Audio Spectrum (Trantor SCSI enabled, Thunder Chip Enabled)",
"PNPb00f", "MediaVision Jazz-16 chipset (OEM Versions)",
"PNPb010", "Auravision VxP500 chipset - Orchid Videola",
"PNPb018", "MediaVision Pro Audio Spectrum 8-bit",
"PNPb019", "MediaVision Pro Audio Spectrum Basic (no Trantor SCSI, Thunder chip Disabled)",
"PNPb020", "Yamaha OPL3-compatible FM synthesizer device",
"PNPb02f", "Joystick/Game port",
"PNPb000", "Compaq 14400 Modem (TBD)",
"PNPc001", "Compaq 2400/9600 Modem (TBD)",
"PNP0XXX", "Unknown System Device",
"PNP8XXX", "Unknown Network Adapter",
"PNPaXXX", "Unknown SCSI, Proprietary CD Adapter",
"PNPbXXX", "Unknown Multimedia Device",
"PNPcXXX", "Unknown Modem",
"ACPI0001", "SMBus 1.0 Host Controller",
"ACPI0002", "Smart Battery Subsystem",
"ACPI0003", "AC Device",
"ACPI0004", "Module Device",
"ACPI0005", "SMBus 2.0 Host Controller",
"ACPI0006", "GPE Block Device",
"ACPI0007", "Processor Device",
"ACPI0008", "Ambient Light Sensor Device",
"ACPI0009", "I/O APIC/SAPIC Device",
"ACPI000A", "I/O APIC Device",
"ACPI000B", "I/O SAPIC Device"

};

typedef struct _GpeDevice_
{
    ACPI_HANDLE Device;
    UINT32      GpeNumber;
    UINT32      SxDState;
    char        PnPid[8];
    char        *PnPidInfo;
} GPEDEVICE, *PGPEDEVICE;

typedef struct _GPEEvent_
{
    UINT8       GpeUsed;
    UINT8       GpeTriggered;
    UINT8       WakeRintime;
    ACPI_HANDLE GpeNdl;
} GPEEVENT, *PGEPEEVENT;

GPEEVENT     Event[256];
GPEDEVICE    Device[256];
UINT32       DeviceCount = 0;;
ACPI_OBJECT  Object[20];
/**
* Callback for GPE scanning
* @ObjHandle    handle to GPE object
* @NestingLevel nesting level
* @Context      context
* @ReturnValue  return value
* @return       ACPI status
*/
ACPI_STATUS
GetGpeNotify(
    ACPI_HANDLE ObjHandle,
    UINT32 NestingLevel,
    void *Context,
    void **ReturnValue)
{
    ACPI_BUFFER Buffer;
    char        DevName[5];
    ULONG       GpeNumber, GpeTriggered, i, Flag;

    Buffer.Length  = 5;
    Buffer.Pointer = DevName;
    AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    DevName[5]     = 0;

    if ((DevName[1] == 'L') || (DevName[1] == 'l'))
    {
         GpeTriggered = ACPI_GPE_LEVEL_TRIGGERED;
    }

    if ((DevName[1] == 'E') || (DevName[1] == 'e'))
    {
         GpeTriggered = ACPI_GPE_EDGE_TRIGGERED;
    }

    DevName[0] = '0';
    DevName[1] = 'X';
    GpeNumber  = strtoul(DevName,NULL,0);
    Event[GpeNumber].GpeNdl       = ObjHandle;
    Event[GpeNumber].GpeUsed      = 1;
    Event[GpeNumber].GpeTriggered = GpeTriggered;
    printf("GPE0x%2.2X triggered by %s \n",GpeNumber,
                                        Event[GpeNumber].GpeTriggered == ACPI_GPE_EDGE_TRIGGERED ? "Edge" : "Level"
          );

    Flag = 0;
    for (i = 0; i < DeviceCount; i++)
    {
         if (Device[i].GpeNumber != GpeNumber)
            continue;
         if (!Flag)
         {
             printf("        Using by device:\n\n");
             Flag++;
         }
         Buffer.Length  = 5;
         Buffer.Pointer = DevName;
         AcpiTkGetName(Device[i].Device, ACPI_SINGLE_NAME, &Buffer);
         DevName[5]     = 0;
         printf("        [%s] S%dD %s %s\n",DevName, Device[i].SxDState,Device[i].PnPid, Device[i].PnPidInfo == NULL? "" :Device[i].PnPidInfo);
    }
    if (Flag)
       printf("\n");

    return(AE_OK);
}
/*
 * CallBack function for _PSW
 */
ACPI_STATUS
GetGpeDevice(
    ACPI_HANDLE ObjHandle,
    UINT32      NestingLevel,
    void        *Context,
    void        **ReturnValue)
{
    char        DevName[5];
    ACPI_BUFFER Results, Buffer;
    ACPI_OBJECT       *Obj;
    ACPI_STATUS     Status;
    ULONG           i,ndev;
    ACPI_DEVICE_INFO *DevInfo = NULL;

    Results.Length  = sizeof(Object);
    Results.Pointer = Object;

    Status = AcpiTkEvaluateObject(ObjHandle,"_PRW",NULL,&Results);
    if (Status != AE_OK)
        return AE_OK;

    Buffer.Length  = 5;
    Buffer.Pointer = DevName;
    AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    DevName[5]     = 0;

    Obj = (ACPI_OBJECT *)Object;
    if ((Obj->Type == ACPI_TYPE_PACKAGE) && (Obj->Package.Count == 2))
    {
        Device[DeviceCount].Device    = ObjHandle;
        Device[DeviceCount].GpeNumber = Obj->Package.Elements[0].Integer.Value;
        Device[DeviceCount].SxDState  = Obj->Package.Elements[1].Integer.Value;
        Device[DeviceCount].PnPidInfo = NULL;
        Device[DeviceCount].PnPid[0]  = '\0';

        Status         = AcpiTkGetObjectInfoAlloc3(ObjHandle, &DevInfo);
        if (Status)
        {
            DeviceCount++;
            return AE_OK;
        }
        sprintf(Device[DeviceCount].PnPid,"%7.7s",&DevInfo->HardwareId);
        ndev = sizeof(HidCodeInfo)/sizeof(struct Hid_Code_Info);
        for (i = 0; i < ndev; i++)
        {
             if (!stricmp((char *)&(DevInfo->HardwareId), HidCodeInfo[i].pHid) )
             {
                 Device[DeviceCount].PnPidInfo = HidCodeInfo[i].Info;
                 break;
             }
        }
        DeviceCount++;
    }
    if (DevInfo)
    {
        AcpiTkOsFree(DevInfo);
    }
    return AE_OK;
}
int
main(
    void)
{
    ACPI_STATUS     Status;
    ACPI_HANDLE     GpeHdl;
    ACPI_TK_VERSION ApiBuffer;


    if (AcpiTkValidateVersion(ACPI_TK_VERSION_MAJOR, ACPI_TK_VERSION_MINOR)) {
        printf("This program is not compatible with the version of ACPI that is installed.\n");

        /* display the version information */
        ApiBuffer.Size = sizeof(ACPI_TK_VERSION);
        if (AcpiTkGetVersion(&ApiBuffer)) exit(1);
        if (ApiBuffer.Size != sizeof(ACPI_TK_VERSION)) exit(1);

        printf("The version of ACPI installed is %u.%u\n", ApiBuffer.PSD.Major, ApiBuffer.PSD.Minor);
        printf("The API version is %u.%u, but version %u.%u is needed.\n",
            ApiBuffer.Api.Major, ApiBuffer.Api.Minor, ACPI_TK_VERSION_MAJOR, ACPI_TK_VERSION_MINOR);
    }

    Status = AcpiTkWalkNamespace(ACPI_TYPE_DEVICE, // All events
                               ACPI_ROOT_OBJECT, // Start object
                               ACPI_UINT32_MAX,  // Max depth
                               GetGpeDevice,     // Callback
                               NULL, NULL);      // Callback param and poitner to return value

    memset (Event ,0 ,sizeof (GPEEVENT) * 256);
    Status = AcpiTkGetHandle (ACPI_ROOT_OBJECT,"_GPE",&GpeHdl);
    if (Status == AE_OK)
    {
        Status = AcpiTkWalkNamespace(ACPI_TYPE_ANY,    // All events
                                   GpeHdl,           // Start object
                                   ACPI_UINT32_MAX,  // Max depth
                                   GetGpeNotify,     // Callback
                                   &GpeHdl, NULL);   // Callback param and poitner to return value
        printf("EndScan GPE Status %x\n",Status);
    }
    else
    {
        printf("Get GpeHdl Status = 0x%x\n",Status);
        return 1;
    }

    return 0;
}
