#! /bin/sh
#

#
# Install osfree build deps
#

ME=`whoami`
sudo mkdir -p /root/osfree /root/dist /root/src
#echo "deb http://deb.debian.org/debian bullseye main contrib non-free" >/etc/apt/sources.list
#echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >>/etc/apt/sources.list
#echo "deb http://security.debian.org/debian-security bullseye-security main contrib non-free" >>/etc/apt/sources.list
sudo apt-get update
sudo apt-get install -y wget curl unzip tar gzip bash build-essential fpc
cd /root/dist
wget ftp://osfree.org/upload/ow/ow-1.9.zip
sudo unzip ow-1.9.zip -d /opt
wget https://nav.dl.sourceforge.net/project/regina-rexx/regina-rexx/3.9.4/regina-rexx-3.9.4.tar.gz
sudo tar xvzf regina-rexx-3.9.4.tar.gz -C ../src
sudo chown -R ${ME}.${ME} /root/osfree /root/dist /root/src
cd ../src/regina-rexx-3.9.4
./configure --prefix=/usr/local
make && sudo make install
sudo rm -rf ../regina-rexx-3.9.4
sudo echo /usr/local/lib >>/etc/ld.so.conf.d/local.conf
sudo ldconfig
cd /root/osfree
