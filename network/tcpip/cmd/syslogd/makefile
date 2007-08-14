ALL:      logger.exe syslogd.exe syslog.lib

clean:
  -del *.exe
  -del *.obj
  -del *.lib

logger.exe: logger.obj getopt.obj syslog.lib
  ICC.EXE /Ti logger.obj getopt.obj syslog.lib tcp32dll.lib so32dll.lib 

syslogd.exe: syslogd.obj fix_env.obj getopt.obj
  ICC.EXE /Ti syslogd.obj fix_env.obj getopt.obj tcp32dll.lib so32dll.lib 

syslog.lib: syslog.obj
  -del syslog.lib
  lib syslog.lib +syslog.obj;

.c.obj:
   ICC.EXE /Ti /Gm /C   .\$*.c

logger.obj:   logger.c   syslog.h getopt.h
syslogd.obj:  syslogd.c  syslog.h getopt.h fix_env.h
syslog.obj:   syslog.c   syslog.h
getopt.obj:   getopt.c   getopt.h
getopt1.obj:  getopt1.c  getopt.h
fix_env.obj:  fix_env.c  fix_env.h