#! /bin/sh
#
# map required directories into chroot
#
# Use:
# "./mountall.sh mount" to mount all dirs
# "./mountall.sh unmount" to unmount
#
# Add/remove extra dirs as needed:
dirs="/dev /dev/shm /dev/pts /proc /sys /tmp /mnt /mnt/os2l /mnt/os2l/src/l4"

case $1 in
mount)
  for i in $dirs; do
      mount -o bind $i .$i
  done
  ;;

unmount)
  for i in $dirs; do
      umount -R -f .$i
  done
  ;;
esac
