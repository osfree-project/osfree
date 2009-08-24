ifidni MODEL,s
include "../include/small.inc"
else
ifidni MODEL,l
include "../include/large.inc"
else
error "Unsupported MODEL; assign 's' or 'l' to macro MODEL"
endif
endif
