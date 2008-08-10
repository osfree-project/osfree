/*
 *   map2sym.cmd -- a script to convert a linker map file to Bochs
 *   debugger symbols file.
 *   (must be rewritten in AWK). 
 *   (c) valerius, 2006/10/01.
 */

 parse arg map base opt

 if map = '' then do
   call usage
   exit -1
 end

 if base = '' then
    base = 0

 if opt  = '-com' then
    org  = x2d(100)

 if pos('0x', base) then do
    parse var base '0x' base
    base = x2d(base)
 end
 else
 if pos('0X', base) then do
    parse var base '0X' base
    base = x2d(base)
 end

 flag = 0
 do while lines(map)

   if \flag then
      line = linein(map)
   else
     flag = \flag

   parse var line first .

   if first = 'Module:' then
   do while lines(map)
     flag = 1
     line = linein(map)
     if line = '' then do
       flag = 0
       leave
     end
     parse var line addr symbol
     if addr = 'Module:'
        then leave
     parse var addr segment ':' offset
     offset = strip(offset, 'T', '+')
     offset = strip(offset, 'T', '*')

     segment = base + segment - x2d(10)
     offset  = x2d(offset)

     segment = pad(d2x(segment), 4)
     offset  = pad(d2x(offset), 4)

     addr = '0x'segment':0x'offset

     say  addr symbol

   end

 end

 exit 0
/* ------------------------------------------------------------- */
pad: procedure
s = arg(1)
n = arg(2)

do while length(s) < n
  s = '0's
end


return s
/* ------------------------------------------------------------- */
usage:

say 'Watcom linker map files to Bochs debugger symbol'
say 'file converter.'
say 'usage:'
say
say 'map2sym <map file> <base segment address> > symbol file'
say


return
/* ------------------------------------------------------------- */
