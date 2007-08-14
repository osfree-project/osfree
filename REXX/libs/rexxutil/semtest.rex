/* semaphore test.
 * this is a sort-of client/server thing, where the server sits in a loop
 * waiting for events, and the client writes them to a file and signals
 *
 * to execute, run once with the argument `server' to start the server, and then
 * with any other argument to start the client.
 * to shut down the server, start a client with `regina semtest quit'
 * you should be able to start several servers concurrently.
 */

parse arg ument

datafile = 'semtest.dat'

call rxfuncadd 'sysloadfuncs','rexxutil','sysloadfuncs'
call sysloadfuncs

if ument = 'server' then do
  esem = syscreateeventsem('semtest.event')
  if esem = '' then do
    say "server couldn't create event semaphore!"
    exit 1
  end

  mux = syscreatemutexsem('semtest.mux')
  if mux = '' then do
    say "server couldn't create mutex semaphore!"
    exit 1
  end

  /* now wait for things to do */
  do forever
    rcc = syswaiteventsem(esem)
    if rcc = 0 then do
      rcc = sysrequestmutexsem(mux)

      do while lines(datafile) > 0
         val = linein(datafile)
         say val
         end
      call stream datafile, 'c', 'close'
      call sysfiledelete datafile

      rcc = sysreleasemutexsem(mux)
      end

    if val = 'quit' then leave
    end
  end

else do
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

  rcc = sysrequestmutexsem(mux)

  if rcc = 0 then do
    call lineout datafile, ument
    call stream datafile, 'c', 'close'
    rcc = sysreleasemutexsem(mux)
    call syspulseeventsem esem
    end
  end
