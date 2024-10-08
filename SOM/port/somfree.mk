
PORT_NAME = tools$(SEP)somfree
PORT_TYPE = svn
PORT_URL  = https://svn.code.sf.net/p/somfree/code/
PORT_REV  = trunk
PORT_PATCHES  = somfree.diff somfree2.diff somfree3.diff somfree4.diff &
  somfree5.diff somfree6.diff somfree7.diff somfree8.diff somfree9.diff &
  somfree10.diff

!include $(%ROOT)tools/mk/port.mk
