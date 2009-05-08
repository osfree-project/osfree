/*  make assembler header from binary file
 *  for inclusion into another binary
 */

parse arg binary header
parse source os .

/* a number of byter in single 'db' string */
chunksize = 20

if os = 'OS/2'       then del = 'del '
else if os = 'WIN32' then del = 'del '
else del = 'rm -f '

if stream(header, 'c', 'query exist') \= '' then del || header

if binary = '' then do
  say 'You must supply a valid binary file and output file!'
  exit -1
end

rc   = stream(binary, 'c', 'query exist')

if rc = '' then do
  say 'You must supply a valid binary file!'
  exit -1
end

size = stream(binary, 'c', 'query size')

if size = 0 then do
  say 'You''re supplied an empty file!'
  exit -2
end

do i = 1 to size / chunksize + 1
  chunk = charin(binary, chunksize * (i - 1) + 1, chunksize)
  if length(chunk) > 0 then
     call charout header, '  db  '
  do j = 1 to length(chunk) - 1
    ch = substr(chunk, j, 1)
    call charout header, '0x' || c2x(ch) || ','
  end
  ch = substr(chunk, length(chunk), 1)
  call charout header, '0x' || c2x(ch) || '0d 0a'x
end
