Build instructions

1. The COMMON\ subdirectory contains a number of my commonly-used libraries.
Between them is a improved version of CRT.PAS which:
a) features a speedy screen output (VP`s CRT is pretty slow...)
b) Handles properly input and output redirection. Two additional
boolean variables: RedirInput and RedirOutput are available.

2. Edit MAKE.CMD and set base VP directory as appropiate.

3. Run MAKE.CMD. This should create the directory OUT into which
all output will go. After compilation executables are copied into ..\lxLite\ 
directory.

4. Follow instructions on screen.
