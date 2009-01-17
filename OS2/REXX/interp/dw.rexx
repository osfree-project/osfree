call P
say 'after return: p.1='p.1 /* 'ok' */

call Q
say 'after return: q.1='q.1 /* 'bad!' */

exit

P:
procedure expose p.1
p.1 = 'ok'
say 'before return: p.1='p.1 /* 'ok' */
return

Q:
procedure expose q.1
q. = 'bad!'
q.1 = 'ok'
say 'before return: q.1='q.1 /* 'ok' */
return 
