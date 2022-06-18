#!/bin/sh
#
#  Copyright 2008, Roger Brown
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

CHMOD_EXECUTABLE="u+x"
CONFIG_GUESS=../../config/unix/config.guess
DEPENDS_SH=../../toolbox/depends.sh

for d in ../../toolbox/*.sh \
		 $DEPENDS_SH \
		 $CONFIG_GUESS
do
	if test ! -x $d
	then
		chmod $CHMOD_EXECUTABLE $d

		if test "$?" != "0"
		then
			exit 1
		fi
	fi
done

if test ! -x $CONFIG_GUESS
then
	echo could not find $CONFIG_GUESS 1>&2
	exit 1
fi

PLATFORM_PROTO=

if test "$PLATFORM_HOST" = ""
then
	PLATFORM_HOST=`$CONFIG_GUESS`
fi

if test "$PLATFORM_HOST" = ""
then
	echo could not determine host platform 1>&2
	exit 1
fi

if test "$PLATFORM" = ""
then
	PLATFORM=$PLATFORM_HOST
fi

if test "$BUILDTYPE" = ""
then
	BUILDTYPE=default
fi

if test "$BUILDTYPE_HOST" = ""
then
	BUILDTYPE_HOST="$BUILDTYPE"
fi

if test "$MAKE" = ""
then
	echo MAKE has not been set 1>&2
	exit 1
fi

SELECT_LAST=
SELECT_VOTE=0

for d in ../*/select.sh
do
	if test -f "$d"
	then
		if chmod $CHMOD_EXECUTABLE "$d"
		then
			if "$d" vote "$PLATFORM" >/dev/null
			then
				e=`"$d" vote "$PLATFORM"`
				if test "$e" != ""
				then
					if test "$e" -gt "$SELECT_VOTE"
					then
						SELECT_VOTE="$e"
						SELECT_LAST="$d"
					fi
				fi
			fi
		fi
	fi
done

if test "$SELECT_LAST" = ""
then
	echo failed to find configuration to support $PLATFORM 1>&2
	exit 1
fi

PLATFORM_PROTO=`"$SELECT_LAST" name "$PLATFORM"`
USE_OPTIONS=`"$SELECT_LAST" options "$PLATFORM"`

if test "$PLATFORM_PROTO" = ""
then
	echo failed to find configuration to support $PLATFORM 1>&2
	exit 1
fi

# echo proto $PLATFORM_PROTO
# echo options $USE_OPTIONS
# echo MAKEOPTS $MAKEOPTS

if test ! -d ../$PLATFORM
then
	if test ! -d ../$PLATFORM_PROTO
	then
		echo cant find prototype build for $PLATFORM_PROTO 1>&2
		exit 1
	fi
	mkdir ../$PLATFORM
	if test "$?" != "0"
	then
		echo mkdir `pwd`/../$PLATFORM failed 1>&2
		exit 1
	fi
	for d in ../$PLATFORM_PROTO/*
	do
		read N <"$d"
		if test "$?" = "0"
		then
			if test "$N" = "#!/bin/sh"
			then
				if test ! -x "$d"
				then
					chmod $CHMOD_EXECUTABLE "$d"
					if test "$?" != "0"
					then
						echo chmod $CHMOD_EXECUTABLE `pwd`/"$d" 1>&2
						exit 1
					fi
				fi
			fi
		fi
	done
fi

OUTDIR_DEFAULT_ROOT=../../products

if test "$OUTDIR" = ""
then
	OUTDIR="$OUTDIR_DEFAULT_ROOT/$PLATFORM/$BUILDTYPE"
else
	case "$OUTDIR" in
	/* )
		;;
	* )
		OUTDIR="../../$OUTDIR"
		;;
	esac
fi

# HOSTDIR is compilation for the local host
# and provides access to cross compilation tools

if test "$HOSTDIR" = ""
then
	HOSTDIR="$OUTDIR_DEFAULT_ROOT/$PLATFORM_HOST/$BUILDTYPE_HOST"
else
    case "$HOSTDIR" in
    /* )
        ;;
    * )
        HOSTDIR="../../$HOSTDIR"
        ;;
    esac
fi

if test "$HOSTDIR" != "$OUTDIR"
then
	if test ! -d "$HOSTDIR"
	then
		echo $0: HOSTDIR $HOSTDIR does not exist 1>&2
		exit 1
	fi

	if test ! -f "$HOSTDIR/bin/cpp"
	then
		echo $0: Cannot find $HOSTDIR/bin/cpp 1>&2
		exit 1
	fi

	"$HOSTDIR/bin/cpp" </dev/null >/dev/null

	if test "$?" != "0"
	then
		echo $0: Running  $HOSTDIR/bin/cpp failed 1>&2
		exit 1
	fi
fi

MAKEDEFS_EXPORTS="PLATFORM PLATFORM_HOST PLATFORM_PROTO BUILDTYPE OUTDIR HOSTDIR"

# echo USE_OPTIONS=$USE_OPTIONS

cd ../$PLATFORM

if test "$?" != "0"
then
	echo cd to ../$PLATFORM failed 1>&2
	exit 1
fi

if test ! -d $BUILDTYPE
then
	mkdir $BUILDTYPE

	if test "$?" != "0"
	then
		echo mkdir `pwd`/$BUILDTYPE failed 1>&2
		exit 1
	fi
fi

gen_makedefs_sh()
{
	echo "#" `date`
	echo "#" $PLATFORM

	for d in $MAKEDEFS_EXPORTS $MAKEDEFS_VARS
	do
		echo $d=\"`eval echo \\\$\$d`\" 
	done

	for d in $MAKEDEFS_EXPORTS
	do
		echo export $d
	done
}

gen_makedefs_mk()
{
	echo "#" `date`
	echo "#" $PLATFORM

	for d in $MAKEDEFS_DEFS
	do
		echo $d=`eval echo \\\$\$d`
	done
}

INTDIR=$BUILDTYPE

if test ! -f $INTDIR/makedefs.sh
then
	(
		for d in $USE_OPTIONS
		do
			if test -f ../$PLATFORM_PROTO/$d.sh
			then
				. ../$PLATFORM_PROTO/$d.sh
			else
				echo platform $PLATFORM_PROTO does not support option $d 1>&2
				exit 1
			fi
		done

		gen_makedefs_sh >$INTDIR/makedefs.sh
		gen_makedefs_mk >$INTDIR/makedefs.mk
	)

	RC=$?

	if test "$RC" != "0"
	then
		exit $RC
	fi
fi

. $INTDIR/makedefs.sh

if test "$MAKE" = ""
then
	echo MAKE environment variable not set 1>&2
	exit 1
fi

cd ../unix

find ../.. -type f -name "makedefs.cf" -print | $DEPENDS_SH >/dev/null

RC=$?

if test "$RC" = "0"
then
	PARTS=`find ../.. -type f -name "makedefs.cf" -print | $DEPENDS_SH`
	PARTS=`echo $PARTS`
	echo PARTS=$PARTS
	echo MAKE=$MAKE
	echo ARGS=$@
	pwd
	PARTS="$PARTS" $MAKE $@
	RC="$?"
	echo RC=$RC
fi

exit $RC
