/*
 * Copyright (C) 1999, The University of Queensland
 * Copyright (C) 1999-2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*
 * TranslatedFile.java. 
 */

import java.io.*;

/**
 * A TranslatedFile object implements the run time support for class files
 * compiled with the EGCS JVM port. It is responsible for setting up memory
 * for the class file and supplying library functions.
 *
 *  9/9/99  - Trent Waddington: created file.
 * 12/2/99  - Cristina: added ___Mod and __Umod helper functions. 12/2/99.
 * 12/9/99  - Cristina: _main uses the C signature "int _main(int,int)"
 *  7/7/00  - Brian: reformatted file.
 *  7/8/00  - Sameer: added the function offset(), used by translated
 *            programs to compute physical addresses from logical ones.
 *  8/31/00 - Brian: added __ftoi conversion routine.
 * 11/14/00 - Brian: much rework and addition of new methods.
 * 11/15/00 - Brian: removed code that tried to save memory by not allocating
 *            memory before the first rw/ro/bss data. The need to subtract an
 *            offset from a "logical" address to form a "physical" address
 *            caused no end of problems with storage not in the "memory" array:
 *            e.g., heap-allocated AFP arrays, other heap-allocated memory.
 * 11/16/00 - Brian: made memory a byte array, removed use of "ldc" for memory
 *            addresses.
 */

abstract class TranslatedFile {
    // Debug flag set on the command line: e.g., java -Duqbt.debug=true ...
    static boolean debug =
        (System.getProperty("uqbt.debug") != null) &&
        ((System.getProperty("uqbt.debug").compareTo("true") == 0) ||
	 (System.getProperty("uqbt.debug").compareTo("1") == 0));
    static boolean verboseDebug = false;

    /**
     * The subclass (the translated program) must have a main method that
     * takes two integers that are argc and the address argv. This method
     * is called by realMain() after it initializes memory.
     */
    public abstract int _main(int argc, int argv);
    
    /**
     * The subclass (the translated program) must have a name() method to
     * specify the program name to be placed in argv[0].
     */
    public abstract String name();
    
    /**
     * The subclass (the translated program) must have a datanames() method to
     * supply a list of filenames for the ro, rw, and bss data to be loaded.
     */
    public abstract String[] datanames();
    
    /**
     * The subclass (the translated program) must have a datasizes() method
     * to supply a list of sizes for the files containing the ro, rw, and
     * bss data.
     */
    public abstract int[] datasizes();
    
    /**
     * The subclass (the translated program) must have a datastarts() method
     * to supply a list of start addresses for the data to be loaded.
     */
    public abstract int[] datastarts();
    
    /**
     * The subclass (the translated program) must have a globalsize() method
     * to specify the size of the global data section, if a file is found
     * called "global".
     */
    public abstract int globalsize();
    
    /*
     * The memory array used by the translated program.
     */
    public static byte[] memory;
    
    /**
     * Stores a byte to the "memory" array at address "addr". The argument
     * "value", an integer, is truncated to a byte before the store.
     */
    public static void memWriteByte(int addr, int value) {
	memory[addr] = (byte)(value & 0xff);
    }
    
    /**
     * Returns the byte in the "memory" array at address "addr".
     */
    public static int memReadByte(int addr) {
	return (memory[addr] & 0xff);
    }
    
   /**
     * Write a 32 bit word to address "addr" in little-endian order.
     */
    public static void memWrite(int addr, int value) {
	if (debug) {
	    System.out.println("memWrite to addr " + addr +
			       " of int " + value);
	}
	memWriteByte(addr  , (value >> 24));
	memWriteByte(addr+1, (value >> 16));
	memWriteByte(addr+2, (value >>  8));
	memWriteByte(addr+3, (value      ));
    }
    
   /**
     * Returns the 32 bit word at address "addr" in little-endian order.
     */
    public static int memRead(int addr) {
	int result;
	result  = memReadByte(addr  ) << 24;
	result |= memReadByte(addr+1) << 16;
	result |= memReadByte(addr+2) << 8;
	result |= memReadByte(addr+3);
	if (debug) {
	    System.out.println("memRead from addr " + addr +
			       " returning int " + result);
	}
	return result;			
    }

   /**
     * Method to read the rw, ro, bss, and global data from files into the
     * memory array at the addresses specified by the subclass's datastarts()
     * method (4096 for the global data).
     */
    public void initMemory() {
	String[] fileNames = datanames();
	int[]    sizes     = datasizes();
	int[]    starts    = datastarts();
	int largestOffset, largestIdx, smallestIdx, i;

	largestOffset = largestIdx = 0;
	if (fileNames.length > 0) {
	    for (i = 0;  i < starts.length;  i++) {
		if (starts[i] > largestOffset) {
		    largestOffset = starts[i];
		    largestIdx = i;
		}
	    }
	    memory = new byte[largestOffset + sizes[largestIdx]];
	    if (debug) {
		System.out.println("Max(datastarts()) is " + largestOffset +
				   " at index " + largestIdx);
		System.out.println("   so created memory array of length " +
				   (largestOffset + sizes[largestIdx]));
	    }
	} else {
	    memory = new byte[4096 + 65536];
	    if (debug) {
		System.out.println("No filenames, so created memory array" +
				   " with default length " +
				   (4096 + 65536));
	    }
	}

	// Load the ro, rw, and bss data from the files named in fileNames.
	for (i = 0;  i < fileNames.length;  i++) {
	    try {
		FileInputStream f = new FileInputStream(fileNames[i]);
		f.read(memory, starts[i], sizes[i]);
		f.close();
		if (debug) {
		    System.out.println("Loaded " + sizes[i] +
				       " bytes from " + fileNames[i] +
				       " to addr " + starts[i]);
		}
	    } catch (FileNotFoundException e) {
		//	System.err.println("cannot find " + fileNames[i]);
	    } catch (IOException e) {
		System.err.println("initMemory: failed to read from file " +
				   fileNames[i]);
	    }
	}		
	
	// Load the global data from the file "global".
	try {
	    FileInputStream f = new FileInputStream("global");
	    f.read(memory, 4096, globalsize());
	    f.close();
	    if (debug) {
		System.out.println("loaded " + globalsize() +
				   " bytes from global to address 4096");
	    }
	} catch (FileNotFoundException e) {
	    if (debug) {
		System.out.println("File \"global\" not found");
	    }
	} catch (IOException e) {
	    System.err.println("initMemory: cannot read file \"global\"");
	}
    }
    
    /**
     * The subclass (the translated program) starts execution in its main()
     * method. This, in turn, creates a single TranslatedFile instance and
     * calls this method to set up the program's environment.
     */
    public void realMain(String[] args) {
	String progName   = name();
	byte[] nameBytes  = progName.getBytes();
	int    nameLength = progName.length();
	int addr, i;
	if (debug) {
	    System.out.println("Loading data files and initializing memory " +
			       "for program " + progName);
	}
	initMemory();

	// Write the arguments to the start of the "memory" array.
	if (debug) {
	    System.out.println("\nSetting up argv and argc...");
	}
	int[] argAddrs = new int[args.length + 1];
	addr = 0;
	argAddrs[0] = 0;	   // arg0 starts at address 0
	for (i = 0;  i < nameBytes.length;  i++) {
	    memory[i] = nameBytes[i];
	}
	memory[nameLength] = 0;
	if (debug) {
	    System.out.println("argument 0: program name string '" +
			       progName + "' at addr 0");
	}
	
	addr = (nameLength + 1);
	for (i = 0;  i < args.length;  i++) {
	    byte[] argBytes = args[i].getBytes();
	    argAddrs[i+1] = addr; // start of argument i
	    for (int j = 0;  j < argBytes.length;  j++) {
		memory[addr + j] = argBytes[j];
	    }
	    if (debug) {
		System.out.println("argument " + (i + 1) +
				   ": string '" + args[i] +
				   "' at addr " + argAddrs[i+1]);
	    }
	    addr += args[i].length();
	    memory[addr] = 0;
	    addr++;
	}
	
	int argv = addr;
	for (i = 0;  i < argAddrs.length;  i++) {
	    memWrite(addr, argAddrs[i]);
	    addr += 4;
	}
	if (debug) {
	    System.out.println("The " + (args.length + 1) +
			       " args take up " + addr +
			       " bytes at the start of memory");
	}
	
	// Invoke translated user's code
	if (debug) {
	    System.out.print("Calling _main(");
	    for (i = 0;  i < argAddrs.length;  i++) {
		if (i != 0) {
		    System.out.print(", ");
		}
		System.out.print(argAddrs[i]);
	    }
	    System.out.println(")\n");
	}
	int result = _main((args.length + 1), argv);
	if (debug) {
	    System.out.println("\nTranslated file terminated with result " +
			       result);
	}
    }
    
    /*
     *=====================================================================
     *
     * Implementations of C library functions.
     *
     *===================================================================== 
     */
    
    public static int _setlocale(int i, int j) {
	if (debug) {
	    System.out.println("setlocale called with category " + i +
			       ", locale name " + j +
			       " (\"" +
			       ((j == 0)? "(null)" : buildString(j)) +
			       "\")");
	}
	return 0;
    }
    
    public static int _textdomain(int i) {
	if (debug) {
  	    System.out.println("textdomain called with " + i +
			       " : \"" +
			       ((i == 0)? "(null)" :
				          buildString(i)) + "\"");
	}
	return i;
    }
    
    public static int _gettext(int i) {
	if (debug) {
  	    System.out.println("gettext called with " + i + " : \"" +
			       ((i == 0)? "(null)" : buildString(i)) + "\"");
	}
	return i;
    }
    
    /**
     * _exit: terminate the program and the Java Virtual Machine.
     * CORRECT THIS METHOD TO HAVE RETURN TYPE void AFTER THAT BUG IS FIXED.
     */
    public static int _exit(int i) {
	System.exit(i);
	return 0;
    }
    
    /**
     * strcmp: Compares two strings lexicographically. Returns -1, 0, or 1
     * depending on whether String s1 is less than, equal to, or greater than
     * String s2 using a character-by-character comparison.
     */
    public static int strcmp(String s1, String s2) {
	return s1.compareTo(s2); 
    }
    
    /**
     * ___Mod: signed integer modulus.
     */
    public static int ___Mod(int i, int j) {
	return (i % j); 
    }
    
    /**
     * ___Umod: unsigned integer modulus.
     */
    public static int ___Umod(int i, int j) {
	return (i & 0xFFFFFFFF) % (j & 0xFFFFFFFF); 
    }
    
    /**
     * __ftoi: float to integer conversion.
     */
    public static int __ftoi(float f) {
	return (int)f;
    }
    
    /**
     * __ftou: float to unsigned integer conversion. Java doesn't have
     * unsigned ints, so we simply return a signed int.
     */
    public static int __ftou(float f) {
	return (int)f;
    }
    
    /**
     * tan: returns the trigonometric tangent of an angle in radians.
     */
    public static float tan(float f) {
	float f2 = (float)(Math.tan(f));
	return f2;
    }
    
    /**
     * sqrt: returns the square root of a float value.
     */
    public static float sqrt(float f) {
	float f2 = (float)(Math.sqrt(f));
	return f2;
    }
    
    /**
     * _ex_rethrow_q: dummy routine (original routine in C++ runtime used to
     * rethrow C++ exceptions?)
     */
    public static void _ex_rethrow_q() {
	if (debug) {
	    System.out.println("_ex_rethrow_q called");
	}
    }
    
    /**
     * _vector_con_: dummy routine.
     */
    public static void _vector_con_() {
	if (debug) {
	    System.out.println("_vector_con_ called");
	}
    }
    
    /*
     * ------------ String and memory methods ------------
     */
    
    public static int strlen(int str) {
	int addr = str;
	int c = 0;
	while (memReadByte(addr) != 0) {
	    addr++;
	    c++;
	}
	if (verboseDebug) {
	    System.out.println("strlen: string at addr " + str +
			       " has length " + c);
	}
	return c;
    }
    
    public static int _strlen(int str) {
	return strlen(str);
    }
    
    public static int strcpy(int str1, int str2) {
	if (debug) {
	    System.out.println("strcpy with " + str1 + " and " + str2);
	}
	int start = str1;
	int c = memReadByte(str2);
	while (c != 0) {
	    memWriteByte(str1, c);
	    str1++;  str2++;
	    c = memReadByte(str2);
	}
	return start;
    }
    
    public static int _strcpy(int str1, int str2) {
	return strcpy(str1, str2);
    }	
    
    public static int puts(int str) {
	if (debug) {
	    System.out.println("puts: " + str);
	}
	System.out.println(buildString(str));
	return 0;		
    }	
    
    public static int _puts(int str) {
	return puts(str);
    }
    
    public static int memcpy(int dest, int src, int num) {
	for (int i = 0;  i < num;  i++) {
	    memWriteByte((dest + i), memory[src+i]);
	}
	return dest;
    }
    
    public static int _memcpy(int dest, int src, int num) {
	if (debug) {
  	    System.out.println("_memcpy from " + src + " to " + dest +
			       " for " + num);
	}
	for (int i = 0;  i < num;  i++) {
	    memWriteByte((dest + i), memory[src+i]);
	}
	return dest;
    }
    
    public static String buildString(int str) {
	int len = strlen(str);
	byte[] b = new byte[len];
	for (int i = 0;  i < b.length;  i++) {
	    b[i] = (byte)memReadByte(str + i);
	}
	String s = new String(b);
	if (debug) {
	    int strLen = s.length();
	    boolean hasTrailingNL =
		(strLen > 0) && ((s.charAt(strLen - 1) == '\n'));
	    if (hasTrailingNL) {
		String allButNL = s.substring(0, (strLen - 1));
		System.out.println("buildString: addr " + str +
				   ", \"" + allButNL +
				   "\\n\", length " + strLen);
	    } else {
		System.out.println("buildString: addr " + str +
				   ", \"" + s +
				   "\", length " + strLen);
	    }
	}
	return s;
    }
   

    /*
     * ------------ Printf methods ------------
     */

    public static int _printf(String fmt, int arg1, int arg2) {
	int[] args = new int[2];
	args[0] = arg1;  args[1] = arg2;
	return printf(fmt, args);	
    }
    
    public static int _printf(String fmt, int arg1, int arg2, int arg3) {
	int[] args = new int[3];
	args[0] = arg1;  args[1] = arg2;  args[2] = arg3;
	return printf(fmt, args);	
    }
    
    public static int _printf(int format) {
	int[] args = new int[0];
	return printf(buildString(format), args);
    }
    
    public static int _printf(int format, int arg1) {
	int[] args = new int[1];
	args[0] = arg1;
	return printf(buildString(format), args);
    }
    
    public static int _printf(int format, int arg1, int arg2) {
	int[] args = new int[2];
	args[0] = arg1;  args[1] = arg2;
	return printf(buildString(format), args);
    }
    
    public static int _printf(int format, int arg1, int arg2, int arg3) {
	int[] args = new int[3];
	args[0] = arg1;  args[1] = arg2;  args[2] = arg3;
	return printf(buildString(format), args);
    }
    
    public static int _printf(int format, int arg1, int arg2, int arg3,
			      int arg4) {
	int[] args = new int[4];
	args[0] = arg1;  args[1] = arg2;  args[2] = arg3;  args[3] = arg4;
	return printf(buildString(format), args);
    }
    
    private static String[] hexchars = {"0", "1", "2", "3",
					"4", "5", "6", "7",
				        "8", "9", "a", "b",
					"c", "d", "e", "f"};
    
    public static int printf(String fmt, int[] args) {
	if (debug) {
	    int strLen = fmt.length();
	    boolean hasTrailingNL =
		(strLen > 0) && ((fmt.charAt(strLen - 1) == '\n'));
	    if (hasTrailingNL) {
		String allButNL = fmt.substring(0, (strLen - 1));
		System.out.println("printf: fmt string \"" + allButNL +
				   "\\n\" and " + args.length + " arguments");
	    } else {
		System.out.println("printf: fmt string \"" + fmt +
				   "\" and " + args.length + " arguments");
	    } 
	    if (strLen == 0) {
		System.out.println("printf: zero-length fmt string!");
	    }
	}
	int arg = 0;
	for (int i = 0;  i < fmt.length();  i++) {
	    if (fmt.charAt(i) == '%') {
		switch(fmt.charAt(i+1)) {
		case 'd':
		case 'i':
		    System.out.print(args[arg++]);
		    break;
		    
		case 'x':
		    System.out.print(hexchars[(args[arg] >> 28) & 0x0f]);
		    System.out.print(hexchars[(args[arg] >> 24) & 0x0f]);
		    System.out.print(hexchars[(args[arg] >> 20) & 0x0f]);
		    System.out.print(hexchars[(args[arg] >> 16) & 0x0f]);
		    System.out.print(hexchars[(args[arg] >> 12) & 0x0f]);
		    System.out.print(hexchars[(args[arg] >>  8) & 0x0f]);
		    System.out.print(hexchars[(args[arg] >>  4) & 0x0f]);
		    System.out.print(hexchars[args[arg++]       & 0x0f]);
		    break;
		    
		case 'c':
		    System.out.print((char)args[arg++]);
		    break;
		    
		case 's':
		    System.out.print(buildString(args[arg++]));
		    break;
		    
		case '%':
		    System.out.print("%");
		    break;
		}
		i++;
      	    } else {
		System.out.print("" + fmt.charAt(i));
	    }
	}
	return arg;
    }

    /*
     * ------------ Scanf methods ------------
     */

    public static int _scanf(int format, int arg1, int arg2, int arg3) {
	int[] args = new int[3];
	args[0] = arg1;  args[1] = arg2;  args[2] = arg3;
	return scanf(buildString(format), args);
    }
    
    public static int _scanf(int format, int arg1, int arg2) {
	int[] args = new int[2];
	args[0] = arg1;  args[1] = arg2; 
	return scanf(buildString(format), args);
    }
    
    public static int _scanf(int format, int arg1) {
	int[] args = new int[1];
	args[0] = arg1;  
	return scanf(buildString(format), args);
    }
    
    public static int scanf(String fmt, int[] args) {
	if (debug) {
	    int strLen = fmt.length();
	    boolean hasTrailingNL =
		(strLen > 0) && ((fmt.charAt(strLen - 1) == '\n'));
	    if (hasTrailingNL) {
		String allButNL = fmt.substring(0, (strLen - 1));
		System.out.println("scanf: fmt string \"" + allButNL +
				   "\\n\" and " + args.length + " arguments");
	    } else {
		System.out.println("scanf: fmt string \"" + fmt +
				   "\" and " + args.length + " arguments:");
	    }
	    for (int i = 0;  i < args.length;  i++) {
		System.out.println("   " + i + ": \"" + args[i] + "\"");
	    }
	    if (strLen == 0) {
		System.out.println("scanf: zero-length fmt string!");
	    }
	}
	int arg = 0;
	try {
	    for (int i = 0;  i < fmt.length();  i++) {
		if (fmt.charAt(i) == '%') {
		    switch(fmt.charAt(i+1)) {
		    case 'd':
		    case 'i':
			{
			    int ch;
			    do 
				ch = System.in.read();
			    while ((ch < (int)'0') || (ch > (int)'9'));
			    int v = 0;
			    do { 
				v *= 10;
				v += (ch - (int)'0');
				ch = System.in.read();
			    } while ((ch >= (int)'0') && (ch <= (int)'9'));
			    memWrite(args[arg], v);							    arg++;
			}
			break;
			
		    case 'c':
			memWriteByte(args[arg], System.in.read());
			arg++;	
			break;
			
		    case 's': 
			System.out.println("scanf: bad string in " + fmt);
			System.exit(0);
			break;
		    }
		    i++;				
		} 
	    }		
	} catch (IOException e) {
	}
	return 0;
    }

    /*
     * ------------ Other runtime support methods ------------
     */

    /**
     * Simple debugging utility that prints an integer and location string.
     */
    public static void Dump(String where, int value) {
	System.out.println("Dump: " + where + ": value = " + value);
    }
    
    /**
     * Given two longs, returns -1, 0, or +1 depending on the result of
     * comparing the two longs as unsigned values. This procedure computes
     * the unsigned equivalent of the lcmp bytecode.
     */
    public static int unsignedLongCompare(long l, long m) {
	int result;
	long allButHOB = 0x7fffffffffffffffL;
	if (l > 0) {
	    if (m > 0) {
		result = ((l < m)? -1 : ((l == m)? 0 : +1));
	    } else {
		result = -1;	// l's HOB is 0 but m's is 1 so l < m
	    }
	} else {
	    if (m > 0) {
		result = 0;	// l's HOB is 1 but m's is 0 so l > m
	    } else {
		long t1 = (l & allButHOB);
		long t2 = (m & allButHOB);
		result = ((t1 < t2)? -1 : ((t1 == t2)? 0 : +1));
	    }
	}
	return result;
    }
    
}
