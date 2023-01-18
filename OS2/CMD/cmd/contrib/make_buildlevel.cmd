/*  REXX script to generate a buildlevel string
    creates a buildlevel.txt file which can be linked with
    wlink ... option description @..\buildlevel.txt

    For use with 4os2. Uses enviroment variable %HOSTNAME% as build machine entry.
    Optionally uses enviroment variable %VENDOR% as vendor name entry.
    Date/Time is extracted from msg.obj to match ver /r output.
    Version is taken from version.h and build.h

    20100515 AB initial
    20111115 AB added VER_REVISION
    2017-02-18 SHL Allow leading dot in VER_REVISION
    2019-05-13 SHL Ignore comment lines
*/

/* Use value from environment (no more than 31 chars) or default to AB */
VendorName = value('VENDOR',,'OS2ENVIRONMENT')
if VendorName == '' THEN VendorName = 'AB'

BuildMachine = value('HOSTNAME',,'OS2ENVIRONMENT')
ASD      = ''
Language = ''           /* f.i. EN or empty string */
Country  = ''           /* f.i. AT or empty string */
CPU      = ''           /* U for uniprocessor or empty string */
FixPack  = ''           /* */
Description = '4os2 Command Shell Extension - http://4os2.netlabs.org/'

/* process command line parameters
called with trace or t gives trace (debug) messages
called with TRACE or T gives even more debug info   */
fTrace = 0
IF LEFT(ARG(1), 1) = 't' THEN fTrace = 1
IF LEFT(ARG(1), 1) = 'T' THEN fTrace = 2
/* SAY LEFT(ARG(1), 1) */

/* limit vendor name */
IF LENGTH(VendorName) > 31 THEN DO
    VendorName = LEFT(VendorName,31)
END

/* get version info from version.h */
file = '..\h\version.h'
IF fTrace > 0 THEN SAY 'Lines='lines(file)
DO WHILE lines(file) > 0
    line = LINEIN(file)
    line = STRIP(line)
    IF LEFT(line, 2) == '//' THEN ITERATE
    IF LEFT(line, 2) == '/*' THEN ITERATE
    pos = WORDPOS('VER_MAJOR' , line)
    IF pos > 0 THEN DO
        ver_maj = WORD(line, pos + 1)
        IF fTrace > 0 THEN SAY 'Major='ver_maj
    END
    pos = WORDPOS('VER_MINOR' , line)
    IF pos > 0 THEN DO
        ver_min = WORD(line, pos + 1)
        IF fTrace > 0 THEN SAY 'Minor='ver_min
    END
    pos = WORDPOS('VER_REVISION' , line)
    IF pos > 0 THEN DO
        ver_rev = WORD(line, pos + 1)
        /* Strip quotes */
        ver_rev = LEFT(ver_rev, WORDLENGTH(ver_rev, 1) - 1);
        ver_rev = RIGHT(ver_rev, WORDLENGTH(ver_rev, 1) - 1);
        IF fTrace > 0 THEN SAY 'Revision='ver_rev
    END
END

/* OS2_REVISION (see msg.c) does not supply leading dot - VER_REVSION may have leading dot */
if LEFT(ver_rev, 1) == '.' THEN ver_rev = SUBSTR(ver_rev, 2) /* Strip dot */
IF WORDLENGTH(ver_rev, 1) > 0 THEN ver_min = ver_min'.'ver_rev
IF fTrace > 0 THEN SAY 'Version='ver_maj'.'ver_min

/* get build level from build.h */
file = '..\h\build.h'
DO WHILE lines(file) > 0
    line = LINEIN(file)
    pos = WORDPOS('VER_BUILD', line)
    IF pos > 0 THEN DO
        ver_build = WORD(line, pos + 1)
        IF fTrace > 0 THEN SAY 'Build='ver_build
    END
END
IF LENGTH(ver_build) > 7 THEN ver_build = LEFT(ver_build,7)

/* extract compile date/time from msg.obj */
file = 'msg.obj'
SearchText = '4OS2 - Build'
SearchTextOffs = LENGTH(SearchText) + 1
DO WHILE lines(file) > 0
    line = LINEIN(file)
    pos = POS(SearchText , line)
    IF pos > 0 THEN DO
        line = SUBSTR(line, pos + SearchTextOffs, 40)  /* max. 20 characters for DEBUG string */
        IF fTrace > 0 THEN SAY 'Date='line
        month = WORD(line, 1)
        day = WORD(line, 2)
        year = WORD(line, 3)
        time = WORD(line, 4)
        month=WORDPOS(month, 'Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec')
        IF LENGTH(month) < 2 THEN month = INSERT('0',month)
        IF LENGTH(day) < 2 THEN day = INSERT('0',day)
        IF fTrace > 1 THEN SAY 'Day='day' Month='month' Year='year
        /* check if it's a debug build */
        IF POS('DEBUG_BUILD', line) > 0 THEN DO
            IF fTrace > 0 THEN SAY 'Debug build'
            ver_min=ver_min'.DEBUG'
            END
    END
END

/* process time */
time = TRANSLATE(time, ' ', ':')
hour = WORD(time, 1)
minu=  WORD(time, 2)
IF LENGTH(hour) < 2 THEN hour = INSERT('0',hour)
IF LENGTH(minu) < 2 THEN minu = INSERT('0',minu)
time = hour':'minu

datetime = year'-'month'-'day' 'time
IF fTrace > 1 THEN SAY 'datetime='datetime

/* compose Feature string (:ASD:EN:US:4b:U:8101) */
Feature=':'ASD':'Language':'Country':'ver_build':'CPU':'FixPack
/* SAY 'Feature='Feature */

/* build time stamp und build machine string */
/* date/time have to be 26 chars (leading ' ' required) */
IF LENGTH(BuildMachine) > 11 THEN DO
    BuildMachine = LEFT(BuildMachine, 11)
    END
bldmachlen = LENGTH(BuildMachine)
IF fTrace > 1 THEN SAY 'bldmach='BuildMachine'<-- len='bldmachlen
datimmach = LEFT(datetime, 24)
datimmach = datimmach' 'BuildMachine
IF fTrace > 1 THEN SAY datimmach'<--'

/* bldlevel do not display more than 79 characters */
IF LENGTH(Description) > 79 THEN DO
    Description = LEFT(Description, 79)
    SAY 'Description lenght corrected'
    END


FullString = "'@#"VendorName':'ver_maj'.'ver_min'#@##1## 'datimmach''Feature'@@  'Description"'"
'@CALL del ..\buildlevel.txt 2>NUL >NUL'
file = '..\buildlevel.txt'
IF LINEOUT(file,FullString,1) <> 0 THEN SAY "ERROR - can't write buildlevel.txt"
ELSE SAY FullString

