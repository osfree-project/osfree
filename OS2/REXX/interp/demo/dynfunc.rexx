/*
 * Test program for Regina 08e
 *
 * This program demonstrates the loading of two dynamic/shared
 * libraries of external functions.
 *
 * Assumes that the dynamic/shared libraries have been built
 * and are in the current directory or in a directory specified
 * by the environment variable appropriate to the Operating System
 * eg. LD_LIBRARY_PATH under Unix, LIBPATH under OS/2.
 *
 * There is one external function in each of the dynamic libraries.
 * All this function does is displays each of the parameters passed
 * to it, plus the name of the dynamic library.
 */
Trace o
Parse Source platform .

Call Rxfuncadd 'Test1LoadFuncs','rxtest1','Test1LoadFuncs'
Call Rxfuncadd 'Test2LoadFuncs','rxtest2','Test2LoadFuncs'

/*
 * Set the arguments for each function
 */
parm1 = 'Param1'
parm2 = 'Param2'

If Test1LoadFuncs() \= 0 Then Abort("Calling Test1LoadFuncs")
If Test2LoadFuncs() \= 0 Then Abort("Calling Test2LoadFuncs")

If Test1Function1('Test1Function1',parm1) \= 0 Then
   Abort("Calling Test1Function1")
/*
 * Test for set variables...
 */
If test1function1.0 \= 2 Then
   Abort("Stem index: test1function1.0 not set correctly")
If test1function1.1 \= 'Test1Function1' Then
   Abort("Stem value: test1function1.1 not set correctly")
If test1function1.2 \= parm1 Then
   Abort("Stem value: test1function1.2 not set correctly")

If Test2Function1('Test2Function1',parm1) \= 0 Then
   Abort("Calling Test2Function1")
/*
 * Test for set variables...
 */
If test2function1.0 \= 2 Then
   Abort("Stem index: test2function1.0 not set correctly")
If test2function1.1 \= 'Test2Function1' Then
   Abort("Stem value: test2function1.1 not set correctly")
If test2function1.2 \= parm1 Then
   Abort("Stem value: test2function1.2 not set correctly")

If Test1Function2('Test1Function2',parm1,parm2) \= 0 Then
   Abort("Calling Test1Function2")
/*
 * Test for set variables...
 */
If test1function2.0 \= 3 Then
   Abort("Stem index: test1function2.0 not set correctly")
If test1function2.1 \= 'Test1Function2' Then
   Abort("Stem value: test1function2.1 not set correctly")
If test1function2.2 \= parm1 Then
   Abort("Stem value: test1function2.2 not set correctly")
If test1function2.3 \= parm2 Then
   Abort("Stem value: test1function2.3 not set correctly")

If Test2Function2('Test2Function2',parm1,parm2) \= 0 Then
   Abort("Calling Test2Function2")
/*
 * Test for set variables...
 */
If test2function2.0 \= 3 Then
   Abort("Stem index: test2function2.0 not set correctly")
If test2function2.1 \= 'Test2Function2' Then
   Abort("Stem value: test2function2.1 not set correctly")
If test2function2.2 \= parm1 Then
   Abort("Stem value: test2function2.2 not set correctly")
If test2function2.3 \= parm2 Then
   Abort("Stem value: test2function2.3 not set correctly")

return 0

Abort: Procedure
Parse Arg mesg
Say "Error trapped:" mesg
Exit 1
