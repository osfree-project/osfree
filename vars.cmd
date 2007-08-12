#!/usr/local/bin/rexx
#
#
/* rexx */

parse arg file

if file = '' then do
   say 'You must specify ' || file || ' file!'
   exit -1
end

call process_file file

do i = 1 to words(vars)
  wd = word(vars, i)
  say wd || ' = ' || vars.wd
end

say vars


exit 0
/* ================================= */
process_file: procedure expose vars. vars
file = arg(1)

/*
if stream(file, 'c', 'query exists') = '' then do
  say 'File ' || file || ' doesn''t exist!'
  exit -1
end
*/

rc = stream(file, 'c', 'open read')

vars = ''
do while lines(file)
  line = linein(file)
  p = pos('#', line)
  if p > 0 then line = delstr(line, p)
  if line = '' then iterate
  if pos('include', line) = 1 then do
    parse var line 'include' file1
    file = strip(file)
    call process_file file1
    iterate
  end
  if pos('=', line) <= 0 then iterate 
  parse var line variable '=' val
  val = strip(val)
  if pos('$(', val) > 0 then
    val = substitute(val)
  variable = translate(variable)    
  vars.variable = val
  vars = vars || variable || ' '
end

rc = stream(file, 'c', 'close')


return
/* ================================= */
substitute: procedure expose vars. vars
val = arg(1)

p = 1
do while p > 0
  parse var val first '$(' var ')' last
  var = translate(var)
  v = vars.var
  val = first || v || last
  p = pos('$(', val)
end


return val
/* ================================= */
