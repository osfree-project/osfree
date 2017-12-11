/*==============================================================================
 * FILE:        reg.h
 * OVERVIEW:    Header information for the Register class.
 *
 * Copyright (C) 2000-2001, The University of Queensland, BT group
 *============================================================================*/

/* 
 * $Revision: 1.10 $
 * 19 Jan 00 - Mike: Added flt member and associated functions
 * 15 Nov 00 - Mike: Added comments
 * 22 Aug 01 - Mike: Trivial change for compiling w/o prev #include of global.h
 */

#ifndef __REG_H__
#define __REG_H__

class Type;                 // In case global.h not earlier #included

/*==============================================================================
 * The Register class summarises one line of the @REGISTERS section of an SSL
 * file. This class is used extensively in sslparser.y, and there is a public
 * member of RTLInstDict called DetRegMap which gives a Register object from
 * a register index (register indices may not always be sequential, hence it's
 * not just an array of Register objects).
 * This class plays a more active role in the Interpreter, which is not yet
 * integrated into uqbt
 *============================================================================*/

class Register {
 public:
  
  Register();       // needed for use in stl classes.
  Register(const Register&);
  Register operator=(const Register& r2);
  bool operator==(const Register& r2) const;
  bool operator<(const Register& r2) const;

  // access and set functins
  void s_name(const char *);
  void s_size(int s) {size = s;}
  void s_float(bool f) {flt = f;}
  void s_address(void *p) {address = p;}

/* These are only used in the interpreter */
  char *g_name() const;
  void *g_address() const {return address;}

  int g_size() const {return size;}
  Type g_type() const;

  /* Set the mapped index. For COVERS registers, this is the lower register
   * of the set that this register covers. For example, if the current register
   * is f28to31, i would be the index for register f28
   * For SHARES registers, this is the "parent" register, e.g. if the current
   * register is %al, the parent is %ax (note: not %eax)
   */
  void s_mappedIndex(int i) {mappedIndex = i;}
  /* Set the mapped offset. This is the bit number where this register starts,
     e.g. for register %ah, this is 8. For COVERS regisers, this is 0 */
  void s_mappedOffset(int i) {mappedOffset = i;}
  /* Get the mapped index (see above) */
  int g_mappedIndex() const {return mappedIndex;}
  /* Get the mapped offset (see above) */
  int g_mappedOffset() const {return mappedOffset;}
  /* Get a bool which is true if this is a floating point register */
  bool isFloat() const {return flt;}

 private:
  char *name;
  short size;
  void *address;
  int mappedIndex;
  int mappedOffset;
  bool flt;             // True if this is a floating point register
};

#endif
