/* createdoc.cmd:

    this calls xdoc.exe on all the *.c files
    to have HTML reference documentation created.

    xdoc.exe must be in ..\xwphelpers (which is true if you have
    checked out the xwphelpers repository).

    After this has been executed, open HTML\index.html to view the
    documentation. */

'echo on'
'..\xwphelpers\xdoc "-iinclude;..\xwphelpers\include" src\classes\*.c src\config\*.c src\Daemon\*.c src\filesys\*.c src\hook\*.c src\media\*.c src\shared\*.c src\startshut\*.c src\widgets\*.c src\xcenter\*.c ..\xwphelpers\src\helpers\*.c'

dir = directory();

rc = SysCreateObject('WPShadow', "index.html", dir, "SHADOWID="dir"\HTML\index.html", 'F')

