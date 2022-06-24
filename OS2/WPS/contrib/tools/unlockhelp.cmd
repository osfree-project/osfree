/*
 *  unlockhelp.cmd:
 *      unlocks the XWP help file by displaying a non-existing
 *      help file from WPHELP.HLP.
 *
 *      (C) 2002 Ulrich M”ller.
 */

Call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
Call SysLoadFuncs

rc = SysSetObjectData("<WP_DESKTOP>", "SHOWHELPPANEL=1");


