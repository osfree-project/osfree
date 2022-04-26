/* */

parse arg fin

call stream fin, 'c', 'open read'

SECTOR = CHARIN(fin, 1, chars(fin))

say '/* Automatically generated file. Don''t edit it manually */'
say '#define MBR_LENGTH '||length(SECTOR)
say 'BYTE mbr[MBR_LENGTH]= {'
do i=1 to length(SECTOR)
  if i><length(SECTOR) then say '0x'||c2x(substr(SECTOR, i, 1))||','
  if i==length(SECTOR) then say '0x'||c2x(substr(SECTOR, i, 1))
end

call stream fin, 'c', 'close'

say '};'
