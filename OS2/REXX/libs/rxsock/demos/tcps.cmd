/* */
call RxFuncAdd 'SockLoadFuncs', 'rxsock', 'SockLoadFuncs'
call SockLoadFuncs

arg port

if port = '' then do
   say "Usage: tcps port"
   exit 1;
end

s = SockSocket('AF_INET', 'SOCK_STREAM', 0)
if s < 0 then do
   say "SockSocket() failed." sockpsock_errno()
   exit 2;
end

/*
 * Enable socket to be reused
 */
if SockSetSockOpt(s, 'SOL_SOCKET', 'SO_REUSEADDR', 1) < 0 then do
   say "SockSetSockOpt() failed" sockpsock_errno()
   exit 3;
end

server.!family = 'AF_INET';
server.!port = port;
server.!addr = 'INADDR_ANY';

if SockBind(s, 'server.!') < 0 then do
   say "SockBind() failed" sockpsock_errno()
   exit 4;
end

if SockListen(s, 1) \= 0 then do
   say "SockListen() failed" sockpsock_errno()
   exit 5;
end

ns = SockAccept(s)
if ns = -1 then do
   say "SockAccept() failed" sockpsock_errno()
   exit 6;
end

/* Read data from the client into the Rexx       */
/* variable BUF.                                 */
/* The 3rd parameter is the number of bytes to   */
/* read from the socket.                         */
namelen = SockRecv(ns, 'buf', 12)
if namelen = -1 then do
   say "SockRecv() failed" sockpsock_errno()
   exit 7;
end

say "Received" buf

if SockSend(ns, buf) < 0 then do
   say "SockSend() failed" sockpsock_errno()
   exit;
end

call SockClose ns
call SockClose s

Parse Version ver
say 'Server ended successfully using' ver
