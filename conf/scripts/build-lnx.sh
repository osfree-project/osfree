# !/bin/sh
#

# Edit here to use your own *.conf
# conf=docker.conf
#`which rexx` setenv.cmd ${conf} >/dev/null 2>&1
. setvars-lnx.sh
if [ "$LOG" != "" ]; then
    wmake -h $1 $2 $3 $4 $5 $6 $7 $8 $9  2>&1 | tee $(BLD)build.txt
else
    wmake -h $1 $2 $3 $4 $5 $6 $7 $8 $9
fi