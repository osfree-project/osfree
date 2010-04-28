/* */
cli.0=1
cli.1='ansi.exe'

do i=1 to cli.0
  say 'Testing '||cli.i
  interpret 'call '||cli.i||'.cmd'
end