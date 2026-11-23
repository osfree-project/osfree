set em=def
call cmd /c _wcc.cmd
cd /d c:\som21nt\bin
cd
call a.cmd
copy D:\yuri\osfree\build\bin\host\win32\os2tk45\som\common\dll\emit%em%.dll %sombase%bin
sc -s%em% c:\som21nt\include\somobj.idl
cd /d c:\som30nt\bin
cd
call a.cmd
copy D:\yuri\osfree\build\bin\host\win32\os2tk45\som\common\dll\emit%em%.dll %sombase%bin
sc -s%em% c:\som30nt\include\somobj.idl
cd /d d:\yuri\osfree\som\host\emit%em%
