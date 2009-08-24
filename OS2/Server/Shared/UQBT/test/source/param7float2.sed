#######################################################################
# FILE: param7float2.sed
# OVERVIEW: sed script to overcome current limitations of UQBTPS for the
#           param7float.1 test. Operates on the uqbt.h file
#
# (C) 2000-2001 The University of Queensland, BT Group
#######################################################################

# $Revision: 1.1 $
# 08 Mar 01 - Mike: created 

/int32 work(int32 v0, int32 v1, int32 v2, int32 v3, int32 v4, int32 v5, floatmax v6);/{
a\
	/*int32 work(int32 v0, int32 v1, int32 v2, int32 v3, int32 v4, int32 v5, floatmax v6);*/
d
}

