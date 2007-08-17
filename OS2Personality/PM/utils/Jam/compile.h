/*
 * Copyright 1993, 2000 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * compile.h - compile parsed jam statements
 */

void compile_builtins(void);

LIST *compile_append( PARSE *parse, LOL *args );
LIST *compile_foreach( PARSE *parse, LOL *args );
LIST *compile_if( PARSE *parse, LOL *args );
LIST *compile_include( PARSE *parse, LOL *args );
LIST *compile_list( PARSE *parse, LOL *args );
LIST *compile_local( PARSE *parse, LOL *args );
LIST *compile_null( PARSE *parse, LOL *args );
LIST *compile_rule( PARSE *parse, LOL *args );
LIST *compile_rules( PARSE *parse, LOL *args );
LIST *compile_set( PARSE *parse, LOL *args );
LIST *compile_setcomp( PARSE *parse, LOL *args );
LIST *compile_setexec( PARSE *parse, LOL *args );
LIST *compile_settings( PARSE *parse, LOL *args );
LIST *compile_switch( PARSE *parse, LOL *args );

LIST *evaluate_rule( char *rulename, LOL *args );

/* Flags for compile_set(), etc */

# define ASSIGN_SET	0x00	/* = assign variable */
# define ASSIGN_APPEND	0x01	/* += append variable */
# define ASSIGN_DEFAULT	0x02	/* set only if unset */

/* Flags for compile_setexec() */

# define EXEC_UPDATED	0x01	/* executes updated */
# define EXEC_TOGETHER	0x02	/* executes together */
# define EXEC_IGNORE	0x04	/* executes ignore */
# define EXEC_QUIETLY	0x08	/* executes quietly */
# define EXEC_PIECEMEAL	0x10	/* executes piecemeal */
# define EXEC_EXISTING	0x20	/* executes existing */

/* Conditions for compile_if() */

# define COND_NOT	0	/* ! cond */
# define COND_AND	1	/* cond && cond */
# define COND_OR	2	/* cond || cond */

# define COND_EXISTS	3	/* arg */
# define COND_EQUALS	4	/* arg = arg */
# define COND_NOTEQ	5	/* arg != arg */
# define COND_LESS	6	/* arg < arg  */
# define COND_LESSEQ	7	/* arg <= arg */
# define COND_MORE	8	/* arg > arg  */
# define COND_MOREEQ	9	/* arg >= arg */
# define COND_IN	10	/* arg in arg */
