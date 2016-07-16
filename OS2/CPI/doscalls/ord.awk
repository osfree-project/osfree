{
  # printf "%s\n", $3

  printf "APIRET APIENTRY\t\t%s(void)\n", $0
  printf "{\n"
  printf "  return unimplemented(__FUNCTION__);\n"
  printf "}\n"
  printf "\n"
  printf "\n"
}
