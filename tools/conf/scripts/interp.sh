#! /bin/sh
#

cmd=`which $1`
shift

qemu-i386 $cmd $*

rc=$?
exit $rc
