Parse Arg _out _name _ver _mach _desc
start = .LINE + 1
/*
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
<assemblyIdentity
  version="%VER%.0"
  processorArchitecture="%MACH%"
  name="%NAME%"
  type="win32"
/>
<description>%DESC%</description>
<compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
  <application>
    <!-- Windows Vista -->
    <supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}"/>
    <!-- Windows 7 -->
    <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
    <!-- Windows 8 -->
    <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
    <!-- Windows 8.1 -->
    <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
    <!-- Windows 10 -->
    <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
  </application>
</compatibility>
</assembly>
*/
end = .LINE - 2
Call Stream _out, 'C', 'OPEN WRITE REPLACE'
Do i = start To end
   line = Sourceline(i)
   Select
      When Countstr( '%NAME%', line ) \= 0 Then line = Changestr( '%NAME%', line, _name )
      When Countstr( '%VER%', line ) \= 0 Then line = Changestr( '%VER%', line, _ver )
      When Countstr( '%MACH%', line ) \= 0 Then line = Changestr( '%MACH%', line, _mach )
      When Countstr( '%DESC%', line ) \= 0 Then line = Changestr( '%DESC%', line, _desc )
      Otherwise Nop
   End
   Call Lineout _out,line
End

Call Stream _out, 'C', 'CLOSE'
