/* generate wrappers */

parse arg args

n = args

do i = 0 to n
  say "SUBCOM_HANDLER(" || i || ")"
end
