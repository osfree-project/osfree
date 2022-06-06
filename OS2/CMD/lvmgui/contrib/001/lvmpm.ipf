:userdoc.

.nameit symbol=os2 text='OS/2'


.*****************************************************************************
:h1 x=left y=bottom width=100% height=100% res=001.Logical Volume Manager - Introduction
:p.The Logical Volume Manager allows you to create, delete, and modify
volumes and partitions on your computer's disk drives. This program, LVMPM,
is a graphical user interface for the Logical Volume Manager functions.

:p.For an explanation of the terms used in this program, see
:link reftype=hd res=002.Terminology:elink..

:p.For a guide to the graphical user interface, go to the section describing
the :link reftype=hd res=100.LVMPM user interface:elink..


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% id=terms res=002.Terminology
:p.Below is a summary of some of the terms used by the Logical Volume
Manager and its graphical user interface.

.* NOTE: markup conventions for this glossary:
.* - Bold (hp1) is used for term headings, and also for references to other terms
.*   on this page within term descriptions. Only the first use of a term within
.*   a single entry (including the term heading) should be bold.
.* - Italics are used to emphasize technical concepts which do not have their own
.*   entries in this glossary.
.* - Single quotes are used for minor nomenclature such as alternate terms.

:p.:dl break=all.

:dt.:hp2.Advanced volume:ehp2.
:dd.Advanced volumes, also called &osq.LVM volumes&csq. (the original IBM
term), are not compatible with most other operating systems. Their most
significant feature is the ability to link multiple partitions together into a
single volume, with a single name, drive letter, and file system.

:dt.:hp2.Disk:ehp2.
:dd.While this term has a number of uses in general computing, within LVM it
refers to a :hp1.partitioned direct-access storage device:ehp1.. LVM
distinguishes between two types of disks&colon. :hp2.fixed disks:ehp2. and
:hp2.partitioned removable media:ehp2. (PRM).

:dt.:hp2.Drive letter:ehp2.
:dd.An alphabetical letter used to represent a volume.  A volume requires a drive
letter in order to be usable under &os2..

:dt.:hp2.File system:ehp2.
:dd.A file system defines the data format in which files and folders are stored
on a volume. A volume must be formatted with a file system before it can be used
to store data. Selecting file systems and formatting volumes are performed using
&os2. system tools, and are tasks that lie outside the scope of LVM.

:dt.:hp2.Fixed disk:ehp2.
:dd.A fixed disk is a partitionable disk (generally a hard disk or solid state
drive) which is permanently attached to the computer via a local bus interface.

:dt.:hp2.Free space:ehp2.
:dd.Any area of disk space which is not allocated to a partition.

:dt.:hp2.KB:ehp2. or :hp2.KiB:ehp2.
.br
:hp2.MB:ehp2. or :hp2.MiB:ehp2.
.br
:hp2.GB:ehp2. or :hp2.GiB:ehp2.
.br
:hp2.TB:ehp2. or :hp2.TiB:ehp2.
:dd.Units of storage size. For consistency, both forms of notation mean the
same thing throughout LVM&colon. specifically, standard binary (base-two) units
of storage. Thus&colon. one KB (or one KiB) is 1024 bytes;  one MB (or one MiB)
is 1024 KB, and one GB (or one GiB) is 1024 MB.
[:link reftype=fn refid=iec.Note:elink.]

:dt.:hp2.Large floppy:ehp2.
:dd.A removable storage device which presents itself to &os2. as a single,
partitionless block of storage. Some USB devices present themselves this way.
Large floppies are reported by LVM as volumes on non-LVM devices.
:p.Note that &os2. only supports large floppies of up to 2 GB in size. Such
devices prepared for use under other operating systems may be larger than this,
in which case &os2. cannot use them unless they are modified to use normal
(MBR style) partitioning.

:dt.:hp2.Logical Volume Manager:ehp2.
:dd.The Logical Volume Manager (or LVM) is the disk storage management subsystem
of &os2., for which this program is a user interface.

:dt.:hp2.Partition:ehp2.
:dd.A contiguous region of a disk allocated for storage. At present, all
partitions usable by LVM must be defined according to the standard MBR
partition table scheme.
:p.There are two types of partition, &osq.primary&csq. and &osq.logical&csq..
As far as &os2. is concerned, there is little practical difference between
the two types.  However, if you have other operating systems installed on the
same computer, you may be required to keep them on primary partitions.
:p.A single disk drive may be allocated into a maximum of four primary
partitions, or three primary partitions and any number of consecutive logical
partitions.

:dt.:hp2.Partitioned removable media:ehp2.
:dd.Partitioned removable media, or PRM, is a general term for removable storage
devices which are capable of holding partitions and volumes. Such devices include
USB flash drives or external hard disks, memory cards, and IOMega ZIP(R) drives,
among others.
:p.PRM devices which are defined by &os2. but do not have the actual storage media
attached are reported by LVM as empty disks with no partitions.

:dt.:hp2.Standard volume:ehp2.
:dd.A standard volume is the basic volume type in LVM. It consists of a single
partition which has been designated as a volume by LVM. Standard volumes are
compatible with other operating systems, which simply treat them as ordinary
partitions (according to their own disk management logic).
:p.Standard volumes are sometimes referred to as &osq.compatibility
volumes&csq., which is the IBM terminology for them.

:dt.:hp2.Volume:ehp2.
:dd.A volume is essentially a virtual drive&colon. named storage which may
have a drive letter and a file system.
:p.Generally speaking, volumes are simply partitions which have been designated
as volumes by LVM, and are thus capable of being used as storage under &os2..
However, volumes have several special features which distinguish them from
partitions&colon.
:ul.
:li.Not all partitions are necessarily volumes. For example, partitions that
are used by other operating systems on the same computer might not be.  Such
partitions cannot be used by &os2. unless they are converted (or added) to
a volume.
:li.Multiple partitions can be combined into a single volume. This is only
possible with the Advanced (or &osq.LVM&csq.) type of volume.
:li.Any device with a drive letter assigned to it under &os2. is seen as a
volume, even if it does not correspond to any fixed-disk partition(s).  For
example, optical (CD/DVD/BD) drives, network-attached LAN drives, or other
virtual drives are seen by LVM as :hp1.non-LVM devices:ehp1.. They can be
accessed from the operating system like normal volumes, but LVM cannot
manage or modify them.
:eul.
:p.There are two different types of volume available in LVM&colon.
:hp2.standard volumes:ehp2. and :hp2.advanced volumes:ehp2.. See the
corresponding entries for details.

:edl.

.* ------------------------------------------------------------------------
:fn id=iec.
:p.&osq.KB&csq., &osq.MB&csq., &osq.GB&csq., and &osq.TB&csq. are the
traditional notation; &osq.KiB&csq., &osq.MiB&csq., &osq.GiB&csq., and
&osq.TiB&csq. are a newer notation proposed by IEC. The command-line LVM
utility, and most existing &os2. software, uses the traditional notation,
and hence that is the default in the graphical user interface as well.
:p.If you prefer, you can change to using the IEC notation throughout the GUI
instead, via the :link reftype=hd res=1100.Preferences:elink. dialog.
:efn.


.*****************************************************************************
:h1 x=left y=bottom width=100% height=100% res=100.LVMPM User Interface
:p.The main window consists of top and bottom areas, separated by a moveable
split-bar control. The top area is a :hp1.logical view:ehp1. of system storage,
showing information about volumes. The bottom area represents a :hp1.physical
view:ehp1. of the disks attached to the computer.

:p.:hp7.Logical view (top):ehp7.

:p.This area shows information about the volumes that exist on the system.
By default, all volumes known to LVM are shown &ndash. this includes both
volumes controlled by LVM, and those which correspond to non-LVM devices such
as optical drives or network shares. (You can choose to hide volumes on
non-LVM devices by setting the corresponding option in
:link reftype=hd res=1100.Preferences:elink..)

:p.The right-hand side of this area is a status panel showing details about
the currently-selected volume.

:p.You can select operations on a volume by highlighting the desired volume
and either accessing the :hp2.Volumes:ehp2. menu from the menu-bar, or by
right-clicking on the volume to bring up its context menu. The menu will
show all available volume operations; those which are not supported for the
current volume will be disabled automatically.

:p.Whenever you select a volume in the list, the partition (or partitions)
belonging to that volume will appear cross-hatched in the physical view at
the bottom part of the window.  (Note, however, that this will not actually
change the currently-selected partition or disk in the physical view.)

:p.:hp7.Physical view (bottom):ehp7.

:p.This area displays the system's disk drives and the partitions on them. By
default, this shows all disks, including allocated but empty removable media
drives. (You can configure the GUI to hide unavailable removable media drives
by setting the corresponding option in :link reftype=hd res=1100.Preferences:elink..)

:p.At the very bottom of the window is a status bar that shows information
about the currently-selected partition. (This status bar is only enabled when
the disk list has input focus.)

:p.You can select operations on a partition by right-clicking on it to bring up
the partition context menu. This menu shows all available partition operations;
those which are not supported for the current partition will be disabled
automatically.

.* .br
.* :p.:hp1. ...more to come:ehp1.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=200.Disk Name
:p.The Disk Name dialog allows you to set a disk's LVM-defined name.

:p.This dialog also displays the disk's hardware-reported serial number,
which cannot be changed.

:p.The disk name has no function other than to provide a human-readable
description for the disk. It is not used outside LVM applications.

:p.A disk name has a maximum length of 20, and may contain any characters
which are valid for the current codepage.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=300.Boot Manager Options
:p.This dialog allows you to configure the behaviour and appearance of IBM
Boot Manager, if installed. (It does not apply to Air-Boot.)

:dl break=all.
:dt.:hp2.Boot default:ehp2.
:dd.This controls which volume or partition Boot Manager will select for
booting by default when the computer starts up.
:dt.:hp2.Boot automatically after ... seconds:ehp2.
:dd.Use these controls to cause Boot Manager to automatically boot from
the default volume or partition (above) following a certain number of
seconds after the menu appears.
:dt.:hp2.Simple display
.br
Verbose display:ehp2.
:dd.Use these radio-buttons to control whether the Boot Manager menu uses
simple or verbose display mode.
:edl.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=400.Volume Creation Dialog
:p.This dialog determines the parameters of the volume to be created.

:p.Select either the standard (compatibility) or advanced (LVM) volume type.
(See :link reftype=hd res=002.Terminology:elink. for information on the two
types.) Standard volumes are generally recommended unless you want to be able
to add more than one partition to the volume (either now or in the future).

:p.When creating a standard volume, if Air-Boot is not installed you will
also be offered the option of making the volume bootable or startable. This
option has slightly different effects depending on whether IBM Boot Manager
is installed&colon.
:ul compact.
:li.If IBM Boot Manager is installed, the volume will be added to the Boot
Manager menu automatically.
:li.If IBM Boot Manager is not installed, the volume will be set startable
if (and only if) it consists of a primary partition.
:eul.
:p.Note that if Air-Boot is installed, this option does not apply, as
Air-Boot controls the definition of bootable volumes/partitions through
its own menu (available at boot time).


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=500.Partition Selection Dialog
:p.On this dialog, select the partition(s) used to create the new volume.
A list of disks and the partitions on them is displayed.

:p.Only partitions or free space areas which are not currently part of any
volume may be selected. All other partitions will be displayed in grey and
will be unselectable.

:p.Select an available partition or area of free space using the mouse or
keyboard.
:ul.
:li.If you selected to create a standard (compatibility) volume, you
may only select one partition; use the &osq.Create&csq. button to accept
your selection and create the volume.
:li.If you selected to create an advanced (LVM) volume, then use the
&osq.Add&csq. button to add the selected partition or free space to the
volume. Select &osq.Create&csq. when you are done selecting partitions.
:eul.

:p.If you selected an area of free space, you will need to create a
partition in it before the volume gets created; in this case, the
:link reftype=hd res=800.Create New Partition:elink. dialog will appear
automatically.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=600.Volume Letter
:p.This dialog allows you to change the selected volume's drive letter.

:p.Select the new drive letter from the drop-down list. Alternatively, you can
choose a drive letter of &osq.None&csq., meaning that no drive letter will be
assigned to the volume; this has the effect of hiding the volume from &os2.. You
also have the option of choosing &osq.Automatic&csq., which will cause &os2. to
automatically choose a drive letter for the volume whenever the system boots
(or, in the case of partitioned removable media, whenever the device is attached).

:p.List items marked with &osq.&asterisk.&csq. indicate drive letters which are
presently in use by volumes with automatic letter assignments. You can still
choose to assign one of these drive letters to a volume, but doing so will
require a system reboot so that &os2. can update the automatic drive letter
assignments.

:p.Letters which are already explicitly assigned by LVM to other volumes will
not appear in the list.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=700.Volume/Partition Name
:p.This dialog allows you to change the name of the current volume or partition.

:p.If you have IBM Boot Manager installed, and the volume or partition is set
bootable, the name will be used to identify it on the Boot Manager menu.
(Air-Boot may also use this name on its boot menu, although it may be truncated
as Air-Boot has limitations on the length of the menu text.)
Otherwise, the name is not used outside LVM-aware applications.

:p.A volume or partition name has a maximum length of 20, and may contain any
characters which are valid for the current codepage.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=800.Create New Partition
:p.This dialog allows you to create a new partition in an area of free space.

:p.Choose a name for the partition. This may be up to 20 characters (bytes)
in length.

:p.Next, specify the partition size (in binary megabytes). The maximum size
is the size of the selected area of free space. If you specify a size smaller
than this, the space is allocated from the beginning of the free space by
default; you can choose to allocate it from the end of the free space instead
by checking the &osq.Allocate from end&csq. checkbox.

:p.Depending on the current disk layout, you may have the choice of creating
a primary or logical partition. If a primary partition is possible in the
current disk layout, the &osq.Primary partition&csq. checkbox will be enabled.
If this checkbox is not checked (or is not enabled), a logical partition will
be created.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=1000.Add Partition to Volume
:p.This dialog appears when selecting :hp2.Append to volume:ehp2. from a
partition's context menu. It prompts you to select the existing volume to
which the indicated partition will be added.

:note.Appending is only supported for advanced (LVM) volumes, and only for
unformatted or JFS-formatted volumes. If you want to create a new standard
(compatibility) volume from a partition, use the :hp2.Convert to volume:ehp2.
menu command instead.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=1100.Preferences
:p.The :hp2.Preferences:ehp2. dialog allows you to customize certain aspects
of the Logical Volume Manager's appearance and behaviour.

:dl break=all.
:dt.:hp2.Use IBM terminology for volume type:ehp2.
:dd.This option will cause standard volumes to be identified as
&osq.compatibility&csq. volumes, and advanced volumes as &osq.LVM&csq.
volumes.  This reflects the terminology originally introduced by IBM in OS/2
Warp Server for e-business.  This is purely a cosmetic preference for users who
are more comfortable with the older IBM terms.

:dt.:hp2.Use IEC terminology for binary sizes:ehp2.
:dd.In recent years, new abbreviations for binary byte units (KiB, MiB, GiB
instead of KB, MB, GB, etc.) have started to enter popular use.  The IEC has
issued a standard for the use of these new abbreviations to describe storage
sizes.  This option causes the new abbreviations to be used throughout the
graphical user interface.

:dt.:hp2.Show partitions with uniform width:ehp2.
:dd.By default, partitions in the physical view panel are scaled according
to their relative size on the disk. Enabling this option will cause all
partitions on a disk to be drawn with a uniform width instead.

:dt.:hp2.Automatically select corresponding objects:ehp2.
:dd.If this option is enabled, selecting a volume in the volume list will
cause the (first) partition corresponding to that volume to be selected
automatically in the physical view panel; and selecting a partition in
the physical view will cause the corresponding volume, if any, to be
selected automatically in the volume list.

:dt.:hp2.Warn when no bootable volumes exist:ehp2.
:dd.If this option is enabled, LVMPM will pop up a warning message on program
exit if no bootable volumes exist.

:dt.:hp2.Hide empty removable media drives:ehp2.
:dd.By default, all disks known to LVM are shown in the physical view panel.
If you have several removable media devices defined (for example, using the
USB mass storage or PCMCIA auto-drive drivers), they will appear as empty
(partitionless) disks when no media is inserted. Enabling this setting will
prevent empty disks such as these from being shown.

:dt.:hp2.Hide volumes not managed by LVM:ehp2.
:dd.By default, all volumes recognized by LVM will be shown in the volumes
list. This includes volumes which are not managed by LVM, such as network
devices or optical drives. Enabling this setting will prevent such volumes
from being shown.

:dt.:hp2.Visual style:ehp2.
:dd.This allows you to change the visual style used by the graphical user
interface. :hp1.Presentation Manager:ehp1. is the default style, based on the
standard OS/2 Presentation Manager appearance. :hp1.Document:ehp1. is an
alternate style with a more two-dimensional appearance and a lighter main
window background.

:dt.:hp2.Enable IBM Boot Manager installation:ehp2.
:dd.Checking this option enables support for installing the IBM Boot Manager
product. Boot Manager does not always work well with large modern disks,
hence it is not recommended by default; however, it may be desirable in
some environments, especially with older hardware.

:dt.:hp2.Enable Air-Boot installation:ehp2.
:dd.Checking this option enables support for installing the Air-Boot boot
menu on your system. Air-Boot is a modern boot management program similar
to IBM Boot Manager, but with some key differences&colon.
:ul compact.
:li.It does not require the use of a primary partition.
:li.It supports very large hard disks.
:li.Boot menu configuration is done through the boot menu itself (at
system boot time), and not through the LVM user interface.
:eul.
:edl.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=1200.Fonts
:p.The :hp2.Fonts:ehp2. dialog allows you to configure the screen fonts used
in various parts of the user interface.

:dl break=all.
:dt.:hp2.Volume list:ehp2.
:dd.Controls the font used when displaying the list of volumes.
:dt.:hp2.Volume details:ehp2.
:dd.Controls the font used in the volume details panel on the upper right-hand
side of the main window.
:dt.:hp2.Disk list:ehp2.
:dd.Controls the font used in the physical view of disks and partitions.
:dt.:hp2.Status bar:ehp2.
:dd.Controls the font used in the main window status bar. Note that the status
bar has a static vertical size (based on the system default font), so selecting
too large a font here may result in clipped text. Generally, you should not
select a font which is larger than the system default font.
:dt.:hp2.Secondary dialogs:ehp2.
:dd.Controls the font used in secondary dialog windows. Note that secondary
dialogs have static sizes based on the system default font; selecting too large
a font here may result in clipped text. Generally, you should not select a font
which is larger than the system default font.
:p.You can reset the font for secondary dialogs back to the system default by
selecting the &osq.Clear&csq. button.
:edl.
:note.The &osq.system default&csq. font is an &os2. system setting, and is used
in conjunction with the current graphics resolution to determine how dialog
coordinates are scaled. On non-Asian versions of &os2. it defaults to either
10 or 12 point System Proportional (depending on the screen size), and is not
normally user-configurable &endash. although advanced users can change it by
editing the &os2. desktop profile directly.


.*****************************************************************************

.im 001\errors.ipf


.*****************************************************************************
:h1 x=left y=bottom width=100% height=100% res=9900.Notices
:p.:hp2.Logical Volume Manager for Presentation Manager:ehp2.
.br
(C) 2011-2019 Alexander Taylor

:lm margin=4.
:p.This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

:p.This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

:p.You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
:lm margin=1.

:p.See :link reftype=hd refid=license.the following section:elink. for the
full text of the GNU General Public License.

:p.LVMPM source code repository&colon. https&colon.//github.com/altsan/os2-lvmpm


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% id=license res=9910.License
.im 001\license.ipf

:euserdoc.


