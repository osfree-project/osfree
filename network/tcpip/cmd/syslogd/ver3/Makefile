CC = gcc
CFLAGS = -Wall -O2 -DTCPIPV3

all:	logger.exe syslogd.exe syslog.a syslog.lib

clean:
	rm -f *.exe *.o *.a *.lib

logger.exe: logger.o syslog.a
	$(CC) -Zcrtdll -o logger.exe logger.o syslog.a -lsocket

syslogd.exe: syslogd.o fix_env.o
	$(CC) -Zcrtdll -o syslogd.exe syslogd.o fix_env.o -lsocket

syslog.a: syslog.o
	ar cr syslog.a syslog.o

syslog.lib: syslog.a
	emxomf syslog.a

logger.o:   logger.c   syslog.h 

syslogd.o:  syslogd.c  syslog.h  fix_env.h

syslog.o:   syslog.c   syslog.h
	$(CC) $(CFLAGS) -D__ST_MT_ERRNO__ -o syslog.o -c syslog.c

fix_env.o:  fix_env.c  fix_env.h
