
/*
 * bs_string.h:
 *      header file for the various string classes.
 *
 *      This declares BSString, BSUString, and
 *      BSUniCodec, among other things. These give
 *      you not only a very efficient basic string
 *      class (BSString), but also full Unicode
 *      and codepage support and conversion between
 *      them.
 *
 *      Warning: if you include this header, you'll
 *      get a typedef for "string" to BSString and
 *      for "ustring" to BSUString, which might
 *      conflict with other string classes that you
 *      might be using.
 *
 *@@include #include "helpers\stringh.h"
 *@@include #include "helpers\xstring.h"
 *@@include #include "base\bs_string.h"
 */

/*
 *      This file Copyright (C) 1999-2015 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef WARPIN_STRING_HEADER_INCLUDED
    #define WARPIN_STRING_HEADER_INCLUDED

    typedef unsigned long size_type;

    class BSString;
    class BSUString;

    // extern BSMutex G_mtxStrings;

    // #define STRINGLOCK   BSLock lock(G_mtxStrings)

    #define STRINGLOCK

    /*
     *@@ BSUniCodec:
     *      Unicode conversion object (COder and
     *      DECoder).
     *
     *      Create one such object to allow conversion
     *      from UTF-8 to codepage-specific and back.
     *      An instance of this is required as input
     *      to the conversion methods from BSString
     *      to BSUString and vice versa, such as:
     *
     *      --  BSString::assignUtf8 (convert UTF-8
     *          to codepage-specific)
     *
     *      --  BSString::appendUtf8 (append UTF-8
     *          to codepage-specific)
     *
     *      --  BSUString::assignCP (convert
     *          codepage-specific to UTF-8)
     *
     *      This thing throws BSExcptBase if the
     *      input codepage is not supported or if
     *      we run into invalid code sequences.
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    class BSUniCodec : public BSRoot
    {
        private:
            void            *_pCodec;
            unsigned short  _usCodepage;
            char            _achDBCS[12];
            BOOL            _fDouble;       // TRUE if double-byte codepage

        public:
            DECLARE_CLASS(BSUniCodec);

            BSUniCodec(unsigned short usCodepage);
            ~BSUniCodec();

            void Codepage2Uni(BSUString &ustr,
                              const char *pcszCP,
                              unsigned long ulLength);

            void Uni2Codepage(BSString &str,
                              const char *pcszUni,
                              unsigned long ulLength);

            /*
             *@@ QueryCodepage:
             *      returns the codepage that this
             *      BSUniCodec was created for.
             */

            inline unsigned short QueryCodepage()
            {
                return _usCodepage;
            }
    };

    /*
     *@@ BSStringBuf:
     *      string buffer encapsulation for BSStringBase.
     *
     *      Several BSStringBase instances can share
     *      the same string buffer when copied from
     *      one another. This is new with V0.9.18,
     *      while previously each BSString had its own
     *      buffer, which lead to a lot of redundant
     *      copying.
     *
     *      Note that all methods are private in order
     *      not to let anyone mess with these things.
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    class BSStringBuf : public BSRoot
    {
        friend class BSStringBase;
        friend class BSString;
        friend class BSUString;

        private:
            XSTRING         _str;

            size_t          *_pShiftTable;
            unsigned long   _fRepeat;

            unsigned long   _cShared;       // if 0, buf is used only by one
                                            // instance; otherwise, no. of
                                            // BSString's that share this buf

            DECLARE_CLASS(BSStringBuf);

            /*
             *@@ BSStringBuf:
             *      constructor to create a
             *      new member XSTRING from the
             *      given C string.
             */

            BSStringBuf(const char *pcsz,
                        unsigned long ulLength,
                        unsigned long cbAllocate)
                : BSRoot(tBSStringBuf)
            {
                xstrInit(&_str, cbAllocate);
                xstrcpy(&_str, pcsz, ulLength);
                _pShiftTable = NULL;
                _fRepeat = 0;
                _cShared = 0;
            }

            /*
             *@@ BSStringBuf:
             *      evil second constructor
             *      to take over an existing
             *      XSTRING. Used by BSString::_take_from
             *      only.
             */

            BSStringBuf(XSTRING &str)
                : BSRoot(tBSStringBuf)
            {
                memcpy(&_str, &str, sizeof(XSTRING));
                _pShiftTable = NULL;
                _fRepeat = 0;
                _cShared = 0;

                // and nuke the source XSTRING so
                // that the caller won't free it
                memset(&str, 0, sizeof(XSTRING));
            }

            /*
             *@@ ~BSStringBuf:
             *      destructor. Frees all memory associated
             *      with the buffer.
             */

            ~BSStringBuf()
            {
                ClearShiftTable();
                xstrClear(&_str);
            }

            /*
             *@@ ClearShiftTable:
             *      clears the internal shift
             *      table when the string buffer
             *      has changed.
             */

            inline void ClearShiftTable()
            {
                if (_pShiftTable)
                {
                    free(_pShiftTable);
                    _pShiftTable = 0;
                }
                _fRepeat = FALSE;
            }
    };

    /*
     *@@ BSStringBase:
     *      common superclass for BSString and
     *      BSUString. This implements most of
     *      the functionality for the two, but
     *      since the constructor is protected,
     *      this cannot be created as such.
     *
     *      BSString and BSUString are implemented
     *      as subclasses of this to prevent illegal
     *      assignments between codepage-specific
     *      and UTF-8 strings already by the compiler.
     *
     *      This has a BSStringBuf pointer as
     *      a member so that several strings can
     *      share the same memory.
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    class BSStringBase : public BSRoot
    {
        public:
            DECLARE_CLASS(BSStringBase);

        #ifdef __IBMCPP__
            friend class BSStringBase;
        #endif
        // 2000-01-15: This line is extraordinary useless - class instances
        //             of the same type are always friends (czw)
        // V0.9.1 (2000-02-01) [umoeller]: Hi Jens. Not with VAC++, apparently,
        //             because without this line, this wouldn't compile.

        protected:
            BSStringBuf     *_pBuf;

            /********************************************
             *
             *  protected methods
             *
             ********************************************/

            /*
             *@@ Init:
             *      private helper method to initialize
             *      the instance. Called by all constructors.
             */

            void Init();

            void FreeBuf();

            void CopyFrom(const BSStringBase &s);
            void CopyFrom(const BSStringBase &s,
                          size_type ulPos,
                          size_type n = npos);
            void CopyFrom(const char *psz);
            void CopyFrom(const char *p1, const char *p2);
            void CopyFrom(char c);

            BSStringBase(BSClassID &Class);

        private:

            /*
             *@@ BSStringBase:
             *      private default copy constructor
             *      to make sure this can never be
             *      called by the subclasses.
             */

            BSStringBase(const BSStringBase &s)
                : BSRoot(s._Class)
            {
            };

        protected:

            BSStringBase(const BSStringBase &s,
                         BSClassID &Class);

            virtual ~BSStringBase();

        public:

            static const size_type npos;

            /********************************************
             *
             *  property queries
             *
             ********************************************/

            /*
             *@@ size:
             *      returns the no. of characters in
             *      the string (excluding our internal
             *      null terminator) or 0 if the string
             *      is empty.
             *
             *      Be warned, if the string is UTF-8
             *      encoded, this will not take multi-byte
             *      encodings into account. This returns
             *      the number of bytes, not the number
             *      of UTF characters.
             */

            inline size_type size()
                             const
            {
                return ((_pBuf) ? (_pBuf->_str.ulLength) : 0);
            }

            /*
             *@@ length:
             *      STL string compatibility function,
             *      same as BSStringBase::size().
             */

            inline size_type length()
                             const
            {
                return (size());
            }

            /*
             *@@ capacity:
             *      returns the amount of memory presently
             *      allocated for the string. This is at
             *      least what size() or length() return,
             *      but can be more.
             *
             *@@added V0.9.6 (2000-10-31) [umoeller]
             */

            inline size_type capacity()
                             const
            {
                return ((_pBuf) ? (_pBuf->_str.cbAllocated) : 0);
            }

            /*
             *@@ operator():
             *      returns 1 (TRUE) if the string
             *      contains something, 0 (FALSE)
             *      otherwise.
             *
             *      Note: as far as I know, this operator
             *      is NOT defined with the C++ string class.
             */

            inline int operator()()
                       const
            {
                return (size() != 0);
            }

            /*
             *@@ empty:
             *      returns 1 (TRUE) if the string
             *      contains nothing, 0 (FALSE) otherwise.
             */

            inline int empty()
                       const
            {
                return (size() == 0);
            }

            /********************************************
             *
             *  assignment, modification
             *
             ********************************************/

            void reserve(size_type stExtra = 0);

            void _take_from(XSTRING &str);

            size_type _printf(const char *pszFormatString, ...);

            void _itoa10(int value, char cThousands);

            BSStringBase& erase(size_type ulPos = 0, size_type n = npos);

            /********************************************
             *
             *  comparison
             *
             ********************************************/

        protected:

            int compare(const BSStringBase &s) const;

            int compare(const char *psz) const;

            /********************************************
             *
             *  find, replace
             *
             ********************************************/

            size_type _find_replace(BSStringBase &strFind,
                                    const BSStringBase &strReplaceWith,
                                    size_type *pulPos);

            size_type _find_replace(const char *pszFind,
                                    const BSStringBase &strReplaceWith,
                                    size_type *pulPos);

        public:

            size_type _find_replace(char cFind,
                                    char cReplace,
                                    size_type *pulPos);

            /********************************************
             *
             *  misc
             *
             ********************************************/

            /*
             *@@ swap:
             *      swaps the contents of this instance
             *      with that of another BSStringBase.
             *
             *      This is very fast because only the
             *      pointers are exchanged.
             *
             *@@added V0.9.6 (2000-10-31) [umoeller]
             */

            void swap(BSStringBase &str)
            {
                BSStringBuf *pBuf = str._pBuf;
                str._pBuf = _pBuf;
                _pBuf = pBuf;
            }
    };

    /*
     *@@ BSString:
     *      string class vaguely modelled after the C++ ANSI
     *      string class which IBM VAC 3.08 has no support for.
     *
     *      This implements the "BSString" class and typedefs "string"
     *      to "BSString". As a result, you can use either "BSString"
     *      or "string".
     *
     *      <B>Introduction</B>
     *
     *      If you're not familiar with the C++ "string" class, here's
     *      a short introduction.
     *
     *      Basically, the "string" class allows you to use strings
     *      as a basic type, like int, char, long, and so on, without
     *      having to worry about memory management.
     *
     *      Here is an example:
     +
     +          string str = "Hello."
     +
     +          if (str == "Hello")
     +          {
     +              string str = "We should say ";
     +              string str2 = str + "Hello now"
     *              str2 += '.';
     +
     +              printf("%s", str2.c_str());
     +          }
     *
     *      This shows some of the most convenient features of the
     *      string class:
     *
     *      -- You can just create an instance of it on the stack.
     *         The destructor will automatically free the memory
     *         that has been allocated for the string -- no more
     *         memory leaks as with the C functions.
     *
     *      -- You can compare strings using ==, !=, <, >. (This
     *         calls strcmp internally.)
     *
     *      -- You can concatenate strings using the "+" operator.
     *
     *      -- If you need a C-type string (e.g. for OS/2 API
     *         functions), use the BSString::c_str() method,
     *         which returns a "const char*" pointer to the string
     *         in the class.
     *
     *      In addition, the string class has methods for more
     *      complicated things, such BSString::find and
     *      BSString::replace.
     *
     *      Finally, this class is NOT thread-safe. If methods are
     *      operating on the same instance on several threads at
     *      the same time, this will DEFINITELY crash.
     *
     *      <B>C++ ANSI "string" class compatibility</B>
     *
     *      Unless marked otherwise, all methods and operators are
     *      compatible with the standard "string" class.
     *
     *      However, we only support single-byte (8-bit) character
     *      sets at this point, and only a subset of the "string"
     *      methods and operators. But since WarpIN uses UTF-8
     *      for encoding things, this is not really a problem.
     *
     *      Non-standard functions are marked with a leading
     *      underscore (e.g. BSString::_printf).
     *
     *      <B>Changes</B>
     *
     *      With V0.9.6, I have made improvements on memory management.
     *      Memory no longer gets reallocated with every change, but
     *      only if necessary.
     *
     *      With V0.9.18, I finally implemented buffer sharing as
     *      with the C++ string class. This allows several BSString
     *      instances to share the same memory buffer. See BSStringBuf.
     *
     *      In addition, V0.9.18 added Unicode support. See BSUString
     *      and BSUniCodec for more information.
     *
     *      All the V0.9.18 changes are backward-compatible. So even
     *      though the implementation has changed significantly,
     *      all old code should still compile.
     */

    class BSString : public BSStringBase
    {
        public:
            DECLARE_CLASS(BSString);

            /*
             *@@ BSString:
             *      default constructor to create an empty string.
             */

            BSString()
                : BSStringBase(tBSString)
            {
            }

            /*
             *@@ BSString:
             *      default copy constructor;
             *      required for exception handling!
             */

            BSString(const BSString &s)
                : BSStringBase(s, tBSString)
            {
            }

            BSString(const BSString &s,
                     size_type ulPos,
                     size_type n = npos);
            BSString(const char *psz);
            BSString(const char *p1, const char *p2);
            BSString(char c);

            BSString(BSUniCodec *pCodec,
                     const BSUString &ustr);

            /********************************************
             *
             *  property queries
             *
             ********************************************/

            char operator[](const size_type ul) const;

            /*
             *@@ c_str:
             *      returns the string member as a
             *      classic const C string. Note that
             *      this _never_ returns NULL. If the
             *      member string is empty, this
             *      returns a pointer to a null byte.
             */

            inline const char* c_str()
                               const
            {
                return ((_pBuf && _pBuf->_str.psz) ? _pBuf->_str.psz : "");
            }

            /********************************************
             *
             *  assignment, modification
             *
             ********************************************/

            BSString& assign(const BSString &s);
            BSString& assign(const BSString &s,
                             size_type ulPos,
                             size_type n = npos);
            BSString& assign(const char *psz);
            BSString& assign(const char *p1, const char *p2);
            BSString& assign(char c);

            BSString& assignUtf8(BSUniCodec *pCodec,
                                 const BSUString &ustr);

            /*
             *@@ operator=:
             *
             */

            inline BSString& operator=(const BSString &s)
            {
                return (assign(s));
            }

            /*
             *@@ operator=:
             *
             */

            inline BSString& operator=(const char *psz)
            {
                return (assign(psz));
            }

            /*
             *@@ operator=:
             *
             */

            inline BSString& operator=(char c)
            {
                return (assign(c));
            }

            BSString& append(const BSString &s);
            BSString& append(const char *psz);
            BSString& append(char c);

            BSString& appendUtf8(BSUniCodec *pCodec, const BSUString &s);

            /*
             *@@ operator+=:
             *      shortcut to append(BSStringBase &s).
             *
             *@@changed V0.9.2 (2000-03-29) [umoeller]: added return *this
             */

            inline BSString& operator+=(const BSString &s)     // in: string to append
            {
                return (append(s));
            }

            /*
             *@@ operator+=:
             *      shortcut to append(const char *psz).
             *
             *@@changed V0.9.2 (2000-03-29) [umoeller]: added return *this
             */

            inline BSString& operator+=(const char *psz)     // in: string to append
            {
                return (append(psz));
            }

            /*
             *@@ operator+=:
             *      shortcut to append(char c).
             *
             *@@changed V0.9.2 (2000-03-29) [umoeller]: added return *this
             */

            inline BSString& operator+=(char c)     // in: character to append
            {
                return (append(c));
            }

            /********************************************
             *
             *  comparison
             *
             ********************************************/

            /*
             *@@ compare:
             *
             */

            inline int compare(const BSString &s) const
            {
                return BSStringBase::compare(s);
            }

            /*
             *@@ compare:
             *
             */

            inline int compare(const char *psz) const
            {
                return BSStringBase::compare(psz);
            }

            int compare(size_type ulPos, size_type n, const BSString &s) const;

            /********************************************
             *
             *  find, replace
             *
             ********************************************/

            size_type find(const BSString &strFind, size_type ulPos = 0) const;
            size_type find(const char *pszFind, size_type ulPos = 0) const;
            size_type find(char c, size_type ulPos = 0) const;

            size_type rfind(char c, size_type ulPos = npos) const;

            size_type find_first_of(char c, size_type ulPos = 0) const;
            size_type find_first_of(const char *achChars, size_type ulPos = 0) const;

            size_type find_first_not_of(char c, size_type ulPos = 0) const;
            size_type find_first_not_of(const char *achChars, size_type ulPos = 0) const;

            BSString& replace(size_type ulPosThis,
                              size_type nThis,
                              const BSString &strReplace);

            /* BSString& replace(size_type ulPosThis,
                              size_type nThis,
                              const BSString &strReplace,
                              size_type ulPosReplace = 0,
                              size_type nReplace = npos);

            BSString& replace(size_type ulPosThis,
                              size_type nThis,
                              const char *pszReplace,
                              size_type nReplace);

            BSString& replace(size_type ulPosThis,
                              size_type nThis,
                              const char *pszReplace); */

            size_type _find_word(BSString &strFind,
                                 size_type ulPos = 0,
                                 const char *pcszBeginChars = "\x0d\x0a ()/\\-,.;*",
                                 const char *pcszEndChars   = "\x0d\x0a ()/\\-,.:;*&'")
                      const;

            size_type _find_word(const char *pszFind,
                                 size_type ulPos = 0,
                                 const char *pcszBeginChars = "\x0d\x0a ()/\\-,.;*",
                                 const char *pcszEndChars   = "\x0d\x0a ()/\\-,.:;*&'")
                      const;

            /*
             *@@ _find_replace:
             *
             */

            inline size_type _find_replace(BSString &strFind,
                                           const BSString &strReplaceWith,
                                           size_type *pulPos)
            {
                return BSStringBase::_find_replace(strFind, strReplaceWith, pulPos);
            }

            /*
             *@@ _find_replace:
             *
             */

            inline size_type _find_replace(const char *pszFind,
                                           const BSString &strReplaceWith,
                                           size_type *pulPos)
            {
                return BSStringBase::_find_replace(pszFind, strReplaceWith, pulPos);
            }

            /*
             *@@ _find_replace:
             *
             */

            inline size_type _find_replace(char cFind,
                                           char cReplace,
                                           size_type *pulPos)
            {
                return BSStringBase::_find_replace(cFind, cReplace, pulPos);
            }

            BSString substr(size_type ulPos = 0,
                                size_type n = npos)
                     const;

            int _extract_word(unsigned long ulIndex,
                              BSString &strTarget,
                              size_type ulPos = 0,
                              const char *pcszBeginChars = "\x0d\x0a ()/\\-,.;*",
                              const char *pcszEndChars   = "\x0d\x0a ()/\\-,.:;*&'")
                const;

            void _format();
    };

    int operator==(const BSString &s1, const BSString &s2);

    int operator==(const char *psz1, const BSString &s2);

    int operator==(const BSString &s1, const char *psz2);

    int operator!=(const BSString &s1, const BSString &s2);

    int operator!=(const char *psz1, const BSString &s2);

    int operator!=(const BSString &s1, const char *psz2);

    int operator<(const BSString &s1, const BSString &s2);

    BSString operator+(const BSString &s1, const BSString &s2);

    BSString operator+(const char *psz1, const BSString &s2);

    BSString operator+(const BSString &s1, const char *psz2);

    /*
     *@@ string:
     *      typedef for using BSStrings like the
     *      standard string class.
     */

    typedef BSString string;

    /*
     *@@ BSUString:
     *      unmodified BSString subclass for marking
     *      strings that are not plain ASCII, but UTF-8
     *      encoded. This is used in some declarations
     *      to make sure that interfaces can react
     *      properly.
     *
     *      Most notably, this has restrictions on the
     *      assigment and conversion methods. In order
     *      to not accidentally assign codepage strings
     *      to a BSUString, regular assignments from
     *      const char * to BSUStrings are not allowed.
     *      Use BSUstring::assignUtf8 for this.
     *
     *      In detail, the following things do not work:
     *
     *      --  extracting substrings, because with UTF-8
     *          you can never be sure if an offset points
     *          into the middle of a multi-byte character;
     *
     *      --  c_str() does not exist to avoid accidentally
     *          passing the member buffer to legacy APIs.
     *          Use BSUString::GetBuffer() instead.
     *
     *      --  "=" assignments, "==" comparisons and "+"
     *          concatenation between BSString and BSUString.
     *          See BSUniCodec on how to convert BSString's
     *          to BSUString's and back.
     *
     *      However, "==" between two BSUString's works.
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    class BSUString : public BSStringBase
    {
        public:
            DECLARE_CLASS(BSUString);

            /*
             *@@ BSUString:
             *      default constructor to create an empty string.
             */

            BSUString()
                : BSStringBase(tBSUString)
            {
            }

            /*
             *@@ BSUString:
             *      default copy constructor;
             *      required for exception handling!
             */

            BSUString(const BSUString &s)
                : BSStringBase(s, tBSUString)
            {
            }

            BSUString(BSUniCodec *pCodec,
                      const BSString &str);

            /********************************************
             *
             *  property queries
             *
             ********************************************/

            char operator[](const size_type ul) const;

            /*
             *@@ GetBuffer:
             *      like BSString::c_str(), but with
             *      a different name to make sure
             *      ustrings don't get passed to
             *      code which cannot handle that.
             */

            inline const char* GetBuffer()
                               const
            {
                return ((_pBuf && _pBuf->_str.psz) ? _pBuf->_str.psz : "");
            }

            /********************************************
             *
             *  assignment, modification
             *
             ********************************************/

            BSUString& assign(const BSUString &s);
            BSUString& assignUtf8(const char *psz);
            BSUString& assignUtf8(const char *p1, const char *p2);
            BSUString& assignCP(BSUniCodec *pCodec,
                                const BSString &str);
            BSUString& assignCP(BSUniCodec *pCodec,
                                const char *psz);

            /*
             *@@ operator=:
             *
             */

            inline BSUString& operator=(const BSUString &s)
            {
                return (assign(s));
            }

            BSUString& append(const BSUString &s);
            BSUString& appendUtf8(const char *psz);
            BSUString& appendCP(BSUniCodec *pCodec,
                                const BSString &str);

            /*
             *@@ operator+=:
             *      shortcut to append(BSUString &s).
             *
             */

            inline BSUString& operator+=(const BSUString &s)     // in: string to append
            {
                return (append(s));
            }

            /********************************************
             *
             *  comparison
             *
             ********************************************/

            inline int compare(const BSUString &s) const
            {
                return BSStringBase::compare(s);
            }

            inline int compareUtf8(const char *psz) const
            {
                return BSStringBase::compare(psz);
            }

            int compareI(const BSUString &s) const;

            int compareUtf8I(const char *psz) const;

            /********************************************
             *
             *  find, replace
             *
             ********************************************/

            /*
             *@@ _find_replace:
             *
             */

            inline size_type _find_replace(BSUString &strFind,
                                           const BSUString &strReplaceWith,
                                           size_type *pulPos)
            {
                return BSStringBase::_find_replace(strFind, strReplaceWith, pulPos);
            }

            /*
             *@@ _find_replace:
             *
             */

            inline size_type _find_replace(const char *pszFind,
                                           const BSUString &strReplaceWith,
                                           size_type *pulPos)
            {
                return BSStringBase::_find_replace(pszFind, strReplaceWith, pulPos);
            }

            /*
             *@@ _find_replace:
             *
             */

            inline size_type _find_replace(char cFind,
                                           char cReplace,
                                           size_type *pulPos)
            {
                return BSStringBase::_find_replace(cFind, cReplace, pulPos);
            }
    };

    int operator==(const BSUString &s1, const BSUString &s2);

    int operator!=(const BSUString &s1, const BSUString &s2);

    BSUString operator+(const BSUString &s1, const BSUString &s2);

    /*
     *@@ ustring:
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    typedef BSUString ustring;

#endif

