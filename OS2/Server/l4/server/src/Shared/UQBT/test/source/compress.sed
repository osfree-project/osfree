#######################################################################
# FILE: compress.sed
# OVERVIEW: sed script to overcome current limitations of UQBTPS for the
#           compress.1 test. Operates on main.c
#
# (C) 2001 The University of Queensland, BT Group
#######################################################################

# $Revision: 1.1 $

/scanf/ {
a\
*(int*)v3=_swap4(*(int*)v3); \
*(int*)v5=_swap4(*(int*)v5);
}
