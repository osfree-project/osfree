/*
 * Test program for Regina 3.3
 *
 * This program tests the OpenVMS specific BIFs.
 *
 */
Trace o
Say
Say 'Name of the my current disk:' f$getdvi("sys$disk:", 'volnam')
Say 'My own pid:' f$getjpi("0","pid")
Say Begin of external command
dir '*.cxx'
Say end of external command
Return 0
