/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

import java.lang.*;

class emumain
{
   private static boolean debug = false; 

   static {
	System.loadLibrary("sparcemu");
   }

   private native static sparcemu initNative(String[] args, int arg);

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
		System.err.println("Usage: emulate [options] <filename> [application-options]\n" );
	 	System.exit(1);
	}

	sparcemu emulator = initNative(args, arg);
	emulator.debug = debug;
	emulator.run();
   }
}

		  


