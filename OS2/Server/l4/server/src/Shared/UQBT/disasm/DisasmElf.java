/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

import sun.jvm.hotspot.debugger.posix.elf.*;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.TreeMap;
import java.util.SortedMap;

public class DisasmElf {

    private static boolean debug = false; 

    // Default platform is Pentium
    private static String platform = "pent";

    // Size of text section
    private static long sectionSize;

    // Offset in ELF file to the text section 
    private static long textOffset;

    // VMA address of the text section
    private static int memAddress;
    
    // Holds the map of function addresses to function names
    protected static TreeMap symbolAddressMap;

    //  getElfInfo is used to get the Elf header information
    //  of sectionSize, textOffset and memAddress
    protected static void getElfInfo(String argValue) {

        try {
            ELFFile elfFile = ELFFileParser.getParser().parse(argValue);
            ELFHeader elfHeader = elfFile.getHeader();

	    symbolAddressMap = new TreeMap();

            for (int i = 0; i < elfHeader.getNumberOfSectionHeaders(); i++) {
                ELFSectionHeader sh = elfHeader.getSectionHeader(i);
                String str = sh.getName();               
		
		// Debug Info
		// System.out.println("Section names: " + str + "\n");
		// System.out.println("Number of symbols are: " + sh.getNumberOfSymbols() + "\n");
		
		// Function names are found in the .dynsym and .symtab section		
		if (str != null && (str.equals(".dynsym") || str.equals(".symtab"))){
		    for (int j = 1; j < sh.getNumberOfSymbols(); j++) {
	     	        ELFSymbol elfSym = sh.getELFSymbol(j);
		        if (elfSym != null) {
			    symbolAddressMap.put(new Integer(elfSym.getValue()), elfSym.getName());	
			}		    
		    }		        		    		
		}
		
		// The instruction that we decode are in the .text section
                else if (str != null && str.equals(".text")) {
                    memAddress = sh.getAddress();
                    textOffset = sh.getOffset();		    
		    sectionSize = sh.getSectionSize();		    	    	
                }
            }
	    
        } catch (ELFException e) {
            System.err.println("ELF parser exception : \n" + e);
            System.exit(3);
        } catch (Exception e) {
            System.err.println("General error : \n" + e);
            System.exit(4);
        }
    }


    // loadElfInMem loads the text section from the Elf file into 
    // a byte array
    protected static void loadElfInMem(byte[] binInMem, String inArg) {
        try {
            FileInputStream inFile = new FileInputStream(inArg);
            inFile.skip(textOffset);
            inFile.read(binInMem);
            inFile.close();
        } catch (FileNotFoundException e) {
            System.err.println("File not found : \n" + e);
            System.exit(3);
        } catch (Exception e) {
            System.err.println("General Error : \n" + e);
            System.exit(4);
        }
    }
          
    // Takes the inMemAddress parameter (which represents the current functions 
    // mem address and finds the next function's mem address and returns it
    protected static int nextFunction(int inMemAddress) {

	SortedMap subMap = symbolAddressMap.subMap(new Integer(inMemAddress), 
						(Integer)symbolAddressMap.lastKey());
	
	Integer currentFunctionKey = (Integer) subMap.firstKey();
		
	// Getting next function and return its mem address
	subMap = symbolAddressMap.subMap(new Integer(inMemAddress + 1), (Integer)symbolAddressMap.lastKey());
	Integer nextFunctionKey = (Integer) subMap.firstKey();	
	
	return nextFunctionKey.intValue();
    }		     

    protected static String errorMessage() {
	StringBuffer errorString = new StringBuffer();
        errorString.append("Usage: java DisasmElf [options] <executable-filename>\n\n" );
        errorString.append("Options: -h                 Displays this help message.\n");
        errorString.append("         -p <platform>      Specifies platform of the executable program.\n");
        errorString.append("                            Supported platforms: pent  (x86, 32-bit)\n");	
        errorString.append("                                                 sparc (v8)\n");
        errorString.append("                                                 Default is pent\n");
	return errorString.toString();
    }
   
    // Returns the offset to the last argument which should be the program path and name 
    protected static int parseCommandLine(String[] args){
    
	int arg = 0;

	while ( arg < args.length && args[arg].charAt(0) == '-' && args[arg].charAt(1) != '-' ) {
	    switch (args[arg].charAt(1)) {
		case 'h':
		    System.err.println(errorMessage());
		    System.exit(1);
		case 'p':
		    if (++arg >= args.length) {
		        System.err.println("Must specify platform after -p parameter");
			System.err.println(errorMessage());
			System.exit(2);
		    }
		    platform = args[arg].toLowerCase();
		    break; 

		/** More Options can be added here **/

		default:
	    	    System.err.println("Unrecognized option: " + args[arg] + "\n");
		    System.err.println(errorMessage());
	            System.exit(2);
	    }
	    arg++;
	}

	if (arg >= args.length) {
	    System.err.println("Name of Elf executable must be given");
	    System.err.println(errorMessage());
	    System.exit(3);
	}    
	return arg;
    }

    public static void main(String[] args) {
       
        int arg = parseCommandLine(args);

	// args[arg] should be the the source path + filename
	getElfInfo(args[arg]);

	// Create a byte array to hold the text section of the 
	// elf file. The "+ 8" makes the array have space to 
	// hold at least one additional instruction
	byte binInMem[] = new byte[(int)sectionSize + 8];
	loadElfInMem(binInMem, args[arg]);
	
	genericDis dis = null;

//	if (platform.equals("pent")) 
//	    dis = new x86Dis(memAddress, binInMem);
	if (platform.equals("sparc"))
	    dis = new sparcdiself(memAddress, binInMem);
	else {
	    System.err.println("Unsupported platform " + platform);
	    System.err.println(errorMessage());
	    System.exit(4);
	}

	try {
	    int currentFunctionAddress = memAddress;
	    int nextFunctionAddress;

            // Iterate through the whole text section one function at a time
	    // from the smallest mem address to the largest
	    while (currentFunctionAddress < memAddress + (int) sectionSize) {
  	        nextFunctionAddress = nextFunction(currentFunctionAddress);
	
		String functionName = (String) symbolAddressMap.get(new Integer(currentFunctionAddress));
	        System.out.println(functionName + ":");

		// The + 4 is necessary to get the last instruction of the function.
		// However, on systems where the instruction length is not 4 bytes or
		// the instructions are of variable size, the last instruction in 
		// each function can not be 100% trusted. Also, additional instruction
		// might be decoded at the end that does not belong to the function
		if (nextFunctionAddress <= memAddress + (int) sectionSize)	
                    dis.disassembleRegion(currentFunctionAddress, nextFunctionAddress + 4);
                else
                    dis.disassembleRegion(currentFunctionAddress, memAddress + (int) sectionSize + 4);
		    
		currentFunctionAddress = nextFunctionAddress;
	    }
        } catch( Exception e ) {
            System.err.println("Exception caught: " + e.toString() );
        }
   }
}
