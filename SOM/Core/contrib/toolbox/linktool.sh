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

LINKTOOL_SH=`basename $0`
INCLUDE_FILES=
SOURCE_FILES=
OBJECTS=
LIBDIRS=
LIBLIST=
DLLNAME=
DEFFILE=
NAME=
SONAME=
SONAME_FILE=
VERSION=
VERSION_STYLE=
LASTFLAG=
EXPORTFILE=
EXPORTS=
ENTRY=
OUTPUT_TYPE=
LINKER_FLAGS=
OUTPUT_FILE=
FINAL_BINARY=
BUNDLE_LOADER=
LANGFLAGS=
LAUNDRY_LIST=
DLLONLY=true
IMPORT_LIBS=
IMPORT_DIRS=
LINKTOOL_SYMLINK=do_link
LINKTOOL_STUBLIB=do_link
LINKTOOL_GETVERSIONS=getversions
DLL_EXPORTS="SOMInitModule DllGetClassObject DllCanUnloadNow DllRegisterServer DllUnregisterServer"

first()
{
	echo $1
}

last()
{
	LAST=
	for LASTN in $@
	do
		LAST=$LASTN
	done
	echo $LAST
}

is_member()
{
	is_member_1=$1
	shift
	for is_member_i in $@
	do
		if test "$is_member_i" = "$is_member_1"
		then
			return 0
		fi
	done
	return 1
}

not_member()
{
	not_member_1=$1
	shift
	for not_member_i in $@
	do
		if test "$not_member_1" = "$not_member_i"
		then
			return 1
		fi
	done
	return 0
}

for d in $@
do
	if test "$LASTFLAG" = ""
	then
		case $d in 
		-n | -e | -I | -s | -v | -o | --export | -impdir | -implib )
			LASTFLAG=$d
			;;
		-cxx )
			LANGFLAGS="$LANGFLAGS $d"
			;;
		-shared | -bundle )
			if test "$OUTPUT_TYPE" != ""
			then
				echo output type already set as $OUTPUT_TYPE 1>&2
				exit 1
			fi
			OUTPUT_TYPE=$d
			;;
		-l* )
			LIBLIST="$LIBLIST $d"
			;;
		-L* )
			if not_member $d $LIBDIRS
			then
				LIBDIRS="$LIBDIRS $d"
			fi
			;;
		-Wl,* )
			LINKER_FLAGS="$LINKER_FLAGS $d"
			;;
		*.o )
			OBJECTS="$OBJECTS $d"
			;;
		* )
			echo internal error with switch = $d 1>&2
			exit 1
			;;
		esac		
	else
		case $LASTFLAG in
		-n )
			if test "$NAME" != ""
			then
				echo name already set as $NAME  1>&2
				exit 1
			fi
			NAME=$d
			;;
		-v )
			if test "$VERSION" != ""
			then
				echo version already set as $VERSION  1>&2
				exit 1
			fi
			VERSION=$d
			;;
		-impdir )
			IMPORT_DIRS="$IMPORT_DIRS $d"
			;;
		-implib )
			IMPORT_LIBS="$IMPORT_LIBS $d"
			;;
		--export )
			if test "$EXPORTFILE" != ""
			then
				echo exports already set as $EXPORTFILE  1>&2
				exit 1
			fi
			EXPORTFILE=$d
			DEFFILE=$INTDIR/`basename $d`.def
			;;
		-e )
			if test "$ENTRY" != ""
			then
				echo entry already set as $ENTRY  1>&2
				exit 1
			fi
			ENTRY=$d
			;;
		-o )
			if test "$OUTPUT_FILE" != ""
			then
				echo output file already set as $OUTPUT_FILE  1>&2
				exit 1
			fi
			OUTPUT_FILE=$d
			;;
		-I )
			INCLUDE_FILES="$INCLUDE_FILES $d"
			;;
		-s )
			SOURCE_FILES="$SOURCE_FILES $d"
			;;
		* )
			echo internal error with LASTFLAG = $LASTFLAG 1>&2
			exit 1
			;;
		esac
		LASTFLAG=
	fi
done

ECHO_MODE=

echo_n()
{
	case $ECHO_MODE in
	n )
		echo -n "$@"
		;;
	e )
		echo -e "$@\c"
		;;
	* )
		echo "$@\c"
		;;
	esac
}

defsonly()
{
	for def in $@
	do
		case "$def" in
		-D* )
			echo "$def"
			;;
		* )
			;;
		esac
	done
}

readexports()
{
	if test "$EXPORTFILE" != ""
	then
		if test -f $EXPORTFILE
		then
			if test -x $HOSTDIR/bin/cpp
			then
				$HOSTDIR/bin/cpp `defsonly $CFLAGS $PLATFORM_CFLAGS $MAKEDEFS_CFLAGS` <$EXPORTFILE | while read N
				do
					if test "$N" != ""
					then
						case "$N" in
						\#* )
							;;
						* )
							echo $N
							;;
						esac
					fi
				done
			else
				echo $HOSTDIR/bin/cpp is not executable 1>&2
				exit 1
			fi
		else
			echo $EXPORTFILE does not exist 1>&2
			exit 1
		fi
	fi	
}

for et in "" n e z
do
	ECHO_MODE=$et

	if test "`echo_n a ; echo_n b`" = ab
	then
		ECHO_TEST="`echo_n \"a	b\"`"
		if test "`echo $ECHO_TEST`" = "a b"
		then
			break
		fi
	fi

	if test "$et" = "z"
	then
		echo failed to sort out echo without newlines 1>&2
		exit 1
	fi
done

if test "$OUTPUT_FILE" = ""
then
	echo no output specified 1>&2
	exit 1
fi

if test "`echo_n a ; echo_n b`" != ab
then
	echo failed to sort out echo without newlines 1>&2
	exit 1
fi

for d in $SOURCE_FILES
do
	. $d
	if test "$?" != "0"
	then
		echo $d returned an error 1>&2
		exit 1
	fi
done

for d in $INCLUDE_FILES
do
	echo include $d
done

# echo about to do EXPORTS 1>&2

if test "$EXPORTFILE" != ""
then
	EXPORTS=`readexports`
	if test "$ENTRY" != ""
	then
		if not_member $ENTRY $EXPORTS
		then
			EXPORTS="$ENTRY $EXPORTS"
		fi
	fi
else
	if test "$ENTRY" != ""
	then
		EXPORTS="$ENTRY"
	fi
fi

if test "$EXPORTS" = ""
then
	if test "$OUTPUT_TYPE" != ""
	then
		echo output_type $OUTPUT_TYPE has no exports 1>&2
		exit 1
	fi
else
	if test "$DEFFILE" = ""
	then
		if test "$NAME" != ""
		then
			DEFFILE=$INTDIR/$NAME.exp.def
		fi
	fi
fi

if test "$NAME" = ""
then
	DLLONLY=false
	NAME=`basename $OUTPUT_FILE`
else
	for d in $EXPORTS
	do
		if is_member $d $DLL_EXPORTS
		then
			DLLNAME=`dirname $OUTPUT_FILE`/$NAME.dll
		else
			DLLONLY=false
		fi
	done
fi

LAUNDRY_LIST="$LAUNDRY_LIST $DLLNAME"

getversions()
{
	VERSION_BUILD=
	for d in `echo $1 | sed y/./\ /`
	do
		if test "$VERSION_BUILD" = ""
		then
			VERSION_BUILD="$d"
		else
			VERSION_BUILD=$VERSION_BUILD.$d
		fi

		echo $VERSION_BUILD
	done
}

do_dll_stub_src_fn()
{
	case $1 in
	SOMInitModule )
		echo "extern void $1(long,long,char *);"
		echo "void ${NAME}_$1(long a ,long b,char *c) { $1(a,b,c); }"
		;;
	DllGetClassObject )
		echo "extern int $1(void *,void *,void *);"
		echo "int ${NAME}_$1(void *a ,void *b,void *c) { return $1(a,b,c); }"
		;;
	DllCanUnloadNow )
		echo "extern int $1(void);"
		echo "int ${NAME}_$1(void) { return $1(); }"
		;;
	* )
		echo function $1 not supported in $0 1>&2
		exit 1
		;;	

	esac
}

begin_cpp()
{
	echo "#ifdef __cplusplus"
	echo "extern \"C\" {"
	echo "#endif /* __cplusplus */"
}

end_cpp()
{
	echo "#ifdef __cplusplus"
	echo "}"
	echo "#endif /* __cplusplus */"
}

do_dll_proxy_src_fn()
{
	case $1 in
	SOMInitModule )
		begin_cpp
		echo "extern void ${NAME}_$1(long,long,char *);"
		echo "void $1(long a ,long b,char *c) { ${NAME}_$1(a,b,c); }"
		end_cpp
		;;
	DllGetClassObject )
		begin_cpp
		echo "extern int ${NAME}_$1(void *,void *,void *);"
		echo "int $1(void *a ,void * b,void *c) { return ${NAME}_$1(a,b,c); }"
		end_cpp
		;;
	DllCanUnloadNow )
		begin_cpp
		echo "extern int ${NAME}_$1(void);"
		echo "int $1(void) { return ${NAME}_$1(); }"
		end_cpp
		;;
	* )
		echo function $1 not supported in $0 1>&2
		exit 1
		;;	

	esac
}

do_dll_proxy_src()
{
	for d in $EXPORTS
	do
		if is_member $d $DLL_EXPORTS
		then
			do_dll_proxy_src_fn $d $NAME
		fi
	done
}

do_dll_proxy_exp()
{
	for d in $EXPORTS
	do
		if is_member $d $DLL_EXPORTS
		then
			echo $d
		fi
	done
}

# echo $OUTPUT_FILE DLLONLY = $DLLONLY 1>&2

if test "$DLLONLY" = "true"
then
	if test "$OUTPUT_TYPE" = ""
	then
		FINAL_BINARY=$OUTPUT_FILE
		SONAME_FILE=$OUTPUT_FILE
	else
		if test "$LINKTOOL_DLLNAME" != ""
		then
			FINAL_BINARY=`$LINKTOOL_DLLNAME $OUTPUT_FILE`
		else
			FINAL_BINARY=$DLLNAME
		fi
		SONAME_FILE=$DLLNAME
		OUTPUT_FILE=$DLLNAME
	fi
else
	VERSION_LIST=`$LINKTOOL_GETVERSIONS $VERSION`
	VERSION_LIST=`echo $VERSION_LIST`
	LAST_VERSION=`last $VERSION_LIST`
	FIRST_VERSION=`first $VERSION_LIST`

	if test "$OUTPUT_TYPE" = ""
	then
		LINKTOOL_VERSION=
	fi

	if test "$LINKTOOL_VERSION" = ""
	then
		FINAL_BINARY=$OUTPUT_FILE 
		SONAME_FILE=$OUTPUT_FILE
	else
		FINAL_BINARY=`$LINKTOOL_VERSION $OUTPUT_FILE $LAST_VERSION` 
		SONAME_FILE=`$LINKTOOL_VERSION $OUTPUT_FILE $FIRST_VERSION` 
	fi
fi

SONAME=`basename $SONAME_FILE`

# echo about to do DEFFILE 1>&2

if test "$DEFFILE" != ""
then
	if test "$LINKTOOL_EMIT_EXPORTS" != ""
	then
		$LINKTOOL_EMIT_EXPORTS $SONAME $EXPORTS >$DEFFILE
		if test "$?" != "0"
		then
			echo $LINKTOOL_EMIT_EXPORTS $DEFFILE failed 1>&2
			exit 1
		fi
		LAUNDRY_LIST="$LAUNDRY_LIST $DEFFILE"
	fi
fi

echo

echo_n "all:"
echo_n " $OUTPUT_FILE"
if test "$DLLNAME" != "$OUTPUT_FILE"
then
	echo_n " $DLLNAME"
fi
echo
echo_n "	if test \"\$(MAKEFILE)\" != \"\"; then if test \"\$(MAKEFILE)\" != \"$INTDIR/clean.$NAME.mak\"; then cp \"\$(MAKEFILE)\" \"$INTDIR/clean.$NAME.mak\"; fi; fi"
echo
echo

do_link()
{
	DIR_1=`dirname $1`
	BASE_1=`basename $1`
	BASE_2=`basename $2`
	echo
	echo_n $1": "
	echo $2
	echo "	cd $DIR_1; if test -h $BASE_1; then rm $BASE_1; fi; ln -s $BASE_2 $BASE_1"
	echo
}

if test "$OUTPUT_TYPE" = "-shared"
then
	if test "$LINKTOOL_VERSION" != ""
	then
		if test "$VERSION_LIST" != ""
		then
			PREV_LINK=
			PREV_VER=

			for d in "" $VERSION_LIST
			do
				if test "$d" = ""
				then
					THIS_LINK=$OUTPUT_FILE
				else
					THIS_LINK=`$LINKTOOL_VERSION $OUTPUT_FILE $d`
				fi
				if test "$PREV_LINK" != ""
				then
					if test "$PREV_VER" = ""
					then
						$LINKTOOL_STUBLIB $PREV_LINK $THIS_LINK
					else
						$LINKTOOL_SYMLINK $PREV_LINK $THIS_LINK
					fi
					LAUNDRY_LIST="$LAUNDRY_LIST $PREV_LINK"
				fi
				PREV_LINK=$THIS_LINK
				PREV_VER=$d
			done
		fi
	fi
fi

echo
echo_n $FINAL_BINARY": "
echo $OBJECTS
echo_n "	"
if test "$OUTPUT_TYPE" = "-shared"
then
	$LINKTOOL_LINKSHLB $FINAL_BINARY $LANGFLAGS
else
	if test "$OUTPUT_TYPE" != ""
	then
		echo unknown output type $OUTPUT_TYPE 1>&2
		exit 1
	fi
	if test "$LINKTOOL_LINKAPP" = ""
	then
		echo LINKTOOL_LINKAPP not defined 1>&2
		exit 1
	fi
	$LINKTOOL_LINKAPP $LANGFLAGS
fi

echo_n " " $LINKER_FLAGS

if test "$SONAME" != ""
then
	if test "$OUTPUT_TYPE" != ""
	then
		if test "$LINKTOOL_SET_SONAME" != ""
		then
			echo_n " "
			$LINKTOOL_SET_SONAME $SONAME
		fi
	fi
fi

if test "$DEFFILE" != ""
then
	if test "$LINKTOOL_SET_EXPORT_FILE" != ""
	then
		echo_n " "
		$LINKTOOL_SET_EXPORT_FILE $DEFFILE
	fi
fi

if test "$ENTRY" != ""
then
	if test "$LINKTOOL_SET_ENTRY" != ""
	then
		echo_n " "
		$LINKTOOL_SET_ENTRY $ENTRY
	fi
fi

echo_n " " $OBJECTS
echo_n " " $LIBDIRS
echo_n " " $LIBLIST
echo_n " -o" "\$@"
echo

LAUNDRY_LIST="$LAUNDRY_LIST $FINAL_BINARY"

if test "$DLLNAME" != ""
then
	if test "$DLLNAME" != "$FINAL_BINARY"
	then
		if test "$DLLONLY" = "true"
		then
			$LINKTOOL_SYMLINK $DLLNAME $FINAL_BINARY
		else
			if test "$LINKTOOL_LINKDLL" = ""
			then
				$LINKTOOL_SYMLINK $DLLNAME $SONAME_FILE
			else
				$LINKTOOL_LINKDLL $DLLNAME $SONAME_FILE
			fi
		fi
	fi
fi

echo
echo "clean:"
CLEANED=
for d in $LAUNDRY_LIST
do
	if not_member $d $CLEANED
	then
		echo "	if test -f $d; then rm $d; fi"
	fi

	CLEANED="$CLEANED $d"
done

