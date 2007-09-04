Pascal Source File Processing Units
===================================

(c) 2000-2003 Areca Systems GmbH / Sebastian Guenther, sg@freepascal.org

This directory contains some useful units for dealing with Pascal source
files: Parsing, creating structures in memory, and output to files.


pastree.pp
----------
Pascal parse tree classes, for storing a complete Pascal module in memory.
Currently, mainly declarations are supported; support for implementation
code is very basic.


paswrite.pp
-----------
A class and helper functions for creating Pascal source files from parse trees


pscanner.pp
-----------
Lexical scanner class for Pascal sources


pparser.pp
----------
Parser for Pascal source files. Reads files via the pscanner unit and stores
all parsed data in a parse tree, as implemented in the pastree unit.
