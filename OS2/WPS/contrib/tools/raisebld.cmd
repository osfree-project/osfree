/* */
/*  RAISEBLD.CMD (new with V1.00):
 *      this little script reads in the "build.h" file
 *      and raises the XWorkplace build level by one.
 *      This gets called by xfldr.mak after each successful
 *      link of the whole XWorkplace thing.
 *
 *      Copyright (C) 1999 Ulrich M”ller.
 *      This file is part of the XFolder source package.
 *      XFolder is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XFolder main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

LineCount = 0;
InputFile = "";

parse arg args

if args \= "" then
    InputFileName = args
else
    InputFileName = "build.h";

InputFile = linein(InputFileName);
parse var InputFile define ident level;
call stream InputFileName, 'c', 'close';

say 'old level: "'level'"';

Newlevel = abs(Level)+1;

Output = define" "ident' 'Newlevel;
Say 'new line: 'Output

"@del" InputFileName;
call stream InputFileName, 'c', 'open write';
rc = lineout(InputFileName, Output);
call stream InputFileName, 'c', 'close';


