typedef union {
    string *str;
    vector<string *> *strv;
    FieldIdent *fieldident;
    vector<FieldIdent *> *fiv;
    Expr *expr;
    ListExpr *list;
    ExprOp op;
    BitSlice bitslice;
    char ch;

    Ident *id;
    struct { int pos; string *str; } binding;
    struct { Expr *eq, *pat; } branch;
    OpcodePart opname;
    vector<OpcodePart> *opcode;
    OperandIdent * operand;
    struct { string *asmstr; vector<OperandIdent *> *ops; } operands;
    bool flag;
    struct { FieldChecked checked; vector<string *> *strv; } fielditem;
    int i;
} YYSTYPE;
#define	LEQUAL	257
#define	NEQUAL	258
#define	ELLIPSIS	259
#define	RARROW	260
#define	GEQUAL	261
#define	ADDRESS	262
#define	ANY	263
#define	ASSEMBLY	264
#define	BIT	265
#define	COLUMNS	266
#define	COMPONENT	267
#define	CONSTRUCTORS	268
#define	DISCARD	269
#define	ELSE	270
#define	EPSILON	271
#define	FETCH	272
#define	FIELDINFO	273
#define	FIELDS	274
#define	FOR	275
#define	IS	276
#define	KEEP	277
#define	NAMES	278
#define	OF	279
#define	OPCODE	280
#define	OTHERWISE	281
#define	PATTERNS	282
#define	MOST	283
#define	SIGNIFICANT	284
#define	PCUNITBITS	285
#define	PLACEHOLDER	286
#define	RELOCATABLE	287
#define	SOME	288
#define	SPARSE	289
#define	SYNTAX	290
#define	TO	291
#define	TYPE	292
#define	USING	293
#define	WHEN	294
#define	WHICH	295
#define	WORDSIZE	296
#define	NEWLINE	297
#define	CHECKED	298
#define	UNCHECKED	299
#define	GUARANTEED	300
#define	IDENT	301
#define	STRING	302
#define	LABEL	303
#define	FIELDIDENT	304
#define	RELOCIDENT	305
#define	PATTERNIDENT	306
#define	INTEGER	307


extern YYSTYPE yylval;
