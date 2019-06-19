/* F_pgl.hpp */
#ifndef  F_PGL_HPP
  #define F_PGL_HPP

/* structure for visual configuration */
/* Non  compatible with IBM's pgl.h ! */
typedef struct visualconfig 
{
   unsigned int  vid;  /*Visual ID*/
   int rgbFlag;
   int dbFlag;
   int stereoFlag;
   int redBits;
   int greenBits;
   int blueBits;
   int alphaBits;
   int indexBits;
   int depthBits;
   int stencilBits;
   int accumRedBits;
   int accumGreenBits;
   int accumBlueBits;
   int accumAlphaBits;
   unsigned int redMask, greenMask, blueMask, alphaMask;

   void * reserved;
   struct visualconfig *next;
} VISUALCONFIG, *PVISUALCONFIG;
#endif
   //F_PGL_HPP