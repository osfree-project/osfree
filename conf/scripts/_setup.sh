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
apt-get install -y wget curl unzip tar gzip bash build-essential make
cd $_CWD/dist
wget ftp://osfree.org/upload/ow/ow-1.9.zip
unzip -o ow-1.9.zip -d /opt
wget https://downloads.sourceforge.net/project/freepascal/OS_2/3.2.2/os2322.zip
unzip -o os2322.zip -d $_CWD/dist/os2
wget https://downloads.sourceforge.net/project/freepascal/Linux/3.2.2/fpc-3.2.2.i386-linux.tar
tar xf fpc-3.2.2.i386-linux.tar
cd fpc-3.2.2.i386-linux
./install.sh </dev/null
for file in $CWD/dist/os2/*.zip; do
  unzip -o $file units/* fpmkinst/* -d /usr/lib/fpc/3.2.2/
done
wget https://nav.dl.sourceforge.net/project/regina-rexx/regina-rexx/3.9.4/regina-rexx-3.9.4.tar.gz
tar xvzf regina-rexx-3.9.4.tar.gz -C $_CWD/src
chown -R $_ME.$_ME $_CWD/osfree $_CWD/dist $_CWD/src $_CWD/bin
cd $_CWD/src/regina-rexx-3.9.4
./configure --prefix=/usr/local
make && make install
cd ..
rm -rf $_CWD/src/regina-rexx-3.9.4
echo /usr/local/lib >>/etc/ld.so.conf.d/local.conf
ldconfig
cd $_CWD
find /opt/watcom -type d -exec chmod 755 {} \;
find /opt/watcom -type f -exec chmod 644 {} \;
find /opt/watcom/binl -type f -exec chmod 755 {} \;
