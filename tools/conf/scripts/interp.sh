#! /bin/sh
#

x=$1
shift
y=$*

qemu-i386 `which $x` $y
