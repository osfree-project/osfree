/* Creates CONTROL.scr */

pkname = 'MMAudio Pak 'LINEIN('VERSION')

say 'package="'pkname'"'
say 'codepage=437'
say 'filelist="MASTER.DSK"'
say 'groupcount=3'
say 'munitcount=1'
say 'medianame="'pkname' Install"'
say
say '/*** Destination Directorys ***/'
say 'destindir = "\\MMOS2\\DLL\\"                           = 0'
say 'destindir = "\\MMOS2\\INSTALL\\"                       = 1'
say
say '/*** Groups ***/'
say 'ssgroup=3'
say 'sssize='trunc(stream('mmioFlac.dll','c','query size')/1024)+1
say 'ssname="MMIOFLAC"'
say 'ssversion="'LINEIN('mmioFlac/VERSION')'"'
say 'ssinich="MMIOFLAC.SCR"'
say 'ssselect="YES"'
say 'sstermdll="install.dll"'
say 'sstermdllentry="InstallExtension"'
say 'ssdllinputparms="FLA"'
say
say 'ssgroup=2'
say 'sssize='trunc(stream('mmioVorb.dll','c','query size')/1024)+1
say 'ssname="MMIOVorbis"'
say 'ssversion="'LINEIN('mmioVorbis/VERSION')'"'
say 'ssinich="MMIOVorb.SCR"'
say 'ssselect="YES"'
say 'sstermdll="install.dll"'
say 'sstermdllentry="InstallExtension"'
say 'ssdllinputparms="OGG"'
say
say 'ssgroup=1'
say 'sssize='trunc(stream('mmioMP3.dll','c','query size')/1024)+1
say 'ssname="MMIOMP3"'
say 'ssversion="'LINEIN('mmioMP3/VERSION')'"'
say 'ssinich="MMIOMP3.SCR"'
say 'ssselect="YES"'
say 'sstermdll="install.dll"'
say 'sstermdllentry="InstallExtension"'
say 'ssdllinputparms="MP3"'

