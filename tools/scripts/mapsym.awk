# $Id: mapsym.awk,v 1.1.1.1 2003/10/04 08:40:16 prokushev Exp $
#
# Converts WLINK map files for processing with IBM's MAPSYM.
# WARNING: It has only been briefly tested with 32-bit LX output.
# Feb 27, 2014, valerius
# Added support for 16-bit NE executables

BEGIN {
 group=1
 segment=2
 memory=3
}

/^Executable Image:/ {
 sub("\\..*$", "", $3)
 printf "\n %s\n\n", $3
}

/^Entry point address:/ {
 printf "\nProgram entry point at %s", $4
}

/^Group *Address *Size/ {
 mode=group
 groups=0
}

/^Segment *Class *Group *Address *Size/ {
 mode=segment
 print " Start         Length     Name                   Class\n"
}

/^Address *Symbol$/ {
 mode=memory
 if(groups>0)
 {
  printf "\n Origin          Group\n\n"
  for(i=0; i<groups; i++)
   print group_buf[i]
 }
 printf "\n  Address         Publics by Value\n\n"
}

# HACK: the following works around the problem of __DOSseg__ symbol belonging
# to an undefined segment (0000).

/__DOSseg__/ {
 next
}

/^[^= ]* *....:........  *........$/ {
 if(mode==group)
 {
  group_buf[groups++]=sprintf(" %s   %s", toupper($2), $1)
 }
}

/^[^= ]* *....:....  *........$/ {
 if(mode==group)
 {
  group_buf[groups++]=sprintf(" %s   %s", toupper($2), $1)
 }
}

/^[^= ]* *[^ ]* *[^ ]*  *....:....  *........$/ {
 if(mode==segment && length($4) == 9)
 {
  printf " %s 0%sH %-22s %s 16-bit\n", toupper($4), toupper($5), $1, $2
 }
}

/^[^= ]* *[^ ]* *[^ ]*  *....:........  *........$/ {
 if(mode==segment && length($4) == 13)
 {
  printf " %s 0%sH %-22s %s 32-bit\n", toupper($4), toupper($5), $1, $2
 }
}

/^....:....\*?  *[^ ]*$/ {
 if(mode==memory && length($1) <= 10)
 {
  sel = substr($1, 1, 4)
  off = substr($1, 6, 4)
  printf " %s:0000%s       %s\n", toupper(sel), toupper(off), $2
 }
}

/^....:........\*?  *[^ ]*$/ {
 if(mode==memory && length($1) >= 13)
 {
  printf " %s       %s\n", toupper(substr($1, 1, 13)), $2
 }
}

{
 next
}
