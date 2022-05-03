/*  Cut a piece from file
 *
 */

parse arg from howmany file

/* convert from hex to decimal, if needed */
if pos('0x', from) = 1    then from    = x2d(delstr(from, 1, 2))
if pos('0x', howmany) = 1 then howmany = x2d(delstr(howmany, 1, 2))

size = stream(file, 'c', 'query size')

from = from + 1

if from > 1 then do
  t = charin(file, 1, from)
  call charout stdout, t
end

t = charin(file, from + howmany, size - from - howmany + 1)
call charout stdout, t
