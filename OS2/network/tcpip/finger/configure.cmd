/* */
'@type make.in > make.cmd'
'@sed -V 2> \dev\nul'
if rc \== 0 then
do
'cls'
say ""
say "Ouch!"
say ""
say " Since you haven't got sed 2.05 or thereabouts installed, you have to define"
say " FINGER_PATH in sf.h with the name of the directory in which finger.exe "
say " resides. Note that the pathname must include double backslashes and"
say  " be enclosed in quotes, e.g.      ""PATH=c:\\bin"""
'@type sf\sf.h.in > sf\sf.h'
'@start e sf\sf.h'
say ""
'@type makens.in >> make.cmd'
end
else
do
say ""
say "Phew. Sed exists."
'@type makesf.in >> make.cmd'
end
say "Type ""make"" to continue."
