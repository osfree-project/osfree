/* REXX script to sort OS/2 .DLG files */
/* (C) 2000 Paul Ratcliffe */
parse upper source . . me
call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs
arg opt
if opt = '' then do
  me = filespec('N', me)
  say
  say 'Usage:' left(me, lastpos('.', me) - 1) '<file.dlg> [file.dlg] [file.dlg] ...'
  return
end
if translate(right(opt, 4)) \= '.DLG' then
  opt = opt'.dlg'
call SysFileTree opt, 'stem', 'O'
do i = 1 to stem.0
  if i \= 1 then
    say
  say 'Processing "'stem.i'"'
  call processdlg stem.i
end
return

processdlg: procedure expose os
  arg fileout
  fileout = translate(fileout, "abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  say fileout
  filein = left(fileout, length(fileout)-1)'U'
  call SysFileDelete filein
  '@ren' fileout '*.dlu'
  dialog.0 = 0
  do while lines(filein) \= 0
    line = linein(filein)
    sline = striptab(line)
    parse upper var sline key1 key2 key3 .
    if key1 = 'DLGTEMPLATE' then do
      dialog.0 = dialog.0 + 1;
      i = dialog.0
      dialog.i.num = key2
      j = 1
      dialog.i.j = line
      do while lines(filein) \= 0
    line = linein(filein)
    sline = striptab(line)
    j = j + 1
    dialog.i.j = line
    parse upper var sline key1 .
    if length(sline) = 3 & sline = 'END' then
      leave
      end
      dialog.i.0 = j
    end; else if dialog.0 = 0 then
      call lineout fileout, line
  end
  say '  Dialogs:' dialog.0
  if dialog.0 \= 0 then do
    stem.0 = dialog.0
    do i = 1 to stem.0
      stem.i.index = i
      stem.i = dialog.i.num
    end
    call heapsort
    do i = 1 to stem.0
      k = stem.i.index
      say '   ' dialog.k.num
      do j = 1 to dialog.k.0
    call lineout fileout, dialog.k.j
      end
      call lineout fileout, ''
    end
  end
  call stream filein, 'C', 'CLOSE'
  call stream fileout, 'C', 'CLOSE'
  return

striptab: procedure
  arg line
  do forever
    i = pos(d2c(9), line)
    if i = 0 then
      leave
    line = overlay(' ', line, i)
  end
  return line

heapsort: procedure expose stem.
  n = stem.0
  do i = n % 2 to 1 by -1
    call downheap i n
  end
  do while n > 1
    tmp = stem.1.index
    stem.1.index = stem.n.index
    stem.n.index = tmp
    tmp = stem.1
    stem.1 = stem.n
    stem.n = tmp
    n = n - 1
    call downheap 1 n
  end
  return

downheap: procedure expose stem.
  parse arg k n
  v = stem.k
  w = stem.k.index
  do while k <= n % 2
    j = k + k
    if j < n then do
      i = j + 1
      if stem.j < stem.i then
        j = j + 1
    end
    if v >= stem.j then
      signal label
    stem.k = stem.j
    stem.k.index = stem.j.index
    k = j
  end
label:
  stem.k = v
  stem.k.index = w
  return
