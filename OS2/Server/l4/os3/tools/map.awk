/0xbb/ {
   if ($2 ~ /^Kal/)
     printf "%s %s\n", $1, $2
}
