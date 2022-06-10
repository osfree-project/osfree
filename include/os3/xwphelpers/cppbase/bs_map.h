
/*
 *@@sourcefile bs_list.h:
 *      replacement for the STL list class template.
 */

#ifndef BSMAP_INCLUDED
    #define BSMAP_INCLUDED

    /* ******************************************************************
     *
     *  Replacement map class
     *
     ********************************************************************/

    /*
     *@@ BSMap:
     *
     */

    template <class P, FNTREE_COMPARE *pfnCompare>
    struct BSMap
    {
        TREE            *_TreeRoot;
        LONG            _cEntries;

        BSMap()
        {
            treeInit(&_TreeRoot, &_cEntries);
        }

        int inline insert(P p)
        {
            return (treeInsert(&_TreeRoot,
                               &_cEntries,
                               (TREE*)p,
                               pfnCompare));
        }

        P inline first()
        {
            return (P)treeFirst(_TreeRoot);
        }

        P inline next(P p)
        {
            return (P)treeNext((TREE*)p);
        }
    };

    /*
     *@@ StringMapEntry:
     *      string map entry for codepaged strings.
     */

    struct StringMapEntry
    {
        TREE            _Tree;
                        // ulKey has a const pointer to the strFuncName.c_str()
        const string    _strKey;
        string          _strValue;

        StringMapEntry(const string &strKey,
                       const string &strValue)
            : _strKey(strKey),
              _strValue(strValue)
        {
            // store PCSZ strKey as the tree key
            _Tree.ulKey = (ULONG)_strKey.c_str();
        }

        StringMapEntry(PCSZ pcszKey,
                       PCSZ pcszValue)
            : _strKey(pcszKey),
              _strValue(pcszValue)
        {
            // store PCSZ strKey as the tree key
            _Tree.ulKey = (ULONG)_strKey.c_str();
        }
    };

    /*
     *@@ UStringMapEntry:
     *      string map entry for Unicode (UTF-8) strings.
     *
     *@@added V0.9.20 (2002-07-03) [umoeller]
     */

    struct UStringMapEntry
    {
        TREE            _Tree;
                        // ulKey has a const pointer to the strFuncName.c_str()
        const ustring   _ustrKey;
        ustring         _ustrValue;

        UStringMapEntry(const ustring &ustrKey,
                        const ustring &ustrValue)
            : _ustrKey(ustrKey),
              _ustrValue(ustrValue)
        {
            // store PCSZ strKey as the tree key
            _Tree.ulKey = (ULONG)_ustrKey.GetBuffer();
        }
    };

#endif


