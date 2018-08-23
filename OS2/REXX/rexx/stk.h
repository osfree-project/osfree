#define EXTRA_STACK_SIZE 0x400000

extern char *extra_stack;
extern char *old_stack;

APIRET APIENTRY init (void);
APIRET APIENTRY term (void);

static inline void stkon (void)
{
  __asm 
  {
    mov  eax, esp
    mov  old_stack, eax
    mov  eax, extra_stack
    mov  esp, eax
  }
}

static inline void stkoff (void)
{
  __asm 
  {
    mov  eax, old_stack
    mov  esp, eax
  }
}
