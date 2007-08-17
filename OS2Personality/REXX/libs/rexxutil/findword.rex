/* Comparison of sysfilesearch and regstemsearch -- loads the documentation
 * and searches for a few common words.
 *
 * My typical results indicate that regstemsearch is much the slowest
 * way to do the search. This might be my fault, but I think it points
 * to performance problems in the Regina SAA API.
 *
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/findword.rex,v 1.3 2004/08/21 14:48:38 prokushev Exp $
 */

if rxfuncadd('regstemsearch', 'rexxutil', 'regstemsearch') > 0 then do
  say rxfuncerrmsg();
  exit 1;
  end

if rxfuncadd('sysfilesearch', 'rexxutil', 'sysfilesearch') > 0 then do
  say rxfuncerrmsg();
  exit 1;
  end

if rxfuncadd('regstemread', 'rexxutil', 'regstemread') > 0 then do
  say rxfuncerrmsg();
  exit 1;
  end

filename = 'regutil.tex'

/* start elapsed-time counter */
call time 'r'
call regstemread filename, 'docs'
el = time('e')
say 'read' docs.0 'lines from' filename 'in' el 'seconds'

call findwordlinear 'matching'
call findwordlinear '\index'
call findwordlinear 'noodlepoodle'

call rxfindwordlinear 'matching'
call rxfindwordlinear '\index'
call rxfindwordlinear 'noodlepoodle'

call findwordfile 'matching'
call findwordfile '\index'
call findwordfile 'noodlepoodle'

exit 1

findwordfile: procedure expose results. filename
  parse arg word

  call time 'r'
  rc = sysfilesearch(word, filename, 'results.')
  el = time('e')

  say 'found' results.0 'instances of' word 'in' el 'seconds'
/*
  do i = 1 to results.0
    say results.i
    end
*/
  return results.0  

findwordlinear: procedure expose docs. results.

  parse arg word

  call time 'r'
  count = 0
  ord = 0
  do until ord = 0
    ord = regstemsearch(word, 'docs.',ord+1,'C')
    if ord \= 0 then do
      count = count + 1
      results.count = docs.ord
      end
    end
  results.0 = count
  el = time('e')

  say 'found' count 'instances of' word 'in' el 'seconds'
/*
  do i = 1 to count
    say results.i
    end
*/
  return count

rxfindwordlinear: procedure expose docs. results.
  parse arg word

  call time 'r'
  count = 0
  ord = 0
  do ord = 1 to docs.0
    if index(docs.ord, word) > 0 then do
      count = count + 1
      results.count = docs.ord
      end
    end

  results.0 = count
  el = time('e')

  say 'found' count 'instances of' word 'in' el 'seconds'
/*
  do i = 1 to count
    say results.i
    end
*/
  return count

