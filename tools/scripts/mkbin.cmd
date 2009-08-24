/*
 *   mkbin.cmd
 *   A script to convert DOS EXE file with no relocations
 *   to raw binary image.
 *   Used to create bootsector image
 *   and COM files.
 *   (c) osFree project,
 *   valerius, 2006/10/03
 */

  parse arg exefile bsfile

  /* Some EXE Header fields offsets */

  NumRelocsOffset  = 6       /* Number of relocation Items offset             */
  HdrSizeOffset    = 8       /* EXE header size offset                        */
  FirstRelocOffset = x2d(18) /* 1st stub program reloc item (=40h, if NewExe) */

  if exefile = '' | bsfile = '' then do
    call givehelp
    exit -1
  end

  p = lastpos('.EXE', translate(exefile))

  if p <= 0 then do
    say 'The file has no .EXE extension!'
    exit -1
  end

  rc = stream(exefile, 'c', 'query exists')

  if rc = '' then do
     say exefile' doesn''t exist!'
     exit -1
  end

  rc = stream(exefile, 'c', 'open read')

  magic     = charin(exefile, 1,  2)

  if magic \= 'MZ' then do
    say 'This is not a DOS EXE file!'
    exit -1
  end

  FirstReloc = charin(exefile, FirstRelocOffset + 1, 1)
  FirstReloc = c2x(FirstReloc)

  if FirstReloc = 40 then do
    say 'This is a NewEXE file.'
    say 'A file must be a DOS EXE!'
    exit -1
  end

  NumRelocs = charin(exefile, NumRelocsOffset + 1, 2)
  NumRelocs = c2d(NumRelocs)

  if NumRelocs \= 0 then do
    say 'Number of relocations is not 0!'
    exit -1
  end

  size = stream(exefile, 'c', 'query size')

  HdrSize = charin(exefile, HdrSizeOffset, 2)
  HdrSize = c2d(HdrSize) * 16

  /* Size of file minus size of EXE header */
  count = size - HdrSize

  if d2x(HdrSize) \= FirstReloc then do
    say 'A binary data in EXE does not begin'
    say 'right after its header!'
    exit -1
  end

  bs = charin(exefile, HdrSize + 1, count)

  rc = stream(bsfile, 'c', 'query exists')

  if rc \= '' then
    '@del 'bsfile' 2>&1 >\dev\nul'

  rc = stream(bsfile,  'c', 'open write')
  call charout bsfile, bs
  rc = stream(bsfile,  'c', 'close')

  rc = stream(exefile, 'c', 'close')


exit 0
/* --------------------------------------------- */
givehelp:

 say 'Usage:'
 say 'mkbin <exe file> <bin file>'
 say


return
/* --------------------------------------------- */
