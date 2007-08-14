/* REXX RXXMATH v1.3 (19 Nov 1996); Copyright 1992, 1996 by John Brock

For help, type "rxxmath" without arguments at the command line.

*/

signal on halt /* First instruction, because we could halt anytime! */
signal on syntax /* Mainly in case we hit internal numeric limits. */
signal on novalue /* Shouldn't ever happen. */

parse source . cmd? . .
cmd? = cmd? = "COMMAND" /* Were we called as a command? */
dgt = digits() /* Default for precision argument. */
glb.0dgt = dgt /* (Save it). */
extra_dgt? = 1 /* Increase internal precision above requested value? */

/* Get precision, function name, and arguments from argument list.  */
/* Note that the "extra_dgt?" variable may be taken from argument   */
/* list.  This is an "undocumented interface", which is used to     */
/* avoid a second and redundant increase in internal precision      */
/* when RXXMATH calls itself recursively through the CALC function. */
if cmd? /* Do basic argument parsing. */
  then parse arg func x y . 1 . str /* (str is for CALC). */
  else parse arg func 1 _extra_dgt? _dgt, x 1 str, y
select /* Deal with possible non-basic argument parsing. */
  when cmd?
    then if datatype(func, "N") /* First argument is precision. */
      then parse arg dgt func x y . 1 . . str
      else if func = ""
        then signal helpmsg /* No arguments, so print help message. */
  when (_extra_dgt? == 0 | _extra_dgt? == 1) & datatype(_dgt, "N")
    then parse arg extra_dgt? dgt, func, x 1 str, y
  otherwise if datatype(func, "N") /* First argument is precision. */
    then parse arg dgt, func, x 1 str, y
  end
func = space(translate(func))
dgt = dgt / 1

/* Validate precision argument. */
if \datatype(dgt, "W") | dgt < 0
  then call error "Illegal value for precision:" dgt

/* List of functions supported by RXXMATH (and available in CALC). */
funcs = "FACT PERM COMB SQRT POW LOG EXP LN PI SIN COS TAN",
  "COT SEC CSC ARCSIN ARCCOS ARCTAN ARCCOT ARCSEC ARCCSC CALC"

/* "SCAN" and "PULLSCAN" are special functions which support CALC. */
/* They are "undocumented", and not listed in the function list    */
/* (which means that they are not available as CALC functions).    */
/* Note that dgt = 0 has a special meaning for these functions.    */
if \cmd? & (func = "SCAN" | func = "PULLSCAN") then do
  if func = "PULLSCAN" then do
    if y \= "LIT" & queued() + lines() = 0 then say x
    parse pull x
    end
  exit scan(funcs, extra_dgt?, dgt, x)
  end

/* Validate function name. */
if func = "" then call error "Function name is missing."
if words(func) \= 1 | wordpos(func, funcs) = 0
  then call error "Invalid function name:" func
glb.0func = func /* Save function name for use in error messages. */

/* If precision is zero then use default value. */
if dgt = 0 then dgt = glb.0dgt

/* Remove excess precision from arguments. */
numeric digits dgt
if datatype(x, "N")
  then x = x / 1
  else x = space(x)
if datatype(y, "N")
  then y = y / 1
  else y = space(y)

numeric digits intdgt(dgt, extra_dgt?) /* Set internal precision. */

select /* Call appropriate function. */
  when func == "FACT" then z = fact(x)
  when func == "PERM" then z = perm(x, y)
  when func == "COMB" then z = comb(x, y)
  when func == "SQRT" then z = sqrt(x)
  when func == "POW" then z = pow(x, y)
  when func == "LOG" then z = log(x, y)
  when func == "EXP" then z = exp(x)
  when func == "LN" then z = ln(x)
  when func == "PI" then z = pi()
  when func == "SIN" then z = sin(x)
  when func == "COS" then z = cos(x)
  when func == "TAN" then z = tan(x)
  when func == "COT" then z = cot(x)
  when func == "SEC" then z = sec(x)
  when func == "CSC" then z = csc(x)
  when func == "ARCSIN" then z = arcsin(x)
  when func == "ARCCOS" then z = arccos(x)
  when func == "ARCTAN" then z = arctan(x)
  when func == "ARCCOT" then z = arccot(x)
  when func == "ARCSEC" then z = arcsec(x)
  when func == "ARCCSC" then z = arccsc(x)
  /* The interpretive CALC function is supported by RXXMATHI,      */
  /* leaving RXXMATH eligible for REXX compilation (if available). */
  when func == "CALC" then do
    call "RXXMATHI" str, extra_dgt? & str = "",,
      intdgt(dgt, extra_dgt? & str \= ""), errmsg(),,
      "RXXMATH interactive mode -- enter any valid REXX instruction:"
    if symbol("result") = "VAR" /* Process output from CALC function. */
      then if str = "" /* If we were in interactive CALC mode... */
        then exit result /* ...then return result immediately. */
        else z = result /* Process non-interactive result further. */
      else if str = ""
        then exit /* A simple exit is OK from interactive mode... */
        else call error /* ...but otherwise means there was an error. */
    end
  otherwise call error "Function name is valid but not implemented."
  end

numeric digits dgt /* Reset precision to requested value. */
z = z / 1 /* Format result, removing trailing zeros. */

/* All done, so return result to user! */
if \cmd? then exit z
say z
exit


/* Given a string, scan it for function calls that can be executed */
/* by RXXMATH, and replace them with direct calls to RXXMATH.  For */
/* example, "exp(2)" might become "RXXMATH(0 digits(), 'EXP', 2)". */
/* Note that the calls to RXXMATH that are constructed here supply */
/* extra_dgt? as part of the first argument.  This is really the   */
/* only place where this should ever happen.                       */
scan: procedure expose glb.
parse arg funcs, extra_dgt?, dgt, str
if dgt = 0 then dgt = "digits()"

do i = 1 to words(funcs) /* Loop thru list of supported functions. */
  func = translate(word(funcs, i)) /* Select a function. */
  pos = 0 /* Start at the beginning of the string. */
  do forever /* Look for multiple occurrences. */
    prepstr = prepstr(str, max(1, pos), '"' "'", '"' "'")
    pos = pos(func"(", prepstr, pos + 1) /* Needn't find ")". */
    if pos = 0 then leave /* Function not found in string. */
    if pos > 1
      then if symbol(substr(prepstr, pos - 1, 1)) \= "BAD"
        then iterate /* (What we found was part of another function!) */
    /* Supported function found, so replace it with call to RXXMATH. */
    str = insert("'RXXMATH'("extra_dgt? dgt", '"func"', ",,
      delstr(str, pos, length(func) + 1), pos - 1)
    end
  end

return str


/* Take a string, uppercase it, and blank out any quoted       */
/* substrings (to avoid unnecessary function call expansions). */
/* Input is string, starting position, left delimiters, and    */
/* corresponding right delimiters.                             */
/* Example: "ln(2) + calc('pi()')" ==> "ln(2) + calc('    ')". */
prepstr: procedure expose glb.
arg str, start, lftdlm, rgtdlm

do while start <= length(str)

  /* Find the nearest left delimiter, if any. */
  parse value (length(str) + 1) with lftpos lft rgt
  do i = 1 to words(lftdlm)
    pos = pos(word(lftdlm, i), str, start)
    if pos > 0 & pos < lftpos then do
      lft = word(lftdlm, i)
      rgt = word(rgtdlm, i)
      lftpos = pos
      end
    end

  if lft = "" then leave /* No left delimiter found. */

  /* Blank out string between left and right delimiter (if any). */
  lftpos = lftpos + length(lft)
  rgtpos = pos(rgt, str, lftpos)
  if rgtpos = 0 then rgtpos = length(str) + 1
  str = overlay(copies(" ",  rgtpos - lftpos), str, lftpos)
  start = rgtpos + length(rgt)
  end

return str


/* Determine how numeric digits should be set internally.           */
/* If extra_dgt? = 1 then we double requested precision, using      */
/* the default setting of numeric digits to set a lower limit (so   */
/* that a requested precision of 2, for example, won't result in an */
/* unreasonably low internal precision of 4).  If extra_dgt? = 0    */
/* then we just use the requested precision, whatever it is.        */
intdgt: procedure expose glb.
arg dgt, extra_dgt?
if extra_dgt?
  then return 2 * max(dgt, glb.0dgt) /* Extended precision. */
  else return dgt /* Unextended precision. */


/* Find value of x!. */
fact: procedure expose glb.

x = arg_ok(arg(1), "W >=0")

y = 1
do n = 2 to x
  y = y * n
  end

return y


/* Find the number of permutations of x things taken y at a time. */
perm: procedure expose glb.

x = arg_ok(arg(1), "W >=0")
y = arg_ok(arg(2), "W >=0")
if x < y then call error "Illegal arguments:" x "<" y

z = 1
do n = (x - y) + 1 to x
  z = z * n
  end

return z


/* Find the number of combinations of x things taken y at a time. */
comb: procedure expose glb.

x = arg_ok(arg(1), "W >=0")
y = arg_ok(arg(2), "W >=0")
if x < y then call error "Illegal arguments:" x "<" y

if x - y < y then y = x - y /* Adjust y. */

z = 1
do n = (x - y) + 1 to x
  z = z * n
  end

do n = 2 to y
  z = z / n
  end

return z


/* Find square root using Newton's method. */
sqrt: procedure expose glb.

x = arg_ok(arg(1), ">=0")

if x = 0 then return 0
if x = 1 then return 1

/* Adjust number so it is between 1 and 100. */
i = ilog(100, x, 1, 100)
x = x * 100 ** -i

y = x / 2
old = y
do n = 1
  y = ((x / y) + y) / 2
  if y = old then leave
  old = y
  end

return y * 10 ** i


/* Find x to the power of y. */
pow: procedure expose glb.

x = arg_ok(arg(1))
y = arg_ok(arg(2))

if y = 0 then return 1
if datatype(y, "W") & x \= 0 then return x ** y
if x <= 0 then do
  if x < 0 then call error,
    "Non-integer power of a negative number:" "(" || x || ") **" y
  if y < 0 then call error "Zero taken to a negative power:" y
  return 0
  end

/* If possible use square root function (for speed). */
if abs(y // 1) = 0.5 then return sqrt(x) ** sign(y) * x ** (y % 1)

return exp(y * ln(x))


/* Find the log of y base x. */
log: procedure expose glb.
return ln(arg(2)) / ln(arg_ok(arg(1), "\=1"))


/* Find e to the power of x. */
exp: procedure expose glb.

x = arg_ok(arg(1))

i = x % 1
if abs(x - i) > 0.5 then i = i + sign(x)
x = x - i /* Adjust x for quick convergence. */

y = sum_e(x)
if i \= 0 then y = y * sum_e(1) ** i

return y


/* Find the natural log of x (using sum of -((-1)**n)*(x-1)**n/n). */
ln: procedure expose glb.

x = arg_ok(arg(1), ">0")

if x = 10 then do /* Special assist for ln(10). */
  m = "2.30258509299404568401799145468436420760110148862877"
  y = format(m)
  if y \== m then return y
  end

/* Adjust x between 0.5 and 1.5 (for convergence). */
i = ilog(sum_e(1), x, 0.5, 1.5)
x = x * sum_e(1) ** -i

x = x - 1
f = -1
y = 0
old = y
do n = 1
  f = -f * x
  y = y + f / n
  if y = old then leave
  old = y
  end

return y + i


/* Find e to the power of x (using the usual series expansion).    */
/* If x = 1 then save value of e (because it may be needed again). */
sum_e: procedure expose glb.
parse arg x

if x = 1 then do /* Return hard coded value if precision allows. */
  if symbol("glb.0e") = "VAR" then return glb.0e
  e = "2.71828182845904523536028747135266249775724709369996"
  glb.0e = format(e)
  if glb.0e \== e /* Does hard coded value exceed internal precision? */
    then return glb.0e
  end

y = 1
f = 1
old = y
do n = 1
  f = f * x / n
  y = y + f
  if y = old then leave
  old = y
  end

if x = 1 then glb.0e = y /* Save value of e. */

return y


/* Return i such that bot <= (y / (x ** i)) <= top.               */
/* Conditions are x, y, top, bot > 0, x \= 1, top >= bot          */
/* This procedure works by repeatedly looking for factors of the  */
/* form x ** (2 ** j) which are divided out of y to move it       */
/* toward the intended range (without overshooting, if possible). */
/* Note: if x > 1 then ilog(x, y, 1, x) = trunc(log(x, y)).       */
ilog: procedure expose glb.
arg x, y, bot, top

high = y > top
sign = 1 - 2 * (high \= (x > 1))

i = 0
do while high & y > top | \high & y < bot

  f = x
  do j = -1
    q = y * f ** -sign
    if j >= 0 & (high & q < top | \high & q > bot) then leave
    f = f * f
    g = q
    end

  y = g
  i = i + sign * 2 ** j
  end

return i


/* Calculate (and save) pi, using arcsin(1/2) = pi/6. */
/* Saved or hard coded value of pi used if possible.  */
pi: procedure expose glb.

if symbol("glb.0pi") = "VAR" then return glb.0pi

pi = "3.14159265358979323846264338327950288419716939937511"
glb.0pi = format(pi)

if glb.0pi == pi then glb.0pi = 6 * arcsin(0.5)

return glb.0pi


/* Find the sine of x (usual series expansion). */
sin: procedure expose glb.

x = arg_ok(arg(1), "TRIG")

x = x // (2 * pi()) /* Adjust between pi and -pi, for convergence. */
if abs(x) > pi() then x = x - sign(x) * 2 * pi()

f = x
y = x
x = x ** 2
old = y
do n = 2 by 2
  f = -f * x / (n * (n + 1))
  y = y + f
  if y = old then leave
  old = y
  end

return y


/* Find the cosine of x (usual series expansion). */
cos: procedure expose glb.

x = arg_ok(arg(1), "TRIG")

x = x // (2 * pi()) /* Adjust between pi and -pi, for convergence. */
if abs(x) > pi() then x = x - sign(x) * 2 * pi()

f = 1
y = 1
x = x ** 2
old = y
do n = 2 by 2
  f = -f * x / (n * (n - 1))
  y = y + f
  if y = old then leave
  old = y
  end

return y


/* Find the tangent of x. */
tan: procedure expose glb.

x = arg_ok(arg(1))

y = cos(x)
if y = 0 then call error "Result is infinite."

return sin(x) / y


/* Find the cotangent of x. */
cot: procedure expose glb.

x = arg_ok(arg(1))

y = sin(x)
if y = 0 then call error "Result is infinite."

return cos(x) / y


/* Find the secant of x. */
sec: procedure expose glb.

x = arg_ok(arg(1))

y = cos(x)
if y = 0 then call error "Result is infinite."

return 1 / y


/* Find the cosecant of x. */
csc: procedure expose glb.

x = arg_ok(arg(1))

y = sin(x)
if y = 0 then call error "Result is infinite."

return 1 / y


/* Find the inverse sine of x (usual series expansion). */
arcsin: procedure expose glb.

x = arg_ok(arg(1), "ABS<=1")

/* Avoid region where series converges slowly.  (Note recursion!) */
if abs(x) >= 0.75 then return sign(x) * arccos(sqrt(1 - x ** 2))

f = x
y = x
x = x ** 2
old = y
do n = 2 by 2
  f = f * x * (n - 1) / n
  y = y + f / (n + 1)
  if y = old then leave
  old = y
  end

return y


/* Find the inverse cosine of x. */
arccos: procedure expose glb.
return pi() / 2 - arcsin(arg(1))


/* Find the inverse tangent of x. */
arctan: procedure expose glb.

x = arg_ok(arg(1))

return arcsin(x / sqrt(x ** 2 + 1))


/* Find the inverse cotangent of x. */
arccot: procedure expose glb.

x = arg_ok(arg(1))

return arccos(x / sqrt(x ** 2 + 1))


/* Find the inverse secant of x. */
arcsec: procedure expose glb.

x = arg_ok(arg(1), "ABS>=1")

return sign(x) * arccos(1 / x)


/* Find the inverse cosecant of x. */
arccsc: procedure expose glb.

x = arg_ok(arg(1), "ABS>=1")

if x > 0
  then return arcsin(1 / x)
  else return -(pi() + arcsin(1 / x))


/* Validate function arguments against a given lists of tests. */
arg_ok: procedure expose glb.
parse arg x, test_list
test_list = translate(test_list)

/* Argument must always exist and be a valid number. */
if x = "" then call error "Missing argument."
if \datatype(x, "N") then call error "Illegal argument:" x

if wordpos("W", test_list) > 0 /* Must be whole number. */
  then if \datatype(x, "W")
    then call error "Illegal argument:" x
if wordpos(">=0", test_list) > 0 /* Must be >= zero. */
  then if x < 0
    then call error "Argument < 0:" x
if wordpos(">0", test_list) > 0 /* Must be > zero. */
  then if x <= 0
    then call error "Argument <= 0:" x
if wordpos("\=1", test_list) > 0 /* Must not equal one. */
  then if x = 1
    then call error "Illegal argument:" x
if wordpos("ABS<=1", test_list) > 0 /* Absolute value must be <= one. */
  then if abs(x) > 1
    then call error "Argument is > 1 or < -1:" x
if wordpos("ABS>=1", test_list) > 0 /* Absolute value must be >= one. */
  then if abs(x) < 1
    then call error "Argument is < 1 and > -1:" x

/* To accurately calculate trigonometric functions the absolute value */
/* of the argument must be small enough that we can subtract from it  */
/* a multiple of 2 * pi and get an accurate result between pi and     */
/* -pi.  Actually the test used here is too rigorous.  In most (but   */
/* not all) cases we get a completely accurate result by using        */
/* digits() % 2 (instead of digits() % 4).  But then, who really      */
/* needs sine or cosine of very large numbers anyway?                 */
if wordpos("TRIG", test_list) > 0
  then if abs(x) >= 10 ** (digits() % 4)
    then call error "Argument is too large:" x

return x


halt: call error errortext(4)


syntax: call error errortext(rc)


novalue: call error "NOVALUE at line" sigl


/* Print error message and exit. */
error: procedure expose glb.
if arg(1, "E") then say errmsg(arg(1))
parse source . cmd? .
if cmd? = "COMMAND"
  then exit 1 /* Error return code is 1 when called as a command. */
  else exit   /* If called as function caller will blow up. */


/* Create an error message. */
errmsg: procedure expose glb.
parse source . . pname .
errmsg = "Error in" pname
if symbol("glb.0func") = "VAR" then errmsg = errmsg "("glb.0func")"
errmsg = errmsg "--"
if arg(1, "E") then errmsg = errmsg arg(1)
return errmsg


/* Print out a help message. */
helpmsg:

say "      RXXMATH v1.3 -- Arbitrary Precision Math Functions for REXX"
say "                   Copyright 1992, 1996 by John Brock"
say
say "Syntax:     Command line:   rxxmath [digits] function [x [y]]"
say "            REXX function:  rxxmath([digits,] function [,x [,y]])"
say
say "Where:      digits     is the desired precision of the result"
say "                         (if omitted or zero the NUMERIC DIGITS"
say "                          default, which should be 9, is used)"
say "            function   is the name of a supported function"
say "            x, y       are arguments to the function"
say
say "Functions:  FACT(x)    --  Factorial of x"
say "            PERM(x,y)  --  Permutations of x by y"
say "            COMB(x,y)  --  Combinations of x by y"
say "            SQRT(x)    --  Square root of x"
say "            POW(x,y)   --  x to the power of y"
say "            LOG(x,y)   --  Log of y base x"
say "            EXP(x)     --  e to the power of x"
say "            LN(x)      --  Natural log of x"
say "            PI()       --  Value of pi"
say "            SIN(x)     --  Sine of x"
say "            COS(x)     --  Cosine of x"
say "            TAN(x)     --  Tangent of x"
say "            COT(x)     --  Cotangent of x"
say "            SEC(x)     --  Secant of x"
say "            CSC(x)     --  Cosecant of x"
say "            ARCSIN(x)  --  Inverse sine of x"
say "            ARCCOS(x)  --  Inverse cosine of x"
say "            ARCTAN(x)  --  Inverse tangent of x"
say "            ARCCOT(x)  --  Inverse cotangent of x"
say "            ARCSEC(x)  --  Inverse secant of x"
say "            ARCCSC(x)  --  Inverse cosecant of x"
say "            CALC(x)    --  Calculate the value of an expression"
say "                             (e.g. '1 + exp(pi() / 2)')"
say
say "Notes:      When RXXMATH is called as a command with no arguments"
say "              it prints a help message to the screen."
say "            All trigonometric function arguments are in radians."
say "            When invoked from the command line RXXMATH prints its"
say "              result to the screen and returns 0 (or 1 on error)."
say "            When called as a REXX function RXXMATH returns its"
say "              result to REXX (on error no result is returned)."
say "            When the CALC function is called without an argument"
say "              RXXMATH goes into interpretive REXX calculator mode"
say "              (change REXX variable ""prompt"" to alter prompt)."

exit
