#!/bin/sh

SC=$0

pipe_prefix()
{
	for fn in $@
	do
		if test "$fn" != ""
		then
			echo "# 1 \"$fn\""
		fi
	done

	exec cat
}

if test "$SOMCPP" = ""
then
	if test "$SOMBASE" != ""
	then
		for d in "$SOMBASE/bin/somcpp$EXESUFFIX" "$SOMBASE/bin/somcpp" "$SOMBASE/bin/$d/somcpp.exe"
		do
			if test -f "$d"
			then
				if test -x "$d"
				then
					SOMCPP="$d"
					break
				fi
			fi
		done
	fi
fi

if test "$SOMCPP" = ""
then
	d="`dirname $SC`"
	if test "$d" != ""
	then
		for d in "$d/somcpp$EXESUFFIX" "$d/somcpp" "$d/somcpp.exe"
		do
			if test -f "$d"
			then
				if test -x "$d"
				then
					SOMCPP="$d"
					break
				fi
			fi
		done
	fi
fi

if test "$SOMCPP" = ""
then
	for d in /usr/lib/cpp \
			 /usr/bin/cpp
	do
		if test -f "$d"
		then
			if test -x "$d"
			then
				SOMCPP="$d"
			fi
		fi
	done
fi

if test "$SOMCPP" = ""
then
	SOMCPP=somcpp
fi

# echo SOMCPP=$SOMCPP

if test "$SOMIPC" = ""
then
	d="`dirname $SC`"
	if test "$d" != ""
	then
		for d in "$d/somipc$EXESUFFIX" "$d/somipc" "$d/somipc.exe"
		do
#			echo testing $d

			if test -f "$d"
			then
				if test -x "$d"
				then
					SOMIPC="$d"
					break
				fi
			fi
		done
	fi

	if test "$SOMIPC" = ""
	then
		SOMIPC=somipc
	fi
fi

# echo SOMIPC=$SOMIPC

OUTPUTDIR=
EMITTERS=
LASTFLAG=
SOMCPP_CMD="$SOMCPP -D__SOMIDL__"

for d in $@
do
	if test "$LASTFLAG" = ""
	then
		case "$d" in
		-o | -d | -I | -D | -e | -s )
			LASTFLAG="$d"
			;;
		-e* )
			d=`echo $d | sed s/-e/\\ /`
			EMITTERS="$EMITTERS $d"
			;;
		-s* )
			d=`echo $d | sed s/-s/\\ /`
			EMITTERS="$EMITTERS $d"
			;;
		-I* )
			d=`echo $d | sed s/-I//`
			if test "$d" != ""
			then
				SOMCPP_CMD="$SOMCPP_CMD -I$d"
			fi
			;;
		-D* )
			d=`echo $d | sed s/-D//`
			if test "$d" != ""
			then
				SOMCPP_CMD="$SOMCPP_CMD -D$d"
			fi
			;;
		-p )
			SOMCPP_CMD="$SOMCPP_CMD -D__PRIVATE__"
			;;
		-u )
			;;
		-* )
			echo $SC: misunderstood switch $d 1>&2
			exit 1
			;;
		*.idl )
			FILELIST="$FILELIST $d"
			;;
		* )
			echo $SC: misunderstood file $d 1>&2
			exit 1
			;;
		esac
	else
		case "$LASTFLAG" in
		-I | -D )
			SOMCPP_CMD="$SOMCPP_CMD $LASTFLAG$d"
			;;
		-d | -o )
			if test "$OUTPUTDIR" != ""
			then
				echo $SC: $LASTFLAG already set as $OUTPUTDIR 1>&2
				exit 1
			fi
			OUTPUTDIR="$d"
			;;
		-e | -s )
			EMITTERS="$EMITTERS $d"
			;;
		* )
			echo $SC: misunderstood argument $LASTFLAG $d 1>&2
			exit 1
			;;
		esac
		LASTFLAG=
	fi
done

# echo SOMCPP_CMD="$SOMCPP_CMD"

for e in `echo $EMITTERS | sed y/\\:/\\ /`
do
#	echo EMITTER=$e

	for d in $FILELIST
	do
#		echo FILENAME=$d

		for OUTPUT in `basename $d | sed y/\\./\\ /`
		do
#			echo OUTPUT=\"$OUTPUT\"

			BASENAME=`basename $d`

			if test "$BASENAME" != "$OUTPUT.idl"
			then	
				echo $SC: "$BASENAME" != "$OUTPUT.idl" 1>&2

				exit 1
			fi

			SOMIPC_CMD="$SOMIPC -s $e "

			case "$e" in
			ir )
				if test "$SOMIR" = ""
				then
					SOMIR=som.ir
				fi

				OUTPUT=

				for f in `echo $SOMIR | sed y/\\:/\\ /`
				do
					OUTPUT=$f
				done

				if test ! -f $OUTPUT
				then
					(
						OUTPUT_DIR=`dirname $OUTPUT`
						OUTPUT_BASE=`basename $OUTPUT`

						cd $OUTPUT_DIR

						if test "$?" != "0"
						then
							echo cd $OUTPUT_DIR failed 1>&2
							exit 1
						fi

						uudecode <<EOF
begin 644 $OUTPUT_BASE
@!1>9>0   ,@          +"0K0\$     G\*\$\$       !
 
end
EOF

						RC=$?

						if test "$RC" != "0"
						then
							exit $RC
						fi
					)

					RC=$?

					if test "$RC" != "0"
					then
						exit $RC
					fi
				fi
				;;
			* )
				if test "$OUTPUTDIR" = ""
				then
					OUTPUT="$OUTPUT.$e"
				else
					OUTPUT="$OUTPUTDIR/$OUTPUT.$e"
				fi
				;;
			esac

#			echo OUTPUT=\"$OUTPUT\"

			SOMIPC_CMD="$SOMIPC_CMD -o $OUTPUT"

#			echo SOMIPC_CMD="$SOMIPC_CMD"

			echo "#include \"$d\"" | $SOMCPP_CMD | pipe_prefix "$d" | $SOMIPC_CMD

			SOMIPC_RC=$?

			if test "$SOMIPC_RC" != "0"
			then
				exit $SOMIPC_RC
			fi

			if test -f "$OUTPUT"
			then
				break
			else
				echo $SC: $OUTPUT missing
				exit 1
			fi
		done
	done
done

