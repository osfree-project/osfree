#!/bin/sh
#
#  Copyright 2008, Roger Brown
#
#  This file is part of Roger Brown's Toolkit.
#
#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
# 
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#
# $Id$
#

if test "$CC" = ""
then
	echo $0, CC not set 1>&2
	exit 1
fi

if test "$INTDIR" = ""
then
	echo $0, INTDIR not set 1>&2
	exit 1
fi

X11LIBS=
XTLIBS=
XMLIBS=
X11LIBPATH=
X11APP=
X11INCL=

expand_cc()
{
    echo echo $@ | sh
}

try_cc()
{
     INTDIR="$INTDIR" ../../toolbox/trycc.sh \
			$CFLAGS $PLATFORM_CFLAGS \
			`expand_cc $CONFIG_OPTS` \
			$@ 
}

arglist()
{
	OUTPUT=
    pref=$1
    shift
    while test "$1" != ""
    do
		if test "$OUTPUT" = ""
		then
       		OUTPUT="${pref}$1"
		else
       		OUTPUT="$OUTPUT ${pref}$1"
		fi
       shift
    done
	if test "$OUTPUT" != ""
	then
		echo "$OUTPUT"
	fi
}

if test "$THREADLIBS" = ""
then
	try_cc <<EOF
#ifdef _REENTRANT
int main(int argc,char **argv) { return argc && argv; }
#else
#error	not _REENTRANT
#endif
EOF
	RC="$?"
	if test "$RC" = "0"
	then
		for e in "" pthread
		do
			try_cc `arglist -l $e` <<EOF
#include <pthread.h>
#ifdef _REENTRANT
static void myclean(void *pv) { if (pv) { } ; }
int main(int argc,char **argv) { 
pthread_t self=pthread_self();
void *pv=NULL;
pthread_cleanup_push(myclean,pv);
pthread_cancel(self);
pthread_testcancel();
pthread_cleanup_pop((argc ? 1 : 0));
pthread_join(self,pv);
return argc && argv; }
#else
#error	not _REENTRANT
#endif
EOF
			RC="$?"
			if test "$RC" = "0"
			then
				if test "$e" != ""
				then
					THREADLIBS=`arglist -l $e`
				fi
				break
			fi
		done
	fi
fi

if test "$SOCKLIBS" = ""
then
	for e in "" socket nsl "socket nsl"
	do
    	try_cc `arglist -l $e` <<EOF
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
int main(int argc,char **argv) { 
return argc && argv && gethostbyname(argv[0]) &&
	socket(AF_UNIX,SOCK_STREAM,0) && getsockname(0,NULL,NULL); }
EOF
	    RC="$?"
    	if test "$RC" = "0"
	    then
			SOCKLIBS=`arglist -l $e`
			SOCKLIBS=`echo $SOCKLIBS`
        	break
	    fi
	done
fi

if test "$DLLIBS" = ""
then
	for e in "" dl
	do
    	try_cc `arglist -l $e` <<EOF
#include <sys/types.h>
#include <dlfcn.h>
int main(int argc,char **argv) { 
return argc && argv && dlopen(argv[0],0); }
EOF
	    RC="$?"
    	if test "$RC" = "0"
	    then
			DLLIBS=`arglist -l $e`
			DLLIBS=`echo $DLLIBS`
        	break
	    fi
	done
fi

# first find Xlib.h 

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
        d="$d/include"
    fi
    try_cc `if test "$d"; then echo -I$d; fi` <<EOF
#include <X11/Xlib.h>
int main(int argc,char **argv) { return argc && argv; }
EOF
    RC="$?"
    if test "$RC" = "0"
    then
        if test "$d" != ""
        then
            X11INCL=`arglist -I $d`
        fi
        break
    fi
done

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
        d="$d/include"
    fi
    try_cc `if test "$d"; then echo -I$d; fi` <<EOF
#include <Xm/Xm.h>
int main(int argc,char **argv) { return argc && argv; }
EOF
    RC="$?"
    if test "$RC" = "0"
    then
        if test "$d" != ""
        then
            X11INCL=`arglist -I $d`
        fi
        break
    fi
done

# now look for ICE

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
	for e in "" -lICE
    do
       try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/ICE/ICElib.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
IceCloseConnection(NULL);
return argc && argv; }
EOF
       RC="$?"
       if test "$RC" = "0"
       then
           if test "$e" != ""
           then
               X11LIBS="$e $X11LIBS"
           fi
           if test "$d" != ""
           then
               X11LIBPATH="$d $X11LIBPATH"           
           fi
           break
       fi
    done
    if test "$RC" = "0"
    then
       break
    fi
done

# now look for SM

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lSM
    do
       try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/SM/SMlib.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
SmcCloseConnection(NULL,0,NULL);
return argc && argv; }
EOF
       RC="$?"
       if test "$RC" = "0"
       then
		   if test "$e" != ""
           then
              X11LIBS="$e $X11LIBS"
           fi
           if test "$d" != ""
           then
              X11LIBPATH="$X11LIBPATH"           
           fi
           break
       fi
    done
    if test "$RC" = "0"
    then
        break
    fi 
done

# now look for X11

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lX11
    do
       	try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
return argc && argv && XOpenDisplay(NULL); }
EOF
      	RC="$?"
      	if test "$RC" = "0"
      	then
         	if test "$e" != ""
         	then
            	X11LIBS="$e $X11LIBS"
        	fi
        	if test "$d" != ""
 	 		then
             	X11LIBPATH="$d $X11LIBPATH"           
          	fi
			PLATFORM_CFLAGS="$PLATFORM_CFLAGS -D_PLATFORM_X11_"
			PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -D_PLATFORM_X11_"
			PLATFORM_SCFLAGS="$PLATFORM_SCFLAGS -D_PLATFORM_X11_"
			break
       	fi
   	done
   	if test "$RC" = "0"
   	then
       	break
   	fi
done

# now look for Xext

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lXext
    do
      try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
return argc && argv && XShapeQueryExtension(NULL,NULL,NULL); }
EOF
      RC="$?"
      if test "$RC" = "0"
      then
        if test "$e" != ""
        then
           X11LIBS="$e $X11LIBS"
        fi
        if test "$d" != ""
        then
           X11LIBPATH="$d $X11LIBPATH"           
        fi
        break
      fi
   done
   if test "$RC" = "0"
   then
       break
   fi
done

# now look for Xpm

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lXpm
    do
      try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
return argc && argv && XpmGetErrorString(0); }
EOF
      RC="$?"
      if test "$RC" = "0"
      then
          if test "$e" != ""
          then
             X11LIBS="$e $X11LIBS"
          fi
          if test "$d" != ""
          then
             X11LIBPATH="$d $X11LIBPATH"           
          fi
          break
      fi
   done
   if test "$RC" = "0"
   then
      break
   fi
done

# now look for Xp

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lXp
    do
      try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/Print.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
XpSetContext((Display *)0,(XPContext)0);
return argc && argv; }
EOF
      RC="$?"
      if test "$RC" = "0"
      then
          if test "$e" != ""
          then
             X11LIBS="$e $X11LIBS"
          fi
          if test "$d" != ""
          then
             X11LIBPATH="$d $X11LIBPATH"           
          fi
          break
      fi
   done
   if test "$RC" = "0"
   then
      break
   fi
done

# now look for Xt

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lXt
    do
       try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Intrinsic.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
return argc && argv &&  XtCreateApplicationContext(); }
EOF
       RC="$?"
       if test "$RC" = "0"
       then
          if test "$e" != ""
          then
             XTLIBS="-lXt $XTLIBS"
          fi
          if test "$d" != ""
          then
             X11LIBPATH="$d $X11LIBPATH"           
          fi
          break
       fi
    done
    if test "$RC" = "0"
    then
       break
    fi
done

# now look for Xmu

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lXmu
    do
      try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $XTLIBS $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <X11/Xlib.h>
$FINDLIBS_REQUIRED_GLOBALS
extern Bool _XEditResCheckMessages(void);
int main(int argc,char **argv) {
return argc && argv && _XEditResCheckMessages(); }
EOF
      RC="$?"
      if test "$RC" = "0"
      then
          if test "$e" != ""
          then
             XTLIBS="$e $XTLIBS"
          fi
          if test "$d" != ""
          then
             X11LIBPATH="$d $X11LIBPATH"           
          fi
          break
      fi
   done
   if test "$RC" = "0"
   then
      break
   fi
done

# now look for Xm

for d in "" $X11CONTENDERS
do
    if test "$d"
    then
       d="$d/lib"
    fi
    for e in "" -lXm
    do
      	try_cc $X11INCL `arglist -L $d $X11LIBPATH` $e $XTLIBS $X11LIBS $SOCKLIBS <<EOF
#include <stdio.h>
#include <Xm/Xm.h>
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
XmString str=NULL;
XtCreateApplicationContext(); 
XmStringFree(str);
return argc && argv; }
EOF
      	RC="$?"
      	if test "$RC" = "0"
      	then
          	if test "$e" != ""
          	then
             	XMLIBS="$e $XMLIBS"
          	fi
          	if test "$d" != ""
          	then
             	X11LIBPATH="$d $X11LIBPATH"           
          	fi
			PLATFORM_CFLAGS="$PLATFORM_CFLAGS -D_PLATFORM_MOTIF_"
			PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -D_PLATFORM_MOTIF_"
			PLATFORM_SCFLAGS="$PLATFORM_SCFLAGS -D_PLATFORM_MOTIF_"
          	break
      	fi
   	done
   	if test "$RC" = "0"
   	then
      	break
   	fi
done

for d in "" "-lpcsclite" "-lPCSC" "-L$OUTDIR/otherlib -lPCSC"
do
	if test "$PCSCLIBS" != ""
	then
		break
	fi
	for e in "#include <winscard.h>"  "#include <PCSC/winscard.h>"
	do
    	try_cc $d $SOCKLIBS <<EOF
#include <stdio.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
static SCARDHANDLE hCard;
SCardDisconnect(hCard,SCARD_UNPOWER_CARD);
return argc && argv; }
EOF
    	RC="$?"
		if test "$RC" = "0"
		then
			PCSCLIBS="$d"
			break
		fi
	done
done

for d in "" "-lusb"
do
	if test "$USBLIBS" != ""
	then
		break
	fi
	for e in "#include <usb.h>"  
	do
    	try_cc $d $SOCKLIBS <<EOF
#include <stdio.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
usb_init();
usb_find_busses();
usb_find_devices();
return argc && argv; }
EOF
    	RC="$?"
		if test "$RC" = "0"
		then
			USBLIBS="$d"
			break
		fi
	done
done

for d in "" "-lexpat"
do
	if test "$EXPATLIBS" != ""
	then
		break
	fi
	for e in "#include <expat.h>"  
	do
    	try_cc $d <<EOF
#include <stdio.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
	XML_Parser p=XML_ParserCreate(NULL);
	return (p==NULL);}
EOF
    	RC="$?"
		if test "$RC" = "0"
		then
			EXPATLIBS="$d"
			break
		fi
	done
done

for d in "" "-lcrypto"
do
	if test "$CRYPTOLIBS" != ""
	then
		break
	fi
	for e in "#include <openssl/crypto.h>"  
	do
    	try_cc $d <<EOF
#include <stdio.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
	const char * p=SSLeay_version(1);
	return (p==NULL);}
EOF
    	RC="$?"
		if test "$RC" = "0"
		then
			CRYPTOLIBS="$d"
			break
		fi
	done
done

for d in "" "-lssl" "-lssl $CRYPTOLIBS"
do
	if test "$SSLLIBS" != ""
	then
		break
	fi
	for e in "#include <openssl/ssl.h>"  
	do
    	try_cc $d <<EOF
#include <stdio.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
	SSL *s=NULL;
	const char *p=SSL_get_version(s);
	return (p==NULL);}
EOF
    	RC="$?"
		if test "$RC" = "0"
		then
			SSLLIBS="$d"
			break
		fi
	done
done

for d in "" "-lcurl"
do
	if test "$CURLLIBS" != ""
	then
		break
	fi
	for e in "#include <curl/curl.h>"  
	do
    	try_cc $d <<EOF
#include <stdio.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
	CURL *curl=NULL;
	curl_easy_reset(curl);
	return 0;}
EOF
    	RC="$?"
		if test "$RC" = "0"
		then
			CURLLIBS="$d"
			break
		fi
	done
done

for d in " " "-luuid" "-lCoreFoundation" "-L/usr/local/lib -lCoreFoundation"
do
	for e in 												\
		"#include <sys/uuid.h>"  							\
		"#include <uuid/uuid.h>" 							\
		"#include <uuid.h>"
	do
		for f in 											\
			"uuid_t a; uint_t s; uuid_create(&a,&s);" 		\
			"uuid_t a; uuid_generate(a); " 					\
			"uuid_t a; uint32_t s; uuid_create(&a,&s);" 
		do
			if test "$UUIDLIBS" != ""
			then
				break
			fi
	   		try_cc $d <<EOF
#include <sys/types.h>
$e
$FINDLIBS_REQUIRED_GLOBALS
int main(int argc,char **argv) {
$f
return argc && argv; }
EOF
			RC="$?"
			if test "$RC" = "0"
			then
				UUIDLIBS="$d"
				break
			fi
		done
	done
done

echo THREADLIBS=\"$THREADLIBS\"\;
echo SOCKLIBS=\"$SOCKLIBS\"\;
echo DLLIBS=\"$DLLIBS\"\;
echo X11INCL=\"$X11INCL\"\;
echo X11LIBPATH=\"$X11LIBPATH\"\;
echo X11LIBS=\"$X11LIBS\"\;
echo X11APP=\"$X11APP\"\;
echo XTLIBS=\"$XTLIBS\"\;
echo XMLIBS=\"$XMLIBS\"\;
echo USBLIBS=\"$USBLIBS\"\;
echo PCSCLIBS=\"$PCSCLIBS\"\;
echo UUIDLIBS=\"$UUIDLIBS\"\;
echo EXPATLIBS=\"$EXPATLIBS\"\;
echo CRYPTOLIBS=\"$CRYPTOLIBS\"\;
echo SSLLIBS=\"$SSLLIBS\"\;
echo CURLLIBS=\"$CURLLIBS\"\;

