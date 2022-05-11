Windows 1.01 LGO format
=======================

LGO is a LOGO file format which executed during boot of Windows and executed by WIN.COM.
LGO is a raw binary file, attached to WIN.COM. WIN.COM adjust CS for LGO and pass control
to 4 offset to show logo, and to 7 offset to disable logo. Bytes 0-3 is a signature 'LOGO'.

