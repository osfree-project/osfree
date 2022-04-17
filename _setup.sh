#! /bin/sh
#

#
# Install osfree build deps
#

_ME=`whoami`
_CWD=`pwd`
mkdir -p $_CWD/dist $_CWD/src $_CWD/bin $_CWD/osfree
PATH=$_CWD/bin:$PATH
#echo "deb http://deb.debian.org/debian bullseye main contrib non-free" >/etc/apt/sources.list
#echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >>/etc/apt/sources.list
#echo "deb http://security.debian.org/debian-security bullseye-security main contrib non-free" >>/etc/apt/sources.list
apt-get update
apt-get install -y wget curl unzip tar gzip bash
cd $_CWD/dist
wget ftp://osfree.org/upload/ow/ow-1.9.zip
unzip ow-1.9.zip -d /opt
wget ftp://ftp.hu.freepascal.org/pub/fpc/dist/3.2.2/x86_64-linux/fpc-3.2.2.x86_64-linux.tar
tar xvf fpc-3.2.2.x86_64-linux.tar
cd fpc-3.2.2.x86_64-linux
./install.sh </dev/null
cd ..
wget https://nav.dl.sourceforge.net/project/regina-rexx/regina-rexx/3.9.4/regina-rexx-3.9.4.tar.gz
tar xvzf regina-rexx-3.9.4.tar.gz -C $_CWD/src
chown -R $_ME.$_ME $_CWD/osfree $_CWD/dist $_CWD/src $_CWD/bin
cd $_CWD/src/regina-rexx-3.9.4
./configure --prefix=/usr/local
CC=gcc CXX=g++ make && make install
rm -rf $_CWD/src/regina-rexx-3.9.4
echo /usr/local/lib >>/etc/ld.so.conf.d/local.conf
ldconfig
cd $_CWD
