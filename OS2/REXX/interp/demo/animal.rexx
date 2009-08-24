/* Written by Stig Hemmer <stig@pvv.unit.no> */

parse arg infile .
if infile\='' then 
   do while lines(infile)>0
      interpret linein(infile) ;
      end
else
   node = 'an elephant'

do until left(answer,1) = 'N'
   where = 'NODE'
   finished = 0
   do while finished = 0
      if symbol(where'.Y')='VAR' then do
         say value(where)
         pull answer
         if left(space(answer),1) = 'N' then
            where = where'.N'
         else
            where = where'.Y'
      end; else do
         finished = 1;
         say 'Did you think of 'value(where)'?'
         pull answer
         if left(answer,1) = 'Y' then
            say 'Good!'
         else do
            say 'Hmmm, what did you think of?'
            parse pull newanimal
            say 'Give me a question that separates between 'newanimal' from',
                value(where)':'
            parse pull quest
            say 'And what is the answer for 'newanimal'?'
            pull answer
            answer = space(answer)
            if left(answer,1) = 'Y' then do
               interpret where'.N =' where
               interpret where'.Y = newanimal'
               interpret where '= quest'
            end; else do
               interpret where'.Y =' where
               interpret where'.N = newanimal'
               interpret where '= quest'
            end
         end
      end
   end
   say 'Another play?'
   pull answer
   answer = space(answer)
end 

say 'File to save in (leave empty if you don''t want to save)?'
parse pull filename
if filename\='' then do
   call lineout filename,, 1
   signal on notready
   call dump 'NODE' 
   end

exit 0

dump: procedure expose filename node node.
   parse arg arg 
   if symbol(arg)='VAR' then do
      call lineout filename, arg'="'insulate(value(arg))'"' 
      call dump arg'.Y'
      call dump arg'.N'
      end
   return ;

insulate:
   parse arg string
   pos = -1
   do until pos=0
      pos = pos('"', string, pos+2)
      if (pos>0) then
         string = left(string,pos-1) || '""' || substr(string,pos+1)
      end
   return string

notready:
   say 'error while dumping to file; dumping terminated'
   exit 1

