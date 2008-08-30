/*==============================================================================
 * FILE:       patternparser.cc
 * OVERVIEW:   Stand alone program that is a driver for the pattern
 *             parser used to generate a pattern decoder.
 *
 * Copyright (C) 1998, The University of Queensland, BT group
 *============================================================================*/

#include "patternparser.h"


void usage(char* progName)
{
	cerr << "usage: " << progName << "{ -d } pattern_file\n\n";
	cerr << "\tThe pattern_file argument is the full path name\n";
	cerr << "\tof the pattern file to be parsed. The generated\n";
	cerr << "\tfiles will be <pattern_file>.h and <pattern_file>.m\n";
	cerr << "\trespectively.\n";
	cerr << "The -d flag turns on debugging.\n\n";
	exit(1);
}

int main(int argc, char* argv[])
{
	bool debug = false;
	string patFile;

	if (argc < 2 || argc > 3)
		usage(argv[0]);
	else {
		debug = (strcmp(argv[1],"-d") == 0);
		patFile = argv[argc - 1 ];
	}

	// These are the valid type names that correspond to the logue
	// types declared in class Logue (csr.h)
	set<string> types;
	types.insert("CALLER_PROLOGUE");
	types.insert("CALLEE_PROLOGUE");
	types.insert("CALLEE_EPILOGUE");
	types.insert("CALLER_EPILOGUE");

	PatternParser parser(patFile,types,debug);
	return parser.yyparse();
}
