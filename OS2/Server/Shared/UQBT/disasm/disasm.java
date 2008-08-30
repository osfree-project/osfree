/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

import java.lang.*;

class disasm
{
   private static boolean debug = false; 
    
   static {
	System.loadLibrary("disasm");
   }
    
   private native static long[] loadBinaryFile( String filename );
   private native static void unloadBinaryFile( long handle);
//   private native static int initVM();

   public static void main(String[] args)
   {    
	int arg = 0;
    	int breakpt = 0;
    	int tracefrom = 0;
    	int tracept = 0;

	while ( arg < args.length && args[arg].charAt(0) == '-' && args[arg].charAt(1) != '-' ) 
	{
		switch (args[arg].charAt(1))
		{
			case 'd':
				debug = true;
				break;
			case 'b':
				if( arg+1 < args.length ) {
					arg++;
					breakpt = (Long.getLong(args[arg])).intValue();
				}
				debug = true;
				break;
			case 't':
				if( arg+1 < args.length ) {		
					arg++;
					tracefrom = (Long.getLong(args[arg])).intValue();
				}
				break;
            		case 'T':
				if( arg+1 < args.length ) {
					arg++;
					tracept = (Long.getLong(args[arg])).intValue();
				}
				break;
			default:
				System.err.println("Unrecognized option: " + args[arg] + "\n");
				System.exit(2);
			}
			arg++;
	}


	if( arg >= args.length ) {
		System.err.println("Usage: java disasm [options] <filename>\n" );
	 	System.exit(1);
	}

        long result[] = loadBinaryFile(args[arg]);
        if( result.length == 0 ) {
            System.err.println("Unable to open binary file: " + args[arg] );
            System.exit(1);
        }

        long handle = result[0];
        long start = result[1];
        long end = result[2];
        long membase = result[3];
        sparcdis dis = new sparcdis((int)membase);
        try {
            dis.disassembleRegion((int)start, (int)end);
        } catch( Exception e ) {
            System.err.println("Exception caught: " + e.toString() );
        }
        unloadBinaryFile(handle);
   }
}

		  


