
/*
 * bs_tokenizer.h:
 *      header file for the BSTokenizer class.
 *
 *@@added V1.0.21 (2015-01-29) [pr]
 *@@include #include "cppbase\bs_base.h"
 *@@include #include "cppbase\bs_string.h"
 *@@include #include "cppbase\bs_list.h"
 */

/*
 *      This file Copyright (C) 2011-2015 Paul Ratcliffe.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef WARPIN_TOKENIZER_HEADER_INCLUDED
    #define WARPIN_TOKENIZER_HEADER_INCLUDED

    #include "cppbase\bs_base.h"
    #include "cppbase\bs_string.h"
    #include "cppbase\bs_list.h"

    /*
     *@@ BSTokenizer:
     *
     * Really wanted a vector here, but have to mis-use a list instead as that's
     * all we have.
     *
     */

    class BSTokenizer : public BSRoot
    {
        private:
            list<BSString *> *_pTokens;
            size_t _size;

            BSTokenizer(const BSTokenizer &);
            BSTokenizer &operator=(const BSTokenizer &);

        public:
            DECLARE_CLASS(BSTokenizer);

            static BSString &Dequote(BSString &str);

            BSTokenizer();
            BSTokenizer(const BSString &str, const BSString &delimiters = " ", BOOL unquote = TRUE);
            virtual ~BSTokenizer();

            size_t Tokenize(const BSString &str, const BSString &delimiters = " ", BOOL unquote = TRUE);
            inline size_t Size(void) const { return _size; }
            inline list<BSString *>::iterator begin() const { return _pTokens->begin(); }
            inline const list<BSString *>::iterator end() const { return (const list<BSString *>::iterator) NULL; }
            const char *GetString(size_t num) const;
    };

#endif

