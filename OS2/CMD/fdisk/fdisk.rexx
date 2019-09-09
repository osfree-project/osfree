/*  */
call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs

say SysGetMessage(500,'lvm.msg') /* Use LVM, not FDISK */

