#include "Types.r"
#include "SysTypes.r"
#include "CWPlugins.r"

resource 'Flag' (128,"Resource Compiler")
{
	kCurrentResourceVersion,
	Compiler
	{
		VERSION4API
		{
			doesntGenerateCode,
			doesntGenerateResources,
			cantPreprocess,
			canPrecompile,
			isntPascal,
			cantImport,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			reserved,
			UnknownLanguage
		}
	}
};

resource 'Targ' (128,"Resource Compiler")
{
	kCurrentResourceVersion,
	{
		Any
	},
	{
		Any
	}
};

resource 'EMap' (128,"Resource Compiler")
{
	kCurrentResourceVersion,
	{
		text,
		".rc",
		doPrecompile,
		notLaunchable,
		notResourceFile,
		handledByMake,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved,
		reserved
	}
};
	
resource 'STR ' (128,"Resource Compiler Name") { "TWIN Resource Compiler" };
resource 'STR ' (129,"Resource Compiler CPU") { "" };
resource 'STR#' (128,"Resource Compiler Panels") { { "" } };

resource 'vers' (1)
{
	0x02,
	0x00,
	final,
	0x01,
	0x00,
	"2.0.1",
	"v2.0.1, from the Willows Toolkit"
};

resource 'vers' (2)
{
	0x02,
	0x00,
	final,
	0x01,
	0x00,
	"2.0.1",
	"Copyright 1996, Willows Software"
};
