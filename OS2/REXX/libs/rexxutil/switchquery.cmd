/* Search the current directory for all files with the extension 'ptr'
 * and rename them to have the extension 'ico'.
 */



/*
 * Now load all functions
 */
say "Load all functons..."
rc = rxfuncadd('SysLoadFuncs','REXXUTIL', 'SysLoadFuncs')
say rc
call SysLoadFuncs
say "Done."

say SysQuerySwitchList('Test', 'IGND')

say "X: "test.0

DO i = 1 TO test.0
    say  test.i
END
