/*!
 * @file lb.h
 * @brief Line breaking.
 *
 * Copyright (c) 1993-1995 Eberhard Mattes
 *
 * This file is part of emxdoc.
 *
 * emxdoc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * emxdoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with emxdoc; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/*!
 * @brief Out of memory error code.
 */
#define LB_NOMEM        1
/*!
 * @brief Invalid argument error code.
 */
#define LB_INVAL        2
/*!
 * @brief Internal error code.
 */
#define LB_INTERN       3

/*!
 * @brief Word element type.
 */
#define LBN_WORD        1
/*!
 * @brief Pre-hyphen fragment element type.
 */
#define LBN_PRE         2
/*!
 * @brief Post-hyphen fragment element type.
 */
#define LBN_POST        3
/*!
 * @brief Glue element type.
 */
#define LBN_GLUE        4
/*!
 * @brief Newline element type.
 */
#define LBN_NEWLINE     5
/*!
 * @brief End-of-input element type.
 */
#define LBN_END         6

/*!
 * @brief Infinite cost.
 */
#define LB_INFINITY             10000
/*!
 * @brief Square root of LB_INFINITY.
 */
#define LB_SQRT_INFINITY        100

/*!
 * @brief Hyphenation mark in dictionary words.
 */
#define LB_HYPHEN       '*'

/*!
 * @brief Line-breaking state.
 */
struct lb;
/*!
 * @brief Hyphenation dictionary.
 */
struct lbh;

/*!
 * @brief One element returned by lb_next().
 */
struct lb_node
{
  int type;                     /*!< Element type. */
  int value;                    /*!< Element width. */
  const char *word;             /*!< Element text. */
  const void *info;             /*!< Caller information. */
};

/*!
 * @brief Initialize a line-breaking state.
 *
 * @param[out] pp      Receives the new state. Not NULL.
 * @param[in]  lmargin Left margin.
 * @param[in]  rmargin Right margin.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_init (struct lb **pp, int lmargin, int rmargin);
/*!
 * @brief Free a line-breaking state.
 *
 * @param[in,out] p State to free; set to NULL. Not NULL.
 *
 * @return Always 0.
 */
int lb_exit (struct lb **p);
/*!
 * @brief Set the left margin of the first line.
 *
 * @param[in] p      Line-breaking state. Not NULL.
 * @param[in] margin Margin in characters.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_first_lmargin (struct lb *p, int margin);
/*!
 * @brief Set the right margin of the first line.
 *
 * @param[in] p      Line-breaking state. Not NULL.
 * @param[in] margin Margin in characters.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_first_rmargin (struct lb *p, int margin);
/*!
 * @brief Append a penalty node.
 *
 * @param[in] p       Line-breaking state. Not NULL.
 * @param[in] penalty Penalty value.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_penalty (struct lb *p, int penalty);
/*!
 * @brief Append a word node.
 *
 * @param[in] p     Line-breaking state. Not NULL.
 * @param[in] width Word width.
 * @param[in] word  Word text. Not NULL.
 * @param[in] info  Caller information.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_word (struct lb *p, int width, const char *word, const void *info);
/*!
 * @brief Append a discretionary hyphenation point.
 *
 * @param[in] p           Line-breaking state. Not NULL.
 * @param[in] width_word  Whole-word width.
 * @param[in] word        Word text. Not NULL.
 * @param[in] width_pre   Pre-hyphen width.
 * @param[in] pre         Pre-hyphen text or NULL.
 * @param[in] width_post  Post-hyphen width.
 * @param[in] post        Post-hyphen text or NULL.
 * @param[in] info        Caller information.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_discr (struct lb *p, int width_word, const char *word,
    int width_pre, const char *pre, int width_post, const char *post,
    const void *info);
/*!
 * @brief Append a discretionary hyphen.
 *
 * @param[in] p     Line-breaking state. Not NULL.
 * @param[in] width Hyphen width.
 * @param[in] info  Caller information.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_hyphen (struct lb *p, int width, const void *info);
/*!
 * @brief Append a glue node.
 *
 * @param[in] p     Line-breaking state. Not NULL.
 * @param[in] width Glue width.
 * @param[in] info  Caller information.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_glue (struct lb *p, int width, const void *info);
/*!
 * @brief Compute the optimal line break positions.
 *
 * @param[in,out] p Line-breaking state. Not NULL.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_format (struct lb *p);
/*!
 * @brief Retrieve the next output line-break element.
 *
 * @param[in,out] p   Line-breaking state. Not NULL.
 * @param[out]    dst Receives the next element. Not NULL.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lb_next (struct lb *p, struct lb_node *dst);

/*!
 * @brief Attach a hyphenation dictionary.
 *
 * @param[in] p Line-breaking state. Not NULL.
 * @param[in] h Hyphenation dictionary. Not NULL.
 *
 * @return Always 0.
 */
int lb_use_hyphenation (struct lb *p, const struct lbh *h);

/*!
 * @brief Initialize a hyphenation dictionary.
 *
 * @param[out] pp Receives the new dictionary. Not NULL.
 *
 * @return 0 on success, or an LB_* error code.
 */
int lbh_init (struct lbh **pp);
/*!
 * @brief Add a hyphenation word.
 *
 * @param[in] p Dictionary. Not NULL.
 * @param[in] s Word text. Not NULL.
 *
 * @return 0 on success, LB_INVAL on duplicate with different hyphenation,
 *         or an LB_* error code.
 */
int lbh_word (struct lbh *p, const char *s);
/*!
 * @brief Free a hyphenation dictionary.
 *
 * @param[in,out] pp Dictionary to free; set to NULL. Not NULL.
 *
 * @return Always 0.
 */
int lbh_exit (struct lbh **pp);
