/*!
 * @file sed.h
 *
 * @brief Types and constants for the stream editor.
 *
 * sed.h -- types and constants for the stream editor.
 *
 * This header defines the compiled-command representation used by
 * the compilation phase (sedcomp.c) and the execution phase
 * (sedexec.c) of the stream editor, together with the shared data
 * areas and the small set of external variables they communicate
 * through.
 */

/* data area sizes used by both modules */
#define MAXBUF          8192    /*!< Current line buffer size.     */
#define MAXAPPENDS      20      /*!< Maximum number of appends.    */
#define MAXTAGS         9       /*!< Tagged patterns are \1 to \9. */

/* constants for compiled-command representation */
#define EQCMD   0x01    /*!< = -- print current line number.          */
#define ACMD    0x02    /*!< a -- append text after current line.     */
#define BCMD    0x03    /*!< b -- branch to label.                    */
#define CCMD    0x04    /*!< c -- change current line.                */
#define DCMD    0x05    /*!< d -- delete all of pattern space.        */
#define CDCMD   0x06    /*!< D -- delete first line of pattern space. */
#define GCMD    0x07    /*!< g -- copy hold space to pattern space.   */
#define CGCMD   0x08    /*!< G -- append hold space to pattern space. */
#define HCMD    0x09    /*!< h -- copy pattern space to hold space.   */
#define CHCMD   0x0A    /*!< H -- append hold space to pattern space. */
#define ICMD    0x0B    /*!< i -- insert text before current line.    */
#define LCMD    0x0C    /*!< l -- print pattern space in escaped form.*/
#define NCMD    0x0D    /*!< n -- get next line into pattern space.   */
#define CNCMD   0x0E    /*!< N -- append next line to pattern space.  */
#define PCMD    0x0F    /*!< p -- print pattern space to output.      */
#define CPCMD   0x10    /*!< P -- print first line of pattern space.  */
#define QCMD    0x11    /*!< q -- exit the stream editor.             */
#define RCMD    0x12    /*!< r -- read in a file after current line.  */
#define SCMD    0x13    /*!< s -- regular-expression substitute.      */
#define TCMD    0x14    /*!< t -- branch on last substitute ok.       */
#define CTCMD   0x15    /*!< T -- branch on last substitute failed.   */
#define WCMD    0x16    /*!< w -- write pattern space to file.        */
#define CWCMD   0x17    /*!< W -- write first line of pattern space.  */
#define XCMD    0x18    /*!< x -- exhange pattern and hold spaces.    */
#define YCMD    0x19    /*!< y -- transliterate text.                 */

/*!
 * @brief Compiled-command representation.
 *
 * Use this name for declarations.
 */
typedef struct cmd_t    sedcmd;

/*!
 * @struct cmd_t
 * @brief Compiled-command representation.
 *
 * One instance is produced by compile() for each command in the
 * script. The interpreter in sedexec.c reads this table via the
 * cmds[] array.
 */
struct cmd_t {
    char                *addr1;         /*!< First address for command.      */
    char                *addr2;         /*!< Second address for command.     */
    union {
        char            *lhs;           /*!< s command lhs.                  */
        sedcmd          *link;          /*!< Label link.                     */
    }                   u;
    char                command;        /*!< Command code.                   */
    char                *rhs;           /*!< s command replacement string.   */
    FILE                *fout;          /*!< Associated output file.         */
    struct {
        unsigned        nthone  :11;    /*!< If non-zero, only nth replace.  */
        unsigned        allbut  : 1;    /*!< Was negation specified?         */
        unsigned        global  : 1;    /*!< Was g postfix specified?        */
        unsigned        print   : 2;    /*!< Was p postfix specified?        */
        unsigned        inrange : 1;    /*!< In an address range?            */
    }                   flags;          /*!< Command flags.                  */
};

#define BAD     ((char *) -1)           /*!< Guaranteed not a string ptr.    */

/* address and regular expression compiled-form markers */
#define STAR    1       /*!< Marker for Kleene star.                */
#define CCHR    2       /*!< Non-newline char to be matched follows.*/
#define CDOT    4       /*!< Dot wild-card marker.                  */
#define CCL     6       /*!< Character class follows.               */
#define CNL     8       /*!< Match line start.                      */
#define CDOL    10      /*!< Match line end.                        */
#define CBRA    12      /*!< Tagged pattern start marker.           */
#define CKET    14      /*!< Tagged pattern end marker.             */
#define CBACK   16      /*!< Backslash-digit pair marker.           */
#define CLNUM   18      /*!< Numeric-address index follows.         */
#define CEND    20      /*!< Symbol for end-of-source.              */
#define CEOF    22      /*!< End-of-field mark.                     */
#define MTYPE   32      /*!< Multiple counts |'d into RE \{...\}.   */

/*!
 * @brief Execute the compiled commands of the stream editor.
 *
 * Runs the compiled commands in cmds[] against the text of @p file,
 * or against standard input when @p file is NULL.
 *
 * @param[in] file  Name of the text source file to filter, or NULL
 *                  to read from standard input.
 *
 * @see compile
 */
extern void     execute( const char *file ); /* In sedexec.c */

/* The following are in sedcomp.c */
                                        /* main data areas */
/*!
 * @brief Current-line buffer.
 */
extern char             linebuf[];
/*!
 * @brief Compiled-command table.
 */
extern sedcmd           cmds[];
/*!
 * @brief Numeric-addresses table.
 */
extern long             linenum[];

                                        /* miscellaneous shared variables */
/*!
 * @brief -n option flag.
 */
extern int              nflag;
/*!
 * @brief Scratch copy of the argument count.
 */
extern int              eargc;
/*!
 * @brief Bitmask table.
 */
extern char const       bits[];

/* sed.h ends here */
