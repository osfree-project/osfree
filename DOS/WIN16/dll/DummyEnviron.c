/*    DummyEnviron.c	1.3	1/29/96 dummy declaration of environ for linux
 */

/*
 *  Dummy declaration of _environ needed on Linux platform, due to bug
 *  in the way the crt*.o files interoperate with -shared -Bsymbolic flags.
 */

#if defined(linux)
char **__environ;
#endif

/*
 *  And another dummy declaration, just so this file isn't empty on
 *  other platforms.
 */
int TWIN_DLL_dummy_environ;

