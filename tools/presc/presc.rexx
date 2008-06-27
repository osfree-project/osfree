/*  This is SOM IDL precompiler which produces C header files required to build real SOM Compiler */

Say 'SOM IDL precompiler'
Say ''

if arg(1)='' then
do
  Say 'No file to process'
  return -1
end

f=arg(1)
TempFile=F||'.tmp'
TempFile2=F||'.tmp.tmp'
/* '@if exists '||TempFile||' @del '||TempFile */

/* Delete all comments and leading spaces first */

do while lines(f)>0
  s=linein(f)
  parse value s with start '//' skip
  rc=lineout(TempFile, strip(start))
end

rc=stream(f,'c','close')
rc=stream(TempFile,'c','close')

/* Preprocess */
do while lines(TempFile)>0

  s=linein(TempFile)

  if pos('/*', s)>0 then
  do

    parse value s with start '/*' skip
    xend = ''
    s=skip

    if pos('*/', s) >0 then 
    do
      parse value s with skip '*/' xend
      if length(start||xend)>0 then rc=lineout(TempFile2, start||xend)
      s=''
    end
    else
    do
      if length(start)>0 then rc=lineout(TempFile2, start)

      do while lines(TempFile)>0
        s=linein(TempFile)
        if pos('*/', s) >0 then 
        do
          parse value s with skip '*/' end
          if length(end)>0 then rc=lineout(TempFile2, end)
          s=''
          leave
        end
      end
    end

  end

  if length(s)>0 then rc=lineout(TempFile2, s)
end

return

rc=stream(TempFile,'c','close')
rc=stream(TempFile2,'c','close')
