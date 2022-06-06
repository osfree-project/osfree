.* ****************************************************************************
:h1 x=left y=bottom width=100% height=100% id=errors res=12000.Error messages
:p.This following sections describe the various error messages which can be
returned by the LVM engine.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12001.01 - Out of Memory
:p.:hp2."Out of memory":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.Not enough memory was available for the LVM engine to process the request.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Try closing LVMPM and starting it again. Also try closing any other
nonessential programs that are presently running.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12002.02 - I/O Error
:p.:hp2."I/O error":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine was unable to read from or write to a disk.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Make sure the disk is properly connected. If this is a removable storage
device, please verify that the device is working properly and has been inserted
correctly.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12003.03 - Invalid Handle
:p.:hp2."Invalid handle":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The application passed an invalid handle to the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12004.04 - Internal Error
:p.:hp2."Internal error":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.An internal error was detected by the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12005.05 - LVM Engine Is Already Open
:p.:hp2."LVM engine is already open":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.This error may occur if a program attempts to reopen the LVM engine
without closing it first.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12006.06 - LVM Engine Is Not Open
:p.:hp2."LVM engine is not open":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The application is attempting to call an LVM engine function, but the LVM
engine has not been opened.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12007.07 - Name Exceeds Maximum Allowed Length
:p.:hp2."Name exceeds maximum allowed length":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The application has specified a device name which exceeds the allowable
length.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12008.08 - Operation Is Not Allowed
:p.:hp2."Operation is not allowed":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The requested operation is not allowed.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.You can try any of the following&colon.
:ul.
:li.Close LVMPM and re-open it, then try the operation again.
:li.Reboot and then try the operation again.
:li.Try the operation with slightly different parameters.
:li.Contact support and describe as precisely as possible the operation which
is failing.
:eul.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12009.09 - Drive Open Failure
:p.:hp2."Drive open failure":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine was unable to gain access to one or more disk drives. This
error may occur if another program is already using the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Make sure that no other program which uses LVM functionality is running.
This includes the text-mode LVM program, the Java-based LVMGUI, and the
Installation Volume Manager, as well as certain third-party applications such
as :hp1.DFSee:ehp1..

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12010.10 - Invalid Partition
:p.:hp2."Invalid partition":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The specified partition is not usable.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Select a different partition and try the operation again.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12011.11 - Cannot Create Primary Partition
:p.:hp2."Cannot create primary partition":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.It is not possible to create a primary partition using the requested
parameters.
:p.The most likely cause of this error is that you attempted to create a
primary partition in an illegal location. According to the basic rules of
MBR style disk partitioning, a primary partition cannot be created between
two logical partitions.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Make sure you are not trying to create a primary partition between two
logical partitions.
:p.This could also indicate an internal program error in partition-creation
logic, so you may wish to contact support as well.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12012.12 - Too Many Primary Partitions
:p.:hp2."Too many primary partitions":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.On a single disk drive there is a maximum of either four primary
partitions, or three primary partitions plus one or more logical partitions.
This error is generated if you attempt to exceed this limit.
:p.(Keep in mind that IBM Boot Manager, if installed, occupies one primary
partition.)
:p.
:p.:hp5.Suggested Action:ehp5.
:p.You may have to re-think the layout of your disk drive in order to conform
with the above limit.
:p.This error could also indicate an internal program error
in partition-creation logic, so you may wish to contact support as well.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12013.13 - Cannot Create Logical Partition
:p.:hp2."Cannot create logical partition":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.It is not possible to create the requested logical partition. According
to MBR drive geometry rules&colon.
:ul.
:li.All logical partitions on a disk drive must be adjacent.
:li.Logical partitions cannot be created on a disk which already contains four
primary partitions.
:eul.
:p.This error probably means that you have attempted to violate these rules.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.You may have to re-think the layout of your disk drive in order to conform
with the above limit.
:p.This error could also indicate an internal program error
in partition-creation logic, so you may wish to contact support as well.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12014.14 - Requested Size Is Too Large
:p.:hp2."Requested size is too large":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The requested size of the volume or partition is too large for the available
space.
:p.:hp5.Suggested Action:ehp5.
:p.Try reducing the requested size and trying again.
:p.In addition, since LVMPM should not be allowing you to request an invalid
volume size in the first place, please contact support if you encounter this
error message.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12015.15 - System Does Not Report INT13X Boot Compatibility
:p.:hp2."System does not report INT13X boot compatibility":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.Normally, &os2. supports booting beyond the 1024 cylinder limit of a
hard disk drive (which generally translates to just over 8 GB from the
beginning of the drive). This capability is referred to as the &osq.INT13
Extensions&csq. (INT13X). However, it depends on your computer's BIOS
supporting this feature as well.

:p.If you receive this error message, it means that you have encountered the
following combination of circumstances&colon.
:ol.
:li.You are attempting to either install IBM Boot Manager, create a bootable
volume, or make an existing volume or partition bootable; and
:li.The partition, volume, or Boot Manager location in question extends past
the 1024 cylinder limit on the current hard disk drive; and
:li.The volume management engine is reporting that your computer does not
support INT13X.
:eol.

:p.Some older computers do not support INT13X, and therefore are not capable of
booting from volumes or partitions which extend beyond the 1024 cylinder limit.
:hp2.However:ehp2., receiving this error message does not necessarily mean that
this is the case for your system.

:p.Even on computers which do support INT13X, it may be necessary to rewrite your
Master Boot Record (MBR) in order to enable this feature. See below for details.

:p.Finally, some hard disks may be mis-reporting their true size. (Some IDE hard
disks do this in order to maintain compatibility with older computer BIOSes.)
Both the AHCI (OS2AHCI.ADD) and Enhanced IDE (DANIS506.ADD) drivers attempt to
detect the true size of the hard disk; however, in some circumstances this can
cause the above error to appear. A possible workaround in this case is to reboot
using the Standard IDE driver (IBM1S506.ADD), and try the operation again. See
below for details.

:p.
:p.:hp5.Suggested Action:ehp5.
:p.This error does not necessarily mean that your computer does not support INT13X.
The volume management engine may simply require your hard disk's Master Boot Record
(MBR) to be rewritten.  If this is the case, the problem can usually be resolved by
doing the following&colon.
:ol.
:li.Make sure the first hard disk is selected in the physical view panel, then
go to the :hp1.System:ehp1. menu and choose :hp1.Rewrite MBR:ehp1..
:li.Save your changes and then close LVMPM.
:li.Shut down and reboot the computer.
:li.After the computer comes back up, start LVMPM and try again.
:eol.

:p.If this does not work, you may also try rebooting using the Standard IDE driver
(IBM1S506.ADD) instead of the AHCI (OS2AHCI.ADD) or Enhanced IDE (DANIS506.ADD)
drivers. This disables the IDE disk size-detection mechanism which has been known
to cause this error. Alternatively, you can use the Enhanced IDE driver and specify
the parameters :hp2./A&colon.0 /U&colon.0 /!SETMAX:ehp2. in its load statement
(available either through the boot options menu on the &os2. installation
media, or else by editing CONFIG.SYS on an installed &os2. system).

:p.If the above does not work, you may have to settle for creating all bootable
partitions, and/or installing IBM Boot Manager, near the beginning of the disk(s).

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12016.16 - Partition Alignment Error
:p.:hp2."Partition alignment error":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.This error can occur if the disk drive's partition table is improperly
formatted in some way. Certain partitioning tools can cause this problem.
In particular, many Linux installation utilities and/or partition managers
tend to write data to the partition table in a way that the Logical Volume
Manager cannot properly interpret.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.There is no easy solution to this problem.  The best remedy is to avoid
it in the first place&colon. in general, you are advised to avoid creating or
modifying partitions using Linux-based tools or installers, and instead create
your partitions using &os2. or other (non-Linux) operating systems.
:p.If you do encounter this error, there are a couple of things you can try.
The most extreme solution is to delete all partitions on the disk and recreate
them using LVM. However, this action would erase all data on the hard disk,
which could be undesirable.
:p.The best alternative is to use a partition-management tool which is capable
of recognizing the problem and repairing it. One such tool is :hp1.DFSee:ehp1.
from FSYS Software, available at http&colon.//www.dfsee.com/.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12017.17 - Requested Size Is Too Small
:p.:hp2."Requested size is too small":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The requested size is too small.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Increase the requested size and try again. If that is not possible, you
may have to reorganize your disk layout.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12018.18 - Not Enough Free Space
:p.:hp2."Not enough free space":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.There is not enough free space on the selected disk to perform the requested
operation.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Either free up space on the disk by deleting one or more existing partitions
or volumes, or try the operation on a different disk drive.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12019.19 - Invalid Allocation Algorithm
:p.:hp2."Invalid allocation algorithm":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The program attempted to use an invalid allocation algorithm.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12020.20 - Duplicate Name
:p.:hp2."Duplicate name":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.A duplicate name was requested. All names must be unique.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Try using a different name.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12021.21 - Invalid Name
:p.:hp2."Invalid name":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.An invalid name was specified.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Try using a different name.
:p.This may also be an internal program error; try contacting support and
describing the failing operation.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12022.22 - Invalid Drive Letter Preference
:p.:hp2."Invalid drive letter preference":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The requested drive letter is not currently available.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Use a different drive letter.
:p.This may also be an internal program error; try contacting support and
describing the failing operation.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12023.23 - No Drives Found
:p.:hp2."No drives found":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.No partitionable disk drives (either hard disks or removable media) were
found in the system.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Shut down the computer and make sure that all hard disks are properly
connected and powered. Then try again.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12024.24 - Incorrect Volume Type
:p.:hp2."Incorrect volume type":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The selected volume type is not valid for the requested operation.
For instance, you might have tried to mark an advanced (LVM) volume as bootable
or startable.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Select a different volume before trying the operation again.
:p.Alternatively, you might be able to delete the volume and recreate it as a
different type (e.g. as a standard instead of an advanced volume). However,
doing this will cause any data currently on the volume to be lost.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12025.25 - Volume Is Too Small
:p.:hp2."Volume is too small":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The requested volume size is too small.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Increase the requested size and try again. If that is not possible, you
may have to reorganize your disk layout.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12026.26 - Boot Manager Is Already Installed
:p.:hp2."Boot Manager is already installed":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.This error appears if you try to install IBM Boot Manager when it is already
installed on the system.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.If you wish to reinstall IBM Boot Manager, first remove it and then install
it again.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12027.27 - Boot Manager Not Found
:p.:hp2."Boot Manager not found":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.IBM Boot Manager does not appear to be installed. You must install it before
you can perform any Boot Manager-specific operations.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12028.28 - Invalid Parameter
:p.:hp2."Invalid parameter":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The program specified an invalid parameter when calling the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12029.29 - Invalid Feature Set
:p.:hp2."Invalid feature set":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The program specified an invalid feature set when calling the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12030.30 - Too Many Partitions Selected
:p.:hp2."Too many partitions selected":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.An partition-related operation was attempted, but too many partitions were
selected at once.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Since the program does not currently allow the selection of multiple
partitions, this message most likely represents an internal program error.
Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12031.31 - LVM (Advanced) Volumes Cannot Be Made Bootable
:p.:hp2."LVM (advanced) volumes cannot be made bootable":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine does not allow advanced (LVM) volumes to be set as bootable
or startable.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.LVMPM should be designed so as to make this error message unreachable.
Therefore, this is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12032.32 - Partition Is Already In Use
:p.:hp2."Partition is already in use":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The selected partition could not be marked as a volume, because it is already
designated as 'in use' (either by an existing volume, or for some special
function such as IBM Boot Manager).
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Use a different partition.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12033.33 - Selected Partition Is Not Bootable
:p.:hp2."Selected partition is not bootable":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The selected partition could not be added to the IBM Boot Manager menu for
some reason.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Use a different partition, or try recreating the partition with different
parameters.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12034.34 - Volume Not Found
:p.:hp2."Volume not found":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.A non-existent volume was specified to the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12035.35 - Drive Not Found
:p.:hp2."Drive not found":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.A non-existent drive was specified to the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12036.36 - Partition Not Found
:p.:hp2."Partition not found":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.A non-existent partition was specified to the LVM engine.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12037.37 - Too Many Features Active
:p.:hp2."Too many features active":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.Too many features within the LVM engine are active.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12038.38 - Partition Is Too Small
:p.:hp2."Partition is too small":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The selected partition is too small for the requested operation.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Try using a larger partition.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12039.39 - The Maximum Number of Partitions Is Already In Use
:p.:hp2."The maximum number of partitions is already in use":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The maximum allowable number of partitions is already in use.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Use fewer partitions.
:p.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12040.40 - I/O Request Out of Range
:p.:hp2."I/O request out of range":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The read or write request was out of range.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.
:p.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12041.41 - The Specified Partition Is Not Startable
:p.:hp2."The specified partition is not startable":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The selected partition could not be made startable (directly bootable).
:p.Note that logical partitions may not be made startable; some type of boot
management software is necessary for this.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.If IBM Boot Manager is installed, you may mark the partition as bootable
via the :hp1.Partition:ehp1. menu. If Air-Boot is installed, configure the
partition as a bootable volume in the Air-Boot setup menu (accessible on system
boot). If you have some other third-party boot management software on your
system, you will need to use that software to make this partition bootable.
:p.Alternatively, use a different (primary) partition instead.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12042.42 - The Selected Volume Is Not Startable
:p.:hp2."The selected volume is not startable":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The selected volume could not be made startable (directly bootable).
:p.Note that volumes which are logical partitions may not be made bootable
directly; some type of boot management software is necessary for this.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.If IBM Boot Manager is installed, you may make the volume bootable via the
:hp1.Volume:ehp1. menu. If Air-Boot is installed, mark the volume as bootable
in the Air-Boot setup menu (accessible on system boot). If you have some other
third-party boot management software on your system, you will need to use that
software to make this volume's partition bootable.
:p.Alternatively, use a different volume (one which consists of a primary
partition) instead.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12043.43 - ExtendFS Operation Failed
:p.:hp2."ExtendFS operation failed":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The volume management engine tried to add a new partition to the selected
volume, but was unable to do so.
:p.Expanding a volume by adding a new partition to it is only possible if
certain conditions are met. Specifically, the existing volume must be an
advanced (LVM type) volume, and must be either unformatted, or formatted using
the Journaled File System (JFS).
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Ensure that the volume you are trying to expand is an advanced (LVM) volume.
Also ensure that the volume is either unformatted, or is formatted as JFS.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12044.44 - Reboot Required
:p.:hp2."Reboot required":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The system must be rebooted.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Shut down and reboot the computer before proceeding.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12045.45 - Cannot Open Log File
:p.:hp2."Cannot open log file":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine logging feature could not be activated because the target
log file could not be opened. The drive may be read-only, in which case
logging will not be possible.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This program does not make use of the logging feature; therefore, this
message probably represents an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12046.46 - Cannot Write To Log File
:p.:hp2."Cannot write to log file":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine logging feature tried to write to the log file, but failed
for some reason. The drive may be full, or the log file may have been corrupted
or deleted.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This program does not support the logging feature; therefore, this message
probably represents an internal program error. Please contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12047.47 - Rediscover Failed
:p.:hp2."Rediscover failed":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine tried to check for removable media devices, but the operation
failed for some reason.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Check the power and connections on your removable media devices, then try
the operation again. If it continues to fail, contact support.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12070.Unsupported Partitioning Error
:p.:hp2."An unsupported partitioning scheme is in use on ...":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.There is a disk drive connected to the system which uses a non-MBR-based
partitioning scheme such as GPT or LUKS. Such disk drives are not supported by
LVM. For safety, all LVM engine functions are disabled when such a disk is
detected.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.Remove the non-MBR disk drive from the system, or erase its contents completely,
before trying again. Normally, the disk drive in question should be indicated in the
error message. If not, try running a diagnostic tool (such as :hp1.DFSee:ehp1. from
FSys Software) to get more information.
:p.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12099.Undefined Error
:p.:hp2."Undefined error":ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.The LVM engine returned an unrecognized error code.
:p.
:p.:hp5.Suggested Action:ehp5.
:p.This is most likely an internal program error. Please contact support.
:p.


.* ****************************************************************************
:h1 x=left y=bottom width=100% height=100% id=others res=12100.Warning messages
:p.This following sections describe warning messages which LVMPM may display
under certain circumstances.

.* ----------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=12101.There are no bootable OS/2 volumes defined.
:p.:hp2.There are no bootable OS/2 volumes defined.  Do you really want to exit?:ehp2.
:p.
:p.:hp5.Explanation:ehp5.
:p.There is no OS/2 volume marked as either bootable or startable.

:p.If you have OS/2 installed on your system and this message appears, it
indicates that your OS/2 system is not presently functional.

:p.
:p.:hp5.Suggested Action:ehp5.
:p.If you are in the process of installing (or are about to install)
&os2., you must ensure that you have at least one volume that meets the
criteria listed above.

:p.If &os2. is already installed on your system and you wish to be able
to use it, then you should make sure that the &os2. boot volume is
properly bootable (as described above) and has a drive letter assigned.


