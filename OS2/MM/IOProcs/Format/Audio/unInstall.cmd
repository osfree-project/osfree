/* MMIOMP3, MMIOVorbis, and MMIOFlac Uninstaller */

Say "Do you want to uninstall the MP3 IOProc (y/N)?"
Pull answer

IF (answer="y") | (answer="Y") | (answer="yes") | (answer="Yes") | (answer="YES") 
THEN
'@unInstallIOProc MP3 MP3'

Say "Do you want to uninstall the Vorbis IOProc (y/N)?"
Pull answer

IF (answer="y") | (answer="Y") | (answer="yes") | (answer="Yes") | (answer="YES") 
THEN
'@unInstallIOProc OGGS OGG'

Say "Do you want to uninstall the FLAC IOProc (y/N)?"
Pull answer

IF (answer="y") | (answer="Y") | (answer="yes") | (answer="Yes") | (answer="YES") 
THEN
'@unInstallIOProc FLAC FLA'
Say "Restart WPS for any changes to take effect"
