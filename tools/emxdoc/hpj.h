/*!
 * @file hpj.h
 * @brief HPJ (WinHelp project) output emitter for emxdoc.
 *
 * Emits an HPJ project file for the Microsoft Windows Help
 * Compiler (HC30.EXE).  HPJ is a config file, not a document
 * format, so the emitter ignores all normal document tags and
 * produces only the fixed skeleton plus whatever the author writes
 * inside %hpj ... %endhpj blocks.
 *
 * In addition to the fixed skeleton, the emitter collects every
 * visible heading during the second pass and emits a [MAP] section
 * that associates each heading's context string with a stable
 * numeric ID.  The context string is "topic_<ref>", the same rule
 * that hc30.c uses for RTF footnotes; the numeric ID is 0x1000+ref.
 *
 * @copyright Copyright (C) 2026 osFree Project.
 *            License - see LICENSE in the project root.
 */

/*! @brief Initializes HPJ output; emits the fixed skeleton. */
void hpj_start (void);

/*! @brief Finalizes HPJ output; emits [MAP] and [WINDOWS]. */
void hpj_end (void);

/*!
 * @brief Remember the ref and flags of the heading being started.
 *
 * @param[in] level  Heading level (unused).
 * @param[in] ref    Reference number of the heading.
 * @param[in] global Non-zero if the section is global (unused).
 * @param[in] flags  HF_* flags.
 */
void hpj_heading1 (int level, int ref, int global, unsigned int flags);

/*!
 * @brief Record the heading's context string in the [MAP] list.
 *
 * @param[in] s Heading text (unused; the context string is
 *              derived from the ref number).
 */
void hpj_heading2 (const uchar *s);
