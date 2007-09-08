alias ls='ls -F'
alias l='ls -l'
alias v='less'
alias zv='zless'
alias la='ls -a'
alias /='history 24'
alias rm='rm -i'
alias pg='more'
alias uz='tar -xzvf'
alias cls='tput clear'
alias clear='tput clear'

cd /mnt
mount -t vfat /dev/hdb1 /mnt/hdb1
mount -t ext2 /dev/hdc1 /mnt/hdc1
cd ./hdc1
if [ -f ../hdb1/os2ldr ]; then
   cp ../hdb1/os2ldr .
fi
cd /

