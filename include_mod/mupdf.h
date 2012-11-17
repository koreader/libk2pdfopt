#ifndef MUPDF_H
#define MUPDF_H

#ifndef FITZ_H
#define FITZ_H

/*
	Include the standard libc headers.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>	/* INT_MAX & co */
#include <float.h> /* FLT_EPSILON, FLT_MAX & co */
#include <fcntl.h> /* O_RDONLY & co */

#include <setjmp.h>

/* Copyright (C) 2011 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/
   or contact Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134,
   San Rafael, CA  94903, U.S.A., +1(415)492-9861, for further information.
*/

/* Memento: A library to aid debugging of memory leaks/heap corruption.
 *
 * Usage:
 *    First, build your project with MEMENTO defined, and include this
 *    header file wherever you use malloc, realloc or free.
 *    This header file will use macros to point malloc, realloc and free to
 *    point to Memento_malloc, Memento_realloc, Memento_free.
 *
 *    Run your program, and all mallocs/frees/reallocs should be redirected
 *    through here. When the program exits, you will get a list of all the
 *    leaked blocks, together with some helpful statistics. You can get the
 *    same list of allocated blocks at any point during program execution by
 *    calling Memento_listBlocks();
 *
 *    Every call to malloc/free/realloc counts as an 'allocation event'.
 *    On each event Memento increments a counter. Every block is tagged with
 *    the current counter on allocation. Every so often during program
 *    execution, the heap is checked for consistency. By default this happens
 *    every 1024 events. This can be changed at runtime by using
 *    Memento_setParanoia(int level). 0 turns off such checking, 1 sets
 *    checking to happen on every event, any other number n sets checking to
 *    happen once every n events.
 *
 *    Memento keeps blocks around for a while after they have been freed, and
 *    checks them as part of these heap checks to see if they have been
 *    written to (or are freed twice etc).
 *
 *    A given heap block can be checked for consistency (it's 'pre' and
 *    'post' guard blocks are checked to see if they have been written to)
 *    by calling Memento_checkBlock(void *blockAddress);
 *
 *    A check of all the memory can be triggered by calling Memento_check();
 *    (or Memento_checkAllMemory(); if you'd like it to be quieter).
 *
 *    A good place to breakpoint is Memento_breakpoint, as this will then
 *    trigger your debugger if an error is detected. This is done
 *    automatically for debug windows builds.
 *
 *    If a block is found to be corrupt, information will be printed to the
 *    console, including the address of the block, the size of the block,
 *    the type of corruption, the number of the block and the event on which
 *    it last passed a check for correctness.
 *
 *    If you rerun, and call Memento_paranoidAt(int event); with this number
 *    the the code will wait until it reaches that event and then start
 *    checking the heap after every allocation event. Assuming it is a
 *    deterministic failure, you should then find out where in your program
 *    the error is occurring (between event x-1 and event x).
 *
 *    Then you can rerun the program again, and call
 *    Memento_breakAt(int event); and the program will call
 *    Memento_Breakpoint() when event x is reached, enabling you to step
 *    through.
 *
 *    Memento_find(address) will tell you what block (if any) the given
 *    address is in.
 *
 * An example:
 *    Suppose we have a gs invocation that crashes with memory corruption.
 *     * Build with -DMEMENTO.
 *     * In your debugger put breakpoints on Memento_inited and
 *       Memento_Breakpoint.
 *     * Run the program. It will stop in Memento_inited.
 *     * Execute Memento_setParanoia(1);  (In VS use Ctrl-Alt-Q). (Note #1)
 *     * Continue execution.
 *     * It will detect the memory corruption on the next allocation event
 *       after it happens, and stop in Memento_breakpoint. The console should
 *       show something like:
 *
 *       Freed blocks:
 *         0x172e610(size=288,num=1415) index 256 (0x172e710) onwards corrupted
 *           Block last checked OK at allocation 1457. Now 1458.
 *
 *     * This means that the block became corrupted between allocation 1457
 *       and 1458 - so if we rerun and stop the program at 1457, we can then
 *       step through, possibly with a data breakpoint at 0x172e710 and see
 *       when it occurs.
 *     * So restart the program from the beginning. When we hit Memento_inited
 *       execute Memento_breakAt(1457); (and maybe Memento_setParanoia(1), or
 *       Memento_setParanoidAt(1457))
 *     * Continue execution until we hit Memento_breakpoint.
 *     * Now you can step through and watch the memory corruption happen.
 *
 *    Note #1: Using Memento_setParanoia(1) can cause your program to run
 *    very slowly. You may instead choose to use Memento_setParanoia(100)
 *    (or some other figure). This will only exhaustively check memory on
 *    every 100th allocation event. This trades speed for the size of the
 *    average allocation event range in which detection of memory corruption
 *    occurs. You may (for example) choose to run once checking every 100
 *    allocations and discover that the corruption happens between events
 *    X and X+100. You can then rerun using Memento_paranoidAt(X), and
 *    it'll only start exhaustively checking when it reaches X.
 *
 * More than one memory allocator?
 *
 *    If you have more than one memory allocator in the system (like for
 *    instance the ghostscript chunk allocator, that builds on top of the
 *    standard malloc and returns chunks itself), then there are some things
 *    to note:
 *
 *    * If the secondary allocator gets its underlying blocks from calling
 *      malloc, then those will be checked by Memento, but 'subblocks' that
 *      are returned to the secondary allocator will not. There is currently
 *      no way to fix this other than trying to bypass the secondary
 *      allocator. One way I have found to do this with the chunk allocator
 *      is to tweak its idea of a 'large block' so that it puts every
 *      allocation in its own chunk. Clearly this negates the point of having
 *      a secondary allocator, and is therefore not recommended for general
 *      use.
 *
 *    * Again, if the secondary allocator gets its underlying blocks from
 *      calling malloc (and hence Memento) leak detection should still work
 *      (but whole blocks will be detected rather than subblocks).
 *
 *    * If on every allocation attempt the secondary allocator calls into
 *      Memento_failThisEvent(), and fails the allocation if it returns true
 *      then more useful features can be used; firstly memory squeezing will
 *      work, and secondly, Memento will have a "finer grained" paranoia
 *      available to it.
 */

#ifndef MEMENTO_H

#include <memory.h>

#define MEMENTO_H

#ifndef MEMENTO_UNDERLYING_MALLOC
#define MEMENTO_UNDERLYING_MALLOC malloc
#endif
#ifndef MEMENTO_UNDERLYING_FREE
#define MEMENTO_UNDERLYING_FREE free
#endif
#ifndef MEMENTO_UNDERLYING_REALLOC
#define MEMENTO_UNDERLYING_REALLOC realloc
#endif
#ifndef MEMENTO_UNDERLYING_CALLOC
#define MEMENTO_UNDERLYING_CALLOC calloc
#endif

#ifndef MEMENTO_MAXALIGN
#define MEMENTO_MAXALIGN (sizeof(int))
#endif

#define MEMENTO_PREFILL   0xa6
#define MEMENTO_POSTFILL  0xa7
#define MEMENTO_ALLOCFILL 0xa8
#define MEMENTO_FREEFILL  0xa9

#define MEMENTO_FREELIST_MAX 0x2000000

int Memento_checkBlock(void *);
int Memento_checkAllMemory(void);
int Memento_check(void);

int Memento_setParanoia(int);
int Memento_paranoidAt(int);
int Memento_breakAt(int);
int Memento_getBlockNum(void *);
int Memento_find(void *a);
void Memento_breakpoint(void);
int Memento_failAt(int);
int Memento_failThisEvent(void);
void Memento_listBlocks(void);
void Memento_listNewBlocks(void);
size_t Memento_setMax(size_t);
void Memento_stats(void);
void *Memento_label(void *, const char *);

void *Memento_malloc(size_t s);
void *Memento_realloc(void *, size_t s);
void  Memento_free(void *);
void *Memento_calloc(size_t, size_t);

#ifdef MEMENTO

#ifndef COMPILING_MEMENTO_C
#define malloc  Memento_malloc
#define free    Memento_free
#define realloc Memento_realloc
#define calloc  Memento_calloc
#endif

#else

#define Memento_malloc  MEMENTO_UNDERLYING_MALLOC
#define Memento_free    MEMENTO_UNDERLYING_FREE
#define Memento_realloc MEMENTO_UNDERLYING_REALLOC
#define Memento_calloc  MEMENTO_UNDERLYING_CALLOC

#define Memento_checkBlock(A)    0
#define Memento_checkAllMemory() 0
#define Memento_check()          0
#define Memento_setParanoia(A)   0
#define Memento_paranoidAt(A)    0
#define Memento_breakAt(A)       0
#define Memento_getBlockNum(A)   0
#define Memento_find(A)          0
#define Memento_breakpoint()     do {} while (0)
#define Memento_failAt(A)        0
#define Memento_failThisEvent()  0
#define Memento_listBlocks()     do {} while (0)
#define Memento_listNewBlocks()  do {} while (0)
#define Memento_setMax(A)        0
#define Memento_stats()          do {} while (0)
#define Memento_label(A,B)       (A)

#endif /* MEMENTO */

#endif /* MEMENTO_H */

#ifdef __APPLE__
#define HAVE_SIGSETJMP
#elif defined(__unix)
#define HAVE_SIGSETJMP
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "libmupdf"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define LOGI(...) do {} while(0)
#define LOGE(...) do {} while(0)
#endif

#define nelem(x) (sizeof(x)/sizeof((x)[0]))

/*
	Some differences in libc can be smoothed over
*/

#ifdef _MSC_VER /* Microsoft Visual C */

#pragma warning( disable: 4244 ) /* conversion from X to Y, possible loss of data */
#pragma warning( disable: 4996 ) /* The POSIX name for this item is deprecated */
#pragma warning( disable: 4996 ) /* This function or variable may be unsafe */

#include <io.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);

#define snprintf _snprintf
#define isnan _isnan
#define hypotf _hypotf

#else /* Unix or close enough */

#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

/*
	Variadic macros, inline and restrict keywords

	inline is standard in C++, so don't touch the definition in this case.
	For some compilers we can enable it within C too.
*/

#ifndef __cplusplus
#if __STDC_VERSION__ == 199901L /* C99 */
#elif _MSC_VER >= 1500 /* MSVC 9 or newer */
#define inline __inline
#elif __GNUC__ >= 3 /* GCC 3 or newer */
#define inline __inline
#else /* Unknown or ancient */
#define inline
#endif
#endif

/*
	restrict is standard in C99, but not in all C++ compilers. Enable
	where possible, disable if in doubt.
 */
#if __STDC_VERSION__ == 199901L /* C99 */
#elif _MSC_VER >= 1500 /* MSVC 9 or newer */
#define restrict __restrict
#elif __GNUC__ >= 3 /* GCC 3 or newer */
#define restrict __restrict
#else /* Unknown or ancient */
#define restrict
#endif

/*
	GCC can do type checking of printf strings
*/

#ifndef __printflike
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#define __printflike(fmtarg, firstvararg) \
	__attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#else
#define __printflike(fmtarg, firstvararg)
#endif
#endif

/*
	Some standard math functions, done as static inlines for speed.
	People with compilers that do not adequately implement inlines may
	like to reimplement these using macros.
*/
static inline float fz_abs(float f)
{
	return (f < 0 ? -f : f);
}

static inline int fz_absi(int i)
{
	return (i < 0 ? -i : i);
}

static inline float fz_min(float a, float b)
{
	return (a < b ? a : b);
}

static inline int fz_mini(int a, int b)
{
	return (a < b ? a : b);
}

static inline float fz_max(float a, float b)
{
	return (a > b ? a : b);
}

static inline int fz_maxi(int a, int b)
{
	return (a > b ? a : b);
}

static inline float fz_clamp(float f, float min, float max)
{
	return (f > min ? (f < max ? f : max) : min);
}

static inline int fz_clampi(int i, float min, float max)
{
	return (i > min ? (i < max ? i : max) : min);
}

static inline double fz_clampd(double d, double min, double max)
{
	return (d > min ? (d < max ? d : max) : min);
}

static inline void *fz_clampp(void *p, void *min, void *max)
{
	return (p > min ? (p < max ? p : max) : min);
}

#define DIV_BY_ZERO(a, b, min, max) (((a) < 0) ^ ((b) < 0) ? (min) : (max))

/*
	Contexts
*/

typedef struct fz_alloc_context_s fz_alloc_context;
typedef struct fz_error_context_s fz_error_context;
typedef struct fz_warn_context_s fz_warn_context;
typedef struct fz_font_context_s fz_font_context;
typedef struct fz_aa_context_s fz_aa_context;
typedef struct fz_locks_context_s fz_locks_context;
typedef struct fz_store_s fz_store;
typedef struct fz_glyph_cache_s fz_glyph_cache;
typedef struct fz_context_s fz_context;

struct fz_alloc_context_s
{
	void *user;
	void *(*malloc)(void *, unsigned int);
	void *(*realloc)(void *, void *, unsigned int);
	void (*free)(void *, void *);
};

/*
	Where possible (i.e. on platforms on which they are provided), use
	sigsetjmp/siglongjmp in preference to setjmp/longjmp. We don't alter
	signal handlers within mupdf, so there is no need for us to
	store/restore them - hence we use the non-restoring variants. This
	makes a large speed difference on MacOSX (and probably other
	platforms too.
*/
#ifdef HAVE_SIGSETJMP
#define fz_setjmp(BUF) sigsetjmp(BUF, 0)
#define fz_longjmp(BUF,VAL) siglongjmp(BUF, VAL)
#define fz_jmp_buf sigjmp_buf
#else
#define fz_setjmp(BUF) setjmp(BUF)
#define fz_longjmp(BUF,VAL) longjmp(BUF,VAL)
#define fz_jmp_buf jmp_buf
#endif

struct fz_error_context_s
{
	int top;
	struct {
		int code;
		fz_jmp_buf buffer;
	} stack[256];
	char message[256];
};

void fz_var_imp(void *);
#define fz_var(var) fz_var_imp((void *)&(var))


/*
	Exception macro definitions. Just treat these as a black box - pay no
	attention to the man behind the curtain.
*/

#define fz_try(ctx) \
	if (fz_push_try(ctx->error) && \
		((ctx->error->stack[ctx->error->top].code = fz_setjmp(ctx->error->stack[ctx->error->top].buffer)) == 0))\
	{ do {

#define fz_always(ctx) \
		} while (0); \
	} \
	{ do { \

#define fz_catch(ctx) \
		} while(0); \
	} \
	if (ctx->error->stack[ctx->error->top--].code)

int fz_push_try(fz_error_context *ex);
void fz_throw(fz_context *, char *, ...) __printflike(2, 3);
void fz_rethrow(fz_context *);
void fz_warn(fz_context *ctx, char *fmt, ...) __printflike(2, 3);

/*
	fz_flush_warnings: Flush any repeated warnings.

	Repeated warnings are buffered, counted and eventually printed
	along with the number of repetitions. Call fz_flush_warnings
	to force printing of the latest buffered warning and the
	number of repetitions, for example to make sure that all
	warnings are printed before exiting an application.

	Does not throw exceptions.
*/
void fz_flush_warnings(fz_context *ctx);

struct fz_context_s
{
	fz_alloc_context *alloc;
	fz_locks_context *locks;
	fz_error_context *error;
	fz_warn_context *warn;
	fz_font_context *font;
	fz_aa_context *aa;
	fz_store *store;
	fz_glyph_cache *glyph_cache;
};

/*
	Specifies the maximum size in bytes of the resource store in
	fz_context. Given as argument to fz_new_context.

	FZ_STORE_UNLIMITED: Let resource store grow unbounded.

	FZ_STORE_DEFAULT: A reasonable upper bound on the size, for
	devices that are not memory constrained.
*/
enum {
	FZ_STORE_UNLIMITED = 0,
	FZ_STORE_DEFAULT = 256 << 20,
};

/*
	fz_new_context: Allocate context containing global state.

	The global state contains an exception stack, resource store,
	etc. Most functions in MuPDF take a context argument to be
	able to reference the global state. See fz_free_context for
	freeing an allocated context.

	alloc: Supply a custom memory allocator through a set of
	function pointers. Set to NULL for the standard library
	allocator. The context will keep the allocator pointer, so the
	data it points to must not be modified or freed during the
	lifetime of the context.

	locks: Supply a set of locks and functions to lock/unlock
	them, intended for multi-threaded applications. Set to NULL
	when using MuPDF in a single-threaded applications. The
	context will keep the locks pointer, so the data it points to
	must not be modified or freed during the lifetime of the
	context.

	max_store: Maximum size in bytes of the resource store, before
	it will start evicting cached resources such as fonts and
	images. FZ_STORE_UNLIMITED can be used if a hard limit is not
	desired. Use FZ_STORE_DEFAULT to get a reasonable size.

	Does not throw exceptions, but may return NULL.
*/
fz_context *fz_new_context(fz_alloc_context *alloc, fz_locks_context *locks, unsigned int max_store);

/*
	fz_clone_context: Make a clone of an existing context.

	This function is meant to be used in multi-threaded
	applications where each thread requires its own context, yet
	parts of the global state, for example caching, is shared.

	ctx: Context obtained from fz_new_context to make a copy of.
	ctx must have had locks and lock/functions setup when created.
	The two contexts will share the memory allocator, resource
	store, locks and lock/unlock functions. They will each have
	their own exception stacks though.

	Does not throw exception, but may return NULL.
*/
fz_context *fz_clone_context(fz_context *ctx);

/*
	fz_free_context: Free a context and its global state.

	The context and all of its global state is freed, and any
	buffered warnings are flushed (see fz_flush_warnings). If NULL
	is passed in nothing will happen.

	Does not throw exceptions.
*/
void fz_free_context(fz_context *ctx);

/*
	fz_aa_level: Get the number of bits of antialiasing we are
	using. Between 0 and 8.
*/
int fz_aa_level(fz_context *ctx);

/*
	fz_set_aa_level: Set the number of bits of antialiasing we should use.

	bits: The number of bits of antialiasing to use (values are clamped
	to within the 0 to 8 range).
*/
void fz_set_aa_level(fz_context *ctx, int bits);

/*
	Locking functions

	MuPDF is kept deliberately free of any knowledge of particular
	threading systems. As such, in order for safe multi-threaded
	operation, we rely on callbacks to client provided functions.

	A client is expected to provide FZ_LOCK_MAX number of mutexes,
	and a function to lock/unlock each of them. These may be
	recursive mutexes, but do not have to be.

	If a client does not intend to use multiple threads, then it
	may pass NULL instead of a lock structure.

	In order to avoid deadlocks, we have one simple rule
	internally as to how we use locks: We can never take lock n
	when we already hold any lock i, where 0 <= i <= n. In order
	to verify this, we have some debugging code, that can be
	enabled by defining FITZ_DEBUG_LOCKING.
*/

struct fz_locks_context_s
{
	void *user;
	void (*lock)(void *user, int lock);
	void (*unlock)(void *user, int lock);
};

enum {
	FZ_LOCK_ALLOC = 0,
	FZ_LOCK_FILE,
	FZ_LOCK_FREETYPE,
	FZ_LOCK_GLYPHCACHE,
	FZ_LOCK_MAX
};

/*
	Memory Allocation and Scavenging:

	All calls to MuPDFs allocator functions pass through to the
	underlying allocators passed in when the initial context is
	created, after locks are taken (using the supplied locking function)
	to ensure that only one thread at a time calls through.

	If the underlying allocator fails, MuPDF attempts to make room for
	the allocation by evicting elements from the store, then retrying.

	Any call to allocate may then result in several calls to the underlying
	allocator, and result in elements that are only referred to by the
	store being freed.
*/

/*
	fz_malloc: Allocate a block of memory (with scavenging)

	size: The number of bytes to allocate.

	Returns a pointer to the allocated block. May return NULL if size is
	0. Throws exception on failure to allocate.
*/
void *fz_malloc(fz_context *ctx, unsigned int size);

/*
	fz_calloc: Allocate a zeroed block of memory (with scavenging)

	count: The number of objects to allocate space for.

	size: The size (in bytes) of each object.

	Returns a pointer to the allocated block. May return NULL if size
	and/or count are 0. Throws exception on failure to allocate.
*/
void *fz_calloc(fz_context *ctx, unsigned int count, unsigned int size);

/*
	fz_malloc_struct: Allocate storage for a structure (with scavenging),
	clear it, and (in Memento builds) tag the pointer as belonging to a
	struct of this type.

	CTX: The context.

	STRUCT: The structure type.

	Returns a pointer to allocated (and cleared) structure. Throws
	exception on failure to allocate.
*/
#define fz_malloc_struct(CTX, STRUCT) \
	((STRUCT *)Memento_label(fz_calloc(CTX,1,sizeof(STRUCT)), #STRUCT))

/*
	fz_malloc_array: Allocate a block of (non zeroed) memory (with
	scavenging). Equivalent to fz_calloc without the memory clearing.

	count: The number of objects to allocate space for.

	size: The size (in bytes) of each object.

	Returns a pointer to the allocated block. May return NULL if size
	and/or count are 0. Throws exception on failure to allocate.
*/
void *fz_malloc_array(fz_context *ctx, unsigned int count, unsigned int size);

/*
	fz_resize_array: Resize a block of memory (with scavenging).

	p: The existing block to resize

	count: The number of objects to resize to.

	size: The size (in bytes) of each object.

	Returns a pointer to the resized block. May return NULL if size
	and/or count are 0. Throws exception on failure to resize (original
	block is left unchanged).
*/
void *fz_resize_array(fz_context *ctx, void *p, unsigned int count, unsigned int size);

/*
	fz_strdup: Duplicate a C string (with scavenging)

	s: The string to duplicate.

	Returns a pointer to a duplicated string. Throws exception on failure
	to allocate.
*/
char *fz_strdup(fz_context *ctx, char *s);

/*
	fz_free: Frees an allocation.

	Does not throw exceptions.
*/
void fz_free(fz_context *ctx, void *p);

/*
	fz_malloc_no_throw: Allocate a block of memory (with scavenging)

	size: The number of bytes to allocate.

	Returns a pointer to the allocated block. May return NULL if size is
	0. Returns NULL on failure to allocate.
*/
void *fz_malloc_no_throw(fz_context *ctx, unsigned int size);

/*
	fz_calloc_no_throw: Allocate a zeroed block of memory (with scavenging)

	count: The number of objects to allocate space for.

	size: The size (in bytes) of each object.

	Returns a pointer to the allocated block. May return NULL if size
	and/or count are 0. Returns NULL on failure to allocate.
*/
void *fz_calloc_no_throw(fz_context *ctx, unsigned int count, unsigned int size);

/*
	fz_malloc_array_no_throw: Allocate a block of (non zeroed) memory
	(with scavenging). Equivalent to fz_calloc_no_throw without the
	memory clearing.

	count: The number of objects to allocate space for.

	size: The size (in bytes) of each object.

	Returns a pointer to the allocated block. May return NULL if size
	and/or count are 0. Returns NULL on failure to allocate.
*/
void *fz_malloc_array_no_throw(fz_context *ctx, unsigned int count, unsigned int size);

/*
	fz_resize_array_no_throw: Resize a block of memory (with scavenging).

	p: The existing block to resize

	count: The number of objects to resize to.

	size: The size (in bytes) of each object.

	Returns a pointer to the resized block. May return NULL if size
	and/or count are 0. Returns NULL on failure to resize (original
	block is left unchanged).
*/
void *fz_resize_array_no_throw(fz_context *ctx, void *p, unsigned int count, unsigned int size);

/*
	fz_strdup_no_throw: Duplicate a C string (with scavenging)

	s: The string to duplicate.

	Returns a pointer to a duplicated string. Returns NULL on failure
	to allocate.
*/
char *fz_strdup_no_throw(fz_context *ctx, char *s);

/*
	Safe string functions
*/
/*
	fz_strsep: Given a pointer to a C string (or a pointer to NULL) break
	it at the first occurence of a delimiter char (from a given set).

	stringp: Pointer to a C string pointer (or NULL). Updated on exit to
	point to the first char of the string after the delimiter that was
	found. The string pointed to by stringp will be corrupted by this
	call (as the found delimiter will be overwritten by 0).

	delim: A C string of acceptable delimiter characters.

	Returns a pointer to a C string containing the chars of stringp up
	to the first delimiter char (or the end of the string), or NULL.
*/
char *fz_strsep(char **stringp, const char *delim);

/*
	fz_strlcpy: Copy at most n-1 chars of a string into a destination
	buffer with null termination, returning the real length of the
	initial string (excluding terminator).

	dst: Destination buffer, at least n bytes long.

	src: C string (non-NULL).

	n: Size of dst buffer in bytes.

	Returns the length (excluding terminator) of src.
*/
int fz_strlcpy(char *dst, const char *src, int n);

/*
	fz_strlcat: Concatenate 2 strings, with a maximum length.

	dst: pointer to first string in a buffer of n bytes.

	src: pointer to string to concatenate.

	n: Size (in bytes) of buffer that dst is in.

	Returns the real length that a concatenated dst + src would have been
	(not including terminator).
*/
int fz_strlcat(char *dst, const char *src, int n);

/*
	fz_chartorune: UTF8 decode a single rune from a sequence of chars.

	rune: Pointer to an int to assign the decoded 'rune' to.

	str: Pointer to a UTF8 encoded string.

	Returns the number of bytes consumed. Does not throw exceptions.
*/
int fz_chartorune(int *rune, char *str);

/*
	fz_runetochar: UTF8 encode a rune to a sequence of chars.

	str: Pointer to a place to put the UTF8 encoded character.

	rune: Pointer to a 'rune'.

	Returns the number of bytes the rune took to output. Does not throw
	exceptions.
*/
int fz_runetochar(char *str, int rune);

/*
	fz_runelen: Count how many chars are required to represent a rune.

	rune: The rune to encode.

	Returns the number of bytes required to represent this run in UTF8.
*/
int fz_runelen(int rune);

/*
	getopt: Simple functions/variables for use in tools.
*/
extern int fz_getopt(int nargc, char * const *nargv, const char *ostr);
extern int fz_optind;
extern char *fz_optarg;

/*
	fz_point is a point in a two-dimensional space.
*/
typedef struct fz_point_s fz_point;
struct fz_point_s
{
	float x, y;
};

/*
	fz_rect is a rectangle represented by two diagonally opposite
	corners at arbitrary coordinates.

	Rectangles are always axis-aligned with the X- and Y- axes.
	The relationship between the coordinates are that x0 <= x1 and
	y0 <= y1 in all cases except for infinte rectangles. The area
	of a rectangle is defined as (x1 - x0) * (y1 - y0). If either
	x0 > x1 or y0 > y1 is true for a given rectangle then it is
	defined to be infinite.

	To check for empty or infinite rectangles use fz_is_empty_rect
	and fz_is_infinite_rect. Compare to fz_bbox which has corners
	at integer coordinates.

	x0, y0: The top left corner.

	x1, y1: The botton right corner.
*/
typedef struct fz_rect_s fz_rect;
struct fz_rect_s
{
	float x0, y0;
	float x1, y1;
};

/*
	fz_bbox is a bounding box similar to a fz_rect, except that
	all corner coordinates are rounded to integer coordinates.
	To check for empty or infinite bounding boxes use
	fz_is_empty_bbox and fz_is_infinite_bbox.

	x0, y0: The top left corner.

	x1, y1: The bottom right corner.
*/
typedef struct fz_bbox_s fz_bbox;
struct fz_bbox_s
{
	int x0, y0;
	int x1, y1;
};

/*
	A rectangle with sides of length one.

	The bottom left corner is at (0, 0) and the top right corner
	is at (1, 1).
*/
extern const fz_rect fz_unit_rect;

/*
	A bounding box with sides of length one. See fz_unit_rect.
*/
extern const fz_bbox fz_unit_bbox;

/*
	An empty rectangle with an area equal to zero.

	Both the top left and bottom right corner are at (0, 0).
*/
extern const fz_rect fz_empty_rect;

/*
	An empty bounding box. See fz_empty_rect.
*/
extern const fz_bbox fz_empty_bbox;

/*
	An infinite rectangle with negative area.

	The corner (x0, y0) is at (1, 1) while the corner (x1, y1) is
	at (-1, -1).
*/
extern const fz_rect fz_infinite_rect;

/*
	An infinite bounding box. See fz_infinite_rect.
*/
extern const fz_bbox fz_infinite_bbox;

/*
	fz_is_empty_rect: Check if rectangle is empty.

	An empty rectangle is defined as one whose area is zero.
*/
#define fz_is_empty_rect(r) ((r).x0 == (r).x1 || (r).y0 == (r).y1)

/*
	fz_is_empty_bbox: Check if bounding box is empty.

	Same definition of empty bounding boxes as for empty
	rectangles. See fz_is_empty_rect.
*/
#define fz_is_empty_bbox(b) ((b).x0 == (b).x1 || (b).y0 == (b).y1)

/*
	fz_is_infinite: Check if rectangle is infinite.

	An infinite rectangle is defined as one where either of the
	two relationships between corner coordinates are not true.
*/
#define fz_is_infinite_rect(r) ((r).x0 > (r).x1 || (r).y0 > (r).y1)

/*
	fz_is_infinite_bbox: Check if bounding box is infinite.

	Same definition of infinite bounding boxes as for infinite
	rectangles. See fz_is_infinite_rect.
*/
#define fz_is_infinite_bbox(b) ((b).x0 > (b).x1 || (b).y0 > (b).y1)

/*
	fz_matrix is a a row-major 3x3 matrix used for representing
	transformations of coordinates throughout MuPDF.

	Since all points reside in a two-dimensional space, one vector
	is always a constant unit vector; hence only some elements may
	vary in a matrix. Below is how the elements map between
	different representations.

	/ a b 0 \
	| c d 0 | normally represented as [ a b c d e f ].
	\ e f 1 /
*/
typedef struct fz_matrix_s fz_matrix;
struct fz_matrix_s
{
	float a, b, c, d, e, f;
};


/*
	fz_identity: Identity transform matrix.
*/
extern const fz_matrix fz_identity;

/*
	fz_concat: Multiply two matrices.

	The order of the two matrices are important since matrix
	multiplication is not commutative.

	Does not throw exceptions.
*/
fz_matrix fz_concat(fz_matrix left, fz_matrix right);

/*
	fz_scale: Create a scaling matrix.

	The returned matrix is of the form [ sx 0 0 sy 0 0 ].

	sx, sy: Scaling factors along the X- and Y-axes. A scaling
	factor of 1.0 will not cause any scaling along the relevant
	axis.

	Does not throw exceptions.
*/
fz_matrix fz_scale(float sx, float sy);

/*
	fz_shear: Create a shearing matrix.

	The returned matrix is of the form [ 1 sy sx 1 0 0 ].

	sx, sy: Shearing factors. A shearing factor of 0.0 will not
	cause any shearing along the relevant axis.

	Does not throw exceptions.
*/
fz_matrix fz_shear(float sx, float sy);

/*
	fz_rotate: Create a rotation matrix.

	The returned matrix is of the form
	[ cos(deg) sin(deg) -sin(deg) cos(deg) 0 0 ].

	degrees: Degrees of counter clockwise rotation. Values less
	than zero and greater than 360 are handled as expected.

	Does not throw exceptions.
*/
fz_matrix fz_rotate(float degrees);

/*
	fz_translate: Create a translation matrix.

	The returned matrix is of the form [ 1 0 0 1 tx ty ].

	tx, ty: Translation distances along the X- and Y-axes. A
	translation of 0 will not cause any translation along the
	relevant axis.

	Does not throw exceptions.
*/
fz_matrix fz_translate(float tx, float ty);

/*
	fz_invert_matrix: Create an inverse matrix.

	matrix: Matrix to invert. A degenerate matrix, where the
	determinant is equal to zero, can not be inverted and the
	original matrix is returned instead.

	Does not throw exceptions.
*/
fz_matrix fz_invert_matrix(fz_matrix matrix);

/*
	fz_is_rectilinear: Check if a transformation is rectilinear.

	Rectilinear means that no shearing is present and that any
	rotations present are a multiple of 90 degrees. Usually this
	is used to make sure that axis-aligned rectangles before the
	transformation are still axis-aligned rectangles afterwards.

	Does not throw exceptions.
*/
int fz_is_rectilinear(fz_matrix m);

/*
	fz_matrix_expansion: Calculate average scaling factor of matrix.
*/
float fz_matrix_expansion(fz_matrix m); /* sumatrapdf */

/*
	fz_bbox_covering_rect: Convert a rect into the minimal bounding box
	that covers the rectangle.

	Coordinates in a bounding box are integers, so rounding of the
	rects coordinates takes place. The top left corner is rounded
	upwards and left while the bottom right corner is rounded
	downwards and to the right. Overflows or underflowing
	coordinates are clamped to INT_MIN/INT_MAX.

	Does not throw exceptions.
*/
fz_bbox fz_bbox_covering_rect(fz_rect rect);

/*
	fz_round_rect: Convert a rect into a bounding box.

	Coordinates in a bounding box are integers, so rounding of the
	rects coordinates takes place. The top left corner is rounded
	upwards and left while the bottom right corner is rounded
	downwards and to the right. Overflows or underflowing
	coordinates are clamped to INT_MIN/INT_MAX.

	This differs from fz_bbox_covering_rect, in that fz_bbox_covering_rect
	slavishly follows the numbers (i.e any slight over/under calculations
	can cause whole extra pixels to be added). fz_round_rect
	allows for a small amount of rounding error when calculating
	the bbox.

	Does not throw exceptions.
*/
fz_bbox fz_round_rect(fz_rect rect);

/*
	fz_intersect_rect: Compute intersection of two rectangles.

	Compute the largest axis-aligned rectangle that covers the
	area covered by both given rectangles. If either rectangle is
	empty then the intersection is also empty. If either rectangle
	is infinite then the intersection is simply the non-infinite
	rectangle. Should both rectangles be infinite, then the
	intersection is also infinite.

	Does not throw exceptions.
*/
fz_rect fz_intersect_rect(fz_rect a, fz_rect b);

/*
	fz_intersect_bbox: Compute intersection of two bounding boxes.

	Similar to fz_intersect_rect but operates on two bounding
	boxes instead of two rectangles.

	Does not throw exceptions.
*/
fz_bbox fz_intersect_bbox(fz_bbox a, fz_bbox b);

/*
	fz_union_rect: Compute union of two rectangles.

	Compute the smallest axis-aligned rectangle that encompasses
	both given rectangles. If either rectangle is infinite then
	the union is also infinite. If either rectangle is empty then
	the union is simply the non-empty rectangle. Should both
	rectangles be empty, then the union is also empty.

	Does not throw exceptions.
*/
fz_rect fz_union_rect(fz_rect a, fz_rect b);

/*
	fz_union_bbox: Compute union of two bounding boxes.

	Similar to fz_union_rect but operates on two bounding boxes
	instead of two rectangles.

	Does not throw exceptions.
*/
fz_bbox fz_union_bbox(fz_bbox a, fz_bbox b);

/*
	fz_transform_point: Apply a transformation to a point.

	transform: Transformation matrix to apply. See fz_concat,
	fz_scale, fz_rotate and fz_translate for how to create a
	matrix.

	Does not throw exceptions.
*/
fz_point fz_transform_point(fz_matrix transform, fz_point point);

/*
	fz_transform_vector: Apply a transformation to a vector.

	transform: Transformation matrix to apply. See fz_concat,
	fz_scale and fz_rotate for how to create a matrix. Any
	translation will be ignored.

	Does not throw exceptions.
*/
fz_point fz_transform_vector(fz_matrix transform, fz_point vector);

/*
	fz_transform_rect: Apply a transform to a rectangle.

	After the four corner points of the axis-aligned rectangle
	have been transformed it may not longer be axis-aligned. So a
	new axis-aligned rectangle is created covering at least the
	area of the transformed rectangle.

	transform: Transformation matrix to apply. See fz_concat,
	fz_scale and fz_rotate for how to create a matrix.

	rect: Rectangle to be transformed. The two special cases
	fz_empty_rect and fz_infinite_rect, may be used but are
	returned unchanged as expected.

	Does not throw exceptions.
*/
fz_rect fz_transform_rect(fz_matrix transform, fz_rect rect);

/*
	fz_transform_bbox: Transform a given bounding box.

	Similar to fz_transform_rect, but operates on a bounding box
	instead of a rectangle.

	Does not throw exceptions.
*/
fz_bbox fz_transform_bbox(fz_matrix matrix, fz_bbox bbox);

/*
	fz_buffer is a wrapper around a dynamically allocated array of bytes.

	Buffers have a capacity (the number of bytes storage immediately
	available) and a current size.
*/
typedef struct fz_buffer_s fz_buffer;

/*
	fz_keep_buffer: Increment the reference count for a buffer.

	buf: The buffer to increment the reference count for.

	Returns a pointer to the buffer. Does not throw exceptions.
*/
fz_buffer *fz_keep_buffer(fz_context *ctx, fz_buffer *buf);

/*
	fz_drop_buffer: Decrement the reference count for a buffer.

	buf: The buffer to decrement the reference count for.
*/
void fz_drop_buffer(fz_context *ctx, fz_buffer *buf);

/*
	fz_buffer_storage: Retrieve information on the storage currently used
	by a buffer.

	data: Pointer to place to retrieve data pointer.

	Returns length of stream.
*/
int fz_buffer_storage(fz_context *ctx, fz_buffer *buf, unsigned char **data);

/*
	fz_stream is a buffered reader capable of seeking in both
	directions.

	Streams are reference counted, so references must be dropped
	by a call to fz_close.

	Only the data between rp and wp is valid.
*/
typedef struct fz_stream_s fz_stream;

/*
	fz_open_file: Open the named file and wrap it in a stream.

	filename: Path to a file. On non-Windows machines the filename should
	be exactly as it would be passed to open(2). On Windows machines, the
	path should be UTF-8 encoded so that non-ASCII characters can be
	represented. Other platforms do the encoding as standard anyway (and
	in most cases, particularly for MacOS and Linux, the encoding they
	use is UTF-8 anyway).
*/
fz_stream *fz_open_file(fz_context *ctx, const char *filename);

/*
	fz_open_file_w: Open the named file and wrap it in a stream.

	This function is only available when compiling for Win32.

	filename: Wide character path to the file as it would be given
	to _wopen().
*/
fz_stream *fz_open_file_w(fz_context *ctx, const wchar_t *filename);

/*
	fz_open_fd: Wrap an open file descriptor in a stream.

	file: An open file descriptor supporting bidirectional
	seeking. The stream will take ownership of the file
	descriptor, so it may not be modified or closed after the call
	to fz_open_fd. When the stream is closed it will also close
	the file descriptor.
*/
fz_stream *fz_open_fd(fz_context *ctx, int file);

/*
	fz_open_memory: Open a block of memory as a stream.

	data: Pointer to start of data block. Ownership of the data block is
	NOT passed in.

	len: Number of bytes in data block.

	Returns pointer to newly created stream. May throw exceptions on
	failure to allocate.
*/
fz_stream *fz_open_memory(fz_context *ctx, unsigned char *data, int len);

/*
	fz_open_buffer: Open a buffer as a stream.

	buf: The buffer to open. Ownership of the buffer is NOT passed in
	(this function takes it's own reference).

	Returns pointer to newly created stream. May throw exceptions on
	failure to allocate.
*/
fz_stream *fz_open_buffer(fz_context *ctx, fz_buffer *buf);

/*
	fz_close: Close an open stream.

	Drops a reference for the stream. Once no references remain
	the stream will be closed, as will any file descriptor the
	stream is using.

	Does not throw exceptions.
*/
void fz_close(fz_stream *stm);

/*
	fz_tell: return the current reading position within a stream
*/
int fz_tell(fz_stream *stm);

/*
	fz_seek: Seek within a stream.

	stm: The stream to seek within.

	offset: The offset to seek to.

	whence: From where the offset is measured (see fseek).
*/
void fz_seek(fz_stream *stm, int offset, int whence);

/*
	fz_read: Read from a stream into a given data block.

	stm: The stream to read from.

	data: The data block to read into.

	len: The length of the data block (in bytes).

	Returns the number of bytes read. May throw exceptions.
*/
int fz_read(fz_stream *stm, unsigned char *data, int len);

/*
	fz_read_all: Read all of a stream into a buffer.

	stm: The stream to read from

	initial: Suggested initial size for the buffer.

	Returns a buffer created from reading from the stream. May throw
	exceptions on failure to allocate.
*/
fz_buffer *fz_read_all(fz_stream *stm, int initial);

/*
	Bitmaps have 1 bit per component. Only used for creating halftoned
	versions of contone buffers, and saving out. Samples are stored msb
	first, akin to pbms.
*/
typedef struct fz_bitmap_s fz_bitmap;

/*
	fz_keep_bitmap: Take a reference to a bitmap.

	bit: The bitmap to increment the reference for.

	Returns bit. Does not throw exceptions.
*/
fz_bitmap *fz_keep_bitmap(fz_context *ctx, fz_bitmap *bit);

/*
	fz_drop_bitmap: Drop a reference and free a bitmap.

	Decrement the reference count for the bitmap. When no
	references remain the pixmap will be freed.

	Does not throw exceptions.
*/
void fz_drop_bitmap(fz_context *ctx, fz_bitmap *bit);

/*
	An fz_colorspace object represents an abstract colorspace. While
	this should be treated as a black box by callers of the library at
	this stage, know that it encapsulates knowledge of how to convert
	colors to and from the colorspace, any lookup tables generated, the
	number of components in the colorspace etc.
*/
typedef struct fz_colorspace_s fz_colorspace;

/*
	fz_find_device_colorspace: Find a standard colorspace based upon
	it's name.
*/
fz_colorspace *fz_find_device_colorspace(fz_context *ctx, char *name);

/*
	fz_device_gray: Abstract colorspace representing device specific
	gray.
*/
extern fz_colorspace *fz_device_gray;

/*
	fz_device_rgb: Abstract colorspace representing device specific
	rgb.
*/
extern fz_colorspace *fz_device_rgb;

/*
	fz_device_bgr: Abstract colorspace representing device specific
	bgr.
*/
extern fz_colorspace *fz_device_bgr;

/*
	fz_device_cmyk: Abstract colorspace representing device specific
	CMYK.
*/
extern fz_colorspace *fz_device_cmyk;

/*
	Pixmaps represent a set of pixels for a 2 dimensional region of a
	plane. Each pixel has n components per pixel, the last of which is
	always alpha. The data is in premultiplied alpha when rendering, but
	non-premultiplied for colorspace conversions and rescaling.
*/
typedef struct fz_pixmap_s fz_pixmap;

/*
	fz_pixmap_bbox: Return a bounding box for a pixmap.

	Returns an exact bounding box for the supplied pixmap.
*/
fz_bbox fz_pixmap_bbox(fz_context *ctx, fz_pixmap *pix);

/*
	fz_pixmap_width: Return the width of the pixmap in pixels.
*/
int fz_pixmap_width(fz_context *ctx, fz_pixmap *pix);

/*
	fz_pixmap_height: Return the height of the pixmap in pixels.
*/
int fz_pixmap_height(fz_context *ctx, fz_pixmap *pix);

/*
	fz_new_pixmap: Create a new pixmap, with it's origin at (0,0)

	cs: The colorspace to use for the pixmap, or NULL for an alpha
	plane/mask.

	w: The width of the pixmap (in pixels)

	h: The height of the pixmap (in pixels)

	Returns a pointer to the new pixmap. Throws exception on failure to
	allocate.
*/
fz_pixmap *fz_new_pixmap(fz_context *ctx, fz_colorspace *cs, int w, int h);

/*
	fz_new_pixmap_with_bbox: Create a pixmap of a given size,
	location and pixel format.

	The bounding box specifies the size of the created pixmap and
	where it will be located. The colorspace determines the number
	of components per pixel. Alpha is always present. Pixmaps are
	reference counted, so drop references using fz_drop_pixmap.

	colorspace: Colorspace format used for the created pixmap. The
	pixmap will keep a reference to the colorspace.

	bbox: Bounding box specifying location/size of created pixmap.

	Returns a pointer to the new pixmap. Throws exception on failure to
	allocate.
*/
fz_pixmap *fz_new_pixmap_with_bbox(fz_context *ctx, fz_colorspace *colorspace, fz_bbox bbox);

/*
	fz_new_pixmap_with_data: Create a new pixmap, with it's origin at
	(0,0) using the supplied data block.

	cs: The colorspace to use for the pixmap, or NULL for an alpha
	plane/mask.

	w: The width of the pixmap (in pixels)

	h: The height of the pixmap (in pixels)

	samples: The data block to keep the samples in.

	Returns a pointer to the new pixmap. Throws exception on failure to
	allocate.
*/
fz_pixmap *fz_new_pixmap_with_data(fz_context *ctx, fz_colorspace *colorspace, int w, int h, unsigned char *samples);

/*
	fz_new_pixmap_with_bbox_and_data: Create a pixmap of a given size,
	location and pixel format, using the supplied data block.

	The bounding box specifies the size of the created pixmap and
	where it will be located. The colorspace determines the number
	of components per pixel. Alpha is always present. Pixmaps are
	reference counted, so drop references using fz_drop_pixmap.

	colorspace: Colorspace format used for the created pixmap. The
	pixmap will keep a reference to the colorspace.

	bbox: Bounding box specifying location/size of created pixmap.

	samples: The data block to keep the samples in.

	Returns a pointer to the new pixmap. Throws exception on failure to
	allocate.
*/
fz_pixmap *fz_new_pixmap_with_bbox_and_data(fz_context *ctx, fz_colorspace *colorspace, fz_bbox bbox, unsigned char *samples);

/*
	fz_keep_pixmap: Take a reference to a pixmap.

	pix: The pixmap to increment the reference for.

	Returns pix. Does not throw exceptions.
*/
fz_pixmap *fz_keep_pixmap(fz_context *ctx, fz_pixmap *pix);

/*
	fz_drop_pixmap: Drop a reference and free a pixmap.

	Decrement the reference count for the pixmap. When no
	references remain the pixmap will be freed.

	Does not throw exceptions.
*/
void fz_drop_pixmap(fz_context *ctx, fz_pixmap *pix);

/*
	fz_pixmap_colorspace: Return the colorspace of a pixmap

	Returns colorspace. Does not throw exceptions.
*/
fz_colorspace *fz_pixmap_colorspace(fz_context *ctx, fz_pixmap *pix);

/*
	fz_pixmap_components: Return the number of components in a pixmap.

	Returns the number of components. Does not throw exceptions.
*/
int fz_pixmap_components(fz_context *ctx, fz_pixmap *pix);

/*
	fz_pixmap_samples: Returns a pointer to the pixel data of a pixmap.

	Returns the pointer. Does not throw exceptions.
*/
unsigned char *fz_pixmap_samples(fz_context *ctx, fz_pixmap *pix);

/*
	fz_clear_pixmap_with_value: Clears a pixmap with the given value.

	pix: The pixmap to clear.

	value: Values in the range 0 to 255 are valid. Each component
	sample for each pixel in the pixmap will be set to this value,
	while alpha will always be set to 255 (non-transparent).

	Does not throw exceptions.
*/
void fz_clear_pixmap_with_value(fz_context *ctx, fz_pixmap *pix, int value);

/*
	fz_clear_pixmap_with_value: Sets all components (including alpha) of
	all pixels in a pixmap to 0.

	pix: The pixmap to clear.

	Does not throw exceptions.
*/
void fz_clear_pixmap(fz_context *ctx, fz_pixmap *pix);

/*
	fz_invert_pixmap: Invert all the pixels in a pixmap. All components
	of all pixels are inverted (except alpha, which is unchanged).

	Does not throw exceptions.
*/
void fz_invert_pixmap(fz_context *ctx, fz_pixmap *pix);

/*
	fz_invert_pixmap: Invert all the pixels in a given rectangle of a
	pixmap. All components of all pixels in the rectangle are inverted
	(except alpha, which is unchanged).

	Does not throw exceptions.
*/
void fz_invert_pixmap_rect(fz_pixmap *image, fz_bbox rect);

/*
	fz_gamma_pixmap: Apply gamma correction to a pixmap. All components
	of all pixels are modified (except alpha, which is unchanged).

	gamma: The gamma value to apply; 1.0 for no change.

	Does not throw exceptions.
*/
void fz_gamma_pixmap(fz_context *ctx, fz_pixmap *pix, float gamma);

/*
	fz_unmultiply_pixmap: Convert a pixmap from premultiplied to
	non-premultiplied format.

	Does not throw exceptions.
*/
void fz_unmultiply_pixmap(fz_context *ctx, fz_pixmap *pix);

/*
	fz_convert_pixmap: Convert from one pixmap to another (assumed to be
	the same size, but possibly with a different colorspace).

	dst: the source pixmap.

	src: the destination pixmap.
*/
void fz_convert_pixmap(fz_context *ctx, fz_pixmap *dst, fz_pixmap *src);

/*
	fz_write_pixmap: Save a pixmap out.

	name: The prefix for the name of the pixmap. The pixmap will be saved
	as "name.png" if the pixmap is RGB or Greyscale, "name.pam" otherwise.

	rgb: If non zero, the pixmap is converted to rgb (if possible) before
	saving.
*/
void fz_write_pixmap(fz_context *ctx, fz_pixmap *img, char *name, int rgb);

/*
	fz_write_pnm: Save a pixmap as a pnm

	filename: The filename to save as (including extension).
*/
void fz_write_pnm(fz_context *ctx, fz_pixmap *pixmap, char *filename);

/*
	fz_write_pam: Save a pixmap as a pam

	filename: The filename to save as (including extension).
*/
void fz_write_pam(fz_context *ctx, fz_pixmap *pixmap, char *filename, int savealpha);

/*
	fz_write_png: Save a pixmap as a png

	filename: The filename to save as (including extension).
*/
void fz_write_png(fz_context *ctx, fz_pixmap *pixmap, char *filename, int savealpha);

/*
	fz_write_pbm: Save a bitmap as a pbm

	filename: The filename to save as (including extension).
*/
void fz_write_pbm(fz_context *ctx, fz_bitmap *bitmap, char *filename);

/*
	fz_md5_pixmap: Return the md5 digest for a pixmap

	filename: The filename to save as (including extension).
*/
void fz_md5_pixmap(fz_pixmap *pixmap, unsigned char digest[16]);

/*
	Images are storable objects from which we can obtain fz_pixmaps.
	These may be implemented as simple wrappers around a pixmap, or as
	more complex things that decode at different subsample settings on
	demand.
*/
typedef struct fz_image_s fz_image;

/*
	fz_image_to_pixmap: Called to get a handle to a pixmap from an image.

	image: The image to retrieve a pixmap from.

	w: The desired width (in pixels). This may be completely ignored, but
	may serve as an indication of a suitable subsample factor to use for
	image types that support this.

	h: The desired height (in pixels). This may be completely ignored, but
	may serve as an indication of a suitable subsample factor to use for
	image types that support this.

	Returns a non NULL pixmap pointer. May throw exceptions.
*/
fz_pixmap *fz_image_to_pixmap(fz_context *ctx, fz_image *image, int w, int h);

/*
	fz_drop_image: Drop a reference to an image.

	image: The image to drop a reference to.
*/
void fz_drop_image(fz_context *ctx, fz_image *image);

/*
	fz_keep_image: Increment the reference count of an image.

	image: The image to take a reference to.

	Returns a pointer to the image.
*/
fz_image *fz_keep_image(fz_context *ctx, fz_image *image);

/*
	A halftone is a set of threshold tiles, one per component. Each
	threshold tile is a pixmap, possibly of varying sizes and phases.
	Currently, we only provide one 'default' halftone tile for operating
	on 1 component plus alpha pixmaps (where the alpha is ignored). This
	is signified by an fz_halftone pointer to NULL.
*/
typedef struct fz_halftone_s fz_halftone;

/*
	fz_halftone_pixmap: Make a bitmap from a pixmap and a halftone.

	pix: The pixmap to generate from. Currently must be a single color
	component + alpha (where the alpha is assumed to be solid).

	ht: The halftone to use. NULL implies the default halftone.

	Returns the resultant bitmap. Throws exceptions in the case of
	failure to allocate.
*/
fz_bitmap *fz_halftone_pixmap(fz_context *ctx, fz_pixmap *pix, fz_halftone *ht);

/*
	An abstract font handle. Currently there are no public API functions
	for handling these.
*/
typedef struct fz_font_s fz_font;

/*
	The different format handlers (pdf, xps etc) interpret pages to a
	device. These devices can then process the stream of calls they
	recieve in various ways:
		The trace device outputs debugging information for the calls.
		The draw device will render them.
		The list device stores them in a list to play back later.
		The text device performs text extraction and searching.
		The bbox device calculates the bounding box for the page.
	Other devices can (and will) be written in future.
*/
typedef struct fz_device_s fz_device;

/*
	fz_free_device: Free a devices of any type and its resources.
*/
void fz_free_device(fz_device *dev);

/*
	fz_new_trace_device: Create a device to print a debug trace of
	all device calls.
*/
fz_device *fz_new_trace_device(fz_context *ctx);

/*
	fz_new_bbox_device: Create a device to compute the bounding
	box of all marks on a page.

	The returned bounding box will be the union of all bounding
	boxes of all objects on a page.
*/
fz_device *fz_new_bbox_device(fz_context *ctx, fz_bbox *bboxp);

/*
	fz_new_draw_device: Create a device to draw on a pixmap.

	dest: Target pixmap for the draw device. See fz_new_pixmap*
	for how to obtain a pixmap. The pixmap is not cleared by the
	draw device, see fz_clear_pixmap* for how to clear it prior to
	calling fz_new_draw_device. Free the device by calling
	fz_free_device.
*/
fz_device *fz_new_draw_device(fz_context *ctx, fz_pixmap *dest);

/*
	fz_new_draw_device_with_bbox: Create a device to draw on a pixmap.

	dest: Target pixmap for the draw device. See fz_new_pixmap*
	for how to obtain a pixmap. The pixmap is not cleared by the
	draw device, see fz_clear_pixmap* for how to clear it prior to
	calling fz_new_draw_device. Free the device by calling
	fz_free_device.

	clip: Bounding box to restrict any marking operations of the
	draw device.
*/
fz_device *fz_new_draw_device_with_bbox(fz_context *ctx, fz_pixmap *dest, fz_bbox clip);

/*
	Text extraction device: Used for searching, format conversion etc.

	(In development - Subject to change in future versions)
*/

typedef struct fz_text_style_s fz_text_style;
typedef struct fz_text_char_s fz_text_char;
typedef struct fz_text_span_s fz_text_span;
typedef struct fz_text_line_s fz_text_line;
typedef struct fz_text_block_s fz_text_block;

typedef struct fz_text_sheet_s fz_text_sheet;
typedef struct fz_text_page_s fz_text_page;

/*
	fz_text_sheet: A text sheet contains a list of distinct text styles
	used on a page (or a series of pages).
*/
struct fz_text_sheet_s
{
	int maxid;
	fz_text_style *style;
};

/*
	fz_text_style: A text style contains details of a distinct text style
	used on a page.
*/
struct fz_text_style_s
{
	fz_text_style *next;
	int id;
	fz_font *font;
	float size;
	int wmode;
	int script;
	/* etc... */
};

/*
	fz_text_page: A text page is a list of blocks of text, together with
	an overall bounding box.
*/
struct fz_text_page_s
{
	fz_rect mediabox;
	int len, cap;
	fz_text_block *blocks;
};

/*
	fz_text_block: A text block is a list of lines of text. In typical
	cases this may correspond to a paragraph or a column of text. A
	collection of blocks makes up a page.
*/
struct fz_text_block_s
{
	fz_rect bbox;
	int len, cap;
	fz_text_line *lines;
};

/*
	fz_text_line: A text line is a list of text spans, with the same
	(or very similar) baseline. In typical cases this should correspond
	(as expected) to complete lines of text. A collection of lines makes
	up a block.
*/
struct fz_text_line_s
{
	fz_rect bbox;
	int len, cap;
	fz_text_span *spans;
};

/*
	fz_text_span: A text span is a list of characters in the same style
	that share a common (or very similar) baseline. In typical cases
	(where only one font style is used in a line), a single span may be
	enough to represent a complete line. In cases where multiple
	font styles are used (for example italics), then a line will be
	broken down into a series of spans.
*/
struct fz_text_span_s
{
	fz_rect bbox;
	int len, cap;
	fz_text_char *text;
	fz_text_style *style;
};

/*
	fz_text_char: A text char is a unicode character and the bounding
	box with which it appears on the page.
*/
struct fz_text_char_s
{
	fz_rect bbox;
	int c;
};

/*
	fz_new_text_device: Create a device to extract the text on a page.

	Gather and sort the text on a page into spans of uniform style,
	arranged into lines and blocks by reading order. The reading order
	is determined by various heuristics, so may not be accurate.

	sheet: The text sheet to which styles should be added. This can
	either be a newly created (empty) text sheet, or one containing
	styles from a previous text device. The same sheet cannot be used
	in multiple threads simultaneously.

	page: The text page to which content should be added. This will
	usually be a newly created (empty) text page, but it can be one
	containing data already (for example when merging multiple pages, or
	watermarking).
*/
fz_device *fz_new_text_device(fz_context *ctx, fz_text_sheet *sheet, fz_text_page *page);

/*
	fz_new_text_sheet: Create an empty style sheet.

	The style sheet is filled out by the text device, creating
	one style for each unique font, color, size combination that
	is used.
*/
fz_text_sheet *fz_new_text_sheet(fz_context *ctx);
void fz_free_text_sheet(fz_context *ctx, fz_text_sheet *sheet);

/*
	fz_new_text_page: Create an empty text page.

	The text page is filled out by the text device to contain the blocks,
	lines and spans of text on the page.
*/
fz_text_page *fz_new_text_page(fz_context *ctx, fz_rect mediabox);
void fz_free_text_page(fz_context *ctx, fz_text_page *page);

/*
	fz_print_text_sheet: Output a text sheet to a file as CSS.
*/
void fz_print_text_sheet(fz_context *ctx, FILE *out, fz_text_sheet *sheet);

/*
	fz_print_text_page_html: Output a page to a file in HTML format.
*/
void fz_print_text_page_html(fz_context *ctx, FILE *out, fz_text_page *page);

/*
	fz_print_text_page_xml: Output a page to a file in XML format.
*/
void fz_print_text_page_xml(fz_context *ctx, FILE *out, fz_text_page *page);

/*
	fz_print_text_page: Output a page to a file in UTF-8 format.
*/
void fz_print_text_page(fz_context *ctx, FILE *out, fz_text_page *page);

/*
	Cookie support - simple communication channel between app/library.
*/

typedef struct fz_cookie_s fz_cookie;

/*
	Provide two-way communication between application and library.
	Intended for multi-threaded applications where one thread is
	rendering pages and another thread wants read progress
	feedback or abort a job that takes a long time to finish. The
	communication is unsynchronized without locking.

	abort: The appliation should set this field to 0 before
	calling fz_run_page to render a page. At any point when the
	page is being rendered the application my set this field to 1
	which will cause the rendering to finish soon. This field is
	checked periodically when the page is rendered, but exactly
	when is not known, therefore there is no upper bound on
	exactly when the the rendering will abort. If the application
	did not provide a set of locks to fz_new_context, it must also
	await the completion of fz_run_page before issuing another
	call to fz_run_page. Note that once the application has set
	this field to 1 after it called fz_run_page it may not change
	the value again.

	progress: Communicates rendering progress back to the
	application and is read only. Increments as a page is being
	rendered. The value starts out at 0 and is limited to less
	than or equal to progress_max, unless progress_max is -1.

	progress_max: Communicates the known upper bound of rendering
	back to the application and is read only. The maximum value
	that the progress field may take. If there is no known upper
	bound on how long the rendering may take this value is -1 and
	progress is not limited. Note that the value of progress_max
	may change from -1 to a positive value once an upper bound is
	known, so take this into consideration when comparing the
	value of progress to that of progress_max.

	errors: count of errors during current rendering.
*/
struct fz_cookie_s
{
	int abort;
	int progress;
	int progress_max; /* -1 for unknown */
	int errors;
};

/*
	Display list device -- record and play back device commands.
*/

/*
	fz_display_list is a list containing drawing commands (text,
	images, etc.). The intent is two-fold: as a caching-mechanism
	to reduce parsing of a page, and to be used as a data
	structure in multi-threading where one thread parses the page
	and another renders pages.

	Create a displaylist with fz_new_display_list, hand it over to
	fz_new_list_device to have it populated, and later replay the
	list (once or many times) by calling fz_run_display_list. When
	the list is no longer needed free it with fz_free_display_list.
*/
typedef struct fz_display_list_s fz_display_list;

/*
	fz_new_display_list: Create an empty display list.

	A display list contains drawing commands (text, images, etc.).
	Use fz_new_list_device for populating the list.
*/
fz_display_list *fz_new_display_list(fz_context *ctx);

/*
	fz_new_list_device: Create a rendering device for a display list.

	When the device is rendering a page it will populate the
	display list with drawing commsnds (text, images, etc.). The
	display list can later be reused to render a page many times
	without having to re-interpret the page from the document file
	for each rendering. Once the device is no longer needed, free
	it with fz_free_device.

	list: A display list that the list device takes ownership of.
*/
fz_device *fz_new_list_device(fz_context *ctx, fz_display_list *list);

/*
	fz_run_display_list: (Re)-run a display list through a device.

	list: A display list, created by fz_new_display_list and
	populated with objects from a page by running fz_run_page on a
	device obtained from fz_new_list_device.

	dev: Device obtained from fz_new_*_device.

	ctm: Transform to apply to display list contents. May include
	for example scaling and rotation, see fz_scale, fz_rotate and
	fz_concat. Set to fz_identity if no transformation is desired.

	area: Only the part of the contents of the display list
	visible within this area will be considered when the list is
	run through the device. This does not imply for tile objects
	contained in the display list.

	cookie: Communication mechanism between caller and library
	running the page. Intended for multi-threaded applications,
	while single-threaded applications set cookie to NULL. The
	caller may abort an ongoing page run. Cookie also communicates
	progress information back to the caller. The fields inside
	cookie are continually updated while the page is being run.
*/
void fz_run_display_list(fz_display_list *list, fz_device *dev, fz_matrix ctm, fz_bbox area, fz_cookie *cookie);

/*
	fz_free_display_list: Frees a display list.

	list: Display list to be freed. Any objects put into the
	display list by a list device will also be freed.

	Does not throw exceptions.
*/
void fz_free_display_list(fz_context *ctx, fz_display_list *list);

/*
	Links

	NOTE: The link destination struct is scheduled for imminent change!
	Use at your own peril.
*/

typedef struct fz_link_s fz_link;

typedef struct fz_link_dest_s fz_link_dest;

typedef enum fz_link_kind_e
{
	FZ_LINK_NONE = 0,
	FZ_LINK_GOTO,
	FZ_LINK_URI,
	FZ_LINK_LAUNCH,
	FZ_LINK_NAMED,
	FZ_LINK_GOTOR
} fz_link_kind;

enum {
	fz_link_flag_l_valid = 1, /* lt.x is valid */
	fz_link_flag_t_valid = 2, /* lt.y is valid */
	fz_link_flag_r_valid = 4, /* rb.x is valid */
	fz_link_flag_b_valid = 8, /* rb.y is valid */
	fz_link_flag_fit_h = 16, /* Fit horizontally */
	fz_link_flag_fit_v = 32, /* Fit vertically */
	fz_link_flag_r_is_zoom = 64 /* rb.x is actually a zoom figure */
};

/*
	fz_link_dest: This structure represents the destination of
	an fz_link; this may be a page to display, a new file to open,
	a javascript action to perform, etc.

	kind: This identifies the kind of link destination. Different
	kinds use different sections of the union.

	For FZ_LINK_GOTO or FZ_LINK_GOTOR:

		gotor.page: The target page number to move to (0 being the
		first page in the document).

		gotor.flags: A bitfield consisting of fz_link_flag_*
		describing the validity and meaning of the different parts
		of gotor.lr and gotor.rb. Link destinations are constructed
		(as far as possible) so that lt and rb can be treated as a
		bounding box, though the validity flags indicate which of the
		values was actually specified in the file.

		gotor.lt: The top left corner of the destination bounding box.

		gotor.rb: The bottom right corner of the destination bounding
		box. If fz_link_flag_r_is_zoom is set, then the r figure
		should actually be interpretted as a zoom ratio.

		gotor.file_spec: If set, this destination should cause a new
		file to be opened; this field holds a pointer to a remote
		file specification (UTF-8). Always NULL in the FZ_LINK_GOTO
		case.

		gotor.new_window: If true, the destination should open in a
		new window.

	For FZ_LINK_URI:

		uri.uri: A UTF-8 encoded URI to launch.

		uri.is_map: If true, the x and y coords (as ints, in user
		space) should be appended to the URI before launch.

	For FZ_LINK_LAUNCH:

		launch.file_spec: A UTF-8 file specification to launch.

		launch.new_window: If true, the destination should be launched
		in a new window.

	For FZ_LINK_NAMED:

		named.named: The named action to perform. Likely to be
		client specific.
*/
struct fz_link_dest_s
{
	fz_link_kind kind;
	union
	{
		struct
		{
			int page;
			int flags;
			fz_point lt;
			fz_point rb;
			char *file_spec;
			int new_window;
		}
		gotor;
		struct
		{
			char *uri;
			int is_map;
		}
		uri;
		struct
		{
			char *file_spec;
			int new_window;
		}
		launch;
		struct
		{
			char *named;
		}
		named;
	}
	ld;
};

/*
	fz_link is a list of interactive links on a page.

	There is no relation between the order of the links in the
	list and the order they appear on the page. The list of links
	for a given page can be obtained from fz_load_links.

	A link is reference counted. Dropping a reference to a link is
	done by calling fz_drop_link.

	rect: The hot zone. The area that can be clicked in
	untransformed coordinates.

	dest: Link destinations come in two forms: Page and area that
	an application should display when this link is activated. Or
	as an URI that can be given to a browser.

	next: A pointer to the next link on the same page.
*/
struct fz_link_s
{
	int refs;
	fz_rect rect;
	fz_link_dest dest;
	fz_link *next;
};

fz_link *fz_new_link(fz_context *ctx, fz_rect bbox, fz_link_dest dest);
fz_link *fz_keep_link(fz_context *ctx, fz_link *link);

/*
	fz_drop_link: Drop and free a list of links.

	Does not throw exceptions.
*/
void fz_drop_link(fz_context *ctx, fz_link *link);

void fz_free_link_dest(fz_context *ctx, fz_link_dest *dest);

/* Outline */

typedef struct fz_outline_s fz_outline;

/*
	fz_outline is a tree of the outline of a document (also known
	as table of contents).

	title: Title of outline item using UTF-8 encoding. May be NULL
	if the outline item has no text string.

	dest: Destination in the document to be displayed when this
	outline item is activated. May be FZ_LINK_NONE if the outline
	item does not have a destination.

	next: The next outline item at the same level as this outline
	item. May be NULL if no more outline items exist at this level.

	down: The outline items immediate children in the hierarchy.
	May be NULL if no children exist.
*/
struct fz_outline_s
{
	char *title;
	fz_link_dest dest;
	fz_outline *next;
	fz_outline *down;
};

/*
	fz_print_outline_xml: Dump the given outlines as (pseudo) XML.

	out: The file handle to output to.

	outline: The outlines to output.
*/
void fz_print_outline_xml(fz_context *ctx, FILE *out, fz_outline *outline);

/*
	fz_print_outline: Dump the given outlines to as text.

	out: The file handle to output to.

	outline: The outlines to output.
*/
void fz_print_outline(fz_context *ctx, FILE *out, fz_outline *outline);

/*
	fz_free_outline: Free hierarchical outline.

	Free an outline obtained from fz_load_outline.

	Does not throw exceptions.
*/
void fz_free_outline(fz_context *ctx, fz_outline *outline);

/*
	Document interface
*/
typedef struct fz_document_s fz_document;
typedef struct fz_page_s fz_page;

/*
	fz_open_document: Open a PDF, XPS or CBZ document.

	Open a document file and read its basic structure so pages and
	objects can be located. MuPDF will try to repair broken
	documents (without actually changing the file contents).

	The returned fz_document is used when calling most other
	document related functions. Note that it wraps the context, so
	those functions implicitly can access the global state in
	context.

	filename: a path to a file as it would be given to open(2).
*/
fz_document *fz_open_document(fz_context *ctx, char *filename);

/*
	fz_close_document: Close and free an open document.

	The resource store in the context associated with fz_document
	is emptied, and any allocations for the document are freed.

	Does not throw exceptions.
*/
void fz_close_document(fz_document *doc);

/*
	fz_needs_password: Check if a document is encrypted with a
	non-blank password.

	Does not throw exceptions.
*/
int fz_needs_password(fz_document *doc);

/*
	fz_authenticate_password: Test if the given password can
	decrypt the document.

	password: The password string to be checked. Some document
	specifications do not specify any particular text encoding, so
	neither do we.

	Does not throw exceptions.
*/
int fz_authenticate_password(fz_document *doc, char *password);

/*
	fz_load_outline: Load the hierarchical document outline.

	Should be freed by fz_free_outline.
*/
fz_outline *fz_load_outline(fz_document *doc);

/*
	fz_count_pages: Return the number of pages in document

	May return 0 for documents with no pages.
*/
int fz_count_pages(fz_document *doc);

/*
	fz_load_page: Load a page.

	After fz_load_page is it possible to retrieve the size of the
	page using fz_bound_page, or to render the page using
	fz_run_page_*. Free the page by calling fz_free_page.

	number: page number, 0 is the first page of the document.
*/
fz_page *fz_load_page(fz_document *doc, int number);

/*
	fz_load_links: Load the list of links for a page.

	Returns a linked list of all the links on the page, each with
	its clickable region and link destination. Each link is
	reference counted so drop and free the list of links by
	calling fz_drop_link on the pointer return from fz_load_links.

	page: Page obtained from fz_load_page.
*/
fz_link *fz_load_links(fz_document *doc, fz_page *page);

/*
	fz_bound_page: Determine the size of a page at 72 dpi.

	Does not throw exceptions.
*/
fz_rect fz_bound_page(fz_document *doc, fz_page *page);

/*
	fz_run_page: Run a page through a device.

	page: Page obtained from fz_load_page.

	dev: Device obtained from fz_new_*_device.

	transform: Transform to apply to page. May include for example
	scaling and rotation, see fz_scale, fz_rotate and fz_concat.
	Set to fz_identity if no transformation is desired.

	cookie: Communication mechanism between caller and library
	rendering the page. Intended for multi-threaded applications,
	while single-threaded applications set cookie to NULL. The
	caller may abort an ongoing rendering of a page. Cookie also
	communicates progress information back to the caller. The
	fields inside cookie are continually updated while the page is
	rendering.
*/
void fz_run_page(fz_document *doc, fz_page *page, fz_device *dev, fz_matrix transform, fz_cookie *cookie);

/*
	fz_free_page: Free a loaded page.

	Does not throw exceptions.
*/
void fz_free_page(fz_document *doc, fz_page *page);

/*
	fz_meta: Perform a meta operation on a document.

	(In development - Subject to change in future versions)

	Meta operations provide a way to perform format specific
	operations on a document. The meta operation scheme is
	designed to be extensible so that new features can be
	transparently added in later versions of the library.

	doc: The document on which to perform the meta operation.

	key: The meta operation to try. If a particular operation
	is unsupported on a given document, the function will return
	FZ_META_UNKNOWN_KEY.

	ptr: An operation dependent (possibly NULL) pointer.

	size: An operation dependent integer. Often this will
	be the size of the block pointed to by ptr, but not always.

	Returns an operation dependent value; FZ_META_UNKNOWN_KEY
	always means "unknown operation for this document". In general
	FZ_META_OK should be used to indicate successful operation.
*/
int fz_meta(fz_document *doc, int key, void *ptr, int size);

enum
{
	FZ_META_UNKNOWN_KEY = -1,
	FZ_META_OK = 0,

	/*
		ptr: Pointer to block (uninitialised on entry)
		size: Size of block (at least 64 bytes)
		Returns: Document format as a brief text string.
		All formats should support this.
	*/
	FZ_META_FORMAT_INFO = 1,

	/*
		ptr: Pointer to block (uninitialised on entry)
		size: Size of block (at least 64 bytes)
		Returns: Encryption info as a brief text string.
	*/
	FZ_META_CRYPT_INFO = 2,

	/*
		ptr: NULL
		size: Which permission to check
		Returns: 1 if permitted, 0 otherwise.
	*/
	FZ_META_HAS_PERMISSION = 3,

	FZ_PERMISSION_PRINT = 0,
	FZ_PERMISSION_CHANGE = 1,
	FZ_PERMISSION_COPY = 2,
	FZ_PERMISSION_NOTES = 3,

	/*
		ptr: Pointer to block. First entry in the block is
		a pointer to a UTF8 string to lookup. The rest of the
		block is uninitialised on entry.
		size: size of the block in bytes.
		Returns: 0 if not found. 1 if found. The string
		result is copied into the block (truncated to size
		and NULL terminated)

	*/
	FZ_META_INFO = 4,
};

typedef struct fz_write_options_s fz_write_options;

/*
	In calls to fz_write, the following options structure can be used
	to control aspects of the writing process. This structure may grow
	in future, and should be zero-filled to allow forwards compatiblity.
*/
struct fz_write_options_s
{
	int do_ascii;    /*	If non-zero then attempt (where possible) to
				make the output ascii. */
	int do_expand;	/*	Bitflags; each non zero bit indicates an aspect
				of the file that should be 'expanded' on
				writing. */
	int do_garbage;	/*	If non-zero then attempt (where possible) to
				garbage collect the file before writing. */
	int do_linear;   /*	If non-zero then write linearised. */
};

/*	An enumeration of bitflags to use in the above 'do_expand' field of
	fz_write_options.
*/
enum
{
	fz_expand_images = 1,
	fz_expand_fonts = 2,
	fz_expand_all = -1
};

/*
	fz_write: Write a document out.

	(In development - Subject to change in future versions)

	Save a copy of the current document in its original format.
	Internally the document may change.

	doc: The document to save.

	filename: The filename to save to.

	opts: NULL, or a pointer to an options structure.

	May throw exceptions.
*/
void fz_write_document(fz_document *doc, char *filename, fz_write_options *opts);

#endif

typedef struct pdf_document_s pdf_document;

/*
 * Dynamic objects.
 * The same type of objects as found in PDF and PostScript.
 * Used by the filters and the mupdf parser.
 */

typedef struct pdf_obj_s pdf_obj;

pdf_obj *pdf_new_null(fz_context *ctx);
pdf_obj *pdf_new_bool(fz_context *ctx, int b);
pdf_obj *pdf_new_int(fz_context *ctx, int i);
pdf_obj *pdf_new_real(fz_context *ctx, float f);
pdf_obj *fz_new_name(fz_context *ctx, char *str);
pdf_obj *pdf_new_string(fz_context *ctx, char *str, int len);
pdf_obj *pdf_new_indirect(fz_context *ctx, int num, int gen, void *doc);
pdf_obj *pdf_new_array(fz_context *ctx, int initialcap);
pdf_obj *pdf_new_dict(fz_context *ctx, int initialcap);
pdf_obj *pdf_new_rect(fz_context *ctx, fz_rect *rect);
pdf_obj *pdf_new_matrix(fz_context *ctx, fz_matrix *mtx);
pdf_obj *pdf_copy_array(fz_context *ctx, pdf_obj *array);
pdf_obj *pdf_copy_dict(fz_context *ctx, pdf_obj *dict);

pdf_obj *pdf_keep_obj(pdf_obj *obj);
void pdf_drop_obj(pdf_obj *obj);

/* type queries */
int pdf_is_null(pdf_obj *obj);
int pdf_is_bool(pdf_obj *obj);
int pdf_is_int(pdf_obj *obj);
int pdf_is_real(pdf_obj *obj);
int pdf_is_name(pdf_obj *obj);
int pdf_is_string(pdf_obj *obj);
int pdf_is_array(pdf_obj *obj);
int pdf_is_dict(pdf_obj *obj);
int pdf_is_indirect(pdf_obj *obj);
int pdf_is_stream(pdf_document *doc, int num, int gen);

int pdf_objcmp(pdf_obj *a, pdf_obj *b);

/* dict marking and unmarking functions - to avoid infinite recursions */
int pdf_dict_marked(pdf_obj *obj);
int pdf_dict_mark(pdf_obj *obj);
void pdf_dict_unmark(pdf_obj *obj);

/* safe, silent failure, no error reporting on type mismatches */
int pdf_to_bool(pdf_obj *obj);
int pdf_to_int(pdf_obj *obj);
float pdf_to_real(pdf_obj *obj);
char *pdf_to_name(pdf_obj *obj);
char *pdf_to_str_buf(pdf_obj *obj);
pdf_obj *pdf_to_dict(pdf_obj *obj);
int pdf_to_str_len(pdf_obj *obj);
int pdf_to_num(pdf_obj *obj);
int pdf_to_gen(pdf_obj *obj);

int pdf_array_len(pdf_obj *array);
pdf_obj *pdf_array_get(pdf_obj *array, int i);
void pdf_array_put(pdf_obj *array, int i, pdf_obj *obj);
void pdf_array_push(pdf_obj *array, pdf_obj *obj);
void pdf_array_insert(pdf_obj *array, pdf_obj *obj);
int pdf_array_contains(pdf_obj *array, pdf_obj *obj);

int pdf_dict_len(pdf_obj *dict);
pdf_obj *pdf_dict_get_key(pdf_obj *dict, int idx);
pdf_obj *pdf_dict_get_val(pdf_obj *dict, int idx);
pdf_obj *pdf_dict_get(pdf_obj *dict, pdf_obj *key);
pdf_obj *pdf_dict_gets(pdf_obj *dict, char *key);
pdf_obj *pdf_dict_getp(pdf_obj *dict, char *key);
pdf_obj *pdf_dict_getsa(pdf_obj *dict, char *key, char *abbrev);
void pdf_dict_put(pdf_obj *dict, pdf_obj *key, pdf_obj *val);
void pdf_dict_puts(pdf_obj *dict, char *key, pdf_obj *val);
void pdf_dict_putp(pdf_obj *dict, char *key, pdf_obj *val);
void pdf_dict_del(pdf_obj *dict, pdf_obj *key);
void pdf_dict_dels(pdf_obj *dict, char *key);
void pdf_sort_dict(pdf_obj *dict);

int pdf_fprint_obj(FILE *fp, pdf_obj *obj, int tight);

#ifndef NDEBUG
void pdf_print_obj(pdf_obj *obj);
void pdf_print_ref(pdf_obj *obj);
#endif

char *pdf_to_utf8(fz_context *ctx, pdf_obj *src);
unsigned short *pdf_to_ucs2(fz_context *ctx, pdf_obj *src); /* sumatrapdf */
pdf_obj *pdf_to_utf8_name(fz_context *ctx, pdf_obj *src);
char *pdf_from_ucs2(fz_context *ctx, unsigned short *str);

fz_rect pdf_to_rect(fz_context *ctx, pdf_obj *array);
fz_matrix pdf_to_matrix(fz_context *ctx, pdf_obj *array);

int pdf_count_objects(pdf_document *doc);
pdf_obj *pdf_resolve_indirect(pdf_obj *ref);
pdf_obj *pdf_load_object(pdf_document *doc, int num, int gen);

fz_buffer *pdf_load_raw_stream(pdf_document *doc, int num, int gen);
fz_buffer *pdf_load_stream(pdf_document *doc, int num, int gen);
fz_stream *pdf_open_raw_stream(pdf_document *doc, int num, int gen);
fz_stream *pdf_open_stream(pdf_document *doc, int num, int gen);

fz_image *pdf_load_image(pdf_document *doc, pdf_obj *obj);

fz_outline *pdf_load_outline(pdf_document *doc);

/*
	pdf_create_object: Allocate a slot in the xref table and return a fresh unused object number.
*/
int pdf_create_object(pdf_document *xref);

/*
	pdf_delete_object: Remove object from xref table, marking the slot as free.
*/
void pdf_delete_object(pdf_document *xref, int num);

/*
	pdf_update_object: Replace object in xref table with the passed in object.
*/
void pdf_update_object(pdf_document *xref, int num, pdf_obj *obj);

/*
	pdf_update_stream: Replace stream contents for object in xref table with the passed in buffer.

	The buffer contents must match the /Filter setting.
	If storing uncompressed data, make sure to delete the /Filter key from
	the stream dictionary. If storing deflated data, make sure to set the
	/Filter value to /FlateDecode.
*/
void pdf_update_stream(pdf_document *xref, int num, fz_buffer *buf);

/*
	pdf_write_document: Write out the document to a file with all changes finalised.
*/
void pdf_write_document(pdf_document *doc, char *filename, fz_write_options *opts);

/*
	pdf_open_document: Open a PDF document.

	Open a PDF document by reading its cross reference table, so
	MuPDF can locate PDF objects inside the file. Upon an broken
	cross reference table or other parse errors MuPDF will restart
	parsing the file from the beginning to try to rebuild a
	(hopefully correct) cross reference table to allow further
	processing of the file.

	The returned pdf_document should be used when calling most
	other PDF functions. Note that it wraps the context, so those
	functions implicitly get access to the global state in
	context.

	filename: a path to a file as it would be given to open(2).
*/
pdf_document *pdf_open_document(fz_context *ctx, const char *filename);

/*
	pdf_open_document_with_stream: Opens a PDF document.

	Same as pdf_open_document, but takes a stream instead of a
	filename to locate the PDF document to open. Increments the
	reference count of the stream. See fz_open_file,
	fz_open_file_w or fz_open_fd for opening a stream, and
	fz_close for closing an open stream.
*/
pdf_document *pdf_open_document_with_stream(fz_stream *file);

/*
	pdf_close_document: Closes and frees an opened PDF document.

	The resource store in the context associated with pdf_document
	is emptied.

	Does not throw exceptions.
*/
void pdf_close_document(pdf_document *doc);

int pdf_needs_password(pdf_document *doc);
int pdf_authenticate_password(pdf_document *doc, char *pw);

enum
{
	PDF_PERM_PRINT = 1 << 2,
	PDF_PERM_CHANGE = 1 << 3,
	PDF_PERM_COPY = 1 << 4,
	PDF_PERM_NOTES = 1 << 5,
	PDF_PERM_FILL_FORM = 1 << 8,
	PDF_PERM_ACCESSIBILITY = 1 << 9,
	PDF_PERM_ASSEMBLE = 1 << 10,
	PDF_PERM_HIGH_RES_PRINT = 1 << 11,
	PDF_DEFAULT_PERM_FLAGS = 0xfffc
};

int pdf_has_permission(pdf_document *doc, int p);

typedef struct pdf_page_s pdf_page;

int pdf_lookup_page_number(pdf_document *doc, pdf_obj *pageobj);
int pdf_count_pages(pdf_document *doc);

/*
	pdf_load_page: Load a page and its resources.

	Locates the page in the PDF document and loads the page and its
	resources. After pdf_load_page is it possible to retrieve the size
	of the page using pdf_bound_page, or to render the page using
	pdf_run_page_*.

	number: page number, where 0 is the first page of the document.
*/
pdf_page *pdf_load_page(pdf_document *doc, int number);

fz_link *pdf_load_links(pdf_document *doc, pdf_page *page);

/*
	pdf_bound_page: Determine the size of a page.

	Determine the page size in user space units, taking page rotation
	into account. The page size is taken to be the crop box if it
	exists (visible area after cropping), otherwise the media box will
	be used (possibly including printing marks).

	Does not throw exceptions.
*/
fz_rect pdf_bound_page(pdf_document *doc, pdf_page *page);

/*
	pdf_free_page: Frees a page and its resources.

	Does not throw exceptions.
*/
void pdf_free_page(pdf_document *doc, pdf_page *page);

/*
	pdf_run_page: Interpret a loaded page and render it on a device.

	page: A page loaded by pdf_load_page.

	dev: Device used for rendering, obtained from fz_new_*_device.

	ctm: A transformation matrix applied to the objects on the page,
	e.g. to scale or rotate the page contents as desired.
*/
void pdf_run_page(pdf_document *doc, pdf_page *page, fz_device *dev, fz_matrix ctm, fz_cookie *cookie);

void pdf_run_page_with_usage(pdf_document *doc, pdf_page *page, fz_device *dev, fz_matrix ctm, char *event, fz_cookie *cookie);

/*
	Metadata interface.
*/
int pdf_meta(pdf_document *doc, int key, void *ptr, int size);

#endif // MUPDF_H



#ifndef MUPDF_INTERNAL_H
#define MUPDF_INTERNAL_H

// #include "mupdf.h"
#ifndef FITZ_INTERNAL_H
#define FITZ_INTERNAL_H

// #include "fitz.h"

#ifdef _WIN32 /* Microsoft Visual C++ */

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef __int64 int64_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#else
#include <inttypes.h>
#endif

struct fz_warn_context_s
{
	char message[256];
	int count;
};

fz_context *fz_clone_context_internal(fz_context *ctx);

void fz_new_aa_context(fz_context *ctx);
void fz_free_aa_context(fz_context *ctx);
void fz_copy_aa_context(fz_context *dst, fz_context *src);

/* Default allocator */
extern fz_alloc_context fz_alloc_default;

/* Default locks */
extern fz_locks_context fz_locks_default;

#if defined(MEMENTO) || defined(DEBUG)
#define FITZ_DEBUG_LOCKING
#endif

#ifdef FITZ_DEBUG_LOCKING

void fz_assert_lock_held(fz_context *ctx, int lock);
void fz_assert_lock_not_held(fz_context *ctx, int lock);
void fz_lock_debug_lock(fz_context *ctx, int lock);
void fz_lock_debug_unlock(fz_context *ctx, int lock);

#else

#define fz_assert_lock_held(A,B) do { } while (0)
#define fz_assert_lock_not_held(A,B) do { } while (0)
#define fz_lock_debug_lock(A,B) do { } while (0)
#define fz_lock_debug_unlock(A,B) do { } while (0)

#endif /* !FITZ_DEBUG_LOCKING */

static inline void
fz_lock(fz_context *ctx, int lock)
{
	fz_lock_debug_lock(ctx, lock);
	ctx->locks->lock(ctx->locks->user, lock);
}

static inline void
fz_unlock(fz_context *ctx, int lock)
{
	fz_lock_debug_unlock(ctx, lock);
	ctx->locks->unlock(ctx->locks->user, lock);
}


/*
 * Basic runtime and utility functions
 */

/* Range checking atof */
float fz_atof(const char *s);

/*
 * Generic hash-table with fixed-length keys.
 */

typedef struct fz_hash_table_s fz_hash_table;

fz_hash_table *fz_new_hash_table(fz_context *ctx, int initialsize, int keylen, int lock);
void fz_empty_hash(fz_context *ctx, fz_hash_table *table);
void fz_free_hash(fz_context *ctx, fz_hash_table *table);

void *fz_hash_find(fz_context *ctx, fz_hash_table *table, void *key);
void *fz_hash_insert(fz_context *ctx, fz_hash_table *table, void *key, void *val);
void fz_hash_remove(fz_context *ctx, fz_hash_table *table, void *key);

int fz_hash_len(fz_context *ctx, fz_hash_table *table);
void *fz_hash_get_key(fz_context *ctx, fz_hash_table *table, int idx);
void *fz_hash_get_val(fz_context *ctx, fz_hash_table *table, int idx);

#ifndef NDEBUG
void fz_print_hash(fz_context *ctx, FILE *out, fz_hash_table *table);
#endif

/*
 * Math and geometry
 */

/* Multiply scaled two integers in the 0..255 range */
static inline int fz_mul255(int a, int b)
{
	/* see Jim Blinn's book "Dirty Pixels" for how this works */
	int x = a * b + 128;
	x += x >> 8;
	return x >> 8;
}

/* Expand a value A from the 0...255 range to the 0..256 range */
#define FZ_EXPAND(A) ((A)+((A)>>7))

/* Combine values A (in any range) and B (in the 0..256 range),
 * to give a single value in the same range as A was. */
#define FZ_COMBINE(A,B) (((A)*(B))>>8)

/* Combine values A and C (in the same (any) range) and B and D (in the
 * 0..256 range), to give a single value in the same range as A and C were. */
#define FZ_COMBINE2(A,B,C,D) (FZ_COMBINE((A), (B)) + FZ_COMBINE((C), (D)))

/* Blend SRC and DST (in the same range) together according to
 * AMOUNT (in the 0...256 range). */
#define FZ_BLEND(SRC, DST, AMOUNT) ((((SRC)-(DST))*(AMOUNT) + ((DST)<<8))>>8)

void fz_gridfit_matrix(fz_matrix *m);
float fz_matrix_max_expansion(fz_matrix m);

/*
 * Basic crypto functions.
 * Independent of the rest of fitz.
 * For further encapsulation in filters, or not.
 */

/* md5 digests */

typedef struct fz_md5_s fz_md5;

struct fz_md5_s
{
	unsigned int state[4];
	unsigned int count[2];
	unsigned char buffer[64];
};

void fz_md5_init(fz_md5 *state);
void fz_md5_update(fz_md5 *state, const unsigned char *input, unsigned inlen);
void fz_md5_final(fz_md5 *state, unsigned char digest[16]);

/* sha-256 digests */

typedef struct fz_sha256_s fz_sha256;

struct fz_sha256_s
{
	unsigned int state[8];
	unsigned int count[2];
	union {
		unsigned char u8[64];
		unsigned int u32[16];
	} buffer;
};

void fz_sha256_init(fz_sha256 *state);
void fz_sha256_update(fz_sha256 *state, const unsigned char *input, unsigned int inlen);
void fz_sha256_final(fz_sha256 *state, unsigned char digest[32]);

/* sha-512 digests */

typedef struct fz_sha512_s fz_sha512;

struct fz_sha512_s
{
	uint64_t state[8];
	unsigned int count[2];
	union {
		unsigned char u8[128];
		uint64_t u64[16];
	} buffer;
};

void fz_sha512_init(fz_sha512 *state);
void fz_sha512_update(fz_sha512 *state, const unsigned char *input, unsigned int inlen);
void fz_sha512_final(fz_sha512 *state, unsigned char digest[32]);

/* sha-384 digests */

typedef struct fz_sha512_s fz_sha384;

void fz_sha384_init(fz_sha384 *state);
void fz_sha384_update(fz_sha384 *state, const unsigned char *input, unsigned int inlen);
void fz_sha384_final(fz_sha384 *state, unsigned char digest[32]);

/* arc4 crypto */

typedef struct fz_arc4_s fz_arc4;

struct fz_arc4_s
{
	unsigned x;
	unsigned y;
	unsigned char state[256];
};

void fz_arc4_init(fz_arc4 *state, const unsigned char *key, unsigned len);
void fz_arc4_encrypt(fz_arc4 *state, unsigned char *dest, const unsigned char *src, unsigned len);

/* AES block cipher implementation from XYSSL */

typedef struct fz_aes_s fz_aes;

#define AES_DECRYPT 0
#define AES_ENCRYPT 1

struct fz_aes_s
{
	int nr; /* number of rounds */
	unsigned long *rk; /* AES round keys */
	unsigned long buf[68]; /* unaligned data */
};

void aes_setkey_enc( fz_aes *ctx, const unsigned char *key, int keysize );
void aes_setkey_dec( fz_aes *ctx, const unsigned char *key, int keysize );
void aes_crypt_cbc( fz_aes *ctx, int mode, int length,
	unsigned char iv[16],
	const unsigned char *input,
	unsigned char *output );

/*
	Resource store

	MuPDF stores decoded "objects" into a store for potential reuse.
	If the size of the store gets too big, objects stored within it can
	be evicted and freed to recover space. When MuPDF comes to decode
	such an object, it will check to see if a version of this object is
	already in the store - if it is, it will simply reuse it. If not, it
	will decode it and place it into the store.

	All objects that can be placed into the store are derived from the
	fz_storable type (i.e. this should be the first component of the
	objects structure). This allows for consistent (thread safe)
	reference counting, and includes a function that will be called to
	free the object as soon as the reference count reaches zero.

	Most objects offer fz_keep_XXXX/fz_drop_XXXX functions derived
	from fz_keep_storable/fz_drop_storable. Creation of such objects
	includes a call to FZ_INIT_STORABLE to set up the fz_storable header.
 */

typedef struct fz_storable_s fz_storable;

typedef void (fz_store_free_fn)(fz_context *, fz_storable *);

struct fz_storable_s {
	int refs;
	fz_store_free_fn *free;
};

#define FZ_INIT_STORABLE(S_,RC,FREE) \
	do { fz_storable *S = &(S_)->storable; S->refs = (RC); \
	S->free = (FREE); \
	} while (0)

void *fz_keep_storable(fz_context *, fz_storable *);
void fz_drop_storable(fz_context *, fz_storable *);

/*
	The store can be seen as a dictionary that maps keys to fz_storable
	values. In order to allow keys of different types to be stored, we
	have a structure full of functions for each key 'type'; this
	fz_store_type pointer is stored with each key, and tells the store
	how to perform certain operations (like taking/dropping a reference,
	comparing two keys, outputting details for debugging etc).

	The store uses a hash table internally for speed where possible. In
	order for this to work, we need a mechanism for turning a generic
	'key' into 'a hashable string'. For this purpose the type structure
	contains a make_hash_key function pointer that maps from a void *
	to an fz_store_hash structure. If make_hash_key function returns 0,
	then the key is determined not to be hashable, and the value is
	not stored in the hash table.
*/
typedef struct fz_store_hash_s fz_store_hash;

struct fz_store_hash_s
{
	fz_store_free_fn *free;
	union
	{
		struct
		{
			int i0;
			int i1;
		} i;
		struct
		{
			void *ptr;
			int i;
		} pi;
	} u;
};

typedef struct fz_store_type_s fz_store_type;

struct fz_store_type_s
{
	int (*make_hash_key)(fz_store_hash *, void *);
	void *(*keep_key)(fz_context *,void *);
	void (*drop_key)(fz_context *,void *);
	int (*cmp_key)(void *, void *);
#ifndef NDEBUG
	void (*debug)(void *);
#endif
};

/*
	fz_store_new_context: Create a new store inside the context

	max: The maximum size (in bytes) that the store is allowed to grow
	to. FZ_STORE_UNLIMITED means no limit.
*/
void fz_new_store_context(fz_context *ctx, unsigned int max);

/*
	fz_drop_store_context: Drop a reference to the store.
*/
void fz_drop_store_context(fz_context *ctx);

/*
	fz_keep_store_context: Take a reference to the store.
*/
fz_store *fz_keep_store_context(fz_context *ctx);

/*
	fz_store_item: Add an item to the store.

	Add an item into the store, returning NULL for success. If an item
	with the same key is found in the store, then our item will not be
	inserted, and the function will return a pointer to that value
	instead. This function takes its own reference to val, as required
	(i.e. the caller maintains ownership of its own reference).

	key: The key to use to index the item.

	val: The value to store.

	itemsize: The size in bytes of the value (as counted towards the
	store size).

	type: Functions used to manipulate the key.
*/
void *fz_store_item(fz_context *ctx, void *key, void *val, unsigned int itemsize, fz_store_type *type);

/*
	fz_find_item: Find an item within the store.

	free: The function used to free the value (to ensure we get a value
	of the correct type).

	key: The key to use to index the item.

	type: Functions used to manipulate the key.

	Returns NULL for not found, otherwise returns a pointer to the value
	indexed by key to which a reference has been taken.
*/
void *fz_find_item(fz_context *ctx, fz_store_free_fn *free, void *key, fz_store_type *type);

/*
	fz_remove_item: Remove an item from the store.

	If an item indexed by the given key exists in the store, remove it.

	free: The function used to free the value (to ensure we get a value
	of the correct type).

	key: The key to use to find the item to remove.

	type: Functions used to manipulate the key.
*/
void fz_remove_item(fz_context *ctx, fz_store_free_fn *free, void *key, fz_store_type *type);

/*
	fz_empty_store: Evict everything from the store.
*/
void fz_empty_store(fz_context *ctx);

/*
	fz_store_scavenge: Internal function used as part of the scavenging
	allocator; when we fail to allocate memory, before returning a
	failure to the caller, we try to scavenge space within the store by
	evicting at least 'size' bytes. The allocator then retries.

	size: The number of bytes we are trying to have free.

	phase: What phase of the scavenge we are in. Updated on exit.

	Returns non zero if we managed to free any memory.
*/
int fz_store_scavenge(fz_context *ctx, unsigned int size, int *phase);

/*
	fz_print_store: Dump the contents of the store for debugging.
*/
#ifndef NDEBUG
void fz_print_store(fz_context *ctx, FILE *out);
#endif

struct fz_buffer_s
{
	int refs;
	unsigned char *data;
	int cap, len;
	int unused_bits;
};

/*
	fz_new_buffer: Create a new buffer.

	capacity: Initial capacity.

	Returns pointer to new buffer. Throws exception on allocation
	failure.
*/
fz_buffer *fz_new_buffer(fz_context *ctx, int capacity);

/*
	fz_resize_buffer: Ensure that a buffer has a given capacity,
	truncating data if required.

	buf: The buffer to alter.

	capacity: The desired capacity for the buffer. If the current size
	of the buffer contents is smaller than capacity, it is truncated.

*/
void fz_resize_buffer(fz_context *ctx, fz_buffer *buf, int capacity);

/*
	fz_grow_buffer: Make some space within a buffer (i.e. ensure that
	capacity > size).

	buf: The buffer to grow.

	May throw exception on failure to allocate.
*/
void fz_grow_buffer(fz_context *ctx, fz_buffer *buf);

/*
	fz_trim_buffer: Trim wasted capacity from a buffer.

	buf: The buffer to trim.
*/
void fz_trim_buffer(fz_context *ctx, fz_buffer *buf);

void fz_write_buffer(fz_context *ctx, fz_buffer *buf, unsigned char *data, int len);

void fz_write_buffer_byte(fz_context *ctx, fz_buffer *buf, int val);

void fz_write_buffer_bits(fz_context *ctx, fz_buffer *buf, int val, int bits);

void fz_write_buffer_pad(fz_context *ctx, fz_buffer *buf);

/*
	fz_buffer_printf: print formatted to a buffer. The buffer will
	grow, but the caller must ensure that no more than 256 bytes are
	added to the buffer per call.
*/
void fz_buffer_printf(fz_context *ctx, fz_buffer *buffer, char *fmt, ...);

struct fz_stream_s
{
	fz_context *ctx;
	int refs;
	int error;
	int eof;
	int pos;
	int avail;
	int bits;
	unsigned char *bp, *rp, *wp, *ep;
	void *state;
	int (*read)(fz_stream *stm, unsigned char *buf, int len);
	void (*close)(fz_context *ctx, void *state);
	void (*seek)(fz_stream *stm, int offset, int whence);
	unsigned char buf[4096];
};

fz_stream *fz_new_stream(fz_context *ctx, void*, int(*)(fz_stream*, unsigned char*, int), void(*)(fz_context *, void *));
fz_stream *fz_keep_stream(fz_stream *stm);
void fz_fill_buffer(fz_stream *stm);

void fz_read_line(fz_stream *stm, char *buf, int max);

static inline int fz_read_byte(fz_stream *stm)
{
	if (stm->rp == stm->wp)
	{
		fz_fill_buffer(stm);
		return stm->rp < stm->wp ? *stm->rp++ : EOF;
	}
	return *stm->rp++;
}

static inline int fz_peek_byte(fz_stream *stm)
{
	if (stm->rp == stm->wp)
	{
		fz_fill_buffer(stm);
		return stm->rp < stm->wp ? *stm->rp : EOF;
	}
	return *stm->rp;
}

static inline void fz_unread_byte(fz_stream *stm)
{
	if (stm->rp > stm->bp)
		stm->rp--;
}

static inline int fz_is_eof(fz_stream *stm)
{
	if (stm->rp == stm->wp)
	{
		if (stm->eof)
			return 1;
		return fz_peek_byte(stm) == EOF;
	}
	return 0;
}

static inline unsigned int fz_read_bits(fz_stream *stm, int n)
{
	unsigned int x;

	if (n <= stm->avail)
	{
		stm->avail -= n;
		x = (stm->bits >> stm->avail) & ((1 << n) - 1);
	}
	else
	{
		x = stm->bits & ((1 << stm->avail) - 1);
		n -= stm->avail;
		stm->avail = 0;

		while (n > 8)
		{
			x = (x << 8) | fz_read_byte(stm);
			n -= 8;
		}

		if (n > 0)
		{
			stm->bits = fz_read_byte(stm);
			stm->avail = 8 - n;
			x = (x << n) | (stm->bits >> stm->avail);
		}
	}

	return x;
}

static inline void fz_sync_bits(fz_stream *stm)
{
	stm->avail = 0;
}

static inline int fz_is_eof_bits(fz_stream *stm)
{
	return fz_is_eof(stm) && (stm->avail == 0 || stm->bits == EOF);
}

/*
 * Data filters.
 */

fz_stream *fz_open_copy(fz_stream *chain);
fz_stream *fz_open_null(fz_stream *chain, int len, int offset);
fz_stream *fz_open_concat(fz_context *ctx, int max, int pad);
void fz_concat_push(fz_stream *concat, fz_stream *chain); /* Ownership of chain is passed in */
fz_stream *fz_open_arc4(fz_stream *chain, unsigned char *key, unsigned keylen);
fz_stream *fz_open_aesd(fz_stream *chain, unsigned char *key, unsigned keylen);
fz_stream *fz_open_a85d(fz_stream *chain);
fz_stream *fz_open_ahxd(fz_stream *chain);
fz_stream *fz_open_rld(fz_stream *chain);
fz_stream *fz_open_dctd(fz_stream *chain, int color_transform);
fz_stream *fz_open_resized_dctd(fz_stream *chain, int color_transform, int factor);
fz_stream *fz_open_faxd(fz_stream *chain,
	int k, int end_of_line, int encoded_byte_align,
	int columns, int rows, int end_of_block, int black_is_1);
fz_stream *fz_open_flated(fz_stream *chain);
fz_stream *fz_open_lzwd(fz_stream *chain, int early_change);
fz_stream *fz_open_predict(fz_stream *chain, int predictor, int columns, int colors, int bpc);
fz_stream *fz_open_jbig2d(fz_stream *chain, fz_buffer *global);

/*
 * Resources and other graphics related objects.
 */

enum { FZ_MAX_COLORS = 32 };

int fz_lookup_blendmode(char *name);
char *fz_blendmode_name(int blendmode);

struct fz_bitmap_s
{
	int refs;
	int w, h, stride, n;
	unsigned char *samples;
};

fz_bitmap *fz_new_bitmap(fz_context *ctx, int w, int h, int n);

void fz_bitmap_details(fz_bitmap *bitmap, int *w, int *h, int *n, int *stride);

void fz_clear_bitmap(fz_context *ctx, fz_bitmap *bit);

/*
	Pixmaps represent a set of pixels for a 2 dimensional region of a
	plane. Each pixel has n components per pixel, the last of which is
	always alpha. The data is in premultiplied alpha when rendering, but
	non-premultiplied for colorspace conversions and rescaling.

	x, y: The minimum x and y coord of the region in pixels.

	w, h: The width and height of the region in pixels.

	n: The number of color components in the image. Always
	includes a separate alpha channel. For mask images n=1, for greyscale
	(plus alpha) images n=2, for rgb (plus alpha) images n=3.

	interpolate: A boolean flag set to non-zero if the image
	will be drawn using linear interpolation, or set to zero if
	image will be using nearest neighbour sampling.

	xres, yres: Image resolution in dpi. Default is 96 dpi.

	colorspace: Pointer to a colorspace object describing the colorspace
	the pixmap is in. If NULL, the image is a mask.

	samples: A simple block of memory w * h * n bytes of memory in which
	the components are stored. The first n bytes are components 0 to n-1
	for the pixel at (x,y). Each successive n bytes gives another pixel
	in scanline order. Subsequent scanlines follow on with no padding.

	free_samples: Is zero when an application has provided its own
	buffer for pixel data through fz_new_pixmap_with_bbox_and_data.
	If not zero the buffer will be freed when fz_drop_pixmap is
	called for the pixmap.
*/
struct fz_pixmap_s
{
	fz_storable storable;
	int x, y, w, h, n;
	int interpolate;
	int xres, yres;
	fz_colorspace *colorspace;
	unsigned char *samples;
	int free_samples;
};

void fz_free_pixmap_imp(fz_context *ctx, fz_storable *pix);

void fz_clear_pixmap_rect_with_value(fz_context *ctx, fz_pixmap *pix, int value, fz_bbox r);
void fz_copy_pixmap_rect(fz_context *ctx, fz_pixmap *dest, fz_pixmap *src, fz_bbox r);
void fz_premultiply_pixmap(fz_context *ctx, fz_pixmap *pix);
fz_pixmap *fz_alpha_from_gray(fz_context *ctx, fz_pixmap *gray, int luminosity);
unsigned int fz_pixmap_size(fz_context *ctx, fz_pixmap *pix);

fz_pixmap *fz_scale_pixmap(fz_context *ctx, fz_pixmap *src, float x, float y, float w, float h, fz_bbox *clip);

fz_bbox fz_pixmap_bbox_no_ctx(fz_pixmap *src);

struct fz_image_s
{
	fz_storable storable;
	int w, h;
	fz_image *mask;
	fz_colorspace *colorspace;
	fz_pixmap *(*get_pixmap)(fz_context *, fz_image *, int w, int h);
};

fz_pixmap *fz_load_jpx(fz_context *ctx, unsigned char *data, int size, fz_colorspace *cs, int indexed);
fz_pixmap *fz_load_jpeg(fz_context *doc, unsigned char *data, int size);
fz_pixmap *fz_load_png(fz_context *doc, unsigned char *data, int size);
fz_pixmap *fz_load_tiff(fz_context *doc, unsigned char *data, int size);

struct fz_halftone_s
{
	int refs;
	int n;
	fz_pixmap *comp[1];
};

fz_halftone *fz_new_halftone(fz_context *ctx, int num_comps);
fz_halftone *fz_default_halftone(fz_context *ctx, int num_comps);
void fz_drop_halftone(fz_context *ctx, fz_halftone *half);
fz_halftone *fz_keep_halftone(fz_context *ctx, fz_halftone *half);

struct fz_colorspace_s
{
	fz_storable storable;
	unsigned int size;
	char name[16];
	int n;
	void (*to_rgb)(fz_context *ctx, fz_colorspace *, float *src, float *rgb);
	void (*from_rgb)(fz_context *ctx, fz_colorspace *, float *rgb, float *dst);
	void (*free_data)(fz_context *Ctx, fz_colorspace *);
	void *data;
};

fz_colorspace *fz_new_colorspace(fz_context *ctx, char *name, int n);
fz_colorspace *fz_keep_colorspace(fz_context *ctx, fz_colorspace *colorspace);
void fz_drop_colorspace(fz_context *ctx, fz_colorspace *colorspace);
void fz_free_colorspace_imp(fz_context *ctx, fz_storable *colorspace);

void fz_convert_color(fz_context *ctx, fz_colorspace *dsts, float *dstv, fz_colorspace *srcs, float *srcv);

/*
 * Fonts come in two variants:
 *	Regular fonts are handled by FreeType.
 *	Type 3 fonts have callbacks to the interpreter.
 */

char *ft_error_string(int err);

struct fz_font_s
{
	int refs;
	char name[32];

	void *ft_face; /* has an FT_Face if used */
	int ft_substitute; /* ... substitute metrics */
	int ft_bold; /* ... synthesize bold */
	int ft_italic; /* ... synthesize italic */
	int ft_hint; /* ... force hinting for DynaLab fonts */

	/* origin of font data */
	char *ft_file;
	unsigned char *ft_data;
	int ft_size;

	fz_matrix t3matrix;
	void *t3resources;
	fz_buffer **t3procs; /* has 256 entries if used */
	float *t3widths; /* has 256 entries if used */
	char *t3flags; /* has 256 entries if used */
	void *t3doc; /* a pdf_document for the callback */
	void (*t3run)(void *doc, void *resources, fz_buffer *contents, fz_device *dev, fz_matrix ctm, void *gstate);
	void (*t3freeres)(void *doc, void *resources);

	fz_rect bbox;	/* font bbox is used only for t3 fonts */

	/* per glyph bounding box cache */
	int use_glyph_bbox;
	int bbox_count;
	fz_rect *bbox_table;

	/* substitute metrics */
	int width_count;
	int *width_table; /* in 1000 units */
};

void fz_new_font_context(fz_context *ctx);
fz_font_context *fz_keep_font_context(fz_context *ctx);
void fz_drop_font_context(fz_context *ctx);

fz_font *fz_new_type3_font(fz_context *ctx, char *name, fz_matrix matrix);

fz_font *fz_new_font_from_memory(fz_context *ctx, char *name, unsigned char *data, int len, int index, int use_glyph_bbox);
fz_font *fz_new_font_from_file(fz_context *ctx, char *name, char *path, int index, int use_glyph_bbox);

fz_font *fz_keep_font(fz_context *ctx, fz_font *font);
void fz_drop_font(fz_context *ctx, fz_font *font);

void fz_set_font_bbox(fz_context *ctx, fz_font *font, float xmin, float ymin, float xmax, float ymax);
fz_rect fz_bound_glyph(fz_context *ctx, fz_font *font, int gid, fz_matrix trm);
int fz_glyph_cacheable(fz_context *ctx, fz_font *font, int gid);

#ifndef NDEBUG
void fz_print_font(fz_context *ctx, FILE *out, fz_font *font);
#endif

/*
 * Vector path buffer.
 * It can be stroked and dashed, or be filled.
 * It has a fill rule (nonzero or even_odd).
 *
 * When rendering, they are flattened, stroked and dashed straight
 * into the Global Edge List.
 */

typedef struct fz_path_s fz_path;
typedef struct fz_stroke_state_s fz_stroke_state;

typedef union fz_path_item_s fz_path_item;

typedef enum fz_path_item_kind_e
{
	FZ_MOVETO,
	FZ_LINETO,
	FZ_CURVETO,
	FZ_CLOSE_PATH
} fz_path_item_kind;

typedef enum fz_linecap_e
{
	FZ_LINECAP_BUTT = 0,
	FZ_LINECAP_ROUND = 1,
	FZ_LINECAP_SQUARE = 2,
	FZ_LINECAP_TRIANGLE = 3
} fz_linecap;

typedef enum fz_linejoin_e
{
	FZ_LINEJOIN_MITER = 0,
	FZ_LINEJOIN_ROUND = 1,
	FZ_LINEJOIN_BEVEL = 2,
	FZ_LINEJOIN_MITER_XPS = 3
} fz_linejoin;

union fz_path_item_s
{
	fz_path_item_kind k;
	float v;
};

struct fz_path_s
{
	int len, cap;
	fz_path_item *items;
	int last;
};

struct fz_stroke_state_s
{
	int refs;
	fz_linecap start_cap, dash_cap, end_cap;
	fz_linejoin linejoin;
	float linewidth;
	float miterlimit;
	float dash_phase;
	int dash_len;
	float dash_list[32];
};

fz_path *fz_new_path(fz_context *ctx);
fz_point fz_currentpoint(fz_context *ctx, fz_path *path);
void fz_moveto(fz_context*, fz_path*, float x, float y);
void fz_lineto(fz_context*, fz_path*, float x, float y);
void fz_curveto(fz_context*,fz_path*, float, float, float, float, float, float);
void fz_curvetov(fz_context*,fz_path*, float, float, float, float);
void fz_curvetoy(fz_context*,fz_path*, float, float, float, float);
void fz_closepath(fz_context*,fz_path*);
void fz_free_path(fz_context *ctx, fz_path *path);

void fz_transform_path(fz_context *ctx, fz_path *path, fz_matrix transform);

fz_path *fz_clone_path(fz_context *ctx, fz_path *old);

fz_rect fz_bound_path(fz_context *ctx, fz_path *path, fz_stroke_state *stroke, fz_matrix ctm);
void fz_adjust_rect_for_stroke(fz_rect *r, fz_stroke_state *stroke, fz_matrix *ctm);

fz_stroke_state *fz_new_stroke_state(fz_context *ctx);
fz_stroke_state *fz_new_stroke_state_with_len(fz_context *ctx, int len);
fz_stroke_state *fz_keep_stroke_state(fz_context *ctx, fz_stroke_state *stroke);
void fz_drop_stroke_state(fz_context *ctx, fz_stroke_state *stroke);
fz_stroke_state *fz_unshare_stroke_state(fz_context *ctx, fz_stroke_state *shared);
fz_stroke_state *fz_unshare_stroke_state_with_len(fz_context *ctx, fz_stroke_state *shared, int len);

#ifndef NDEBUG
void fz_print_path(fz_context *ctx, FILE *out, fz_path *, int indent);
#endif

/*
 * Glyph cache
 */

void fz_new_glyph_cache_context(fz_context *ctx);
fz_glyph_cache *fz_keep_glyph_cache(fz_context *ctx);
void fz_drop_glyph_cache_context(fz_context *ctx);
void fz_purge_glyph_cache(fz_context *ctx);

fz_path *fz_outline_ft_glyph(fz_context *ctx, fz_font *font, int gid, fz_matrix trm);
fz_path *fz_outline_glyph(fz_context *ctx, fz_font *font, int gid, fz_matrix ctm);
fz_pixmap *fz_render_ft_glyph(fz_context *ctx, fz_font *font, int cid, fz_matrix trm, int aa);
fz_pixmap *fz_render_t3_glyph(fz_context *ctx, fz_font *font, int cid, fz_matrix trm, fz_colorspace *model, fz_bbox scissor);
fz_pixmap *fz_render_ft_stroked_glyph(fz_context *ctx, fz_font *font, int gid, fz_matrix trm, fz_matrix ctm, fz_stroke_state *state);
fz_pixmap *fz_render_glyph(fz_context *ctx, fz_font*, int, fz_matrix, fz_colorspace *model, fz_bbox scissor);
fz_pixmap *fz_render_stroked_glyph(fz_context *ctx, fz_font*, int, fz_matrix, fz_matrix, fz_stroke_state *stroke, fz_bbox scissor);
void fz_render_t3_glyph_direct(fz_context *ctx, fz_device *dev, fz_font *font, int gid, fz_matrix trm, void *gstate);

/*
 * Text buffer.
 *
 * The trm field contains the a, b, c and d coefficients.
 * The e and f coefficients come from the individual elements,
 * together they form the transform matrix for the glyph.
 *
 * Glyphs are referenced by glyph ID.
 * The Unicode text equivalent is kept in a separate array
 * with indexes into the glyph array.
 */

typedef struct fz_text_s fz_text;
typedef struct fz_text_item_s fz_text_item;

struct fz_text_item_s
{
	float x, y;
	int gid; /* -1 for one gid to many ucs mappings */
	int ucs; /* -1 for one ucs to many gid mappings */
};

struct fz_text_s
{
	fz_font *font;
	fz_matrix trm;
	int wmode;
	int len, cap;
	fz_text_item *items;
};

fz_text *fz_new_text(fz_context *ctx, fz_font *face, fz_matrix trm, int wmode);
void fz_add_text(fz_context *ctx, fz_text *text, int gid, int ucs, float x, float y);
void fz_free_text(fz_context *ctx, fz_text *text);
fz_rect fz_bound_text(fz_context *ctx, fz_text *text, fz_matrix ctm);
fz_text *fz_clone_text(fz_context *ctx, fz_text *old);
void fz_print_text(fz_context *ctx, FILE *out, fz_text*);

/*
 * The shading code uses gouraud shaded triangle meshes.
 */

enum
{
	FZ_LINEAR,
	FZ_RADIAL,
	FZ_MESH,
};

typedef struct fz_shade_s fz_shade;

struct fz_shade_s
{
	fz_storable storable;

	fz_rect bbox;		/* can be fz_infinite_rect */
	fz_colorspace *colorspace;

	fz_matrix matrix;	/* matrix from pattern dict */
	int use_background;	/* background color for fills but not 'sh' */
	float background[FZ_MAX_COLORS];

	int use_function;
	float function[256][FZ_MAX_COLORS + 1];

	int type; /* linear, radial, mesh */
	int extend[2];

	int mesh_len;
	int mesh_cap;
	float *mesh; /* [x y 0], [x y r], [x y t] or [x y c1 ... cn] */
};

fz_shade *fz_keep_shade(fz_context *ctx, fz_shade *shade);
void fz_drop_shade(fz_context *ctx, fz_shade *shade);
void fz_free_shade_imp(fz_context *ctx, fz_storable *shade);

fz_rect fz_bound_shade(fz_context *ctx, fz_shade *shade, fz_matrix ctm);
void fz_paint_shade(fz_context *ctx, fz_shade *shade, fz_matrix ctm, fz_pixmap *dest, fz_bbox bbox);

#ifndef NDEBUG
void fz_print_shade(fz_context *ctx, FILE *out, fz_shade *shade);
#endif

/*
 * Scan converter
 */

typedef struct fz_gel_s fz_gel;

fz_gel *fz_new_gel(fz_context *ctx);
void fz_insert_gel(fz_gel *gel, float x0, float y0, float x1, float y1);
void fz_reset_gel(fz_gel *gel, fz_bbox clip);
void fz_sort_gel(fz_gel *gel);
fz_bbox fz_bound_gel(fz_gel *gel);
void fz_free_gel(fz_gel *gel);
int fz_is_rect_gel(fz_gel *gel);

void fz_scan_convert(fz_gel *gel, int eofill, fz_bbox clip, fz_pixmap *pix, unsigned char *colorbv);

void fz_flatten_fill_path(fz_gel *gel, fz_path *path, fz_matrix ctm, float flatness);
void fz_flatten_stroke_path(fz_gel *gel, fz_path *path, fz_stroke_state *stroke, fz_matrix ctm, float flatness, float linewidth);
void fz_flatten_dash_path(fz_gel *gel, fz_path *path, fz_stroke_state *stroke, fz_matrix ctm, float flatness, float linewidth);

/*
 * The device interface.
 */

fz_device *fz_new_draw_device_type3(fz_context *ctx, fz_pixmap *dest);

enum
{
	/* Hints */
	FZ_IGNORE_IMAGE = 1,
	FZ_IGNORE_SHADE = 2,

	/* Flags */
	FZ_DEVFLAG_MASK = 1,
	FZ_DEVFLAG_COLOR = 2,
	FZ_DEVFLAG_UNCACHEABLE = 4,
	FZ_DEVFLAG_FILLCOLOR_UNDEFINED = 8,
	FZ_DEVFLAG_STROKECOLOR_UNDEFINED = 16,
	FZ_DEVFLAG_STARTCAP_UNDEFINED = 32,
	FZ_DEVFLAG_DASHCAP_UNDEFINED = 64,
	FZ_DEVFLAG_ENDCAP_UNDEFINED = 128,
	FZ_DEVFLAG_LINEJOIN_UNDEFINED = 256,
	FZ_DEVFLAG_MITERLIMIT_UNDEFINED = 512,
	FZ_DEVFLAG_LINEWIDTH_UNDEFINED = 1024,
	/* Arguably we should have a bit for the dash pattern itself being
	 * undefined, but that causes problems; do we assume that it should
	 * always be set to non-dashing at the start of every glyph? */
};

struct fz_device_s
{
	int hints;
	int flags;

	void *user;
	void (*free_user)(fz_device *);
	fz_context *ctx;

	void (*fill_path)(fz_device *, fz_path *, int even_odd, fz_matrix, fz_colorspace *, float *color, float alpha);
	void (*stroke_path)(fz_device *, fz_path *, fz_stroke_state *, fz_matrix, fz_colorspace *, float *color, float alpha);
	void (*clip_path)(fz_device *, fz_path *, fz_rect *rect, int even_odd, fz_matrix);
	void (*clip_stroke_path)(fz_device *, fz_path *, fz_rect *rect, fz_stroke_state *, fz_matrix);

	void (*fill_text)(fz_device *, fz_text *, fz_matrix, fz_colorspace *, float *color, float alpha);
	void (*stroke_text)(fz_device *, fz_text *, fz_stroke_state *, fz_matrix, fz_colorspace *, float *color, float alpha);
	void (*clip_text)(fz_device *, fz_text *, fz_matrix, int accumulate);
	void (*clip_stroke_text)(fz_device *, fz_text *, fz_stroke_state *, fz_matrix);
	void (*ignore_text)(fz_device *, fz_text *, fz_matrix);

	void (*fill_shade)(fz_device *, fz_shade *shd, fz_matrix ctm, float alpha);
	void (*fill_image)(fz_device *, fz_image *img, fz_matrix ctm, float alpha);
	void (*fill_image_mask)(fz_device *, fz_image *img, fz_matrix ctm, fz_colorspace *, float *color, float alpha);
	void (*clip_image_mask)(fz_device *, fz_image *img, fz_rect *rect, fz_matrix ctm);

	void (*pop_clip)(fz_device *);

	void (*begin_mask)(fz_device *, fz_rect, int luminosity, fz_colorspace *, float *bc);
	void (*end_mask)(fz_device *);
	void (*begin_group)(fz_device *, fz_rect, int isolated, int knockout, int blendmode, float alpha);
	void (*end_group)(fz_device *);

	void (*begin_tile)(fz_device *, fz_rect area, fz_rect view, float xstep, float ystep, fz_matrix ctm);
	void (*end_tile)(fz_device *);
};

void fz_fill_path(fz_device *dev, fz_path *path, int even_odd, fz_matrix ctm, fz_colorspace *colorspace, float *color, float alpha);
void fz_stroke_path(fz_device *dev, fz_path *path, fz_stroke_state *stroke, fz_matrix ctm, fz_colorspace *colorspace, float *color, float alpha);
void fz_clip_path(fz_device *dev, fz_path *path, fz_rect *rect, int even_odd, fz_matrix ctm);
void fz_clip_stroke_path(fz_device *dev, fz_path *path, fz_rect *rect, fz_stroke_state *stroke, fz_matrix ctm);
void fz_fill_text(fz_device *dev, fz_text *text, fz_matrix ctm, fz_colorspace *colorspace, float *color, float alpha);
void fz_stroke_text(fz_device *dev, fz_text *text, fz_stroke_state *stroke, fz_matrix ctm, fz_colorspace *colorspace, float *color, float alpha);
void fz_clip_text(fz_device *dev, fz_text *text, fz_matrix ctm, int accumulate);
void fz_clip_stroke_text(fz_device *dev, fz_text *text, fz_stroke_state *stroke, fz_matrix ctm);
void fz_ignore_text(fz_device *dev, fz_text *text, fz_matrix ctm);
void fz_pop_clip(fz_device *dev);
void fz_fill_shade(fz_device *dev, fz_shade *shade, fz_matrix ctm, float alpha);
void fz_fill_image(fz_device *dev, fz_image *image, fz_matrix ctm, float alpha);
void fz_fill_image_mask(fz_device *dev, fz_image *image, fz_matrix ctm, fz_colorspace *colorspace, float *color, float alpha);
void fz_clip_image_mask(fz_device *dev, fz_image *image, fz_rect *rect, fz_matrix ctm);
void fz_begin_mask(fz_device *dev, fz_rect area, int luminosity, fz_colorspace *colorspace, float *bc);
void fz_end_mask(fz_device *dev);
void fz_begin_group(fz_device *dev, fz_rect area, int isolated, int knockout, int blendmode, float alpha);
void fz_end_group(fz_device *dev);
void fz_begin_tile(fz_device *dev, fz_rect area, fz_rect view, float xstep, float ystep, fz_matrix ctm);
void fz_end_tile(fz_device *dev);

fz_device *fz_new_device(fz_context *ctx, void *user);



/*
 * Plotting functions.
 */

void fz_decode_tile(fz_pixmap *pix, float *decode);
void fz_decode_indexed_tile(fz_pixmap *pix, float *decode, int maxval);
void fz_unpack_tile(fz_pixmap *dst, unsigned char * restrict src, int n, int depth, int stride, int scale);

void fz_paint_solid_alpha(unsigned char * restrict dp, int w, int alpha);
void fz_paint_solid_color(unsigned char * restrict dp, int n, int w, unsigned char *color);

void fz_paint_span(unsigned char * restrict dp, unsigned char * restrict sp, int n, int w, int alpha);
void fz_paint_span_with_color(unsigned char * restrict dp, unsigned char * restrict mp, int n, int w, unsigned char *color);

void fz_paint_image(fz_pixmap *dst, fz_bbox scissor, fz_pixmap *shape, fz_pixmap *img, fz_matrix ctm, int alpha);
void fz_paint_image_with_color(fz_pixmap *dst, fz_bbox scissor, fz_pixmap *shape, fz_pixmap *img, fz_matrix ctm, unsigned char *colorbv);

void fz_paint_pixmap(fz_pixmap *dst, fz_pixmap *src, int alpha);
void fz_paint_pixmap_with_mask(fz_pixmap *dst, fz_pixmap *src, fz_pixmap *msk);
void fz_paint_pixmap_with_rect(fz_pixmap *dst, fz_pixmap *src, int alpha, fz_bbox bbox);

void fz_blend_pixmap(fz_pixmap *dst, fz_pixmap *src, int alpha, int blendmode, int isolated, fz_pixmap *shape);
void fz_blend_pixel(unsigned char dp[3], unsigned char bp[3], unsigned char sp[3], int blendmode);

enum
{
	/* PDF 1.4 -- standard separable */
	FZ_BLEND_NORMAL,
	FZ_BLEND_MULTIPLY,
	FZ_BLEND_SCREEN,
	FZ_BLEND_OVERLAY,
	FZ_BLEND_DARKEN,
	FZ_BLEND_LIGHTEN,
	FZ_BLEND_COLOR_DODGE,
	FZ_BLEND_COLOR_BURN,
	FZ_BLEND_HARD_LIGHT,
	FZ_BLEND_SOFT_LIGHT,
	FZ_BLEND_DIFFERENCE,
	FZ_BLEND_EXCLUSION,

	/* PDF 1.4 -- standard non-separable */
	FZ_BLEND_HUE,
	FZ_BLEND_SATURATION,
	FZ_BLEND_COLOR,
	FZ_BLEND_LUMINOSITY,

	/* For packing purposes */
	FZ_BLEND_MODEMASK = 15,
	FZ_BLEND_ISOLATED = 16,
	FZ_BLEND_KNOCKOUT = 32
};

struct fz_document_s
{
	void (*close)(fz_document *);
	int (*needs_password)(fz_document *doc);
	int (*authenticate_password)(fz_document *doc, char *password);
	fz_outline *(*load_outline)(fz_document *doc);
	int (*count_pages)(fz_document *doc);
	fz_page *(*load_page)(fz_document *doc, int number);
	fz_link *(*load_links)(fz_document *doc, fz_page *page);
	fz_rect (*bound_page)(fz_document *doc, fz_page *page);
	void (*run_page)(fz_document *doc, fz_page *page, fz_device *dev, fz_matrix transform, fz_cookie *cookie);
	void (*free_page)(fz_document *doc, fz_page *page);
	int (*meta)(fz_document *doc, int key, void *ptr, int size);
	void (*write)(fz_document *doc, char *filename, fz_write_options *opts);
};

#endif

void pdf_set_str_len(pdf_obj *obj, int newlen);
void *pdf_get_indirect_document(pdf_obj *obj);
void pdf_set_int(pdf_obj *obj, int i);

/*
 * PDF Images
 */

typedef struct pdf_image_params_s pdf_image_params;

struct pdf_image_params_s
{
	int type;
	fz_colorspace *colorspace;
	union
	{
		struct
		{
			int columns;
			int rows;
			int k;
			int eol;
			int eba;
			int eob;
			int bi1;
		}
		fax;
		struct
		{
			int ct;
		}
		jpeg;
		struct
		{
			int columns;
			int colors;
			int predictor;
			int bpc;
		}
		flate;
		struct
		{
			int columns;
			int colors;
			int predictor;
			int bpc;
			int ec;
		}
		lzw;
	}
	u;
};


typedef struct pdf_image_s pdf_image;

struct pdf_image_s
{
	fz_image base;
	fz_pixmap *tile;
	int n, bpc;
	pdf_image_params params;
	fz_buffer *buffer;
	int colorkey[FZ_MAX_COLORS * 2];
	float decode[FZ_MAX_COLORS * 2];
	int imagemask;
	int interpolate;
	int usecolorkey;
};

enum
{
	PDF_IMAGE_RAW,
	PDF_IMAGE_FAX,
	PDF_IMAGE_JPEG,
	PDF_IMAGE_RLD,
	PDF_IMAGE_FLATE,
	PDF_IMAGE_LZW,
	PDF_IMAGE_JPX
};

/*
 * tokenizer and low-level object parser
 */

enum
{
	PDF_TOK_ERROR, PDF_TOK_EOF,
	PDF_TOK_OPEN_ARRAY, PDF_TOK_CLOSE_ARRAY,
	PDF_TOK_OPEN_DICT, PDF_TOK_CLOSE_DICT,
	PDF_TOK_OPEN_BRACE, PDF_TOK_CLOSE_BRACE,
	PDF_TOK_NAME, PDF_TOK_INT, PDF_TOK_REAL, PDF_TOK_STRING, PDF_TOK_KEYWORD,
	PDF_TOK_R, PDF_TOK_TRUE, PDF_TOK_FALSE, PDF_TOK_NULL,
	PDF_TOK_OBJ, PDF_TOK_ENDOBJ,
	PDF_TOK_STREAM, PDF_TOK_ENDSTREAM,
	PDF_TOK_XREF, PDF_TOK_TRAILER, PDF_TOK_STARTXREF,
	PDF_NUM_TOKENS
};

enum
{
	PDF_LEXBUF_SMALL = 256,
	PDF_LEXBUF_LARGE = 65536
};

typedef struct pdf_lexbuf_s pdf_lexbuf;
typedef struct pdf_lexbuf_large_s pdf_lexbuf_large;

struct pdf_lexbuf_s
{
	fz_context *ctx;
	int size;
	int base_size;
	int len;
	int i;
	float f;
	char *scratch;
	char buffer[PDF_LEXBUF_SMALL];
};

struct pdf_lexbuf_large_s
{
	pdf_lexbuf base;
	char buffer[PDF_LEXBUF_LARGE - PDF_LEXBUF_SMALL];
};

void pdf_lexbuf_init(fz_context *ctx, pdf_lexbuf *lexbuf, int size);
void pdf_lexbuf_fin(pdf_lexbuf *lexbuf);
ptrdiff_t pdf_lexbuf_grow(pdf_lexbuf *lexbuf);

int pdf_lex(fz_stream *f, pdf_lexbuf *lexbuf);

pdf_obj *pdf_parse_array(pdf_document *doc, fz_stream *f, pdf_lexbuf *buf);
pdf_obj *pdf_parse_dict(pdf_document *doc, fz_stream *f, pdf_lexbuf *buf);
pdf_obj *pdf_parse_stm_obj(pdf_document *doc, fz_stream *f, pdf_lexbuf *buf);
pdf_obj *pdf_parse_ind_obj(pdf_document *doc, fz_stream *f, pdf_lexbuf *buf, int *num, int *gen, int *stm_ofs);

/*
	pdf_print_token: print a lexed token to a buffer, growing if necessary
*/
void pdf_print_token(fz_context *ctx, fz_buffer *buf, int tok, pdf_lexbuf *lex);

/*
 * xref and object / stream api
 */

typedef struct pdf_xref_entry_s pdf_xref_entry;

struct pdf_xref_entry_s
{
	char type;	/* 0=unset (f)ree i(n)use (o)bjstm */
	int ofs;	/* file offset / objstm object number */
	int gen;	/* generation / objstm index */
	int stm_ofs;	/* on-disk stream */
	fz_buffer *stm_buf; /* in-memory stream (for updated objects) */
	pdf_obj *obj;	/* stored/cached object */
};

typedef struct pdf_crypt_s pdf_crypt;
typedef struct pdf_ocg_descriptor_s pdf_ocg_descriptor;
typedef struct pdf_ocg_entry_s pdf_ocg_entry;

struct pdf_ocg_entry_s
{
	int num;
	int gen;
	int state;
};

struct pdf_ocg_descriptor_s
{
	int len;
	pdf_ocg_entry *ocgs;
	pdf_obj *intent;
};

struct pdf_document_s
{
	fz_document super;

	fz_context *ctx;
	fz_stream *file;

	int version;
	int startxref;
	int file_size;
	pdf_crypt *crypt;
	pdf_obj *trailer;
	pdf_ocg_descriptor *ocg;

	int len;
	pdf_xref_entry *table;

	int page_len;
	int page_cap;
	pdf_obj **page_objs;
	pdf_obj **page_refs;

	pdf_lexbuf_large lexbuf;
};

pdf_document *pdf_open_document_no_run(fz_context *ctx, const char *filename);
pdf_document *pdf_open_document_no_run_with_stream(fz_stream *file);

void pdf_cache_object(pdf_document *doc, int num, int gen);

fz_stream *pdf_open_inline_stream(pdf_document *doc, pdf_obj *stmobj, int length, fz_stream *chain, pdf_image_params *params);
fz_buffer *pdf_load_image_stream(pdf_document *doc, int num, int gen, int orig_num, int orig_gen, pdf_image_params *params);
fz_stream *pdf_open_image_stream(pdf_document *doc, int num, int gen, int orig_num, int orig_gen, pdf_image_params *params);
fz_stream *pdf_open_stream_with_offset(pdf_document *doc, int num, int gen, pdf_obj *dict, int stm_ofs);
fz_stream *pdf_open_image_decomp_stream(fz_context *ctx, fz_buffer *, pdf_image_params *params, int *factor);
fz_stream *pdf_open_contents_stream(pdf_document *xref, pdf_obj *obj);
fz_buffer *pdf_load_raw_renumbered_stream(pdf_document *doc, int num, int gen, int orig_num, int orig_gen);
fz_buffer *pdf_load_renumbered_stream(pdf_document *doc, int num, int gen, int orig_num, int orig_gen);
fz_stream *pdf_open_raw_renumbered_stream(pdf_document *doc, int num, int gen, int orig_num, int orig_gen);

void pdf_repair_xref(pdf_document *doc, pdf_lexbuf *buf);
void pdf_repair_obj_stms(pdf_document *doc);
void pdf_resize_xref(pdf_document *doc, int newcap);
pdf_obj *pdf_new_ref(pdf_document *doc, pdf_obj *obj);

void pdf_print_xref(pdf_document *);

/*
 * Encryption
 */

pdf_crypt *pdf_new_crypt(fz_context *ctx, pdf_obj *enc, pdf_obj *id);
void pdf_free_crypt(fz_context *ctx, pdf_crypt *crypt);

void pdf_crypt_obj(fz_context *ctx, pdf_crypt *crypt, pdf_obj *obj, int num, int gen);
void pdf_crypt_buffer(fz_context *ctx, pdf_crypt *crypt, fz_buffer *buf, int num, int gen);
fz_stream *pdf_open_crypt(fz_stream *chain, pdf_crypt *crypt, int num, int gen);
fz_stream *pdf_open_crypt_with_filter(fz_stream *chain, pdf_crypt *crypt, char *name, int num, int gen);

int pdf_crypt_version(pdf_document *doc);
int pdf_crypt_revision(pdf_document *doc);
char *pdf_crypt_method(pdf_document *doc);
int pdf_crypt_length(pdf_document *doc);
unsigned char *pdf_crypt_key(pdf_document *doc);

#ifndef NDEBUG
void pdf_print_crypt(pdf_crypt *crypt);
#endif

/*
 * Functions, Colorspaces, Shadings and Images
 */

typedef struct pdf_function_s pdf_function;

pdf_function *pdf_load_function(pdf_document *doc, pdf_obj *ref, int in, int out);
void pdf_eval_function(fz_context *ctx, pdf_function *func, float *in, int inlen, float *out, int outlen);
pdf_function *pdf_keep_function(fz_context *ctx, pdf_function *func);
void pdf_drop_function(fz_context *ctx, pdf_function *func);
unsigned int pdf_function_size(pdf_function *func);

fz_colorspace *pdf_load_colorspace(pdf_document *doc, pdf_obj *obj);
fz_pixmap *pdf_expand_indexed_pixmap(fz_context *ctx, fz_pixmap *src);

fz_shade *pdf_load_shading(pdf_document *doc, pdf_obj *obj);

fz_image *pdf_load_inline_image(pdf_document *doc, pdf_obj *rdb, pdf_obj *dict, fz_stream *file);
int pdf_is_jpx_image(fz_context *ctx, pdf_obj *dict);

/*
 * Pattern
 */

typedef struct pdf_pattern_s pdf_pattern;

struct pdf_pattern_s
{
	fz_storable storable;
	int ismask;
	float xstep;
	float ystep;
	fz_matrix matrix;
	fz_rect bbox;
	pdf_obj *resources;
	pdf_obj *contents;
};

pdf_pattern *pdf_load_pattern(pdf_document *doc, pdf_obj *obj);
pdf_pattern *pdf_keep_pattern(fz_context *ctx, pdf_pattern *pat);
void pdf_drop_pattern(fz_context *ctx, pdf_pattern *pat);

/*
 * XObject
 */

typedef struct pdf_xobject_s pdf_xobject;

struct pdf_xobject_s
{
	fz_storable storable;
	fz_matrix matrix;
	fz_rect bbox;
	int isolated;
	int knockout;
	int transparency;
	fz_colorspace *colorspace;
	pdf_obj *resources;
	pdf_obj *contents;
	pdf_obj *me;
};

pdf_xobject *pdf_load_xobject(pdf_document *doc, pdf_obj *obj);
pdf_obj *pdf_new_xobject(pdf_document *doc, fz_rect *bbox, fz_matrix *mat);
pdf_xobject *pdf_keep_xobject(fz_context *ctx, pdf_xobject *xobj);
void pdf_drop_xobject(fz_context *ctx, pdf_xobject *xobj);
void pdf_update_xobject_contents(pdf_document *xref, pdf_xobject *from, fz_buffer *buffer);


/*
 * CMap
 */

typedef struct pdf_cmap_s pdf_cmap;
typedef struct pdf_range_s pdf_range;

enum { PDF_CMAP_SINGLE, PDF_CMAP_RANGE, PDF_CMAP_TABLE, PDF_CMAP_MULTI };

struct pdf_range_s
{
	unsigned short low;
	/* Next, we pack 2 fields into the same unsigned short. Top 14 bits
	 * are the extent, bottom 2 bits are flags: single, range, table,
	 * multi */
	unsigned short extent_flags;
	unsigned short offset;	/* range-delta or table-index */
};

struct pdf_cmap_s
{
	fz_storable storable;
	char cmap_name[32];

	char usecmap_name[32];
	pdf_cmap *usecmap;

	int wmode;

	int codespace_len;
	struct
	{
		unsigned short n;
		unsigned short low;
		unsigned short high;
	} codespace[40];

	int rlen, rcap;
	pdf_range *ranges;

	int tlen, tcap;
	unsigned short *table;
};

pdf_cmap *pdf_new_cmap(fz_context *ctx);
pdf_cmap *pdf_keep_cmap(fz_context *ctx, pdf_cmap *cmap);
void pdf_drop_cmap(fz_context *ctx, pdf_cmap *cmap);
void pdf_free_cmap_imp(fz_context *ctx, fz_storable *cmap);
unsigned int pdf_cmap_size(fz_context *ctx, pdf_cmap *cmap);

int pdf_cmap_wmode(fz_context *ctx, pdf_cmap *cmap);
void pdf_set_cmap_wmode(fz_context *ctx, pdf_cmap *cmap, int wmode);
void pdf_set_usecmap(fz_context *ctx, pdf_cmap *cmap, pdf_cmap *usecmap);

void pdf_add_codespace(fz_context *ctx, pdf_cmap *cmap, int low, int high, int n);
void pdf_map_range_to_table(fz_context *ctx, pdf_cmap *cmap, int low, int *map, int len);
void pdf_map_range_to_range(fz_context *ctx, pdf_cmap *cmap, int srclo, int srchi, int dstlo);
void pdf_map_one_to_many(fz_context *ctx, pdf_cmap *cmap, int one, int *many, int len);
void pdf_sort_cmap(fz_context *ctx, pdf_cmap *cmap);

int pdf_lookup_cmap(pdf_cmap *cmap, int cpt);
int pdf_lookup_cmap_full(pdf_cmap *cmap, int cpt, int *out);
int pdf_decode_cmap(pdf_cmap *cmap, unsigned char *s, int *cpt);

pdf_cmap *pdf_new_identity_cmap(fz_context *ctx, int wmode, int bytes);
pdf_cmap *pdf_load_cmap(fz_context *ctx, fz_stream *file);
pdf_cmap *pdf_load_system_cmap(fz_context *ctx, char *name);
pdf_cmap *pdf_load_builtin_cmap(fz_context *ctx, char *name);
pdf_cmap *pdf_load_embedded_cmap(pdf_document *doc, pdf_obj *ref);

#ifndef NDEBUG
void pdf_print_cmap(fz_context *ctx, pdf_cmap *cmap);
#endif

/*
 * Font
 */

enum
{
	PDF_FD_FIXED_PITCH = 1 << 0,
	PDF_FD_SERIF = 1 << 1,
	PDF_FD_SYMBOLIC = 1 << 2,
	PDF_FD_SCRIPT = 1 << 3,
	PDF_FD_NONSYMBOLIC = 1 << 5,
	PDF_FD_ITALIC = 1 << 6,
	PDF_FD_ALL_CAP = 1 << 16,
	PDF_FD_SMALL_CAP = 1 << 17,
	PDF_FD_FORCE_BOLD = 1 << 18
};

enum { PDF_ROS_CNS, PDF_ROS_GB, PDF_ROS_JAPAN, PDF_ROS_KOREA };

void pdf_load_encoding(char **estrings, char *encoding);
int pdf_lookup_agl(char *name);
const char **pdf_lookup_agl_duplicates(int ucs);

extern const unsigned short pdf_doc_encoding[256];
extern const char * const pdf_mac_roman[256];
extern const char * const pdf_mac_expert[256];
extern const char * const pdf_win_ansi[256];
extern const char * const pdf_standard[256];

typedef struct pdf_font_desc_s pdf_font_desc;
typedef struct pdf_hmtx_s pdf_hmtx;
typedef struct pdf_vmtx_s pdf_vmtx;

struct pdf_hmtx_s
{
	unsigned short lo;
	unsigned short hi;
	int w;	/* type3 fonts can be big! */
};

struct pdf_vmtx_s
{
	unsigned short lo;
	unsigned short hi;
	short x;
	short y;
	short w;
};

struct pdf_font_desc_s
{
	fz_storable storable;
	unsigned int size;

	fz_font *font;

	/* FontDescriptor */
	int flags;
	float italic_angle;
	float ascent;
	float descent;
	float cap_height;
	float x_height;
	float missing_width;

	/* Encoding (CMap) */
	pdf_cmap *encoding;
	pdf_cmap *to_ttf_cmap;
	int cid_to_gid_len;
	unsigned short *cid_to_gid;

	/* ToUnicode */
	pdf_cmap *to_unicode;
	int cid_to_ucs_len;
	unsigned short *cid_to_ucs;

	/* Metrics (given in the PDF file) */
	int wmode;

	int hmtx_len, hmtx_cap;
	pdf_hmtx dhmtx;
	pdf_hmtx *hmtx;

	int vmtx_len, vmtx_cap;
	pdf_vmtx dvmtx;
	pdf_vmtx *vmtx;

	int is_embedded;
};

void pdf_set_font_wmode(fz_context *ctx, pdf_font_desc *font, int wmode);
void pdf_set_default_hmtx(fz_context *ctx, pdf_font_desc *font, int w);
void pdf_set_default_vmtx(fz_context *ctx, pdf_font_desc *font, int y, int w);
void pdf_add_hmtx(fz_context *ctx, pdf_font_desc *font, int lo, int hi, int w);
void pdf_add_vmtx(fz_context *ctx, pdf_font_desc *font, int lo, int hi, int x, int y, int w);
void pdf_end_hmtx(fz_context *ctx, pdf_font_desc *font);
void pdf_end_vmtx(fz_context *ctx, pdf_font_desc *font);
pdf_hmtx pdf_lookup_hmtx(fz_context *ctx, pdf_font_desc *font, int cid);
pdf_vmtx pdf_lookup_vmtx(fz_context *ctx, pdf_font_desc *font, int cid);

void pdf_load_to_unicode(pdf_document *doc, pdf_font_desc *font, char **strings, char *collection, pdf_obj *cmapstm);

int pdf_font_cid_to_gid(fz_context *ctx, pdf_font_desc *fontdesc, int cid);

unsigned char *pdf_lookup_builtin_font(char *name, unsigned int *len);
unsigned char *pdf_lookup_substitute_font(int mono, int serif, int bold, int italic, unsigned int *len);
unsigned char *pdf_lookup_substitute_cjk_font(int ros, int serif, unsigned int *len);

pdf_font_desc *pdf_load_type3_font(pdf_document *doc, pdf_obj *rdb, pdf_obj *obj);
pdf_font_desc *pdf_load_font(pdf_document *doc, pdf_obj *rdb, pdf_obj *obj);

pdf_font_desc *pdf_new_font_desc(fz_context *ctx);
pdf_font_desc *pdf_keep_font(fz_context *ctx, pdf_font_desc *fontdesc);
void pdf_drop_font(fz_context *ctx, pdf_font_desc *font);

#ifndef NDEBUG
void pdf_print_font(fz_context *ctx, pdf_font_desc *fontdesc);
#endif

/*
 * Interactive features
 */

typedef struct pdf_annot_s pdf_annot;

struct pdf_annot_s
{
	pdf_obj *obj;
	fz_rect rect;
	pdf_xobject *ap;
	fz_matrix matrix;
	pdf_annot *next;
};

fz_link_dest pdf_parse_link_dest(pdf_document *doc, pdf_obj *dest);
fz_link_dest pdf_parse_action(pdf_document *doc, pdf_obj *action);
pdf_obj *pdf_lookup_dest(pdf_document *doc, pdf_obj *needle);
pdf_obj *pdf_lookup_name(pdf_document *doc, char *which, pdf_obj *needle);
pdf_obj *pdf_load_name_tree(pdf_document *doc, char *which);

fz_link *pdf_load_link_annots(pdf_document *, pdf_obj *annots, fz_matrix page_ctm);

pdf_annot *pdf_load_annots(pdf_document *, pdf_obj *annots);
void pdf_free_annot(fz_context *ctx, pdf_annot *link);

/*
 * Page tree, pages and related objects
 */

struct pdf_page_s
{
	fz_matrix ctm; /* calculated from mediabox and rotate */
	fz_rect mediabox;
	int rotate;
	int transparency;
	pdf_obj *resources;
	pdf_obj *contents;
	fz_link *links;
	pdf_annot *annots;
};

/*
 * Content stream parsing
 */

void pdf_run_glyph(pdf_document *doc, pdf_obj *resources, fz_buffer *contents, fz_device *dev, fz_matrix ctm, void *gstate);

/*
 * PDF interface to store
 */

void pdf_store_item(fz_context *ctx, pdf_obj *key, void *val, unsigned int itemsize);
void *pdf_find_item(fz_context *ctx, fz_store_free_fn *free, pdf_obj *key);
void pdf_remove_item(fz_context *ctx, fz_store_free_fn *free, pdf_obj *key);

#endif // MUPDF_INTERNAL_H
