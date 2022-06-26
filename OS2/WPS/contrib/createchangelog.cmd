/* */

/* createchangelog.cmd:

    this calls xdoc.exe on all the *.c files in \main and \frontend
    to have a plain-text CHANGELOG file generated from the sources.

    xdoc.exe must be in ..\xwphelpers (which is true if you have
    checked out the xwphelpers repository).
*/

'echo on'
'..\xwphelpers\xdoc -C "-iinclude;..\xwphelpers\include" src\classes\*.c src\config\*.c src\Daemon\*.c src\filesys\*.c src\hook\*.c src\media\*.c src\shared\*.c src\startshut\*.c src\widgets\*.c src\xcenter\*.c ..\xwphelpers\src\helpers\*.c'


