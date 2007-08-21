#include <stdio.h>
#include <string.h>

#include "RenModul.h"

int ShowUsage()
{
  printf( "Usage: [/r | /i ] <filename> [<old module name> <new module name]>\n" );
  printf( " With filename only: display module name and imported module names\n" );
  printf( " /r rename this module\n" );
  printf( " /i change imported module name\n" );    
  return RC_PARAM_ERROR;
}

int main( int argc, char** argv )
{
  /* Get and check arguments */
  printf( "RENMODUL %s\n", VERSION );
  if ( argc < 2 )
  {
    ShowUsage();
    return RC_PARAM_ERROR;
  }
  
  if ( stricmp( argv[ 1 ], "/?" ) == 0 )
    return ShowUsage();

  if ( stricmp( argv[ 1 ], "/r" ) == 0 )
  {
    if ( argc != 5 )
      return ShowUsage();
    return RenameModule( argv[ 2 ],
			 RENAME_MODULE, 
			 argv[ 3 ], 
			 argv[ 4 ] );
  }

  if ( stricmp( argv[ 1 ], "/i" ) == 0 )
  {
    if ( argc != 5 )
      return ShowUsage();
    return RenameModule( argv[ 2 ],
			 RENAME_IMPORTED_MODULE, 
			 argv[ 3 ], 
			 argv[ 4 ] );
  }  

  if ( argc != 2 )
    return ShowUsage();

  return RenameModule( argv[ 1 ],
		       LIST_NAMES, 
		       NULL, 
		       NULL );
}
