
/*
 *@@sourcefile bs_tokenizer.cpp:
 *      implements the BSTokenizer class
 *
 *@@header "helpers\xstring.h"
 *@@header "cppbase\bs_tokenizer.h"
 */

/*
 *      This file Copyright (C) 20011-2015 Paul Ratcliffe.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#include <os2.h>

#include "setup.h"

#include "helpers\xstring.h"

// base includes
#include "cppbase\bs_tokenizer.h"

#pragma hdrstop

DEFINE_CLASS(BSTokenizer, BSRoot);

BSString &BSTokenizer::Dequote(BSString &str)
{
    size_t len = str.length();

    if (len && (str[0] == '\'' || str[0] == '"'))
    {
        if (str[len - 1] == str[0])
            str.erase(--len);

        if (len)
            str.erase(0, 1);
    }

    return str;
}


BSTokenizer::BSTokenizer()
    : BSRoot(tBSTokenizer),
    _size(0)
{
    _pTokens = new list<BSString *>(STORE);
}


BSTokenizer::BSTokenizer(const BSString &str, const BSString &delimiters, BOOL unquote)
    : BSRoot(tBSTokenizer),
    _size(0)
{
    _pTokens = new list<BSString *>(STORE);
    Tokenize(str, delimiters, unquote);
}


BSTokenizer::~BSTokenizer()
{
    delete _pTokens;
}


size_t BSTokenizer::Tokenize(const BSString &str, const BSString &delimiters, BOOL unquote)
{
    size_t firstPos, lastPos = 0, quotePos;
    BSString token;

    _pTokens->clear();
    _size = 0;
    for(;;)
    {
        firstPos = str.find_first_not_of(delimiters.c_str(), lastPos);
        if (firstPos == BSString::npos)
            break;

        lastPos = str.find_first_of(delimiters.c_str(), firstPos);
        if (lastPos != BSString::npos)
            for (quotePos = firstPos;;)
            {
                quotePos = str.find_first_of("'\"", quotePos);
                if (quotePos < lastPos)
                {
                    quotePos = str.find_first_of(str.c_str()[quotePos], quotePos + 1);
                    if (quotePos != BSString::npos)
                        if (quotePos > lastPos)
                            lastPos = str.find_first_of(delimiters.c_str(), ++quotePos);
                        else
                            ++quotePos;
                    else
                    {
                        lastPos = quotePos;
                        break;
                    }
                }
                else
                    break;
            }

        token = str.substr(firstPos, lastPos - firstPos);
        if (unquote)
            Dequote(token);

        _pTokens->push_back(new BSString(token));
        ++_size;
    }

    return _size;
}


const char *BSTokenizer::GetString(size_t num) const
{
    const char *p = "";

    if (num < Size())
    {
        list<BSString *>::iterator li = _pTokens->begin();

        for (size_t i = 0; i < num; ++i, ++li);
        p = (*li)->c_str();
    }

    return p;
}

