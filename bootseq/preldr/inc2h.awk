#
# Convert assembler include file
# to C include file
#


/^;/ {
   printf "// %s\n", $0
   next
}

/^$/ {
   print
}

/^include/ {
   p = index($2, ".")
   $2 = substr($2, 0, p) "h"
   printf "#include \"%s\"\n", $2
}

{
   p  = index($0, ";")
   if (p)
     $0 = substr($0, 0, p - 1) "//" substr($0, p + 1, length($0) - p)

   if ($2 == "equ" || $2 == "EQU")
   {
      $2 = ""
      printf "#define %s\n", $0
   }
}
