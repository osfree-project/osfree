#!/bin/sh -ex
#
#  Copyright 2011, Roger Brown
#
#  This file is part of Roger Brown's Toolkit.
#
#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
# 
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#
# $Id$
#

. ../version.sh

if test "$VERSION" = ""
then
	VERSION=0.0.0.1
fi

FLAG=

INTDIR=.

for d in $@
do
	if test "$FLAG" = ""
	then
		FLAG="$d"
	else
		case "$FLAG" in
		-d )
			OUTDIR_DIST="$d"
			;;
		-r )
			OUTDIR="$d"
			;;
		-t )
			INTDIR="$d"
			;;
		* )
			unknown option $d
			exit 1
		esac
		FLAG=
	fi
done

../pkg/mkfs.sh -d "$OUTDIR_DIST" -t "$INTDIR" -r "$OUTDIR"

find "$INTDIR" -print | xargs ls -ld

../../toolbox/dir2rpm.sh "$INTDIR" "$INTDIR/somtk.ir" "$OUTDIR_DIST" <<EOF
Summary: SOMTK Interface Repository
Name: somtk-ir
Version: $VERSION
Release: 1
Group: Applications/System
License: GPL
Prefix: /opt/somtk

%description
Interface Repository for SOMTK

EOF

../../toolbox/dir2rpm.sh "$INTDIR" "$INTDIR/somtk.rte" "$OUTDIR_DIST" <<EOF
Summary: SOMTK RTE
Name: somtk-rte
Version: $VERSION
Release: 1
Group: Applications/System
License: GPL
Prefix: /opt/somtk

%description
Runtime Environment for SOMTK

EOF

../../toolbox/dir2rpm.sh "$INTDIR" "$INTDIR/somtk.dsom" "$OUTDIR_DIST" <<EOF
Summary: SOMTK DSOM
Name: somtk-dsom
Version: $VERSION
Release: 1
Group: Applications/System
License: GPL
Prefix: /opt/somtk

%description
CORBA ORB for SOMTK

EOF

../../toolbox/dir2rpm.sh "$INTDIR" "$INTDIR/somtk.util" "$OUTDIR_DIST" <<EOF
Summary: SOMTK Utils
Name: somtk-util
Version: $VERSION
Release: 1
Group: Applications/System
License: GPL
Prefix: /opt/somtk

%description
Utilities for SOMTK

EOF

../../toolbox/dir2rpm.sh "$INTDIR" "$INTDIR/somtk.comp" "$OUTDIR_DIST" <<EOF
Summary: SOMTK Compiler
Name: somtk-comp
Version: $VERSION
Release: 1
Group: Applications/System
License: GPL
Prefix: /opt/somtk

%description
IDL Compiler for SOMTK

EOF

DPKGARCH=`../../toolbox/pkgtool.sh dpkg-arch "$OUTDIR/bin/somipc"`

../../toolbox/dir2deb.sh "$INTDIR" "$INTDIR/somtk.comp" "$OUTDIR_DIST" <<EOF
Package: somtk-comp
Version: $VERSION
Architecture: $DPKGARCH
Maintainer: somtoolkit@users.sf.net
Recommends:
Provides: somtk-comp
Section: misc
Priority: extra
Description: SOMTK compiler
 IDL compiler for SOMTK
 .
EOF

../../toolbox/dir2deb.sh "$INTDIR" "$INTDIR/somtk.rte" "$OUTDIR_DIST" <<EOF
Package: somtk-rte
Version: $VERSION
Architecture: $DPKGARCH
Maintainer: somtoolkit@users.sf.net
Provides: somtk-rte
Section: misc
Priority: extra
Description: SOMTK RTE
 Run Time Environment for SOMTK
 .
EOF

../../toolbox/dir2deb.sh "$INTDIR" "$INTDIR/somtk.ir" "$OUTDIR_DIST" <<EOF
Package: somtk-ir
Version: $VERSION
Architecture: $DPKGARCH
Maintainer: somtoolkit@users.sf.net
Depends: somtk-rte
Provides: somtk-ir
Section: misc
Priority: extra
Description: SOMTK Utilities
 Utility classes for SOMTK
 .
EOF

../../toolbox/dir2deb.sh "$INTDIR" "$INTDIR/somtk.util" "$OUTDIR_DIST" <<EOF
Package: somtk-util
Version: $VERSION
Architecture: $DPKGARCH
Maintainer: somtoolkit@users.sf.net
Depends: somtk-ir
Provides: somtk-util
Section: misc
Priority: extra
Description: SOMTK Utilities
 Utility classes for SOMTK
 .
EOF

../../toolbox/dir2deb.sh "$INTDIR" "$INTDIR/somtk.dsom" "$OUTDIR_DIST" <<EOF
Package: somtk-dsom
Version: $VERSION
Architecture: $DPKGARCH
Maintainer: somtoolkit@users.sf.net
Depends: somtk-util
Provides: somtk-dsom
Section: misc
Priority: extra
Description: SOMTK DSOM
 CORBA ORB for SOMTK
 .
EOF

