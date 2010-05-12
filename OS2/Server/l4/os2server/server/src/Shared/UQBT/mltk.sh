#!/proj/walkabout/bin/bash 
#!/bin/bash
# Hopefully, the above is found in most systems
#!/opt/local/bin/bash [uq]
#
#
# Copyright (C) 2000-2001, The University of Queensland
# Copyright (C) 2000, Sun Microsystems, Inc
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#
#
# This script is a wrapper to simplify generating decoders from .spec
# files and a .m file.
# This script uses features only found in the GNU bash shell.
#
# 19 Jul 00: Cristina, added environment support as first argument ($1)
# 04 Aug 00: Mike: Changed UQ directories for 110.0.6 sml-nj and 1/Aug mltk
# 23 Jan 01: Mike: Changed UQ directories for mango/luma

SCRATCH=/tmp/mltk.scratch.$$

usage () {
	echo "usage: $0 environment spec-files matcher-file";
    echo "       where environment is one of 'sun' or 'uq'";
	exit;
}

# Clean up temp files if this script is interupted
trap 'rm -f ${SCRATCH}; exit' 0 1 2 15

# Check the invocation
if [ $# -lt 2 ]; then
	usage;
fi

# Use the environment argument to setup paths for tools
case "$1" in
sun)
TKML="/proj/walkabout/tools/sml/bin/.run-sml";
HEAP="@SMLload=/proj/walkabout/tools/mltk/sml-toolkit.sparc-solaris";
UNGENERATE=/proj/walkabout/bin/ungenerate;;

uq)
#TKML="/u9/luna/extra/tools/smlnj/bin/.run-sml";
TKML="/home/02/binary/u9.luna.extra/tools/smlnj/bin/.run-sml";
#HEAP="@SMLload=/net/luna/u9/luna/extra/tools/mltk/sml-toolkit.sparc-solaris";
HEAP="@SMLload=/home/02/binary/u9.luna.extra/tools/mltk/sml-toolkit.sparc-solaris";
#UNGENERATE=/net/luna/u0/luna/tools/NJ/base/ungenerate;;
UNGENERATE=/home/02/binary/u0.luna.tools/NJ/base/ungenerate;;
esac
shift

# Set the matcher to be the last positional parameter and derive the
# .cc file to be generated from it.
MATCHER=${@:$#:1}
MATCHER_DIR=`dirname ${MATCHER}`
MATCHER_CC=`basename ${MATCHER} .m`
MATCHER_CC="${MATCHER_DIR}/${MATCHER_CC}.cc"

# Build the command to be fed into the toolkit
#CMD="val d = CC.matcher [\""
# Norman wrote some 32 bit code for us, at great time expense to him, but
# unfortunately, this seems to cause a memory explosion. The versions from
# 4th Aug onwards have this problem. So for now, we use the 1st Aug version,
# which doesn't have the 32 bit fixes, and we turn off the field fusion
# optimisations (so it doesn't need the 32 bit code to match single sparc
# return instructions, for example).
CMD="val _ = GlobalState.Match.fuse := false; val d = CC.matcher [\""
while [ $# -ne 2 ]; do

	# Make sure the current file is a .spec file
	if [ "${1%%.spec}" = "$1" ]; then
		echo "bad suffix for \`$1': SLED files require the .spec suffix";
		exit;
	fi

	# Make sure the current file exists
	if [ ! -f $1 ]; then
		echo "$1: no such file";
		exit;
	fi

	# Add the current spec file to the command
	CMD="${CMD}${1}\",\"";
	shift;
done
CMD="${CMD}${1}\"];"
shift
CMD="${CMD} d \"${MATCHER}\";"

# Run the toolkit
eval "${TKML} ${HEAP}" <<EOF
	${CMD}
EOF

# Clean up the output from the toolkit and put it into the .cc file
mv ${MATCHER}.d ${MATCHER_CC}
${UNGENERATE} ${MATCHER_CC}
cat ${MATCHER_CC} | awk '
	/#line.*decoder\.m"/ { line = $2; print $1 " " (line-1) " " $3; next; }
	{ print; }
' > ${SCRATCH}
mv ${SCRATCH} ${MATCHER_CC}
