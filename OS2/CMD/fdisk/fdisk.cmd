/* $Id: fdisk.cmd,v 1.2 2004/08/21 14:48:45 prokushev Exp $ */
call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs

say SysGetMessage(500,'lvm.msg') /* Use LVM, not FDISK */

/*
$Log: fdisk.cmd,v $
Revision 1.2  2004/08/21 14:48:45  prokushev
* Another set of changes from my local tree

*/
