/****************************************************************
 * Allocate input grammar variables for bison,
 * Copyright (C) 1984, 1986, 1989 Free Software Foundation, Inc.
 *
 * This file is part of Bison, the GNU Compiler Compiler.
 *
 * Bison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Bison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bison; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ****************************************************************/

/*!
 *  @file gram.c
 *  @brief Data definitions for Bison's internal grammar representation.
 *
 *  Holds the storage for the global variables described in gram.h:
 *  the symbol, rule and item tables that the rest of the generator
 *  reads after the input grammar has been packed.
 *
 *  @copyright Copyright (C) 1984, 1986, 1989 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


/* comments for these variables are in gram.h  */

int nitems;             /*!< Number of items in the ritem table. */
int nrules;             /*!< Number of rules in the grammar. */
int nsyms;              /*!< Total number of symbols (tokens + nonterminals). */
int ntokens;            /*!< Number of tokens. */
int nvars;              /*!< Number of variables (nonterminals). */

short *ritem;           /*!< Right-hand sides of all rules, packed into one array. */
short *rlhs;            /*!< Left-hand side symbol of each rule. */
short *rrhs;            /*!< Index into ritem of the start of each rule's RHS. */
short *rprec;           /*!< Precedence level of each rule. */
short *rprecsym;        /*!< Symbol given via %prec for each rule, or 0. */
short *sprec;           /*!< Precedence level of each symbol. */
short *rassoc;          /*!< Associativity of each rule. */
short *sassoc;          /*!< Associativity of each symbol. */
short *token_translations; /*!< Maps user token numbers to internal numbers. */
short *rline;           /*!< Source line number of each rule. */

int start_symbol;       /*!< Internal symbol number of the start symbol. */

int translations;       /*!< Non-zero if the token translation table is used. */

int max_user_token_number; /*!< Largest token number supplied by the user. */

int semantic_parser;    /*!< Non-zero if the hairy parser should be used. */

int pure_parser;        /*!< Non-zero if a pure (reentrant) parser is generated. */

int error_token_number; /*!< Internal token number of the error token. */

/*!
 *  @brief Dummy definition.
 *
 *  This is to avoid linker problems which occur on VMS when using
 *  GCC, when the file in question contains data definitions only.
 */
void
dummy (void)
{
}
