/*
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*
 * IStrStream.java. 
 */

import java.util.*;

/**
 * An IStrStream object implements the functionality of the standard C++
 * istrstream class: given a string, it provides methods that return
 * string tokens, ints, floats, and longs. Note that Java longs are 64 bits
 * while C++ longs are usually 32 bits.
 *
 * @author      Brian Lewis  8/31/2000.
*/
public class IStrStream {
    private StringTokenizer tokenizer;

    /**
     * Creates an IStrStream to read free-form items from a String.
     */
    public IStrStream(String str) {
	// the second parameter is the set of token delimeter characters
	tokenizer = new StringTokenizer(str, " \n\t");
    }

    /**
     * Returns true if no tokens remain, else false.
     */
    public boolean eof() {
	return !(tokenizer.hasMoreElements());
    }

    /**
     * Returns the next token as a String. Raises NoSuchElementException
     * if no tokens remain.
     */
    public String getString() throws NoSuchElementException
    {
	return tokenizer.nextToken();
    }

    /**
     * Returns the next token as a float. 
     * Raises NoSuchElementException if no tokens remain and
     * raises NumberFormatException if the token cannot be parsed as a float.
     */
    public float getFloat() throws NoSuchElementException,
	                           NumberFormatException
    {
	String s = tokenizer.nextToken();
	return Float.parseFloat(s);
    }

    /**
     * Returns the next token as an int.
     * Raises NoSuchElementException if no tokens remain and
     * raises NumberFormatException if the token cannot be parsed as an int.
     */
    public int getInt() throws NoSuchElementException,
	                       NumberFormatException
    {
	String s = tokenizer.nextToken();
	return Integer.parseInt(s);
    }
    
    /**
     * Returns the next token as a long (64 bit int, not same as C++ "long").
     * Raises NoSuchElementException if no tokens remain and
     * raises NumberFormatException if the token cannot be parsed as a long.
     */
    public long getLong() throws NoSuchElementException,
	                         NumberFormatException
    {
	String s = tokenizer.nextToken();
	return Long.parseLong(s);
    }

    /**
     * A main procedure to allow testing IStrStream by running it as a
     * "program". The program takes no arguments and simply prints "passed"
     * if successful.
     */
    public static void main(String args[]) {
        String str = "Composer 2.1 ascii\ncamera {\n  position 3.84789 3.48111 14.0946\n  viewDirection -123 -0.0143311 27\n}";
	IStrStream stream = new IStrStream(str);
	String input;
	int i;
	float f;
	
	input = stream.getString();
	if (!(input.equals("Composer"))) {
	    System.err.println("unexpected string 1 " + input);
	    System.exit(1);
	}
	f = stream.getFloat();
	if (Math.abs(f - 2.1) > 0.1) {
	    System.err.println("unexpected float 2 " + f);
	    System.exit(1);
	}
	input = stream.getString();
	if (!(input.equals("ascii"))) {
	    System.err.println("unexpected string 3 " + input);
	    System.exit(1);
	}
	while (!stream.eof()) {
	    input = stream.getString();
	    if (input.equals("camera")) {
		input = stream.getString();	// skip over {
		input = stream.getString();
		if (!(input.equals("position"))) {
		    System.err.println("unexpected string 4 " + input);
		    System.exit(1);
		}
		f = stream.getFloat();
		f = stream.getFloat();
		f = stream.getFloat();
		input = stream.getString();
		if (!(input.equals("viewDirection"))) {
		    System.err.println("unexpected string 5 " + input);
		    System.exit(1);
		}
		i = stream.getInt();
		if (i != -123) {
		    System.err.println("unexpected integer 6 " + i);
		    System.exit(1);
		}
		f = stream.getFloat();
		i = stream.getInt();
		if (i != 27) {
		    System.err.println("unexpected integer 7 " + i);
		    System.exit(1);
		}
		input = stream.getString();	// skip over }
	    } else {
		System.err.println("unexpected subcommand string 8 " + input);
		System.exit(1);
	    }
	}
	System.out.println("passed");
    }
}
    
