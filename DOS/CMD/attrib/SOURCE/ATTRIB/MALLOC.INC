static char malloc_buffer[8*1024u];
static size_t malloc_next = 0;

/* "Check for stack overflow" off for BC++ 3.1 */
/* "Suppress redundant load" on for BC++ 3.1 */
#pragma option -N- -Z

#ifdef __cplusplus
extern "C"
#endif
void *_Cdecl malloc (size_t sz) {
	size_t old = malloc_next;
	if (sizeof (malloc_buffer) - old < sz)
		return NULL;

	malloc_next += sz;
	return &malloc_buffer [old];
}

/* "Parameter is never used in function" off for TC/BC */
#pragma warn -par

#ifdef __cplusplus
extern "C"
#endif
void _Cdecl free () {}

#pragma warn .par
#pragma option -N. -Z.
