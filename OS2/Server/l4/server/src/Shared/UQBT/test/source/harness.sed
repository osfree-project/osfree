# A small sed script to cause a few structures in the harness for the
# pentium 129.compress benchmark to be double word (8 byte) aligned
# This is needed until the translator can cope with unaligned binaries

s/prob_tab,131072,4/prob_tab,131072,8/
/freq_tab:/ {
a\
	.align	8
}
