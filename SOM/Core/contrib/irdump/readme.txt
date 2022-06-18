#
#   COMPONENT_NAME: somx
#
#   ORIGINS: 27
#
#
#    25H7912  (C)  COPYRIGHT International Business Machines Corp. 1992,1996
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#    @(#) 1.3 src/somples.nt/somi/c/irdump/readme.txt, somples.somi, som3.0, s309651d 12/19/96 17:51:12 [12/23/96 08:42:18]

/*
 *
 * DISCLAIMER OF WARRANTIES.
 * The following [enclosed] code is sample code created by IBM
 * Corporation. This sample code is not part of any standard or IBM
 * product and is provided to you solely for the purpose of assisting
 * you in the development of your applications.  The code is provided
 * "AS IS". IBM MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE, REGARDING THE FUNCTION OR PERFORMANCE OF
 * THIS CODE.  IBM shall not be liable for any damages arising out of
 * your use of the sample code, even if they have been advised of the
 * possibility of such damages.
 *
 * DISTRIBUTION.
 * This sample code can be freely distributed, copied, altered, and
 * incorporated into other software, provided that it bears the above
 * Copyright notice and DISCLAIMER intact.
 */

This sample program illustrates some simple ways to retrieve
information from the SOM Interface Repository.  Once an IR
object is retrieved the somDumpSelf method is used to cause
it to write its contents to stdout.  Consequently this program
can be used as an easy way to examine the information recorded
in the Interface Repository.

The SOMIR environment variable should be set to run this sample
(e.g. set SOMIR=%SOMBASE%\etc\som.ir).

Run "nmake -f vac.mak"  to build the irdump executable. To run the irdump program enter:

   irdump <Interface Repository Object Name>

For example:

   irdump somNew

would show you the following information about the somNew method:

   "somNew": 1 entry found

   1 OperationDef "somNew"
     id:     ::SOMClass::somNew
     mode:   NORMAL
     result: TypeCodeNew (tk_objref, "::SOMObject")

If invoked without arguments, irdump will display every object in the
Interface Repository.

Run "nmake clean" to clean up the directory after you are finished with the
sample.
