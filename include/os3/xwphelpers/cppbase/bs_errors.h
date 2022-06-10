
/*
 * bs_errors.h:
 *      this declares a number of C++ exception classes which
 *      are used throughout WarpIN for a more flexible error
 *      handling. Some of the more complex constructors are
 *      implemented in warpin.cpp.
 *
 *      All exception classes are derived from BSExcptBase, so
 *      you can always catch BSExcptBase too. main() in warpin.cpp
 *      catches those exceptions, so if you don't, main() will.
 *
 *@@include #include "base\bs_string.h"
 */

/*
 *      This file Copyright (C) 1999-2002 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef WARPIN_ERRORS_HEADER_INCLUDED
    #define WARPIN_ERRORS_HEADER_INCLUDED

    /*
     *@@ BSExcptBase:
     *      exception base class, from which all other exception
     *      classes are derived.
     *
     *@@changed V0.9.18 (2002-03-08) [umoeller]: now using ustring
     */

    class BSExcptBase
    {
        public:
            ustring  _ustrDescription;        // UTF-8!

        protected:
            BSExcptBase() {  };

        public:
            BSExcptBase(const char *pcsz)
            {
                _ustrDescription.assignUtf8(pcsz);
            };
    };

    /*
     *@@ BSCancelExcpt:
     *      dummy exception class which can be thrown by anything,
     *      and WarpIN will terminate itself.
     *      This is caught only by main(), so this is a quick way
     *      to exit WarpIN from anywhere.
     *
     *@@changed V0.9.0 (99-11-06) [umoeller]: renamed from CancelExcpt
     */

    class BSCancelExcpt : public BSExcptBase
    {
    };

    /*
     *@@ BSUnsupportedCPExcpt:
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    class BSUnsupportedCPExcpt : public BSExcptBase
    {
        public:
            unsigned short _usCodepage;

            BSUnsupportedCPExcpt(unsigned short usCodepage)
                : _usCodepage(usCodepage)
            {
                _ustrDescription._printf("Unsupported codepage %u", usCodepage);
            }
    };

#endif

