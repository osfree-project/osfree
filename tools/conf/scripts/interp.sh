#! /bin/sh
#

set -f
cmd=` which $1`
shift

qemu-i386 $cmd $*

rc=$?
set +f
exit $rc
