#!/bin/sh -e
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

FLAG=

INTDIR=.

LIBPREFIX=lib
LIBEXT=.so
LIBDIRNAME=lib

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
		-l )
			LIBDIRNAME="$d"
			;;
		* )
			unknown option $d
			exit 1
		esac
		FLAG=
	fi
done

case "$INTDIR" in
/* )
	;;
* )
	INTDIR=`pwd`/$INTDIR
	;;
esac

case `uname` in
Aix | aix )
	PKGROOT=usr/lpp/somtk
	d;;
* )
	PKGROOT=opt/somtk
	;;
esac

first()
{
	echo $1
}

last()
{
	last_v=
	for last_d in $@
	do
		last_v="$last_d"
	done
	echo $last_v
}

copyBinFile()
{
	for y in $@
	do
		(
			cd `dirname "$y"`
			tar cf - `basename "$y"`
		) | (
			cd "$INTDIR/$PKGNAME/$PKGROOT/bin"
			tar xvf -
		)
	done
}

makeBinLink()
{
	(
		cd "$INTDIR/$PKGNAME/$PKGROOT/bin"
		ln -s "$1" "$2"
		ls -ld "$2"
	)
}

copyLibFile()
{
	for y in $@
	do
		(
			cd `dirname "$y"`
			tar cf - `basename "$y"`
		) | (
			cd "$INTDIR/$PKGNAME/$PKGROOT/$LIBDIRNAME"
			tar xvf -
		)
	done
}

copyIncFile()
{
	for y in $@
	do
		(
			cd `dirname "$y"`
			tar cf - `basename "$y"`
		) | (
			cd "$INTDIR/$PKGNAME/$PKGROOT/include"
			tar xvf -
		)
	done
}

copyLib()
{
	for x in $@
	do
		find "$OUTDIR/lib" -name "$LIBPREFIX"$x"$LIBEXT"\* | while read N
		do
			copyLibFile "$N"
		done
		find "$OUTDIR/lib" -name $x.dll | while read N
		do
			copyLibFile "$N"
		done
	done
}

copyBin()
{
	for x in $@
	do
		find "$OUTDIR/bin" -name "$x" | while read N
		do
			copyBinFile "$N"
		done
	done
}

copyMan()
{
	for x in $@
	do
		for s in 1 8
		do
			find "$OUTDIR/man" -name "$x.$s*" | while read N
			do
				(
					cd "$OUTDIR/man"
					tar cf - `basename "$N"`
				) | (
					cd "$INTDIR/$PKGNAME/$PKGROOT/man/man$s"
					tar xvf -
				)
			done
		done
	done
}

copyTestBin()
{
	for x in $@
	do
		find "$OUTDIR/tests" -name "$x" | while read N
		do
			copyBinFile "$N"
		done
	done
}

copyIdl()
{
	for x in $@
	do
		cp ../../somidl/$x.idl "$INTDIR/$PKGNAME/$PKGROOT/include/$x.idl"
	done
}

pruneDirs()
{
	find "$1" -type d | while read PD1
	do
		find "$1" -type d | while read PD2
		do
			find "$1" -type d | while read PD3
			do
				if rmdir "$PD3" 2>/dev/null
				then
					echo "$PD3" pruned
				fi
			done
		done 
	done
}

for PKGNAME in somtk.comp somtk.dsom somtk.ir somtk.rte somtk.somp somtk.somr somtk.somuc somtk.somx somtk.util
do
	find "$INTDIR/$PKGNAME" | while read N
	do
		if test -h "$N"
		then
			ls -ld "$N"
		else
			chmod +w "$N"
		fi
	done
	rm -rf "$INTDIR/$PKGNAME/$PKGROOT"
	mkdir -p "$INTDIR/$PKGNAME/$PKGROOT"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/bin"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/etc"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/$LIBDIRNAME"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/man"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/man/man1"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/man/man8"
	mkdir "$INTDIR/$PKGNAME/$PKGROOT/include"

	case $PKGNAME in
	somtk.dsom )
		mkdir "$INTDIR/$PKGNAME/$PKGROOT/etc/dsom"
		copyLib somd somdcomm somos somnmf somestrm 
		copyIdl somdobj somdcprx orb somoa boa nvlist om cntxt impldef implrep \
				principl request servmgr somdom somdtype stexcep unotypes somproxy \
				omgestio xmscssae somdserv \
				naming lname xnaming xnamingf lnamec biter \
				somos somddsrv somestio
		copyBin somdd regimpl
		cp "$OUTDIR/etc/somenv.ini" "$INTDIR/$PKGNAME/$PKGROOT/etc/somenv.ini"
		cp "$OUTDIR/bin/somdsvr" "$INTDIR/$PKGNAME/$PKGROOT/bin/somdsvr"
		cp "$OUTDIR/bin/somdchk" "$INTDIR/$PKGNAME/$PKGROOT/bin/somdchk"
		cp "$OUTDIR/bin/somossvr" "$INTDIR/$PKGNAME/$PKGROOT/bin/somossvr"
		copyIncFile ../../somtk/include/somd*.h ../../somtk/include/somd*.xh
		copyIncFile ../../somtk/include/somnm*.h ../../somtk/include/somnm*.xh
		copyIncFile ../../somtk/include/somos*.h ../../somtk/include/somos*.xh
		copyMan somdd regimpl
		;;
	somtk.comp )
		case `uname -s` in
		IRIX* )
			copyBin somcpp
			;;
		SunOS )
#			makeBinLink /usr/lib/cpp somcpp
			copyBin somcpp
			;;
		* )
			;;
		esac
		copyMan sc somipc
		copyBin sc somipc pdl
		copyIncFile ../../somtk/include/somtc*.h ../../somtk/include/somtc*.xh
		cp ../../somtk/unix/somcorba.sh "$INTDIR/$PKGNAME/$PKGROOT/bin/somcorba"
		(
			cd "$INTDIR/$PKGNAME/$PKGROOT/bin"
			chmod +x somcorba
			ln -s somcorba somstars
			ln -s somcorba somxh
		)
		;;
	somtk.rte )
		copyLib som
		copyIdl somcls somobj somcm
		copyIncFile ../../somkpub/include/*.h ../../somkpub/include/*.xh
		;;
	somtk.ir )
		copyMan irdump
		copyLib somtc somir somref
		copyIdl repostry containd containr intfacdf operatdf moduledf paramdef somref typedef excptdef constdef attribdf
		copyBin irdump
		cp "$OUTDIR/etc/somnew.ir" "$INTDIR/$PKGNAME/$PKGROOT/etc/somnew.ir"
		copyIncFile ../../somtk/include/somir*.h ../../somtk/include/somir*.xh
		;;
	somtk.util )
		copyLib soms somst somu somu2 somcdr somany somabs1 somcorba somem
		copyIdl \
			somssock tcpsock \
			workprev timerev somsid eman sinkev clientev emregdat event \
			snglicls somida	\
			omgidobj
		copyIncFile ../../somtk/include/eventmsk.h ../../somtk/include/emtypes.h
		copyIncFile ../../somtk/include/eventmsk.xh ../../somtk/include/emtypes.xh
		;;
	* )
		;;
	esac

	pruneDirs "$INTDIR/$PKGNAME"

	if test "$STRIP" != ""
	then
		find "$INTDIR/$PKGNAME/$PKGROOT" -type f -name "lib*.so*" | while read SN
		do
			$STRIP "$SN"
		done
		find "$INTDIR/$PKGNAME/$PKGROOT" -type f -name "*.dll" | while read SN
		do
			$STRIP "$SN"
		done
		for APP in irdump pdl regimpl somdchk somdd somdsvr somipc somossvr
		do
			if test -x "$INTDIR/$PKGNAME/$PKGROOT/bin/$APP"
			then
				$STRIP "$INTDIR/$PKGNAME/$PKGROOT/bin/$APP"
			fi
		done
	fi

	if test -d "$INTDIR/$PKGNAME"
	then
		find "$INTDIR/$PKGNAME" | while read N
		do
			if test -h "$N"
			then
				ls -ld "$N"
			else
				chmod -w "$N"
			fi
		done
	fi
done

