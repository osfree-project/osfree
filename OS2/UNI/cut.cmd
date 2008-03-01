/* Workaround of uni2h tool unimplemented features */

parse arg

f=arg(1)

do while lines(f)
  line = linein(f)
  if line='#ifdef INCL_CDEFTYPES' then
  do
    /* Skip #ifdef */
    line = linein(f)
    rc=lineout('tmp',line);
    /* Skip #endif */
    line = linein(f)
  end
  else
    rc=lineout('tmp',line);
end

rc=stream('tmp','c','close');
rc=stream(f,'c','close');
