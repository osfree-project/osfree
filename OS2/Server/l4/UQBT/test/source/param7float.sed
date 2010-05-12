#######################################################################
# FILE: param7float.sed
# OVERVIEW: sed script to overcome current limitations of UQBTPS for the
#           param7float.1 test. Operates on the work.c file
#
# (C) 2000-2001 The University of Queensland, BT Group
#######################################################################

# $Revision: 1.3 $
# 08 Mar 01 - Mike: New problem causes last parameter to be declared as floatmax

# Needed for pentium param7float
# Fix the return value problem, caused by the fact that xor eax, eax appears
# to use its operand
s/r24=//
s/return r24/return 0/

# There is a new problem (as of March 2001) that causes the last parameter
# of work() to be declared as a floatmax.
s/floatmax v6/float v6/

# We also need to kill the prototype for work in uqbt.h; this is done in
# param7float2.sed
