#!/bin/bash

function do_test
{
    echo ""
    echo "********** Starting $1 **********"
    echo ""
    ./$1
    return 0
}


function do_all
{
    do_test test01
    sleep 2
    do_test test02
    sleep 2
    do_test test03
    sleep 2
    do_test test03a
    sleep 2
    do_test test04
    sleep 2
    do_test test04a
    sleep 2
    do_test test05
    sleep 2
    do_test test06
    sleep 2
    do_test test07
    sleep 2
    #test07a is invoked by test07
    do_test test08
    sleep 2
    do_test test09
    sleep 2
    do_test test09a
    sleep 2
    do_test test09b
    sleep 2
    do_test test09b
    sleep 2
    do_test test09c
    sleep 2
    do_test test10
    sleep 2
    #test10a is invoked by test10
    do_test test11
    sleep 2
    #test11a is invoked by test11
    do_test test12
    sleep 2
    return 0
}


export LD_LIBRARY_PATH=../lib

if [ "$#" = "0" ]
then
    do_all
else
    do_test $1
fi
exit 0



