Build instructions

1. change to lxLight_src directory

2. The common subdirectory contains a number of my commonly-used libraries.
Between them is a improved version of CRT.PAS which:
a) features a speedy screen output (VP`s CRT is pretty slow...)
b) Handles properly input and output redirection. Two additional
boolean variables: RedirInput and RedirOutput are available.

3. Edit MAKE.CMD and set base VP directory as appropiate.

4. Run MAKE.CMD. This should create the directory OUT into which
all output will go. After compilation executables are copied into ..\lxLite\ 
directory.

5. Follow instructions on screen.

----------------------------------------------------------------------------

To build Win32 version:
* VP in path
* brc32 from Borland Delphi (I got one from 4.0, may be BCC 5.5 will be good
  too).

  mklxlite.cmd

  Sorry, I used LXLITE only as EXE manipulation tool for development, so any
changes performed in random quick way ;)
