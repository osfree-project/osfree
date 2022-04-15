# !/bin/sh
#

# Edit here to use your own *.conf
# conf=docker.conf
#`which rexx` setenv.cmd ${conf} >/dev/null 2>&1
. setvars-lnx.sh
wmake -h $1 $2 $3 $4 $5 $6 $7 $8 $9
