/*
 *
 *
 */

#ifndef __OS3_STACKSW__
#define __OS3_STACKSW__

#ifdef __cplusplus
  extern "C" {
#endif

static inline unsigned long
getstack(void)
{
  unsigned long cur_stack;

  asm volatile("movl  %%esp, %%eax \n\t"
               "movl  %%eax, %0 \n\t"
               :"=m"  (cur_stack)
               :);

  return cur_stack;
}

static inline void
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

static inline void
stackout(void)
{
  asm volatile("popl  %%eax \n\t"
               "popl  %%eax \n\t"
               "movl  %%eax, %%esp \n\t"
               :
               :);
}

#ifdef __SWITCH_STK__
#define STKINIT(a) __stack = getstack(); stackin(a);
#define STKIN  stackin(__stack);
#define STKOUT stackout();
#else
#define STKINIT(a)
#define STKIN
#define STKOUT
#endif

#ifdef __cplusplus
  }
#endif

#endif
