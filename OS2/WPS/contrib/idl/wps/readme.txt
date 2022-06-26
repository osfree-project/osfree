
V0.9.20 (2002-07-29) [umoeller]

The idl\wps\ directory contains slightly tuned default WPS IDL files
to allow us to override methods that IBM didn't care to make public.

The point is that XWP _needs_ to override some of these undocumented
methods, but can't do so easily without support from the SOM compiler.

As a result, with V0.9.20, I have found the following to work:

1)  We set idl\wps\ to be the first directory on the SMINCLUDE path
    so that the SOM compiler will use the files from this dir instead
    of those from the toolkit IDL dir, if present.

    This allows us to override undocumented methods simply by
    tagging them with "override" in IDL files of the XWorkplace
    classes (in the parent directory).

2)  idl\makefile now produces headers from the files in idl\wps\ as
    well and puts them into include\classes\wps\.

3)  The master include path (in setup.in) has been changed to have
    include\classes\wps\ before the toolkit path as well so that
    when we compile, the tuned versions of the WPS headers will be
    used instead of the default ones.

When changing default WPS IDL files, be very careful... you better
get the releaseorder right. Fortunately, IBM _has_ listed most of
the undocumented methods are in the releaseorders and only the
prototypes are missing.


