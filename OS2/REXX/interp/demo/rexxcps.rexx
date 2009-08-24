/* ReXX */
  /* Take a measure of REXX clauses-per-second (CPS)                 */
  /* Mike Cowlishaw (mfc@ibm.com).  Multi-platform.                  */
  /* 1.0 17 Jan 89 Original version                                  */
  /* 2.0  3 Jun 89 Remove attempt to simulate commands               */
  /* 2.1  7 Oct 89 Remove use of not symbols, and correct commentary */
  /* 2.2 31 Mar 00 Allow measures/iterations/tracevar to be passed on*/
  /*               command line. mhes                                */
  rexxcps=2.2    /* REXXCPS version; quotable only if code unchanged */

  /* This measure of REXX execution speed is based on an analysis of */
  /* the dynamic mix of clauses found in a (subjectively chosen)     */
  /* collection of REXX programs (including commands, macros, and    */
  /* personal programs).  Approximately 2,500,000 lines of trace     */
  /* output were analysed, and the program below recreates the       */
  /* dynamic mix of constructs found in that analysis.               */
  /* In view of the dramatic differences between systems in their    */
  /* efficiency of issuing commands, the timed loop does not issue   */
  /* commands (an 'RC=expression; PARSE' sequence is used instead).  */
  /* This program therefore measures the performance of a REXX       */
  /* implementation, exclusive of command execution overhead.        */
  /* Elapsed (user-perceived) time is used, rather than any form of  */
  /* virtual time.                                                   */

  Parse Arg averaging count tracevar
  if averaging = '' Then averaging = 10  /* Averaging-over count */
  if count = '' Then count = 30  /* Repetition count */
  if tracevar = '' then tracevar = 'Off' /* Trace setting (for development use) */
  signal on novalue
  parse source  source  1 system .
  parse version version


  say '----- REXXCPS' rexxcps '-- Measuring REXX clauses/second -----'
  say ' REXX version is:' version
  say '       System is:' system
  say '       Averaging:' averaging 'measures of' count 'iterations'

  /* ----- Calibrate for the empty do-loop (average of 5) ----- */
  empty=0
  do i=1 to averaging
    call time 'R'
    do count; end
    empty=time('R')+empty
    end
  empty=empty/averaging

  noterm=(system='CMS'); if pos('O',tracevar)=1 then noterm=0
  if noterm then do
     say 'Calibration (empty DO):' empty 'secs (average of' averaging')'
     say 'Spooling trace NOTERM'
     'CP SPOOL CON * START NOTERM'; 'CP CLOSE CON PUR'
     end 

  /* Now the true timer loop .. average timing again */
  full=0 
  do i=1 to averaging
    trace value tracevar 
    call time 'R'
    do count;
      /* -----  This is first of the 1000 clauses ----- */
      flag=0; p0='b'
      do loop=1 to 14
        /* This is the "block" comment in loop */
        key1='Key Bee'
        acompound.key1.loop=substr(1234"5678",6,2)
        if flag=acompound.key1.loop then say 'Failed1'
        do j=1.1 to 2.2 by 1.1   /* executed 28 times */
          if j>acompound.key1.loop then say 'Failed2'
          if 17<length(j)-1        then say 'Failed3'
          if j='foobar'            then say 'Failed4'
          if substr(1234,1,1)=9    then say 'Failed5'
          if word(key1,1)='?'      then say 'Failed6'
          if j<5 then do   /* This path taken */
            acompound.key1.loop=acompound.key1.loop+1
            if j=2 then leave
            end
          iterate
          end /* j */
        avar.=1.0''loop
        select
          when flag='string' then say 'FailedS1'
          when avar.flag.2=0 then say 'FailedS2'
          when flag=5+99.7   then say 'FailedS3'
          when flag          then avar.1.2=avar.1.2*1.1
          when flag==0       then flag=0
          end
        if 1 then flag=1
        select
          when flag=='ring'  then say 'FailedT1'
          when avar.flag.3=0 then say 'FailedT2'
          when flag          then avar.1.2=avar.1.2*1.1
          when flag==0       then flag=1
          end
        parse value 'Foo Bar' with v1 +5 v2 .
        trace value trace(); address value address()
        call subroutine 'with' 2 'args', '(This is the second)'1''1
        rc='This is an awfully boring program'; parse var rc p1 (p0) p5
        rc='is an awfully boring program This'; parse var rc p2 (p0) p6
        rc='an awfully boring program This is'; parse var rc p3 (p0) p7
        rc='awfully boring program This is an'; parse var rc p4 (p0) p8
        end loop
      /* -----  This is last of the 1000 clauses ----- */
      end
    full=time('R')+full
    trace off
    end
  full=full/averaging
  if noterm then do
     'CP CLOSE CON'; 'CP SPOOL CON * START TERM'
     say 'Spooling now back on TERM'
     end 

  /* Now display the statistics */
  looptime=(full-empty)/count

  /* Developer's statistics: */
  if left(tracevar,1)='O' then nop; else do
    say
    say 'Total (full DO):' full-empty 'secs (average of' averaging ,
      'measures of' count 'iterations)'
    say 'Time for one iteration (1000 clauses) was:' looptime 'seconds'
    end
  /* And finally, the Result... */

  if looptime = 0 Then do
     say '     The granularity of the system clock appears to be too coarse to'
     say '     obtain an effective result.  Re-run this progam and increase the'
     say '     number of iterations or the repeat count.'
     end
  else do
     say
     say'     Performance:' format(1000/looptime,,0) 'REXX clauses per second'
     say
     end

  exit


  /* Target routine for the timed CALL - called 14 times */
  subroutine:
    parse upper arg a1 a2 a3 ., a4
    parse var a3 b1 b2 b3 .
    do 1; rc=a1 a2 a3; parse var rc c1 c2 c3; end
    return

  novalue: if noterm then 'CP SP CON STOP TERM'
    say 'novalue triggered'
  /* -- end of program -- */
