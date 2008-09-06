/* */

parse arg arg

if arg == ""
then
    call do_all
else
    call do_test arg
exit 0

do_test: procedure
parse arg arg
    rc=do_compile(arg)
    if rc=0 then
    do
      say ""
      say "********** Starting "||arg||" **********"
      say ""
      '@'||arg
    end
return 0

do_compile: procedure
parse arg arg
    say ""
    say "********** Compile "||arg||" **********"
    say ""
    '@wcl386 -q '||arg
return rc

do_all: procedure
    call do_test test01
    call do_test test02
    call do_test test03
    call do_test test03a
    call do_test test04
    call do_test test04a
    call do_test test05
    call do_test test06
    call do_compile test07a
    call do_test test07
    /* test07a is invoked by test07 */
    call do_test test08
    call do_test test09
    call do_test test09a
    call do_test test09b
    call do_test test09b
    call do_test test09c
    call do_compile test10a
    call do_test test10
    /* test10a is invoked by test10 */
    call do_compile test11a
    call do_test test11
    /* test11a is invoked by test11 */
    call do_test test12
    call do_test test13
    call do_test test14
    call do_test test15
    call do_test test16
    call do_test test17
    call do_test test18
    call do_test test19
    call do_test test20
    call do_test test21
return 0
