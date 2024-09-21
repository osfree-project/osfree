#! /bin/sh
#

export ROOT=.
while [ ! -f "$ROOT/tools/mk/all.mk" ]; do ROOT="$ROOT/.."; done
export PATH=$ROOT/tools/conf/scripts:$PATH
build-lnx.sh $*
