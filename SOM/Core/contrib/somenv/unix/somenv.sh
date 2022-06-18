#!/bin/sh

SOMENV_SH="$1"
SOMENV_INI="$2"

if test "$HOSTNAME" = ""
then
	HOSTNAME=`hostname`
fi

if test "$HOSTNAME" = ""
then
	HOSTNAME=localhost
fi

genbase()
{
	(
		cd $SOMBASE
		if test "$?" = "0"
		then
			SOMBASE_PWD=`pwd`
			if test "$SOMBASE_PWD" = ""
			then
				echo SOMBASE="$SOMBASE"
			else
				echo SOMBASE="$SOMBASE_PWD"
			fi
		else
			echo SOMBASE="$SOMBASE"
		fi
	)
}

genscript()
{
	genbase
	cat <<EOF
SOMENV=\$SOMBASE/etc/somenv.ini
SOMDDIR=\$SOMBASE/etc/dsom
SOMIR=\$SOMBASE/etc/som.ir

PATH=\$PATH:\$SOMBASE/bin

EOF

	if test "$DLLPATHENV" != "PATH"
	then
		if test "$DLLPATHENV" != ""
		then
			cat <<EOF
if test "\$$DLLPATHENV" = ""
then
	$DLLPATHENV=\$SOMBASE/lib
else
	$DLLPATHENV=$DLLPATHENV:\$SOMBASE/lib
fi

export $DLLPATHENV
EOF
		fi
	fi

	cat <<EOF
export SOMBASE
export SOMDDIR
export SOMENV
export SOMIR
export PATH
EOF
}

gencfg()
{
	cat <<EOF
[somd]
SOMDPROTOCOLS=SOMD_TCPIP
HOSTNAME=localhost
[SOMD_TCPIP]
SOMDPORT=3003
HOSTNAME=localhost
EOF
}

genscript >"$SOMENV_SH"
gencfg >"$SOMENV_INI"
