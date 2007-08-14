/* */
call RxFuncAdd 'SockLoadFuncs', 'rxsock', 'SockLoadFuncs'
call SockLoadFuncs

parse arg host port

if port = '' then do
   say 'Usage: tcpc hostname port'
   exit 1;
end

if SockGetHostByName(host, 'host.!') = 0 then do
   say 'SockGetHostByName failed' sockpsock_errno()
   say 'Errno' SockSock_Errno();
   exit 2;
end

/* Set up a message. */
buf = 'the message'
say 'buf =' buf

/* Put server info into the server stem. */
server.!family = 'AF_INET'
server.!port   = port
server.!addr   = host.!addr

/* Get a stream socket. */
s = SockSocket('AF_INET', 'SOCK_STREAM', 0)
if s < 0 then do
   call SockPSock_Errno 'Socket'
   exit 3;
end

/* Connect to the server. */
if SockConnect(s, 'server.!') < 0 then do
   call SockPSock_Errno
   exit 4;
end

if SockSend(s, buf) < 0 then do
   say 'Send() failed'
   say "SockErrno =" SockSock_Errno()
   exit 5;
end

/* The server sends back a message.  Receive it. */
/* The 3rd parameter is the number of bytes to   */
/* read from the socket.                         */
if SockRecv(s, 'buf', 12) < 0 then do
   say 'Recv() failed'
   say "SockErrno =" SockSock_Errno()
   exit 6;
end

call SockClose s;
Parse Version ver
say 'Client Ended Successfully using' ver
