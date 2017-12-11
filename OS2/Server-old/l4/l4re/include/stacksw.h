/*
 *
 *
 */

#ifndef __STACK_SW__
#define __STACK_SW__
 
L4_INLINE unsigned long
getstack(void)
{
  unsigned long cur_stack;

  asm volatile("movl  %%esp, %%eax \n\t"
               "movl  %%eax, %0 \n\t" 
               :"=m"  (cur_stack)
	       :);
	       
  return cur_stack;
}

L4_INLINE void
stackin(unsigned long new_stack)
{
  asm volatile("movl   %0, %%edx \n\t"
               "subl   $4, %%edx \n\t"
               "movl   %%esp,  %%eax \n\t"
               "movl   %%edx,  %%esp \n\t"
               "pushl  %%eax \n\t" 
               "pushl  %%eax \n\t" 
	       :
	       :   "m"  (new_stack));
}

L4_INLINE void
stackout(void)
{
  asm volatile("popl  %%eax \n\t"
               "popl  %%edx \n\t"
	       "movl  %%eax, %%esp \n\t"
	       :
	       :);
}

//#define __SWITCH_STK__

#ifdef __SWITCH_STK__
#define STKINIT(a) __stack = getstack(); stackin(a);
#define STKIN  stackin(__stack);
#define STKOUT stackout();
#else
#define STKINIT(a)
#define STKIN
#define STKOUT
#endif

#endif
