typedef union {
	IDL_tree tree;
	struct {
		IDL_tree tree;
		gpointer data;
	} treedata;
	GHashTable *hash_table;
	char *str;
	gboolean boolean;
	IDL_declspec_t declspec;
	IDL_longlong_t integer;
	double floatp;
	enum IDL_unaryop unaryop;
	enum IDL_param_attr paramattr;
} YYSTYPE;
#define	TOK_ANY	257
#define	TOK_ATTRIBUTE	258
#define	TOK_BOOLEAN	259
#define	TOK_CASE	260
#define	TOK_CHAR	261
#define	TOK_CONST	262
#define	TOK_CONTEXT	263
#define	TOK_DEFAULT	264
#define	TOK_DOUBLE	265
#define	TOK_ENUM	266
#define	TOK_EXCEPTION	267
#define	TOK_FALSE	268
#define	TOK_FIXED	269
#define	TOK_FLOAT	270
#define	TOK_IN	271
#define	TOK_INOUT	272
#define	TOK_INTERFACE	273
#define	TOK_LONG	274
#define	TOK_MODULE	275
#define	TOK_NATIVE	276
#define	TOK_OBJECT	277
#define	TOK_OCTET	278
#define	TOK_ONEWAY	279
#define	TOK_OP_SCOPE	280
#define	TOK_OP_SHL	281
#define	TOK_OP_SHR	282
#define	TOK_OUT	283
#define	TOK_RAISES	284
#define	TOK_READONLY	285
#define	TOK_SEQUENCE	286
#define	TOK_SHORT	287
#define	TOK_STRING	288
#define	TOK_STRUCT	289
#define	TOK_SWITCH	290
#define	TOK_TRUE	291
#define	TOK_TYPECODE	292
#define	TOK_TYPEDEF	293
#define	TOK_UNION	294
#define	TOK_UNSIGNED	295
#define	TOK_VARARGS	296
#define	TOK_VOID	297
#define	TOK_WCHAR	298
#define	TOK_WSTRING	299
#define	TOK_FLOATP	300
#define	TOK_INTEGER	301
#define	TOK_DECLSPEC	302
#define	TOK_PROP_KEY	303
#define	TOK_PROP_VALUE	304
#define	TOK_NATIVE_TYPE	305
#define	TOK_IDENT	306
#define	TOK_SQSTRING	307
#define	TOK_DQSTRING	308
#define	TOK_FIXEDP	309
#define	TOK_CODEFRAG	310
#define	TOK_SRCFILE	311


extern YYSTYPE yylval;
