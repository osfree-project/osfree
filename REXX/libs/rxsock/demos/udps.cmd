/* */
call RxFuncAdd 'SockLoadFuncs', 'rxSock', 'SockLoadFuncs'
call SockLoadFuncs

call SockInit

/* Create a datagram socket in the internet domain. */
s = SockSocket('AF_INET', 'SOCK_DGRAM', 0)
if s < 0 then do
   say "SockSocket() error" sockpsock_errno()
   exit
end

/*
 * Bind my name to this socket so that client on the network can
 * send me messages. (This allows the operating system to demultiplex
 * messages and get them to the correct server)
 *
 * Set up the server name. The internet address is specified as the
 * wildcard INADDR_ANY so that the server can get messages from any
 * of the physical internet connections on this host. (Otherwise we
 * would limit the server to messages from only one network interface)
 */
server.!family = 'AF_INET'   /* Server is in Internet Domain */
server.!port = 0             /* Use any available port       */
server.!addr = 'INADDR_ANY'  /* Server's Internet Address    */

if SockBind(s, 'server.!') < 0 then do
   say "SockBind() error" sockpsock_errno()
   exit
end

/* Find out what port was really assigned and print it. */
if SockGetSockName(s, 'server.!') < 0 then do
   say "SockGetSockName() error" sockpsock_errno()
   exit
end

say "Port assigned is" server.!port

/*
 * Receive a message on socket s in buf  of maximum size 32
 * from a client. 
 * The name of the client will be placed into the
 * client stem.
 */
if SockRecvFrom(s, 'buf', 32, 'client.!') < 0 then do
   say "SockRecvFrom() error" sockpsock_errno()
   exit
end

/*
 * Print the message and the name of the client.
 */
say "Received message" buf "from domain" client.!family,
 "port" client.!port "internet address" client.!addr

call SockClose s
