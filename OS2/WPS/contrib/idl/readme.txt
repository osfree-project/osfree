
The IDL directory contains all the IDL source files for the
various XWorkplace classes. These are fed into the IBM SOM
compiler (SC.EXE) to produce headers, .DEF files, and create
or update .C sources.

When any of the IDL sources is changed, the makefiles take
care that the SC.EXE will (re)write the headers in include\classes
and update the C sources in src\classes.

Also, the .DEF files will be created in this directory. Note that
these are not used, but only src\shared\xwp.def, which must be updated
manually when new classes are introduced.

NOTE: Starting with V0.9.20, the idl\wps subdirectory contains
slightly tuned default WPS IDL files to allow us to override
methods that IBM didn't care to make public. See the readme there.

See PROGREF.INF for details.


