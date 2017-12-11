;==============================================================================
; FILE:       readData.j
; OVERVIEW:   This is the prototype jasmin source file (or a copy of it) which
;				tells the uqbtj system the names, sizes, and start addresses of
;				the data sections. Where information needs to be substituted,
;				there are strings starting with "$" (e.g. "$ dsizero" for size
;				of the read-only data section); these strings are changed to
;				the actual values using readData.sed and the "sed" stream
;				editing tool.
;
; Copyright (C) 1999, The University of Queensland, BT group
;============================================================================*/

.method datanames()[Ljava/lang/String;
	.limit locals 2
	.limit stack 3

	iconst_3
	anewarray	java/lang/String
	astore_1
	aload_1
	iconst_0
	ldc		"rodata"
	aastore
	aload_1
	iconst_1
	ldc		"rwdata"
	aastore
	aload_1
	iconst_2
	ldc		"bssdata"
	aastore
	aload_1
	areturn
.end method

.method datasizes()[I
	.limit locals 2
	.limit stack 3

	iconst_3
	newarray	 int
	astore_1
	aload_1
	iconst_0
	sipush		$dsize_ro
	iastore
	aload_1
	iconst_1
	sipush		$dsize_rw
	iastore
	aload_1
	iconst_2
	sipush		$dsize_bss
	iastore
	aload_1
	areturn
.end method

.method datastarts()[I
	.limit locals 2
	.limit stack 3

	iconst_3
	newarray	int
	astore_1
	aload_1
	iconst_0
	ldc		$dstart_ro
	iastore
	aload_1
	iconst_1
	ldc		$dstart_rw
	iastore
	aload_1
	iconst_2
	ldc		$dstart_bss
	iastore
	aload_1
	areturn
.end method
