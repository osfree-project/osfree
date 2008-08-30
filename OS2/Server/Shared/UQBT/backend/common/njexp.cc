/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        njexp.cc
 * OVERVIEW:    This file contains the implementation of the NJExpander class.
 *              This is a subclass of the Expander class, and a super class of
 *              all classes that need the NJMCTK encoding routines for
 *              code generation.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.3 $
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "njexp.h"
#include "options.h"
#include "ElfBinaryFile.h"

/*==============================================================================
 * NJMCTK declarations.
 *============================================================================*/

extern "C" {
#include <stdarg.h>
}

/*==============================================================================
 * FUNCTION:    njmctk_fail (NJMCTK)
 * OVERVIEW:    Private error handler for closures.
 * PARAMETERS:  printf's like
 * RETURNS:     <nothing>
 *============================================================================*/
static void njmctk_fail(char *fmt, ...)
{
    char *sym_name;
    va_list args;

    // Dump message
    va_start(args, fmt);
    sym_name = va_arg(args, char*);
    fprintf(stdout, fmt, sym_name);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);

    // Halt
    exit(1);
}

/*==============================================================================
 * FUNCTION:    mc_alloc (NJMCTK)
 * OVERVIEW:    Allocates a new NJ object.
 * PARAMETERS:  size: Size of the object
 *              pool: Type of the object
 * RETURNS:     <nothing>
 *============================================================================*/
extern "C" void *mc_alloc(int size, Mc_alloc_pool pool)
{
    return (void *)malloc(size);
}

/*==============================================================================
 * FUNCTION:    mc_alloc_closure (NJMCTK)
 * OVERVIEW:    Creates a closure at the current relocatable block.
 * PARAMETERS:  size:       Size of the closure.
 *              dest_block: Target relocatable block.
 *              dest_lc:    Target PC
 * RETURNS:     New closure
 *============================================================================*/
set <RClosure> closure;     // Closure set
extern "C" RClosure mc_alloc_closure (unsigned size, RBlock dest_block, unsigned dest_lc)
{
    RClosure cl = (RClosure)malloc(size);
    closure.insert(cl);
    return cl;
}

/*==============================================================================
 * NJExpander class implementation.
 *============================================================================*/

/*==============================================================================
 * FUNCTION:    NJExpander::NJExpander
 * OVERVIEW:    Constructor for class NJExpander
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
NJExpander::NJExpander()
{
    // Initialize data
    fail = &njmctk_fail;    // Error handler
    rawCode = NULL;         // Non-allocated raw zone
}

/*==============================================================================
 * FUNCTION:    Expander::~NJExpander
 * OVERVIEW:    Destructor for class NJExpander
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
NJExpander::~NJExpander()
{
    // Delete binary code
    free(rawCode);

    // Delete relocatable block (procedure) - FIXME?
    // free(text);
}

/*==============================================================================
 * FUNCTION:    NJExpander::updateLocalRAddr
 * OVERVIEW:    Update the local relacatable address mapping.
 *              If there is no entry for the address, add a new one.
 * PARAMETERS:  lbl: label for target BB.
 * RETURNS:     Related relocatable address
 *============================================================================*/
RAddr NJExpander::updateLocalRAddr(PBB pTgt)
{
    // BB must have a defined label
    int lbl = pTgt->getLabel();
    assert(lbl != 0);

    // Check for local relocatable address
    if (relLocalAddr.find(lbl) == relLocalAddr.end())
    {
        // Address not found, let's create a label & relocatable address
        relLocalAddr[lbl] = addr_new(label_new(NULL), /* current label */0);
    }

    // Return relocatable address
    return relLocalAddr[lbl];
}

/*==============================================================================
 * FUNCTION:    NJExpander::updateGlobalRAddr
 * OVERVIEW:    Update the global relacatable address mapping.
 *              Add *always* a new entry for the address.
 * PARAMETERS:  sName: name of target symbol.
 * RETURNS:     Related relocatable address
 *============================================================================*/
RAddr NJExpander::updateGlobalRAddr(const char *sName)
{
    // Create a new label and relocatable address
    RAddr target = addr_new(label_new((char*)sName), 0); 
    label_define(target->label, /* Unknown */0);

    // Update global relocation information
    globReloc[/*offset*/ block_lc(text)] = string(sName);

    // Return relocatable address
    return target;
}

/*==============================================================================
 * FUNCTION:    NJExpander::emitLabel
 * OVERVIEW:    Emits a label for the current basic block.
 * PARAMETERS:  pTgt: Pointer to target BB.
 * RETURNS:     <nothing>
 *============================================================================*/
void NJExpander::emitLabel(PBB pTgt)
{
    // Update local relocatable info & set label
    label_define(updateLocalRAddr(pTgt)->label, /* current offset */0);

    // Emit function prolog for entry BB
    if (pTgt == proc->getEntryBB())
        emitProlog();
}

/*==============================================================================
 * FUNCTION:    NJExpander::alignd ;-)
 * OVERVIEW:    Get new aligned offset for data.
 *              Takes into account the size of the data to allocate.
 * PARAMETERS:  current: Current offset in bytes.
 *              size:    Data size (in bits!!!).
 *              alignto: Data must be aligned to ... in bytes.
 * RETURNS:     New offset in bytes.
 *============================================================================*/
int NJExpander::alignd(int current, int size, int alignto = 0)
{
    // Convert bits to bytes (round up)
    size = (size >> 3) + (size & 0x7 ? 1 : 0);

    // Alignment for single data
    if (alignto == 0)
    {
        switch (size)
        {
            case 1:     // char
                return current + 1;
            case 2:     // short
            case 4:     // word
            case 8:     // long
                // Get number of bytes
                alignto = size;
                break;
            default:    // other
            {
                // Invalid data size
                ostrstream ost;
                ost << size << " byte invalid data size\n";
                error(str(ost));
                exit(1);
            }
        }
    }

    // Compute new size for align data
    int factor = (current + size) % alignto;
    size += factor ? alignto - factor : 0;
    return current + size;
}

/*==============================================================================
 * FUNCTION:    NJExpander::specificPreProcess
 * OVERVIEW:    Sparc-dependent preprocess function.
 *              The main thing is allocate a RBlock for emitting.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void NJExpander::specificPreProcess()
{
    // Initialize data
    clearLocations();
    ofsPool.clear();
    relLocalAddr.clear();
    closure.clear();

    // Compute the stack size
    frameSize = buildStack();

    // Assign a new relocatable block
    text = block_new(/* hint! */ 0);
    text->label->name = (char *)proc->getName();
    set_address(text, /* offset */ 0);
    set_block(text);
    set_lc(text, 0);
    align(8);

    // Current offset -- must be zero!
    procSize = block_lc(text);
    assert(procSize == 0);
}

/*==============================================================================
 * FUNCTION:    NJExpander::specificPostProcess
 * OVERVIEW:    NJMCTK-dependent postprocess function. Among other things...
 *              - Update the global relocation information
 *              - Update the binary code generated and size
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void NJExpander::specificPostProcess()
{
    // Copy global rellocation info
    /* -> Not needed! It was made "on-the-fly" */

    // Apply closures
    set <RClosure>::const_iterator it;
    for (it = closure.begin(); it != closure.end(); it++)
        apply_closure((*it), cl_emitb, (FailCont)fail);

    // Get procedure size (in bytes) - Update info
    procSize = block_lc(text) - procSize;
    rawSize = procSize;

    // Copy raw code - Reallocate memory first
    rawCode = (void *)realloc(rawCode, rawSize);
    block_copy((unsigned char *)rawCode, text, /* low */0, rawSize);
}

/*==============================================================================
 * FUNCTION:    NJExpander::generateFile
 * OVERVIEW:    Call to right object file generation.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void NJExpander::generateFile()
{
    // Write ELF object file
    ElfBinaryFile ebf;
    ebf.writeObjectFile
        (progOptions.outDir, proc->getName(), rawCode, rawSize, globReloc);
}
