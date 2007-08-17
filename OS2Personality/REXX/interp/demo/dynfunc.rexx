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

Call Rxfuncadd 'Test1LoadFuncs','test1','Test1LoadFuncs'
Call Rxfuncadd 'Test2LoadFuncs','test2','Test2LoadFuncs'

/*
 * With these variables set here, they are lost as a result
 * of Test?LoadFuncs()
 */
parm1 = 'Param1'
parm2 = 'Param2'

If Test1LoadFuncs() \= 0 Then Abort("Calling Test1LoadFuncs")
If Test2LoadFuncs() \= 0 Then Abort("Calling Test2LoadFuncs")

/*
 * With these variables set here, everything works correctly.
 *
parm1 = 'Param1'
parm2 = 'Param2'
 */

If Test1Function1('Test1Function1',parm1) \= 0 Then
   Abort("Calling Test1Function1")

If Test2Function1('Test2Function1',parm1) \= 0 Then
   Abort("Calling Test2Function1")

If Test1Function2('Test1Function2',parm1,parm2) \= 0 Then
   Abort("Calling Test1Function2")

If Test2Function2('Test2Function2',parm1,parm2) \= 0 Then
   Abort("Calling Test2Function2")

return 0

Abort: Procedure
Parse Arg mesg
Say "Error trapped:" mesg
Exit 1
