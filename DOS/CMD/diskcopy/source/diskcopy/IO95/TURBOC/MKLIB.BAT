tcc -v -y -vi -c *.c >errlist
del t.obj
lib /c io95 -+ .\* , io95
