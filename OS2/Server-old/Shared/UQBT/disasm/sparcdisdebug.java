


 






 










import sun.misc.Unsafe;

import java.math.BigInteger;
import java.io.Writer;
import java.io.StringWriter;
import java.io.PrintWriter;









 


 







public class sparcdisdebug 



{

       



    public long mem;
    public long icount = 0;
    public boolean debug = false;

    protected static Unsafe unsafe = Unsafe.getUnsafe();


    private String returnStringValue;


     
    public sparcdisdebug(int base 



    ) {



        mem = (long)base;
        mem &= 0xffffffffL;
    }











     
    void setMembyte  ( long a, byte x )  { unsafe.putByte(mem+a,x); }
    void setMembyte  ( long a, int x )   { unsafe.putByte(mem+a,(byte)x); }
    void setMemshort ( long a, short x ) { unsafe.putShort(mem+a,x); }
    void setMemshort ( long a, int x )   { unsafe.putShort(mem+a,(short)x); }
    void setMemint   ( long a, int x )   { unsafe.putInt(mem+a,x); }
    void setMemint   ( long a, float x ) { unsafe.putFloat(mem+a,x); }
    void setMemlong  ( long a, long x )  { unsafe.putLong(mem+a,x); }
    void setMemlong  ( long a, double x ){ unsafe.putDouble(mem+a,x); }
    void setMemfloat ( long a, float x ) { unsafe.putFloat(mem+a,x); }
    void setMemdouble( long a, double x ){ unsafe.putDouble(mem+a,x); }

    byte   getMembyte  ( long a ) { return unsafe.getByte(mem+a); }
    short  getMemshort ( long a ) { return unsafe.getShort(mem+a); }
    int    getMemint   ( long a ) { return unsafe.getInt(mem+a); }
    long   getMemlong  ( long a ) { return unsafe.getLong(mem+a); }
    float  getMemfloat ( long a ) { return unsafe.getFloat(mem+a); }
    double getMemdouble( long a ) { return unsafe.getDouble(mem+a); }


    byte getByte(int a) {
        return getMembyte((long) a);
    }

    int getWord(int a) {
        return getMemint((long) a);
    }

     
    void assertion( boolean cond ) {
        if( cond ) {
	    throw new RuntimeException( "Assertion failed" );
	}
    }

 
    protected final static int HEXCUTOFF = 1024;
    protected String opname;

    protected String IMMEDTOSTRING( int val ) {
        if( val > HEXCUTOFF || val < -HEXCUTOFF ) {
            return "0x" + Integer.toHexString(val);
        } else {
            return Integer.toString(val);
        }
    }
    protected String formatHex( int val, int digits ) {
        StringBuffer buf = new StringBuffer(Integer.toHexString(val));
        while( buf.length() < digits ) buf.insert(0,'0');
        return buf.toString();
    }
    protected String rightPad( String val, int length ) {
        StringBuffer buf = new StringBuffer(val);
        while( buf.length() < length )
            buf.append(' ');
        return buf.toString();
    }
    protected String leftPad( String val, int length ) {
        StringBuffer buf = new StringBuffer(val);
        while( buf.length() < length )
            buf.insert(0,' ');
        return buf.toString();
    }
    protected String zeroPad( String val, int length ) {
        StringBuffer buf = new StringBuffer(val);
        while( buf.length() < length )
            buf.insert(0,'0');
        return buf.toString();
    }
    protected String callName( int address) {






        return ( IMMEDTOSTRING( address));

    }
    


    final static String aNames[] = { "", ",a" };
final static String cdNames[] = { "%c0", "%c1", "%c2", "%c3", "%c4", "%c5", "%c6", "%c7", "%c8", "%c9", "%c10", "%c11", "%c12", "%c13", "%c14", "%c15", "%c16", "%c17", "%c18", "%c19", "%c20", "%c21", "%c22", "%c23", "%c24", "%c25", "%c26", "%c27", "%c28", "%c29", "%c30", "%c31" };
final static String fddNames[] = { "%f0to1", "a", "%f2to3", "b", "%f4to5", "c", "%f6to7", "d", "%f8to9", "e", "%f10to11", "f", "%f12to13", "g", "%f14to15", "h", "%f16to17", "i", "%f18to19", "j", "%f20to21", "k", "%f22to23", "l", "%f24to25", "m", "%f26to27", "n", "%f28to29", "o", "%f30to31", "p" };
final static String fdqNames[] = { "%f0to3", "q", "r", "s", "%f4to7", "t", "u", "v", "%f8to11", "w", "x", "y", "%f12to15", "z", "A", "B", "%f16to19", "C", "D", "E", "%f20to23", "F", "G", "H", "%f24to27", "I", "J", "K", "%f28to31", "L", "M", "N" };
final static String fdsNames[] = { "%f0", "%f1", "%f2", "%f3", "%f4", "%f5", "%f6", "%f7", "%f8", "%f9", "%f10", "%f11", "%f12", "%f13", "%f14", "%f15", "%f16", "%f17", "%f18", "%f19", "%f20", "%f21", "%f22", "%f23", "%f24", "%f25", "%f26", "%f27", "%f28", "%f29", "%f30", "%f31" };
final static String fs1dNames[] = { "%f0to1", "a", "%f2to3", "b", "%f4to5", "c", "%f6to7", "d", "%f8to9", "e", "%f10to11", "f", "%f12to13", "g", "%f14to15", "h", "%f16to17", "i", "%f18to19", "j", "%f20to21", "k", "%f22to23", "l", "%f24to25", "m", "%f26to27", "n", "%f28to29", "o", "%f30to31", "p" };
final static String fs1qNames[] = { "%f0to3", "q", "r", "s", "%f4to7", "t", "u", "v", "%f8to11", "w", "x", "y", "%f12to15", "z", "A", "B", "%f16to19", "C", "D", "E", "%f20to23", "F", "G", "H", "%f24to27", "I", "J", "K", "%f28to31", "L", "M", "N" };
final static String fs1sNames[] = { "%f0", "%f1", "%f2", "%f3", "%f4", "%f5", "%f6", "%f7", "%f8", "%f9", "%f10", "%f11", "%f12", "%f13", "%f14", "%f15", "%f16", "%f17", "%f18", "%f19", "%f20", "%f21", "%f22", "%f23", "%f24", "%f25", "%f26", "%f27", "%f28", "%f29", "%f30", "%f31" };
final static String fs2dNames[] = { "%f0to1", "a", "%f2to3", "b", "%f4to5", "c", "%f6to7", "d", "%f8to9", "e", "%f10to11", "f", "%f12to13", "g", "%f14to15", "h", "%f16to17", "i", "%f18to19", "j", "%f20to21", "k", "%f22to23", "l", "%f24to25", "m", "%f26to27", "n", "%f28to29", "o", "%f30to31", "p" };
final static String fs2qNames[] = { "%f0to3", "q", "r", "s", "%f4to7", "t", "u", "v", "%f8to11", "w", "x", "y", "%f12to15", "z", "A", "B", "%f16to19", "C", "D", "E", "%f20to23", "F", "G", "H", "%f24to27", "I", "J", "K", "%f28to31", "L", "M", "N" };
final static String fs2sNames[] = { "%f0", "%f1", "%f2", "%f3", "%f4", "%f5", "%f6", "%f7", "%f8", "%f9", "%f10", "%f11", "%f12", "%f13", "%f14", "%f15", "%f16", "%f17", "%f18", "%f19", "%f20", "%f21", "%f22", "%f23", "%f24", "%f25", "%f26", "%f27", "%f28", "%f29", "%f30", "%f31" };
final static String rdNames[] = { "%g0", "%g1", "%g2", "%g3", "%g4", "%g5", "%g6", "%g7", "%o0", "%o1", "%o2", "%o3", "%o4", "%o5", "%sp", "%o7", "%l0", "%l1", "%l2", "%l3", "%l4", "%l5", "%l6", "%l7", "%i0", "%i1", "%i2", "%i3", "%i4", "%i5", "%fp", "%i7" };
final static String rs1Names[] = { "%g0", "%g1", "%g2", "%g3", "%g4", "%g5", "%g6", "%g7", "%o0", "%o1", "%o2", "%o3", "%o4", "%o5", "%sp", "%o7", "%l0", "%l1", "%l2", "%l3", "%l4", "%l5", "%l6", "%l7", "%i0", "%i1", "%i2", "%i3", "%i4", "%i5", "%fp", "%i7" };
final static String rs2Names[] = { "%g0", "%g1", "%g2", "%g3", "%g4", "%g5", "%g6", "%g7", "%o0", "%o1", "%o2", "%o3", "%o4", "%o5", "%sp", "%o7", "%l0", "%l1", "%l2", "%l3", "%l4", "%l5", "%l6", "%l7", "%i0", "%i1", "%i2", "%i3", "%i4", "%i5", "%fp", "%i7" };
public int disassembleaddress_( int pc, Writer out ) throws java.io.IOException
{
int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    if ((MATCH_w_32_0 >> 13 & 0x1)   == 1) { 
      int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
      int   simm13 = 
        (((int)(( (MATCH_w_32_0 & 0x1fff)   ) << (4*8-(  13 )))) >> (4*8-(  13 ))) ;
      nextPC = 4 + MATCH_p; 
      
      

       { out.write(rs1Names[rs1] );

      out.write( " + " );

      out.write( IMMEDTOSTRING( simm13));

       opname = "dispA"; }

      
      
      
    }   
    else { 
      int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
      int  rs2 = (MATCH_w_32_0 & 0x1f)  ;
      nextPC = 4 + MATCH_p; 
      
      

       { out.write(rs1Names[rs1] );

      out.write( " + " );

      out.write(rs2Names[rs2] );

       opname = "indexA"; }

      
      
      
    }   
    
  } 
  
    
  
}


return nextPC;
}
public int disassemblereg_or_imm( int pc, Writer out ) throws java.io.IOException
{
int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    if ((MATCH_w_32_0 >> 13 & 0x1)   == 1) { 
      int   simm13 = 
        (((int)(( (MATCH_w_32_0 & 0x1fff)   ) << (4*8-(  13 )))) >> (4*8-(  13 ))) ;
      nextPC = 4 + MATCH_p; 
      
      

       { out.write( IMMEDTOSTRING( simm13));

       opname = "imode"; }

      
      
      
    }   
    else { 
      int  rs2 = (MATCH_w_32_0 & 0x1f)  ;
      nextPC = 4 + MATCH_p; 
      
      

       { out.write(rs2Names[rs2] );

       opname = "rmode"; }

      
      
      
    }   
    
  } 
  
    
  
}


return nextPC;
}
public int disassembleregaddr( int pc, Writer out ) throws java.io.IOException
{
int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    if ((MATCH_w_32_0 >> 13 & 0x1)   == 1) { 
      nextPC = MATCH_p; 
      
      

      { opname = "#ERR#"; ; };

      
      
      
    }   
    else { 
      int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
      int  rs2 = (MATCH_w_32_0 & 0x1f)  ;
      nextPC = 4 + MATCH_p; 
      
      

       { out.write(rs1Names[rs1] );

      out.write( " + " );

      out.write(rs2Names[rs2] );

       opname = "indexR"; }

      
      
      
    }   
    
  } 
  
    
  
}


return nextPC;
}


    int disassembleInstruction( int pc, Writer out ) throws java.io.IOException {
        int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    
      switch((MATCH_w_32_0 >> 30 & 0x3)  ) {
        case 0: 
          
            switch((MATCH_w_32_0 >> 22 & 0x7)  ) {
              case 0: case 1: case 3: case 4: case 5: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction1(pc, out); };} break;
              case 2: 
                if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) 
                  
                    switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                      case 0: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BN,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 1: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BE,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 2: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BLE,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 3: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BL,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 4: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BLEU,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 5: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BCS,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 6: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BNEG,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 7: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BVS,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 8: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BA,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 9: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BNE,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 10: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BG,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 11: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BGE,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 12: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BGU,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 13: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BCC,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 14: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BPOS,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 15: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BVC,a"; }

                          
                          
                          
                        }
                        
                        break;
                      default: assertion(false);
                    }    
                else 
                  
                    switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                      case 0: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BN"; }

                          
                          
                          
                        }
                        
                        break;
                      case 1: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BE"; }

                          
                          
                          
                        }
                        
                        break;
                      case 2: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BLE"; }

                          
                          
                          
                        }
                        
                        break;
                      case 3: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BL"; }

                          
                          
                          
                        }
                        
                        break;
                      case 4: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BLEU"; }

                          
                          
                          
                        }
                        
                        break;
                      case 5: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BCS"; }

                          
                          
                          
                        }
                        
                        break;
                      case 6: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BNEG"; }

                          
                          
                          
                        }
                        
                        break;
                      case 7: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BVS"; }

                          
                          
                          
                        }
                        
                        break;
                      case 8: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BA"; }

                          
                          
                          
                        }
                        
                        break;
                      case 9: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BNE"; }

                          
                          
                          
                        }
                        
                        break;
                      case 10: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BG"; }

                          
                          
                          
                        }
                        
                        break;
                      case 11: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BGE"; }

                          
                          
                          
                        }
                        
                        break;
                      case 12: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BGU"; }

                          
                          
                          
                        }
                        
                        break;
                      case 13: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BCC"; }

                          
                          
                          
                        }
                        
                        break;
                      case 14: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BPOS"; }

                          
                          
                          
                        }
                        
                        break;
                      case 15: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "BVC"; }

                          
                          
                          
                        }
                        
                        break;
                      default: assertion(false);
                    }    
                break;
              case 6: 
                
                  switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                    case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                    case 7: case 10: case 11: case 12: case 13: case 14: 
                    case 15: 
                      { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction1(pc, out); };} break;
                    case 8: 
                      if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                        int  reloc = 
                          4 * (((int)(( 
                                      (MATCH_w_32_0 & 0x3fffff) 
                                              ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                          ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write( IMMEDTOSTRING( reloc));

                         opname = "FBA,a"; }

                        
                        
                        
                      }   
                      else { 
                        int  reloc = 
                          4 * (((int)(( 
                                      (MATCH_w_32_0 & 0x3fffff) 
                                              ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                          ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write( IMMEDTOSTRING( reloc));

                         opname = "FBA"; }

                        
                        
                        
                      }   
                      
                      break;
                    case 9: 
                      if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                        int  reloc = 
                          4 * (((int)(( 
                                      (MATCH_w_32_0 & 0x3fffff) 
                                              ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                          ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write( IMMEDTOSTRING( reloc));

                         opname = "FBE,a"; }

                        
                        
                        
                      }   
                      else { 
                        int  reloc = 
                          4 * (((int)(( 
                                      (MATCH_w_32_0 & 0x3fffff) 
                                              ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                          ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write( IMMEDTOSTRING( reloc));

                         opname = "FBE"; }

                        
                        
                        
                      }   
                      
                      break;
                    default: assertion(false);
                  }   
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) 
                  
                    switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                      case 0: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CBN,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 1: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB123,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 2: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB12,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 3: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB13,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 4: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB1,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 5: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB23,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 6: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB2,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 7: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB3,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 8: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CBA,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 9: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB0,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 10: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB03,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 11: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB02,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 12: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB023,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 13: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB01,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 14: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB013,a"; }

                          
                          
                          
                        }
                        
                        break;
                      case 15: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB012,a"; }

                          
                          
                          
                        }
                        
                        break;
                      default: assertion(false);
                    }    
                else 
                  
                    switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                      case 0: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CBN"; }

                          
                          
                          
                        }
                        
                        break;
                      case 1: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB123"; }

                          
                          
                          
                        }
                        
                        break;
                      case 2: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB12"; }

                          
                          
                          
                        }
                        
                        break;
                      case 3: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB13"; }

                          
                          
                          
                        }
                        
                        break;
                      case 4: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB1"; }

                          
                          
                          
                        }
                        
                        break;
                      case 5: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB23"; }

                          
                          
                          
                        }
                        
                        break;
                      case 6: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB2"; }

                          
                          
                          
                        }
                        
                        break;
                      case 7: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB3"; }

                          
                          
                          
                        }
                        
                        break;
                      case 8: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CBA"; }

                          
                          
                          
                        }
                        
                        break;
                      case 9: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB0"; }

                          
                          
                          
                        }
                        
                        break;
                      case 10: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB03"; }

                          
                          
                          
                        }
                        
                        break;
                      case 11: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB02"; }

                          
                          
                          
                        }
                        
                        break;
                      case 12: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB023"; }

                          
                          
                          
                        }
                        
                        break;
                      case 13: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB01"; }

                          
                          
                          
                        }
                        
                        break;
                      case 14: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB013"; }

                          
                          
                          
                        }
                        
                        break;
                      case 15: 
                        { 
                          int  reloc = 
                            4 * (((int)(( 
                                        (MATCH_w_32_0 & 0x3fffff) 
                                                ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                            ( MATCH_p ) ;
                          nextPC = 4 + MATCH_p; 
                          
                          

                           { out.write( IMMEDTOSTRING( reloc));

                           opname = "CB012"; }

                          
                          
                          
                        }
                        
                        break;
                      default: assertion(false);
                    }    
                break;
              default: assertion(false);
            }   
          break;
        case 1: case 3: 
          { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction1(pc, out); };} break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f)  ) {
              case 0: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ADD"; }

                  
                  
                  
                }
                
                break;
              case 1: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "AND"; }

                  
                  
                  
                }
                
                break;
              case 2: case 3: case 4: case 6: case 7: case 9: case 10: 
              case 11: case 12: case 13: case 14: case 15: case 18: case 19: 
              case 20: case 22: case 23: case 25: case 26: case 27: case 28: 
              case 29: case 30: case 31: case 32: case 33: case 34: case 35: 
              case 36: case 37: case 38: case 39: case 40: case 41: case 42: 
              case 43: case 44: case 45: case 46: case 47: case 49: case 50: 
              case 51: case 53: case 54: case 55: case 56: case 57: case 58: 
              case 59: case 60: case 61: case 62: case 63: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction1(pc, out); };} break;
              case 5: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ANDN"; }

                  
                  
                  
                }
                
                break;
              case 8: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ADDX"; }

                  
                  
                  
                }
                
                break;
              case 16: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ADDcc"; }

                  
                  
                  
                }
                
                break;
              case 17: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ANDcc"; }

                  
                  
                  
                }
                
                break;
              case 21: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ANDNcc"; }

                  
                  
                  
                }
                
                break;
              case 24: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ADDXcc"; }

                  
                  
                  
                }
                
                break;
              case 48: 
                if ((MATCH_w_32_0 >> 25 & 0x1f)   == 0 && 
                  (0 <= (MATCH_w_32_0 >> 13 & 0x1)   && 
                  (MATCH_w_32_0 >> 13 & 0x1)   < 2)) { 
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", %y" );

                   opname = "WRY"; }

                  
                  
                  
                }   
                else { 
                  int  rdi = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", %asr" );

                  out.write( IMMEDTOSTRING( rdi));

                   opname = "WRASR"; }

                  
                  
                  
                }   
                
                break;
              case 52: 
                if (10 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 65 || 
                  68 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 512) 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction1(pc, out); };}   
                else 
                  switch((MATCH_w_32_0 >> 5 & 0x1ff)  ) {
                    case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                    case 7: case 8: 
                      { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction1(pc, out); };} break;
                    case 9: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FABSs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 65: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1sNames[fs1s] );

                        out.write( ", " );

                        out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FADDs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 66: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FADDd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 67: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FADDq"; }

                        
                        
                        
                      }
                      
                      break;
                    default: assertion(false);
                  }   
                break;
              default: assertion(false);
            }   
          break;
        default: assertion(false);
      }   
    
  } 
  
   
     
    
    
  
}


return nextPC;
}

    int disassembleInstruction1( int pc, Writer out ) throws java.io.IOException {
	int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    
      switch((MATCH_w_32_0 >> 30 & 0x3)  ) {
        case 0: 
          if ((MATCH_w_32_0 >> 22 & 0x7)   == 6) 
            
              switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                case 0: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBN,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBN"; }

                    
                    
                    
                  }   
                  
                  break;
                case 1: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBNE,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBNE"; }

                    
                    
                    
                  }   
                  
                  break;
                case 2: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBLG,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBLG"; }

                    
                    
                    
                  }   
                  
                  break;
                case 3: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUL,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUL"; }

                    
                    
                    
                  }   
                  
                  break;
                case 4: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBL,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBL"; }

                    
                    
                    
                  }   
                  
                  break;
                case 5: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUG,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUG"; }

                    
                    
                    
                  }   
                  
                  break;
                case 6: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBG,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBG"; }

                    
                    
                    
                  }   
                  
                  break;
                case 7: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBU,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBU"; }

                    
                    
                    
                  }   
                  
                  break;
                case 8: case 9: 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };} break;
                case 10: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUE,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUE"; }

                    
                    
                    
                  }   
                  
                  break;
                case 11: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBGE,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBGE"; }

                    
                    
                    
                  }   
                  
                  break;
                case 12: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUGE,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBUGE"; }

                    
                    
                    
                  }   
                  
                  break;
                case 13: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBLE,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBLE"; }

                    
                    
                    
                  }   
                  
                  break;
                case 14: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBULE,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBULE"; }

                    
                    
                    
                  }   
                  
                  break;
                case 15: 
                  if ((MATCH_w_32_0 >> 29 & 0x1)   == 1) { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBO,a"; }

                    
                    
                    
                  }   
                  else { 
                    int  reloc = 
                      4 * (((int)(( 
                                  (MATCH_w_32_0 & 0x3fffff) 
                                          ) << (4*8-(  22 )))) >> (4*8-(  22 )))  + 
                      ( MATCH_p ) ;
                    nextPC = 4 + MATCH_p; 
                    
                    

                     { out.write( IMMEDTOSTRING( reloc));

                     opname = "FBO"; }

                    
                    
                    
                  }   
                  
                  break;
                default: assertion(false);
              }    
          else 
            { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
          break;
        case 1: 
          { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };} break;
        case 2: 
          if (0 <= (MATCH_w_32_0 >> 19 & 0x3f)   && 
            (MATCH_w_32_0 >> 19 & 0x3f)   < 52) 
            { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
          else 
            switch((MATCH_w_32_0 >> 19 & 0x3f)  ) {
              case 54: case 55: case 57: case 58: case 60: case 61: case 62: 
              case 63: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };} break;
              case 52: 
                if (6 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 41 || 
                  111 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 196 || 
                  212 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 512) 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                else 
                  switch((MATCH_w_32_0 >> 5 & 0x1ff)  ) {
                    case 0: case 2: case 3: case 4: case 44: case 45: 
                    case 46: case 47: case 48: case 49: case 50: case 51: 
                    case 52: case 53: case 54: case 55: case 56: case 57: 
                    case 58: case 59: case 60: case 61: case 62: case 63: 
                    case 64: case 65: case 66: case 67: case 68: case 72: 
                    case 76: case 80: case 81: case 82: case 83: case 84: 
                    case 85: case 86: case 87: case 88: case 89: case 90: 
                    case 91: case 92: case 93: case 94: case 95: case 96: 
                    case 97: case 98: case 99: case 100: case 101: case 102: 
                    case 103: case 104: case 106: case 107: case 108: 
                    case 109: case 197: case 202: case 207: case 208: 
                      { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };} break;
                    case 1: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FMOVs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 5: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FNEGs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 41: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FSQRTs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 42: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FSQRTd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 43: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FSQRTq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 69: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1sNames[fs1s] );

                        out.write( ", " );

                        out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FSUBs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 70: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FSUBd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 71: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FSUBq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 73: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1sNames[fs1s] );

                        out.write( ", " );

                        out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FMULs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 74: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FMULd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 75: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FMULq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 77: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1sNames[fs1s] );

                        out.write( ", " );

                        out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FDIVs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 78: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FDIVd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 79: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FDIVq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 105: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FsMULd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 110: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FdMULq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 196: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FiTOs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 198: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FdTOs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 199: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FqTOs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 200: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FiTOd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 201: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FsTOd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 203: 
                      { 
                        int  fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fddNames[fdd] );

                         opname = "FqTOd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 204: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FiTOq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 205: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FsTOq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 206: 
                      { 
                        int  fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fdqNames[fdq] );

                         opname = "FdTOq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 209: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2sNames[fs2s] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FsTOi"; }

                        
                        
                        
                      }
                      
                      break;
                    case 210: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2dNames[fs2d] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FdTOi"; }

                        
                        
                        
                      }
                      
                      break;
                    case 211: 
                      { 
                        int  fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs2qNames[fs2q] );

                        out.write( ", " );

                        out.write(fdsNames[fds] );

                         opname = "FqTOi"; }

                        
                        
                        
                      }
                      
                      break;
                    default: assertion(false);
                  }   
                break;
              case 53: 
                if (0 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 81 || 
                  88 <= (MATCH_w_32_0 >> 5 & 0x1ff)   && 
                  (MATCH_w_32_0 >> 5 & 0x1ff)   < 512) 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                else 
                  switch((MATCH_w_32_0 >> 5 & 0x1ff)  ) {
                    case 84: 
                      { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };} break;
                    case 81: 
                      { 
                        int  fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1sNames[fs1s] );

                        out.write( ", " );

                        out.write(fs2sNames[fs2s] );

                         opname = "FCMPs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 82: 
                      { 
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                         opname = "FCMPd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 83: 
                      { 
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                         opname = "FCMPq"; }

                        
                        
                        
                      }
                      
                      break;
                    case 85: 
                      { 
                        int  fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2s = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1sNames[fs1s] );

                        out.write( ", " );

                        out.write(fs2sNames[fs2s] );

                         opname = "FCMPEs"; }

                        
                        
                        
                      }
                      
                      break;
                    case 86: 
                      { 
                        int  fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2d = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1dNames[fs1d] );

                        out.write( ", " );

                        out.write(fs2dNames[fs2d] );

                         opname = "FCMPEd"; }

                        
                        
                        
                      }
                      
                      break;
                    case 87: 
                      { 
                        int  fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f)  ;
                        int  fs2q = (MATCH_w_32_0 & 0x1f)  ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { out.write(fs1qNames[fs1q] );

                        out.write( ", " );

                        out.write(fs2qNames[fs2q] );

                         opname = "FCMPEq"; }

                        
                        
                        
                      }
                      
                      break;
                    default: assertion(false);
                  }   
                break;
              case 56: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { disassembleaddress_( address_, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "JMPL"; }

                  
                  
                  
                }
                
                break;
              case 59: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { disassembleaddress_( address_, out );

                   opname = "FLUSH"; }

                  
                  
                  
                }
                
                break;
              default: assertion(false);
            }   
          break;
        case 3: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f)  ) {
              case 0: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LD"; }

                  
                  
                  
                }
                
                break;
              case 1: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LDUB"; }

                  
                  
                  
                }
                
                break;
              case 2: case 4: case 5: case 6: case 7: case 8: case 11: 
              case 12: case 14: case 15: case 18: case 20: case 21: case 22: 
              case 23: case 24: case 27: case 28: case 30: case 31: case 34: 
              case 36: case 37: case 38: case 39: case 40: case 41: case 42: 
              case 43: case 44: case 45: case 46: case 47: case 50: case 52: 
              case 53: case 54: case 55: case 56: case 57: case 58: case 59: 
              case 60: case 61: case 62: case 63: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };} break;
              case 3: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LDD"; }

                  
                  
                  
                }
                
                break;
              case 9: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LDSB"; }

                  
                  
                  
                }
                
                break;
              case 10: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LDSH"; }

                  
                  
                  
                }
                
                break;
              case 13: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LDSTUB"; }

                  
                  
                  
                }
                
                break;
              case 16: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                
                break;
              case 17: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDUBA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                
                break;
              case 19: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDDA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                
                break;
              case 25: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDSBA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                
                break;
              case 26: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDSHA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                
                break;
              case 29: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDSTUBA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction2(pc, out); };}   
                
                break;
              case 32: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  fds = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(fdsNames[fds] );

                   opname = "LDF"; }

                  
                  
                  
                }
                
                break;
              case 33: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], %fsr" );

                   opname = "LDFSR"; }

                  
                  
                  
                }
                
                break;
              case 35: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  fdd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(fddNames[fdd] );

                   opname = "LDDF"; }

                  
                  
                  
                }
                
                break;
              case 48: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  cd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(cdNames[cd] );

                   opname = "LDC"; }

                  
                  
                  
                }
                
                break;
              case 49: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], %csr" );

                   opname = "LDCSR"; }

                  
                  
                  
                }
                
                break;
              case 51: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  cd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(cdNames[cd] );

                   opname = "LDDC"; }

                  
                  
                  
                }
                
                break;
              default: assertion(false);
            }   
          break;
        default: assertion(false);
      }   
    
  } 
  
   
     
    
    
  
}


return nextPC;
}

    int disassembleInstruction2( int pc, Writer out ) throws java.io.IOException {
	int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    
      switch((MATCH_w_32_0 >> 30 & 0x3)  ) {
        case 0: 
          
            switch((MATCH_w_32_0 >> 22 & 0x7)  ) {
              case 0: 
                { 
                  int  imm22 = (MATCH_w_32_0 & 0x3fffff)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( IMMEDTOSTRING( imm22));

                   opname = "UNIMP"; }

                  
                  
                  
                }
                
                break;
              case 1: case 2: case 3: case 5: case 6: case 7: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };} break;
              case 4: 
                if ((MATCH_w_32_0 & 0x3fffff)   == 0 && 
                  (MATCH_w_32_0 >> 25 & 0x1f)   == 0) { 
                  nextPC = 4 + MATCH_p; 
                  
                  

                   {  opname = "NOP"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              default: assertion(false);
            }   
          break;
        case 1: 
          { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };} break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f)  ) {
              case 0: case 1: case 3: case 5: case 7: case 8: case 9: 
              case 13: case 16: case 17: case 19: case 21: case 23: case 24: 
              case 25: case 29: case 44: case 45: case 46: case 47: case 48: 
              case 52: case 53: case 54: case 55: case 56: case 59: case 62: 
              case 63: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };} break;
              case 2: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "OR"; }

                  
                  
                  
                }
                
                break;
              case 4: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SUB"; }

                  
                  
                  
                }
                
                break;
              case 6: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ORN"; }

                  
                  
                  
                }
                
                break;
              case 10: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "UMUL"; }

                  
                  
                  
                }
                
                break;
              case 11: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SMUL"; }

                  
                  
                  
                }
                
                break;
              case 12: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SUBX"; }

                  
                  
                  
                }
                
                break;
              case 14: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "UDIV"; }

                  
                  
                  
                }
                
                break;
              case 15: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SDIV"; }

                  
                  
                  
                }
                
                break;
              case 18: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ORcc"; }

                  
                  
                  
                }
                
                break;
              case 20: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SUBcc"; }

                  
                  
                  
                }
                
                break;
              case 22: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "ORNcc"; }

                  
                  
                  
                }
                
                break;
              case 26: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "UMULcc"; }

                  
                  
                  
                }
                
                break;
              case 27: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SMULcc"; }

                  
                  
                  
                }
                
                break;
              case 28: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SUBXcc"; }

                  
                  
                  
                }
                
                break;
              case 30: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "UDIVcc"; }

                  
                  
                  
                }
                
                break;
              case 31: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SDIVcc"; }

                  
                  
                  
                }
                
                break;
              case 32: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "TADDcc"; }

                  
                  
                  
                }
                
                break;
              case 33: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "TSUBcc"; }

                  
                  
                  
                }
                
                break;
              case 34: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "TADDccTV"; }

                  
                  
                  
                }
                
                break;
              case 35: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "TSUBccTV"; }

                  
                  
                  
                }
                
                break;
              case 36: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "MULScc"; }

                  
                  
                  
                }
                
                break;
              case 37: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SLL"; }

                  
                  
                  
                }
                
                break;
              case 38: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SRL"; }

                  
                  
                  
                }
                
                break;
              case 39: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SRA"; }

                  
                  
                  
                }
                
                break;
              case 40: 
                if ((MATCH_w_32_0 >> 14 & 0x1f)   == 0) { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%y,   " );

                  out.write(rdNames[rd] );

                   opname = "RDY"; }

                  
                  
                  
                }   
                else { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  rs1i = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%asr" );

                  out.write( IMMEDTOSTRING( rs1i));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "RDASR"; }

                  
                  
                  
                }   
                
                break;
              case 41: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%psr, " );

                  out.write(rdNames[rd] );

                   opname = "RDPSR"; }

                  
                  
                  
                }
                
                break;
              case 42: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%wim, " );

                  out.write(rdNames[rd] );

                   opname = "RDWIM"; }

                  
                  
                  
                }
                
                break;
              case 43: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%tbr, " );

                  out.write(rdNames[rd] );

                   opname = "RDTBR"; }

                  
                  
                  
                }
                
                break;
              case 49: 
                { 
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", %psr" );

                   opname = "WRPSR"; }

                  
                  
                  
                }
                
                break;
              case 50: 
                { 
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", %wim" );

                   opname = "WRWIM"; }

                  
                  
                  
                }
                
                break;
              case 51: 
                { 
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", %tbr" );

                   opname = "WRTBR"; }

                  
                  
                  
                }
                
                break;
              case 57: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { disassembleaddress_( address_, out );

                   opname = "RETT"; }

                  
                  
                  
                }
                
                break;
              case 58: 
                
                  switch((MATCH_w_32_0 >> 25 & 0xf)  ) {
                    case 0: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TN"; }

                        
                        
                        
                      }
                      
                      break;
                    case 1: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TE"; }

                        
                        
                        
                      }
                      
                      break;
                    case 2: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TLE"; }

                        
                        
                        
                      }
                      
                      break;
                    case 3: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TL"; }

                        
                        
                        
                      }
                      
                      break;
                    case 4: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TLEU"; }

                        
                        
                        
                      }
                      
                      break;
                    case 5: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TCS"; }

                        
                        
                        
                      }
                      
                      break;
                    case 6: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TNEG"; }

                        
                        
                        
                      }
                      
                      break;
                    case 7: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TVS"; }

                        
                        
                        
                      }
                      
                      break;
                    case 8: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TA"; }

                        
                        
                        
                      }
                      
                      break;
                    case 9: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TNE"; }

                        
                        
                        
                      }
                      
                      break;
                    case 10: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TG"; }

                        
                        
                        
                      }
                      
                      break;
                    case 11: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TGE"; }

                        
                        
                        
                      }
                      
                      break;
                    case 12: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TGU"; }

                        
                        
                        
                      }
                      
                      break;
                    case 13: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TCC"; }

                        
                        
                        
                      }
                      
                      break;
                    case 14: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TPOS"; }

                        
                        
                        
                      }
                      
                      break;
                    case 15: 
                      { 
                        int  address_ = ( MATCH_p ) ;
                        nextPC = 4 + MATCH_p; 
                        
                        

                         { disassembleaddress_( address_, out );

                         opname = "TVC"; }

                        
                        
                        
                      }
                      
                      break;
                    default: assertion(false);
                  }   
                break;
              case 60: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SAVE"; }

                  
                  
                  
                }
                
                break;
              case 61: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "RESTORE"; }

                  
                  
                  
                }
                
                break;
              default: assertion(false);
            }   
          break;
        case 3: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f)  ) {
              case 0: case 1: case 3: case 8: case 9: case 10: case 11: 
              case 12: case 13: case 14: case 16: case 17: case 19: case 24: 
              case 25: case 26: case 27: case 28: case 29: case 30: case 32: 
              case 33: case 34: case 35: case 40: case 41: case 42: case 43: 
              case 44: case 45: case 46: case 47: case 48: case 49: case 50: 
              case 51: case 56: case 57: case 58: case 59: case 60: case 61: 
              case 62: case 63: 
                { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };} break;
              case 2: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "LDUH"; }

                  
                  
                  
                }
                
                break;
              case 4: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "ST"; }

                  
                  
                  
                }
                
                break;
              case 5: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STB"; }

                  
                  
                  
                }
                
                break;
              case 6: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STH"; }

                  
                  
                  
                }
                
                break;
              case 7: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STD"; }

                  
                  
                  
                }
                
                break;
              case 15: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleaddress_( address_, out );

                  out.write( "], " );

                  out.write(rdNames[rd] );

                   opname = "SWAP."; }

                  
                  
                  
                }
                
                break;
              case 18: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "LDUHA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              case 20: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                   opname = "STA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              case 21: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                   opname = "STBA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              case 22: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                   opname = "STHA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              case 23: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rdNames[rd] );

                  out.write( ", [" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                   opname = "STDA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              case 31: 
                if ((MATCH_w_32_0 >> 13 & 0x1)   == 0) { 
                  int  asi = (MATCH_w_32_0 >> 5 & 0xff)  ;
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  regaddr = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "[" );

                  disassembleregaddr( regaddr, out );

                  out.write( "]" );

                  out.write( IMMEDTOSTRING( asi));

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "SWAPA"; }

                  
                  
                  
                }   
                else 
                  { 
      nextPC = MATCH_p; 
      
      

      {nextPC = disassembleInstruction3(pc, out); };}   
                
                break;
              case 36: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  fds = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(fdsNames[fds] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STF"; }

                  
                  
                  
                }
                
                break;
              case 37: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%fsr, [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STFSR"; }

                  
                  
                  
                }
                
                break;
              case 38: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%fq,  [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STDFQ"; }

                  
                  
                  
                }
                
                break;
              case 39: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  fdd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(fddNames[fdd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STDF"; }

                  
                  
                  
                }
                
                break;
              case 52: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  cd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(cdNames[cd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STC"; }

                  
                  
                  
                }
                
                break;
              case 53: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%csr, [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STCSR"; }

                  
                  
                  
                }
                
                break;
              case 54: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write( "%cq,  [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STDCQ"; }

                  
                  
                  
                }
                
                break;
              case 55: 
                { 
                  int  address_ = ( MATCH_p ) ;
                  int  cd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(cdNames[cd] );

                  out.write( ", [" );

                  disassembleaddress_( address_, out );

                  out.write( "]" );

                   opname = "STDC"; }

                  
                  
                  
                }
                
                break;
              default: assertion(false);
            }   
          break;
        default: assertion(false);
      }   
    
  } 
  
   
     
    
    
  
}


return nextPC;
}

    int disassembleInstruction3( int pc, Writer out ) throws java.io.IOException {
	int nextPC = pc;



{ 
  int  MATCH_p = 
    
    

    pc
    ;
  int  MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getMemint( MATCH_p ) ; 
    
      switch((MATCH_w_32_0 >> 30 & 0x3)  ) {
        case 0: 
          if ((MATCH_w_32_0 >> 22 & 0x7)   == 4) { 
            int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
            int  val = (MATCH_w_32_0 & 0x3fffff)   << 10;
            nextPC = 4 + MATCH_p; 
            
            

             { out.write( "%hi(" );

            out.write( IMMEDTOSTRING( val));

            out.write( "), " );

            out.write(rdNames[rd] );

             opname = "sethi"; }

            
            
            
          }   
          else 
            { 
      nextPC = MATCH_p; 
      
      

      { opname = "#ERR#"; ; };}   
          
          break;
        case 1: 
          { 
            int  reloc = 
              4 * (((int)(( (MATCH_w_32_0 & 0x3fffffff)   ) << (4*8-(  
                          30 )))) >> (4*8-(                             30 )))  + ( MATCH_p ) ;
            nextPC = 4 + MATCH_p; 
            
            

             { out.write( callName( reloc));

             opname = "call__"; }

            
            
            
          }
          
          break;
        case 2: 
          if (24 <= (MATCH_w_32_0 >> 19 & 0x3f)   && 
            (MATCH_w_32_0 >> 19 & 0x3f)   < 64) 
            { 
      nextPC = MATCH_p; 
      
      

      { opname = "#ERR#"; ; };}   
          else 
            switch((MATCH_w_32_0 >> 19 & 0x3f)  ) {
              case 0: case 1: case 2: case 4: case 5: case 6: case 8: case 9: 
              case 10: case 11: case 12: case 13: case 14: case 15: case 16: 
              case 17: case 18: case 20: case 21: case 22: 
                { 
      nextPC = MATCH_p; 
      
      

      { opname = "#ERR#"; ; };} break;
              case 3: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "XOR"; }

                  
                  
                  
                }
                
                break;
              case 7: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "XNOR"; }

                  
                  
                  
                }
                
                break;
              case 19: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "XORcc"; }

                  
                  
                  
                }
                
                break;
              case 23: 
                { 
                  int  rd = (MATCH_w_32_0 >> 25 & 0x1f)  ;
                  int  reg_or_imm = ( MATCH_p ) ;
                  int  rs1 = (MATCH_w_32_0 >> 14 & 0x1f)  ;
                  nextPC = 4 + MATCH_p; 
                  
                  

                   { out.write(rs1Names[rs1] );

                  out.write( ", " );

                  disassemblereg_or_imm( reg_or_imm, out );

                  out.write( ", " );

                  out.write(rdNames[rd] );

                   opname = "XNORcc"; }

                  
                  
                  
                }
                
                break;
              default: assertion(false);
            }   
          break;
        case 3: 
          { 
      nextPC = MATCH_p; 
      
      

      { opname = "#ERR#"; ; };} break;
        default: assertion(false);
      }   
    
  }
}


return nextPC;
}

    int disassembleInstruction4( int pc, Writer out ) throws java.io.IOException {
        return 0;
    }

    int disassembleInstruction5( int pc, Writer out ) throws java.io.IOException {
        return 0;
    }

    int disassembleInstruction6( int pc, Writer out ) throws java.io.IOException {
        return 0;
    }

    int disassembleInstruction7( int pc, Writer out ) throws java.io.IOException {
        return 0;
    }

    int disassembleInstruction8( int pc, Writer out ) throws java.io.IOException {
        return 0;
    }

    int disassembleInstruction9( int pc, Writer out ) throws java.io.IOException {
        return 0;
    }

    int sprintDisassembleInst( int pc, Writer out ) throws java.io.IOException {
        StringBuffer s = new StringBuffer();
        StringWriter sw = new StringWriter();

	out.flush();        
        int npc = disassembleInstruction( pc, sw );
        
        s.append( formatHex( pc, 8 ) );
        s.append( ": " );
        for( int i = pc; i < npc; i++ ) {
            s.append( formatHex( ((int)getMembyte(i))&0xFF, 2 ) );
            s.append( " " );
        }
        for( int i = npc-pc; i<6; i++ )
            s.append("   ");
        s.append( (rightPad(opname,10)).toLowerCase() );
        s.append( sw.toString() );
        s.append( '\n' );
        out.write(s.toString());
        return npc;
    }

    void disassembleRegion( int pc, int to, Writer out ) throws java.io.IOException {
        while( pc < to ) {
            pc = sprintDisassembleInst( pc, out );
        }
        returnStringValue = out.toString();
    }

    public void disassembleRegion( int pc, int to ) throws java.io.IOException {
        disassembleRegion( pc, to, new StringWriter() );
    }

    String returnDisasm() {
        return returnStringValue.toLowerCase();
    }

    
    
    
}



