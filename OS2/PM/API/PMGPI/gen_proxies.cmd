/*! @file gen_proxies.cmd
    @brief Generate client-side PMGPI.DLL proxy stubs from pmgpi.h.

    Usage:  rexx gen_proxies.cmd [pmgpi.h]

    Outputs:
      gpi_wire.h       - NGpiXxx codes
      gpi_proxies.c    - GpiXxx client proxy stubs (auto-marshaled)
      gpi_manual.c     - GpiXxx client proxy stubs (manual, return GPI_ERROR)
      gpi_entry.c      - GpiDispatch stub
      gpi_entry.h      - GpiDispatch prototype
      dispatch.c       - server DispatchGpi stub
      needs_manual.txt - list of manual functions

    Header convention (OS/2): #define INCL_GPI, then #include <os2.h>.
    pmgpi.h is NOT included directly.
*/

parse arg hFile .
if hFile = '' then hFile = 'pmgpi.h'

fnCount = 0

call DeleteOutput 'gpi_wire.h'
call DeleteOutput 'gpi_proxies.c'
call DeleteOutput 'gpi_manual.c'
call DeleteOutput 'gpi_entry.c'
call DeleteOutput 'gpi_entry.h'
call DeleteOutput 'dispatch.c'
call DeleteOutput 'needs_manual.txt'

/* ---- Parse pmgpi.h ---- */
buf = ''
do while lines(hFile) > 0
    line = linein(hFile)
    do forever
        p = pos('/*', line)
        if p = 0 then leave
        q = pos('*/', line, p + 2)
        if q = 0 then do
            line = substr(line, 1, p - 1)
            leave
        end
        line = substr(line, 1, p - 1) || ' ' || substr(line, q + 2)
    end
    line = strip(line)
    if line = '' then iterate
    if left(line, 1) = '#' then iterate
    if buf = '' then buf = line
    else buf = buf || ' ' || line
    if pos(';', buf) = 0 then iterate
    call parseProto
    buf = ''
end
call stream hFile, 'C', 'CLOSE'

say 'Parsed functions:' fnCount

/* ---- gpi_wire.h ---- */
call lineout 'gpi_wire.h', '/*!'
call lineout 'gpi_wire.h', ' * @file gpi_wire.h'
call lineout 'gpi_wire.h', ' * @brief Wire codes for the PMGPI client-server protocol.'
call lineout 'gpi_wire.h', ' */'
call lineout 'gpi_wire.h', '#ifndef GPI_WIRE_H_INCLUDED'
call lineout 'gpi_wire.h', '#define GPI_WIRE_H_INCLUDED'
call lineout 'gpi_wire.h', ''
codeNum = 1
do i = 1 to fnCount
    oname = fn.i.name
    hex = d2x(codeNum, 8)
    pad = 32 - length(oname)
    if pad < 1 then pad = 1
    call lineout 'gpi_wire.h', '#define N' || oname || copies(' ', pad) || '0x' || hex || 'L'
    fn.i.code = codeNum
    codeNum = codeNum + 1
end
call lineout 'gpi_wire.h', ''
call lineout 'gpi_wire.h', '#endif'
call lineout 'gpi_wire.h'

/* ---- gpi_proxies.c ---- */
call lineout 'gpi_proxies.c', '/*!'
call lineout 'gpi_proxies.c', ' * @file gpi_proxies.c'
call lineout 'gpi_proxies.c', ' * @brief Auto-generated client-side PMGPI proxies.'
call lineout 'gpi_proxies.c', ' */'
call lineout 'gpi_proxies.c', '#define INCL_GPI'
call lineout 'gpi_proxies.c', '#include <os2.h>'
call lineout 'gpi_proxies.c', '#include "gpi_wire.h"'
call lineout 'gpi_proxies.c', '#include "gpi_entry.h"'
call lineout 'gpi_proxies.c', ''

/* ---- gpi_manual.c ---- */
call lineout 'gpi_manual.c', '/*!'
call lineout 'gpi_manual.c', ' * @file gpi_manual.c'
call lineout 'gpi_manual.c', ' * @brief Hand-written client-side PMGPI proxies (stubs).'
call lineout 'gpi_manual.c', ' */'
call lineout 'gpi_manual.c', '#define INCL_GPI'
call lineout 'gpi_manual.c', '#include <os2.h>'
call lineout 'gpi_manual.c', '#include "gpi_entry.h"'
call lineout 'gpi_manual.c', ''

/* ---- needs_manual.txt ---- */
call lineout 'needs_manual.txt', '/* Functions NOT auto-marshaled. Implemented as stubs in gpi_manual.c */'
call lineout 'needs_manual.txt', ''

autoCount   = 0
manualCount = 0

do i = 1 to fnCount
    fname_i = fn.i.name
    fargs_i = fn.i.args
    if isAuto(fname_i, fargs_i) = 1 then do
        autoCount = autoCount + 1
        call genAuto i
    end
    else do
        manualCount = manualCount + 1
        call lineout 'needs_manual.txt', fname_i || '(' || fargs_i || ')'
        call genManual i
    end
end

call lineout 'gpi_proxies.c'
call lineout 'gpi_manual.c'

/* ---- gpi_entry.h ---- */
call lineout 'gpi_entry.h', '/*!'
call lineout 'gpi_entry.h', ' * @file gpi_entry.h'
call lineout 'gpi_entry.h', ' * @brief Client-side dispatcher interface.'
call lineout 'gpi_entry.h', ' */'
call lineout 'gpi_entry.h', '#ifndef GPI_ENTRY_H_INCLUDED'
call lineout 'gpi_entry.h', '#define GPI_ENTRY_H_INCLUDED'
call lineout 'gpi_entry.h', ''
call lineout 'gpi_entry.h', '#define INCL_GPI'
call lineout 'gpi_entry.h', '#include <os2.h>'
call lineout 'gpi_entry.h', ''
call lineout 'gpi_entry.h', '/*!'
call lineout 'gpi_entry.h', ' * @brief Client-side dispatcher.'
call lineout 'gpi_entry.h', ' * @param [in] hps Presentation space handle.'
call lineout 'gpi_entry.h', ' * @param [in] ulFnCode Function code (NGpi*).'
call lineout 'gpi_entry.h', ' * @param [in] pArgs Marshaled arguments.'
call lineout 'gpi_entry.h', ' * @param [in] cbArgs Size of pArgs in bytes.'
call lineout 'gpi_entry.h', ' * @param [out] pulResult Receives result from server.'
call lineout 'gpi_entry.h', ' * @return 0 on success, non-zero on transport error.'
call lineout 'gpi_entry.h', ' * @retval 1 Transport error (stub).'
call lineout 'gpi_entry.h', ' */'
call lineout 'gpi_entry.h', 'ULONG GpiDispatch(HPS hps, ULONG ulFnCode,'
call lineout 'gpi_entry.h', '                  const void *pArgs, ULONG cbArgs,'
call lineout 'gpi_entry.h', '                  ULONG *pulResult);'
call lineout 'gpi_entry.h', ''
call lineout 'gpi_entry.h', '#endif'
call lineout 'gpi_entry.h'

/* ---- gpi_entry.c (stub) ---- */
call lineout 'gpi_entry.c', '/*!'
call lineout 'gpi_entry.c', ' * @file gpi_entry.c'
call lineout 'gpi_entry.c', ' * @brief Client-side dispatcher (stub).'
call lineout 'gpi_entry.c', ' */'
call lineout 'gpi_entry.c', '#define INCL_GPI'
call lineout 'gpi_entry.c', '#include <os2.h>'
call lineout 'gpi_entry.c', '#include "gpi_entry.h"'
call lineout 'gpi_entry.c', ''
call lineout 'gpi_entry.c', '/*!'
call lineout 'gpi_entry.c', ' * @brief Client dispatcher (stub).'
call lineout 'gpi_entry.c', ' * @param [in] hps Presentation space handle.'
call lineout 'gpi_entry.c', ' * @param [in] ulFnCode Function code (NGpi*).'
call lineout 'gpi_entry.c', ' * @param [in] pArgs Marshaled arguments.'
call lineout 'gpi_entry.c', ' * @param [in] cbArgs Size of pArgs in bytes.'
call lineout 'gpi_entry.c', ' * @param [out] pulResult Receives result from server.'
call lineout 'gpi_entry.c', ' * @return 0 on success, non-zero on transport error.'
call lineout 'gpi_entry.c', ' * @retval 1 Transport error (until real transport is wired).'
call lineout 'gpi_entry.c', ' */'
call lineout 'gpi_entry.c', 'ULONG GpiDispatch(HPS hps, ULONG ulFnCode,'
call lineout 'gpi_entry.c', '                  const void *pArgs, ULONG cbArgs,'
call lineout 'gpi_entry.c', '                  ULONG *pulResult)'
call lineout 'gpi_entry.c', '{'
call lineout 'gpi_entry.c', '    (void)hps;'
call lineout 'gpi_entry.c', '    (void)ulFnCode;'
call lineout 'gpi_entry.c', '    (void)pArgs;'
call lineout 'gpi_entry.c', '    (void)cbArgs;'
call lineout 'gpi_entry.c', '    *pulResult = 0;'
call lineout 'gpi_entry.c', '    return 1;'
call lineout 'gpi_entry.c', '}'
call lineout 'gpi_entry.c'

/* ---- dispatch.c (server stub) ---- */
call lineout 'dispatch.c', '/*!'
call lineout 'dispatch.c', ' * @file dispatch.c'
call lineout 'dispatch.c', ' * @brief Server-side DispatchGpi stub.'
call lineout 'dispatch.c', ' */'
call lineout 'dispatch.c', '#define INCL_GPI'
call lineout 'dispatch.c', '#include <os2.h>'
call lineout 'dispatch.c', '#include "gpi_wire.h"'
call lineout 'dispatch.c', ''
call lineout 'dispatch.c', '/*!'
call lineout 'dispatch.c', ' * @brief Server dispatch (stub: returns GPI_ERROR).'
call lineout 'dispatch.c', ' * @param [in] hps Presentation space handle.'
call lineout 'dispatch.c', ' * @param [in] ulFnCode Function code (NGpi*).'
call lineout 'dispatch.c', ' * @param [in] pArgs Marshaled arguments.'
call lineout 'dispatch.c', ' * @param [in] cbArgs Size of pArgs in bytes.'
call lineout 'dispatch.c', ' * @return GPI_ERROR (stub: not implemented).'
call lineout 'dispatch.c', ' */'
call lineout 'dispatch.c', 'ULONG DispatchGpi(HPS hps, ULONG ulFnCode,'
call lineout 'dispatch.c', '                  PBYTE pArgs, ULONG cbArgs)'
call lineout 'dispatch.c', '{'
call lineout 'dispatch.c', '    (void)hps; (void)pArgs; (void)cbArgs;'
call lineout 'dispatch.c', ''
call lineout 'dispatch.c', '    switch (ulFnCode) {'
do i = 1 to fnCount
    call lineout 'dispatch.c', '    case N' || fn.i.name || ':'
    call lineout 'dispatch.c', '        return (ULONG)GPI_ERROR;'
end
call lineout 'dispatch.c', '    default:'
call lineout 'dispatch.c', '        return (ULONG)GPI_ERROR;'
call lineout 'dispatch.c', '    }'
call lineout 'dispatch.c', '}'
call lineout 'dispatch.c'

say 'Auto-marshaled:' autoCount
say 'Manual (stubbed):' manualCount
say 'Done.'
exit 0

DeleteOutput:
    procedure
    fn = arg(1)
    if stream(fn, 'C', 'QUERY EXISTS') \= '' then
        call SysFileDelete fn
    return

parseProto:
    procedure expose fnCount fn. buf seen.
    p = pos('APIENTRY', buf)
    if p = 0 then return
    retType = strip(substr(buf, 1, p - 1))
    rest    = substr(buf, p + 8)
    po = pos('(', rest)
    if po = 0 then return
    fnName = strip(substr(rest, 1, po - 1))
    if left(fnName, 3) \= 'Gpi' then return
    if seen.fnName = 1 then return
    seen.fnName = 1
    depth = 0
    closeP = 0
    j = po
    do while j <= length(rest)
        c = substr(rest, j, 1)
        if c = '(' then depth = depth + 1
        else if c = ')' then do
            depth = depth - 1
            if depth = 0 then do
                closeP = j
                leave
            end
        end
        j = j + 1
    end
    if closeP = 0 then return
    arglist = strip(substr(rest, po + 1, closeP - po - 1))
    if translate(arglist) = 'VOID' then arglist = ''
    fnCount = fnCount + 1
    fn.fnCount.name = fnName
    fn.fnCount.ret  = retType
    fn.fnCount.args = arglist
return

inScalars:
    procedure
    return wordpos(arg(1), "LONG ULONG BOOL FIXED SHORT USHORT BYTE CHAR HPS HDC HBITMAP HRGN HAB HMF HPAL HWND HMQ HMODULE LONG32 ULONG32 APIRET") > 0

inStructs:
    procedure
    return wordpos(arg(1), "POINTL RECTL SIZEL SIZEF POINTS MATRIXLF ARCPARAMS GRADIENTL FATTRS FONTMETRICS KERNINGPAIRS RGB RGB2 HCINFO BITMAPINFOHEADER2") > 0

inPtrStructs:
    procedure
    return wordpos(arg(1), "PPOINTL PRECTL PSIZEL PSIZEF PGRADIENTL PMATRIXLF PARCPARAMS PFATTRS PFONTMETRICS PKERNINGPAIRS") > 0

inPtrs:
    procedure
    return wordpos(arg(1), "PPOINTL PRECTL PSIZEL PSIZEF PGRADIENTL PMATRIXLF PARCPARAMS PFATTRS PFONTMETRICS PKERNINGPAIRS PLONG PULONG PBYTE PCH PSTR8 PSZ PVOID") > 0

ptrTarget:
    procedure
    t = arg(1)
    if t = 'PPOINTL'       then return 'POINTL'
    if t = 'PRECTL'        then return 'RECTL'
    if t = 'PSIZEL'        then return 'SIZEL'
    if t = 'PSIZEF'        then return 'SIZEF'
    if t = 'PGRADIENTL'    then return 'GRADIENTL'
    if t = 'PMATRIXLF'     then return 'MATRIXLF'
    if t = 'PARCPARAMS'    then return 'ARCPARAMS'
    if t = 'PFATTRS'       then return 'FATTRS'
    if t = 'PFONTMETRICS'  then return 'FONTMETRICS'
    if t = 'PKERNINGPAIRS' then return 'KERNINGPAIRS'
    return 'ULONG'

isCountName:
    procedure
    t = translate(arg(1))
    if t = 'LCOUNT'     then return 1
    if t = 'CCOUNT'     then return 1
    if t = 'CCH'        then return 1
    if t = 'CB'         then return 1
    if t = 'CPOINTS'    then return 1
    if t = 'CBELEMENTS' then return 1
    if t = 'CBDATA'     then return 1
    if t = 'ULCOUNT'    then return 1
    if t = 'USCOUNT'    then return 1
    if t = 'LLENGTH'    then return 1
    if t = 'LMAX'       then return 1
    if t = 'CCHSTRING'  then return 1
    return 0

isAuto:
    procedure
    fname   = arg(1)
    arglist = arg(2)
    if arglist = '' then return 0
    if argType(arglist, 1) \= 'HPS' then return 0
    isQuery = 0
    if left(fname, 8) = 'GpiQuery' then isQuery = 1
    if left(fname, 6) = 'GpiGet'   then isQuery = 1
    n = countArgs(arglist)
    sawCount = 0
    do j = 2 to n
        at = argType(arglist, j)
        an = argName(arglist, j)
        if at = an then return 0
        if inScalars(at) = 1 then do
            if isCountName(an) = 1 then sawCount = 1
            else sawCount = 0
            iterate
        end
        if inStructs(at) = 1 then do
            sawCount = 0
            iterate
        end
        if inPtrStructs(at) = 1 then do
            if isQuery = 1 then return 0
            if left(an, 1) = 'a' then return 0
            if sawCount = 1 then return 0
            sawCount = 0
            iterate
        end
        return 0
    end
    return 1

paramDir:
    procedure
    pname = arg(1)
    ptype = arg(2)
    pfn   = arg(3)
    t = translate(ptype)
    isQuery = 0
    if left(pfn, 8) = 'GpiQuery' then isQuery = 1
    if left(pfn, 6) = 'GpiGet'   then isQuery = 1
    isOut = 0
    if inPtrs(t) = 1 then do
        if isQuery = 1 then isOut = 1
    end
    if isOut = 1 then return '[out]'
    return '[in]'

doxyReturn:
    procedure
    r = translate(arg(1))
    if r = 'BOOL' then return 'TRUE on success, FALSE on error.'
    if r = 'LONG' then return 'GPI_OK, GPI_HITS, or GPI_ERROR.'
    if r = 'ULONG' then return 'Implementation-defined value.'
    if r = 'HPS' then return 'Handle, or GPI_ERROR.'
    if r = 'HDC' then return 'Handle, or GPI_ERROR.'
    if r = 'HBITMAP' then return 'Handle, or GPI_ERROR.'
    if r = 'HRGN' then return 'Handle, or GPI_ERROR.'
    if r = 'HPAL' then return 'Handle, or GPI_ERROR.'
    if r = 'HMF' then return 'Handle, or GPI_ERROR.'
    if r = 'FIXED' then return 'Fixed value, or GPI_ERROR.'
    return 'Implementation-defined.'

genAuto:
    procedure expose fn.
    i    = arg(1)
    name = fn.i.name
    ret  = fn.i.ret
    args = fn.i.args
    n    = countArgs(args)
    hpsname = argName(args, 1)
    sigargs = args
    if hpsname = argType(args, 1) then do
        hpsname = 'hps'
        if n = 1 then
            sigargs = 'HPS hps'
        else do
            rest = substr(args, length(argType(args, 1)) + 1)
            if left(rest, 1) = ',' then
                sigargs = 'HPS hps' || rest
            else
                sigargs = 'HPS hps, ' || strip(rest)
        end
    end
    call lineout 'gpi_proxies.c', ''
    call lineout 'gpi_proxies.c', '/*!'
    call lineout 'gpi_proxies.c', ' * @brief Client proxy for ' || name || '.'
    do j = 1 to n
        at = argType(args, j)
        an = argName(args, j)
        if at = an then an = 'hps'
        call lineout 'gpi_proxies.c', ' * @param ' || paramDir(an, at, name) || ' ' || an || ' ' || at || '.'
    end
    call lineout 'gpi_proxies.c', ' * @return ' || doxyReturn(ret)
    call lineout 'gpi_proxies.c', ' */'
    call lineout 'gpi_proxies.c', ret || ' APIENTRY ' || name || '(' || sigargs || ')'
    call lineout 'gpi_proxies.c', '{'
    if n > 1 then do
        call lineout 'gpi_proxies.c', '    struct {'
        do j = 2 to n
            at = argType(args, j)
            an = argName(args, j)
            if inPtrStructs(at) = 1 then
                call lineout 'gpi_proxies.c', '        ' || ptrTarget(at) || ' ' || an || ';'
            else
                call lineout 'gpi_proxies.c', '        ' || at || ' ' || an || ';'
        end
        call lineout 'gpi_proxies.c', '    } args;'
    end
    call lineout 'gpi_proxies.c', '    ULONG result;'
    call lineout 'gpi_proxies.c', '    int   rc;'
    call lineout 'gpi_proxies.c', ''
    if n > 1 then do
        do j = 2 to n
            at = argType(args, j)
            an = argName(args, j)
            if inPtrStructs(at) = 1 then
                call lineout 'gpi_proxies.c', '    args.' || an || ' = *' || an || ';'
            else
                call lineout 'gpi_proxies.c', '    args.' || an || ' = ' || an || ';'
        end
    end
    if n > 1 then do
        call lineout 'gpi_proxies.c', '    rc = GpiDispatch(' || hpsname || ', N' || name || ','
        call lineout 'gpi_proxies.c', '                      &args, sizeof(args), &result);'
    end
    else do
        call lineout 'gpi_proxies.c', '    rc = GpiDispatch(' || hpsname || ', N' || name || ','
        call lineout 'gpi_proxies.c', '                      NULL, 0, &result);'
    end
    call lineout 'gpi_proxies.c', '    if (rc != 0) return (' || ret || ')GPI_ERROR;'
    call lineout 'gpi_proxies.c', '    return (' || ret || ')result;'
    call lineout 'gpi_proxies.c', '}'
return

genManual:
    procedure expose fn.
    i    = arg(1)
    name = fn.i.name
    ret  = fn.i.ret
    args = fn.i.args
    n    = countArgs(args)
    sigargs = ''
    do j = 1 to n
        at = argType(args, j)
        an = argName(args, j)
        if at = an then
            an = 'p' || j
        if j > 1 then sigargs = sigargs || ', '
        sigargs = sigargs || at || ' ' || an
    end
    if n = 0 then sigargs = args
    call lineout 'gpi_manual.c', ''
    call lineout 'gpi_manual.c', '/*!'
    call lineout 'gpi_manual.c', ' * @brief ' || name || ' (stub).'
    do j = 1 to n
        at = argType(args, j)
        an = argName(args, j)
        if at = an then an = 'p' || j
        call lineout 'gpi_manual.c', ' * @param ' || paramDir(an, at, name) || ' ' || an || ' ' || at || '.'
    end
    call lineout 'gpi_manual.c', ' * @return ' || doxyReturn(ret)
    call lineout 'gpi_manual.c', ' */'
    call lineout 'gpi_manual.c', ret || ' APIENTRY ' || name || '(' || sigargs || ')'
    call lineout 'gpi_manual.c', '{'
    do j = 1 to n
        at = argType(args, j)
        an = argName(args, j)
        if at = an then
            call lineout 'gpi_manual.c', '    (void)p' || j || ';'
    end
    call lineout 'gpi_manual.c', '    return (' || ret || ')GPI_ERROR;'
    call lineout 'gpi_manual.c', '}'
return

countArgs:
    procedure
    a = arg(1)
    if a = '' then return 0
    n = 0
    depth = 0
    do j = 1 to length(a)
        c = substr(a, j, 1)
        if c = '(' then depth = depth + 1
        else if c = ')' then depth = depth - 1
        else if c = ',' & depth = 0 then n = n + 1
    end
    return n + 1

argType:
    procedure
    s = strip(nthArg(arg(1), arg(2)))
    p = lastSpace(s)
    if p = 0 then return s
    return strip(substr(s, 1, p - 1))

argName:
    procedure
    s = strip(nthArg(arg(1), arg(2)))
    p = lastSpace(s)
    if p = 0 then return s
    return strip(substr(s, p + 1))

nthArg:
    procedure
    rest = arg(1)
    k    = arg(2)
    do j = 1 to k - 1
        p = nextComma(rest)
        if p = 0 then return ''
        rest = substr(rest, p + 1)
    end
    p = nextComma(rest)
    if p > 0 then rest = substr(rest, 1, p - 1)
    return rest

nextComma:
    procedure
    s = arg(1)
    depth = 0
    do j = 1 to length(s)
        c = substr(s, j, 1)
        if c = '(' then depth = depth + 1
        else if c = ')' then depth = depth - 1
        else if c = ',' & depth = 0 then return j
    end
    return 0

lastSpace:
    procedure
    s = arg(1)
    p = 0
    do j = 1 to length(s)
        if substr(s, j, 1) = ' ' then p = j
    end
    return p
