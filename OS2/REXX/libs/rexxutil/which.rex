/* which for NT/Regina
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/which.rex,v 1.3 2004/08/21 14:48:44 prokushev Exp $
 */
parse arg what '.' .
parse upper arg . '.' ext .

/* read in the path */
path = value('path',,'ENVIRONMENT');
exts = translate(changestr(';', value('pathext',,'ENVIRONMENT'), ' '))

/* don't mind these */
matched = 0
whichmatch. = 0

/* need to add a utility function */
call RxFuncAdd 'sysfiletree', 'rexxutil', 'sysfiletree'

/* now loop through each entry in it */
thisdir = "."
do until path = ""
    /* see if there's a match in this directory */
    rcc = SysFileTree(thisdir || '\' || what || '.*', FileName, "FO")

    /* we're really only interested in the extension */
    do i = 1 to FileName.0
	parse upper var FileName.i . '.' Extension

	/* exact match -- get out of the loop */
	if ext \= "" then do
	    if Extension = ext then do
		matched = 1
		whichmatch. = i
		i = FileName.0
		end
	    end
	else do
	    mp = wordpos('.'Extension, exts)
	    if mp > 0 then do
	        matched = 1
	        whichmatch.mp = i
                end
	    end
	end

    /* let's try the next directory */
    if matched = 0 then do
	thisdir = ''
	do while thisdir = '' & path \= ''
	    parse var path thisdir ';' path
	    end
	end
    else
        path = ""
    end


    if matched then
	do i = 1 to words(exts)
	    if whichmatch.i \= 0 then do
	       j = whichmatch.i
	       say FileName.j
	       leave
	       end
	    end

    else do
        if ext \= "" then
            say 'No match for' what || '.' || ext
        else
            say 'No match for' what
        end
