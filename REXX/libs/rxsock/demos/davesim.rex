/* */
username = 'USERNAME'
password = 'PASSWORD'
symbols = 'MSFT ADPT LCOS'

call RxFuncAdd 'SockLoadFuncs', 'rxsock', 'SockLoadFuncs'
call SockLoadFuncs

call InitGlobals username, password

do while 1
   say COPIES('-', 15)
   
   /* Open a connection and request the price for each symbol. */
   numSocks = 0
   do i = 1 to WORDS(symbols)
      numSocks = numSocks + 1
      globals.!allRdSocks.numSocks = ,
       OpenRemoteConnection(globals.!quoteIPAddr, globals.!quoteServerPort)
      
      sock = globals.!allRdSocks.numSocks

      if sock \= 0 then do
         call RequestStockQuote sock, WORD(symbols, i)
      
         ticker.sock = WORD(symbols, i)
         tickerSym = ticker.sock
      
         /* Open files to store the data read back. */
         if globals.!logConnection then do
            fileName.sock = WORD(symbols, i)'.htm'
            call STREAM fileName.sock, 'C', 'OPEN WRITE REPLACE'
         end

         /* Initialize buffers to record the data read back. */
         quoteBuff.tickerSym = ''
      end
      else do
         /* Connection failed -- this iteration doesn't count. */
         numSocks = numSocks - 1
      end
   end
   
   globals.!allRdSocks.0 = numSocks
   
   /* Wait for data to become available on the sockets. */
   do while numSocks > 0
      do i = 0 to globals.!allRdSocks.0
         conn.i = globals.!allRdSocks.i
      end
      
      numRd = SockSelect('conn.', '', '', globals.!socketReadTimeout)
      if numRd < 0 then do
         say 'SockSelect() error'
         call SockPSock_Errno
         exit
      end
      
      /* If numRd = 0, timeout occurred. */
      if numRd \= 0 then do
         do i = 1 to conn.0
            sock = conn.i
            tickerSym = ticker.sock
            
            numRd = SockRecv(sock, 'buff', 512)
            if numRd = 0 then do
               /* Other side closed the socket. */
               call SockClose sock
               call RemoveRdSock sock
               
               call FindQuoteTable quoteBuff.tickerSym

               last = 'Last'
               time = 'Time'
               change = 'Change (%)'
               say LEFT(tickerSym, 4)':' quote.last quote.time quote.change
               
               /* call STREAM fileName.sock, 'C', 'CLOSE' */
               DROP quoteBuff.tickerSym
               
               DROP fileName.sock ticker.sock
               numSocks = numSocks - 1
            end
            else do
               /* Read some data.  Dump it to the appropriate file. */
               if globals.!logConnection then do
                  call CHAROUT fileName.sock, buff
               end
               quoteBuff.tickerSym = quoteBuff.tickerSym||buff
            end
         end
      end
      else do
         /* Timeout.  Close all connections and start new ones. */
         do i = 1 to globals.!allRdSocks.0
            sock = globals.!allRdSocks.i
            tickerSym = ticker.sock
            
            /* Close the current connection. */
            call SockClose sock
            
            /* Close the corresponding file. */
            if globals.!logConnection then do
               call STREAM fileName.sock, 'C', 'CLOSE'
            end
            quoteBuff.tickerSym = ''
            
            DROP fileName.sock ticker.sock

            /* Open a new connection. */
            sock = OpenRemoteConnection(globals.!quoteIPAddr, ,
             globals.!quoteServerPort)
            
            call RequestStockQuote sock, tickerSym
            
            /* Open a new file. */
            fileName.sock = tickerSym'.htm'
            if globals.!logConnection then do
               call STREAM fileName.sock, 'C', 'OPEN WRITE REPLACE'
            end
            
            ticker.sock = tickerSym
            globals.!allRdSocks.i = sock
         end
      end
   end

   /* Sleep in an interpreter-independent manner. */
   call SockSelect ,,,globals.!updateInterval
end

say 'Done'

exit

/*=========================================================
 = InitGlobals()
 = Initialize global variables.
 = RETURNS: nothing
 =========================================================*/
InitGlobals: PROCEDURE EXPOSE globals.
parse arg username, password
   
   globals.!eol = '0D0A'x
   globals.!quoteMsgPrefix = 'GET /quotes?symbol='
   globals.!quoteMsgSuffix = '&action=Get+It%21&type=Quote HTTP/1.0'
   globals.!quoteServer    = 'rt.freerealtime.com'

   if SockGetHostByName(globals.!quoteServer, 'host.!') = 0 then do
      say 'SockGetHostByName failed'
      call SockPSock_Errno
      exit
   end

   globals.!quoteIPAddr = host.!addr
   globals.!quoteServerPort = 80
   globals.!authString = Base64Encode(username':'password)
   globals.!socketReadTimeout = 10 /* Seconds */
   globals.!updateInterval = 15 * 60 /* Seconds. */
   globals.!maxConnectRetries = 3
   globals.!logConnection = 0
return

/*=========================================================
 = OpenRemoteConnection(ipaddr, port)
 = Open a connection to a remote location.
 = RETURN:
 =    A connected socket to a remote location, if successfull
 =    0 otherwise.
 =========================================================*/
OpenRemoteConnection: PROCEDURE EXPOSE globals.
parse arg ipAddr, port
   remote.!family = 'AF_INET'
   remote.!port = port
   remote.!addr = ipAddr

   s = 0
   do i = 1 to globals.!maxConnectRetries while s = 0
      s = SockSocket('AF_INET', 'SOCK_STREAM', 0)
      if s > 0 then do
         if SockConnect(s, 'remote.!') < 0 then do
            call SockClose s
            s = 0
         end
      end
      else do
         s = 0
      end
   end
return s

/*=========================================================
 = RequestStockQuote(socket, tickerSymbol)
 = Send a request for a stock quote across the socket.
 = The request is of the form appropriate for freerealtime.com
 = RETURN: nothing
 =========================================================*/
RequestStockQuote: PROCEDURE EXPOSE globals.
parse arg sock, tickerSymbol
   call SockSend sock, ,
    globals.!quoteMsgPrefix||tickerSymbol||globals.!quoteMsgSuffix
   
   call SockSend sock, globals.!eol

   call SockSend sock, 'Accept: */*'globals.!eol

   call SockSend sock, 'Authorization: Basic' globals.!authString||globals.!eol

   call SockSend sock, globals.!eol
return

/*=========================================================
 = Base64Encode(str)
 = Performs BASE64 encoding on the string.
 = RETURN: The BASE64 encoded value.
 =========================================================*/
Base64Encode: PROCEDURE
parse arg str
   /* Create the base64 translation table. */
   base64Table = XRANGE('A', 'Z')XRANGE('a', 'z')XRANGE('0', '9')'+/'
   bits4Table  = XRANGE('00'x, '3F'x)
   
   /* Pad str with 0s */
   len = ((LENGTH(str) + 2) % 3) * 3
   newLen = len * 4 / 3
   leftOver = 3 - LENGTH(str) // 3
   if leftOver = 3 then
      leftOver = 0
   str = LEFT(str, len, '00'x)

   /* Split 3 8-bit chunks into 4 6-bit chunks */
   numSplits = len / 3
   encodedStr = ''
   do i = 1 to numSplits
      /* Convert the next 3 bytes of str to bits. */
      bits3 = X2B(C2X(SUBSTR(str, 3 * i - 2, 3)))
      
      /* Convert groups of 6 bits back to char. */
      bits4 = X2C(B2X('00'SUBSTR(bits3, 1, 6)))
      bits4 = bits4||X2C(B2X('00'SUBSTR(bits3, 7, 6)))
      bits4 = bits4||X2C(B2X('00'SUBSTR(bits3, 13, 6)))
      bits4 = bits4||X2C(B2X('00'SUBSTR(bits3, 19, 6)))

      /* Translate bits4 to the encoded string. */
      encodedStr = encodedStr||TRANSLATE(bits4, base64Table, bits4Table)
   end

   /* If string was not an multiple of 3 bytes long, we */
   /* have an extra byte or two at the end that needs   */
   /* to change from 'A' to '='                         */
   if leftOver > 0 then do
      encodedStr = LEFT(encodedStr, newLen - leftOver)||COPIES('=', leftOver)
   end
   
return encodedStr

/*=========================================================
 = RemoveRdSock(oldSocket)
 = Removes the socket from the global list of read sockets.
 =========================================================*/
RemoveRdSock: PROCEDURE EXPOSE globals.
   /* Removes a socket to the read socket list. */
parse arg oldSock
   
   numRdSockets = globals.!AllRdSocks.0

   /* Find the socket in the list. */
   do sockNum = 1 to numRdSockets ,
    while globals.!AllRdSocks.sockNum \= oldSock
   end

   /* See if the socket was found. */
   if globals.!AllRdSocks.sockNum = oldSock then do
      /* Socket found.  Remove it. */
      do i = sockNum to numRdSockets - 1
         other = i + 1
         globals.!AllRdSocks.i = globals.!AllRdSocks.other
      end
      
      globals.!AllRdSocks.0 = numRdSockets - 1
   end
   
return

/*============================================================
 = File the tablw with the quote.  Store it in 'quote.'  Store
 = the fields in 'quote.' in a stem array 'quoteFields.'
 ===========================================================*/
FindQuoteTable: PROCEDURE EXPOSE globals. quote. quoteFields.
parse arg str
   extents = FindHtmlTagExtents('TABLE', str)
   
   /* Loop through all the lowest-level tables (i.e. no subtables) */
   /* looking for the table with the quote.  The table with the    */
   /* quote will have the .1.1 element set to 'Time'               */
   tableFound = 0
   do pos = 1 to WORDS(extents) - 1 while \ tableFound
      if WORD(extents, pos) > 0 & WORD(extents, pos + 1) < 0 then do
         start = WORD(extents, pos)
         stop = -WORD(extents, pos + 1)

         call DecodeHtmlTable SUBSTR(str, start, stop - start + 1)

         /* Check to see it this is the table we want. */
         if htmlTable.1.1 = 'Time' then do
            tableFound = 1
         end
      end   
   end

   if tableFound then do
      /* Copy the htmlTable to 'quote.' */
      /* The table has rows of field names followed by rows of field values. */
      numFields = 0
      do row = 1 to htmlTable.0 by 2
         do col = 1 to htmlTable.row.0
            numFields = numFields + 1
            quoteFields.numFields = htmlTable.row.col
            
            nextRow = row + 1
            fieldName = quoteFields.numFields
            
            quote.fieldName = htmlTable.nextRow.col
         end
      end

      quoteFields.0 = numFields
   end
   else do
      quoteFields.0 = 0
   end
return

/*============================================================
 = Function to pretty print a list of extents.
 ===========================================================*/
PrettyPrintExtents: PROCEDURE EXPOSE globals.
parse arg extents
   level = 0

   do i = 1 to WORDS(extents)
      nxtExt = WORD(extents, i)
      if nxtExt > 0 then do
         say COPIES(' ', level * 3) 'Start table at' nxtExt
         level = level + 1
      end
      else do
         level = level - 1
         nxtExt = -nxtExt
         say COPIES(' ', level * 3) 'Stop table at' nxtExt
      end
   end
return

/*============================================================
 = Returns a list of the extents of the given tag in
 = the given buffer. The list returned has the start of the
 = tag regions as positive numbers and the end of them as
 = negative numbers.
 ===========================================================*/
FindHtmlTagExtents: PROCEDURE EXPOSE globals.
parse arg tag, buff
   extents = ''
   
   stopChar = 0

   startTag = '<'TRANSLATE(tag)
   stopTag  = '</'TRANSLATE(tag)'>'

   startTagLen = LENGTH(startTag)
   stopTagLen = LENGTH(stopTag)
   
   /* Convert buff to upercase for easier processing. */
   buff = TRANSLATE(buff)

   /* Find the start of the next tag. */
   startChar = POS(startTag, buff)
   if startChar = 0 then
      startChar = LENGTH(buff) + 1
   else
      extents = startChar

   startLookForStopPos = startChar + startTagLen

   do while startLookForStopPos < LENGTH(buff)
      /* Search for tag or end tag. */
      stopChar = POS(stopTag, buff, startLookForStopPos)
      startChar = POS(startTag, buff, startLookForStopPos)
            
      if startChar = 0 then
         startChar = LENGTH(buff) + 1
      if stopChar = 0 then
         stopChar = LENGTH(buff) + 1

      /* See if subtable found. */
      if startChar < stopChar then do
         startLookForStopPos = startChar + startTagLen
         extents = extents startChar
      end
      else if stopChar <= LENGTH(buff) then do
         startLookForStopPos = stopChar + stopTagLen
         stopExt = -(stopChar + stopTagLen - 1)
         extents = extents stopExt
      end
      else
         startLookForStopPos = LENGTH(buff) + 1
   end
   
return STRIP(extents)

/*============================================================
 = Strips all the HTML codes from the given string.
 ===========================================================*/
StripHtml: PROCEDURE EXPOSE globals.
parse arg inStr
   /* Copy any leading non-HTML stuff. */
   startTag = POS('<', inStr)
   if startTag \= 0 then
      outStr = SUBSTR(inStr, 1, startTag - 1)
   else
      outStr = inStr

   /* Loop through the input string, copying all non-HTML stuff to outStr */
   do while startTag \= 0
      /* Find the end position of the current tag. */
      stopTag = POS('>', inStr, 2)

      /* Remove the tag. */
      inStr = SUBSTR(inStr, stopTag + 1)

      /* Look for the start of the next tag. */
      startTag = POS('<', inStr)
      if startTag \= 0 then
         outStr = outStr||SUBSTR(inStr, 1, startTag - 1)
      else
         outStr = outStr||inStr
   end

   /* Get rid of any <lf> chars. */
   outStr = TRANSLATE(outStr, ' ', '0a'x)
return STRIP(outStr)

/*============================================================
 = Decodes the first table in 'table'.  The data in the
 = table is returned in the htmlTable. stem.
 ===========================================================*/
DecodeHtmlTable: PROCEDURE EXPOSE globals. htmlTable.
parse arg table
   /* Find the extents of the rows of the table. */
   trExtents = FindHtmlTagExtents('TR', table)
   
   /* There are 2 entries in trExtents for each row in the table. */
   htmlTable.0 = WORDS(trExtents) / 2

   /* Loop through each row of the table. */
   do row = 1 to htmlTable.0
      /* Pull the start and stop of the next row off trExents. */
      parse var trExtents start stop trExtents
      stop = -stop

      rowHtml = SUBSTR(table, start, stop - start + 1)

      /* Find the extents of the columns in this row of the table. */
      colExtents = FindHtmlTagExtents('TD', rowHtml)

      /* 2 colExtents entries per col. */
      htmlTable.row.0 = WORDS(colExtents) / 2

      /* Loop through each column and copy the data. */
      do col = 1 to htmlTable.row.0
         parse var colExtents colStart colStop colExtents
         colStop = -colStop

         htmlTable.row.col = StripHtml(SUBSTR(rowHtml, colStart, ,
          colStop - colStart + 1))
      end
   end
return
