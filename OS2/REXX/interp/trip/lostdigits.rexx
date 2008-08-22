/*
 * This is a simple test program for all the different occurances of possible
 * LOSTDIGITS conditions of Regina.
 *
 * Every error message is writen to standard error.
 * One line is written to standard output on success.
 *
 * Return codes: 0 on success, 1 otherwise
 */
errors = 0

numeric digits 5
call doTest
numeric digits 4
call doTest
if errors = 0 then do
   say "test passed successfully"
   exit 0
   end
exit 1

doTest:
   signal on lostdigits name addlost
   b = 10001 + 1
   adddone:
   if digits() <= 4 then
      call error "LOSTDIGITS not raised in addition, but needs to"
   signal addend

   addlost:
   if digits() > 4 then
      call error "LOSTDIGITS raised in addition, but must not"

   addend:
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

   signal on lostdigits name sublost
   b = -10001 - 1
   subdone:
   if digits() <= 4 then
      call error "LOSTDIGITS not raised in subtraction, but needs to"
   signal subend

   sublost:
   if digits() > 4 then
      call error "LOSTDIGITS raised in subtraction, but must not"

   subend:
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

   signal on lostdigits name mullost
   b = 1.0001 * 2
   muldone:
   if digits() <= 4 then
      call error "LOSTDIGITS not raised in multiplication, but needs to"
   signal mulend

   mullost:
   if digits() > 4 then
      call error "LOSTDIGITS raised in multiplication, but must not"

   mulend:
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

   signal on lostdigits name fdivlost
   b = 1.0001 / 2
   fdivdone:
   if digits() <= 4 then
      call error "LOSTDIGITS not raised in decimal division, but needs to"
   signal fdivend

   fdivlost:
   if digits() > 4 then
      call error "LOSTDIGITS raised in decimal division, but must not"

   fdivend:
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

   signal on lostdigits name idivlost
   b = 1.0001 % 2
   idivdone:
   if digits() <= 4 then
      call error "LOSTDIGITS not raised in integer division, but needs to"
   signal idivend

   idivlost:
   if digits() > 4 then
      call error "LOSTDIGITS raised in integer division, but must not"

   idivend:
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

   signal on lostdigits name powlost
   b = 8.0001 ** -2
   powdone:
   if digits() <= 4 then
      call error "LOSTDIGITS not raised in power expression, but needs to"
   signal powend

   powlost:
   if digits() > 4 then
      call error "LOSTDIGITS raised in power expression, but must not"

   powend:

   return

error:
   call lineout "<stderr>", arg(1)
   errors = errors + 1
   return
