/* Convert a binary file to hexdump */

say '/* Binary files included: */'
say ''
say '/* boot sector */'
say 'char bootsec[] = {'
call bin2h 'boot\sectors\bootsect_1.bin'
say '};'
say ''

say '/* preldr_mini */'
say 'char preldr_mini[] = {'
call bin2h 'boot\loader\preldr_mini.mdl'
say '};'
say ''

say '/* preldr0 */'
say 'char preldr0_mdl[] = {'
call bin2h 'boot\loader\preldr0.mdl'
say '};'
say ''
say '/* preldr0 relocations */'
say 'char preldr0_rel[] = {'
call bin2h 'boot\loader\preldr0.rel'
say '};'
say ''

say '/* fat */'
say 'char fat_mdl[] = {'
call bin2h 'boot\loader\fsd\fat.mdl'
say '};'
say ''
say '/* fat relocations */'
say 'char fat_rel[] = {'
call bin2h 'boot\loader\fsd\fat.rel'
say '};'
say ''

exit
/* -------------------------------- */
bin2h: procedure
file = arg(1)

call stream file, 'c', 'open read'

str = ''
i = 0
do while chars(file) > 0
  ch = charin(file)
  i = i + 1
  ch = c2x(ch)
  str = str || '0x' || ch || ','
  if i >= 20 then do
    say str
    str = ''
    i = 0
  end; else do
    if chars(file) = 0 then
      say str
  end
end

call stream file, 'c', 'close'

return
/* -------------------------------- */
