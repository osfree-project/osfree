# File: hppadis.spec
# Desc: toolkit details for a PA-RISC disassembler (copied from Sparc spec)
#
# Copyright (C) 2001, The University of Queensland, BT group

# interface to NJ 
address type is "DWord"
address to integer using "%a"
#address to integer using "%a - instr + pc"
address add using "%a+%o"
fetch 32 using "getDword(%a)"
