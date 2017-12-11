#!/bin/sh
# Makes a tar'ed version of all the files that have an entry in a CVS/Entries
# file under this directory.

if [ "$1" = "-z" ]; then
	COMPRESS=1
else
	COMPRESS=0
fi

CVSDIRS=`find . -type d -a -name 'CVS'`

SRCFILES=srcs.$$
for dir in ${CVSDIRS}; do
    FULLPATH=`dirname $dir`;
    ENTRIES=$dir/Entries;
	echo $ENTRIES >> ${SRCFILES}
    awk -v DIR=$FULLPATH -F "/" '
        {
            if ($1 !~ /^D/)
				printf "%s/%s\n", DIR, $2
		}
	' < $ENTRIES >> ${SRCFILES};
done;

SRCS=""
# Weed out entries for files that don't exist
for f in `cat $SRCFILES`; do
	if [ -f $f ]; then
		SRCS="${SRCS} $f";
	else
		echo "File not found: $f";
	fi
done;

if [ $COMPRESS -eq 1 ]; then
	echo "Making uqbt.tgz"
	rm uqbt.tgz
	tar cvzf uqbt.tgz ${SRCS}
else
	echo "Making uqbt.tar"
	rm uqbt.tar
	tar cvf uqbt.tar ${SRCS}
fi

rm ${SRCFILES}
