#######################################################################
# FILE: switch_formh.sed
# OVERVIEW: sed script to overcome current limitations of UQBTSS for the
#           switch.7 and switch.8 tests. Operates on the main.c file
#
# (C) 1999-2001 The University of Queensland, BT Group
#######################################################################

# $Revision: 1.2 $
/goto L1;/ {
# v0 is the switch variable; r8 here has the correct value.
# L2 is a label where the switch statement itself starts.
# So this modification overcomes the problem of not being able
# to recognise what parts of the code have to be removed or irnored.
# In the case of form H, this is a very considerable body of code.
a\
v0=r8; goto L2;
} 
