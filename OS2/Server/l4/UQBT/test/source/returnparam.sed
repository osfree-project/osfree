#######################################################################
# FILE: returnparam.sed
# OVERVIEW: sed script to overcome current limitations of UQBTPS for the
#           returnparam.1 test
#
# (C) 2000-2001 The University of Queensland, BT Group
#######################################################################

# $Revision: 1.5 $
# 08 Mar 01 - Mike: Updated for spaces in the backend
# 23 Mar 01 - Mike: Fixed the alignment and swap issues, so first part of the
#               edit is no longer needed
# 30 Mar 01 - Mike: Now that only memory expressions are converted to vars, the
#               var numbers have changed
# 26 Oct 01 - Mike: No longer need to change the parameters to getFirstStr;
#               var numbers changed again (sigh)

# Needed because one parameter is left on the stack after the call to
# the getFirstStr function. We assumed that this would never happen!
s/v9,v8/v9,v8, r27 /
