/* Example of regstemsearch -- loads a sorted file of words and searches
 * for a few common ones. This version is intended to work with
 * ordinary Unix words files, which are sorted case-insensitively.
 *
 * My typical results indicate that regstemsearch does not significantly
 * out-perform a binary search coded in Rexx, although it's usually
 * slightly better in the case-insensitive case. The linear search is
 * usually typically slightly slower than one coded in Rexx.
 *
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/dictcs.rex,v 1.3 2004/08/21 14:48:38 prokushev Exp $
 */

if rxfuncadd('regstemsearch', 'rexxutil', 'regstemsearch') > 0 then do
  say rxfuncerrmsg();
  exit 1;
  end

if rxfuncadd('regstemread', 'rexxutil', 'regstemread') > 0 then do
  say rxfuncerrmsg();
  exit 1;
  end

dictname = '/usr/share/dict/words'

/* start elapsed-time counter */
call time 'r'
call regstemread dictname, 'dict'
el = time('e')
say 'read' dict.0 'lines from' dictname 'in' el 'seconds'

call findwordsorted 'zymome'
call findwordsorted 'abacus'
call findwordsorted 'noodlepoodle'

call rxfindwordsorted 'zymome'
call rxfindwordsorted 'abacus'
call rxfindwordsorted 'noodlepoodle'

call findwordlinear 'zymome'
call findwordlinear 'abacus'
call findwordlinear 'noodlepoodle'

call rxfindwordlinear 'zymome'
call rxfindwordlinear 'abacus'
call rxfindwordlinear 'noodlepoodle'

exit 1


findwordsorted: procedure expose dict.
   parse arg word

   /* start elapsed-time counter */
   call time 'r'
   ord = regstemsearch(word, 'dict.',,'ES')
   el = time('e')

   if ord \= 0 then
      say 'found' word 'at position' ord '('dict.ord') in' el 'seconds'
   else
      say 'missed' word 'in' el 'seconds'

   return ord

rxfindwordsorted: procedure expose dict.
   parse upper arg word

   /* start elapsed-time counter */
   call time 'r'
   l = 1
   r = dict.0
   ord = (r - l) % 2 + l
   do while ord >= l & ord <= r
      do = translate(dict.ord)
      if word = do then leave
      else if word < do then
         r = ord - 1
      else if word > do then
         l = ord + 1
      ord = (r - l) % 2 + l
      end
   el = time('e')

   if l <= ord & r >= ord then
      say 'found' word 'at position' ord '('dict.ord') in' el 'seconds'
   else do
      say 'missed' word 'in' el 'seconds'
      ord = 0
      end

   return ord

findwordlinear: procedure expose dict.
   parse arg word

   /* start elapsed-time counter */
   call time 'r'
   ord = regstemsearch(word, 'DICT.',,'E')
   el = time('e')

   if ord \= 0 then
      say 'found' word 'at position' ord '('dict.ord') in' el 'seconds'
   else
      say 'missed' word 'in' el 'seconds'

   return ord

rxfindwordlinear: procedure expose dict.
   parse upper arg word

   /* start elapsed-time counter */
   call time 'r'
   do ord = 1 to dict.0 while word \= translate(dict.ord)
   end
   el = time('e')

   if ord <= dict.0 then
      say 'found' word 'at position' ord '('dict.ord') in' el 'seconds'
   else do
      say 'missed' word 'in' el 'seconds'
      ord = 0
      end

   return ord

