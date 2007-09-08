/*
 *   Load executable image
 *
 *   \param   image      -- executable image phys addrerss
 *   \param   size       -- its size
 *   \param   load_addr  -- address to load.
 *                          must be zero if load at linked-in address
 *   \retval  p          -- structure with loaded image properties,
 *                          such as cs:[e]ip, ss:[e]sp etc.
 *   \return  0          -- if loaded successfully,
 *                          other value if error
 */

//int load(char far *image, unsigned long size, char far *load_addr, struct exe_params *p);
int load(unsigned long image, unsigned long size, unsigned long load_addr, struct exe_params *p);

struct exe_params
{
   unsigned short  cs;
   unsigned long   eip;
   unsigned short  ss;
   unsigned long   esp;
}
