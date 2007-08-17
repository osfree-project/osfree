/* */
call RxFuncAdd 'SockLoadFuncs', 'rxSock', 'SockLoadFuncs'
call SockLoadFuncs

parse arg hostaddr port

if port == '' then do
   say "Usage: udpc <host address> <port>"
   exit
end

call SockInit

/* Create a datagram socket in the internet domain and use the
 * default protocol (UDP).
 */
s = SockSocket('AF_INET', 'SOCK_DGRAM', 0)
if s < 0 then do
   say "Socket() error" sockpsock_errno()
   exit
end

/* Set up the server name */
server.!family = 'AF_INET'
server.!port   = port;
server.!addr   = hostaddr

/* Send the message in buf to the server */
if SockSendTo(s, 'Hello', 'server.!') < 0 then do
   say "SockSendTo() error" sockpsock_errno()
   exit
end

/* Deallocate the socket */
call SockClose s
