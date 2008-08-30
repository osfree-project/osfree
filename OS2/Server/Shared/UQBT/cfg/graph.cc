/*==============================================================================
 * FILE:       graph.cc
 * OVERVIEW:   Writes a dot file that can be used to display the CFG
 *             visually.
 *
 * Copyright (C) 1997-1999, The University of Queensland, BT group
 *============================================================================*/

/* Dec 97 - Mike, created from code in dasm and dcc.
 * 11 Mar 98 - Cristina  
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 24 Mar 98 - Mike
 *	Changed CreateDotFile to WriteDotFile so can get each CFG in the
 *	dot graph
 * 26 Mar 98 - Cristina
 *	Added check for interprocedural out edges in CreateDotFile() so
 * 		that they are not displayed at present (as only 1 graph is processed).
 * 22 Oct 98 - Mike: Added cyan colour for computed calls
 * 22 Jan 99 - Mike: Replaced m_it[First,Last]Rtl with m_pRtls
 * 27 Jan 99 - Mike: Use COMPJUMP and COMPCALL now
 * 09 Apr 99 - Mike: WriteDotFile() takes the entry address now
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 25 Jun 99 - Mike: Added code to display in-edges (conditionally)
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "ss.h"
#include "cfg.h"
#include "rtl.h"
#include "proc.h"
#include "prog.h"
#include "BinaryFile.h"

/*==============================================================================
 * FUNCTION:        Cfg::writeDotFile
 * OVERVIEW:        Append to a ".dot" file that can then be passed to graphviz
 *                  to display the call graph for the whole program.
 * PARAMETERS:      fi - an open ".dot" file
 *                  pName - name of procedure
 *                  iOffset -
 *                  pBF - Pointer to the BinaryFile object that loaded the
 *                    input binary file
 *                  uEntryAddr -
 * RETURNS:         <nothing>
 *============================================================================*/
bool Cfg::writeDotFile(FILE* fi, const char* pName, int iOffset,
    BinaryFile* pBF, ADDRESS uEntryAddr)	const
{
	// It should be well formed
	if (!m_bWellFormed) return false;
 
	// This variable will be set to the node number (m_first + iOffset) of the
	// node with the entry point address. This is necessary now that the list
	// of BBs gets sorted by native address. (Even though most procedures
	// start at their lowest address, orphans with addresses of zero sort
	// to the top.)
	int iEntryNode = -1;

    /* Create node for this cfg */
    fprintf(fi, "\t\"%s\" [shape=box,style=filled,color=yellow];\n", pName);

	// Sort the BBs by address
	

	for (BB_CIT it=m_listBB.begin(); it != m_listBB.end(); it++)
	{
		if ((*it)->getLowAddr() == uEntryAddr)
			iEntryNode = (*it)->m_first + iOffset;

        if ((*it)->m_nodeType == CALL)
		{
			fprintf(fi,"\t%d [shape=box,style=bold,label=\"%X: call ",
				(*it)->m_first + iOffset,
				(*it)->getLowAddr());
			HLCall* hlCall = static_cast<HLCall*>((*it)->m_pRtls->back());
			ADDRESS uAddr = hlCall->getFixedDest();
			// Next see if it is a library function
			const char* pName;
			Proc* pProc = prog.findProc(uAddr);
			if (pProc)
				fprintf(fi, "%s", pProc->getName());
			else if ((pName = pBF->SymbolByAddress(uAddr)) != 0)
				fprintf(fi, "%s", pName);
            else
		    	fprintf(fi, "?");
            fprintf(fi," \"];\n");
		}
        else
		{   /* other type BB */
			string nt; string shape("ellipse");
			switch ((*it)->m_nodeType)
			{
				case ONEWAY: nt = "jump"; break;
				case TWOWAY: nt = "if"; shape = "diamond"; break;
				case NWAY: nt = "case"; shape = "trapezium"; break;
				case RET:  nt = "ret"; shape = "triangle"; break;
				case FALL: nt = "fall"; break;
				case COMPJUMP: nt = "computed jump";
					shape = "doublecircle"; break;
				case COMPCALL: nt = "computed call";
					shape = "box,style=filled,color=cyan"; break;
				default: nt = "unknown";
			}
			ADDRESS uAddr = (*it)->getLowAddr();
			if (uAddr == 0 && (*it)->m_nodeType != RET)
            	fprintf(fi,"\t%d [shape=parallelogram,label=\"orphan\","
					"fontname=\"Times-Italic\"];\n",
					(*it)->m_first + iOffset);
			else
            	fprintf(fi,"\t%d [shape=%s,label=\"%X: %s\"];\n",
					(*it)->m_first + iOffset,
					shape.c_str(),
					uAddr,
					nt.c_str());
        }
    }

	if (iEntryNode == -1)
	{
		ostrstream ost;
		ost << "Procedure " << pName << "'s entry point (";
		ost << hex << uEntryAddr << " was not found. Dot file may be wrong.";
		warning(str(ost));
		// Join label to first BB
		if (m_listBB.size())
			fprintf(fi, "\t\"%s\" -> %d;\n", pName,
				(m_listBB.front())->m_first + iOffset);
	}
	else
	{
		// Join to the BB with the entry point (may not be the first address)
		fprintf(fi, "\t\"%s\" -> %d;\n", pName, iEntryNode);
	}

	const char* pDotted;
	for (BB_CIT it1=m_listBB.begin(); it1 != m_listBB.end(); it1++)
	{
		for (int i = 0; i < (*it1)->m_iNumOutEdges; i++)
		{
			// check if interprocedural edge, if so, don't display it
//			if ((*it1)->m_OutEdgeInterProc[i] == false)
//			{
				pDotted = "";			// Usually not dotted style
        		if (((*it1)->m_nodeType == TWOWAY) && (i == 1))
					pDotted = " [style=dotted]";
            	fprintf(fi, "\t%d -> %d%s;\n", (*it1)->m_first + iOffset,
					(*it1)->m_OutEdges[i]->m_first + iOffset, pDotted);
//			}
			// else, don't display interproc edge
		}
//#define SHOW_INEDGES 1
#ifdef SHOW_INEDGES
		for (int i=0; i < (*it1)->m_iNumInEdges; i++) {
			fprintf(fi, "\t%d -> %d [color=green];\n", (*it1)->m_first + iOffset,
				(*it1)->m_InEdges[i]->m_first + iOffset);
		}
#endif
    }

	return true;
}
