/*--------------------------------------------------------------------¿
  ³ This REXX program shows that  virtual memory  is  exhausted  with: ³
  ³                                                                    ³
  ³              Regina 3.7                 just past J = 17,300   and ³
  ³              Regina 3.6                 just past J = 17,200   and ³
  ³              Regina 3.5  and  3.4       just past J = 17,400   and ³
  ³              Regina 3.3                 just past J = 73,700.      ³
  ³                                                                    ³
  ³ What this program does is emulate a much larger REXX program that  ³
  ³ reads in many files,  each file contains a number between  one and ³
  ³ one billion.    Each number found is  indicated  by placing a      ³
  ³ non-blank character at the appropriate  offset  from each  segment ³
  ³ (whose length can be  up to  and  including  segSize).             ³
  ³                                                                    ³
  ³ Example:  for the number  4032,    the  4032nd  character in the   ³
  ³           G.0  segment would contain  (say)  a  "1".               ³
  ³                                                                    ³
  ³           For the number 57000,    the  7000th  character in the   ³
  ³           G.5  segment would contain  (say)  a  "1".               ³
  ³                                                                    ³
  ³ The segment number is calculated thusly: seg# = number % segSize   ³
  ³  "    "     offset  "     "         "    segO = number//segSize +1 ³
  ³                                                                    ³
  ³ This program doesn't do any reading,  as the input files are too   ³
  ³ large to upload and would take to much time for REXX to read them. ³
  ³                                                                    ³
  ³ So this REXX program constructs  100,000  fully-populated segment  ³
  ³ records, and then   tries   to consolidate them into  100  segment ³
  ³ records,  DROPping each old segment record as it's processed.      ³
  ³                                                                    ³
  ³ The reason that the large REXX program just doesn't populate the   ³
  ³ large segment records is that is takes to long for REXX to keep    ³
  ³ rebuilding the segment records  using the  OVERLAY  BIF  to change ³
  ³ the  4032nd  and  7000th   character   (as in the above examples). ³
  ³                                                                    ³
  ³ It should be noted that Regina 3.3 sucessfully reads in the entire ³
  ³ set of files  (and subsequently processes them),  Regina 3.4, 3.5, ³
  ³ 3.6, and 3.7   exhaust system recources   while reading the files. ³
  ³                                                                    ³
  ³ Also, it should be noted that the large REXX program doesn't do    ³
  ³ any consolidation,  that's just a mechanism that this program uses ³
  ³ to show (possibly)  how/why the large REXX program is failing with ³
  ³ Regina releases  3.4  -->  3.7  (inclusive).                       ³
  --------------------------------------------------------------------*/

g.=                                    /*the   original   stemmed array.*/
gg.=                                   /*the consolidated    "      "   */
segSize = 10000                        /*size of the segments.          */
times   = 1000000000 % segSize         /*that's  one  billion (before %)*/
parse version v
say 'segSize=' segSize  '  times='times '  REXX version:' v
dummyRec = copies('fa'x, segSize)      /*a stand-in thingy dummy record.*/

    /*------------------------------------------------------------------*/
call time 'R'
                  do i=0  for times    /*generate  100,000  of these.   */
                  g.i = dummyRec       /*generate pseudo segment record.*/
                  end   /*i*/

say '   generating' i "segments took" format(time('E'),,2) "seconds."

    /*------------------------------------------------------------------*/
call time 'R'
                  do j=0  for i        /*process each pseudo segment rec*/
                  cseg = j%100         /*calculate new consolidated seg#*/
if j//100==0 then say 'using ' j  '   and cseg: ' cseg    /*show & tell.*/
                  gg.cseg = gg.cseg || g.j                /*consolidate,*/
                  drop g.j                                /*save memory.*/
                  end  /*j*/

say 'consolidation' i%100 "segments took" format(time('E'),,2) "seconds."
segSize = segSize*100                  /*the new segment size.          */
