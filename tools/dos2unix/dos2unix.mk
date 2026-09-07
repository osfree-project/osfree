PORT_NAME = tools$(SEP)dos2unix
PORT_TYPE = git
PORT_URL  = https://git.code.sf.net/p/dos2unix/dos2unix
PORT_PATCHES = unix2dos.diff

!include $(%ROOT)tools/mk/port.mk
