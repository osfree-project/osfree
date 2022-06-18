#!/bin/sh -e

MAKEDEFS_LST=/tmp/makedefs.cf.$$

cleanup()
{
	rm -rf "$MAKEDEFS_LST"
}

trap cleanup 0

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

getField()
{
	while read A B
	do
		if test "$A" = "$1"
		then echo "$B"
			break
		fi
	done
}

find ../.. -type f -name makedefs.cf >"$MAKEDEFS_LST"

LIST=`../../toolbox/depends.sh $@ <"$MAKEDEFS_LST"`

while read N
do
	NAME=`getField NAME <"$N"`

	if is_member $NAME $LIST
	then
		dirname "$N"
	fi
done <"$MAKEDEFS_LST"

