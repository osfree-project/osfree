/*  Set environment variables */

parse arg cfg

cfg = strip(cfg, 'B')

parse source os addr src

if cfg = '' then signal usage
src = translate(src, 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')

/* Build the full path to the config file */
if pos('/', translate(cfg, '/', '\')) == 0 then do
   l = lastpos('/', translate(src, '/', '\'))
   if l > 0 then cfg = delstr(src, l + 1) || cfg
end

ll = lastpos('\', src)
root = delstr(src, ll)

say root

ll = lastpos('\', root)
if ll > 0 then root = delstr(root, ll)

say root

ll = lastpos('\', root)
if ll > 0 then root = delstr(root, ll + 1)

say root

if stream(cfg, 'c', 'query exists') = '' then signal usage


/* Read config file */
rc = stream(cfg, 'c', 'open read')

if rc \= 'READY:' then do
  say 'Can''t open file ' || cfg || '!'
  exit 1
end

do while lines(cfg) > 0
  line = linein(cfg)
  /* strip comments */
  p = pos(';', line)
  if p > 0 then line = delstr(line, p)
  line = strip(line)
  if line = '' then iterate
  interpret line
end

call stream cfg, 'c', 'close'

env = 'ENVIRONMENT'

if pos(':', imgdir) == 2 then
  imgdir = delstr(imgdir, 1, 2)
imgdir1 = translate(imgdir, '/', '\')

say os

if os == 'OS/2' then do
  os = 'OS2'
  wosdir = '\binp'
end; else
if os == 'WINDOWS' | os == 'WINNT' | os == 'WIN32' then do
    wosdir = '\binnt'
end; else
if os == 'UNIX' | os == 'LINUX' then do
  wosdir = '\binl'
end; else do
  say 'Unsupported OS!'
  exit 2
end

/* delete a driveletter from root */
/**! if pos(':', root) = 2 then root = substr(root, 3) */

/* change a drive letter from small to big letter */
if pos(':', root) = 2 then do
  drvlttr = substr(root, 1, 1)
  drvlttr = translate(drvlttr)
  root = drvlttr || substr(root, 2)
end

if os == 'UNIX' | os == 'LINUX' then do
  root = translate(root, '/', '\')
end

/* append '\' */
if lastpos('\', root) \= length(root) then root = root || '\'

host = 'os2'
tools = root || 'build\bin\host\' || host || '\bin'
os2tk = root || 'build\bin\host\' || host || '\os2tk45'
path  = value('PATH',, env)
path  = watcom || wosdir || ';' || watcom || '\binw;' || tools || ';' || os2tk || '\bin;' || os2tk || '\som\bin;' || fppath || ';' || path
include = watcom || '\h;' || watcom || '\h\dos;' || watcom || '\h\win'
finclude = watcom || '\src\fortran'
edpath = watcom || '\eddat'
wipfc = watcom || '\wipfc'

if os == 'OS2' then do
  help = value('HELP',, env)
  help = watcom || wosdir || '\help;' || help
  bookshelf = value('BOOKSHELF',, env)
  bookshelf = watcom || wosdir || '\help;' || bookshelf
  fp = ''
  parse value fppath with fp '\bin\os2' .
  if fp \= '' then
    beginlibpath = watcom || wosdir || '\dll' || ';' || fp || '\dll'
  else
    beginlibpath = watcom || wosdir || '\dll'
  libos2 = watcom || '\lib386\os2'
end; else do
  help = ''; bookshelf = '';
  beginlibpath = ''; libos2 = '';
end

/* whether to use osFree or Watcom headers */
/* if headers == 'watcom' then
  add_inc = watcom || '\h\os2'
else
  add_inc = root || '\build\include\os2' */


lib = watcom || '\lib286;' || watcom || '\lib286\dos;' || watcom || '\lib286\win;' || libos2 || ';' || root || '\lib'

vars = 'WATCOM IMGDIR ROOT OS2TK IMGDIR1 TOOLS PATH INCLUDE LOG ',
       'FINCLUDE EDPATH HELP BOOKSHELF BEGINLIBPATH HOST',
       'LIBOS2 LIB OS SHELL REXX REXX_PATH MKISOFS SERVERENV WIPFC'

if verbose = 'yes' then
  vars = vars || ' VERBOSE'

/* Set vars */
do i = 1 to words(vars)
  var = word(vars, i)
  val = value(var)
  if os == 'UNIX' | os == 'LINUX' then do
      val = translate(val, '/:', '\;')
      'export '||var||'='||val
  end; else do
      'set '||var||'='||val
  end
  
  /* a bug with Regina REXX: it does not export */
  /* variables to a parent shell!!!             */
  /* call value var, val, env */
  
  say var || '=' || val
end

if os == 'OS2' then 'set beginlibpath=' || beginlibpath


exit 0
/* ------------------------------------ */
usage:

say 'Usage:'
say
say 'setenv <config file>'
say

/* ------------------------------------ */
