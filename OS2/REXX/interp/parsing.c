/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This implementation of the REXX parse template is quite near to
 * what Cowlishaw specified in his book, but there is one very
 * important difference. Cowlishaw sees a template as patterns and
 * variables, with patterns in each end (and some of the patterns
 * and/or the variables might be empty).
 *
 * The concept here is any sequence of variables and patterns, and
 * the system is parsed into two levels and interpreted as such.
 * First there is the level of patterns, which is anchored to
 * particular locations in the string to be parsed.
 *
 * When the anchor-points are determined, the variables are filled
 * in with the parts of the string that comes between each anchor.
 * Here is an example:
 *
 *    parse value 'That is a parse string!' with a b 'is' c d 'i' e
 *
 * In this example, there are four anchors in the string, the two
 * patterns 'is' and 'i', and the two implicit anchors start-of-string
 * and end-of-string. They anchor particular locations in the string
 * to the lists of variables: (a b), (c d) and (e). At the first level
 * the anchors are found and the text between them are deternmined
 * (SOS=Start-Of-String, EOS=End-Of-String):
 *
 *                            That is a parse string!
 *    anchors            (SOS)-----is------------i---(EOS)
 *    strings in-between      <a b>  <<<<c d>>>>> <e>
 *
 * Now, we have a set of substrings, and to each substring, there is
 * possibly empty set of variable which are to receive its value from
 * the contents of that substring. Doing that, we get:
 *
 *    (a b) = 'That '        ==> a='That'   b=' '
 *    (c d) = ' a parse str' ==> c='a'      c=' parse str'
 *    (e)   = 'ng!'          ==> e='ng!'
 *
 * To some extent, one might say that there are one anchor between
 * each variable, since these match a sequence of blank characters.
 * However, these have less priority than the explicit patterns.
 *
 * This file makes available three functions:
 *
 *   doparse()      parses a single string into a single template
 *   parseargtree() parses a set of strings into a set of templates
 *   bmstrstr()     implementation of Boyer-Moore string search
 */

#include "rexx.h"
#include <string.h>
#include <stdio.h>  /* Stupid SunOS acc uses stderr in assert(), yuk! */
#include <assert.h>
#include "strings.h"

/*
 * Using the 'tracestat' field of 'currlevel' in order to determine
 * the current tracing mode.
 */

/*
 * This file-scope variable 'traceparse' is use to cache the value of the
 * setting of the current trace mode. It is initialized at the entrypoint of
 * each parse statement. However, if functionality is expanded, so that
 * expression can occur inside a template, then this variable must be
 * checked in order to ensure that it is up-to-date after the expression
 * has been evaluated.
 */

/*
 * We have to seek for the nullstring rather often, so we make it a
 * file-scope variable, in order to optimize a bit. This way, we have
 * to neither allocate space for it, or deallocate it.
 */
static const streng nullstring={ 0, 0, "" } ;

/*
 * This is an implementation of the strstr() function, for the 'streng'
 * datatype. The algorithm is a straight forward implementation of the
 * Boyer-Moore string search algorithm.
 *
 * The input parameters are first the string to search in, then the start
 * position of the search in that string, and then the string to search
 * for. If the whole of the input string is to be searched, then the
 * start position is set to zero. If start position is set to one, then
 * a match will not be found if it includes the first character of the
 * the string 'heystack' as the first character of the match.
 *
 * The returned value is an integer, which will be the index of the
 * first character of 'heystack->value' that match. If no match is
 * found, -1 is returned, if a match is found in the start of 'heystack'
 * then 0 is returned. This means that both the 'start' parameter and
 * the return value are zero-based.
 */
int bmstrstr( const streng *heystack, int Offset, const streng *needle,
              int caseless )
{
   const unsigned char *TmpPtr, *TmpPtr2;
   int NeedLen, HeyLen;
   const unsigned char *NeedPtr, *HeyPtr, *HeyBase;
   unsigned int NextChr[256];
   int Tmp;
   const unsigned char *eptr;

   NeedPtr = (const unsigned char *) needle->value;
   NeedLen = needle->len;

   HeyBase = (const unsigned char *) heystack->value;
   HeyLen = heystack->len - Offset;
   HeyPtr = HeyBase + Offset;

   /*
    * Check for a fast break-out first.
    */
   if ( HeyLen < NeedLen )
      return -1;

   /*
    * Next, sometimes we want to search for one character only. Although
    * Boyer-Moore works for that case, it is hardly efficient. So, if the
    * search pattern has length one, we use the ANSI C memchr() to find
    * the string. That function is likely to to be more efficient, maybe
    * even written in hand-optimized assembly.
    */
   if (NeedLen==1)
   {
#ifdef __CHECKER__
      /* Stupid Checker 0.9.9.1, FGC */
      if (HeyLen == 0)
         return -1;
#endif
      if ( caseless )
      {
         /*
          * We assume that two memchrs are much faster than several toupper
          * or tolower. This is not true always, but the opposite is even
          * wrong.
          */
         TmpPtr = (const unsigned char *)memchr( HeyPtr, rx_toupper( *NeedPtr ), HeyLen );
         TmpPtr2 = (const unsigned char *)memchr( HeyPtr, rx_tolower( *NeedPtr ), HeyLen );
         if ( TmpPtr == NULL )
            TmpPtr = TmpPtr2;
         else if ( ( TmpPtr2 != NULL ) && ( TmpPtr2 < TmpPtr ) )
            TmpPtr = TmpPtr2;
      }
      else
         TmpPtr = (const unsigned char *)memchr( HeyPtr, *NeedPtr, HeyLen );
      if ( TmpPtr )
         return TmpPtr - HeyBase;
      else
         return -1;
   }

   /*
    * OK, here is the 'real' search. Basically, it consists of two
    * phases: first a table (next) is built up, and then the string
    * is searched using the table to decide how far to hop if a match
    * was not found.
    *
    * Let's recount some of the theory behind Boyer-Moore search. If you
    * got a pattern, P, and string to search in, S, then the first match
    * may be S[1..len(P)]. To verify whether that is actually a match,
    * we have to iterate through all the characters, i.e. over len(P).
    *
    * If that is a match, then return the correct position, else we must
    * continue. Here comes the 'tricky' part. Suppose P is 'abcccd' and
    * that S is 'gddeebfgghhhiiijjj'. Now we check character S[len(P)]
    * and depending on its value we can determine something about the
    * possibiltiy of having a match starting somewhere in the area from
    * S[1] to S[len(P)]. Clearly, if S[len(P)] is a character not in P,
    * then no match can occur in this area. Else, determine the first,
    * of the next possible match, and move forward to check that.
    *
    *
    * First, we have to set up the table to use during the search.
    * Initially, we fill the whole table with the 'default' value, and
    * then we patch the values which should have other values in the
    * loop following the call to memset().
    */
   for ( Tmp = 0; Tmp < 256; Tmp++ )
      NextChr[Tmp] = NeedLen;

   eptr = HeyPtr + HeyLen - NeedLen;
   NeedLen--;

   TmpPtr = NeedPtr;

   if ( caseless )
   {
      for (Tmp = NeedLen; Tmp >= 0; Tmp--, TmpPtr++ )
         NextChr[rx_tolower(*TmpPtr)] = Tmp;

      while ( HeyPtr <= eptr )
      {
         Tmp = NextChr[rx_tolower(HeyPtr[NeedLen])];
         if ( !Tmp )
         {
            /*
             * A hit here doesn't mean that we actually have the match. It is a
             * possible match. We still have to compare the whole string.
             * Remember: Boyer-Moore reduces the lookups for a proper start of
             * the string, not the string comparisons itself.
             *
             * The last character matches, so compare the start.
             * NeedLen = Str_len( needle ) - 1;
             */
            if ( mem_cmpic( HeyPtr, NeedPtr, NeedLen ) == 0 )
               return HeyPtr - HeyBase;
            HeyPtr++;
         }
         else
            HeyPtr += Tmp;
      }
   }
   else
   {
      /*
       * For comments see above.
       */
      for (Tmp = NeedLen; Tmp >= 0; Tmp--, TmpPtr++ )
         NextChr[*TmpPtr] = Tmp;

      while ( HeyPtr <= eptr )
      {
         Tmp = NextChr[HeyPtr[NeedLen]];
         if ( !Tmp )
         {
            if ( memcmp( HeyPtr, NeedPtr, NeedLen ) == 0 )
               return HeyPtr - HeyBase;
            HeyPtr++;
         }
         else
            HeyPtr += Tmp;
      }
   }

   return -1;
}


static const streng *handle_var( tsd_t *TSD, nodeptr thisptr )
{
   if (thisptr->type == X_HEAD_SYMBOL)
      return fix_compound( TSD, thisptr, NULL ) ;
   else
      return shortcut( TSD, thisptr ) ;
}

/*
 * This parses a part of the source string, determined by (start)
 * and (len) into the variables of the (thisptr) tree. Only variables
 * are handled, not patterns. It will be called by doparse() to fit a
 * string into a set of variables and placeholder.
 *
 * Start points to the first character to be parsed, while len gives the
 * length of the string. len MUST be >= 0, which is currently detected by
 * doparse.
 *
 * There is no returnvalue from this function, it is only called to
 * achieve the special effects of setting variables and tracing
 * variables and placeholders. Actually, this routine used to be
 * tailrecursive, but I changed it into a goto.
 *
 * The variables and placeholders to parse ares stored in a chained
 * of pointers, chased through p[0]. 'start' is a ptr to the first
 * characters to be parsed by this function. 'len' gives the length
 * of the string to be parsed by this function.
 */
static void doparse3( tsd_t *TSD, cnodeptr thisptr, const char *start, int len )
{
   int wordlen ;
   streng *tptr ;
   int CutLast = 0; /* see below */

   recurse:
   assert(len >= 0);
   /*
    * Since we are going to put the next word of the input string
    * into a variable, we must first find that value. The if tests
    * whether we are the last word before an 'anchor' to be parsed.
    * if so, use the rest of the string. If not, scan forwards to
    * identify a word.
    */
   if (thisptr->p[0])
   {
      /*
       * We shall only fetch out one word. First skip leading spaces,
       * then find the end of the next word.
       */
      while (len && rx_isspace(*start))
      {
         start++;
         len--;
      }

      wordlen = 0;
      while ((wordlen < len) && !rx_isspace(start[wordlen]))
         wordlen++;
   }
   else
   {
      /*
       * We are last word, use rest of string as value.
       * FGC: This is NOT true. Accoring to ANSI standard, we have to
       *      cut the first char if it is a space AND it is not
       *      the only pattern to match.
       */
      if (CutLast && len && rx_isspace(*start))
      {
         start++;
         len--;
      }
      wordlen = len;
   }
   CutLast = 1;

   /*
    * We have found the word to be parsed into something. Now we have
    * to decide what to parse it into. There are two possibilities.
    * It might be a variable, or just a placeholder (dot). The two are
    * handled in each part of the if-statement below. The setting of
    * 'tptr' could be lifted out of the if-statement to save space,
    * but at the cost of MUCH more CPU.
    * DON'T DO IT!
    */
   if ( thisptr->type == X_TPL_SYMBOL )
   {
      tptr = Str_ncreTSD( start, wordlen );
      if ( TSD->traceparse )
         tracevalue( TSD, tptr, '>' );

      if ( thisptr->p[1]->type == X_HEAD_SYMBOL )
         fix_compound( TSD, thisptr->p[1], tptr );
      else
         setshortcut( TSD, thisptr->p[1], tptr );
   }
   else
   {
      /*
       * It's a placeholder, actually, we skip this if we arn't
       * tracing. No harm is done if tracevalue() is called unnecessary,
       * but this way we save three function calls whenever we're not
       * doing TRACE INT.
       *
       * The three operations below do: 1) get the value to be traced,
       * 2) then output the trace information, 3) and then free the
       * temporary storage.
       */

      if ( TSD->traceparse )
      {
         tptr = Str_ncreTSD( start, wordlen );
         tracevalue( TSD, tptr, '.' );
         Free_stringTSD( tptr );
      }
   }

   /*
    * Now, this should actually be a tail recursion, but since be don't
    * trust compilers, we are optimizeing it ourselves.
    */
    if ((thisptr = thisptr->p[0]) != NULL)
    {
       start += wordlen ;
       len -= wordlen ;
       goto recurse ;
    }
}

/*
 * This routine parses a string (source) into the template that is
 * specified by the structure in the (thisptr) tree. It handles find the next
 * template, and handles the aread between two templates.
 *
 * It calls it self recursively to handle a sequence of templates.
 * Well, actually, it used to call it self recuseively, but the
 * tail recursion has been removed to improve efficiency.
 *
 * A sequence of patterns must be chained together using p[1], while
 * each pattern can contain the vars/placeholders as a chain linked
 * in at p[0].
 *
 * 'source' is the string to be parsed, 'thisptr' it a ptr to the top
 * of the parsetree that describes how 'source' after start'th
 * position is to be parsed. 'start' is a ptr to the first char in
 * 'source' to be parsed by this part of the template.
 */

void doparse( tsd_t *TSD, const streng *source, cnodeptr thisptr, int caseless )
{
   int start=0,point=0,length=0, end=0, nextstart=0, solid=0 ;
   const streng *pattern=NULL ;
   const streng *xtmp=NULL ;
   char tch=' ' ;

   nextstart = 0 ;  /* too keep gcc from complaining about uninitialized */
   tch = TSD->currlevel->tracestat ;
   TSD->traceparse = ((tch=='I') || (tch=='R')) ;

recurse:
   /*
    * Cache the length of source, to avoid chasing ponters later.
    * Then make pattern default to the nullstring. The nullstring is
    * so muched used, that we don't want to allocate and deallocate
    * that all the time.
    */
   length = source->len ;
   pattern = &nullstring ;

   /*
    * There are two main cases, either this is the last pattern, in
    * which case we use the rest of the string. Or either there is
    * another pattern further out, in which case we have to find it.
    *
    */
   if (thisptr->p[1])
   {
      /*
       * We are not the last pattern, so first find the next pattern.
       * First cache the type, so we don't chase pointers. There are
       * two main choises: either seek for a string of some sort, or
       * use an offset of some sort.
       */
      solid = thisptr->p[1]->type ;
      if ((solid==X_TPL_MVE)||(solid==X_TPL_VAR))
      {
         /*
          * The pattern to search for is either a literal string, or it
          * is the value hold in a variable, set pattern to whatever
          * it might be. Pattern previous points to a statically
          * allocated variable, so don't bother to deallocate.
          */
         if (solid==X_TPL_MVE)
            pattern = thisptr->p[1]->name ;
         else
            pattern = handle_var( TSD, thisptr->p[1]->p[0] ) ;
         /*
          * Then we must find where in the source string pattern occurs.
          * If it don't occur there, we use the rest of the string, else
          * we use the string up to, but not including, the first char
          * that matched pattern. The 'bmstrstr' returns -1 for not
          * found, so correct that to rest-of-string. Also note that if
          * the pattern is the nullstring, it should match the end of
          * the string.
          */
         if (Str_len(pattern))
         {
            end = bmstrstr( source, start, pattern, caseless ) ;
            if (end<0)
            {
               point = end = length ;
               nextstart = end ;
            }
            else
            {
               nextstart = end + Str_len(pattern) ;
               point = end ;
            }
         }
         else
         {
            nextstart = point = end = length ;
         }

         /*
          * While 'end' marks how much to stuff into variables, nextstart
          * marks where to start the search for the next pattern (if
          * any). Remember that patterns "eat" up the part of the
          * parse string that they match.
          */
/*       nextstart = end + Str_len(pattern) ; */
      }
      else
      {
         /*
          * The next pattern to match is not a string to match, but a
          * positional movement, which will always be numeric, and if
          * it contains a sign, that should have been stripped off during
          * parsing. But a variable may be negative, too.
          */
         if (thisptr->p[1]->name)
            xtmp = thisptr->p[1]->name ;
         else
            xtmp = handle_var( TSD, thisptr->p[1]->p[0] ) ;

         end = streng_to_int( TSD, xtmp, &nextstart ) ;
         if (nextstart)
            exiterror( ERR_INVALID_INTEGER, 4, tmpstr_of( TSD, xtmp ) );

         /*
          * Depending on what sort of positional movement, do the right
          * thing.
          */
         if (solid==X_NEG_OFFS)
         {
            /*
             * If it is a movement backwards, the concept goes something
             * like move-it-foreward-to-a-backwards-position. That is,
             * the string to be parsed continues forwards to the end of
             * the sting and stops there, while the nextposition wraps
             * round to the start again.
             *
             * Anyway, parse all the rest of the sting in this parse, and
             * start on the specified position for the next parse.
             */
            start = point ;
            nextstart = point - end ;
            end = length ;
            if (nextstart > length)
               nextstart = length;
            if (nextstart < 0)
               nextstart = 0;

            point = nextstart ;
         }

         else if (solid==X_POS_OFFS)
         {
            /*
             * If the movement is forward, it is simpler, just move the
             * position of both the end of thisptr, and the start of next
             * to the right point.
             */
            start = point ;
            nextstart = point + end ;
            if (nextstart > length)
               nextstart = length;
            if (nextstart < 0)
               nextstart = 0;
            end = nextstart ;
            if (end<=start)
               end = length ;

            point = nextstart ;
         }

         else if (solid==X_ABS_OFFS)
         {
            /*
             * Same applies if the position is absolute, just move it.
             */
            end--;
            if (end > length)
               end = length;
            if (end < 0)        /* fixes bug 1107757 */
               end = 0;

            point = nextstart = end;
            if (end <= start)
               end = length;
         }
      }
   }
   else
      /*
       * We are last pattern to match, set the end of the string to
       * be parsed to the rest of the string available.
       */
      end = nextstart = length ;

   /*
    * Make sure that we didn't do anything illegal when we pushed
    * around on the value of end and nextstart. These should have been
    * set correctly in the statements above.
    */
   assert((0<=nextstart) && (nextstart<=length)) ;

   /*
    * Then handle end. It must be _after_ the last character in
    * the pattern, while it must not be larger than length.
    */
   assert((start <= end) && (end <= length)) ;

   /*
    * Now we have marked off an area to be parsed, so call 'doparse3' to
    * put values into the variables. Note that end is decremented,
    * since doparse3 expects ptr to last char to use, not ptr to char
    * after last char to use.
    */
   if (thisptr->p[0])
   {
      doparse3( TSD, thisptr->p[0], source->value+start, end-start);
      --end;
   }

   /*
    * Then make a tailrecursive call, or rather, simulate one. This
    * operation will take care of the next set of variables to be
    * parsed values into.
    */
   if ((thisptr=thisptr->p[2]) != NULL)
   {
      start = nextstart ;
      goto recurse ;
   }

}



/*
 * A single parse clause can parse multiple strings into multiple
 * patterns. Normally, this is only done using 'parse arg'. The
 * following piece of code parses multiples strings in an structure
 * of arguments into the matching pattern.
 *
 * There are no limits on the number of arguments to be parsed,
 * other than memory.
 */
void parseargtree( tsd_t *TSD, cparamboxptr argbox, cnodeptr thisptr, int flags )
{
   const streng *source ;
   streng *upplow ;

   /*
    * All templates in a list of template are connected though the
    * next field of the template.
    */
   for (; thisptr; thisptr=thisptr->next)
   {
      assert(thisptr->type==X_TPL_SOLID) ;

      /*
       * Else, it is a tempate into which a string is to be parsed.
       * That string is an argument, to first get that argument,
       * if it exist, else use the nullstring. Never bother about
       * deallocating thisptr string; either it is part of an arguemnt
       * which is deallocated somewhere else, or it is the statically
       * allocated nullstring.
       */
      if ((argbox)&&(argbox->value))
         source = argbox->value ;
      else
         source = &nullstring ;

      if (flags & PARSE_UPPER)
      {
         upplow = Str_upper( Str_dupTSD( source )) ;
         doparse( TSD, upplow, thisptr, flags & PARSE_CASELESS ) ;
         Free_stringTSD( upplow ) ;
      }
      else if (flags & PARSE_LOWER)
      {
         upplow = Str_lower( Str_dupTSD( source )) ;
         doparse( TSD, upplow, thisptr, flags & PARSE_CASELESS ) ;
         Free_stringTSD( upplow ) ;
      }
      else
         doparse( TSD, source, thisptr, flags & PARSE_CASELESS ) ;

      if (argbox)
         argbox = argbox->next ;
   }
}
