#! /bin/sh
#

#
# Install osfree build deps
#

ME=`whoami`
CWD=`pwd`
mkdir -p $CWD/dist $CWD/src $CWD/bin $CWD/osfree
PATH=$CWD/bin:$PATH
#echo "deb http://deb.debian.org/debian bullseye main contrib non-free" >/etc/apt/sources.list
#echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >>/etc/apt/sources.list
#echo "deb http://security.debian.org/debian-security bullseye-security main contrib non-free" >>/etc/apt/sources.list
apt-get update
apt-get install -y wget curl unzip tar gzip bash build-essential fpc
cd $CWD/dist
wget ftp://osfree.org/upload/ow/ow-1.9.zip
unzip ow-1.9.zip -d /opt
wget https://nav.dl.sourceforge.net/project/regina-rexx/regina-rexx/3.9.4/regina-rexx-3.9.4.tar.gz
tar xvzf regina-rexx-3.9.4.tar.gz -C $CWD/src
chown -R ${ME}.${ME} $CWD/osfree $CWD/dist $CWD/src $CWD/bin
cd $CWD/src/regina-rexx-3.9.4
./configure --prefix=/usr/local
CC=gcc CXX=g++ make && make install
rm -rf $CWD/src/regina-rexx-3.9.4
echo /usr/local/lib >>/etc/ld.so.conf.d/local.conf
ldconfig
cd $CWD/osfree
