:userdoc.
:docprof toc=123456.
:title.OS/2 Installable File Systems

:h1 id=0.Cover page
:link reftype=hd refid=1.:link reftype=hd refid=2.  
:h1 id=1 hide.os2logo
:artwork name='img0.bmp' align=center. :artwork name='img1.bmp' align=center.   
:h1 id=2 hide.titlep
:font facename='Helv' size=24x24.
:lines align=center.
:hp2.Installable File Systems :ehp2.
.br 

.br 
:font facename=default.For OS/2 Version 2&per.0
.br 
OS/2 File Systems Department 
.br 

.br 
PSPC Boca Raton, Florida 
.br 

.br 

.br 

.br 
Translated to IPF by Andre Asselin 
.br 

.br 
Version 0&per.1
.br 
July 1993 
.br 

:elines.
:font facename=default.
:h1 id=3.Disclaimer

:p.This document includes a written description of the Installable File System 
Driver Interface for IBM OS/2 Standard Edition Version 2&per.0&per.   
:p.INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THIS DOCUMENT &osq.AS IS
&osq. WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
PURPOSE&per.   
:p.IN NO EVENT WILL IBM BE LIABLE FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO 
ANY LOST PROFITS, LOST SAVINGS OR ANY INCIDENTAL OR CONSEQUENTIAL DAMAGES, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGES, OR FOR ANY CLAIM BY YOU BASED ON A THIRD 
PARTY CLAIM&per.   
:p.Some or all of the interfaces described in this document are unpublished&per. 
IBM reserves the right to change or delete them in future versions of OS/2, at IBM
&apos.s sole discretion, without notice to you&per. IBM does not guarantee that 
compatibility of your applications can or will be maintained with future versions of OS/2
&per.   
:p.This document could include technical inaccuracies or typographical errors
&per. It is possible that this document may contain reference to, or information 
about, IBM products, programming or services that are not announced in your country
&per. Such references or information must not be construed to mean that IBM intends 
to announce such IBM products, programming or services in your country&per. IBM 
may have patents or pending patent applications covering subject matter in this 
document&per. The furnishing of this document does not give you any license to these 
patents&per. You can send license inquiries, in writing, to the IBM Director of 
Commercial Relations, IBM Corporation, Armonk NY 10504&per.   
:p.Copyright International Business Machines Corporation 1991&per. All rights 
reserved&per. Note to U&per.S&per. Government Users - Documentation related to 
restricted rights - Use, duplication or disclosure is subject to restrictions set forth in 
GSA ADP Schedule Contract with IBM Corp&per.   
:h1 id=4.Changes

:p.This document is based on the February 17, 1992 version of the printed IFS 
documentation&per. 
:p.July, 1993   
:h1 id=5.Installable File System Mechanism
The OS/2 Installable File System (IFS) Mechanism supports the following&colon.   
:p.oCoexistence of multiple, active file systems in a single PC 
.br 
oMultiple logical volumes (partitions) 
.br 
oMultiple and different storage devices 
.br 
oRedirection or connection to remote file systems 
.br 
oFile system flexibility in managing its data and I/O for optimal performance 
.br 
oTransparency at both the user and application level 
.br 
oStandard set of File I/O API 
.br 
oExisting logical file and directory structure 
.br 
oExisting naming conventions 
.br 
oFile system doing its own buffer management 
.br 
oFile system doing file I/O without intermediate buffering 
.br 
oExtensions to the Standard File I/O API (FSCTL) 
.br 
oExtensions to the existing naming conventions 
.br 
oIOCTL type of communication between a file system and a device driver 
.br 
  
:h2 id=6.Installable File System Overview
  
:h3 id=7.System Relationships

:p.Installable File System (IFS) Mechanism defines the relationships among the 
operating system, the file systems, and the device drivers&per. The basic model of the 
system is represented in Figure 1-1&per. 
:cgraphic.
:font facename='Courier' size=12x12.  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³                                               ³
  ³          File System Request Router           ³
  ³                                               ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿         ³
                                        ³         ³
  ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿³         ³
  ³  File  ³    ³  File  ³    ³  File  ³³         ³
  ³ System ³    ³ System ³    ³ System ³³         ³
  ³        ³    ³        ³    ³        ³³         ³
  ³ LOCAL  ³    ³  NET   ³    ³  NET   ³³         ³
  ³        ³    ³ REDIR1 ³    ³ REDIR2 ³³         ³
  ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ³         ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ         ³
  ³                                               ³
  ³      FS Helper Routines/                      ³
  ³      Device Driver Request Router             ³
  ³                                               ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿         ³
                                        ³         ³
  ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿³         ³
  ³        ³    ³        ³    ³        ³³         ³
  ³ Device ³    ³ Device ³    ³ Device ³³         ³
  ³ Driver ³    ³ Driver ³    ³ Driver ³³         ³
  ³        ³    ³        ³    ³        ³³         ³
  ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ³         ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ         ³
  ³                                               ³
  ³       Device Driver Helper Routines           ³
  ³                                               ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 1-1&per. System relationships for Installable File Systems :ehp2.  
:p.The file system request router directs file system function calls to the 
appropriate file system for processing&per.   
:p.The file systems manage file I/O and control the format of information on the 
storage media&per. An installable file system (FS) will be referred to as a file system 
driver (FSD)&per.   
:p.The FS Helper Routines provide a variety of services to the file systems&per.   
:p.The device drivers manage physical I/O with devices&per. Device drivers do 
not understand the format of information on the media&per.   
:h3 id=8.File I/O API

:p.Standard file I/O is performed through the Standard File I/O API&per. The 
application makes a function call and the file system request router passes the request to 
the correct file system for processing&per. See Figure 1-2&per.   
:cgraphic.
:font facename='Courier' size=12x12.                     ÚÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                     ³ application ³
                     ÀÄÄÄÄÄÄÂÄÄÄÄÄÄÙ
                            
              ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿  Dynamic
              ³ Standard File I/O API   ³  Link
              ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÙ  Library
                            
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³          File System Request Router          ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        ³
                                        ³        ³
  ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿³        ³
  ³ File   ³    ³  File  ³    ³  File  ³³        ³
  ³ System ³    ³ System ³    ³ System ³³        ³
  ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ³        ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        ³
  ³                                              ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        ³
                                        ³        ³
  ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿³        ³
  ³ Device ³    ³ Device ³    ³ Device ³³        ³
  ³ Driver ³    ³ Driver ³    ³ Driver ³³        ³
  ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ³        ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        ³
  ³                                              ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 1-2&per. Standard File I/O :ehp2.  
:p.New API may be provided by a file system to implement functions specific to 
the file system or not supplied through the standard file I/O interface&per. New 
API are provided in a dynamic link library that uses the DosFSCtl standard function 
call to communicate with the specific file system (FSD)&per. See Figure 1-3&per.   
:cgraphic.
:font facename='Courier' size=12x12.              ÚÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
              ³ application ³
              ÀÄÄÄÄÄÄÂÄÄÄÄÄÄÙ
                     ³
                     
         ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
         ³ Extended File I/O API   ³ Dynamic Link Library
         ³  for File System X      ³
         ÀÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                     ³
                     
         ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
         ³       DosFsCtl          ³  Standard File I/O API
         ÀÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
                     ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ³ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³                  ³                           ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ³ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        ³
                                       ³        ³
  ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿³        ³
  ³  File  ³    ³  File  ³    ³  File  ³³        ³
  ³ System ³    ³ System ³    ³ System ³³        ³
  ³        ³    ³    X   ³    ³        ³³        ³
  ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ³        ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        ³
  ³                                              ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        ³
                                        ³        ³
  ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿    ÚÄÄÄÄÄÄÄÄ¿³        ³
  ³ Device ³    ³ Device ³    ³ Device ³³        ³
  ³ Driver ³    ³ Driver ³    ³ Driver ³³        ³
  ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ    ÀÄÄÄÄÄÄÄÄÙ³        ³
  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        ³
  ³                                              ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 1-3&per. Extended File I/O :ehp2.  
:h3 id=9.Buffer Management

:p.In 2&per.0 the FAT buffer management helpers were removed because of lack of 
use by any 1&per.x FSD&per. FSDs should handle all buffer/cache management 
themselves&per.   
:p.The FSD moves all data requiring partial sector I/O between the application
&apos.s buffers and its cache buffers&per. The FS helper routines initiate the I/O for 
local file systems&per.   
:h3 id=10.Volume Management

:p.Volume management (that is, detecting when the wrong volume is mounted and 
notifying the operator to take corrective action) is handled directly through OS/2 and 
the device driver&per. Each FSD is responsible for generating a volume label and 32
-bit volume serial number&per. These are stored in a reserved location in logical 
sector zero at format time&per. Because an FSD is the only system component to touch 
this information, an FSD is not required to store it in a particular format&per. OS/
2 calls the FSD to perform operations that might involve it&per. The FSD is 
required to update the volume parameter block (VPB) whenever the volume label or serial 
number is changed&per.   
:p.When the FSD passes an I/O request to an FS helper routine, the FSD passes 
the 32-bit volume serial number and the user&apos.s volume label (through the VPB)
&per. When the I/O is performed, OS/2 compares the requested volume serial number 
with the current volume serial number it maintains for the device&per. This is an in
-storage test (no I/O required) performed by checking the drive parameter block
&apos.s (DPB) VPB of the volume mounted on the drive&per. If unequal, OS/2 signals the 
critical error handler to prompt the user to insert the volume having the serial number 
and label specified&per.   
:p.When OS/2 detects a media change in a drive, or the first time a volume is 
accessed, OS/2 determines which FSD is responsible for managing I/O to that volume&per. 
OS/2 allocates a VPB and polls the installed FSDs (by calling the FS_MOUNT entry 
point) until an FSD indicates that it does recognize the media&per.   
:p.:hp2.Note&colon.  :ehp2.The FAT FSD is the last in the list of installed FSDs and acts as 
the default FSD when no other FSD recognition takes place&per.   
:h3 id=11.Connectivity

:p.There are two classes of file system drivers&colon.   
:p.oFSDs that use a block device driver to do I/O to a local or remote device
&per. These are called local file systems&per. 
.br 
oFSDs that access a remote system without a block device driver&per. These are 
called remote file systems&per. 
.br 

:p.The connection between a drive letter and a remote file system is achieved 
through a command interface provided with the FSD (FS_ATTACH)&per.   
:p.When a local volume is first accessed, OS/2 sequentially asks each installed 
FSD to accept the media, by calling each FSD&apos.s FS_MOUNT entry point&per. If no 
FSD accepts the media, it is then assigned to the default FAT file system&per. Any 
further attempt that is made to access an unrecognized media, other than by FORMAT, 
results in an &apos.Invalid media format&apos. message&per.   
:p.When a volume has been recognized, the relationship between drive, FSD, 
volume serial number, and volume label is remembered&per. The volume serial number and 
label are stored in the volume parameter block (VPB)&per. The VPB is maintained by OS
/2 for open files (I/O based on file-handles), searches, and buffer references
&per. The VPB represents the media&per.   
:p.Subsequent requests for a volume that has been removed require polling the 
installed FSDs for volume recognition by calling FS_MOUNT&per. The volume serial number 
and volume label of the VPB returned by the recognizing FSD and the existing VPB 
are compared&per. If the test fails, OS/2 signals the critical error handler to 
prompt the user for the correct volume&per.   
:p.The connection between media and VPB is remembered until all open files on 
the volume are closed and search and cache buffer references are removed&per. Only 
volume changes cause a redetermination of the media at the time of next access&per.   
:h3 id=12.IPL Mechanism

:p.A primary DOS disk partition (type 1, 4, or 6) may be used to boot the system
&per. The code for FSDs may reside in any partition readable by a previously 
installed FSD&per. An IFS partition must be a type 7 partition&per.   
:p.The OS/2 boot volume includes the following&colon. Bootrecord and basic file 
system&per. The root directory of this volume will contain a mini-file system in 
OS2BOOT, a kernel loader in OS2LDR, the OS/2 kernel in OS2KRNL, and the CONFIG&per.SYS 
file&per.   
:p.Device drivers and FSDs are loaded in the order they appear in CONFIG&per.SYS 
and are considered elements of the same ordered set&per. Therefore, both device 
drivers and FSDs may be loaded from installed file systems as long as they are started 
in the proper order&per. For example&colon.   
:cgraphic.
:font facename='Courier' size=12x12.DEVICE = c&colon.&bsl.diskdriv&per.sys
REM Block device D&colon. is now defined&per. (diskdriv&per.sys controls this&per.)
IFS = c&colon.&bsl.fsd&bsl.newfsl&per.fsd
REM If we assume that D&colon. contains a fixed newfsl type partition,
REM then we&apos.re now ready to use D&colon. to load the device driver and
REM FSD for E&colon.&per.
DEVICE = d&colon.&bsl.root&bsl.dev&bsl.special&per.dev
REM Block device e&colon. is now defined&per.
IFS = d&colon.&bsl.root&bsl.fsd&bsl.special&per.fsd
REM E&colon. can now be read&per.
DEVICE = e&colon.&bsl.music

:ecgraphic.
:font facename=default.  
:h3 id=13.OS/2 Partition Access

:p.Access to the OS/2 partition on a bootable, logically partitioned media is 
through the full OS/2 function set&per. See :hp1.OS/2 Version 2&per.0 Physical Device Driver 
Reference :ehp1.for a detailed description of the disk partitioning design&per.   
:h3 id=14.Permissions

:p.There are no secure file system clients identified for OS/2 Version 2&per.0 
incorporating the IFS architecture&per.   
:h3 id=15.File Naming Conventions

:p.See :hp1.OS/2 Version 2&per.0 Programming Guide :ehp1.for a detailed description of OS/2 
Version 2&per.0 file naming conventions&per.   
:h3 id=16.Meta Character Processing

:p.See :hp1.OS/2 Version 2&per.0 Programming Guide :ehp1.for a detailed description of OS/2 
Version 2&per.0 meta character processing&per.   
:h3 id=17.FSD Pseudo-character Device Support

:p.A pseudo-character device (single file device) may be redirected to an FSD
&per. The behavior of this file is very similar to the behavior of a normal OS/2 
character device&per. It may be read from (DosRead) and written to (DosWrite)&per. The 
difference is that the DosChgFilePtr and DosFileLocks functions can also be applied to the 
file&per. The user would perceive this file as a device name for a non-existing 
device&per. This file is seen as a character device because the current drive and 
directory have no effect on the name&per. That is what happens in OS/2 today for 
character devices&per.   
:p.The format of an OS/2 pseudo-character device name is that of an ASCIIZ 
string in the format of an OS/2 file name in a subdirectory called &bsl.DEV&bsl.&per. 
The pseudo device name XXX is accessible at the API level (DosQFSAttach) through 
the path name &apos.&bsl.DEV&bsl.XXX&apos.&per.   
:h3 id=18.Family API Issues

:p.Since the IFS Mechanism is not present in any release of DOS, FAPI will not 
be extended to support the new interfaces&per.   
:h3 id=19.FSD Utilities
  
.br 
:hp2.FSD Utility Support :ehp2.
:p.Each FSD is required to provide a single &per.DLL executable module that 
supports the OS/2 FORMAT, CHKDSK, SYS, and RECOVER utilities&per. The FS-supported 
executable will be invoked by these utilities when performing a FORMAT, CHKDSK, SYS, or 
RECOVER function for that file system&per. The command line that was passed to the 
utility will be passed unchanged to the FS-specific executable&per.   
:p.The procedures that support these utilities reside in a file called U<fsdname
>&per.DLL, where <fsdname> is the name returned by DosQFSAttach&per. If the file 
system utility support &per.DLL file is to reside on a FAT partition, then <fsdname> 
should be up to 7 bytes long&per.   
.br 
:hp2.FSD Utility Guidelines :ehp2.
:p.The FSD utility procedures are expected to follow these guidelines&colon.   
:p.oNo preparation is done by the base utilities before they invoke the FSD 
utility procedure&per. Therefore, base utilities do not lock drives, parse names, open 
drives, etc&per. This allows maximum flexibility for the FSD&per.   
:p.oThe FSD utility procedures are expected to follow the standard conventions 
for the operations that they are performing, for example, /F for CHKDSK implies 
&osq.fix&osq.&per.   
:p.oThe FSD procedures may use stdin, stdout, and stderr, but should be aware 
that they may have been redirected to a file or device&per.   
:p.oIt is the responsibility of the FSD procedures to worry about volumes being 
changed while the operation is in progress&per. The normal action would be to stop the 
operation when such a situation is detected&per.   
:p.oWhen the FSD procedures are called, they will be passed argc, argv, and envp
, that they can use to determine the operations&per.   
:p.oFSD procedures are responsible for displaying relevant prompts and messages
&per.   
:p.oFSD utility procedures must follow the standard convention of entering the 
target drive as specified for each utility&per. 
.br 
  
.br 
:hp2.FSD Utility Interfaces :ehp2.
:p.All FSD utility procedures are called with the same arguments&colon.   
:cgraphic.
:font facename='Courier' size=12x12.int far pascal CHKDSK(int argc, char far * far *argv,
char far * far *envp);

int far pascal FORMAT(int argc, char far * far *argv,
char far * far *envp);

int far pascal RECOVER(int argc, char far * far *argv,
char far * far *envp);

int far pascal SYS(int argc, char far * far *argv,
char far * far *envp);

:ecgraphic.
:font facename=default.
:p.where argc, argv, and envp have the same semantics as the corresponding 
variables in C&per.   
:h2 id=20.Extended Attributes

:p.Extended attributes (EAs) are a mechanism whereby an application can attach 
information to a file system object (directories or files) describing the object to another 
application, to the operating system, or to the FSD managing that object&per.   
:p.EAs associated with a file object are not part of a file object&apos.s data, 
but are maintained separately and managed by the file system that manages that 
object&per.   
:p.Each extended attribute consists of a name and a value&per. An EA name 
consists of ASCII text, chosen by the application developer, that is used to identify a 
particular EA&per. EA names are restricted to the same character set as a filename&per. An 
EA value consists of arbitrary data, that is, data of any form&per. Because of 
this OS/2 does not check data that is associated with an EA&per.   
:p.So that EA data is understandable to other applications, conventions have 
been established for&colon. 
:p.oNaming EAs 
.br 
oIndicating the type of data contained in EAs 
.br 

:p.In addition, a set of standard EAs (SEAs) have been defined&per. SEAs define 
a common set of information that can be associated with most files (for example, 
file type and file purpose)&per. Through SEAs, many applications can access the same
, useful information associated with files&per.   
:p.Applications are not limited to using SEAs to associate information with 
files&per. They may define their own application-specific extended attributes&per. 
Applications define and associate extended attributes with a file object through file system 
function calls&per.   
:p.See the :hp1.OS/2 Version 2&per.0 Programming Guide :ehp1.for a complete description of 
EA naming conventions and data types and standard extended attributes&per. See 
also the :hp1.OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.for a complete 
description of the file system function calls&per.   
:p.EAs may be viewed as a property list attached to file objects&per. The 
services for manipulating EAs are&colon. add/replace a series of name/value pairs, 
return name/value pairs given a list of names, and return the total set of EAs&per.   
:p.There are two formats for EAs as passed to OS/2 Version 2&per.0 API&colon. 
Full EAs (FEA) and Get EAs (GEA)&per.   
:h3 id=21.FEAs

:p.FEAs are complete name/value pairs&per. In order to simplify and speed up 
scanning and processing of these names, they are represented as length-preceded data
&per. FEAs are defined as follows&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct FEA {
    unsigned char fEA;        /* byte of flags      */
    unsigned char cbName;     /* length of name     */
    unsigned short cbValue;   /* length of value    */
    unsigned char szName[];   /* ASCIIZ name        */
    unsigned char aValue[];   /* free format value  */
};

:ecgraphic.
:font facename=default.
:p.There is only one flag defined in fEA&per. That is 0x80 which is fNeedEA&per. 
Setting the flag marks this EA as needed for the proper operation on the file to which 
it is associated&per. Setting this bit has implications for access to this file by 
old applications, so it should not be set arbitrarily&per.   
:p.If a file has one or more NEED EAs, old applications are not allowed to open 
the file&per. For DOS mode applications to access files with NEED EAs, they must 
have the EA bit set in their exe header&per. For OS/2 mode, only applications with 
the NEWFILES bit set in the exe header may open files with NEED EAs&per.   
:p.Programs that change EAs should preserve the NEED bit in the EAs unless there 
is a good reason to change it&per.   
:p.The name length does not include the trailing NUL&per. The maximum EA name 
length is 255 bytes&per. The minimum EA name length is 1 byte&per. The characters that 
form the name are legal filename characters&per. Wildcard characters are not allowed
&per. EA names are case-insensitive and should be uppercased&per. The FSD should call 
FSH_CHECKEANAME and FSH_UPPERCASE for each EA name it receives to check for invalid 
characters and correct length, and to uppercase it&per.   
:p.The FSD may not modify the flags&per.   
:p.A list of FEAs is a packed set of FEA structures preceded by a length of the 
list (including the length itself) as indicated in the following structure&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct FEAList {
    unsigned long cbList;       /* length of list       */
    struct FEA list[];          /* packed set of FEAs   */
};

:ecgraphic.
:font facename=default.
:p.FEA lists are used for adding, deleting, or changing EAs&per. A particular 
FSD may store the EAs in whatever format it desires&per. Certain EAs may be stored 
to optimize retrieval&per.   
:h3 id=22.GEAs

:p.A GEA is an attribute name&per. Its format is&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct GEA {
    unsigned char cbName;       /* length of name    */
    unsigned char szName[];     /* ASCIIZ name       */
};

:ecgraphic.
:font facename=default.
:p.The name length does not include the trailing NUL&per.   
:p.A list of GEAs is a packed set of GEA structures preceded by a length of the 
list (including the length itself) as indicated in the following structure&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct GEAList {
    unsigned long cbList;    /* length of list     */
    struct GEA list[];       /* packed set of GEAs */
};

:ecgraphic.
:font facename=default.
:p.GEA lists are used for retrieving the values for a particular set of 
attributes&per. A GEA list is used as input only&per.   
:p.Name lengths of 0 are illegal and are considered in error&per. A value length 
of 0 has special meaning&per. Setting an EA with a value length of 0 will cause 
that attribute to be deleted (if possible)&per. Upon retrieval, a value length of 0 
indicates that the attribute is not present&per.   
:p.Setting attributes contained in an FEA list does not treat the entire FEA 
list as atomic&per. If an error occurs before the entire list of EAs has been set, 
all, some, or none of them may actually remain set on the file&per. No program 
should depend on an EA set being atomic to force EAs to be consistent with each other
&per. Programs must be careful not to depend on atomicity, since a given file system 
may provide it&per.   
:p.Manipulation of extended attributes is associated with access permission to 
the associated file or directory&per. For querying and setting file EAs, read and 
write/read permission, respectively, for the associated file is required&per. No 
directory create or delete may occur while querying EAs for that directory&per.   
:p.For handle-based operations on extended attributes, access permission is 
controlled by the sharing/access mode of the associated file&per. If the file is open for 
read, querying the extended attributes is allowed&per. If the file is open for write
, setting the extended attributes is allowed&per. These operations are 
DosQFileInfo and DosSetFileInfo&per.   
:p.For path-based manipulation of extended attributes, the associated file or 
directory will be added to the sharing set for the duration of the call&per. The 
requested access permission for setting EAs is write/deny-all and for querying EAs is 
read/deny-write&per. The path-based API are DosQPathInfo, DosSetPathInfo, and 
DosFindFirst2/DosFindNext&per.   
:p.For create-only operations of extended attributes, the extended attributes 
are set without examining the sharing/access mode of the associated file/directory
&per. These operations are DosOpen2 and DosMkDir2&per.   
:p.The routing of EA requests is accomplished by the IFS routing mechanism&per. 
EA requests that apply to names are routed to the FSD attached to the specified 
drive&per. Those requests that apply to a handle (file or directory) are routed to 
the FSD attached to the handle&per. No interpretation of either FEA lists nor GEA 
lists is performed by the IFS router&per.   
:p.:hp2.Note&colon.  :ehp2.It is the responsibility of each FSD to provide support for EAs
&per.   
:p.It is expected that some FSDs will be unable to store EAs; for example, UNIX 
and MVS compatible file systems&per.   
:p.:hp2.Note&colon.  :ehp2.The FAT FSD implementation will provide for the complete 
implementation of EAs&per. There will be no special EAs for the FAT FSD&per.   
:p.All EA manipulation is performed using the following structure&colon. The 
relevance of each field is described within each API&per.   
:cgraphic.
:font facename='Courier' size=12x12.struct EAOP {
    struct GEAList far * fpGEAList; /* GEA set           */
    struct FEAList far * fpFEAList; /* FEA set           */
    unsigned long offError;         /* offset of FEA err */
};

:ecgraphic.
:font facename=default.
:p.See the descriptions of the file system function calls in :hp1.OS/2 Version 2&per.
0 Control Program Programming Reference :ehp1.for the relevance of each field&per.   
:p.In OS/2 Version 2&per.0, values of cbList greater than (64K-1) are not 
allowed&per. This is an implementation-defined limitation which may be raised in the 
future&per. Because this limit may change, programs should avoid enumerating the list 
of all EAs, but instead manipulate only EAs that they know about&per. For 
operations such as copying, the DosCopy API should be used&per. If enumeration is 
necessary, the DosEnumAttribute API should be used&per.   
:p.A special category of attributes, called create-only attributes, is defined 
as the set of extended attributes that a file system may only allow to be set at 
creation time&per. (Such attributes may be used to control file allocation and structure 
configuration&per.) File systems are expected to allow create-only attributes to be set at 
any time from when the object is created to when it is first modified, that is, 
data is written into a file or an entry added to a directory&per. Programs that copy 
objects should copy all of the EAs for an object before otherwise modifying it in order 
to assure that any create-only attributes from the source are properly applied to 
the target&per. The DosCopy API is the preferred method of copying files or 
directories&per.   
:h2 id=23.FSD File Image

:p.An FSD loads from a file which is in the format of a standard OS/2 dynamic 
link library file&per. Exactly one FSD resides in each file&per. The FSD exports 
information to OS/2 using a set of predefined public names&per.   
:p.The FSD is initialized by a call to the exported entry point FS_INIT&per.   
:p.FS entry points for Mount, Read, Write, etc&per. are exported with known 
names as standard far entry points&per.   
:p.The FSD exports its name as a public ASCIIZ character string under the name 
&apos.FS_NAME&apos.&per. All comparisons with user-specified strings are done similar 
to file names; case is ignored and embedded blanks are significant&per. FS_NAMEs, 
however, may be input to applications by users&per. Embedded blanks should be avoided
&per. The name exported as FS_NAME need NOT be the same as the 1-8 FSD name in the 
boot sector of formatted media, although it may be&per. The ONLY name the kernel 
pays any attention to, and the only name accessible to user programs through the API
, is the name exported as FS_NAME&per.   
:p.In addition to various entry points, the FSD must export a dword bit vector 
of attributes&per. Attributes are exported under the name &apos.FS_ATTRIBUTE&apos.
&per. FS_ATTRIBUTE specifies special properties of the FSD and is described in the 
next section&per.   
:h3 id=24.FSD Attribute

:p.The format of the OS/2 FS_ATTRIBUTE is defined in Figure 1-4 and the 
definition list that follows it&per.   
:cgraphic.
:font facename='Courier' size=12x12. 31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
ÚÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄ¿
³ E ³ V ³ V ³ V ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³
³ x ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³
³ A ³ r ³ r ³ r ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³
³ t ³ s ³ s ³ s ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³
ÃÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄÅÄÄÄ´
³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ R ³ L ³ F ³ U ³ R ³
³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ v ³ I ³ N ³ e ³
³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ s ³ l ³ / ³ C ³ m ³
³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ v ³ 7 ³ O ³   ³ t ³
ÀÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÙ
 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 1-4&per. OS/2 FSD Attribute :ehp2.  
:p.:hp2.Bits :ehp2.:hp2.Description :ehp2.  
:p.31 :hp2.FSD Additional attributes&per. :ehp2.If 1, FSD has additional attributes&per. If 
0, FS_ATTRIBUTE is the only FSD attribute information&per.   
:p.30-28 :hp2.VERSION NUMBER - FSD version number&per. :ehp2.  
:p.27-4 :hp2.RESERVED :ehp2.  
:p.3 :hp2.LEVEL7 - QPathInfo Level 7 bit&per. :ehp2.Set if FSD is case-preserving&per. If 
this bit is set, the kernel will call the FS_PATHINFO entry point with a level equal 
to 7&per. The output buffer is to be filled with a case-preserved copy of the path 
that was passed in by the user&per.   
:p.2 :hp2.FILEIO - File I/O bit&per. :ehp2.Set if FSD wants to see file locking/unlocking 
operations and compacted file I/O operations&per. If not set, the file I/O calls will be 
broken up into individual lock/unlock/read/write/seek calls and the FSD will not see 
the lock/unlock calls&per. FSDs that do not support file locking can set this bit 
to enable compacted file I/O operations&per.   
:p.1 :hp2.UNC - Universal Naming Convention bit&per. :ehp2.Set if FSD supports the 
Universal Naming Convention&per. OS/2 Version 2&per.0 supports multiple loaded UNC 
redirectors&per.   
:p.0 :hp2.REMOTE - Remote File System (Redirector)&per. :ehp2.This bit tells the system 
whether the FSD uses static or dynamic media attachment&per. Local FSDs always use 
dynamic media attachment&per. Remote FSDs always use static media attachment&per. This 
bit is clear if it is a dynamic media attachment and set, if a static attachment
&per. No FSD supports both static and dynamic media attachment&per. To support proper 
file locking, a remote FSD should also set the FILEIO bit&per. 
.br 
  
:h2 id=25.FSD Initialization

:p.FSD initialization occurs at system initialization time&per. FSDs are loaded 
through the IFS= configuration command in CONFIG&per.SYS&per. Once the FSD has been 
loaded, the FSD&apos.s initialization entry point is called to initialize it&per.   
:p.FSDs are structured the same as dynamic link library modules&per. Once an FSD 
is loaded, the initialization routine FS_INIT is called&per. This gives the FSD 
the ability to process any parameters that may appear on the CONFIG&per.SYS command 
line, which are passed as a parameter to the FS_INIT routine&per. A LIBINIT routine 
in an FSD will be ignored&per.   
:p.OS/2 FSDs initialize in protect mode&per. Because of the special state of the 
system, an FSD may make dynamic link system calls at init-time&per.   
:p.The list of systems calls that an FSD may make are as follows&colon.   
:p.oDosBeep 
.br 
oDosChgFilePtr 
.br 
oDosClose 
.br 
oDosDelete 
.br 
oDosDevConfig 
.br 
oDosDevIoCtl 
.br 
oDosFindClose 
.br 
oDosFindFirst 
.br 
oDosFindNext 
.br 
oDosGetEnv 
.br 
oDosGetInfoSeg 
.br 
oDosGetMessage 
.br 
oDosOpen 
.br 
oDosPutMessage 
.br 
oDosQCurDir 
.br 
oDosQCurDisk 
.br 
oDosQFileInfo 
.br 
oDosQFileMode 
.br 
oDosQSysInfo 
.br 
oDosRead 
.br 
oDosWrite 
.br 

:p.The FSD may not call ANY FS helper routines at initialization time&per.   
:p.Note that multiple code and data segments are not discarded by the loader as 
in the case of device drivers&per.   
:p.The FSD may call DosGetInfoSeg to obtain access to the global and process 
local information segments&per. The local segment may be used in the context of all 
processes without further effort to make it accessible and has the same selector&per. The 
local infoseg is not valid in real mode or at interrupt time&per.   
:h3 id=26.OS/2 and DOS Extended Boot Structure and BIOS Parameter Block

:p.The Extended Boot structure is as follows&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct Extended_Boot {
    unsigned char Boot_jmp[3];
    unsigned char Boot_OEM[8];
    struct Extended_BPB Boot_BPB;
    unsigned char Boot_DriveNumber;
    unsigned char Boot_CurrentHead;
    unsigned char Boot_Sig = 41; /* Indicate Extended Boot */
    unsigned char Boot_Serial[4];
    unsigned char Boot_Vol_Label[11];
    unsigned char Boot_System_ID[8];
};

:ecgraphic.
:font facename=default.
:p.:hp2.Where :ehp2.
:p.Boot_Serial is the 32-bit binary volume serial number for the media&per.   
:p.Boot_System_ID is an 8-byte name written when the media is formatted&per. It 
is used by FSDs to identify their media but need not be the same as the name the 
FSD exports via FS_NAME and is NOT the name users employ to refer to the FSD&per. (
They may, however, be the same names)&per.   
:p.Boot_Vol_Label is the 11-byte ASCII label of the disk/diskette volume&per. 
FAT file systems must ALWAYS use the volume label in the root directory for 
compatibility reasons&per. An FSD may use the one in the boot sector&per. 
.br 
  
:p.The extended BPB structure is a super-set of the conventional BPB structure, 
as follows&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct Extended_BPB {
    unsigned short BytePerSector;
    unsigned char SectorPerCluster;
    unsigned short ReservedSectors;
    unsigned char NumberOfFats;
    unsigned short RootEntries;
    unsigned short TotalSectors;
    unsigned char MediaDescriptor;
    unsigned short SectorsPerFat;
    unsigned short SectorsPerTrack;
    unsigned short Heads;
    unsigned long HiddenSectors;
    unsigned long Ext_TotalSectors;
};

:ecgraphic.
:font facename=default.  
:h2 id=27.IFS Commands
  
:h3 id=28.IFS = (CONFIG.SYS Command)

:p.An FSD is loaded and initialized at system start-up when an IFS= statement is 
encountered in CONFIG&per.SYS&per. The syntax of this command is as follows&colon.   
:cgraphic.
:font facename='Courier' size=12x12.IFS=drive&colon.path&bsl.name&per.ext parms

:ecgraphic.
:font facename=default.
:p.:hp2.where :ehp2.
.br 
drive&colon.path&bsl.name&per.ext specifies the FSD to load and initialize&per.   
.br 
parms represents an FSD-defined string of initialization parameters&per. 
.br 

:p.See the :hp1.OS/2 Version 2&per.0 Online Command Reference :ehp1.for a detailed 
description of this command&per.   
:h2 id=29.File System Function Calls

:p.The :hp1.OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.gives a 
detailed description of the 32-bit file system calls new for OS/2 Version 2&per.0 See 
the :hp1.OS/2 Version 2&per.0 Programming Guide :ehp1.for a description of how to use these 
calls&per. For detailed descriptions of the 16-bit file system calls see the :hp1.OS/2 
Version 1&per.3 Control Program Programming Reference:ehp1., and the :hp1.OS/2 Version 1&per.3 
Programming Guide :ehp1.on how to use these calls&per.   
:p.:hp2.Note&colon.  :ehp2.The data structures for some of the file system calls have 
changed in their 32-bit implementations&per. For OS/2 Version 2&per.0 the kernel will 
handle all remapping between the 32-bit structures and the 16-bit structures used by 
individual FSDs&per.   
:h3 id=30.Application File I/O Notes

:p.File handle values of 0xFFFF do not represent actual file handles but are 
used throughout the file system interface to indicate specific actions to be taken 
by the file system&per. Usage of this special file handle where it is not expected 
by the file system will result in an error&per.   
:p.Null pointers are defined to be 0x00000000 throughout this document&per.   
:p.File systems that conform to the Standard Application Program Interface (
Standard API) may not necessarily support all the described information kept on a file 
basis&per. When this is the case, FSDs are required to return to the application a 
null (zero) value for the unsupported parameter&per.   
:p.An FSD may support version levels of files&per. 
:p.
.br 
:hp2.Date/Time Stamps :ehp2.
:p.The format of OS/2 dates are show below in Figure 1-5&per.   
:cgraphic.
:font facename='Courier' size=12x12.ÚÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄ¿
³ Y ³ Y ³ Y ³ Y ³ Y ³ Y ³ Y ³ M ³ M ³ M ³ M ³ D ³ D ³ D ³ D ³ D ³
³ e ³ e ³ e ³ e ³ e ³ e ³ e ³ o ³ o ³ o ³ o ³ a ³ a ³ a ³ a ³ a ³
³ a ³ a ³ a ³ a ³ a ³ a ³ a ³ n ³ n ³ n ³ n ³ y ³ y ³ y ³ y ³ y ³
³ r ³ r ³ r ³ r ³ r ³ r ³ r ³ t ³ t ³ t ³ t ³   ³   ³   ³   ³   ³
³   ³   ³   ³   ³   ³   ³   ³ h ³ h ³ h ³ h ³   ³   ³   ³   ³   ³
ÀÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÙ
 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 1-5&per. OS/2 Date Format :ehp2.  
:p.:hp2.Bits :ehp2.:hp2.Description :ehp2.  
.br 
15-9 YEARS - Number of years since 1980&per.   
.br 
8-5 MONTH - is the month of the year (1-12)   
.br 
4-0 DAY - is the day of the month (1-31) 
.br 

:p.The format of OS/2 times are show below in Figure 1-6&per.   
:cgraphic.
:font facename='Courier' size=12x12.ÚÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄ¿
³ H ³ H ³ H ³ H ³ H ³ M ³ M ³ M ³ M ³ M ³ M ³ 2 ³ 2 ³ 2 ³ 2 ³ 2 ³
³ o ³ o ³ o ³ o ³ o ³ i ³ i ³ i ³ i ³ i ³ i ³ ³ ³ ³ ³ ³ ³ ³ ³ ³ ³
³ u ³ u ³ u ³ u ³ u ³ n ³ n ³ n ³ n ³ n ³ n ³ S ³ S ³ S ³ S ³ S ³
³ r ³ r ³ r ³ r ³ r ³   ³   ³   ³   ³   ³   ³ e ³ e ³ e ³ e ³ e ³
³   ³   ³   ³   ³   ³   ³   ³   ³   ³   ³   ³ c ³ c ³ c ³ c ³ c ³
ÀÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÙ
 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 1-6&per. OS/2 Time Format :ehp2.  
:p.:hp2.Bits :ehp2.:hp2.Description :ehp2.  
.br 
15-9 HOUR - is the hour of the day (0-23)   
.br 
8-5 MINUTE - is the minute of the hour (0-59)   
.br 
4-0 2-SECOND - is the second of the minute(in increments of 2) (0-29) 
.br 
  
.br 
:hp2.I/O Error Codes :ehp2.
:p.Some file system functions may return device-driver/device-manager generated 
errors&per. These include&colon.   
:p.oERROR_WRITE_PROTECT - the media in the drive has write-protection enabled
&per. 
.br 
oERROR_BAD_UNIT - there is a breakdown of internal consistency between OS/2
&apos.s mapping between logical drive and device driver&per. Internal Error&per. 
.br 
oERROR_NOT_READY - the device driver detected that the device is not ready&per. 
.br 
oERROR_BAD_COMMAND - there is a breakdown of internal consistency between OS/2
&apos.s idea of the capability of a device driver and that of the device driver&per. 
.br 
oERROR_CRC - the device driver has detected a CRC error&per. 
.br 
oERROR_BAD_LENGTH - there is a breakdown of internal consistency between OS/2
&apos.s idea of the length of a request packet and the device driver&apos.s idea of 
that length&per. Internal Error&per. 
.br 
oERROR_SEEK - the device driver detected an error during a seek operation&per. 
.br 
oERROR_NOT_DOS_DISK - the disk is not recognized as being OS/2 manageable&per. 
.br 
oERROR_SECTOR_NOT_FOUND - the device is unable to find the specific sector&per. 
.br 
oERROR_OUT_OF_PAPER - the device driver has detected that the printer is out of 
paper&per. 
.br 
oERROR_WRITE_FAULT - other write-specific error&per. 
.br 
oERROR_READ_FAULT - other read-specific error&per. 
.br 
oERROR_GEN_FAILURE - other error&per. 
.br 

:p.There are also errors defined by and specific to the device drivers&per. 
These are indicated by either 0xFF or 0xFE in the high byte of the error code&per.   
:p.:hp2.Note&colon.  :ehp2.Error codes listed in the function call descriptions in the :hp1.OS/2 
Version 2&per.0 Control Program Programming Reference :ehp1.are not complete&per. They are 
errors most likely to be returned by the FS router and the FAT file system&per. Each 
FSD may generate errors based upon its own circumstances&per.   
:h2 id=31.FSD System Interfaces
  
:h3 id=32.Overview

:p.Installable file system entry points are called by the kernel as a result of 
action taken through the published standard file I/O application programming interface 
in OS/2 Version 2&per.0&per.   
:p.Installable file systems are installed as OS/2 dynamic link library modules
&per. Unlike device drivers, they may include any number of segments, all of which 
will remain after initialization, unless the FSD itself takes some action to free 
them&per.   
:p.An FSD exports FS entries to the OS/2 kernel using standard PUBLIC 
declarations&per. Each FS entry is called directly&per. The OS/2 kernel manages the 
association between internal data structures and FSDs&per.   
:p.When a file system service is required, OS/2 assembles an argument list, and 
calls the appropriate FS entry for the relevant FSD&per. If a back-level FSD is 
loaded, the OS/2 kernel assures that all arguments passed and all structures passed 
are understood by the FSD&per.   
:p.Application program interfaces that are unsupported by an FSD receive an 
UNSUPPORTED FUNCTION error from the FSD&per.   
:p.Certain routines, for example, FS_PROCESSNAME, may provide no processing, no 
processing is needed, or processing does not make sense&per. These routines return no 
error, not ERROR_NOT_SUPPORTED&per.   
:h3 id=33.Data Structures

:p.OS/2 data structures that include a pointer to the file system driver, as 
well as file system specific data areas are&colon.   
:p.othe CDS (current directory structure) 
.br 
othe SFT (system file table entry), 
.br 
othe VPB (volume parameter block) 
.br 
othe file search structures&per. 
.br 

:p.File system service routines are generally passed pointers to two parameter 
areas, in addition to read-only parameters which are specific to each call&per. The 
FSD does not need to verify these pointers&per. The two parameter areas contain 
file-system-independent data which is maintained jointly by OS/2 and the file system 
driver and file-system-dependent data which is unused by OS/2 and which may be used in 
any way by the file system driver&per. The file system driver is generally 
permitted to use the file-system-dependent information in any way&per. The file-system-
dependent information may contain all the information needed to describe the current 
state of the file or directory, or it may contain a handle which will direct it to 
other information about the file maintained within the FSD&per. Handles must be GDT 
selectors because any SFT, CDS, or VPB may be seen by more than one process&per. File-
system-dependent and file-system-independent parameter areas are defined by data 
structures described in the remainder of this section&per.   
:h4 id=34.Disk media and file system layout

:p.are described by the following structures&per. The data which is provided to 
the file system may depend on the level of file system support provided by the 
device driver attached to the block device&per. These structures are relevant only for 
local file systems&per.   
:cgraphic.
:font facename='Courier' size=12x12./* file system independent - volume parameters */

struct vpfsi {
    unsigned long  vpi_vid;        /* 32 bit volume ID */
    unsigned long  vpi_hDEV;       /* handle to device driver */
    unsigned short vpi_bsize;      /* sector size in bytes */
    unsigned long  vpi_totsec;     /* total number of sectors */
    unsigned short vpi_trksec;     /* sectors / track */
    unsigned short vpi_nhead;      /* number of heads */
    char           vpi_text[12];   /* ASCIIZ volume name */
    void far *     vpi_pDCS;       /* device capability structure */
    void far *     vpi_pVCS;       /* volume characteristics */
    unsigned char  vpi_drive;      /* drive (0=A) */
    unsigned char  vpi_unit;       /* unit code */
};

/* file system dependent - volume parameters */

struct vpfsd {
    char           vpd_work[36];   /* work area */
};

:ecgraphic.
:font facename=default.  
:h4 id=35.Per-disk current directories

:p.are described by the following structures&per. These structures can only be 
modified by the FSD during FS_ATTACH and FS_CHDIR operations&per.   
:cgraphic.
:font facename='Courier' size=12x12./* file system independent - current directories */

struct cdfsi {
    unsigned short cdi_hVPB;           /* VPB handle for associated device */
    unsigned short cdi_end;            /* offset to root of path */
    char           cdi_flags;          /* FS independent flags */
    char           cdi_curdir[260];    /* text of current directory */
};

/* file system dependent - current directories */

struct cdfsd {
    char           cdd_work[8];        /* work area */
};

:ecgraphic.
:font facename=default.  
:h4 id=36.Open files

:p.are described by data initialized at file open time and discarded at the time 
of last close of all file handles which had been associated with that open 
instance of that file&per. There may be multiple open file references to the same file 
at any one time&per.   
:p.All time stamps on files are stamped and propagated to other SFTs by OS/2 
when the file is closed or committed (flushed)&per. For example, if a file is opened 
at time 1, written at time 2, and closed at time 3, the last write time is time 3
&per. Subdirectories need only have creation time stamps because the last write and 
last read time stamps on subdirectories are either very difficult to implement (
propagate up to parent subdirectories), or are not very useful&per. An FSD, however, may 
implement them&per. FSDs are required to support direct access opens&per. These are 
indicated by a bit set in the sffsi&per.sfi_mode field&per.   
:cgraphic.
:font facename='Courier' size=12x12./* file system independent - file instance */

struct sffsi {
    unsigned long   sfi_mode;       /* access/sharing mode */
    unsigned short  sfi_hVPB;       /* volume info&per. */
    unsigned short  sfi_ctime;      /* file creation time */
    unsigned short  sfi_cdate;      /* file creation date */
    unsigned short  sfi_atime;      /* file access time */
    unsigned short  sfi_adate;      /* file access date */
    unsigned short  sfi_mtime;      /* file modification time */
    unsigned short  sfi_mdate;      /* file modification date */
    unsigned long   sfi_size;       /* size of file */
    unsigned long   sfi_position;   /* read/write pointer */

/* the following may be of use in sharing checks */

    unsigned short  sfi_UID;        /* user ID of initial opener */
    unsigned short  sfi_PID;        /* process ID of initial opener */
    unsigned short  sfi_PDB;        /* PDB (in 3&per.x box) of initial opener */
    unsigned short  sfi_selfsfn;    /* system file number of file instance */
    unsigned char   sfi_tstamp;     /* time stamp flags */
    unsigned short  sfi_type;       /* type of object opened */
    unsigned long   sfi_pPVDBFil;   /* performance counter data block pointer */
    unsigned char   sfi_DOSattr;    /* DOS file attributes D/S/A/H/R */
};

/* file system dependent - file instance */

struct sffsd {
    char            sfd_work[30];   /* work area */
};

:ecgraphic.
:font facename=default.
:p.The Program Data Block, or PDB (sfi_pdb), is the unit of sharing for DOS mode 
processes&per. For OS/2 mode processes, the unit of sharing is the Process ID, PID (sfi_
pid)&per. FSDs should use the combination PDB, PID, UID as indicating a distinct 
process&per.   
:h4 id=37.File search records

:cgraphic.
:font facename='Courier' size=12x12./* file system independent - file search parameters */

struct fsfsi {
        unsigned short  fsi_hVPB;       /* volume info&per. */
};

/* file system dependent - file search parameters */

struct fsfsd {
        char            fsd_work[24];   /* work area */
};

:ecgraphic.
:font facename=default.
:p.Existing file systems that conform to the Standard Application Program 
Interface (Standard API) described in this section, may not necessarily support all the 
described information kept on a file basis&per. When this is the case, file system 
drivers are required to return to the application a null (zero) value for the 
unsupported parameter (when the unsupported data are a subset of the data returned by the 
API) or to return a ERROR_NOT_SUPPORTED error (when all of the data returned by the 
API is unsupported)&per.   
:h4 id=38.Time Stamping

:p.All time stamps on files are stamped and propagated to other SFTs when the 
file is closed or committed (flushed)&per. If a file is opened at time 1, written to 
at time 2, and closed at time 3, the last write time will be time 3&per. 
Subdirectories only have creation time stamps&per.   
:p.The sfi_tstamp field of the file instance structure sffsi contains six flags
&colon.   :font facename='Courier' size=12x12.
:cgraphic.
:color fc=default.:color bc=default.
 Name            Value   Description
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 ST_SCREAT       1       stamp creation time
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 ST_PCREAT       2       propagate creation time
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 ST_SWRITE       4       stamp last write time
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 ST_PWRITE       8       propagate last write time
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 ST_SREAD        16      stamp last read time
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 ST_PREAD        32      propagate last read time


:ecgraphic.

:p.These flags are cleared when an SFT is created, and some of them may 
eventually be set by a file system worker routine&per. They are examined when the file is 
closed or flushed&per.   
:p.For each time stamp, there are three meaningful actions&colon.   :font facename='Courier' size=12x12.
:cgraphic.
:color fc=default.:color bc=default.
 ST_Sxxx    ST_Pxxx    Action
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 clear      clear      don&apos.t do anything
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 set        set        stamp and propagate (to other SFTs and
                       disk)
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 clear      set        don&apos.t stamp, but propagate existing
                       value


:ecgraphic.
  
:h2 id=39.FSD Calling Conventions and Requirements

:p.Calling conventions between FS router, FSD, and FS helpers are&colon.   
:p.oArguments will be pushed in left-to-right order onto the stack&per. 
:p.oThe callee is responsible for cleaning up the stack&per. 
:p.oRegisters DS, SI, DI, BP, SS, SP are preserved&per. 
:p.oReturn conditions appear in AX with the convention that AX == 0 indicates 
successful completion&per. AX != 0 indicates an error with the value of AX being the error 
code&per. 
.br 

:p.Interrupts must ALWAYS be enabled and the direction flag should be presumed 
to be undefined&per. Calls to the FS helpers will change the direction flag at 
will&per.   
:p.In OS/2, file system drivers are always called in kernel protect mode&per. 
This has the advantage of allowing the FSD to execute code without having to account 
for preemption; no preemption occurs when in kernel mode&per. While this greatly 
simplifies FSD structure, it forces the FSD to yield the CPU when executing long segments 
of code&per. In particular, an FSD must not hold the CPU for more than 2 
milliseconds at a time&per. The FSD helper FSH_YIELD is provided so that FSDs may relinquish 
the CPU&per.   
:p.File system drivers cannot have any interrupt-time activations&per. Because 
they occupy high, movable, and swappable memory, there is no guarantee of 
addressability of the memory at interrupt time&per.   
:p.Each FS service routine may block&per.   
:h3 id=40.Error Codes

:p.FSDs should use existing error codes when possible&per. New error codes must 
be in the range reserved for FSDs&per. The FS_FSCTL interface must support 
returning information about new error codes&per.   
:p.The set of error codes for errors general to all FSDs is 0xEE00 - 0xEEFF&per. 
The following errors have been defined&colon.   
:p.oERROR_VOLUME_NOT_MOUNTED = 0xEE00 - the FSD did not recognize the volume
&per. 
.br 

:p.The set of error codes which are defined by each FSD is 0xEF00 - 0xFEFF&per.   
:h1 id=41.FS Service Routines

:p.The following table summarizes the entry points that make up the interface 
between the kernel and the FSD&per.   
:p.:hp2.Note&colon.  :ehp2.Names must be in all upper case, as required by OS/2 naming 
conventions&per.   :font facename='Courier' size=12x12.
:cgraphic.
:color fc=default.:color bc=default.
 FS Entry Point       Description                         FSDs Required
                                                          to export
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=42.FS_ALLOCATEPAGESPACE:elink. Adjust the size of paging file      PAGE I/O
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=43.FS_ATTACH:elink.            Attach to an FSD                    ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=44.FS_CANCELLOCKREQUEST:elink. Cancel file record lock request     FILE I/O
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=45.FS_CHDIR:elink.             Change/Verify directory path        ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=46.FS_CHGFILEPTR:elink.        Move a file&apos.s position pointer      ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=47.FS_CLOSE:elink.             Release a file handle               ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=48.FS_COMMIT:elink.            Flush a file&apos.s buffer to disk       ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=49.FS_COPY:elink.              Copy a file                         ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=50.FS_DELETE:elink.            Delete a file                       ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=51.FS_DOPAGEIO:elink.          Perform paging I/O operations       PAGE I/O
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=52.FS_EXIT:elink.              End of a process cleanup            ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=53.FS_FILEATTRIBUTE:elink.     Query/Set file&apos.s attributes         ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=54.FS_FILEINFO:elink.          Query/Set file&apos.s information        ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=55.FS_FILEIO:elink.            Multi-function file I/O             ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=56.FS_FILELOCKS:elink.         Request a file record lock/unlock   FILE I/O
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=57.FS_FINDCLOSE:elink.         Directory search close              ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=58.FS_FINDFIRST:elink.         Find first matching filename        ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=59.FS_FINDFROMNAME:elink.      Find matching filename from name    ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=60.FS_FINDNEXT:elink.          Find next matching filename         ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=61.FS_FINDNOTIFYCLOSE:elink.   Close FindNotify handle             ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=62.FS_FINDNOTIFYFIRST:elink.   Monitor a directory for changes     ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=63.FS_FINDNOTIFYNEXT:elink.    Resume reporting directory changes  ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=64.FS_FLUSHBUF:elink.          Commit file buffers to disk         ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=65.FS_FSCTL:elink.             File system control                 ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=66.FS_FSINFO:elink.            Query/Set file system information   ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=67.FS_INIT:elink.              FSD initialization                  ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=68.FS_IOCTL:elink.             I/O device control                  ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=69.FS_MKDIR:elink.             Make a directory                    ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=70.FS_MOUNT:elink.             Mount/unmount volumes               ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=71.FS_MOVE:elink.              Move a file or subdirectory         ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=72.FS_NEWSIZE:elink.           Change a file&apos.s logical size        ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=73.FS_NMPIPE:elink.            Do a named pipe operation           ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=74.FS_OPENCREATE:elink.        Open/create/replace files           ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=75.FS_OPENPAGEFILE:elink.      Create paging file and handle       PAGE I/O
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=76.FS_PATHINFO:elink.          Query/Set a file&apos.s information      ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=77.FS_PROCESSNAME:elink.       FSD unique name canonicalization    ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=78.FS_READ:elink.              Read data from a file               ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=79.FS_RMDIR:elink.             Remove a subdirectory               ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=80.FS_SETSWAP:elink.           Notification of swapfile ownership  ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=81.FS_SHUTDOWN:elink.          Shutdown file system                ALL
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=82.FS_VERIFYUNCNAME:elink.     Verify UNC server ownership         UNC
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=83.FS_WRITE:elink.             Write data to a file                ALL


:ecgraphic.

:p.Each FS entry point has a distinct parameter list composed of those 
parameters needed by that particular entry&per. Parameters include&colon.   
:p.oFile pathname 
.br 
oCurrent disk/directory information 
.br 
oOpen file information 
.br 
oApplication data buffers 
.br 
oDescriptions of file extended attributes 
.br 
oOther parameters specific to an individual call 
.br 

:p.Most of the FS entry points have a level parameter for specifying the level 
of information they are provided or have to supply&per. FSDs must provide for 
additional levels which may be added in future versions of OS/2 by returning ERROR_NOT_
SUPPORTED for any level they do not recognize&per.   
:p.File system drivers which support hierarchical directory structures must use 
&apos.&bsl.&apos. and &apos./&apos. as path name component separators&per. File system 
drivers which do not support hierarchical directory structures must reject as illegal 
any use of &apos.&bsl.&apos. or &apos./&apos. in path names&per. The file names 
&apos.&per.&apos. and &apos.&per.&per.&apos. are reserved for use in hierarchical 
directory structures for the current directory and the parent of the current directory, 
respectively&per.   
:p.Unless otherwise specified in the descriptions below, data buffers may be 
accessed without concern for the accessibility of the data&per. OS/2 will either check 
buffers for accessibility and lock them, or transfer them into locally accessible data 
areas&per.   
:p.Simple parameters will be verified by the IFS router before the FS service 
routine is called&per.   
:p.:hp2.Note&colon.  :ehp2.New with 2&per.0, some entry points need only be exported and 
supported by those FSDs which desire to service the pager (PAGE I/0), UNC servers (UNC) 
and/or file locking (FILE I/O)&per. With these new entry point groups, a FSD must 
export all or none of the entry points within a particular group&per.   
:p.These optional entry points are&colon.   
:cgraphic.
:font facename='Courier' size=12x12.FS_ALLOCATEPAGESPACE    (PAGE I/O)
FS_CANCELLOCKREQUEST    (FILE I/O)
FS_DOPAGEIO             (PAGE I/O)
FS_FILELOCKS            (FILE I/O)
FS_OPENPAGEFILE         (PAGE I/O)
FS_VERIFYUNCNAME        (UNC)

:ecgraphic.
:font facename=default.  
:h2 id=42.FS_ALLOCATEPAGESPACE - Adjust the size of paging file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Changes the size the paging file on disk&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_ALLOCATEPAGESPACE(psffsi, psffsd, ulsize, ulWantContig)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
unsigned long ulsize;
unsigned short ulWantContig;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.ulsize is the desired new size of the paging file&per. If the new size is 
smaller than the current size, the excess space is released&per. If the new size is 
larger than the current size, the requested size is allocated&per.   
:p.ulWantContig indicates the minimum contiguity requirement (in bytes)&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.ulWantContig is a demand for contiguity&per. If ulWantContig is non-zero(0), 
the FSD must allocate any space in the swap file that is not contiguous in 
ulWantContig chunks on ulWantContig boundaries&per. If it is not possible to grow the file 
to ulSize bytes meeting the ulWantContig requirement, the operation should fail
&per. If the file is being shrunk ulWantContig is irrelevant and should be ignored
&per.   
:p.FSDs that support the paging I/O interface should be expected to be sensible 
in allocating page space&per. In particular, they are expected to always attempt 
to allocate space such that ulWantContig sized blocks on ulWantContig boundaries 
are physically contiguous on disk, and to keep the page file as a whole contiguous 
as possible&per.   
:h2 id=43.FS_ATTACH - Attach to an FSD

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Attach or detach a remote drive or pseudo-device to an FSD&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_ATTACH(flag, pDev, pvpfsd, pcdfsd, pParm, pLen)

unsigned short flag;
char far * pDev;
struct vpfsd far * pvpfsd;
struct cdfsd far * pcdfsd;
char far * pParm;
unsigned short far * pLen;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates attach or detach&colon. 
:p.flag == 0 requests an attach&per. The FSD is being called to attach a 
specified driver or character device&per. 
.br 
flag == 1 requests a detach&per. 
.br 
flag == 2 requests the FSD to fill in the specified buffer with attachment 
information&per.   
:p.pDev is a pointer to the ASCIIZ text of either the driver (driver letter 
followed by a colon) or to the character device (must be &bsl.DEV&bsl.device) that is 
being attached, detached, or queried&per. The FSD does not need to verify this 
pointer&per.   
:p.tpvpfsd is a pointer to a data structure containing file-system-dependent 
volume parameter information&per. When an attach/detach/query of a character device is 
requested, this pointer is NULL&per. When attaching a drive, this structure contains no 
data and is available for the FSD to store information needed to manage the remote 
drive&per. All subsequent FSD calls have access to the hVPB in one of the structures 
passed in, so the FSD has access to this structure by using FSH_GETVOLPARMS&per. This 
structure will have its contents as the FSD had left them&per. When detaching or querying 
a drive, this structure contains the data as the FSD left them&per.   
:p.pcdfsd is a pointer to a data structure containing file-system dependent 
working directory information for drives&per. When attaching a drive, this structure 
contains no data and is available for the FSD to store information needed to manage the 
working directory&per. All subsequent FSD calls generated by API calls that reference 
this drive are passed a pointer to this structure with contents left as the FSD left 
them&per. When detaching or querying a drive, this structure contains the data as 
the FSD left them&per. For character devices, pcdfsd points to a DWORD&per. When a 
device is attached, the DWORD contains no data, and can be used by the FSD to store a 
reference to identify the device later on during FS_OPENCREATE, when it is passed in to 
the FSD&per. When detaching or querying the device, this DWORD contains the data as 
the FSD left them&per.   
:p.pParm is the address of the application parameter area&per.   
:p.When an attach is requested, this will point to the API-specified user data 
block that contains information regarding the attach operation (for example, 
passwords)&per. For a query, the OS/2 kernel will fill in part of the buffer, adjust the 
pointer, and call the FSD to fill in the rest (see the structures returned by 
DosQFSAttach; pParm will point to cbFSAData; the FSD should fill in cbFSAData and rgFSAData
&per.)   
:p.Addressing of this data area is not validated by the OS/2 kernel&per. pParm 
must be verified, even in the query case&per. The FSD verifies this parameter by 
calling the FS helper routine FSH_PROBEBUF&per.   
:p.pLen is the pointer to the length of the application parameter area&per. For 
attach, this points to the length of the application data buffer&per. For query, this 
is the length of the remaining space in the application data buffer&per. Upon 
filling in the buffer, the FSD will set this to the length of the data returned&per. If 
the data returned is longer than the data buffer length, the FSD sets this value to 
be the length of the data that query could return&per. In this case, the FSD also 
returns a BUFFER OVERFLOW error&per.   
:p.The FSD does not need to verify this pointer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Local FSDs will never get called with attempts to attach or detach drives or 
queries about drives&per.   
:p.For remote FSDs called to do a detach, the kernel does not do any checking to 
see if there are any open references on the drive (for example, open or search 
references)&per. It is entirely up to the FSD to decide whether it should allow the detach 
operation&per.   
:h2 id=44.FS_CANCELLOCKREQUEST - Cancel file record lock request

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Cancels an outstanding FS_FILELOCKS request on a file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_CANCELLOCKREQUEST(psffsi, psffsd, pLockRange)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
struct filelock far * pLockRange;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.pLockRange is a pointer to a filelock structure&per. The filelock structure 
has the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct FileLock {
    unsigned long FileOffset;   /* offset where the lock/unlock begins  */
    unsigned long RangeLength;  /* length of region locked/unlocked     */
}

:ecgraphic.
:font facename=default.
.br 
:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This entry point was added to support the 32-bit DosCancelLockRequest API
&per.   
:p.This function provides a simple mechanism for canceling the lock range 
request of an outstanding FS_FILELOCKS call&per. If two threads in a process are 
blocked on a lock range and a cancel request is issued by another thread, both blocked 
threads will be released&per.   
:h2 id=45.FS_CHDIR - Change/Verify Directory Path

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Change or verify the directory path for the requesting process   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_CHDIR(flag, pcdfsi, pcdfsd, pDir, iCurDirEnd)

unsigned short flag;
struct cdsfi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pDir;
unsigned short iCurDirEnd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates what action is to be taken on the directory&per. 
:p.flag == 0 indicates that an explicit directory-change request has been made
&per. 
.br 
flag == 1 indicates that the working directory needs to be verified&per. 
.br 
flag == 2 indicates that this reference to a directory is being freed&per.   
:p.The flag passed to the FSD will have a valid value&per.   
:p.pcdfsi is a pointer to a file-system-independent working directory structure
&per.   
:p.For flag == 0, this pointer points to the previous current directory on the 
drive&per. 
.br 
For flag == 1, this pointer points to the most recent working directory on the 
drive&per. The cdi_curdir field contains the text of the directory that is to be 
verified&per. 
.br 
For flag == 2, this pointer is null&per.   
:p.The FSD must never modify the cdfsi&per. The OS/2 kernel handles all updates
&per.   
:p.pcdfsd is a pointer to a file-system-dependent working directory structure
&per.   
:p.This is a place for the FSD to store information about the working directory
&per. For flag == 0 or 1, this is the information left there by the FSD&per. The FSD 
is expected to update this information if the directory exists&per. For flag == 2, 
this is the information left there by the FSD&per.   
:p.pDir is a pointer to directory text&per.   
:p.For flag == 0, this is the pointer to the directory&per.  For flag == 1 or 
flag == 2, this pointer is null&per. The FSD does not need to verify this pointer
&per.   
:p.iCurDirEnd is the index of the end of the current directory in pDir&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no current directory relevant to the directory text, that is, a device&per. 
This parameter only has meaning for flag == 0&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.  
:p.The FSD should cache no information when the directory is the root&per. Root 
directories are a special case&per. They always exist, and never need validation&per. The 
OS/2 kernel does not pass root directory requests to the FSD&per. An FSD is not 
allowed to cache any information in the cdfsd data structure for a root directory&per. 
Under normal conditions, the kernel does not save the CDS for a root directory and 
builds one from scratch when it is needed&per. (One exception is where a validate CDS 
fails, and the kernel sets it to the root, and zeroes out the cdfsd data structure
&per. This CDS is saved and is cleaned up later&per.)   
:p.The following is information about the exact state of the cdfsi and cdfsd 
data structures passed to the FSD for each flag value and guidelines about what an 
FSD should do upon receiving an FS_CHDIR call&colon.   
:cgraphic.
:font facename='Courier' size=12x12.IF (flag == 0)     /* Set new Current Directory */
   pcdfsi, pcdfsd = copy of CDS we&apos.re starting from; may be useful as starting
                    point for verification&per.

           cdfsi contents&colon.

              hVPB - handle of Volume Parameter Block mapped to this drive

              end - end of root portion of CurDir

              flags - various flags (indicating state of cdfsd)

              IsValid - cdfsd is unknown format (ignore contents)
                        IsValid == 0x80

              IsRoot - cdfsd is meaningless if CurDir = root (not kept)
                        IsRoot == 0x40

              IsCurrent - cdfsd is know format, but may not be current (medium
                          may have been changed)&per.
                          IsCurrent == 0x20

              text - Current Directory Text

   icurdir = if Current Directory is in the path of the new Current Directory,
             this is the index to the end of the Current Directory&per. If not,
             this is -1 (Current Directory does not apply)&per.

   pDir = path to verify as legal directory

THEN

   Validate path named in pDir&per.
       /* This means both that it exists AND that it is a directory&per.  pcdfsi,
          pcdfsd, icurdir give old CDS, which may allow optimization */

   IF (Validate succeeds)
      IF (pDir != ROOT)
         Store any cache information in area pointed to by pcdfsd&per.
      ELSE
         Do Nothing&per.
         /* Area pointed to by pcdfsd will be thrown away, so don&apos.t bother
            storing into it */
         Return success&per.
    ELSE
      Return failure&per.
      /* Kernel will create new CDS using pDir data and pcdfsd data&per. If the
         old CDS is valid, the kernel will take care of cleaning it up&per. The
         FSD must not edit any structure other than the *Pcdfsd area, with
         which it may do as it chooses&per. */
/* flag == 0 */
ELSE
IF (flag == 1)       /* Validate current CDS structure */

   pcdfsi = pointer to copy of cdfsi of interest&per.

   pcdfsd = pointer to copy of cdfsd&per. Flags in cdfsi indicate the state of
            this cdfsd&per. It may be&colon. (1) completely invalid (unknown
            format), (2) known format, but non-current information,
            (3) completely valid, or (4) all zero (root)&per.

THEN

    Validate that CDS still describes a legal directory (using cdi_text)&per.

    IF (valid)
       Update cdfsd if necessary&per.
       Return success&per.
       /* kernel will copy cdfsd into real CDS */
    ELSE
       IF (cdi_isvalid)
          Release any resources associated with cdfsd&per.
          /* kernel will force Current Directory to root, and will zero out
             cdfsd in real CDS */
          Return failure&per.
    /* The FSD must not modify any structure other than the cdfsd pointed to by
       pcdfsd&per.  */
ELSE
IF (flag == 2)   /* previous CDS no longer in use; being freed */

   pcdfsd = pointer to copy of cdfsd of CDS being freed&per.

THEN

   Release any resources associated with the CDS&per.
   /* For example, if cdfsd (where pcdfsd points) contains a pointer to
      some FSD private structure associated with the CDS, that structure
      should be freed&per. */

/* kernel will not retain the cdfsd */

:ecgraphic.
:font facename=default.  
:h2 id=46.FS_CHGFILEPTR - Move a file's position pointer

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Move a file&apos.s logical read/write position pointer&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_CHGFILEPTR(psffsi, psffsd, offset, type, IOflag)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
long offset;
unsigned short type;
unsigned short IOflag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.The FSD uses the current file size or sfi_position along with offset and type 
to compute a new sfi_position&per. This is updated by the system&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per. The FSD may store or adjust data as appropriate in this structure&per.   
:p.offset is the signed offset to be added to the current file size or position 
to form the new position within the file&per.   
:p.type indicates the basis of a seek operation&per.   
:p.type == 0 indicates seek relative to beginning of file&per. 
.br 
type == 1 indicates seek relative to current position within the file&per. 
.br 
type == 2 indicates seek relative to end of file&per.   
:p.The value of type passed to the FSD will be valid&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through&per. 
.br 
IOflag == 0x0020 indicates no-cache&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The file system may want to take the seek operation as a hint that an I/O 
operation is about to take place at the new position and initiate a positioning operation 
on sequential access media or read-ahead operation on other media&per.   
:p.Some DOS mode programs expect to be able to do a negative seek&per. OS/2 
passes these requests on to the FSD and returns an error for OS/2 mode negative seek 
requests&per. Because a seek to a negative position is, effectively, a seek to a very 
large offset, it is suggested that the FSD return end-of-file for subsequent read 
requests&per.   
:p.FSDs must allow seeks to positions beyond end-of-file&per.   
:p.The information passed in IOflag is what was set for the handle during a 
DosOpen/DosOpen2 operation, or by a DosSetFHandState call&per.   
:h2 id=47.FS_CLOSE - Close a file.

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Closes the specified file handle&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_CLOSE(type, IOflag, psffsi, psffsd)

unsigned short type;
unsigned short IOflag;
struct sffsi far * psffsi;
struct sffsd far * psffsd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.type indicates what type of a close operation this is&per.   
:p.type == 0 indicates that this is not the final close of the file or device
&per. 
.br 
type == 1 indicates that this is the final close of this file or device for this 
process&per. 
.br 
type == 2 indicates that this is the final close for this file or device for the 
system&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through&per. 
.br 
IOflag == 0x0020 indicates no-cache&per.   
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This entry point is called on the every close of a file or device&per.   
:p.Any reserved resources for this instance of the open file may be released It 
may be assumed that all open files will be closed at process termination&per. That 
is, this entry point will always be called at process termination for any files or 
devices open for the process&per.   
:p.A close operation should be interpreted by the FSD as meaning that the file 
should be committed to disk as appropriate&per.   
:p.Of the information passed in IOflag, the write-through bit is a mandatory bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:h2 id=48.FS_COMMIT - Commit a file's buffers to Disk

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Flush requesting process&apos.s cache buffers and update directory 
information for the file handle&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_COMMIT(type, IOflag, psffsi, psffsd)

unsigned short type;
unsigned short IOflag;
struct sffsi far * psffsi;
struct sffsd far* psffsd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.type indicates what type of a commit operation this is&per.   
:p.type == 1 indicates that this is a commit for a specific handle&per. This 
type is specified if FS_COMMIT is called for a DosBufReset of a specific handle&per.   
:p.type == 2 indicates that this is a commit due to a DosBufReset (-1)&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through&per. 
.br 
IOflag == 0x0020 indicates no-cache&per.   
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This entry point is called only as a result of a DosBufReset function call
&per. OS/2 reserves the right to call FS_COMMIT even if no changes have been made to 
the file&per.   
:p.For DosBufReset (-1), FS_COMMIT will be called for each open handle on the 
FSD&per.   
:p.The FSD should update access and modification times, if appropriate&per.   
:p.Any locally cached information about the file must be output to the media
&per. The directory entry for the file is to be updated from the sffsi and sffsd data 
structures&per.   
:p.Since mini-FSDs used to boot IFSs are read-only file systems, they need not 
support the FS_COMMIT call&per.   
:p.Of the information passed in IOflag, the write-through bit is a MANDATORY bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:p.The FSD should copy all supported time stamps from the SFT to the disk&per. 
Beware that the last read time stamp may need to be written to the disk even though 
the file is clean&per. After this is done, the FSD should clear the sfi_tstamp 
field to avoid having to write to the disk again if the user calls commit repeatedly 
without changing any of the time stamps&per.   
:p.If the disk is not writeable and only the last read time stamp has changed, 
the FSD should either issue a warning or ignore the error&per. This relieves the 
user from having to un-protect an FSD floppy disk in order to read the files on it
&per.   
:h2 id=49.FS_COPY - Copy a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Copy a specified file or subdirectory to a specified target&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_COPY(flag, pcdfsi, pcdfsd, pSrc, iSrcCurDirEnd, pDst,
                       iDstCurDirEnd, nameType)

unsigned short flag;
struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pSrc;
unsigned short iSrcCurDirEnd;
char far * pDst; unsigned short iDstCurDirEnd;
unsigned short nameType;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag is a bit mask controlling copy   
:p.0x0001 specifies that an existing target file/directory should be replaced 
.br 
0x0002 specifies that a source file will be appended to the destination file
&per. 
.br 
All other bits are reserved&per.   
:p.(See the description of the DosCopy function call in the :hp1.OS/2 Version 2&per.0 
Control Program Programming Reference:ehp1.&per.)   
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pSrc is a pointer to the ASCIIZ name of the source file/directory&per.   
:p.iSrcCurDirEnd is the index of the end of the current directory in pSrc&per. 
If = -1, there is no current directory relevant to the source name&per.   
:p.pDst is a pointer to the ASCIIZ name of the destination file/directory&per.   
:p.iDstCurDirEnd is the index of the end of the current directory in pDst&per. 
If = -1, there is no current directory relevant to the destination name&per.   
:p.nameType indicates the destination name type&per.   
:p.NameType == 0x0040 indicates non-8&per.3 filename format&per. All other 
values are reserved&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The file specified in the source file name should be copied to the target 
file if possible&per.   
:p.The files specified may not be currently open&per. File system drivers must 
assure consistency of file allocation information and directory entries&per.   
:p.The file system driver returns the special CANNOT COPY error if it cannot 
perform the copy because&colon.   
:p.oit does not know how 
.br 
othe source and target are on different volumes 
.br 
oof any other reason for which it would make sense for its caller to perform the 
copy operation manually&per. 
.br 

:p.Returning ERROR_CANNOT_COPY indicates to its caller that it should attempt to 
perform the copy operation manually&per. Any other error will be returned directly to 
the caller of DosCopy&per. See the description of the DosCopy function call in the :hp1.
OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.for other error codes 
that can be returned&per.   
:p.FS_COPY needs to check that certain types of illegal copying operations are 
not performed&per. A directory cannot be copied to itself or to one of its 
subdirectories&per. This is especially critical in situations where two different fully-
qualified pathnames can refer to the same file or directory&per. For example, if X&colon. 
is redirected to &bsl.&bsl.SERVER&bsl.SHARE, the X&colon.&bsl.PATH and &bsl.&bsl.
SERVER&bsl.SHARE&bsl.PATH refer to the same object&per.   
:p.The behavior of FS_COPY should match the behavior of the generic DosCopy 
routine&per.   
:p.The non-8&per.3 filename format attribute in the directory entry for the 
destination name should be set according to the value in nameType&per.   
:h2 id=50.FS_DELETE - Delete a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Removes a directory entry associated with a filename&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_DELETE(pcdfsi, pcdfsd, pFile, iCurDirEnd)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pFile;
unsigned short iCurDirEnd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pFile is a pointer to the ASCIIZ name of the file or directory&per. The FSD 
does not need to validate this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pFile&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no current directory relevant to the name text, that is, a device&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The files specified are deleted&per.   
:p.The deletion of a file opened in DOS mode by the same process requesting the 
delete is supported&per. OS/2 calls FS_CLOSE for the file before calling FS_DELETE
&per.   
:p.The file name may not contain wildcard characters&per.   
:h2 id=51.FS_DOPAGEIO - Perform paging I/O operations

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Performs all the I/O operations in a PageCmdList&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_DOPAGEIO(psffsi, psffsd, pList)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
struct PageCmdHeader far * pList;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.pList is a pointer to a PageCmdHeader structure&per.   
:p.The PageCmdHeader structure has the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct PageCmdHeader {
    unsigned char  InFlags;      /* Input Flags                        */
    unsigned char  OutFlags;     /* Output Flags - must be 0 on entry  */
    unsigned char  OpCount;      /* Number of operations               */
    unsigned char  Pad;          /* Pad for DWORD alignment            */
    unsigned long  Reserved1;    /* Currently Unused                   */
    unsigned long  Reserved2;    /* Currently Unused                   */
    unsigned long  Reserved3;    /* Currently Unused                   */
    struct PageCmd PageCmdList;  /* Currently Unused                   */
}

:ecgraphic.
:font facename=default.
:p.The PageCmd structure has the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct PageCmd {
    unsigned char Cmd;           /* Cmd Code (Read,Write,Verify)       */
    unsigned char Priority;      /* Same values as for req packets     */
    unsigned char Status;        /* Status byte                        */
    unsigned char Error;         /* I24 error code                     */
    unsigned long Addr;          /* Physical(0&colon.32) or Virtual(16&colon.16)   */
    unsigned long FileOffset;    /* Byte Offset in page file  */
}

:ecgraphic.
:font facename=default.
.br 
:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FS_DOPAGEIO performs all the I/O operations specified in the PageCmdList&per.   
:p.If the disk driver Extended Strategy requests, a request list will be built 
from the PageCmdList and issued to the driver&per.   
:p.If the disk driver does not support Extended Strategy requests, the FSD can 
either let the kernel do the emulation (See FS_OPENPAGEFILE to set this state) or has 
the option to do the emulation itself&per.   
:p.For a detailed description of the Extended Strategy request interface please 
see the :hp1.OS/2 Version 2&per.0 Physical Device Driver Reference:ehp1.&per.   
:h2 id=52.FS_EXIT - End of process

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Release FSD resources still held after process termination&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FS_EXIT(uid, pid, pdb);

unsigned short uid;
unsigned short pid;
unsigned short pdb;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.uid is the user ID of the process&per. This will be a valid value&per.   
:p.pid is the process ID of the process&per. This will be a valid value&per.   
:p.pdb is the DOS mode process ID of the process&per. This will be a valid value
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Because all files are closed when a process terminates, this call is not 
needed to release file resources&per. It is, however, useful if resources are being 
held due to unterminated searches (as in searches initiated from the DOS mode)&per.   
:h2 id=53.FS_FILEATTRIBUTE - Query/Set File Attribute

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Query/Set the attribute of the specified file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FILEATTRIBUTE(flag, pcdfsi, pcdfsd, pName, iCurDirEnd,
                                pAttr)

unsigned short flag;
struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;
unsigned short far * pAttr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates retrieval or setting of attributes, with&colon.   
:p.flag == 0 indicates retrieving the attribute&per. 
.br 
flag == 1 indicates setting the attribute&per. 
.br 
flag == all other values, reserved&per.   
:p.The value of flag passed to the FSD will be valid&per.   
:p.pcdfsi is a pointer to the file-system independent portion of an open file 
instance&per.   
:p.pcdfsd is a pointer to the file-system dependent portion of an open file 
instance&per.   
:p.pName is a pointer to the ASCIIZ name of the file or directory&per.   
:p.The FSD does not need to validate this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no current directory relevant to the name text, that is, a device&per.   
:p.pAttr is a pointer to the attribute&per.   
:p.For flag == 0, the FSD should store the attribute in the indicated location
&per. 
.br 
For flag == 1, the FSD should retrieve the attribute from this location and set 
it in the file or directory&per.   
:p.The FSD does not need to validate this pointer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.None   
:h2 id=54.FS_FILEINFO - Query/Set a File's Information

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Returns information for a specific file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FILEINFO(flag, psffsi, psffsd, level, pData, cbData,
                           IOflag)

unsigned short flag;
struct sffsi far * psffsi;
struct sffsd far * psffsd;
unsigned short level;
char far * pData;
unsigned short cbData;
unsigned short IOflag;

:ecgraphic.
:font facename=default.  
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates retrieval or setting of information&per.   
:p.flag == 0 indicates retrieving information&per. 
.br 
flag == 1 indicates setting information&per. 
.br 
All other values are reserved&per.   
:p.The value of flag passed to the FSD will be valid&per.   
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.level is the information level to be returned&per.   
:p.Level selects among a series of data structures to be returned&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area is validated by the kernel (see FSH_PROBEBUF)
&per.   
:p.When retrieval (flag == 0) is specified, the FSD will place the information 
into the buffer&per.   
:p.When outputting information to a file (flag == 1), the FSD will retrieve that 
data from the application buffer&per.   
:p.cbData is the length of the application data area&per.   
:p.For flag == 0, this is the length of the data the application wishes to 
retrieve&per. If there is not enough room for the entire level of data to be returned, 
the FSD will return a BUFFER OVERFLOW error&per.   
:p.For flag == 1, this is the length of data to be applied to the file&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through&per. 
.br 
IOflag == 0x0020 indicates no-cache&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.If setting the time/date/DOS attributes on a file&colon.   
:p.oCopy the new time/date/DOS attributes into the SFT 
.br 
oSet ST_PCREAT, ST_PWRITE, and ST_PREAD 
.br 
oClear ST_SCREAT, ST_SWRITE, and ST_SREAD 
.br 

:p.:hp2.Note&colon.  :ehp2.ALSO NEW FOR 2&per.0, it is suggested that the FSD copy the DOS 
file attributes from the directory entry into the SFT&per. This allows the FSD and 
the OS2 kernel to handle FCB opens more efficiently&per.   
:p.If querying the date/time/DOS attributes on a file, simply copy the date/time
/DOS attributes from the directory entry into the SFT&per.   
:p.Of the information passed in IOflag, the write-through bit is a mandatory bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:p.Supported information levels are described in the :hp1.OS/2 Version 2&per.0 
Control Program Programming Reference:ehp1.&per.   
:h2 id=55.FS_FILEIO - Multi-function file I/O

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Perform multiple lock, unlock, seek, read, and write I/O&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FILEIO (psffsi, psffsd, pCmdList, cbCmdList, poError,
                          IOflag)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
char far * pCmdList;
unsigned short cbCmdList;
unsigned short far * poError;
unsigned short IOflag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.pCmdList is a pointer to a command list that contains entries indicating what 
commands will be performed&per.   
:p.Each individual operation (CmdLock, CmdUnlock, CmdSeek, CmdIO) is performed 
as atomic operations until all are complete or until one fails&per. CmdLock 
executes a multiple range lock as an atomic operation&per. CmdUnlock executes a multiple 
range unlock as an atomic operation&per. Unlike CmdLock, CmdUnlock cannot fail as 
long as the parameters to it are correct, and the calling application had done a 
Lock earlier, so it can be viewed as atomic&per.   
:p.The validity of the user address is not verified (see FSH_PROBEBUF)&per.   
:p.:hp2.For CmdLock, the command format is&colon. :ehp2.  
:cgraphic.
:font facename='Courier' size=12x12.struct CmdLock {
    unsigned short Cmd = 0;   /* 0 for lock operations         */
    unsigned short LockCnt;   /* number of locks that follow   */
    unsigned long  TimeOut;   /* ms time-out for lock success  */
}

:ecgraphic.

:p.which is followed by a series of records of the following format&colon.   
:cgraphic.
struct Lock {
    unsigned short Share = 0;   /* 0 for exclusive, 1 for read-only  */
    long           Start;       /* start of lock region              */
    long           Length;      /* length of lock region             */
}

:ecgraphic.
:font facename=default.
:p.If a lock within a CmdLock causes a time-out, none of the other locks within 
the scope of CmdLock are in force, because the lock operation is viewed as atomic
&per.   
:p.CmdLock&per.TimeOut is the count in milliseconds, until the requesting 
process is to resume execution if the requested locks are not available&per. If CmdLock
&per.TimeOut == 0, there will be no wait&per. If CmdLock&per.TimeOut < 0xFFFFFFFF it 
is the number of milliseconds to wait until the requested locks become available
&per. If CmdLock&per.TimeOut == 0xFFFFFFFF then the thread will wait indefinitely 
until the requested locks become available&per.   
:p.Lock&per.Share defines the type of access other processes may have to the 
file-range being locked&per. If its value == 0, other processes have No-Access to 
the locked range&per.  If its value == 1, other process have Read-Only access to the 
locked range&per.   
:p.:hp2.For CmdUnlock, the command format is&colon. :ehp2.  
:cgraphic.
:font facename='Courier' size=12x12.struct CmdUnlock {
    unsigned short Cmd = 1;     /* 1 for unlock operations        */
    unsigned short UnlockCnt;   /* Number of unlocks that follow  */
}

:ecgraphic.
:font facename=default.
:p.which is followed by a series of records of the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct UnLock {
    long Start;                 /* start of locked region         */
    long Length;                /* length of locked region        */
}

:ecgraphic.
:font facename=default.
:p.:hp2.For CmdSeek, the command format is&colon. :ehp2.  
:cgraphic.
:font facename='Courier' size=12x12.struct CmdSeek {
    unsigned short Cmd = 2;   /* 2 for seek operation         */
    unsigned short Method;    /* 0 for absolute               */
                              /* 1 for relative to current    */
                              /* 2 for relative to EOF        */
    long           Position;  /* file seek position or delta  */
    long           Actual;    /* actual position seeked to    */
}

:ecgraphic.
:font facename=default.
:p.:hp2.For CmdIO, the command format is&colon. :ehp2.  
:cgraphic.
:font facename='Courier' size=12x12.struct CmdIO {
    unsigned short Cmd;        /* 3 for read, 4 for write      */
    void far * Buffer;         /* pointer to the data buffer   */
    unsigned short BufferLen;  /* number of bytes requested    */
    unsigned short Actual;     /* number of bytes transferred  */
}

:ecgraphic.
:font facename=default.
:p.cbCmdList is the length in bytes of the command list&per.   
:p.poError is the offset within the command list of the command that caused the 
error&per.   
:p.This field has a value only when an error occurs&per.   
:p.The validity of the user address has not been verified (see FSH_PROBEBUF)
&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through&per. 
.br 
IOflag == 0x0020 indicates no-cache&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This function provides a simple mechanism for combining the file I/O 
operations into a single request and providing improved performance, particularly in a 
networking environment&per.   
:p.File systems that do not have the FileIO bit in their attribute field do not 
see this call&colon. The command list is parsed by the IFS router&per. The FSD sees 
only FS_CHGFILEPTR, FS_READ, FS_WRITE calls&per.   
:p.File systems that have the FileIO bit in their attribute field see this call 
in its entirety&per. The atomicity guarantee applies only to the commands 
themselves and not to the list as a whole&per.   
:p.Of the information passed in IOflag, the write-through bit is a mandatory bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:h2 id=56.FS_FILELOCKS - Request a file record lock/unlock

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Locks and/or unlocks a range (record)  in a opened file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FILELOCKS(psffsi, psffsd, pUnLockRange, pLockRange, timeout,
                            flags)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
struct filelock far * pUnLockRange;
struct filelock far * pLockRange;
unsigned long timeout;
unsigned long flags;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.pUnLockRange is a pointer to a filelock structure, identifying the range of 
the file to be unlocked&per. The filelock structure has the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct filelock {
    unsigned long FileOffset;   /* offset where the lock/unlock begins  */
    unsigned long RangeLength;  /* length of region locked/unlocked     */
}

:ecgraphic.
:font facename=default.  
:p.If RangeLength is zero, no unlocking is required&per.   
:p.pLockRange is a pointer to a filelock structure, identifying the range of the 
file to be locked&per. If RangeLength is zero, no locking is required&per.   
:p.timeout is the maximum time in milliseconds that the requester wants to wait 
for the requested ranges, if they are not immediately available&per.   
:p.flags is the bit mask which specifies what actions are to taken&colon.   
:p.:hp2.SHARE Bit 0 on :ehp2.indicates other processes can share access to this locked 
range&per. Ranges with SHARE bit on can overlap&per.   
:p.:hp2.SHARE Bit 0 off :ehp2.indicates the current process has exclusive access to the 
locked range&per. A range with the SHARE bit off CANNOT overlap with any other lock 
range&per.   
:p.:hp2.ATOMIC Bit 1 :ehp2.on indicates an atomic lock request&per. If the lock range 
equals the unlock range, an atomic lock will occur&per. If the ranges are not equal, 
an error will be returned&per.   
:p.All other bits (2-31) are reserved and must be zero&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This entry point was added to support the 32-bit DosSetFileLocks API&per.   
:p.If the lock and unlock range lengths are both zero, an error, ERROR_LOCK_
VIOLATION will be returned to the caller&per. If only a lock is desired, pUnLockRange can 
be NULL or both FileOffset and RangeLength should be set to zero when the call is 
made&per. The opposite is true for an unlock&per.   
:p.When the atomic bit is not set, the unlock occurs first then the lock is 
performed&per. If an error occurs on the unlock, an error is returned and the lock is not 
performed&per. If an error occurs on the lock, an error is returned and the unlock 
remains in effect if one was requested&per. If the atomic bit is set and the unlock 
range equals the lock range and the unlock range has shared access but wants to 
change the access to exclusive access, the function is atomic&per. FSDs may not 
support atomic lock functions&per. If error ERROR_ATOMIC_LOCK_NOT_SUPPORTED is returned
, the application should do an unlock and lock the range using nonatomic 
operations&per. The application should also be sure to refresh its internal buffers prior 
to making any modifications&per.   
:p.Closing a file with locks still in force causes the locks to be released in 
no defined order&per.   
:p.Terminating a process with a file open and having issued locks on that file 
causes the file to be closed and the locks to be released in no defined order&per.   
:p.The figure below describes the level of access granted when the accessed 
region is locked&per. The locked regions can be anywhere in the logical file&per. 
Locking beyond end-of-file is not an error&per. It is expected that the time in which 
regions are locked will be short&per. Duplicating the handle duplicates access to the 
locked regions&per. Access to the locked regions is not duplicated across the 
DosExecPgm system call&per. The proper method for using locks is not to rely on being 
denied read or write access, but attempting to lock the region desired and examining 
the error code&per.   
:p.:hp2.Locked Access Table :ehp2.:font facename='Courier' size=12x12.
:cgraphic.
:color fc=default.:color bc=default.
 Action               Exclusive Lock                 Shared Lock
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 Owner read           Success                        Success
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 Non-owner read       Return code, not block         Success
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 Owner write          Success                        Return code, not block
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 Non-owner write      Return code, not block         Return code, not block


:ecgraphic.
:font facename=default.
:p.The locked access table has the actions on the left as to whether owners or 
non-owners of a file do either reads or writes of files that have exclusive or 
shared locks set&per. A range to be locked for exclusive access must first be cleared 
of any locked subranges or locked any locked subranges or locked overlapping 
ranges&per.   
:h2 id=57.FS_FINDCLOSE - Directory Read (Search) Close

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Provides the mechanism for an FSD to release resources allocated on behalf of 
FS_FINDFIRST and FS_FINDNEXT&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDCLOSE(pfsfsi, pfsfsd)

struct fsfsi far * pfsfsi;
struct fsfsd far * pfsfsd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pfsfsi is a pointer to the file-system-independent file search structure&per.   
:p.The FSD should not update this structure&per.   
:p.pfsfsd is a pointer to the file-system-dependent file search structure&per.   
:p.The FSD may use this to store information about continuation of its search
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.DosFindClose has been called on the handle associated with the search buffer
&per. Any file system related information may be released&per.   
:p.If FS_FINDFIRST for a particular search returns an error, an FS_FINDCLOSE for 
that search will not be issued&per.   
:h2 id=58.FS_FINDFIRST - Find First Matching File Name

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Find first occurrence of a file name in a directory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDFIRST(pcdfsi, pcdfsd, pName, iCurDirEnd, attr, pfsfsi,
                            pfsfsd, pData, cbData, pcMatch, level, flags)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;
unsigned short attr;
struct fsfsi far * pfsfsi;
struct fsfsd far * pfsfsd;
char far * pData;
unsigned short cbData;
unsigned short far * pcMatch;
unsigned short level;
unsigned short flags;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pName is a pointer to the ASCIIZ name of the file or directory&per.   
:p.Wildcard characters are allowed only in the last component&per. The FSD does 
not need to validate this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1 there 
is no current directory relevant to the name text, that is, a device&per.   
:p.attr is a bit field that governs the match&per.   
:p.Any directory entry whose attribute bit mask is a subset of attr and whose 
name matches that in pName should be returned&per. For example, an attribute of 
system and hidden is passed in&per. A file with the same name and an attribute of 
system is found&per. This file is returned&per. A file with the same name and no 
attributes (a regular file) is also returned&per. The attributes read-only and file-
archive will not be passed in and should be ignored when comparing directory attributes
&per.   
:p.The value of attr passed to the FSD will be valid&per. The bit 0x0040 
indicates a non-8&per.3 filename format&per. It should be treated the same way as system 
and hidden attributes are&per.   
:p.pfsfsi is a pointer to the file-system-independent file-search structure&per.   
:p.The FSD should not update this structure&per.   
:p.pfsfsd is a pointer to the file-system-dependent file-search structure&per.   
:p.The FSD may use this to store information about continuation of the search
&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. The FSD will fill in this area with a set of packed, variable- length 
structures that contain the requested data and matching file name&per.   
:p.cbData is the length of the application data area in bytes&per.   
:p.pcMatch is a pointer to the number of matching entries&per.   
:p.The FSD returns, at most, this number of entries; the FSD returns in this 
parameter the number of entries actually placed in the data area&per.   
:p.The FSD does not need to validate this pointer&per.   
:p.level is the information level to be returned&per.   
:p.Level selects among a series of data structures to be returned&per. The level 
passed to the FSD is valid&per.   
:p.flags indicates whether to return file-position information&per.   
:p.flags == 0 indicates that file-position information should not be returned 
and the information format described under DosFindFirst should be used&per. 
.br 
flags == 1 indicates that file-position information should be returned and the 
information format described below should be used&per.   
:p.The flag passed to the FSD has a valid value&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.For flags == 1, the FSD must store in the first DWORD of the per-file 
attributes structure adequate information to allow the search to be resumed from the file 
by calling FS_FINDFROMNAME&per. For example, an ordinal representing the file
&apos.s position in the directory could be stored&per. If the filename must be used to 
restart the search, the DWORD may be left blank&per.   
:p.For level 0x0001 and flags == 0, directory information for FS_FINDFIRST is 
returned in the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct FileFindBuf {
    unsigned short dateCreate;
    unsigned short timeCreate;
    unsigned short dateAccess;
    unsigned short timeAccess;
    unsigned short dateWrite;
    unsigned short timeWrite;
    long           cbEOF;
    long           cbAlloc;
    unsigned short attr;
    unsigned char  cbName;
    unsigned char  szName[];
}

:ecgraphic.
:font facename=default.
:p.For level 0x0001 and flags == 1, directory information for FS_FINDFIRST is 
returned in the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct FileFromFindBuf {
    long           position;    /* position given to FSD on following */
                                /* FS_FINDFROMNAME call               */
    unsigned short dateCreate;
    unsigned short timeCreate;
    unsigned short dateAccess;
    unsigned short timeAccess;
    unsigned short dateWrite;
    unsigned short timeWrite;
    long           cbEOF;
    long           cbAlloc;
    unsigned short attr;
    unsigned char  cbName;
    unsigned char  szName[];
}

:ecgraphic.
:font facename=default.
:p.The other information levels have similar format, with the position the first 
field in the structure for flags == 1&per.   
:p.If the non-8&per.3 filename format bit is set in the attributes of a file 
found by FS_FINDFIRST/NEXT/FROMNAME, it must be turned off in the copy of the 
attributes returned in pData&per.   
:p.If FS_FINDFIRST for a particular search returns an error, an FS_FINDCLOSE for 
that search will not be issued&per.   
:p.Sufficient information to find the next matching directory entry must be 
saved in the fsfsd data structure&per.   
:p.In the case where directory entry information overflows the pData area, the 
FSD should be able to continue the search from the entry which caused the overflow 
on the next FS_FINDNEXT or FS_FINDFROMNAME&per.   
:p.In the case of a global search in a directory, the first two entries in that 
directory as reported by the FSD should be &apos.&per.&apos. and &apos.&per.&per.&apos. (
current and the parent directories&per.   
:p.The example above just shows the effect of flags == 1 on a level 1 filefind 
record; level 2 and level 3 filefind records are similarly affected&per.   
:p.:hp2.Note&colon.   :ehp2.The FSD will be called with the FINDFIRST/FINDFROMNAME interface 
when the 32-bit DosFindFirst/DosFindNext APIs are called&per. THIS IS A CHANGE FROM 
1&per.X IFS interface for redirector FSDs&per. The kernel will also be massaging 
the find records so that they appear the way the caller expects&per. Redirectors 
who have to resume searches should take this information into account&per. (i&per.e
&per. You might want to reduce the size of the buffer sent to the server, so that the 
position fields can be added to the beginning of all the find records)&per.   
:h2 id=59.FS_FINDFROMNAME - Find matching file name starting from name

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Find occurrence of a file name in a directory starting from a position or 
name&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDFROMNAME(pfsfsi, pfsfsd, pData, cbData, pcMatch, level,
                               position, pName, flags)

struct fsfsi far * pfsfsi;
struct fsfsd far * pfsfsd;
char far * pData;
unsigned short cbData;
unsigned short far * pcMatch;
unsigned short level;
unsigned long position;
char far * pName;
unsigned short flags;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pfsfsi is a pointer to the file-system-independent file search structure&per. 
The FSD should not update this structure&per.   
:p.pfsfsd is a pointer to the file-system-dependent file search structure&per. 
The FSD may use this to store information about continuation of the search&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area has not been validated by the kernel (see FSH_
PROBEBUF)&per. The FSD will fill in this area with a set of packed, variable- length 
structures that contain the requested data and matching file names in the format required 
for DosFindFirst/DosFindNext&per.   
:p.cbData is the length of the application data area in bytes&per.   
:p.pcMatch is a pointer to the number of matching entries&per. The FSD will 
return at most this number of entries&per. The FSD will store into it the number of 
entries actually placed in the data area&per. The FSD does not need to validate this 
pointer&per.   
:p.level is the information level to be returned&per. Level selects among a 
series of structures of data to be returned&per. The level passed to the FSD is valid
&per.   
:p.position is the file-system-specific information about where to restart the 
search from&per. This information was returned by the FSD in the ResultBuf for an FS_
FINDFIRST/FS_FINDNEXT/FS_FINDFROMNAME call&per.   
:p.pName is the filename from which to continue the search&per. The FSD does not 
need to validate this pointer&per.   
:p.flags indicates whether to return file position information&per. The flag 
passed to the FSD has a valid value&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The FSD may use the position or filename to determine the position > from 
which to resume the directory search&per. The FSD need not return position if it uses 
name and vice versa&per.   
:p.For flags == 1, the FSD must store in the position field adequate information 
to allow the search to be resumed from the file by calling FS_FINDFROMNAME&per. 
See FS_FINDFIRST for a description of the data format&per.   
:p.The FSD must ensure that enough information is stored in the fsfsd data 
structure to enable it to continue the search&per.   
:p.:hp2.Note&colon.   :ehp2.The FSD will be called with the FINDFIRST/FINDFROMNAME interface 
when the 32-bit DosFindFirst/DosFindNext APIs are called&per. THIS IS A CHANGE FROM 
1&per.X IFS interface for redirector FSDs&per. The kernel will also be massaging 
the find records so that they appear the way the caller expects&per. Redirectors 
who have to resume searches should take this information into account&per. (i&per.e
&per.  You might want to reduce the size of the buffer sent to the server, so that the 
position fields can be added to the beginning of all the find records)&per.   
:h2 id=60.FS_FINDNEXT - Find next matching file name.

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Find the next occurrence of a file name in a directory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDNEXT(pfsfsi, pfsfsd, pData, cbData, pcMatch, level,
                           flags)

struct fsfsi far * pfsfsi;
struct fsfsd far * pfsfsd;
char far * pData;
unsigned short cbData;
unsigned short far * pcMatch;
unsigned short level;
unsigned short flags;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pfsfsi is a pointer to the file-system-independent file-search structure&per. 
The FSD should not update this structure&per.   
:p.pfsfsd is a pointer to the file-system-dependent file-search structure&per. 
The FSD may use this to store information about continuation of the search&per.   
:p.pData is the address of the application area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. The FSD fills in this area with a set of packed, variable- length 
structures that contain the requested data and matching file names&per.   
:p.cbData is the length of the application data area in bytes&per.   
:p.pcMatch is a pointer to the number of matching entries&per.   
:p.The FSD returns, at most, this number of entries&per. The FSD returns the the 
number of entries actually placed in the data area in this parameter&per.   
:p.The FSD does not need to validate this pointer&per.   
:p.level is the information level to be returned&per.  Level selects among a 
series of structures of data to be returned&per. The level passed to the FSD is valid
&per.   
:p.flags indicates whether to return file-position information&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.For flags == -1, the FSD must store in the position field adequate 
information to allow the search to be resumed from the file by calling FS_FINDFROMNAME&per. 
See FS_FINDFIRST for a description of the data format&per.   
:p.The level passed to FS_FINDNEXT is the same level as that passed to FS_
FINDFIRST to initiate the search&per.   
:p.Sufficient information to find the next matching directory entry must be 
saved in the fsfsd data structure&per.   
:p.The FSD should take care of the case where the pData area overflow may occur
&per. FSDs should be able to start the search from the same entry for the next FS_
FINDNEXT as the one for which the overflow occurred&per.   
:p.In the case of a global search in a directory, the first two entries in that 
directory as reported by the FSD should be &apos.&per.&apos. and &apos.&per.&per.&apos. (
current and parent directories)&per.   
:h2 id=61.FS_FINDNOTIFYCLOSE - Close Find-Notify Handle

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Closes the association between a Find-Notify handle and a DosFindNotifyFirst 
or DosFindNotifyNext function&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDNOTIFYCLOSE(handle)

unsigned short handle;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.handle is the directory handle&per.   
:p.This handle was returned by the FSD on a previous FS_FINDNOTIFYFIRST or FS_
FINDNOTIFYNEXT call&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Provides the mechanism for an FSD to release resources allocated on behalf of 
FS_FINDNOTIFYFIRST and FS_FINDNOTIFYNEXT&per.   
:p.FS_FINDNOTIFYFIRST returns a handle to the find-notify request&per. FS_
FINDNOTIFYCLOSE closes the handle associated with that find-notify request and releases file 
system information related to that handle&per.   
:h2 id=62.FS_FINDNOTIFYFIRST - Monitor a directory for changes.

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Start monitoring a directory for changes&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDNOTIFYFIRST(pcdfsi, pcdfsd, pName, iCurDirEnd, attr,
                                  pHandle, pData, cbData, pcMatch, level,
                                  timeout)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;
unsigned short attr;
unsigned short far * pHandle; char far * pData;
unsigned short cbData;
unsigned short far * pMatch;
unsigned short level;
unsigned long timeout;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pName is a pointer to the ASCIIZ name of the file or directory&per.   
:p.Wildcard characters are allowed only in the last component&per. The FSD does 
not need to verify this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1 there 
is no current directory relevant to the name text, that is, a device&per.   
:p.attr is the bit field that governs the match&per.   
:p.Any directory entry whose attribute bit mask is a subset of attr and whose 
name matches that in pName should be returned&per. See FS_FINDFIRST for an 
explanation&per.   
:p.pHandle is a pointer to the handle for the find-notify request&per.   
:p.The FSD allocates a handle for the find-notify request, that is, a handle to 
the directory monitoring continuation information, and stores it here&per. This 
handle is passed to FS_FINDNOTIFYNEXT to continue directory monitoring&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. The FSD fills in this area with a set of packed, variable- length 
structures that contain the requested data and matching file names&per.   
:p.cbData is the length of the application data area in bytes&per.   
:p.pcMatch is a pointer to the number of matching entries&per.   
:p.The FSD returns, at most, this number of entries&per. The FSD returns in this 
parameter the number of entries actually placed in the data area&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.level is the information level to be returned&per.   
:p.Level selects among a series of data structures to be returned&per. See the 
description of DosFindNotifyFirst in the :hp1.OS/2 Version 2&per.0 Control Program Programming 
Reference :ehp1.for more information&per.   
:p.The level passed to the FSD is valid&per.   
:p.timeout is the time-out in milliseconds&per.   
:p.The FSD waits until either the time-out has expired, the buffer is full, or 
the specified number of entries has been returned before returning to the caller
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.None&per.   
:h2 id=63.FS_FINDNOTIFYNEXT - Resume reporting directory changes

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Resume reporting of changes to a file or directory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FINDNOTIFYNEXT(handle, pData, cbData, pcMatch, level,
                                 timeout)

unsigned short handle;
char far * pData;
unsigned short cbData;
unsigned short far * pcMatch;
unsigned short level;
unsigned long timeout;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.handle is the handle to the find-notify request&per.   
:p.This handle was returned by the FSD and is associated with a previous FS_
FINDNOTIFYFIRST or FS_FINDNOTIFYNEXT call&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. The FSD fills in this area with a set of packed, variable- length 
structures that contain the requested data and matching file names&per.   
:p.cbData is the length of the application data area in bytes&per.   
:p.pcMatch is a pointer to the number of matching entries&per.   
:p.The FSD returns, at most, this number of entries&per. The FSD returns in this 
parameter the number of entries actually placed in the data area&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.level is the information level to be returned&per.   
:p.Level selects among a series of data structures to be returned&per. See the 
description of DosFindNotifyFirst in the :hp1.OS/2 Version 2&per.0 Control Program Programming 
Reference :ehp1.for more information&per.   
:p.The level passed to the FSD is valid&per.   
:p.timeout is the time-out in milliseconds&per.   
:p.The FSD waits until either the time-out has expired, the buffer is full, or 
the specified number of entries has been returned before returning to the caller
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.pcMatch is the number of changes required to directories or files that match 
the pName target and attr specified during a related, previous FS_FINDNOTIFYFIRST
&per. The file system uses this field to return the number of changes that actually 
occurred since the issue of the present FS_FINDNOTIFYNEXT&per.   
:p.The level passed to FS_FINDNOTIFYNEXT is the same level as that passed to FS_
FINDNOTIFYFIRST to initiate the search&per.   
:h2 id=64.FS_FLUSHBUF - Commit file buffers

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Flushes cache buffers for a specific volume&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FLUSHBUF(hVPB, flag)

unsigned short hVPB;
unsigned short flag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.hVPB is the handle to the volume for flush&per.   
:p.flag is used to indicate discarding of cached data&per.   
:p.flag == 0 indicates cached data may be retained&per. 
.br 
flag == 1 indicates the FSD will discard any cached data after flushing it to 
the specified volume&per.   
:p.All other values are reserved&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.None&per.   
:h2 id=65.FS_FSCTL - File System Control

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Allow an extended standard interface between an application and a file system 
driver&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FSCTL(pArgdat, iArgType, func, pParm, lenParm, plenParmIO,
                        pData, lenData, plenDataIO)

union argdat far * pArgDat;
unsigned short iArgType;
unsigned short func;
char far * pParm;
unsigned short lenParm;
unsigned short far * plenParmIO;
char far * pData;
unsigned short lenData;
unsigned short far * plenDataIO;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pArgDat is a pointer to the union whose contents depend on iArgType&per. The 
union is defined as follows&colon.   
:cgraphic.
:font facename='Courier' size=12x12.union argdat {

    /* pArgType = 1, FileHandle directed case */

    struct sf {
        struct sffsi far * psffsi;
        struct sffsd far * psffsd;
    };

    /* pArgType = 2, Pathname directed case */

    struct cd {
        struct cdfsi far * pcdfsi;
        struct cdfsd far * pcdfsd;
        char far *         pPath;
        unsigned short     iCurDirEnd;
    };

    /* pArgType = 3, FSD Name directed case */
    /* pArgDat is Null                      */
};

:ecgraphic.
:font facename=default.
:p.iArgType indicates the argument type&per.   
:p.iArgType = 1 
.br 
means that pArgDat->sf&per.psffsi and pArgDat->sf&per.psffsd point to an sffsi 
and sffsd, respectively&per.   
:p.iArgType = 2 
.br 
means that pArgDat->cd&per.pcdfsi and pArgDat->cd&per.pcdfsd point to a cdfsi 
and cdfsd, pArgDat->cd&per.pPath points to a canonical pathname, and pArgDat->cd
&per.iCurDirEnd gives the index of the end of the current directory in pPath&per. The 
FSD does not need to verify the pPath pointer&per.   
:p.iArgType = 3 
.br 
means that the call was FSD name routed, and pArgDat is a NULL pointer&per. 
:p.func indicates the function to perform&per.   
:p.func == 1 indicates a request for new error code information&per. 
.br 
func == 2 indicates a request for the maximum EA size and EA list size supported 
by the FSD&per.   
:p.pParm is the address of the application input parameter area&per.   
:p.Addressing of this data area has not been validated by the kernel (see FSH_
PROBEBUF)&per.   
:p.lenParm is the maximum length of the application parameter area (pParm)&per.   
:p.plenParmIO On input, contains the length in bytes of the parameters being 
passed in to the FSD in pParm&per. On return, contains the length in bytes of data 
returned in pParm by the FSD&per. The length of the data returned by the FSD in pParm 
must not exceed the length in lenParm&per. Addressing of this area is not validated 
by the kernel (see FSH_PROBEBUF)&per.   
:p.pData is the address of the application output data area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per.   
:p.lenData is the maximum length of the application output data area (pData)
&per.   
:p.plenDataIO On input, contains the length in bytes of the data being passed in 
to the FSD in pData&per. On return, contains the length in bytes of data returned 
in pData by the FSD&per. The length of the data returned by the FSD in pData must 
not exceed the length in lenData&per. Addressing of this area is not validated by 
the kernel (see FSH_PROBEBUF)&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The accessibility of the parameter and data buffers has not been validated by 
the kernel&per. FS_PROBEBUF must be used&per.   
:p.All FSDs must support func == 1 to return new error code information and func 
== 2 to return the limits of the EA sizes&per.   
:p.For func == 1, the error code is passed to the FSD in the first WORD of the 
parameter area&per. On return, the first word of the data area contains the length of the 
ASCIIZ string containing an explanation of the error code&per. The data area contains 
the ASCIIZ string beginning at the second WORD&per.   
:p.For func == 2, the maximum EA and EA list sizes supported by the FSD are 
returned in the buffer pointed to by pData in the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct EASizeBufStruc {
    unsigned short easb_MaxEASize;     /* Max size of an individual EA */
    unsigned long easb_MaxEAListSize;  /* Max full EA list size        */
}

:ecgraphic.
:font facename=default.  
:h2 id=66.FS_FSINFO - File System Information

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Returns or sets information for a file system device&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_FSINFO(flag, hVPB, pData, cbData, level)

unsigned short flag;
unsigned short hVPB;
char far * pData;
unsigned short cbData;
unsigned short level;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates retrieval or setting of information&per.   
:p.flag == 0 indicates retrieving information&per. 
.br 
flag == 1 indicates setting information on the media&per. 
.br 
All other values are reserved&per.   
:p.hVPB is the handle to the volume of interest&per.   
:p.pData is the address of the application output data area&per.   
:p.Addressing of this data area has not been validated by the kernel (see (FSH_
PROBEBUF)&per.   
:p.cbData is the length of the application data area&per.   
:p.For flag == 0, this is the length of the data the application wishes to 
retrieve&per. If there is not enough room for the entire level of data to be returned, 
the FSD will return a BUFFER OVERFLOW error&per. For flag == 1, this is the length 
of the data to be sent to the file system&per.   
:p.level is the information level to be returned&per.   
:p.Level selects among a series of structures of data to be returned or set&per. 
See DosQFSInfo and DosSetFSInfo for information&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.None&per.   
:h2 id=67.FS_INIT - File system driver initialization

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Request file system driver initialization&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_INIT(szParm, DevHelp, pMiniFSD)

char far * szParm;
unsigned long DevHelp;
unsigned long far * pMiniFSD;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.szParm is a pointer to the ASCIIZ parameters following the CONFIG&per.SYS IFS
= command that loaded the FSD&per. The FSD does not need to verify this pointer
&per.   
:p.DevHelp is the address of the kernel entry point for the DevHelp routines
&per.   
:p.This is used exactly as the device driver DevHelp address, and can be used by 
an FSD that needs access to some of the device helper services&per.   
:p.pMiniFSD is a pointer to data passed between the mini-FSD and the FSD, or 
null&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This call is made during system initialization to allow the FSD to perform 
actions necessary for beginning operation&per. The FSD may successfully initialize by 
returning 0 or may reject installation (invalid parameters, incompatible hardware, etc
&per.) by returning the appropriate error code&per. If rejection is selected, all FSD 
selectors and segments are released&per.   
:p.pMiniFSD will be null, except when booting from a volume managed by an FSD 
and the exported name of the FSD matches the exported name of the mini-FSD&per. In 
this case, pMiniFSD will point to data established by the mini-FSD (See MFS_INIT)
&per.   
:h2 id=68.FS_IOCTL - I/O Control for Devices

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Perform control function on the device specified by the opened device handle
&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_IOCTL(psffsi, psffsd, cat, func, pParm, lenMaxParm,
                        plenInOutParm, pData, lenMaxData, plenInOutData)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
unsigned short cat;
unsigned short func;
char far * pParm;
unsigned short lenMaxParm;
unsigned short * plenInOutParm;
char far * pData;
unsigned short lenMaxData;
unsigned short * plenInOutData;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.cat is the category of the function to be performed&per.   
:p.func is the function within the category to be performed&per.   
:p.pParm is the address of the application input parameter area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. A null value indicates that the parameter is unspecified for this function
&per. lenMaxParm is the byte length of the application input parameter area&per.   
:p.If lenMaxParm is 0, *plenInOutParm is 0, and pParm is not null, it means that 
the data buffer length is unknown due to the request being submitted via an old 
IOCTL or DosDevIOCtl interface&per.   
:p.plenInOutParm is the pointer to an unsigned short that contains the length of 
the parameter area in use on input and is set by the FSD to be the length of the 
parameter area in use on output&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. A null value indicates that the parameter is unspecified for this function
&per.   
:p.pData is the address of the application output data area&per.   
:p.Addressing of this data area has not been validated by the kernel (see FSH_
PROBEBUF)&per. A null value indicates that the parameter is unspecified for this 
function&per.   
:p.lenMaxData is the byte length of the application output data area&per.   
:p.If lenMaxData is 0, *plenInOutData is 0, and pData is not null, it means that 
the data buffer length is unknown due to the request being submitted via an old 
IOCTL or DosDevIOCtl interface&per.   
:p.plenInOutData is the pointer to an unsigned short that contains the length of 
the data area in use on input and is set by the FSD to be the length of the data 
area in use on output&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. A null value indicates that the parameter is unspecified for this function
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.:hp2.Note&colon.   :ehp2.This entry point&apos.s parameter list definition has changed 
from the 1&per.x IFS document&per. If the parameters plenInOutParm and plenInOutData 
are null, use the lenMax parameters as the buffer sizes sent to any file system 
helper&per.   
:h2 id=69.FS_MKDIR - Make Subdirectory

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Create the specified directory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_MKDIR(pcdfsi, pcdfsd, pName, iCurDirEnd, pEABuf, flags)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;
char far * pEABuf;
unsigned short flags;

:ecgraphic.
:font facename=default.  
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pName is a pointer to the ASCIIZ name of the directory to be created&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no current directory relevant to the name text, that is, a device&per.   
:p.pEABuf is a pointer to the extended attribute buffer&per.   
:p.This buffer contains attributes that will be set upon creation of the new 
directory&per. If NULL, no extended attributes are to be set&per. Addressing of this data 
area has not been validated by the kernel (see FSH_PROBEBUF)&per.   
:p.flags indicates the name type&per.   
:p.Flags == 0x0040 indicates a non-8&per.3 filename format&per. All other values 
are reserved&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The FSD needs to do the time stamping itself&per. There is no aid in the 
kernel for time stamping sub-directories&per. FAT only supports creation time stamp 
and sets the other two fields to zeroes&per. An FSD should do the same&per. The FSD 
can obtain the current time/date from the infoseg&per.   
:p.A new directory called pName should be created if possible&per. The standard 
directory entries &apos.&per.&apos. and &apos.&per.&per.&apos. should be put into the 
directory&per.   
:p.The non-8&per.3 filename format attribute in the directory entry should be 
set according to the value in flags&per.   
:h2 id=70.FS_MOUNT - Mount/unmount volumes

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Examination of a volume by an FSD to see if it recognizes the file system 
format&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_MOUNT(flag, pvpfsi, pvpfsd, hVPB, pBoot)

unsigned short flag;
struct vpfsi far * pvpfsi;
struct vpfsd far * pvpfsd;
unsigned short hVPB;
char far * pBoot;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates operation requested&per.   
:p.flag == 0 indicates that the FSD is requested to mount or accept a volume
&per.   
:p.flag == 1 indicates that the FSD is being advised that the specified volume 
has been removed&per.   
:p.flag == 2 indicates that the FSD is requested to release all internal storage 
assigned to that volume as it has been removed from its driver and the last kernel-
managed reference to that volume has been removed&per.   
:p.flag == 3 indicates that the FSD is requested to accept the volume regardless 
of recognition in preparation for formatting for use with the FSD&per.   
:p.All other values are reserved&per.   
:p.The value passed to the FSD will be valid&per.   
:p.pvpfsi is a pointer to the file-system-independent portion of VPB&per.   
:p.If the media contains an OS/2-recognizable boot sector, then the vpi_vid 
field contains the 32-bit identifier for that volume&per. If the media does not 
contain such a boot sector, the FSD must generate a unique label for the media and 
place it into the vpi_vid field&per.   
:p.pvpfsd is a pointer to the file-system-dependent portion of VPB&per.   
:p.The FSD may store information as necessary into this area&per.   
:p.hVPB is the handle to the volume   
:p.pBoot is a pointer to sector 0 read from the media&per.   
:p.This pointer is only valid when flag == 0&per.The buffer the pointer refers 
to must not be modified&per. The pointer is always valid and does not need to be 
verified when flag == 0&per. If a read error occurred, the buffer will contain zeroes
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The FSD examines the volume presented and determine whether it recognizes the 
file system&per. If it does, it returns zero, after having filled in appropriate 
parts of the vpfsi and vpfsd data structures&per. The vpi_vid and vpi_text fields 
must be filled in by the FSD&per. If the FSD has an OS/2 format boot sector, it must 
convert the label from the media into ASCIIZ form&per.  The vpi_hDev field is filled in 
by OS/2&per. If the volume is unrecognized, the driver returns non-zero&per.   
:p.The vpi_text and vpi_vid must be updated by the FSD each time these values 
change&per.   
:p.The contents of the vpfsd data structure are as follows&colon.   
:p.FLAG = 0 The FSD is expected to issue an FSD_FINDDUPHVPB to see if a 
duplicate VPB exists&per. If one does exist, the VPB fs dependent area of the new VPB is 
invalid and the new VPB will be unmounted after the FSD returns from the MOUNT&per. The 
FSD is expected to update the FS dependent area of the old duplicate VPB&per. If no 
duplicate VPB exists, the FSD should initialize the FS dependent area&per.   
:p.FLAG = 1 VPB FS dependent part is same as when FSD last modified it&per.   
:p.FLAG = 2 VPB FS dependent part is same as when FSD last modified it&per. 
.br 

:p.After media recognition time, the volume parameters may be examined using the 
FSH_GETVOLPARM call&per. The volume parameters should not be changed after media 
recognition time&per.   
:p.During a mount request, the FSD may examine other sectors on the media by 
using FSH_DOVOLIO to perform the I/O&per. If an uncertain-media return is detected, 
the FSD is expected to clean up and return an UNCERTAIN MEDIA error in order to 
allow the volume mount logic to restart on the newly-inserted media&per. The FSD must 
provide the buffer to use for additional I/O&per.   
:p.The OS/2 kernel manages the VPB through a reference count&per. All volume- 
specific objects are labeled with the appropriate volume handle and represent references 
to the VPB&per. When all kernel references to a volume disappear, FS_MOUNT is 
called with flag == 2, indicating a dismount request&per.   
:p.When the kernel detects that a volume has been removed from its driver, but 
there are still outstanding references to the volume, FS_MOUNT is called with flag == 
1 to allow the FSD to drop clean (or other regenerable) data for the volume&per. 
Data which is dirty and cannot be regenerated should be kept so that it may be 
written to the volume when it is remounted in the drive&per.   
:p.When a volume is to be formatted for use with an FSD, the kernel calls the 
FSD&apos.s FS_MOUNT entry point with flag == 3 to allow the FSD to prepare for the 
format operation&per. The FSD should accept the volume even if it is not a volume of 
the type that FSD recognizes, since the point of format is to change the file 
system on the volume&per. The operation may fail if formatting does not make sense
&per. (For example, an FSD which supports only CD-ROM&per.)   
:p.Since the hardware does not allow for kernel-mediated removal of media, it is 
certain that the unmount request is issued when the volume is not present in any drive
&per.   
:h2 id=71.FS_MOVE - Move a file or subdirectory

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Moves (renames) the specified file or subdirectory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_MOVE(pcdfsi, pcdfsd, pSrc, iSrcCurDirEnd, pDst,
                       iDstCurDirEnd, flags)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pSrc;
unsigned short iSrcCurDirEnd;
char far * pDst;
unsigned short iDstCurDirEnd;
unsigned short flags;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pSrc is a pointer to the ASCIIZ name of the source file or directory&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.iSrcCurDirEnd is the index of the end of the current directory in pSrc&per.   
:p.This is used to optimize FSD path processing&per. If iSrcCurDirEnd == -1 
there is no current directory relevant to the source name text&per.   
:p.pDst is a pointer to the ASCIIZ name of the destination file or directory
&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.iDstCurDirEnd is the index of the end of the current directory in pDst&per.   
:p.This is used to optimize FSD path processing&per. If iDstCurDirEnd == -1 
there is no current directory relevant to the destination name text&per.   
:p.flags indicates destination name type&per.   
:p.Flags == 0x0040 indicates non-8&per.3 filename format&per. All other values 
are reserved&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The file specified in filename should be moved to or renamed as the 
destination filename, if possible&per.   
:p.Neither the source nor the destination filename may contain wildcard 
characters&per.   
:p.FS_MOVE may be used to change the case in filenames&per.   
:p.The non-8&per.3 filename format attribute in the directory entry for the 
destination name should be set according to the value in flags&per.   
:p.In the case of a subdirectory move, the system does the following checking
&colon.   
:p.oNo files in this directory or its subdirectories are open&per. 
.br 
oThis directory or any of its subdirectories is not the current directory for 
any process in the system&per. 
.br 

:p.In addition, the system also checks for circularity in source and target 
directory names; that is, the source directory is not a prefix of the target directory
&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=72.FS_NEWSIZE - Change File's Logical Size

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Changes a file&apos.s logical (EOD) size&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_NEWSIZE(psffsi, psffsd, len, IOflag)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
unsigned long len;
unsigned short IOflag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.len is the desired new length of the file&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through&per. 
.br 
IOflag == 0x0020 indicates no-cache&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The FSD should return an error if an attempt is made to write beyond the end 
with a direct access device handle&per.   
:p.The file system driver attempts to set the size (EOD) of the file to newsize 
and update sfi_size, if successful&per. If the new size is larger than the 
currently allocated size, the file system driver arranges for for efficient access to the 
newly-allocated storage&per.   
:p.Of the information passed in IOflag, the write-through bit is a mandatory bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:h2 id=73.FS_NMPIPE - Do a remote named pipe operation.

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Perform a special purpose named pipe operation remotely&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_NMPIPE(psffsi, psffsd, OpType, pOpRec, pData, pName)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
unsigned short OpType;
union npoper far * pOpRec;
char far * pData;
char far * pName;

:ecgraphic.
:font facename=default.  
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.OpType is the operation to be performed&per. This parameter has the following 
values&colon.   
:cgraphic.
:font facename='Courier' size=12x12.NMP_GetPHandState    0x21
NMP_SetPHandState    0x01
NMP_PipeQInfo        0x22
NMP_PeekPipe         0x23
NMP_ConnectPipe      0x24
NMP_DisconnectPipe   0x25
NMP_TransactPipe     0x26
NMP_ReadRaw          0x11
NMP_WriteRaw         0x31
NMP_WaitPipe         0x53
NMP_CallPipe         0x54
NMP_QNmPipeSemState  0x58

:ecgraphic.
:font facename=default.
:p.pOpRec is the data record which varies depending on the value of OpType&per. 
The first parameter in each structure encodes the length of the parameter block
&per. The second parameter, if non-zero, indicates that the pData parameter is 
supplied and gives its length&per. The following record formats are used&colon.   
:cgraphic.
:font facename='Courier' size=12x12.union npoper {
    struct phs_param phs;
    struct npi_param npi;
    struct npr_param npr;
    struct npw_param npw;
    struct npq_param npq;
    struct npx_param npx;
    struct npp_param npp;
    struct npt_param npt;
    struct qnps_param qnps;
    struct npc_param npc;
    struct npd_param npd;
};

/* Get/SetPhandState parameter block */

struct phs_param {
    short phs_len;
    short phs_dlen;
    short phs_pmode; /* pipe mode set or returned */
};

/* DosQNmPipeInfo parameter block */
struct npi_param {
    short npi_len;
    short npi_dlen;
    short npi_level; /* information level desired */
};

/* DosRawReadNmPipe parameters */
/* data is buffer addr         */

struct npr_param {
    short npr_len;
    short npr_dlen;
    short npr_nbyt; /* number of bytes read */
};


/* DosRawWriteNmPipe parameters */
/* data is buffer addr          */

struct npw_param {
    short npw_len;
    short npw_dlen;
    short npw_nbyt; /* number of bytes written */
};

/* NPipeWait parameters */

struct npq_param {
    short npq_len;
    short npq_dlen;
    long npq_timeo;  /* time-out in milliseconds */
    short npq_prio;  /* priority of caller       */
};

/* DosCallNmPipe parameters */
/* data is in-buffer addr   */

struct npx_param {
    short npx_len;
    unsigned short npx_ilen;  /* length of in-buffer      */
    char far * npx_obuf;      /* pointer to out-buffer    */
    unsigned short npx_ilen;  /* length of out-buffer     */
    unsigned short npx_nbyt;  /* number of bytes read     */
    long npx_timeo;           /* time-out in milliseconds */
};

/* PeekPipe parameters, data is buffer addr */

struct npp_param {
    short npp_len;
    unsigned short npp_dlen;
    unsigned short npp_nbyt;   /* number of bytes read      */
    unsigned short npp_av10;   /* bytes left in pipe        */
    unsigned short npp_av11;   /* bytes left in current msg */
    unsigned short npp_state;  /* pipe state                */
};

/* DosTransactNmPipe parameters */
/* data is in-buffer addr       */

struct npt_param {
    short npt_len;
    unsigned short npt_ilen;  /* length of in-buffer   */
    char far * npt_obuf;      /* pointer to out-buffer */
    unsigned short npt_olen;  /* length of out-buffer  */
    unsigned short npt_nbyt;  /* number of bytes read  */
};

/* QNmPipeSemState parameter block */ /* data is user data buffer */

struct qnps_param {
    unsigned short qnps_len;  /* length of parameter block      */
    unsigned short qnps_dlen;  /* length of supplied data block */
    long qnps_semh;            /* system semaphore handle       */
    unsigned short qnps_nbyt;  /* number of bytes returned      */
};

/* ConnectPipe parameter block, no data block */

struct npc_param {
    unsigned short npc_len;   /* length of parameter block */
    unsigned short npc_dlen;  /* length of data block      */
};

/* DisconnectPipe parameter block, no data block */

struct npd_param {
    unsigned short npd_len;   /* length of parameter block */
    unsigned short npd_dlen;  /* length of data block      */
};

:ecgraphic.
:font facename=default.
:p.pData is a pointer to a user data for operations which require it&per. When 
the pointer is supplied, its length will be given by the second element of the 
pOpRec structure&per.   
:p.pName is a pointer to a remote pipe name&per. Supplied only for NMP_WAITPIPE 
and NMP_CALLPIPE operations&per. For these two operations only, the psffsi and 
psffsd parameters have no significance&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This entry point is for support of special remote named pipe operations&per. 
Not all pointer parameters are used for all operations&per. In cases where a 
particular pointer has no significance, it will be NULL&per.   
:p.This entry point will be called only for the UNC FSD&per. Non-UNC FSDs are 
required to have this entry point, but should return NOT SUPPORTED if called&per.   
:h2 id=74.FS_OPENCREATE - Open a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Opens (or creates) the specified file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_OPENCREATE(pcdfsi, pcdfsd, pName, iCurDirEnd, psffsi,
                             psffsd, ulOpenMode, usOpenFlag, pusAction,
                             usAttr, pcEABuf, pfgenflag)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;
struct sffsi far * psffsi;
struct sffsd far * psffsd;
unsigned long ulOpenMode;
unsigned short usOpenFlag;
unsigned short far * pusAction;
unsigned short usAttr;
char far * pcEABuf;
unsigned short far * pfgenflag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.The contents of this structure are invalid for direct access opens&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per. The contents of this structure are invalid for direct access opens&per. For 
remote character devices, this field contains a pointer to a DWORD that was obtained 
from the remote FSD when the remote device was attached to this FSD&per. The FSD can 
use this DWORD to identify the remote device&per.   
:p.pName is a pointer to the ASCIIZ name of the file to be opened&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no current directory relevant to the name text, that is a device&per. This 
value is invalid for direct access opens&per.   
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.ulOpenMode indicates the desired sharing mode and access mode for the file 
handle&per.   
:p.See :hp1.OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.for a 
description of the OpenMode parameter for DosOpen&per.   
:p.An additional access mode 3 is defined when the file is being opened on 
behalf of OS/2, loaded for purposes of executing a file or loading a module&per. If 
the file system does not support an executable attribute, it should treat this 
access mode as open for reading&per. The value of ulOpenMode passed to the FSD will be 
valid&per.   
:p.usOpenFlag indicates the action taken when the file is present or absent&per.   
:p.See :hp1.OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.for a 
description of the usOpenFlag parameter for DosOpen&per.   
:p.The value of openflag passed to the FSD is valid&per. This value is invalid 
for direct access opens&per.   
:p.pusAction is the location where the FSD returns a description of the action 
taken as governed by openflag&per.   
:p.The FSD does not need to verify this pointer&per.  The contents of Action are 
invalid on return for direct access opens&per.   
:p.usAttr are the OS/2 file attributes&per.   
:p.This value is invalid for direct access opens&per.   
:p.pcEABuf is a pointer to the extended attribute buffer&per.   
:p.This buffer contains attributes that will be set upon creation of a new file 
or upon replacement of an existing file&per. If NULL, no extended attributes are 
to be set&per. Addressing of this data area has not been validated by the OS/2 
kernel (see FSH_PROBEBUF)&per. The contents of EABuf are invalid on return for direct 
access opens&per.   
:p.pfgenflag is a pointer to an unsigned short of flags returned by the FSD&per. 
The only flag currently defined is 0x0001 fGenNeedEA, which indicates that there 
are critical EAs associated with the file&per. The FSD does not need to verify this 
pointer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.For the file create operation, if successful, ST_CREAT and ST_PCREAT are set
&per. This causes the file to have zero as last read and last write time&per. If the 
last read/write time stamps are to be the same as the create time, simply set ST_
SWRITE, ST_PWRITE, ST_SREAD, and ST_PREAD as well&per.   
:p.For the file open operation, the FSD copies all supported time stamps from 
the directory entry into the SFT&per.   
:p.:hp2.Note&colon.   :ehp2.ALSO NEW FOR 2&per.0, it is suggested that the FSD copy the DOS 
file attributes from the directory entry into the SFT&per. This allows the FSD and 
the OS2 kernel to handle FCB opens more efficiently&per.   
:p.The sharing mode may be zero if this is a request to open a file from the DOS 
mode or for an FCB request&per.   
:p.FCB requests for read-write access to a read-only file are mapped to read-
only access and reflected in the sfi_mode field by the FSD&per.  An FCB request is 
indicated by the third bit set in the sfi_type field&per.   
:p.The flags defined for the sfi_type field are&colon.   
:p.otype == 0x0000 indicates file&per. 
.br 
otype == 0x0001 indicates device&per. 
.br 
otype == 0x0002 indicates named pipe&per. 
.br 
otype == 0x0004 indicates FCB open&per. 
.br 
oAll other values are reserved&per. 
.br 
  
:p.FSDs are required to initialize the sfi_type field, preserving the FCB bit
&per.   
:p.On entry, the sfi_hvpb field is filled in&per. If the file&apos.s logical 
size (EOD) is specified, it is passed in the sfi_size field&per. To the extent 
possible, the file system tries to allocate this much storage for efficient access&per.   
:p.Extended attributes are set for&colon.   
:p.othe creation ofa new file 
.br 
othe truncation of an existing file 
.br 
othe replacement of an existing file&per. 
.br 

:p.They are not set for a normal open&per.   
:p.If the standard OS/2 file creation attributes are specified, they are passed 
in the attr field&per. To the extent possible, the file system interprets the 
extended attributes and applies them to the newly-created or existing file&per. Extended 
attributes (EAs) that the file system does not itself use are retained with the file and 
not discarded or rejected&per.   
:p.When replacing an existing file, the FSD should not change the case of the 
existing file&per.   
:p.FSDs are required to support direct access opens&per. These are indicated by 
a bit set in the sffsi&per.sfi_mode field&per. See :hp1.OS/2 Version 2&per.0 Control 
Program Programming Reference :ehp1.for more information on DosOpen&per. Some of the 
parameters passed to the FSD for direct access opens are invalid, as described above&per.   
:p.On a successful return, the following fields in the sffsi structure must be 
filled in by the file system driver&colon. sfi_size and all the time and date fields
&per.   
:p.The file-system-dependent portion of an open file instance passed to the FSD 
for FS_OPENCREATE is never initialized&per.   
:p.Infinite FCB opens of the same file by the same DOS mode process is supported
&per. The first open is passed through to the FSD&per. Subsequent opens are not seen 
by the FSD&per.   
:p.Any non-zero value returned by the FSD indicates that the open failed and the 
file is not open&per.   
:p.:hp2.Note&colon.   :ehp2.This entry point&apos.s parameter list definition has changed 
from the 1&per.x IFS document&per. The OpenMode parameter has been widened from a 
unsigned short to a unsigned long&per. The upper word of the long is relevant only to a 
special SPOOLER FSD&per. For information about the upper word please contact the OS/2 
Techinal Interface group for the OEMI document for the 2&per.0 API   
:h2 id=75.FS_OPENPAGEFILE - Create paging file and handle

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Creates/opens the paging file for the Pager&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_OPENPAGEFILE(pFlags, pcMaxReq, pName, psffsi, psffsd,
                               usOpenMode, usOpenFlag, usAttr, Reserved)

unsigned long far * pFlag;
unsigned long far * pcMaxReq;
char far * pName;
struct sffsi far * psffsi;
struct sffsd far * psffsi;
unsigned short usOpenMode;
unsigned short usOpenFlag;
unsigned short usAttr;
unsigned long Reserved;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pFlag is a pointer to a flag double word for passing of information between 
the pager and the file system&per.   
:p.pFlag == 0x00000001 indicates first open of the page file&per. 
.br 
pFlag == 0x00004000 indicates physical addresses are required in the page list
&per. 
.br 
pFlag == 0x00008000 indicates 16&colon.16 virtual addresses are required in the 
page list&per. 
:p.All other values are reserved&per.   
:p.pcMaxReq is a pointer to a unsigned long where the FSD places the maximum 
request list length that can be managed by an enhanced strategy device driver&per.   
:p.pName is a pointer to the ASCIIZ path and filename of the paging file&per.   
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.usOpenMode indicates the desired sharing mode and access mode for the file 
handle&per.   
:p.See :hp1.OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.for a 
description of the OpenMode parameter for DosOpen&per.   
:p.usOpenFlag indicates the action taken when the file is present or absent&per.   
:p.See :hp1.OS/2 Version 2&per.0 Control Program Programming Reference :ehp1.for a 
description of the usOpenFlag parameter for DosOpen&per.   
:p.usAttr are the OS/2 file attributes&per.   
:p.Reserved is a double word parameter reserved for use in the future&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Enough information is provided for the FSD to perform a &osq.normal&osq. open
/create call&per.   
:p.Since a page file has special requirements about contiguity of its 
allocations, FS_OPENPAGEFILE must assure that any data sectors allocated are returned (
Create call only)&per. FS_ALLOCATEPAGESPACE will be called to handle file allocation
&per.   
:p.If the FSD cannot support the FS_DOPAGEIO (usually due to an disk device 
driver which does not support the Extended strategy entry point), the FSD can return 
zero (0) for *pcMaxReq&per. This tells the kernel file system that it must emulate 
FS_DOPAGEIO&per.   
:p.The FSD can require either physical or virtual (16&colon.16) addresses for 
subsequent calls to FS_DOPAGEIO&per. This allows an FSD to emulate FS_DOPAGEIO without 
having to worry about dealing with physical addresses&per.   
:p.For a detailed description of the Extended Strategy request interface please 
see the :hp1.OS/2 Version 2&per.0 Physical Device Driver Reference:ehp1.&per.   
:h2 id=76.FS_PATHINFO - Query/Set a File's Information

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Returns information for a specific path or file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_PATHINFO(flag, pcdfsi, pcdfsd, pName, iCurDirEnd, level,
                           pData, cbData)

unsigned short flag;
struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;
unsigned short level;
char far * pData;
unsigned short cbData;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates retrieval or setting of information&per. 
:p.flag == 0x0000 indicates retrieving information 
.br 
flag == 0x0001 indicates setting information on the media 
.br 
flag == 0x0010 indicates that the information being set must be written-through 
onto the disk before returning&per. This bit is never set when retrieving 
information&per. 
:p.All other values are reserved&per.   
:p.pcdfsi is a pointer the file-system-independent working directory structure
&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pName is a pointer to the ASCIIZ name of the file or directory for which 
information is to be retrieved or set&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no current directory relevant to the name text, that is a device&per.   
:p.level is the information level to be returned&per.   
:p.Level selects among a series of data structures to be returned or set&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per. When retrieval (flag == 0) is specified, the FSD places the information 
into the buffer&per. When outputting information to a file (flag == 1), the FSD 
retrieves that data from the application buffer&per.   
:p.cbData is the length of the application data area&per.   
:p.For flag == 0, this is the length of the data the application wishes to 
retrieve&per. If there is not enough room for the entire level of data to be returned, 
the FSD returns a BUFFER OVERFLOW error&per. For flag == 1, this is the length of 
data to be applied to the file&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.See the descriptions of DosQPathInfo and DosSetPathInfo in the :hp1.OS/2 Version 2
&per.0 Control Program Programming Reference :ehp1.for details on information levels&per.   
:p.The FSD will not be called for DosQPathInfo level 5&per.   
:p.FSDs that are case-preserving (like HPFS) can decide to accept level 7 
requests&per. A level 7 DosQueryPathInfo request asks the FSD to fill the pData buffer 
with the case-preserved path and filename of the path/filename passed in pName&per. 
Routing of level 7 requests will be determined by the kernel by checking the LV7 bit in 
a FSD&apos.s attribute double word&per.   
:h2 id=77.FS_PROCESSNAME - Allow FSD to modify name after OS/2 canonicalization

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Allow an FSD to modify filename to its own specification after the OS/2 
canonicalization process has completed&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_PROCESSNAME(pNameBuf)

char far * pNameBuf;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pNameBuf is a pointer to the ASCIIZ pathname&per.   
:p.The FSD should modify the pathname in place&per. The buffer is guaranteed to 
be the length of the maximum path&per. The FSD does not need to verify this 
pointer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The resulting name must be within the maximum path length returned by 
DosQSysInfo&per.   
:p.This routine allows the FSD to enforce a different naming convention than OS/
2&per. For example, an FSD could remove blanks embedded in component names or 
return an error if it found such blanks&per. It is called after the OS/2 
canonicalization process has succeeded&per. It is not called for FSH_CANONICALIZE&per.   
:p.This routine is called for all APIs that use pathnames&per.   
:p.This routine must return no error if the function is not supported&per.   
:h2 id=78.FS_READ - Read from a File

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Read the specified number of bytes from a file to a buffer location&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_READ(psffsi, psffsd, pData, pLen, IOflag)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
char far * pData;
unsigned short far * pLen;
unsigned short IOflag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.sfi_position is the location within the file where the data is to be read 
from&per. The FSD should update the sfi_position field&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area has not been validated by the kernel (see FSH_
PROBEBUF)&per.   
:p.pLen is a pointer to the length of the application data area&per.   
:p.On input, this is the number of bytes to be read&per. On output, this is the 
number of bytes successfully read&per. If the application data area is smaller than 
the length, no transfer is to take place&per. The FSD will not be called for zero 
length reads&per. The FSD does not need to verify this pointer&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through 
.br 
IOflag == 0x0020 indicates no-cache 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.If read is successful and is a file, the FSD should set ST_SREAD and ST_PREAD 
to make the kernel time stamp the last modification time in the SFT&per.   
:p.Of the information passed in IOflag, the write-through bit is a mandatory bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:h2 id=79.FS_RMDIR - Remove Subdirectory

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Removes a subdirectory from the specified disk&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_RMDIR(pcdfsi, pcdfsd, pName, iCurDirEnd)

struct cdfsi far * pcdfsi;
struct cdfsd far * pcdfsd;
char far * pName;
unsigned short iCurDirEnd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcdfsi is a pointer to the file-system-independent working directory 
structure&per.   
:p.pcdfsd is a pointer to the file-system-dependent working directory structure
&per.   
:p.pName is a pointer to the ASCIIZ name of the directory to be removed&per.   
:p.The FSD does not need to verify this pointer&per.   
:p.iCurDirEnd is the index of the end of the current directory in pName&per.   
:p.This is used to optimize FSD path processing&per. If iCurDirEnd == -1, there 
is no directory relevant to the name text, that is a device&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.OS/2 assures that the directory being removed is not the current directory 
nor the parent of any current directory of any process&per.   
:p.The FSD should not remove any directory that has entries other than &apos.
&per.&apos. and &apos.&per.&per.&apos. in it&per.   
:h2 id=80.FS_SETSWAP - Notification of swap-file ownership

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Perform whatever actions are necessary to support the swapper&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_SETSWAP(psffsi, psffsd)

struct sffsi far * psffsi;
struct sffsd far * psffsd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance of the swapper file&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Swapping does not begin until this call returns successfully&per. This call 
is made during system initialization&per.   
:p.The FSD makes all segments that are relevant to swap-file I/O non-swappable (
see FSH_FORCENOSWAP)&per. This includes any data and code segments accessed during 
a read or write&per.   
:p.Any FSD that manages writeable media may be the swapper file system&per.   
:p.FS_SETSWAP may be called more than once for the same or different volumes or 
FSDs&per.   
:h2 id=81.FS_SHUTDOWN - Shutdown file system

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Used to shutdown an FSD in preparation for power-off or IPL&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_SHUTDOWN(type, reserved)

unsigned short type;
unsigned long reserved;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.type indicates what type of a shutdown operation to perform&per.   
:p.type == 0 indicates that the shutdown sequence is beginning&per. The kernel 
will not allow any new I/O calls to reach the FSD&per. The only exception will be I/
O to the swap file by the swap thread through the FS_READ and FS_WRITE entry 
points&per. The kernel will still allow any thread to call FS_COMMIT, FS_FLUSHBUF and 
FS_SHUTDOWN&per. The FSD should complete all pending calls that might generate 
disk corruption&per.   
:p.type == 1 indicates that the shutdown sequence is ending&per. An FS_COMMIT 
has been called on every SFT open on the FSD and following that an FS_FLUSHBUF on 
all volumes has been called&per. All final clean up activity must be completed 
before this call returns&per.   
:p.reserved reserved for future expansion&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.From the perspective of an FSD, the shutdown sequence looks like this&colon.   
:p.First, the system will call the FSD&apos.s FS_SHUTDOWN entry with type == 0
&per. This notifies the FSD that the system will begin committing SFTs in preparation 
for system power off&per. The kernel will not allow any new IO calls to the FSD 
once it receives this first call, except from the swapper thread&per. The swapper 
thread will continue to call the FS_READ and FS_WRITE entry points to read and write 
the swap file&per. The swapper thread will not attempt to grow or shrink the swap 
file nor should the FSD reallocate it&per. The kernel will continue to allow FS_
COMMIT and FS_FLUSHBUF calls from any thread&per. This call should not return from the 
FSD until disk data modifying calls have completed to insure that a thread already 
inside the FSD does not wake and change disk data&per.   
:p.After the first FS_SHUTDOWN call returns, the kernel will start committing 
SFTs&per. The FSD will see a commit for every SFT associated with it&per. During 
these FS_COMMIT calls, the FSD must flush any data associated with these SFTs to disk
&per. The FSD must not allow any FS_COMMIT or FS_FLUSHBUF call to block permanently
&per.   
:p.Once all of the SFTs associated with the FSD have been committed, FS_SHUTDOWN 
will be called with type == 1&per. This will tell the FSD to flush all buffers to 
disk&per. From this point, the FSD must not buffer any data destined for disk&per. 
Reads and writes to the swap file will continue, but the allocation of the swap file 
will not change&per. Once this call has completed, no file system corruption should 
occur if power is shut off&per.   
:h2 id=82.FS_VERIFYUNCNAME - Verify UNC server ownership

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Used to poll installed UNC FSDs to determine server ownership&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_VERIFYUNCNAME(flag, pName)

unsigned short flag;
char far * pName;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates which &osq.pass&osq. of the poll the FSD is being called&per.   
:p.flag == 0x0000 indicates that this is a pass 1 poll&per. 
.br 
flag == 0x0001 indicates that this is a pass 2 poll&per.   
:p.pName is a pointer to the ASCIIZ name of the server in UNC format&per.   
:p.The FSD does not need to verify this pointer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.What the kernel expects from UNC FSDs for this entry point&colon.   
:p.For pass 1, the FSD will be called and passed a pointer to the UNC server 
name&per. It is to respond immediately if it recognizes (manages) the server with a 
NO_ERROR return code&per. This pass expects the that the FSD will be keeping 
tables in memory that contain the UNC names of the servers it is currently servicing
&per. If the UNC name cannot be validated immediately, the FSD should respond with an 
error (non-zero) return code&per. The FSD SHOULD NOT send messages in an attempt to 
validate the server name&per.   
:p.For pass 2, the FSD is permitted to do whatever is reasonable, including 
sending LAN &osq.are you there&osq. messages, to determine if they are able to service 
the request for UNC server&per.   
:h2 id=83.FS_WRITE - Write to a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Write the specified number of bytes to a file from a buffer location&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FS_WRITE(psffsi, psffsd, pDat, pLen, IOflag)

struct sffsi far * psffsi;
struct sffsd far * psffsd;
char far * pData;
unsigned short far * pLen;
unsigned short IOflag;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.psffsi is a pointer to the file-system-independent portion of an open file 
instance&per.   
:p.sfi_position is the location within the file where the data is to be written 
to&per. The FSD should update the sfi_position and sfi_size fields&per.   
:p.psffsd is a pointer to the file-system-dependent portion of an open file 
instance&per.   
:p.pData is the address of the application data area&per.   
:p.Addressing of this data area is not validated by the kernel (see FSH_PROBEBUF
)&per.   
:p.pLen is a pointer to the length of the application data area&per.   
:p.On input, this is the number of bytes that are to be written&per. On output, 
this is the number of bytes successfully written&per. If the application data area 
is smaller than the length, no transfer is to take place&per. The FSD does not 
need to verify this pointer&per.   
:p.IOflag indicates information about the operation on the handle&per.   
:p.IOflag == 0x0010 indicates write-through 
.br 
IOflag == 0x0020 indicates no-cache 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.If write is successful and is a file, the FSD should set ST_SWRITE and ST_
PWRITE to make the kernel time stamp the last modification time in the SFT&per.   
:p.The FSD should return an error if an attempt is made to write beyond the end 
with a direct access device handle&per.   
:p.Of the information passed in IOflag, the write-through bit is a mandatory bit 
in that any data written to the block device must be put out on the medium before 
the device driver returns&per. The no-cache bit, on the other hand, is an advisory 
bit that says whether the data being transferred is worth caching or not&per.   
:h1 id=84.FS Helper Functions

:p.The following table summarizes the routines that make up the File System 
Helper interface between FSDs and the kernel&per.   :font facename='Courier' size=12x12.
:cgraphic.
:color fc=default.:color bc=default.
 FS Helper Routine    Description
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=85.FSH_ADDSHARE:elink.         Add a name to the sharing set
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 FSH_BUFSTATE         REMOVED in OS/2 Version 2&per.0
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=86.FSH_CALLDRIVER:elink.       Call Device Driver&apos.s Extended
                      Strategy entry point
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=87.FSH_CANONICALIZE:elink.     Convert pathname to canonical form
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=88.FSH_CHECKEANAME:elink.      Check EA name validity
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=89.FSH_CRITERROR:elink.        Signal a hard error to the daemon
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=90.FSH_DEVIOCTL:elink.         Send IOCTL request to device driver
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=91.FSH_DOVOLIO:elink.          Volume-based sector-oriented
                      transfer
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=92.FSH_DOVOLIO2:elink.         Send volume-based IOCTL request to
                      device driver&per.
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=93.FSH_FINDCHAR:elink.         Find first occurrence of char in
                      string
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=94.FSH_FINDDUPHVPB:elink.      Locates equivalent hVPBs
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 FSH_FLUSHBUF         REMOVED in OS/2 Version 2&per.0
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=95.FSH_FORCENOSWAP:elink.      Force segments permanently into
                      memory
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 FSH_GETBUF           REMOVED in OS/2 Version 2&per.0
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 FSH_GETFIRSTOVERLAPB REMOVED in OS/2 Version 2&per.0
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=96.FSH_GETPRIORITY:elink.      Get current thread&apos.s I/O priority
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=97.FSH_GETVOLPARM:elink.       Get VPB data from VPB handle
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=98.FSH_INTERR:elink.           Signal an internal error
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=99.FSH_IOBOOST:elink.          Gives the current thread an I/O
                      priority boost
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=100.FSH_IOSEMCLEAR:elink.       Clear an I/O-event semaphore
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=101.FSH_ISCURDIRPREFIX:elink.   Test for a prefix of a current
                      directory
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=102.FSH_LOADCHAR:elink.         Load character from a string
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=103.FSH_NAMEFROMSFN:elink.      Get the full path name from an SFN
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=104.FSH_PREVCHAR:elink.         Move backward in string
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=105.FSH_PROBEBUF:elink.         User address validity check
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=106.FSH_QSYSINFO:elink.         Query system information
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=107.FSH_REGISTERPERFCTRS:elink. Register a FSD with PERFVIEW
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 FSH_RELEASEBUF       REMOVED in OS/2 Version 2&per.0
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=108.FSH_REMOVESHARE:elink.      Remove a name from the sharing set
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=109.FSH_SEGALLOC:elink.         Allocate a GDT or LDT segment
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=110.FSH_SEGFREE:elink.          Release a GDT or LDT segment
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=111.FSH_SEGREALLOC:elink.       Change segment size
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=112.FSH_SEMCLEAR:elink.         Clear a semaphore
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=113.FSH_SEMREQUEST:elink.       Request a semaphore
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=114.FSH_SEMSET:elink.           Set a semaphore
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=115.FSH_SEMSETWAIT:elink.       Set a semaphore and wait for clear
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=116.FSH_SEMWAIT:elink.          Wait for clear
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=117.FSH_SETVOLUME:elink.        force a volume to be mounted on the
                      drive
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=118.FSH_STORECHAR:elink.        Store character into string
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=119.FSH_UPPERCASE:elink.        Uppercase ASCIIZ string
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=120.FSH_WILDMATCH:elink.        Match using OS/2 wildcards
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=121.FSH_YIELD:elink.            Yield CPU to higher priority
                      threads


:ecgraphic.

:p.FSDs are loaded as dynamic link libraries and may import services provided by 
the kernel&per. These services can be called directly by the file system, passing 
the relevant parameters&per.   
:p.No validation of input parameters is done unless otherwise specified&per. The 
FSD calls FSH_PROBEBUF, where appropriate, before calling the FS help routine&per.   
:p.When any service returns an error code, the FSD must return to the caller as 
soon as possible and return the specific error code from the helper to the FS router
&per.   
:p.There are many deadlocks that may occur as a result of operations issued by 
FSDs&per. OS/2 provides no means whereby deadlocks between file systems and 
applications can be detected&per.   
:h2 id=85.FSH_ADDSHARE - Add a name to the share set

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function adds a name to the currently active sharing set&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_ADDSHARE(pName, mode, hVPB, phShare)

char far * pName;
unsigned short mode;
unsigned short hVPB;
unsigned long far * phShare;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.:hp2.pName :ehp2.is a pointer to the ASCIIZ name to be added into the share set&per.   
:p.The name must be in canonical form&colon. no &apos.&per.&apos. or &apos.&per.
&per.&apos. components, uppercase, no meta characters, and full path name specified
&per.   
:p.:hp2.mode :ehp2.is the sharing mode and access mode as defined in the DosOpen API&per. 
All other bits (direct open, write-through, etc&per.) must be zero&per.   
:p.:hp2.hVPB :ehp2.is the handle to the volume where the named object is presumed to exist
&per.   
:p.:hp2.phShare :ehp2.is the pointer to the location where a share handle is stored&per. 
This handle may be passed to FSH_REMOVESHARE&per. 
.br 
  
:p.pName is a pointer to the ASCIIZ name to be added into the share set&per.   
:p.The name must be in canonical form&colon. no &apos.&per.&apos. or &apos.&per.
&per.&apos. components, uppercase, no meta characters, and full path name specified
&per.   
:p.mode is the sharing mode and access mode as defined in the DosOpen API&per. 
All other bits (direct open, write-through, etc&per.) must be zero&per.   
:p.hVPB is the handle to the volume where the named object is presumed to exist
&per.   
:p.phShare is the pointer to the location where a share handle is stored&per. 
This handle may be passed to FSH_REMOVESHARE&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_SHARING_VIOLATION 
.br 
the file is open with a conflicting sharing/access mode&per.   
:p.oERROR_TOO_MANY_OPEN_FILES 
.br 
there are too many files open at the present time&per.   
:p.oERROR_SHARING_BUFFER_EXCEEDED 
.br 
there is not enough memory to hold sharing information&per.   
:p.oERROR_INVALID_PARAMETER 
.br 
invalid bits in mode&per.   
:p.oERROR_FILENAME_EXCED_RANGE 
.br 
path name is too long&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Do not call FSH_ADDSHARE for character devices&per.   
:p.FSH_ADDSHARE may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=86.FSH_CALLDRIVER - Call Device Driver's Extended Strategy entry point

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This routine allows FSDs to call a device driver&apos.s Extended Strategy 
entry point&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_CALLDRIVER(pPkt, hVPB, fControl)

void far * pPkt;
unsigned short hVPB;
unsigned short fControl;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pPkt is a pointer to device driver Extended strategy request packet&per.   
:p.hVPB is the volume handle for the source of I/O&per.   
:p.fControl is the bit mask of pop-up control actions&colon. 
:p.Bit 0 off indicates volume change pop-up desired 
.br 
Bit 0 on indicates no volume change pop-up 
:p.All other bits are reserved and must be zero&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_VOLUME_CHANGED 
.br 
is an indication that removable media volume change has occurred&per.   
:p.oERROR_INVALID_PARAMETER 
.br 
the fControl flag word has reserved bits on&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This routine should be called for any Extended strategy requests going to a 
drive that has removable media&per.   
:p.For a detailed description of the Extended Strategy request interface please 
see the :hp1.OS/2 Version 2&per.0 Physical Device Driver Reference :ehp1.  
:p.FSH_CALLDRIVER may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=87.FSH_CANONICALIZE - Convert a path name to a canonical form

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function converts a path name to a canonical form by processing &apos.
&per.&apos.s and &apos.&per.&per.&apos.s, uppercasing, and prepending the current 
directory to non-absolute paths&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_CANONICALIZE(pPathName, cbPathBuf, pPathBuf, pFlags)

char far * pPathName;
unsigned short cbPathBuf;
char far * pPathBuf;
unsigned short far * pFlags;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pPathName is a pointer to the ASCIIZ path name to be canonicalized&per.   
:p.cbPathBuf is the length of path name buffer&per.   
:p.pPathBuf is the pointer to the buffer into which to copy the canonicalized 
path&per.   
:p.pFlags is the pointer to flags returned to the FSD&per.   
:p.Flags == 0x0040 indicates a non-8&per.3 filename format&per. All other values 
are reserved&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_PATH_NOT_FOUND 
.br 
is an invalid path name-too many &apos.&per.&per.&apos.s   
:p.oERROR_BUFFER_OVERFLOW 
.br 
the path name is too long&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This routine processes DBCS characters properly&per.   
:p.The FSD is responsible for verifying the string pointers and checking for 
segment boundaries&per.   
:p.FSH_CANONICALIZE should be called for names passed into the FSD raw data 
packets&per. For example, names passed to FS_FSCTL in the parameter area should be 
passed to FSH_CANONICALIZE&per. This routine does not need to be called for explicit 
names passed to the FSD, that is, the name passed to FS_OPENCREATE&per.   
:p.If the canonicalized name is being created as a file or directory, the non-8
&per.3 attribute in the directory entry should be set according to the value returned 
in pFlags&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=88.FSH_CHECKEANAME - Check for valid EA name

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Check extended attribute name validity&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_CHECKEANAME(iLevel, cbEAName, szEAName)

unsigned short iLevel;
unsigned long cbEAName;
char far * szEAName;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.iLevel is the extended attributes name checking level&per.   
:p.iLevel = 0x0001 indicates OS/2 Version 2&per.0 name checking&per.   
:p.cbEAName is the length of the extended attribute name, not including 
terminating NUL&per.   
:p.szEAName is the extended attribute name to check for validity&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_NAME 
.br 
pathname contains invalid or wildcard characters, or is too long&per. 
:p.oERROR_INVALID_PARAMETER 
.br 
invalid level&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This routine processes DBCS characters properly&per.   
:p.The set of invalid characters for EA names is the same as that for filenames
&per. In OS/2 Version 2&per.0, the maximum length of an EA name, not including the 
terminating NUL, is 255 bytes&per. The minimum length is 1 byte&per.   
:p.The FSD is responsible for verifying the string pointers and checking for 
segment boundaries&per.   
:p.FSH_CHECKEANAME should be called for extended attribute names passed to the 
FSD&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=89.FSH_CRITERROR - Signal hard error to daemon

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function signals a hard error to the daemon&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_CRITERROR(cbMessage, pMessage, nSubs, pSubs, fAllowed)

unsigned short cbMessage;
char far * pMessage;
unsigned short nSubs;
char far * pSubs;
unsigned short fAllowed;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.cbMessage is the length of the message template&per.   
:p.pMessage is the pointer to the message template&per.   
:p.This may contain replaceable parameters in the format used by the message 
retriever&per.   
:p.nSubs is the number of replaceable parameters&per.   
:p.pSubs is the pointer to the replacement text&per.   
:p.The replacement text is a packed set of ASCIIZ strings&per.   
:p.fAllowed is the bit mask of allowed actions&colon.   
:p.Bit 0x0001 on indicates FAIL allowed 
.br 
Bit 0x0002 on indicates ABORT allowed 
.br 
Bit 0x0004 on indicates RETRY allowed 
.br 
Bit 0x0008 on indicates IGNORE allowed 
.br 
Bit 0x0010 on indicates ACKNOWLEDGE only allowed&per.   
:p.All other bits are reserved and must be zero&per. If bit 0x0010 is set, and 
any or some of bits 0x0001 to 0x0008 are also set, bit 0x0010 will be ignored&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.This function returns the action to be taken&colon. 
:p.0x0000 - ignore 
.br 
0x0001 - retry 
.br 
0x0003 - fail 
.br 
0x0004 - continue   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.If the user responds with an action that is not allowed, it is treated as 
FAIL&per. If FAIL is not allowed, it is treated as ABORT&per. ABORT is always 
allowed&per.   
:p.When ABORT is the final action, OS/2 does not return this as an indicator; it 
returns a FAIL&per. The actual ABORT operation is generated when this thread of 
execution is about to return to user code&per.   
:p.The maximum length of the template is 128 bytes, including the NUL&per. The 
maximum length of the message with text substitutions is 512 bytes&per. The maximum 
number of substitutions is 9&per.   
:p.If any action other than retry is selected for a given hard error popup, then 
any subsequent popups (within the same API call) will be automatically failed; a 
popup will not be done&per. This means that (except for retries) there can be at most 
one hard error popup per call to the FSD&per. And, if the kernel generates a popup, 
then the FSD cannot create one&per.   
:p.FSH_CRITERROR will fail automatically if the user application has set 
autofail, or if a previous hard error has occurred&per.   
:p.FSH_CRITERROR may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=90.FSH_DEVIOCTL - Send IOCTL request to device driver

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function sends an IOCTL request to a device driver&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_DEVIOCTL(flag, hDev, sfn, cat, func, pParm, cbParm, pData,
                            cbData)

unsigned short flag;
unsigned long hDev;
unsigned short sfn;
unsigned short cat;
unsigned short func;
char far * pParm;
unsigned short cbParm;
char far * pData;
unsigned short cbData;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flag indicates whether the FSD initiated the call or not&per.   
:p.IOflag == 0x0000 indicates that the FSD is just passing user pointers on to 
the helper&per. 
.br 
IOflag == 0x0001 indicates that the FSD initiated the DevIOCtl call as opposed 
to passing a DevIOCtl that it had received&per.   
:p.All other bits are reserved&per.   
:p.hDev is the device handle obtained from VPB   
:p.sfn is the system file number from open instance that caused the FSH_DEVIOCTL 
call&per.   
:p.This field should be passed unchanged from the sfi_selfsfn field&per. If no 
open instance corresponds to this call, this field should be set to 0xFFFF&per.   
:p.cat is the category of IOCTL to perform&per.   
:p.func is the function within the category of IOCTL&per.   
:p.pParm is the long address to the parameter area&per.   
:p.cbParm is the length of the parameter area&per.   
:p.pData is the long address to the data area&per.   
:p.cbData is the length of the data area&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_FUNCTION 
.br 
indicates the function supplied is incompatible with the category and device 
handle supplied&per. 
:p.oERROR_INVALID_CATEGORY 
.br 
indicates the category supplied is incompatible with the function and device 
handle supplied&per.   
:p.oDevice driver error code 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The only category currently supported for this call is 8, which is for the 
logical disk&per. FSDs call FSH_DEVIOCTL to control device driver operation 
independently from I/O operations&per. This is typically in filtering DosDevIOCtl requests 
when passing the request on to the device driver&per.   
:p.An FSD needs to be careful of pointers to buffers that are passed to it from 
FS_IOCTL, and what it passes to FSH_DEVIOCTL&per. It is possible that such 
pointers may be real mode pointers if the call was made from the DOS mode&per. In any 
case, the FSD must indicate whether it initiated the DevIOCtl call, in which case 
the kernel can assume that the pointers are all protect mode pointers, or if it is 
passing user pointers on to the FSH_DEVIOCTL call, in which case the mode of the 
pointers will depend on whether this is the DOS mode or not&per. An important thing to 
note is that the FSD must not mix user pointers with its own pointers when using 
this helper&per.   
:p.FSH_DEVIOCTL may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=91.FSH_DOVOLIO - Transfer volume-based sector-oriented I/O

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function performs I/O to the specified volume&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_DOVOLIO(operation, fAllowed, hVPB, pData, pcSec, iSec)

unsigned short operation;
unsigned short fAllowed;
unsigned short hVPB;
char far * pData;
unsigned short far * pcSec;
unsigned long iSec;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.operation is the bit mask indicating read/read-bypass/write/write-bypass, and 
verify- after-write/write-through and no-cache operation to be performed&per.   
:p.Bit 0x0001 off indicates read&per. 
.br 
Bit 0x0001 on indicates write&per. 
.br 
Bit 0x0002 off indicates no cache bypass&per. 
.br 
Bit 0x0002 on indicates cache bypass&per. 
.br 
Bit 0x0004 off indicates no verify-after-write operation&per. 
.br 
Bit 0x0004 on indicates verify-after-write operation&per. 
.br 
Bit 0x0008 off indicates errors signaled to the hard error daemon&per. 
.br 
Bit 0x0008 on indicates hard errors will be returned directly&per. 
.br 
Bit 0x0010 off indicates I/O is not write-through&per. 
.br 
Bit 0x0010 on indicates I/O is write-through&per. 
.br 
Bit 0x0020 off indicates data for this I/O should probably be cached&per. 
.br 
Bit 0x0020 on indicates data for this I/O should probably not be cached&per. 
.br 
All other bits are reserved and must be zero&per.   
:p.The difference between the cache bypass and the no cache bits is in the type 
of request packet that the device driver will see&per. With cache bypass, it will 
get a packet with command code 24, 25, or 26&per. With no cache, it will get the 
extended packets for command codes 4, 8, or 9&per. The advantage of the latter is that 
the write-through bit can also be sent to the device driver in the same packet, 
improving the functionality at the level of the device driver&per.   
:p.fAllowed is a bit mask indicating allowed actions&colon.   
:p.Bit 0x0001 on indicates FAIL allowed 
.br 
Bit 0x0002 on indicates ABORT allowed 
.br 
Bit 0x0004 on indicates RETRY allowed 
.br 
Bit 0x0008 on indicates IGNORE allowed 
.br 
Bit 0x0010 on indicates ACKNOWLEDGE only allowed 
.br 
If this bit is set, none of the other bits may be set&per.   
:p.All other bits are reserved and must be set to zero&per.   
:p.hVPB is the volume handle for the source of I/O&per.   
:p.pData is the long address of the user transfer area&per.   
:p.pcSec is the pointer to the number of sectors to be transferred&per.   
:p.On return, this is the number of sectors successfully transferred&per.   
:p.iSec is the sector number of the first sector of the transfer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
indicates the supplied address/length is invalid&per.   
:p.oERROR_UNCERTAIN_MEDIA 
.br 
indicates the device driver can no longer reliably tell if the media has been 
changed&per.   
:p.This occurs only within the context of an FS_MOUNT call&per.   
:p.oERROR_TRANSFER_TOO_LONG 
.br 
indicates the transfer is too long for the device&per.   
:p.oDevice-driver/device-manager errors listed /DDERR/ 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This function formats a device driver request packet for the requested I/O, 
locks the data transfer region, calls the device driver, and reports any errors to 
the hard error daemon before returning to the FSD&per. Any retries indicated by the 
hard error daemon or actions indicated by DosError are done within the call to FSH_
DOVOLIO&per.   
:p.FSH_DOVOLIO may be used at all times within the FSD&per. When called within 
the scope of a FS_MOUNT call, it applies to the volume in the drive without regard 
to which volume it may be&per. However, since volume recognition is not complete 
until the FSD returns to the FS_MOUNT call, the FSD must be careful when an ERROR_
UNCERTAIN_MEDIA is returned&per. This indicates the media has gone uncertain while we are 
trying to identify the media in the drive&per. This may indicate the volume that the 
FSD was trying to recognize was removed&per. In that case, the FSD must release any 
resources attached to the hVPB passed in the FS_MOUNT call and return ERROR_UNCERTAIN_
MEDIA to the FS_MOUNT call&per. This will direct the volume tracking logic to restart 
the mount process&per.   
:p.OS/2 will validate the user transfer area for proper access and size and will 
lock the segment&per.   
:p.Verify-after-write specified on a read is ignored&per.   
:p.On 80386 processors, FSH_DOVOLIO will take care of memory contiguity 
requirements of device drivers&per. It is advisable, therefore, that FSDs use FSH_DOVOLIO 
instead of calling device drivers directly&per. This will improve performance of FSDs 
running on 80386 processors&per.   
:p.FSH_DOVOLIO may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=92.FSH_DOVOLIO2 - Send volume-based IOCTL request to device driver

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function is an FSD call that controls device driver operation 
independently from I/O operations&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_DOVOLIO2(hDev, sfn, cat, func, pParm, cbParm, pData,
                            cbData)

unsigned long hDev;
unsigned short sfn;
unsigned short cat;
unsigned short func;
char far * pParm;
unsigned short cbParm;
char far * pData;
unsigned short cbData;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.hDev is the device handle obtained from VPB   
:p.sfn is the system file number from the open instance that caused the FSH_
DOVOLIO2 call&per.   
:p.This field should be passed unchanged from the sfi-selfsfn field&per. It no 
open instance corresponds to this call, this field should be set to 0xFFFF&per.   
:p.cat is the category of IOCTL to perform&per.   
:p.func is the function within the category of IOCTL&per.   
:p.pParm is the long address to the parameter area&per.   
:p.cbParm is the length of the parameter area&per.   
:p.pData is the long address to the data area&per.   
:p.cbData is the length of the data area&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_FUNCTION 
.br 
indicates the function supplied is incompatible with the category and the device 
handle supplied&per.   
:p.oERROR_INVALID_CATEGORY 
.br 
indicates the category supplied is incompatible with the function and the device 
handle supplied&per.   
:p.oDevice-driver/device-manager errors listed /DDERR/ 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This routine supports volume management for IOCTL operations&per. Any errors 
are reported to the hard error daemon before returning to the FSD&per. Any retries 
indicated by the hard error daemon or actions indicated by DosError are done within the 
call to FSH_DOVOLIO2&per.   
:p.The purpose of this routine is to enable volume tracking with IOCTLs&per. It 
is not available at the API level&per.   
:p.FSH_DOVOLIO2 may block&per.   
:p.System does normal volume checking for this request&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=93.FSH_FINDCHAR - Find first occurrence of character in string

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism to find the first occurrence of any one 
of a set of characters in an ASCIIZ string, taking into account DBCS 
considerations&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_FINDCHAR(nChars, pChars, ppStr)

unsigned short nChars;
char far * pChars;
char far * far * ppStr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.nChars is the number of characters in the search list&per.   
:p.pChars is the array of characters to search for&per. These cannot be DBCS 
characters&per.   
:p.The NUL character cannot be searched for&per.   
:p.ppSTR is the pointer to the character pointer where the search is to begin
&per. This pointer is updated upon return to point to the character found&per. This 
must be an ASCIIZ string&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_CHAR_NOT_FOUND 
.br 
indicates none of the characters were found&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The search will continue until a matching character is found or the end of 
the string is found&per.   
:p.The FSD is responsible for verifying the string pointers and checking for 
segment boundaries&per.   
:p.:hp2.Note&colon.   :ehp2. OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=94.FSH_FINDDUPHVPB - Locate equivalent hVPB

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism to identify a previous instance of a 
volume during the FS_MOUNT process&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_FINDDUPHVPB(hVPB, phVPB)

unsigned short hVPB;
unsigned short far * phVPB;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.hVPB is the handle to the volume to be found&per.   
:p.phVPB is the pointer to where the handle of matching volume will be stored
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_NO_ITEMS 
.br 
indicates there is no matching hVPB&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.When OS/2 is recognizing a volume, it calls the FSD to mount the volume&per. 
At this point, the FSD may elect to allocate storage and buffer data for that 
volume&per. The mount process will allocate a new VPB whenever the media becomes 
uncertain, that is, when the device driver recognizes it can no longer be certain the 
media is unchanged&per. This VPB cannot be collapsed with a previously allocated VPB, 
because of a reinsertion of media, until the FS_MOUNT call returns&per. The previous 
VPB, however, may have some cached data that must be updated from the media (the 
media may have been written while it was removed) FSH_FINDDUPHVPB allows the FSD to 
find this previous occurrence of the volume in order to update the cached 
information for the old VPB&per. Remember the newly created VPB will be unmounted if there 
is another, older VPB for that volume&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=95.FSH_FORCENOSWAP - Force segments permanently into memory

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function permanently forces segments into memory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_FORCENOSWAP(sel)

unsigned short sel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.sel is the selector that is to be made non-swappable&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_ACCESS 
.br 
indicates the selector is invalid&per.   
:p.oERROR_INVALID_DENIED 
.br 
indicates the selector is invalid or the sector belongs to another process&per.   
:p.oERROR_DIRECT_ACCESS_HANDLE 
.br 
indicates the handle does not refer to a segment&per.   
:p.oERROR_NOT_ENOUGH_MEMORY 
.br 
indicates there is not enough physical memory to make a segment nonswappable
&per.   
:p.oERROR_SWAP_TABLE_FULL 
.br 
indicates the attempt to grow the swap file failed&per.   
:p.oERROR_SWAP_FILE_FULL 
.br 
indicates the attempt to grow the swap file failed&per.   
:p.oERROR_PMM_INSUFFICIENT_MEMORY 
.br 
indicates the attempt to grow the swap file failed&per. 
.br 
  
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.An FSD may call FSH_FORCENOSWAP to force segments to be loaded into memory 
and marked non-swappable&per. All segments both in the load image of the FSD and 
those allocated via FSH_SEGALLOC are eligible to be marked&per. There is no way to 
undo the effect of FSH_FORCENOSWAP&per.   
:p.If an FSD is notified it is managing the swapping media, it should make this 
call for the necessary segments&per.   
:p.An FSD should be prepared to see multiple swapping files on more than one 
volume in 80386 processors and in future releases of OS/2&per.   
:p.FSH_FORCENOSWAP may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=96.FSH_GETPRIORITY - Get current thread's I/O priority

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to retrieve the I/O priority of the current 
thread&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_GETPRIORITY(void)

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.This function returns the I/O of the current thread&colon. 
:p.0x0000 - background 
.br 
0x1111 - foreground   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_GETPRIORITY will not block&per.   
:h2 id=97.FSH_GETVOLPARM - Get VPB data from VPB handle

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to retrieve file-system-independent and file-
system-dependent data from a VPB&per. Since the FS router passes in a VPB handle, 
individual FSDs need to map the handle into pointers to the relevant portions&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_GETVOLPARM(hVPB, ppVPBfsi, ppVPBfsd)

unsigned short hVPB;
struc vpfsi far * far * ppVPBfsi;
struc vpfsd far * far * ppVPBfsd;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.hVPB is the volume handle of interest&per.   
:p.ppVPBfsi indicates the location where the pointer to file-system-independent 
data is stored&per.   
:p.ppVPBfsd indicates the location where the pointer to file-system-dependent 
data is stored&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_GETVOLPARM will not block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=98.FSH_INTERR - Signal an internal error

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function signals an internal error&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_INTERR(pMsg, cbMsg)

char far * pMsg;
unsigned short cbMsg;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pMsg is a pointer to the message text&per.   
:p.cbMsg is the length of the message text&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.For reliability, if an FSD detects an internal inconsistency during normal 
operation, the FSD shuts down the system as a whole&per. This is the safest thing to do 
since it is not clear if the system as a whole is in a state that allows normal 
execution to continue&per.   
:p.When an FSD calls FSH_INTERR, the address of the caller and the supplied 
message is displayed on the console&per. The system then halts&per.   
:p.The code used to display the message is primitive&per. The message should 
contain ASCII characters in the range 0x20-0x7E, optionally with 0x0D and 0x0A to break 
the text into multiple lines&per.   
:p.The FSD must preface all such messages with the name of the file system&per.   
:p.The maximum message length is 512 characters&per. Messages longer than this 
are truncated&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=99.FSH_IOBOOST - Gives the current thread an I/O priority boost

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to boost the current thread&apos.s I/O priority 
after a I/O request&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_IOBOOST(void)

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_IOBOOST will not block&per.   
:h2 id=100.FSH_IOSEMCLEAR - Clear an I/O event semaphore

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to clear the I/O event semaphore that is a part 
of the Extended Strategy request packet&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_IOSEMCLEAR(pSem)

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pSem is the handle to the I/O event semaphore&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_EXCL_ALREADY_OWNED 
.br 
the exclusive semaphore is already owned&per.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
the semaphore is inaccessible&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_IOSEMCLEAR may block&per.   
:p.For a detailed description of the Extended Strategy request interface, please 
see the :hp1.OS/2 Version 2&per.0 Physical Device Driver Reference :ehp1.  
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=101.FSH_ISCURDIRPREFIX - Test for a prefix of a current directory

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows FSDs to disallow any modification of any directory that 
is either a current directory of some process or the parent of any current 
directory of some process&per. This is necessary because the kernel manages the text of 
each current directory for each process&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_ISCURDIRPREFIX(pName)

char far * pMsg;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pName is a pointer to the path name&per.   
:p.The name must be in canonical form, that is, no &apos.&per.&apos. or &apos.
&per.&per.&apos. components, uppercase, no meta characters, and full path name 
specified&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_CURRENT_DIRECTORY 
.br 
the specified path is a prefix of or is equal to the current directory of some 
process&per.   
:p.If the current directory is the root and the path name is &osq.d&colon.&bsl.
&osq., ERROR_CURRENT_DIRECTORY will be returned&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_ISCURDIRPREFIX takes the supplied path name, enumerates all current 
directories in use, and tests to see if the specified path name is a prefix or is equal to 
some current directory&per.   
:p.FSH_ISCURDIRPREFIX may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=102.FSH_LOADCHAR - Load a character from a string

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for loading a character from a string, 
taking into account DBCS considerations&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_LOADCHAR(ppStr, pChar)

char far * far * ppStr;
unsigned short far * pChar;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.ppStr is a pointer to the character pointer of a string&per.   
:p.The character at this location will be retrieved and this pointer will be 
updated&per.   
:p.pChar is a pointer to the character returned&per.   
:p.If character is non-DBCS, the first byte will be the character and the second 
byte will be zero&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=103.FSH_NAMEFROMSFN - Get the full path name from an SFN.

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This call allows an FSD to retrieve the full path name for an object to which 
an SFN refers&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_NAMEFROMSFN(sfn, pName, pcbName)

unsigned short sfn;
char far * pName;
unsigned short far * pcbName;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.sfn is the system file number of a file instance, obtained from the sfi_
selfsfn field of the file-system-independent part of the SFT for the object&per.   
:p.pName is the location of where the returned full path name is to be stored
&per.   
:p.pcbName is the location of where the FSD places the size of the buffer 
pointed to by pName&per. On return, the kernel will fill this in with the length of the 
path name&per. This length does not include the terminating null character&per. The 
size of the buffer should be long enough to hold the full path name, or else an 
error will be returned&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_HANDLE 
.br 
the SFN is invalid&per.   
:p.oERROR_BUFFER_OVERFLOW 
.br 
the buffer is too short for the returned path&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_NAMEFROMSFN will not block&per.   
:p.:hp2.Note&colon.  :ehp2.OS/2 does not validate input parameters; the FSD should call FSH_
PROBEBUF where appropriate&per.   
:h2 id=104.FSH_PREVCHAR - Decrement a character pointer

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for decrementing a character point, 
taking into account DBCS considerations&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_PREVCHAR(pBeg, ppStr)

char far * pBeg;
char far * far * ppStr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pBeg is a pointer to the beginning of a string&per.   
:p.ppStr is a pointer to the character pointer of a string&per.   
:p.The value is decremented appropriately upon return&per. If it is at the 
beginning of a string, the pointer is not decremented&per. If it points to the second 
byte of a DBCS character, it will be decremented to point to the first byte of the 
character&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The FSD is responsible for verifying the string pointer and checking for 
segment boundaries&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=105.FSH_PROBEBUF - Check user address validity

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for performing validity checks on 
arbitrary pointers to data that users may pass in&per.   
:p.:hp2.Note&colon.   :ehp2.FSDs must check on these pointers before using them&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_PROBEBUF(operation, pdata, cbData)

unsigned short operation;
char far * pData;
unsigned short cbData;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.operation indicates whether read or write access is desired&per.   
:p.operation ==  0 indicates read access is to be checked&per. 
.br 
operation ==  1 indicates write access is to be checked&per.   
:p.All other values are reserved&per.   
:p.pData is the starting address of user data&per.   
:p.cbData is the length of user data&per. If cbData is 0, a length of 64K is 
indicated&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
indicates access to the indicated memory region is illegal (access to the data 
is inappropriate or the user transfer region is partially or completely 
inaccessible)&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Because users may pass in arbitrary pointers to data, FSDs must perform 
validity checks on these pointers before using them&per. Because OS/2 is multi-threaded, 
the addressability of data returned by FSH_PROBEBUF is only valid until the FSD 
blocks&per. Blocking, either explicitly or implicitly allows other threads to run, 
possibly invalidating a user segment&per. FSH_PROBEBUF must, therefore, be reapplied 
after every block&per.   
:p.FSH_PROBEBUF provides a convenient method to assure a user transfer address 
is valid and present in memory&per. Upon successful return, the user address may 
be treated as a far pointer and accessed up to the specified length without either 
blocking or faulting&per. This is guaranteed until the FSD returns or until the next 
block&per.   
:p.If FSH_PROBEBUF detects a protection violation, the process is terminated as 
soon as possible&per. The OS/2 kernel kills the process once it has exited from the 
FSD&per.   
:p.On 80386 processors, FSH_PROBEBUF ensures all touched pages are physically 
present in memory so the FSD will not suffer an implicit block due to a page fault&per. 
However, FSH_PROBEBUF does NOT guarantee the pages will be physically contiguous in 
memory because FSDs are not expected to do DMA&per.   
:p.FSH_PROBEBUF may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=106.FSH_QSYSINFO - Query system information

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function queries the system about dynamic system variables and static 
system variables not returned by DosQSysInfo&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_QSYSINFO(index, pData, cbData)

unsigned short index;
char far * pData;
unsigned short cbData;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.index is the variable to return&per.   
:p.index == 1 indicates maximum sector size&per.   
:p.index == 2 indicates process identity&per. The data returned will be as 
follows&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct {
    unsigned short PID;
    unsigned short UID;
    unsigned short PDB;
}

:ecgraphic.
:font facename=default.
:p.index == 3 indicates absolute thread number for the current thread&per. This 
will be returned in an unsigned short field&per.   
:p.index == 4 indicates verify on write flag for the process&per. This will be 
returned in an unsigned char (byte) field&per. Zero means verify is off, non-zero means 
it is on&per.   
:p.pData is the long address to the data area&per.   
:p.cbData is the length of the data area&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_PARAMETER 
.br 
the index is invalid&per.   
:p.oERROR_BUFFER_OVERFLOW 
.br 
the specified buffer is too short for the returned data&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=107.FSH_REGISTERPERFCTRS - Register a FSD with PERFVIEW

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows the FSD to register with the PERFVIEW product&per. The 
FSD passes pointers to its counter data and text blocks&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_REGISTERPERFCTRS(pDataBlk, pTextBlk, fsFlags)

void far * pDataBlk;
void far * pTextBlk;
unsigned short fsFlags;

:ecgraphic.
:font facename=default.  
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pDataBlk is a pointer to the data block where the actual counters reside&per.   
:p.pTextBlk is a pointer to the block that contains instance and name 
information about counters in the associated DataBlk&per.   
:p.fsFlags indicates what type of addressing is going to be used&per.   
:p.Bit 0 off indicates 16&colon.16 pointers 
.br 
Bit 0 on indicates 0&colon.32 pointers   
:p.All other bits are reserved and must be zero&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_PARAMETER 
.br 
the flag word is invalid&per.   
:p.oERROR_PVW_INVALID_COUNTER_BLK 
.br 
the specified buffer is not in the correct PERFVIEW data block format   
:p.oERROR_PVW_INVALID_TEXT_BLK 
.br 
the specified buffer is not in the correct PERFVIEW text block format 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.For a detailed description of the PERFVIEW interface and its associated data 
structures please see the OS/2 Version 2&per.0 PERFVIEW OEMI Document&per.   
:p.FSH_REGISTERPERFCTRS may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=108.FSH_REMOVESHARE - Remove a shared entry

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function removes a previously-entered name from the sharing set&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_REMOVESHARE(hShare)

unsigned long hShare;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.hShare is a share handle returned by a prior call to FSH_ADDSHARE&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.When a call to FSH_REMOVESHARE has been issued, the share handle is no longer 
valid&per.   
:p.FSH_REMOVESHARE may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=109.FSH_SEGALLOC - Allocate a GDT or LDT segment

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allocates a GDT or LDT selector&per. The selector will have 
read/write access&per. An FSD may call this function&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEGALLOC(flags, cbSeg, pSel)

unsigned short flags;
unsigned long cbSeg;
unsigned short far * pSel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.flags indicate GDT/LDT, protection ring, swappable/non-swappable&per.   
:p.Bit 0x0001 off indicates GDT selector returned&per. 
.br 
Bit 0x0001 on indicates LDT selector returned&per. 
.br 
Bit 0x0002 off indicates non-swappable memory&per. 
.br 
Bit 0x0002 on indicates swappable memory&per. 
.br 
Bits 13 and 14 are the desired ring number&per.   
:p.All other bits are reserved and must be zero&per.   
:p.cbSeg is the length of the segment&per.   
:p.pSel is the far address of the location where the allocated selector will be 
stored&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INTERRUPT 
.br 
the current thread received a signal&per.   
:p.oERROR_INVALID_PARAMETER 
.br 
the reserved bits in flags are set or requested size is too large&per.   
:p.oERROR_NOT_ENOUGH_MEMORY 
.br 
too much memory is allocated&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.It is strongly suggested that FSDs allocate all their data at protection 
level 0 for maximum protection from user programs&per.   
:p.GDT selectors are a scarce resource; the FSD must be prepared to expect an 
error for allocation of a GDT segment&per. The FSD should limit itself to a maximum 
of 10 GDT segments&per. It is suggested that a large segment be allocated for each 
type of data and divided into per-process records&per.   
:p.FSH_SEGALLOC may block&per.   
:p.Take care to avoid deadlocks between swappable segments and swapper requests
&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=110.FSH_SEGFREE - Release a GDT or LDT segment

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function releases a GDT or LDT segment previously allocated with FSH_
SEGALLOC or loaded as part of the FSD image&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEGFREE(sel)

unsigned short sel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.sel is the selector to be freed&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INVALID_ACCESS 
.br 
the selector is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_SEGFREE may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=111.FSH_SEGREALLOC - Change segment size

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function changes the size of a segment previously allocated with FSH_
SEGALLOC or loaded as part of the FSD image&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEGREALLOC(sel, cbSeg)

unsigned short sel;
unsigned long cbSeg;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.sel is the selector to be changed&per.   
:p.cbSeg is the new size to set for the segment&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_NOT_ENOUGH_MEMORY 
.br 
too much memory is allocated&per.   
:p.oERROR_INVALID_ACCESS 
.br 
the selector is invalid 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The segment may be grown or shrunk&per. The segment may be moved in the 
process&per. When grown, the extra space is uninitialized&per.   
:p.FSH_SEGREALLOC may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=112.FSH_SEMCLEAR - Clear a semaphore

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to release a semaphore that was previously 
obtained on a call to FSH_SEMREQUEST&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEMCLEAR(pSem)

void far * pSem;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pSem is the handle to the system semaphore or the long address of the ram 
semaphore&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_EXCL_ALREADY_OWNED 
.br 
the exclusive semaphore is already owned&per.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
the semaphore is inaccessible&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_SEMCLEAR may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=113.FSH_SEMREQUEST - Request a semaphore

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to obtain exclusive access to a semaphore&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEMREQUEST(pSem, cmsTimeout)

void far * pSem;
unsigned long cmsTimeout;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pSem is the handle to the system semaphore or the long address of the ram 
semaphore&per.   
:p.cmsTimeout is the number of milliseconds to wait&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INTERRUPT 
.br 
the current thread received a signal&per.   
:p.oERROR_SEM_TIMEOUT 
.br 
the time-out expired without gaining access to the semaphore&per.   
:p.oERROR_SEM_OWNER_DIED 
.br 
the owner of the semaphore died&per.   
:p.oERROR_TOO_MANY_SEM_REQUESTS 
.br 
there are too many semaphore requests in progress&per.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
the semaphore is inaccessible&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The time-out value of 0xFFFFFFFF indicates an indefinite time-out&per.   
:p.The caller may receive access to the semaphore after the time-out period has 
expired without receiving an ERROR_SEM_TIMEOUT&per. Semaphore time-out values, 
therefore, should not be used for exact timing and sequencing&per.   
:p.FSH_SEMREQUEST may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=114.FSH_SEMSET - Set a semaphore

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to set a semaphore unconditionally&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEMSET(pSem)

void far * pSem;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pSem is the handle to the system semaphore or the long address of the ram 
semaphore&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_INTERRUPT 
.br 
the current thread received a signal&per.   
:p.oERROR_EXCL_SEM_ALREADY_OWNED 
.br 
the exclusive semaphore is already owned&per.   
:p.oERROR_TOO_MANY_SEM_REQUESTS 
.br 
there are too many semaphore requests in progress&per.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
the semaphore is inaccessible&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSH_SEMSET may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=115.FSH_SEMSETWAIT - Set a semaphore and wait for clear

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to wait for an event&per. The event is signaled 
by a call to FSH_SEMCLEAR&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEMSETWAIT(pSem, cmsTimeout)

void far * pSem;
unsigned long cmsTimeout;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pSem is the handle to the system semaphore or the long address of the ram 
semaphore&per.   
:p.cmsTimeout is the number of milliseconds to wait&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_SEM_TIMEOUT 
.br 
the time-out expired without gaining access to the semaphore&per.   
:p.oERROR_INTERRUPT 
.br 
the current thread received a signal&per.   
:p.oERROR_EXCL_SEM_ALREADY_OWNED 
.br 
the exclusive semaphore is already owned&per.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
the semaphore is inaccessible&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The caller may return after the time-out period has expired without receiving 
an ERROR_SEM_TIMEOUT&per. Semaphore time-out values, therefore, should not be used 
for exact timing and sequence&per.   
:p.FSH_SEMSETWAIT may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=116.FSH_SEMWAIT - Wait for clear

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function allows an FSD to wait for an event&per. The event is signaled 
by a call to FSH_SEMCLEAR&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SEMWAIT(pSem, cmsTimeout)

void far * pSem;
unsigned long cmsTimeout;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pSem is the handle to the system semaphore or the long address of the ram 
semaphore&per.   
:p.cmsTimeout is the number of milliseconds to wait&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per.  If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_SEM_TIMEOUT 
.br 
the time-out expired without gaining access to the semaphore&per.   
:p.oERROR_INTERRUPT 
.br 
the current thread received a signal&per.   
:p.oERROR_PROTECTION_VIOLATION 
.br 
the semaphore is inaccessible&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The caller may return after the time-out period has expired without receiving 
an ERROR_SEM_TIMEOUT&per. Semaphore time-out values, therefore, should not be used 
for exact timing and sequence&per.   
:p.FSH_SEMWAIT may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=117.FSH_SETVOLUME - Force a volume to be mounted on the drive

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for assuring that a desired volume is in 
a removable media drive before I/O is done to the drive&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_SETVOLUME(hVPB , fControl)

unsigned short hVPB;
unsigned short fControl;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.hVPB is the volume handle for the source of I/O&per.   
:p.fControl is the bit mask of pop-up control actions&colon.   
:p.Bit 0 off indicates volume change pop-up desired 
.br 
Bit 0 on indicates no volume change pop-up   
:p.All other bits are reserved and must be zero&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_VOLUME_CHANGED 
.br 
is an indication that removable media volume change has occurred&per.   
:p.oERROR_INVALID_PARAMETER 
.br 
the fControl flag word has reserved bits on&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This routine is used by the FSH_CALLDRIVER routine to insure that the desired 
volume is in a removable media drive&per. FSDs can use it for the same purpose&per.   
:p.FSH_SETVOLUME may block&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=118.FSH_STORECHAR - Store a character in a string

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for storing a character into a string, 
taking into account DBCS considerations&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_STORECHAR(chDBCS, ppStr)

unsigned short chDBCS;
char far * far * ppStr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.chDBCS is the character to be stored&per. This may be either a single-byte 
character or a double-byte character with the first byte occupying the low-order position
&per.   
:p.ppStr is the pointer to the character pointer where the character will be 
stored&per. This pointer is updated upon return&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The FSD is responsible for verifying the string pointer and checking for 
segment boundaries&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=119.FSH_UPPERCASE - Uppercase ASCIIZ string

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function is used to uppercase an ASCIIZ string&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_UPPERCASE(szPathName, cbPathBuf, pPathBuf)

char far * szPathName;
unsigned short cbPathBuf;
char far * pPathBuf;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.szPathName is a pointer to the ASCIIZ pathname to be uppercased&per.   
:p.cbPathBuf is the length of the pathname buffer&per.   
:p.pPathBuf is a pointer to the buffer to copy the uppercased path into 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, the following error code is returned&colon.   
:p.oERROR_BUFFER_OVERFLOW 
.br 
uppercased pathname is too long to fit into buffer&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This routine processes DBCS characters properly&per.   
:p.The FSD is responsible for verifying the string pointers and checking for 
segment boundaries&per.   
:p.szPathName and pPathBuf may point to the same place in memory&per.   
:p.FSH_UPPERCASE should be called for names passed into the FSD in raw data 
packets which are not passed to FSH_CANONICALIZE and should be uppercased, that is, 
extended attribute names&per.   
:p.:hp2.Note&colon.   :ehp2.OS/2 does not validate input parameters&per. Therefore, an FSD 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=120.FSH_WILDMATCH - Match using OS/2 wildcards

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for using OS/2 wildcard semantics to 
form a match between an input string and a pattern, taking into account DBCS 
considerations&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal FSH_WILDMATCH(pPat, pStr)

char far * pPat;
char far * pStr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pPat is the pointer to an ASCIIZ pattern string&per. Wildcards are present 
and are interpreted as described below&per.   
:p.ppStr is the pointer to the test string&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, the following error code is returned&colon.   
:p.oERROR_NO_META_MATCH 
.br 
the wildcard match failed&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Wildcards provide a general mechanism for pattern matching file names&per. 
There are two distinguished characters that are relevant to this matching&per. The 
&apos.?&apos. character matches one character (not bytes) except at a &apos.&per.
&apos. or at the end of a string, where it matches zero characters&per. The &apos.*
&apos. matches zero or more characters (not bytes) with no implied boundaries except 
the end-of-string&per.   
:p.For example, &osq.a*b&osq. matches &osq.ab&osq. and &osq.aCCCCCCCCC&osq. 
while &osq.a?b&osq. matches &osq.aCb&osq. but does not match &osq.aCCCCCCCCCb&osq.   
:p.See the section on meta characters in this document for additional 
information&per.   
:p.The FSD should uppercase the pattern and string before calling FSH_WILDMATCH 
to achieve a case-insensitive compare&per.   
:p.:hp2.Note&colon.  :ehp2.OS/2 does not validate input parameters&per. An FSD, therefore, 
should call FSH_PROBEBUF where appropriate&per.   
:h2 id=121.FSH_YIELD - Yield processor to higher-priority thread

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.This function provides the mechanism for relinquishing the processor to 
higher priority threads&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.void far pascal FSH_YIELD(void)

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.FSDs run under the 2 mS dispatch latency imposed on the OS/2 kernel, meaning 
that no more than 2 mS can be spent in an FSD without an explicit block or yield
&per. FSH_YIELD will test to see if another thread is runable at the current thread
&apos.s priority or at a higher priority&per. If one exists, that thread will be given 
a chance to run&per.   
:h1 id=122.Remote IPL / Bootable IFS

:p.This chapter describes the OS/2 Version 2&per.0 boot architecture and 
extensions to the installable file system mechanism (IFSM) to enable booting from an FSD-
managed volume, referred to as Bootable IFS (BIFS)&per. If the volume is on a remote 
system, it is referred to as Remote IPL (RIPL)&per.   
:p.The mini-FSD is similar to the FSD defined in this document&per. However, it 
has additional requirements for to allow reading of the boot drive before the base 
device drivers are loaded&per.  These requirements are fully defined in the two 
interface sections of this chapter&per.   
:p.To satisfy its I/O requests, the mini-FSD may call the disk device device 
driver imbedded in OS2KRNL (BIFS case) or it may provide its own driver (RIPL case)
&per.   
:p.Along with the mini-FSD, the IFS SYS Utility is required to initialize an FSD
-managed volume with whatever is required to satisfy the requirements of the mini
-FSD and this document&per.   
:p.The IFS mechanism includes some additional calls which the mini-FSD may need 
while it is linked into the IFS chain&per.   
:h2 id=123.Operational Description
  
:h3 id=124.FAT Boot Procedure

:p.The following figure represents the major stages of the OS/2 Version 2&per.0 
FAT boot procedure&per.   
:cgraphic.
:font facename='Courier' size=12x12.ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ time
    POST       BOOT      OS2BOOT    OS2LDR     stage1     stage2     stage3
              SECTOR     (OS2LDR               OS2KRNL
                          loader)

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 4-1&per. OS/2 Version 2&per.0 FAT boot procedure :ehp2.  
:p.Powering-on the machine or pressing CTRL-ALT-DEL causes control to get 
transferred to the power-on-self-test (POST) code&per. This code initializes the interrupt 
vectors to get to the BIOS routines&per. It then scans the I/O adapters looking for and 
linking in any code which exists on them&per. It then executes an interrupt 19h (INT 19
) which causes control to be transferred to the disk or diskette boot code&per.   
:p.The INT 19h code reads the boot sector from disk or diskette into memory at 
7C00H&per. Along with code, the boot sector contains a structure called the BIOS 
Parameter Block(BPB)&per. The BPB contains information which describes how the disk is 
formatted&per. The boot code uses this information to load in the root directory and the 
FAT micro-IFS, which is kept inside the OS2BOOT file&per. After the micro-IFS is 
loaded the boot sector transfer control it via a far jump&per.   
:p.OS2BOOT receives pointers to the RAM copies of the root directory and the BPB
&per. Using the BPB information, OS2BOOT loads in the FAT table from the disk&per. 
Then using the root directory and the FAT table, the OS2LDR file is loaded into 
memory from disk&per. The inclusion of this micro-IFS in the FAT boot process has 
removed the requirement that the OS2LDR file be logically contiguous on the FAT drive
&per.   
:p.OS2LDR contains the OS/2 loader&per. It relocates itself to the top of low 
memory, then scans the root directory for OS2KRNL and reads it into memory&per. After 
the required fixups are applied, control is transferred to OS2KRNL, along with a 
pointer to the BPB and the drive number&per.   
:p.OS2KRNL contains the OS/2 kernel and initialization code&per. It switches to 
protected mode, relocates parts of itself to high memory, then scans the root directory 
for and reads in the base device drivers (stage 1)&per. Once again, the BIOS 
interrupt 13h is used to read the disk, but mode switching must be done&per.   
:p.OS2KRNL then switches to protection level 3 and loads some of the required 
dynamic link libraries (stage 2) followed by the device drivers and FSDs specified in 
CONFIG&per.SYS (stage 3)&per. This is done with standard DOS calls and, therefore, 
goes through the regular file system and device drivers&per.   
:h3 id=125.BIFS Boot Procedure

:p.The following figure represents the major stages of the OS/2 Version 2&per.0 
BIFS boot procedure&per.   
:cgraphic.
:font facename='Courier' size=12x12.ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ time
    POST     BlackBox    OS2LDR     stage1     stage2      stage3
             (Micro                 OS2KRNL
                 FSD)

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 4-2&per. OS/2 Version 2&per.0 BIFS boot procedure :ehp2.  
:p.The major difference between this boot procedure and the FAT boot procedure 
is that there is no assumption of booting off of disk&per. OS/2 Version 2&per.0 
does not define what should happen between when the POST code is run and when the 
OS2LDR program gains control&per.   
:p.When OS2LDR receives control, it must be passed information about the current 
state of memory and pointers to the Open, Read, Close, and Terminate entry points of 
the micro-FSD&per. Included in the memory map information is the positions of the 
micro-FSD, mini-FSD, RIPL data, and the OS2LDR file itself&per.   
:p.:hp2.Note&colon.   :ehp2.This interface is defined in a next section of this chapter&per.   
:p.As with the FAT boot procedure, the OS/2 loader relocates itself to the top 
of low memory, and with the help of the micro-FSD, scans the root directory for 
the OS2KRNL file&per. After reading OS2KRNL into memory and applying the required 
fixups, control is transferred to the kernel&per.   
:p.When OS2KRNL receives control, it goes through the same initialization as 
before (stage 1) with a couple of exceptions&per. The module loader is called to load 
the mini-FSD from its memory image stored by OS2LDR in high memory to its final 
location at the top of low memory&per. Also, the mini-FSD is called to read the base 
device drivers (one at a time) through the stage 1 interfaces&per.   
:p.Before any of the dynalinks are loaded, the mini-FSD will be linked into the 
IFS chain (it will be the only link in the chain) and asked to initialize through 
FS_INIT&per. The FS_INIT call marks the transition from stage 1 to stage 2&per.   
:p.The dynalinks are then loaded using the stage 2 interfaces, followed by the 
device drivers and FSDs&per.   
:p.The mini-FSD is required to support only a small number of the FSD system 
interfaces (the FS_xxxx calls)&per. Therefore, the first FSD loaded must be the 
replacement for the mini-FSD&per.   
:p.After the replacement FSD has been loaded, it is called at FS_INIT to 
initialize itself and take whatever action it needs to effect a smooth transition from the 
mini-FSD to the FSD&per. It then replaces the mini-FSD in the IFS chain, as well as 
in any kernel data structures which keep a handle to the FSD (for example, the SFT
, VPB)&per. This replacement marks the transition from stage 2 to stage 3&per.   
:p.From this point on, the system continues normally&per.   
:h2 id=126.Interfaces
  
:h3 id=127.BlackBox/OS2LDR interface

:p.When initially transferring control to OS2LDR from a &osq.black box&osq., the 
following interface is defined&colon.   
:p.DH boot mode flags&colon. 
:p.bit 0 (NOVOLIO) on indicates that the mini-FSD does not use MFSH_DOVOLIO&per. 
.br 
bit 1 (RIPL) on indicates that boot volume is not local (RIPL boot) 
.br 
bit 2 (MINIFSD) on indicates that a mini-FSD is present&per. 
.br 
bit 3 (RESERVED) 
.br 
bit 4 (MICROFSD) on indicates that a micro-FSD is present&per. 
.br 
bits 5-7 are reserved and MUST be zero&per.   
:p.DL drive number for the boot disk&per. This parameter is ignored if either 
the NOVOLIO or MINIFSD bits are zero&per.   
:p.DS&colon.SI is a pointer to the BOOT Media&apos.s BPB&per. This parameter is 
ignored if either the NOVOLIO or MINIFSD bits are zero&per.   
:p.ES&colon.DI is a pointer to a filetable structure&per. The filetable 
structure has the following format&colon.   
:cgraphic.
:font facename='Courier' size=12x12.struct FileTable {
    unsigned short ft_cfiles; /* # of entries in this table             */
    unsigned short ft_ldrseg; /* paragraph # where OS2LDR is loaded     */
    unsigned long  ft_ldrlen; /* length of OS2LDR in bytes              */
    unsigned short ft_museg;  /* paragraph # where microFSD is loaded   */
    unsigned long  ft_mulen;  /* length of microFSD in bytes            */
    unsigned short ft_mfsseg; /* paragraph # where miniFSD is loaded    */
    unsigned long  ft_mfslen; /* length of miniFSD in bytes             */
    unsigned short ft_ripseg; /* paragraph # where RIPL data is loaded  */
    unsigned long  ft_riplen; /* length of RIPL data in bytes           */

    /* The next four elements are pointers to microFSD entry points     */

    unsigned short (far *ft_muOpen)
                     (char far *pName, unsigned long far *pulFileSize);
    unsigned long (far *ft_muRead)
                     (long loffseek, char far *pBuf, unsigned long cbBuf);
    unsigned long (far *ft_muClose)(void);
    unsigned long (far *ft_muTerminate)(void);
}

:ecgraphic.
:font facename=default.
:p.The microFSD entry points interface is defined as follows&colon.   
:p.mu_Open - is passed a far pointer to name of file to be opened and a far 
pointer to a ULONG to return the file&apos.s size&per. The returned value (in AX) 
indicates success(0) or failure(non-0)&per.   
:p.mu_Read - is passed a seek offset, a far pointer to a data buffer, and the 
size of the data buffer&per. The returned value(in DX&colon.AX) indicates the number 
of bytes actually read&per.   
:p.mu_Close - has no parameters and expects no return value&per. It is a signal 
to the micro-FSD that the loader is done reading the current file&per.   
:p.mu_Terminate - has no parameters and expects no return value&per. It is a 
signal to the micro-FSD that the loader has finished reading the boot drive&per. 
.br 

:p.The loader will call the micro-FSD in a Open-Read-Read-&per.&per.&per.&per.-
Read-Close sequence with each file read in from the boot drive&per.   
:h3 id=128.miniFSD/OS2KRNL interface

:p.When called from OS2KRNL after being linked into the IFS chain, the interface 
will be as described in previous chapters of this document&per. Note that the FS_
INIT interface for a mini-FSD has an additional parameter, as compared to the FS_
INIT interface for an FSD&per.   
:p.When called from OS2KRNL, before being linked into the IFS chain, the 
interface will be through the MFS_xxxx and MFSH_xxxx entry points&per. These interfaces 
are described in this chapter&per. Many of these interfaces parallel the interfaces 
defined for FSDs, while others are unique to the mini-FSD&per.   
:p.The mini-FSD is built as a dynamic link library&per. Supplied functions are 
exported by making the function names public&per. Helper functions are imported by 
declaring the helper names external&colon.far&per. It is required only to support reading 
files and will be called only in protect mode&per. The mini-FSD may NOT make dynamic 
link system calls at initialization time&per.   
:p.Due to the special state of the system as it boots, the programming model for 
the mini-FSD during the state 1 time frame is somewhat different than the model for 
stage 2&per. This difference necessitates 2 different interfaces between OS/2 and the 
mini-FSD&per.   
:p.During stage 1, all calls to the mini-FSD are to the MFS_xxxx functions&per. 
Only the MFSH_xxxx helper functions are available&per. These are the interfaces 
which are addressed in this document&per. Many of these interfaces parallel the 
interfaces defined for FSDs while others are unique to the mini-FSD&per.   
:p.During stage 2, the mini-FSD is treated as a normal FSD&per. Calls are made 
to the FS_xxxx functions and all FSH_xxxx helper functions are available&per.   
:p.During stage 3, the mini-FSD is given a chance to release resources (through 
a call to MFS_TERM) before being terminated&per.   
:p.Transition from stage 1 to stage 2 is marked by calling the FS_INIT function 
in the mini-FSD&per. Transition from stage 2 to stage 3 is marked by calling FS_
INIT in the FSD&per.   
:p.Figure 4-3 on page 4-6 shows the functions called during a typical boot 
sequence&colon.   
:cgraphic.
:font facename='Courier' size=12x12.ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ time
      stage1               stage2               stage3

  MFS_INIT
  MFS_OPEN
  MFS_READ
  MFS_CHGFILEPTR
  MFS_CLOSE

                       FS_INIT
                       FS_MOUNT/ATTACH
                       FS_OPEN
                       FS_READ
                       FS_CHGFILEPTR

                                               MFS_TERM

:ecgraphic.
:font facename=default.
:p.:hp2.Figure 4-3&per. Typical boot sequence :ehp2.  
:p.No files are open at the transition from stage 1 to stage 2&per. Also, only a 
single file at a time is open during stage 1&per. Files and volumes are open during 
the transition from stage 2 to stage 3 (the mini-FSD to the FSD)&per. The FSD must 
do whatever is necessary for it to inherit them&per. The FSD will not receive 
mounts/attaches or opens for volumes and files which were mounted/attached and opened 
by the mini-FSD&per. Also, multiple files may be open simultaneously during stages 
2 and 3&per.   
:p.A special set of helper functions are available to the mini-FSD to support an 
imbedded device driver&per. This might be required for situations such as remote IPL 
where the boot volume is not readable through DOVOLIO&per. These special helper 
functions (referred to as imbedded device driver helpers) are available during all stages 
of the mini-FSD&apos.s life&per. Note that the list of error return codes for the 
helper functions is not exhaustive, but rather represents the most common errors 
returned&per.   
:p.Because the mini-FSD is a new component added to the boot sequence, a new 
interface to OS2LDR is required&per.   
:p.The name and attributes of the mini-FSD must match EXACTLY the name and 
attributes of the replacement FSD&per.   
:p.Due to the instability of the system during initialization, any non-zero 
return code indicates an error has been encountered&per. The actual return code may 
not bake any sense in the context of the function called (for example, having ERROR
_ACCESS_DENIED returned from a call to MFSH_LOCK when in fact an invalid selector 
was passed to the helper)&per. It is also possible for the system to hang or reboot 
itself as a result of invalid parameters being passed to a helper function&per.   
:h3 id=129.Stage 1 Interfaces

:p.The following functions must be made available by the mini-FSD&per. These 
functions will be called only during stage 1&per.   
:p.oMFS_CHGFILEPTR 
.br 
oMFS_CLOSE 
.br 
oMFS_INIT 
.br 
oMFS_OPEN 
.br 
oMFS_READ 
.br 
oMFS_TERM 
.br 

:p.The following helper functions are available to the mini-FSD&per. These 
functions may be called only during stage 1&per.   
:p.oMFSH_DOVOLIO 
.br 
oMFSH_INTERR 
.br 
oMFSH_SEGALLOC 
.br 
oMFSH_SEGFREE 
.br 
oMFSH_SEGREALLOC 
.br 
  
:h3 id=130.Stage 2 Interfaces

:p.The intent of stage 2 is to use the mini-FSD as an FSD&per. Therefore, all 
the guidelines and interfaces specified in this document apply with the following 
exceptions&per.   
:p.The following functions must be fully supported by the mini-FSD&colon.   
:p.oFS_ATTACH (remote mini-FSD only) 
.br 
oFS_ATTRIBUTE 
.br 
oFS_CHGFILEPTR 
.br 
oFS_CLOSE 
.br 
oFS_COMMIT 
.br 
oFS_INIT 
.br 
oFS_IOCTL 
.br 
oFS_MOUNT (local mini-FSD only) 
.br 
oFS_NAME 
.br 
oFS_OPENCREATE (existing file only) 
.br 
oFS_PROCESSNAME 
.br 
oFS_READ 
.br 

:p.Note that since the mini-FSD is only required to support reading, FS_
OPENCREATE need only support opening an existing file (not the create or replace options)
&per.   
:p.None of the other functions required for FSDs are required for the mini-FSD 
but must be defined and should return the ERROR_UNSUPPORTED_FUNCTION return code
&per.   
:p.The full complement of helper functions specified in this document is 
available to the mini-FSD&per. However, the mini-FSD may NOT use any other dynamic link 
calls&per.   
:h3 id=131.Stage 3 Interfaces

:p.The intent of stage 3 is to throw away the mini-FSD and use only the FSD&per.   
:p.The following functions must be supported by the mini-FSD&colon.   
:p.oMFS_TERM 
.br 
  
:h3 id=132.Imbedded Device Driver Helpers

:p.The following helper functions are available to the mini-FSD and may be 
called during stage 1, 2, or 3&per. These helpers are counterparts for some of the 
device help functions and are intended for use by a device driver imbedded within the 
mini-FSD&per.   
:p.oMFSH_CALLRM 
.br 
oMFSH_LOCK 
.br 
oMFSH_PHYSTOVIRT 
.br 
oMFSH_UNLOCK 
.br 
oMFSH_UNPHYSTOVIRT 
.br 
oMFSH_VIRTTOPHYS 
.br 
  
:h2 id=133.Special Considerations

:p.The size of the mini-FSD file image plus the RIPL data area may not exceed 
62K&per. In addition, the memory requirements of the mini-FSD may not exceed 64K
&per.   
:p.The mini-FSD is only required to support reading of a file&per. Therefore, 
any call to DosWrite (or other non-supported functions) which becomes redirected to 
the mini-FSD may be rejected&per. For this reason, it is required that the IFS= 
command which loads the FSD which will replace the mini-FSD be the first IFS= command 
in CONFIG&per.SYS&per. Also, only DEVICE= commands which load device drivers 
required by that FSD should appear before the first IFS= command&per.   
:p.If the mini-FSD needs to switch to real mode, it must use the MFSH_CALLRM 
function&per. This is required to keep OS/2 informed of the mode switching&per.   
:p.Each FSD which is bootable is required to provide their &osq.black box&osq. 
to load OS2LDR and the mini-FSD into memory before OS2LDR is given control&per.   
:p.Additionally, these FSDs are required to provide a single executable module 
in order to support the OS/2 SYS utility&per. The executable provided will be 
invoked by this utility when performing a SYS for that file system&per. The command 
line that was passed to the utility will be passed unchanged to the executable&per.   
:p.The supplied executable must do whatever is required to make the partition 
bootable&per. At the very least, it must install a boot sector&per. It also needs to 
install the &osq.black box&osq., mini-FSD, OS2LDR and OS2KRNL&per.   
:h1 id=134.mini-FSD Entry Points

:p.The following table is a summary of mini-FSD entry points&colon.   
:cgraphic.
:color fc=default.:color bc=default.
 Entry Point          Description
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=135.MFS_CHGFILEPTR:elink.       Move a file&apos.s position pointer
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=136.MFS_CLOSE:elink.            Close a file&per.
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=137.MFS_INIT:elink.             mini-FSD initialization
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=138.MFS_OPEN:elink.             Open a file
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=139.MFS_READ:elink.             Read from a file
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=140.MFS_TERM:elink.             Terminate the mini-FSD


:ecgraphic.

:p.:hp2.Table 4-1&per. Summary of mini-FSD entry points :ehp2.  
:h2 id=135.MFS_CHGFILEPTR - Move a file's position pointer

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Move the file&apos.s logical read position pointer&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFS_CHGFILEPTR(offset, type)

long offset;
unsigned short type;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.offset is the signed offset which depending on the type parameter is used to 
determine the new position within the file&per.   
:p.type indicates the basis of a seek operation&per. 
:p.type == 0 indicates seek relative to beginning of file&per. 
.br 
type == 1 indicates seek relative to current position within the file&per. 
.br 
type == 2 indicates seek relative to end of file&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, a non-zero error code is returned&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The file system may want to take the seek operation as a hint that an I/O 
operation is about to take place at the new position and initiate a positioning operation 
on sequential access media or read-ahead operation on other media&per.   
:h2 id=136.MFS_CLOSE - Close a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Close a file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFS_CLOSE(void)

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, a non-zero error code is returned&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.None   
:h2 id=137.MFS_INIT - mini-FSD Initialization

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Inform the mini-FSD that it should prepare itself for use&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFS_INIT(pBootData, pucResDrives, pulVectorIPL, pBPB, pMiniFSD,
                        pDumpAddr)

void far * pBootData;
char far * pucResDrives;
long far * pulVectorIPL;
void far * pBPB;
unsigned long far * pMiniFSD;
unsigned long far * pDumpAddr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pBootData is a pointer to the data passed from the black box to the mini-FSD 
(null if not passed)&per.   
:p.pucResDrives is a pointer to a byte which may be filled in by the mini-FSD 
with the number of drive letters (beginning with &apos.C&apos.) to skip over before 
assigning drive letters to local fixed disk drivers (ignored if not remote IPL)&per. The 
system will attach the reserved drives to the mini-FSD through a call to FS_ATTACH 
just after the call to FS_INIT&per.   
:p.pulVectorIPL is a pointer to a double word which may be filled in by the mini
-FSD with a pointer to a data structure which will be available to installable 
device drivers through the standard device helper function GetDosVar (variable number 
12)&per. The first eight bytes of the structure MUST be a signature which would 
allow unique identification of the data by cooperating device drivers (for example, 
IBMPCNET)&per.   
:p.BPB is a pointer to the BPB data structure (see OS2LDR interface)&per.   
:p.pMiniFSD is a pointer to a double word which is filled in by the mini-FSD 
with data to be passed on to the FSD&per.   
:p.DumpRoutine is a pointer to a double word which is filled in by the mini-FSD 
with the address of an alternative stand-alone dump procedure&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, a non-zero error code is returned&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The mini-FSD should fill in the data pointed to by pMiniFSD with any 32-bit 
value it wishes to pass on to the FSD (see FS_INIT)&per. OS/2 makes no assumptions 
about the type of data passed&per. Typically, this will be a pointer to important 
data structures within the mini-FSD which the FSD needs to know about&per.   
:p.OS/2 will not free the segment containing BootData&per. It should be freed by 
the mini-FSD if appropriate&per.   
:p.The DumpProcedure is a routine provided by the mini-FSD which replaces the 
diskette-based OS/2 stand-alone dump procedure&per. This routine is given control after 
the OS/2 kernel receives a stand-alone dump request&per. The OS/2 kernel places the 
machine in a stable, real mode state in which most interrupt vectors contain their 
original power-up value&per. If this address is left at zero, the OS/2 kernel will 
attempt to initiate a storage dump to diskette, if a diskette drive exists&per. The 
provided routine must handle the dumping of storage to an acceptable media&per.   
:h2 id=138.MFS_OPEN - Open a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Open the specified file&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFS_OPEN(pszName, pulSize)

char far * pszName;
unsigned long far * pulSize;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pszName is a pointer to the ASCIIZ name of the file to be opened&per. It may 
include a path but will not include a drive&per.   
:p.pulSize is a pointer to a double word which is filled in by the mini-FSD with 
the size of the file in bytes&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, a non-zero error code is returned&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Only one file at a time will be opened by this call&per. The drive will 
always be the boot drive&per.   
:p.The current file position is set to the beginning of the file&per.   
:h2 id=139.MFS_READ - Read from a file

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Read the specified number of bytes from the file to a buffer location&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFS_READ(pcData, pusLength)

char far * pcData;
unsigned long far * pusLength;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcData is a pointer to the data area to be read into&per. The data area is 
guaranteed to be below the 1-Meg boundary&per.   
:p.pusLength is a pointer to a word which on entry specifies the number of bytes 
to be read&per. On return, it is filled in by the mini-FSD with the number of 
bytes successfully read&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, a non-zero error code is returned&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The current file position is advanced by the number of bytes read&per.   
:h2 id=140.MFS_TERM - Terminate the mini-FSD

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Inform the mini-FSD that it should prepare itself for termination&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFS_TERM(void)

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, a non-zero error code is returned&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The system will NOT free any memory explicitly allocated by the mini-FSD 
through MFSH_SEGALLOC or FSH_SEGALLOC&per. It must be explicitly freed by the mini-FSD
&per. (Memory allocated by the mini-FSD and &apos.given&apos. to the FSD need not be 
freed&per.) The system will free all of the segments loaded as part of the mini-FSD 
image immediately after this call&per.   
:h1 id=141.mini-FSD Helper Routines

:p.The following table summaries the mini-FSD Helper Routines&colon.   
:cgraphic.
:color fc=default.:color bc=default.
 FSD Helper           Description
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=142.MFSH_CALLRM:elink.          Put machine in real mode
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=143.MFSH_DOVOLIO:elink.         Read sectors
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=144.MFSH_INTERR:elink.          Internal error
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=145.MFSH_LOCK:elink.            Lock segment
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=146.MFSH_PHYSTOVIRT:elink.      Convert physical to virtual address
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=147.MFSH_SEGALLOC:elink.        Allocate a segment
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=148.MFSH_SEGFREE:elink.         Free a segment
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=149.MFSH_SEGREALLOC:elink.      Change segment size
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=150.MFSH_SETBOOTDRIVE:elink.    Change boot drive number kept by the OS/2 kernel
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=151.MFSH_UNLOCK:elink.          Unlock a segment
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=152.MFSH_UNPHYSTOVIRT:elink.    Mark completion of use of virtual address
 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 :link reftype=hd refid=153.MFSH_VIRT2PHYS:elink.       Convert virtual to physical address


:ecgraphic.

:p.:hp2.Table 4-2&per. Summary of mini-FSD Helpers :ehp2.  
:h2 id=142.MFSH_CALLRM - Put machine in real mode

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Put the machine into real mode, call the specified routine, put the machine 
back into protect mode, and return&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_CALLRM(plRoutine)

unsigned long far * plRoutine;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.plRoutine is a pointer to a double word which contains the VIRTUAL address of 
the routine to call&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.Only registers DS and SI will be preserved between the caller and the target 
routine&per. The selector in DS will be converted to a segment before calling the 
target routine&per. Arguments may not be passed on the stack since a stack switch may 
occur&per.   
:p.This helper allows the mini-FSD to access the ROM BIOS functions which 
typically run in real mode only&per. Great care must be taken in using this function 
since selectors used throughout the system are meaningless in real mode&per. While in 
real mode, no calls to any helpers may be made&per.   
:h2 id=143.MFSH_DOVOLIO - Read sectors

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Read the specified sectors&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_DOVOLIO(pcData, pcSec, ulSec)

char far * pcData;
unsigned short far * pcSec;
unsigned long ulSec;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcData is a pointer to the data area&per. The data area must be below the 1-
Meg boundary&per.   
:p.pcSec is a pointer to the word which specifies the number of sectors to be 
read&per. On return, it is filled in by the helper with the number of sectors 
successfully read&per.   
:p.ulSec is the sector number for the beginning of the sector run&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied address or length is invalid&per.   
:p.oERROR_INVALID_FUNCTION 
:p.either bit 0 of the boot mode flags was set on entry to OS2LDR or the system 
is not in stage 1&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.The only media which can be read by this call is the boot volume&per. The 
machine&apos.s interrupt 13H BIOS function is used to actually do the disk reads&per. 
The data area will be locked and unlocked by this helper&per. Soft errors are 
retried automatically&per. Hard errors are reported to the user through a message and 
the system is stopped&per.   
:h2 id=144.MFSH_INTERR - Internal Error

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Declare an internal error and halt the system&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_INTERR(pcMsg, cbMsg)

char far * pcMsg;
unsigned short cbMsg;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.pcMsg is a pointer to the message text&per.   
:p.cbMsg is the length of the message text&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.There are no error returns&per.   
:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This call should be used when an inconsistency is detected within the mini-
FSD&per. This call does not return&per. An error message will be displayed and the 
system will be stopped&per. See the description of FSH_INTERR&per.   
:h2 id=145.MFSH_LOCK - Lock a segment

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Lock a segment in place in physical memory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_LOCK(usSel, pulHandle)

unsigned short usSel;
unsigned long far * pulHandle;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.usSel is the selector of the segment to be locked&per.   
:p.pulHandle is a pointer to a double word which is filled in by the helper with 
the lock handle&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied address or selector is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This helper is for use by a mini-FSD with an imbedded device driver&per. It 
is the same as the standard device driver LOCK helper with the following 
assumptions&colon. The lock is defined to be short term and will block until the segment is 
loaded&per.   
:h2 id=146.MFSH_PHYSTOVIRT - Convert physical to virtual address

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Translate the physical address of a data buffer into a virtual address&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_PHYSTOVIRT(ulAddr, usLen, pusSel)

unsigned long ulAddr;
unsigned short usLen;
unsigned short far * pusSel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.ulAddr is the physical address to be translated&per.   
:p.usLen is the length of the segment for the physical address&per.   
:p.pusSel is a pointer to the word in which the selector or segment is returned
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If an error is not detected, a zero error code is returned&per. If an error 
is detected, the following error is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied address is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This helper is for use by a mini-FSD with an imbedded device driver&per. It 
is the same as the standard device driver helper PHYSTOVIRT&per. A segment/offset 
pair is returned in real mode for addresses below the 1-Meg boundary&per. Else a 
selector/offset pair is returned&per.   
:p.A caller must issue a corresponding UNPHYSTOVIRT before returning to its 
caller or using any other helpers&per.   
:h2 id=147.MFSH_SEGALLOC - Allocate a segment

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Allocate memory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_SEGALLOC(usFlag, cbSeg, pusSel)

unsigned short usFlag;
unsigned long cbSeg;
unsigned short far * pusSel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.usFlag is set to 1 if the memory must be below the 1-meg boundary or 0 if its 
location does not matter&per.   
:p.cbSeg contains the length of the segment&per.   
:p.pusSel is a pointer to a word in which the helper returns the selector of the 
segment&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, one of the following error codes is returned&colon.   
:p.oERROR_NOT_ENOUGH_MEMORY 
:p.too much memory is allocated&per.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied address is invalid&per.   
:p.oERROR_INVALID_PARAMETER 
:p.either the supplied flag or length is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This function allocates memory with the following attributes&colon.   
:p.oAllocated from the GDT   
:p.oNon-swappable 
.br 

:p.Memory not allocated specifically below the 1-Meg boundary may be given to 
the FSD by passing the selectors through pMiniFSD (see MFS_INIT and FS_INIT)&per.   
:h2 id=148.MFSH_SEGFREE - Free a segment

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Free a memory segment&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_SEGFREE(usSel)

unsigned short usSel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.usSel contains the selector of the segment to be freed&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error error code is returned&per. If an error 
is detected, the following error code is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the selector is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This function releases a segment previously allocated with MFSH_SEGALLOC, or 
loaded as part of the mini-FSD image&per.   
:h2 id=149.MFSH_SEGREALLOC - Change segment size

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Change the size of memory&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_SEGREALLOC(usSel, cbSeg)

unsigned short usSel;
unsigned long cbSeg;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.usSel contains the selector of the segment to be resized&per.   
:p.cbSeg contains the new length of the segment&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, on of the following error codes is returned&colon.   
:p.oERROR_NOT_MEMORY 
:p.too much memory is allocated&per.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied selector is invalid&per.   
:p.oERROR_INVALID_PARAMETER 
:p.the supplied length is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This call changes the size of a segment previously allocated with MFSH_
SEGALLOC, or loaded as part of the mini-FSD image&per.   
:p.The segment may be grown or shrunk&per. When grown, the extra space is 
uninitialized&per. The segment may be moved in the process&per.   
:h2 id=150.MFSH_SETBOOTDRIVE - Change boot drive number kept by the OS/2 kernel

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Change boot drive number kept by the kernel to allow a change in the 
assignment of boot drive as seen by later processes&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_SETBOOTDRIVE(usDrive)

unsigned short usDrive;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.usDrive contains the 0-based drive number that the mini-FSD wants the system 
to consider as the boot drive&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, on of the following error codes is returned&colon.   
:p.oERROR_INVALID_PARAMETER 
:p.the supplied drive number is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This call changes the boot drive number that is kept in the global info 
segment of the system&per. Valid values range from 2 (=C) to 25 (=Z)&per. This function 
must be called during the call to MFS_INIT to update the info segment correctly&per. 
This is routine should be used by RIPL mini-FSDs&per.   
:h2 id=151.MFSH_UNLOCK - Unlock a segment

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Unlock a segment which was previous locked by calling MFSH_LOCK&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_SEGREALLOC(ulHandle)

unsigned long ulHandle;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.ulHandle contains the handle returned from MFSH_LOCK of the segment to unlock
&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, the following error code is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied address is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This helper is for use by a mini-FSD with an imbedded device driver&per. It 
is the same as the standard device driver helper UNLOCK&per.   
:h2 id=152.MFSH_UNPHYSTOVIRT - Mark completion of use of virtual address

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Release the selector allocated previously by calling MFSH_PHYSTOVIRT&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_UNPHYSTOVIRT(usSel)

unsigned short usSel;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.usSel contains the selector to released&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, the following error code is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied selector is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This helper is for use by a mini-FSD with an imbedded device driver&per. It 
is the same as the standard device driver UNPHYSTOVIRT helper&per.   
:p.A caller must issue a corresponding UNPHYSTOVIRT after calling PHYSTOVIRT, 
before returning to its caller or using any other helpers&per.   
:h2 id=153.MFSH_VIRT2PHYS - Convert virtual to physical address

:p.:font facename='Helv' size=18x18.:hp2.Purpose :ehp2.:font facename=default.
:p.Translate the address of a data buffer into a physical address&per.   
:p.:font facename='Helv' size=18x18.:hp2.Calling Sequence :ehp2.:font facename=default.
:cgraphic.
:font facename='Courier' size=12x12.int far pascal MFSH_VIRT2PHYS(ulVirtAddr, pulPhysAddr)

unsigned long ulVirtAddr;
unsigned long far * pulPhysAddr;

:ecgraphic.
:font facename=default.
:p.:font facename='Helv' size=18x18.:hp2.Where :ehp2.:font facename=default.
:p.ulVirtAddr contains the virtual address of the data area&per.   
:p.PhysAddr is a pointer to a double word in which the helper returns the 
physical address of the data area&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Returns :ehp2.:font facename=default.
:p.If no error is detected, a zero error code is returned&per. If an error is 
detected, the following error is returned&colon.   
:p.oERROR_PROTECTION_VIOLATION 
:p.the supplied address is invalid&per. 
.br 

:p.:font facename='Helv' size=18x18.:hp2.Remarks :ehp2.:font facename=default.
:p.This helper is for use by a mini-FSD with an imbedded device driver&per. It 
is the same as the standard device driver helper VIRTTOPHYS&per.   :euserdoc.
