Logical Volume Manager for Presentation Manager (LVMPM)
=======================================================

LVMPM is a native PM application designed to replace the Java-based IBM LVMGUI
interface.  It has been designed as part of the objectives defined by the LVM 
redesign project: http://www.altsan.org/os2/toolkits/lvm/redesign/


Building
--------

In addition to the standard OS/2 Toolkit, building LVMPM requires the LVM 
Toolkit (header and library files) from http://www.altsan.org/os2/toolkits/lvm/

It also uses the XWorkplace Helpers library; in this case the necessary header
and library files are included in the repository here.  

The included Makefile is for the IBM C Compiler or VisualAge C++.  Using the 
prebuilt `helpers.lib` specifically requires ICC version 3.65; to build using
3.08, replace `helpers.lib` with `helpers.308` or else rebuild the `xwphelpers`
target using 3.08.  Note that building `helpers.lib` also requires the ACPI
Toolkit for OS/2, in addition to the XWPHelpers sources from NetLabs.

Building with GCC or OpenWatcom is not tested or supported. 

To build LVMPM: 

1. Copy `local.inc.sample` to `local.inc` and edit the paths as appropriate for
   your system.  Note that only the first item is required to simply build
   `lvmpm.exe`; the other items are only needed for rebuilding `helpers.lib`.

2. Make sure your environment is configured for the IBM C Compiler, and the
   OS/2 Toolkit (4.5) is also installed and configured.

3. Run `nmake32` to build a release binary, or `nmake32 DEBUG=1` for a debug
   build.  The English NLS files (in `001\`) are built by default.


License
-------

Logical Volume Manager for PM (LVMPM)

Copyright (C) 2011, 2019 Alexander Taylor

Program source: https://github.com/altsan/os2-lvmpm

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
