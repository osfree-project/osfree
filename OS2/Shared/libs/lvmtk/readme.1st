******************************************************************************
* THE IBM LOGICAL VOLUME MANAGER PROGRAMMER'S TOOLKIT                        *
******************************************************************************

This package contains the programmer's toolkit and API documentation for
the IBM OS/2 Logical Volume Manager.  The Logical Volume Manager (LVM) is a
disk partition/volume management program which replaces the old 'FDISK'
utility.

This version of LVM is included in the following operating systems:
  * OS/2 Warp Server for e-business (WSeb)
  * Convenience Package (1 and 2) for OS/2 Warp Version 4
  * Convenience Package (1 and 2)for OS/2 Warp Server for e-business
  * eComStation

LVM has a great many features which set it above FDISK, including:
  * 'Sticky' drive letters which may be arbitrarily assigned (i.e., not
    dependent on disk geometry)
  * Enables enhanced file system support (such as JFS)
  * Allows repartitioning without rebooting
  * Volume 'spanning' (combining several partitions into a single
    logical volume, even across multiple hard disks)
  * Dynamic volume expansion (when using the JFS file system)
  * Support for long volume names (e.g. on the Boot Manager menu)

All of LVM's functionality (the 'LVM Engine') is contained in a system DLL
(LVM.DLL) which may be called directly by application programmers.  

Header files for LVM.DLL are included in the IBM Device Driver Kit for OS/2
(in the directory DDK\base32\rel\os2c\include\base\os2\16bit), but no import
library is provided, and the only documentation provided is in the headers 
themselves.

This toolkit provides comprehensive API documentation in INF format, as well
as a prebuilt import library for linking with applications.  It also provides
the equivalent header files, licensed under the GPL instead of the somewhat
more restrictive DDK license.

NOTE: FixPak 2 (XR_E002) or higher seems to be a prerequisite for using
programs compiled with this toolkit under OS/2 Warp Server for e-business.
No FixPaks are required under either Convenience Package.


ARCHIVE CONTENTS
----------------

  README.1ST : This file.
  LICENSE    : A copy of the GNU General Public License.
  JMA_TK.TXT : The original readme file written by John Martin Alfreddson,
               who first compiled LVM.LIB.

  'doc' directory:

      This directory contains the API documentation, in IBM INF format.

      LVMREF.INF : Logical Volume Manager Programming Reference

  'include' directory:

      This directory contains the header files required for using the toolkit.

      lvm_intr.h : Main LVM header file (must be included in applications).
      lvm_cons.h : Additional header file (included by lvm_intr.h).
      lvm_type.h : Additional header file (included by lvm_intr.h).
      lvm_list.h : Additional header file (included by lvm_intr.h).
      lvm_gbls.h : Additional header file (included by lvm_intr.h).
      lvm_data.h : Separate header file for certain internal LVM structures.

  'lib' directory:

      This directory contains the import library required for using the toolkit.

      LVM.LIB : Import library for linking with compiled programs.

  'samples' directory:

      This directory contains two sample programs, with source code.

      lvmtest.cmd  : Batch file for compiling lvmtest.cpp.
      lvmtest.cpp  : Source code for LVMTEST, a simple sample program.
      Makefile     : 'Make' file for querylvm.cpp.
      querylvm.cpp : Source code for QUERYLVM, a more extensive sample program.
      readme.txt   : Notes on the sample programs.

  'src' directory:

      This directory contains the source code for the compiled toolkit and
      documentation files (a GPL requirement).

      ipfsrc.zip : IPF source code for the INF document under 'doc'.
      lvmtk.def  : Definition-file source code for LVM.LIB (under 'lib').


INSTALLATION
------------

Copy the contents of 'lib' to a directory on your compiler's library path
(the %LIB% path for the IBM C/C++ Compilers).

Copy the contents of 'include' to a directory on your compiler's include
path (the %INCLUDE% path for the IBM C/C++ Compilers).

Copy the contents of 'doc' to a directory on your BOOKSHELF path.

Create a program object for the LVM API Documentation which calls VIEW.EXE
(or your favourite alternate help-file viewer) on LVMREF.INF.

Place the samples and the other files in this package wherever you please.


USAGE
-----

Programs using the LVM API must include lvm_intr.h and link with LVM.LIB.
All LVM structures must be aligned on 1-byte boundaries (you can ensure this
when using the IBM C Compiler by compiling with the /Sp1 option).

NOTE: lvm_intr.h includes all other required header files EXCEPT for lvm_data.h
(which defines some of LVM's internal structures, and is not generally required
by most applications).


ACKNOWLEDGEMENTS
----------------

The toolkit and documentation were originally derived from the 'Reference OS/2
Code' located in the CVS repository of the IBM Enterprise Volume Management 
System (EVMS) open source project.  They are licensed under the GNU GPL.

Whether or not programs written using this toolkit must also be licensed under
GPL is uncertain.  Several people have told me that header files are considered
information and not code, thus programs written using them need not be GPL'd.
However, if you want to be legally safe, you might want to double-check with the
Free Software Foundation (http://www.gnu.org) before writing non-Free programs
using this toolkit.

LVM.LIB and the LVMTEST sample program were originally created and distributed
by John Martin Alfreddson.

The API documentation and the QUERYLVM sample program were created by Alex
Taylor.

The toolkit header files were written by IBM.  They have been modifed slightly
in order to fix some errors with certain compilers.  In addition, various
updates have been made to the header files in order to bring them into sync 
with the current implementation of LVM under OS/2.  The necessary information 
was derived through a combination of independent analysis and, where necessary,
comparison with the equivalent header files included in the IBM Device Driver 
Kit for OS/2.

NOTE: There is a distinction made between LVM Version 1 and LVM Version 2.  All
existing versions of LVM for OS/2 (including WSeB and eComStation) are LVM 
Version 1.  By contrast, the reference code published on the EVMS repository is
for LVM Version 2.  The header files and documentation included in this toolkit,
while derived from the EVMS code, have been modified to remove all features and 
definitions which are specific to Version 2.



CHANGES
-------

2011-04-25 (version 1.5)
  . Added lvm_data.h to header file distribution.
  . Modified header files to synchronize with DDK versions.
  . [INF] Removed references to features/functions not available under OS/2.
  . [INF] Massive reorganization of API reference documentation.  Format is now
    close to standard IBM Toolkit style, and includes more information about
    internal LVM structures.  Consolidated into a single INF file (LVMREF.INF).
  . [INF] Many other clarifications and corrections.

2006-03-07 (version 1.4)
  . Removed extremely ill-advised conditional #define of '_System' from 
    lvm_type.h, which should improve compatibility for non-IBM compilers.
  . [INF] Added notes about the behaviour of the Current_Drive_Letter and
    Drive_Letter_Preference fields in the Volume_Information_Record structure
    following a drive-letter change.
  . Added a note about the unreliability of the Boot_Manager_Is_Installed()
    function, and a suggested workaround.

2002-08-18 (version 1.3)
  . [INF] Added some editorial notes to various sections. 
  . [INF] Miscellaneous minor corrections. 

2002-06-04 (version 1.2)
  . First public release of integrated toolkit package.  (Toolkit files and
    API Documentation were previously released separately.)
  . [INF] Removed the separate section on 16-bit functions, and consolidated 
    all the function headers into one entry format. 
  . [INF] Moved structure field descriptions out of code-comments and into 
    footnotes. This should make reading easier, and is more consistent with 
    the standard OS/2 API reference format. 
  . [INF] Set window coordinates so resizing is friendlier. 
  . [INF] Reorganized the lvm_type.h documentation and added some notes. 

2002-02-18 (version 1.1)
  . [INF] Added notes on LVM Version 1 versus LVM Version 2. 
  . [INF] Reorganized the About and Notices sections. 
  . [INF] Some formatting changes. 

2001-09-15 (version 1.0)
  . [INF] Initial release. 

  
--
Alex Taylor
http://www.cs-club.org/~alex
April 25 2011

