#
# Regina error messages - English
# Written by Mark Hessling <M.Hessling@qut.edu.au>
#
# dutch - Gert van der Kooij <geko@wanadoo.nl>
# french - Mark <cotemark@globetrotter.net>
# german - <florian@grosse-coosmann.de>
# norwegian (bokmal) - Vidar Tysse <vtysse@broadpark.no>
# portuguese - brian <bcarpent@nucleus.com>
# russian - Oleg Kulikov <kulikov@xanadu.ru>
# spanish - Pablo Garcia-Abia <Pablo.Garcia@cern.ch>
# svenska - Jan-Erik L„rka <jan-erik@os2ug.se>
#
# Guidelines for Translators
# --------------------------
# - Lines beginning with # are comments
# - No blank lines
# - Any word in UPPER CASE should not be translated
# - On some error message lines, there are tokens following the
#   | character. Again these should not be translated.
# - Please ensure that the order of the substitutions; ie the %s, %d
#   placemarkers are maintained. If this is gramatically impossible,
#   please let me know.
# - There are some comments at the end of this file under the heading:
#   "Extra general phrases requiring translation:"
#   Please translate these phrases also.
# - File to be saved as UTF-8
# - Return the translated error messages in a complete file, the same as this
#   file, with the name XX.mts where XX is the ISO language abbreviation.
#   Important: Please zip the file up to ensure that the file arrives in
#              its original form
#   Add a comment at the top of the file indicating the language, and your name and email address.
#   If you do not want your email address published in the Regina documentation
#   please indicate this. I will retain your address as a contact for future
#   individual message translations, but will only publish your name.
# - And last but by no means least; thanks!
#
  0,  1,Error %s running %s, line %d:|<value>,<source>,<linenumber>
  0,  2,Error %s in interactive trace:|<value>
  0,  3,Interactive trace.  "Trace Off" to end debug. ENTER to continue.
  2,  0,Failure during finalization
  2,  1,Failure during finalization: %s|<description>
  3,  0,Failure during initialization
  3,  1,Failure during initialization: %s|<description>
  4,  0,Program interrupted
  4,  1,Program interrupted with HALT condition: %s|<description>
  5,  0,System resources exhausted
  5,  1,System resources exhausted: %s|<description>
  6,  0,Unmatched "/*" or quote
  6,  1,Unmatched comment delimiter ("/*")
  6,  2,Unmatched single quote (')
  6,  3,Unmatched double quote (")
  7,  0,WHEN or OTHERWISE expected
  7,  1,SELECT on line %d requires WHEN; found "%s"|<linenumber>,<token>
  7,  2,SELECT on line %d requires WHEN, OTHERWISE, or END; found "%s"|<linenumber>,<token>
  7,  3,All WHEN expressions of SELECT on line %d are false; OTHERWISE expected|<linenumber>
  8,  0,Unexpected THEN or ELSE
  8,  1,THEN has no corresponding IF or WHEN clause
  8,  2,ELSE has no corresponding THEN clause
  9,  0,Unexpected WHEN or OTHERWISE
  9,  1,WHEN has no corresponding SELECT
  9,  2,OTHERWISE has no corresponding SELECT
 10,  0,Unexpected or unmatched END
 10,  1,END has no corresponding DO or SELECT
 10,  2,END corresponding to DO on line %d must have a symbol following that matches the control variable (or no symbol); found "%s"|<linenumber>,<token>
 10,  3,END corresponding to DO on line %d must not have a symbol following it because there is no control variable; found "%s"|<linenumber>,<token>
 10,  4,END corresponding to SELECT on line %d must not have a symbol following; found "%s"|<linenumber>,<token>
 10,  5,END must not immediately follow THEN
 10,  6,END must not immediately follow ELSE
 11,  0,[Control stack full]
 12,  0,[Clause > 1024 characters]
 13,  0,Invalid character in program
 13,  1,Invalid character in program "('%x'X)"|<hex-encoding>
 14,  0,Incomplete DO/SELECT/IF
 14,  1,DO instruction requires a matching END
 14,  2,SELECT instruction requires a matching END
 14,  3,THEN requires a following instruction
 14,  4,ELSE requires a following instruction
 15,  0,Invalid hexadecimal or binary string
 15,  1,Invalid location of blank in position %d in hexadecimal string|<position>
 15,  2,Invalid location of blank in position %d in binary string|<position>
 15,  3,Only 0-9, a-f, A-F, and blank are valid in a hexadecimal string; found "%c"|<char>
 15,  4,Only 0, 1, and blank are valid in a binary string; found "%c"|<char>
 16,  0,Label not found
 16,  1,Label "%s" not found|<name>
 16,  2,Cannot SIGNAL to label "%s" because it is inside an IF, SELECT or DO group|<name>
 16,  3,Cannot invoke label "%s" because it is inside an IF, SELECT or DO group|<name>
 17,  0,Unexpected PROCEDURE
 17,  1,PROCEDURE is valid only when it is the first instruction executed after an internal CALL or function invocation
 18,  0,THEN expected
 18,  1,IF keyword on line %d requires matching THEN clause; found "%s"|<linenumber>,<token>
 18,  2,WHEN keyword on line %d requires matching THEN clause; found "%s"|<linenumber>,<token>
 19,  0,String or symbol expected
 19,  1,String or symbol expected after ADDRESS keyword; found "%s"|<token>
 19,  2,String or symbol expected after CALL keyword; found "%s"|<token>
 19,  3,String or symbol expected after NAME keyword; found "%s"|<token>
 19,  4,String or symbol expected after SIGNAL keyword; found "%s"|<token>
 19,  6,String or symbol expected after TRACE keyword; found "%s"|<token>
 19,  7,Symbol expected in parsing pattern; found "%s"|<token>
 20,  0,Name expected
 20,  1,Name required; found "%s"|<token>
 20,  2,Found "%s" where only a name is valid|<token>
 21,  0,Invalid data on end of clause
 21,  1,The clause ended at an unexpected token; found "%s"|<token>
 22,  0,Invalid character string
 22,  1,Invalid character string '%s'X|<hex-encoding>
 23,  0,Invalid data string
 23,  1,Invalid data string '%s'X|<hex-encoding>
 24,  0,Invalid TRACE request
 24,  1,TRACE request letter must be one of "%s"; found "%c"|ACEFILNOR,<value>
 25,  0,Invalid sub-keyword found
 25,  1,CALL ON must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25,  2,CALL OFF must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25,  3,SIGNAL ON must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25,  4,SIGNAL OFF must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25,  5,ADDRESS WITH must be followed by one of the keywords INPUT, OUTPUT or ERROR; found "%s"|<token>
 25,  6,INPUT must be followed by one of the keywords STREAM, STEM, LIFO, FIFO or NORMAL; found "%s"|<token>
 25,  7,OUTPUT must be followed by one of the keywords STREAM, STEM, LIFO, FIFO, APPEND, REPLACE or NORMAL; found "%s"|<token>
 25,  8,APPEND must be followed by one of the keywords STREAM, STEM, LIFO or FIFO; found "%s"|<token>
 25,  9,REPLACE must be followed by one of the keywords STREAM, STEM, LIFO or FIFO; found "%s"|<token>
 25, 11,NUMERIC FORM must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25, 12,PARSE must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25, 13,UPPER must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25, 14,ERROR must be followed by one of the keywords STREAM, STEM, LIFO, FIFO, APPEND, REPLACE or NORMAL; found "%s"|<token>
 25, 15,NUMERIC must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25, 16,FOREVER must be followed by one of the keywords %s; found "%s"|<keywords>,<token>
 25, 17,PROCEDURE must be followed by the keyword EXPOSE or nothing; found "%s"|<token>
 26,  0,Invalid whole number
 26,  1,Whole numbers must fit within current DIGITS setting(%d); found "%s"|<value>,<value>
 26,  2,Value of repetition count expression in DO instruction must be zero or a positive whole number; found "%s"|<value>
 26,  3,Value of FOR expression in DO instruction must be zero or a positive whole number; found "%s"|<value>
 26,  4,Positional parameter of parsing template must be a whole number; found "%s"|<value>
 26,  5,NUMERIC DIGITS value must be a positive whole number; found "%s"|<value>
 26,  6,NUMERIC FUZZ value must be zero or a positive whole number; found "%s"|<value>
 26,  7,Number used in TRACE setting must be a whole number; found "%s"|<value>
 26,  8,Operand to right of power operator ("**") must be a whole number; found "%s"|<value>
 26, 11,Result of %s %% %s operation would need exponential notation at current NUMERIC DIGITS %d|<value>,<value>,<value>
 26, 12,Result of %% operation used for %s // %s operation would need exponential notation at current NUMERIC DIGITS %d|<value>,<value>,<value>
 27,  0,Invalid DO syntax
 27,  1,Invalid use of keyword "%s" in DO clause|<token>
 28,  0,Invalid LEAVE or ITERATE
 28,  1,LEAVE is valid only within a repetitive DO loop
 28,  2,ITERATE is valid only within a repetitive DO loop
 28,  3,Symbol following LEAVE ("%s") must either match control variable of a current DO loop or be omitted|<token>
 28,  4,Symbol following ITERATE ("%s") must either match control variable of a current DO loop or be omitted|<token>
 29,  0,Environment name too long
 29,  1,Environment name exceeds %d characters; found "%s"|#Limit_EnvironmentName,<name>
 30,  0,Name or string too long
 30,  1,Name exceeds %d characters|#Limit_Name
 30,  2,Literal string exceeds %d characters|#Limit_Literal
 31,  0,Name starts with number or "."
 31,  1,A value cannot be assigned to a number; found "%s"|<token>
 31,  2,Variable symbol must not start with a number; found "%s"|<token>
 31,  3,Variable symbol must not start with a "."; found "%s"|<token>
 32,  0,[Invalid use of stem]
 33,  0,Invalid expression result
 33,  1,Value of NUMERIC DIGITS ("%d") must exceed value of NUMERIC FUZZ ("%d")|<value>,<value>
 33,  2,Value of NUMERIC DIGITS ("%d") must not exceed %d|<value>,#Limit_Digits
 33,  3,Result of expression following NUMERIC FORM must start with "E" or "S"; found "%s"|<value>
 34,  0,Logical value not "0" or "1"
 34,  1,Value of expression following IF keyword must be exactly "0" or "1"; found "%s"|<value>
 34,  2,Value of expression following WHEN keyword must be exactly "0" or "1"; found "%s"|<value>
 34,  3,Value of expression following WHILE keyword must be exactly "0" or "1"; found "%s"|<value>
 34,  4,Value of expression following UNTIL keyword must be exactly "0" or "1"; found "%s"|<value>
 34,  5,Value of expression to left of logical operator "%s" must be exactly "0" or "1"; found "%s"|<operator>,<value>
 34,  6,Value of expression to right of logical operator "%s" must be exactly "0" or "1"; found "%s"|<operator>,<value>
 35,  0,Invalid expression
 35,  1,Invalid expression detected at "%s"|<token>
 36,  0,Unmatched "(" in expression
 37,  0,Unexpected "," or ")"
 37,  1,Unexpected ","
 37,  2,Unmatched ")" in expression
 38,  0,Invalid template or pattern
 38,  1,Invalid parsing template detected at "%s"|<token>
 38,  2,Invalid parsing position detected at "%s"|<token>
 38,  3,PARSE VALUE instruction requires WITH keyword
 39,  0,[Evaluation stack overflow]
 40,  0,Incorrect call to routine
 40,  1,External routine "%s" failed|<name>
 40,  3,Not enough arguments in invocation of "%s"; minimum expected is %d|<bif>,<argnumber>
 40,  4,Too many arguments in invocation of "%s"; maximum expected is %d|<bif>,<argnumber>
 40,  5,Missing argument in invocation of "%s"; argument %d is required|<bif>,<argnumber>
 40,  9,%s argument %d exponent exceeds %d digits; found "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,%s argument %d must be a number; found "%s"|<bif>,<argnumber>,<value>
 40, 12,%s argument %d must be a whole number; found "%s"|<bif>,<argnumber>,<value>
 40, 13,%s argument %d must be zero or positive; found "%s"|<bif>,<argnumber>,<value>
 40, 14,%s argument %d must be positive; found "%s"|<bif>,<argnumber>,<value>
 40, 17,%s argument 1, must have an integer part in the range 0:90 and a decimal part no larger than .9; found "%s"|<bif>,<value>
 40, 18,%s conversion must have a year in the range 0001 to 9999|<bif>
 40, 19,%s argument 2, "%s", is not in the format described by argument 3, "%s"|<bif>,<value>,<value>
 40, 21,%s argument %d must not be null|<bif>,<argnumber>
 40, 23,%s argument %d must be a single character; found "%s"|<bif>,<argnumber>,<value>
 40, 24,%s argument 1 must be a binary string; found "%s"|<bif>,<value>
 40, 25,%s argument 1 must be a hexadecimal string; found "%s"|<bif>,<value>
 40, 26,%s argument 1 must be a valid symbol; found "%s"|<bif>,<value>
 40, 27,%s argument 1, must be a valid stream name; found "%s"|<bif>,<value>
 40, 28,%s argument %d, option must start with one of "%s"; found "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,%s conversion to format "%s" is not allowed|<bif>,<value>
 40, 31,%s argument 1 ("%d") must not exceed 100000|<bif>,<value>
 40, 32,%s the difference between argument 1 ("%d") and argument 2 ("%d") must not exceed 100000|<bif>,<value>,<value>
 40, 33,%s argument 1 ("%d") must be less than or equal to argument 2 ("%d")|<bif>,<value>,<value>
 40, 34,%s argument 1 ("%d") must be less than or equal to the number of lines in the program (%d)|<bif>,<value>,<sourceline()>
 40, 35,%s argument 1 cannot be expressed as a whole number; found "%s"|<bif>,<value>
 40, 36,%s argument 1 must be a name of a variable in the pool; found "%s"|<bif>,<value>
 40, 37,%s argument 3 must be the name of a pool; found "%s"|<bif>,<value>
 40, 38,%s argument %d is not large enough to format "%s"|<bif>,<argnumber>,<value>
 40, 39,%s argument 3 is not zero or one; found "%s"|<bif>,<value>
 40, 41,%s argument %d must be within the bounds of the stream; found "%s"|<bif>,<argnumber>,<value>
 40, 42,%s argument 1; cannot position on this stream; found "%s"|<bif>,<value>
 40, 43,%s argument %d must be a single non-alphanumeric character or the null string; found "%s"|<bif>,<argnumber>,<value>
 40, 44,%s argument %d, "%s", is a format incompatible with the separator specified in argument %d|<bif>,<argnumber>,<value>,<argnumber>
 40,914,[%s argument %d, must be one of "%s"; found "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: low-level stream I/O error; %s]|<bif>,<description>
 40,921,[%s argument %d, stream positioning mode "%s"; incompatible with stream open mode]|<bif>,<argnumber>,<value>
 40,922,[%s argument %d, too few sub-commands; minimum expected is %d; found %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[%s argument %d, too many sub-commands; maximum expected is %d; found %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[%s argument %d, invalid positional specification; expecting one of "%s"; found "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, function TIMEOUT, expecting a whole number between 0 and %d; found \"%s\"]|<value>,<value>
 40,980,Unexpected input, either unknown type or illegal data%s%s|: ,<location>
 40,981,Number out of the allowed range%s%s|: ,<location>
 40,982,String too big for the defined buffer%s%s|: ,<location>
 40,983,Illegal combination of type/size%s%s|: ,<location>
 40,984,Unsupported number like NAN, +INF, -INF%s%s|: ,<location>
 40,985,Structure too complex for static internal buffer%s%s|: ,<location>
 40,986,An element of the structure is missing%s%s|: ,<location>
 40,987,A value of the structure is missing%s%s|: ,<location>
 40,988,The name or part of the name is illegal for the interpreter%s%s|: ,<location>
 40,989,A problem occured at the interface between Regina and GCI%s%s|: ,<location>
 40,990,The type won't fit the requirements for basic types (arguments/return value)%s%s|: ,<location>
 40,991,The number of arguments is wrong or an argument is missing%s%s|: ,<location>
 40,992,GCI's internal stack for arguments got an overflow%s%s|: ,<location>
 40,993,GCI counted too many nested LIKE containers%s%s|: ,<location>
 41,  0,Bad arithmetic conversion
 41,  1,Non-numeric value ("%s") to left of arithmetic operation "%s"|<value>,<operator>
 41,  2,Non-numeric value ("%s") to right of arithmetic operation "%s"|<value>,<operator>
 41,  3,Non-numeric value ("%s") used with prefix operator "%s"|<value>,<operator>
 41,  4,Value of TO expression in DO instruction must be numeric; found "%s"|<value>
 41,  5,Value of BY expression in DO instruction must be numeric; found "%s"|<value>
 41,  6,Value of control variable expression of DO instruction must be numeric; found "%s"|<value>
 41,  7,Exponent exceeds %d digits; found "%s"|#Limit_ExponentDigits,<value>
 42,  0,Arithmetic overflow/underflow
 42,  1,Arithmetic overflow detected at "%s %s %s"; exponent of result requires more than %d digits|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,Arithmetic underflow detected at "%s %s %s"; exponent of result requires more than %d digits|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,Arithmetic overflow; divisor must not be zero
 43,  0,Routine not found
 43,  1,Could not find routine "%s"|<name>
 44,  0,Function did not return data
 44,  1,No data returned from function "%s"|<name>
 45,  0,No data specified on function RETURN
 45,  1,Data expected on RETURN instruction because routine "%s" was called as a function|<name>
 46,  0,Invalid variable reference
 46,  1,Extra token ("%s") found in variable reference; ")" expected|<token>
 47,  0,Unexpected label
 47,  1,INTERPRET data must not contain labels; found "%s"|<name>
 48,  0,Failure in system service
 48,  1,Failure in system service: %s|<description>
 48,920,Low-level stream I/O error: %s %s: %s|<description>,<stream>,<description>
 49,  0,Interpretation Error
 49,  1,Interpretation Error: Failed in %s, line %d: "%s". Please report error!|<module>,<linenumber>,<description>
 50,  0,Unrecognized reserved symbol
 50,  1,Unrecognized reserved symbol "%s"|<token>
 51,  0,Invalid function name
 51,  1,Unquoted function names must not end in a period; found "%s"|<token>
 52,  0,Result returned by "%s" is longer than %d characters|<name>,#Limit_String
 53,  0,Invalid option
 53,  1,String or symbol expected after STREAM keyword; found "%s"|<token>
 53,  2,Variable reference expected after STEM keyword; found "%s"|<token>
 53,  3,Argument to STEM must have one period, as its last character; found "%s"|<name>
 53,100,String or symbol expected after LIFO keyword; found "%s"|<token>
 53,101,String or symbol expected after FIFO keyword; found "%s"|<token>
 54,  0,Invalid STEM value
 54,  1,For this STEM APPEND, the value of "%s" must be a count of lines; found "%s"|<name>,<value>
#
# All error messages after this point are not defined by ANSI
#
 60,  0,[Can't rewind transient file]
 61,  0,[Improper seek operation on file]
 64,  0,[Syntax error while parsing]
 64,  1,[Syntax error at line %d]
 64,  2,[General syntax error at line %d, column %d]|<linenumber>,<columnnumber>
 90,  0,[Non-ANSI feature used with "OPTIONS STRICT_ANSI"]
 90,  1,[%s is a Regina extension BIF]|<bif>
 90,  2,[%s is a Regina extension instruction]|<token>
 90,  3,[%s argument %d, option must start with one of "%s" with "OPTIONS STRICT_ANSI"; found "%s"; a Regina extension]|<bif>,<argnumber>,<optionslist>,<value>
 90,  4,[%s is a Regina extension operator]|<token>
 93,  0,[Incorrect call to routine]
 93,  1,[STREAM command %s must be followed by one of "%s"; found "%s"]|<token>,<value>,<value>
 93,  3,[STREAM command must be one of "%s"; found "%s"]|<value>,<value>
 94,  0,[External queue interface error]
 94,  1,[External queue timed out]
 94, 99,[Internal error with external queue interface: %d "%s"]|<description>,<systemerror>
 94,100,[General system error with external queue interface. %s. %s]|<description>,<systemerror>
 94,101,[Error connecting to %s on port %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[Unable to obtain IP address for %s]|<machine>
 94,103,[Invalid format for server in specified queue name: "%s"]|<queuename>
 94,104,[Invalid format for queue name: "%s"]|<queuename>
 94,105,[Unable to start Windows Socket interface: %s]|<systemerror>
 94,106,[Maximum number of external queues exceeded: %d]|<maxqueues>
 94,107,[Error occured reading socket: %s]|<systemerror>
 94,108,[Invalid switch passed. Must be one of "%s"]|<switch>
 94,109,[Queue \"%s\" not found]|<queuename>
 94,110,[%s invalid for external queues]|<bif>
 94,111,[RXQUEUE function %s invalid for internal queues]|<functionname>
 95,  0,[Restricted feature used in "safe" mode]
 95,  1,[%s invalid in "safe" mode]|<token>
 95,  2,[%s argument %d invalid in "safe" mode]|<bif>,<argnumber>
 95,  3,[%s argument %d: "%s", invalid in "safe" mode]|<bif>,<argnumber>,<token>
 95,  4,[STREAM argument 3: Opening files for WRITE access invalid in "safe" mode]
 95,  5,[Running external commands invalid in "safe" mode]
100,  0,[Unknown filesystem error]
#
# Extra general phrases requiring translation:
#
P,  0,Error %d running "%.*s", line %d: %.*s
P,  1,Error %d.%d: %.*s
P,  2,Error %d running "%.*s": %.*s
#P,  3,Unable to open language file: %s
#P,  4,Incorrect number of messages in language file: %s
#P,  5,Unable to read from language file: %s
#P,  6,Text missing from language file: %s.mtb
#P,  7,Language file: %s.mtb is corrupt
# (err1prefix)   "Error %d running \"%.*s\", line %d: %.*s",
# (suberrprefix) "Error %d.%d: %.*s",
# (err2prefix)   "Error %d running \"%.*s\": %.*s",
# (erropen)      "Unable to open language file: %s",
# (errcount)     "Incorrect number of messages in language file: %s",
# (errread)      "Unable to read from language file: %s",
# (errmissing)   "Text missing from language file: %s.mtb",
# (errcorrupt)   "Language file: %s.mtb is corrupt",
