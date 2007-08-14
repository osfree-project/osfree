/* semaphore test.
 * this is an alternative client to the semtest.rex server.
 * this one passes its argument over and over
 *
 * to execute, run once with arguments count and text, eg
 *  regina sembulk 10000 test a
 */

parse arg u ment

datafile = 'semtest.dat'

call rxfuncadd 'sysloadfuncs','rexxutil','sysloadfuncs'
call sysloadfuncs

  esem = sysopeneventsem('semtest.event')
  if esem = '' then do
    say "client couldn't open event semaphore!"
    exit 1
  end

  mux = sysopenmutexsem('semtest.mux')
  if mux = '' then do
    say "client couldn't open mutex semaphore!"
    exit 1
  end

do i = 1 to u
  rcc = sysrequestmutexsem(mux)

  if rcc = 0 then do
    call lineout datafile, ument i
    call stream datafile, 'c', 'close'
    rcc = sysreleasemutexsem(mux)
    call syspulseeventsem esem
    end
  end
