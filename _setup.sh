#! /bin/sh
#

#
# Install osfree build deps
#

ME=`whoami`
mkdir -p /root/osfree /root/dist /root/src
#echo "deb http://deb.debian.org/debian bullseye main contrib non-free" >/etc/apt/sources.list
#echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >>/etc/apt/sources.list
#echo "deb http://security.debian.org/debian-security bullseye-security main contrib non-free" >>/etc/apt/sources.list
apt-get update
apt-get install -y wget curl unzip tar gzip bash build-essential fpc
cd /root/dist
wget ftp://osfree.org/upload/ow/ow-1.9.zip
unzip ow-1.9.zip -d /opt
wget https://nav.dl.sourceforge.net/project/regina-rexx/regina-rexx/3.9.4/regina-rexx-3.9.4.tar.gz
tar xvzf regina-rexx-3.9.4.tar.gz -C ../src
chown -R ${ME}.${ME} /root/osfree /root/dist /root/src
cd ../src/regina-rexx-3.9.4
./configure --prefix=/usr/local
make && make install
rm -rf ../regina-rexx-3.9.4
echo /usr/local/lib >>/etc/ld.so.conf.d/local.conf
ldconfig
cd /root/osfree
