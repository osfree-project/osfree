; $Id: readme.txt,v 1.3 2001/03/12 21:53:38 bird Exp $ 

Id: $

 The JFS source for OS/2
 -----------------------

Just a few words from me, Knut, who has tried to make sense in the reference
code from the Linux JFS port. We have got most of the missing files from
IBM everything is published under GPL. (See the COPYING for more GPL info.)

To compile the tree you'll need:
    1. The OS/2 DDK (available from IBM for free),
    2. IBM C/C++ Compiler and Tools v.3.6.5 (that's the one I use...)
    3. Microsoft C v6.0a
    4. OS/2 Developers Toolkit v4.5 (v4.0 might work well, haven't tried.)

Then you'll have to modify the paths in the setenv.cmd file to reflect you
local tools configuration. Then invoke setenv.cmd to set the environment.

The source for the IFS driver (IFS = Installable File System) or FSD
(FileSystem Driver) if you like, if located in src/jfs/ifs. This source
compiles. Please study the makefile before doing too much wrong. The
BLD_TYPE enviroment variable / makefile macro sets the type of build.
(I've tried to add a an new build type, icatgam, which should make a jfs.ifs
binary which is debuggable with the ICAT Debugger. I haven't got this working
yet... ICAT doesn't find the jfs.ifs module.)
Take a look at the -D_JFS_* defines which aren't enabled. The initial makefile
had only -D_JFS_OS2 defined. I've tried to make it work with all enabled,
I wan't to have the LAZYWRITE thread _VERY_ badly. The performance is
disasterous without a lazywrite! See comment on the in the current state section.

There are some utilites which are most are not compilable due to the libfs.lib
not compiling. This is caused by a missing headerfile and/or an outdated
dskioctl.h in the DDK. I haven't addressed this problem yet.
The utility source is found in src/jfs/utils.

There is a common directory too under src/jfs. In this you'll find a subsystem
(which is JFS) privat makefile include and the C and Assembly include directory.
Note that there is a private include directory under the src/jfs/common/include
directory named priv. There is also a fake include directory under include
named net32, this contains forwarders to toolkit files by the same name. This
is needed because there are source files which includes "net32\<filename>"
instead of include the "<filename>" as it is present in the toolkit.


Current state.
--------------

I've had it compile just fine. And you can't believe it, but it loaded just
fine when installing on my test machine. It even seemed to work fine too.
Then I recompiled it using a debug build type and lazywrite, cachestats and
that stuff enabled, it worked most of the time. The performance is excellent
with the lazywrite enabled. BUT, there seems to be at least one deadlock
which has to be traced down. And if the device in some way is active during
reboot (ctrl+alt+delete) it often hanges, iUnmount complained that the
volume were active... (At least when I had a deadlock or something similar,
with three blocked processes.)
Also, I experienced some asserts in jfs_dmap, on line 2799 and line 2xxx
(don't recall) (not managed to reproduce them yet, haven't tried too much
either.)

So, there was probably a reason for not enabling the lazywriter...


Testing
--------

If we're to do anything with this source, we'll have to create tests.
I've created a Test directory, src/jfs/Tests. Currently one program
which creates a lots of files in the current directory. This made
the debug jfs with lazywrite enabled deadlock. Directory listings
of the directory locked up after at file no. 1639.

The tests should be more than create directories. They should be
entire test scripts, which creates partitions, formats them,
play with them and reports all types of errors. At least this
should be a goal over some time. And the motivation should be
to eliminate _all_ types of errors. I don't wan't by 18GB of
code, programs and data destroyed due to an simple bug in
our custom JFS!


Apr 21 2000 1:15am, Oslo, Norway.

knut st. osmundsen
(knut.stange.osmundsen@mynd.no)


