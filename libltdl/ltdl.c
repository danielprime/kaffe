/* ltdl.c -- system independent dlopen wrapper
   Copyright (C) 1998-1999 Free Software Foundation, Inc.
   Originally by Thomas Tanner <tanner@gmx.de>
   This file is part of GNU Libtool.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define _LTDL_COMPILE_

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STRINGS_H
#include <strings.h>
#endif

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#if HAVE_MEMORY_H
#include <memory.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#include "ltdl.h"

/* max. filename length */
#ifndef LTDL_FILENAME_MAX
# ifdef FILENAME_MAX
#  define LTDL_FILENAME_MAX FILENAME_MAX
# else
#  define LTDL_FILENAME_MAX 1024
# endif
#endif

#undef	LTDL_READTEXT_MODE
/* fopen() mode flags for reading a text file */
#ifdef _WIN32
#define LTDL_READTEXT_MODE "rt"
#else
#define LTDL_READTEXT_MODE "r"
#endif

#undef	LTDL_SYMBOL_LENGTH
/* This is the maximum symbol size that won't require malloc/free */
#define LTDL_SYMBOL_LENGTH	128

#undef	LTDL_SYMBOL_OVERHEAD
/* This accounts for the _LTX_ separator */
#define LTDL_SYMBOL_OVERHEAD	5

static const char objdir[] = LTDL_OBJDIR;
static const char shlib_ext[] = LTDL_SHLIB_EXT;

static const char unknown_error[] = "unknown error";
static const char dlopen_not_supported_error[] = "dlopen support not available";
static const char file_not_found_error[] = "file not found";
static const char no_symbols_error[] = "no symbols defined";
static const char symbol_error[] = "symbol not found";
static const char memory_error[] = "not enough memory";
static const char invalid_handle_error[] = "invalid handle";
static const char buffer_overflow_error[] = "internal buffer overflow";
static const char shutdown_error[] = "library already shutdown";

#ifndef HAVE_PRELOADED_SYMBOLS
/* If libtool won't define it, we'd better do */
const lt_dlsymlist lt_preloaded_symbols[1] = { { 0, 0 } };
#endif

static const char *last_error = 0;

lt_ptr_t (*lt_dlmalloc) __P((size_t size)) = malloc;
void	 (*lt_dlfree)  __P((lt_ptr_t ptr)) = free;

typedef struct lt_dltype_t {
	struct lt_dltype_t *next;
	const char *sym_prefix;	/* prefix for symbols */
	int (*mod_init) __P((void));
	int (*mod_exit) __P((void));
	int (*lib_open) __P((lt_dlhandle handle, const char *filename));
	int (*lib_close) __P((lt_dlhandle handle));
	lt_ptr_t (*find_sym) __P((lt_dlhandle handle, const char *symbol));
} lt_dltype_t;

#define LTDL_TYPE_TOP 0

typedef	struct lt_dlhandle_t {
	struct lt_dlhandle_t *next;
	lt_dltype_t *type;	/* dlopening interface */
	char	*filename;	/* file name */
	char	*name;		/* module name */
	int	usage;		/* usage */
	int	depcount;	/* number of dependencies */
	lt_dlhandle *deplibs;	/* dependencies */
	lt_ptr_t handle;	/* system handle */
	lt_ptr_t system;	/* system specific data */
} lt_dlhandle_t;

#if ! HAVE_STRDUP

#undef strdup
#define strdup xstrdup

static inline char *
strdup(str)
	const char *str;
{
	char *tmp;

	if (!str)
		return str;
	tmp = (char*) lt_dlmalloc(strlen(str)+1);
	if (tmp)
		strcpy(tmp, str);
	return tmp;
}

#endif

#if ! HAVE_STRCHR

# if HAVE_INDEX

#  define strchr index

# else

#  define strchr xstrchr

static inline const char*
strchr(str, ch)
	const char *str;
	int ch;
{
	const char *p;

	for (p = str; *p != (char)ch && p != '\0'; p++)
		/*NOWORK*/;

	return (*p == (char)ch) ? p : 0;
}

# endif

#endif

#if ! HAVE_STRRCHR

# if HAVE_RINDEX

#  define strrchr rindex

# else

#  define strrchr xstrrchr

static inline const char*
strrchr(str, ch)
	const char *str;
	int ch;
{
	const char *p;

	for (p = str; p != '\0'; p++)
		/*NOWORK*/;

	while (*p != (char)ch && p >= str)
		p--;

	return (*p == (char)ch) ? p : 0;
}

# endif

#endif

#if HAVE_LIBDL

/* dynamic linking with dlopen/dlsym */

#if HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#if ! HAVE_DLERROR	/* not all platforms have dlerror() */
#define	dlerror()	unknown_error
#endif

#if RTLD_GLOBAL
# define LTDL_GLOBAL	RTLD_GLOBAL
#else
# if DL_GLOBAL
#  define LTDL_GLOBAL	DL_GLOBAL
# else
#  define LTDL_GLOBAL	0
# endif
#endif

#if RTLD_NOW
# define LTDL_NOW	RTLD_NOW
#else
# if DL_NOW
#  define LTDL_NOW	DL_NOW
# else
#  define LTDL_NOW	0
# endif
#endif

static int
dl_init ()
{
	return 0;
}

static int
dl_exit ()
{
	return 0;
}

static int
dl_open (handle, filename)
	lt_dlhandle handle;
	const char *filename;
{
	handle->handle = dlopen(filename, LTDL_GLOBAL | LTDL_NOW);
	if (!handle->handle) {
		last_error = dlerror();
		return 1;
	}
	return 0;
}

static int
dl_close (handle)
	lt_dlhandle handle;
{
	if (dlclose(handle->handle) != 0) {
		last_error = dlerror();
		return 1;
	}
	return 0;
}

static lt_ptr_t
dl_sym (handle, symbol)
	lt_dlhandle handle;
	const char *symbol;
{
	lt_ptr_t address = dlsym(handle->handle, symbol);
	
	if (!address)
		last_error = dlerror();
	return address;
}

static
lt_dltype_t
#ifdef NEED_USCORE
dl = { LTDL_TYPE_TOP, "_", dl_init, dl_exit,
       dl_open, dl_close, dl_sym };
#else
dl = { LTDL_TYPE_TOP, 0, dl_init, dl_exit,
       dl_open, dl_close, dl_sym };
#endif

#undef LTDL_TYPE_TOP
#define LTDL_TYPE_TOP &dl

#endif

#if HAVE_SHL_LOAD

/* dynamic linking with shl_load (HP-UX) (comments from gmodule) */

#ifdef HAVE_DL_H
#include <dl.h>
#endif

/* some flags are missing on some systems, so we provide
 * harmless defaults.
 *
 * Mandatory:
 * BIND_IMMEDIATE  - Resolve symbol references when the library is loaded.
 * BIND_DEFERRED   - Delay code symbol resolution until actual reference.
 *
 * Optionally:
 * BIND_FIRST	   - Place the library at the head of the symbol search order.
 * BIND_NONFATAL   - The default BIND_IMMEDIATE behavior is to treat all unsatisfied
 *		     symbols as fatal.	This flag allows binding of unsatisfied code
 *		     symbols to be deferred until use.
 *		     [Perl: For certain libraries, like DCE, deferred binding often
 *		     causes run time problems.	Adding BIND_NONFATAL to BIND_IMMEDIATE
 *		     still allows unresolved references in situations like this.]
 * BIND_NOSTART	   - Do not call the initializer for the shared library when the
 *		     library is loaded, nor on a future call to shl_unload().
 * BIND_VERBOSE	   - Print verbose messages concerning possible unsatisfied symbols.
 *
 * hp9000s700/hp9000s800:
 * BIND_RESTRICTED - Restrict symbols visible by the library to those present at
 *		     library load time.
 * DYNAMIC_PATH	   - Allow the loader to dynamically search for the library specified
 *		     by the path argument.
 */

#ifndef	DYNAMIC_PATH
#define	DYNAMIC_PATH	0
#endif	/* DYNAMIC_PATH */
#ifndef	BIND_RESTRICTED
#define	BIND_RESTRICTED	0
#endif	/* BIND_RESTRICTED */

#define	LTDL_BIND_FLAGS	(BIND_IMMEDIATE | BIND_NONFATAL | BIND_VERBOSE | DYNAMIC_PATH)

static int
shl_init ()
{
	return 0;
}

static int
shl_exit ()
{
	return 0;
}

static int
shl_open (handle, filename)
	lt_dlhandle handle;
	const char *filename;
{
	handle->handle = shl_load(filename, LTDL_BIND_FLAGS, 0L);
	if (!handle->handle) {
		last_error = unknown_error;
		return 1;
	}
	return 0;
}

static int
shl_close (handle)
	lt_dlhandle handle;
{
	if (shl_unload((shl_t) (handle->handle)) != 0) {
		last_error = unknown_error;
		return 1;
	}
	return 0;
}

static lt_ptr_t
shl_sym (handle, symbol)
	lt_dlhandle handle;
	const char *symbol;
{
	lt_ptr_t address;

	if (shl_findsym((shl_t) (handle->handle), symbol, TYPE_UNDEFINED,
	    &address) != 0 || !(handle->handle) || !address) {
		last_error = unknown_error;
		return 0;
	}
	return address;
}

static
lt_dltype_t
shl = { LTDL_TYPE_TOP, 0, shl_init, shl_exit,
	shl_open, shl_close, shl_sym };

#undef LTDL_TYPE_TOP
#define LTDL_TYPE_TOP &shl

#endif

#if HAVE_DLD

/* dynamic linking with dld */

static int
dld_init ()
{
	return 0;
}

static int
dld_exit ()
{
	return 0;
}

static int
dld_open (handle, filename)
	lt_dlhandle handle;
	const char *filename;
{
	handle->handle = strdup(filename);
	if (!handle->handle) {
		last_error = no_memory_error;
		return 1;
	}
	if (dld_link(filename) != 0) {
		last_error = unknown_error;
		lt_dlfree(handle->handle);
		return 1;
	}
	return 0;
}

static int
dld_close (handle)
	lt_dlhandle handle;
{
	if (dld_unlink_by_file((char*)(handle->handle), 1) != 0) {
		last_error = unknown_error;
		return 1;
	}
	lt_dlfree(handle->filename);
	return 0;
}

static lt_ptr_t
dld_sym (handle, symbol)
	lt_dlhandle handle;
	const char *symbol;
{
	lt_ptr_t address = dld_get_func(symbol);
	
	if (!address)
		last_error = unknown_error;
	return address;
}

static
lt_dltype_t
dld = { LTDL_TYPE_TOP, 0, dld_init, dld_exit,
	dld_open, dld_close, dld_sym };

#undef LTDL_TYPE_TOP
#define LTDL_TYPE_TOP &dld

#endif

#ifdef _WIN32

/* dynamic linking for Win32 */

#include <windows.h>

static int
wll_init ()
{
	return 0;
}

static int
wll_exit ()
{
	return 0;
}

static int
wll_open (handle, filename)
	lt_dlhandle handle;
	const char *filename;
{
	handle->handle = LoadLibrary(filename);
	if (!handle->handle) {
		last_error = unknown_error;
		return 1;
	}
	return 0;
}

static int
wll_close (handle)
	lt_dlhandle handle;
{
	if (FreeLibrary(handle->handle) != 0) {
		last_error = unknown_error;
		return 1;
	}
	return 0;
}

static lt_ptr_t
wll_sym (handle, symbol)
	lt_dlhandle handle;
	const char *symbol;
{
	lt_ptr_t address = GetProcAddress(handle->handle, symbol);
	
	if (!address)
		last_error = unknown_error;
	return address;
}

static
lt_dltype_t
wll = { LTDL_TYPE_TOP, 0, wll_init, wll_exit,
	wll_open, wll_close, wll_sym };

#undef LTDL_TYPE_TOP
#define LTDL_TYPE_TOP &wll

#endif

/* emulate dynamic linking using preloaded_symbols */

typedef struct lt_dlsymlists_t {
	struct lt_dlsymlists_t *next;
	const lt_dlsymlist *syms;
} lt_dlsymlists_t;

static const lt_dlsymlist *default_preloaded_symbols = 0;
static lt_dlsymlists_t *preloaded_symbols = 0;

static int
presym_init ()
{
	preloaded_symbols = 0;
	if (default_preloaded_symbols)
		return lt_dlpreload(default_preloaded_symbols);
	return 0;
}

static int
presym_free_symlists ()
{
	lt_dlsymlists_t	*lists = preloaded_symbols;
	
	while (lists) {
		lt_dlsymlists_t	*tmp = lists;
		
		lists = lists->next;
		lt_dlfree(tmp);
	}
	preloaded_symbols = 0;
	return 0;
}

static int
presym_exit ()
{
	presym_free_symlists();
	return 0;
}

static int
presym_add_symlist (preloaded)
	const lt_dlsymlist *preloaded;
{
	lt_dlsymlists_t *tmp;
	lt_dlsymlists_t *lists = preloaded_symbols;
	
	while (lists) {
		if (lists->syms == preloaded)
			return 0;
		lists = lists->next;
	}

	tmp = (lt_dlsymlists_t*) lt_dlmalloc(sizeof(lt_dlsymlists_t));
	if (!tmp) {
		last_error = memory_error;
		return 1;
	}
	tmp->syms = preloaded;
	tmp->next = 0;
	if (!preloaded_symbols)
		preloaded_symbols = tmp;
	else {
		/* append to the end */
		lists = preloaded_symbols;
		while (lists->next)
			lists = lists->next;
		lists->next = tmp;
	}
	return 0;
}

static int
presym_open (handle, filename)
	lt_dlhandle handle;
	const char *filename;
{
	lt_dlsymlists_t *lists = preloaded_symbols;

	if (!lists) {
		last_error = no_symbols_error;
		return 1;
	}
	if (!filename)
		filename = "@PROGRAM@";
	while (lists) {
		const lt_dlsymlist *syms = lists->syms;
	
		while (syms->name) {
			if (!syms->address &&
			    strcmp(syms->name, filename) == 0) {
				handle->handle = (lt_ptr_t) syms;
				return 0;
			}
			syms++;
		}
		lists = lists->next;
	}
	last_error = file_not_found_error;
	return 1;
}

static int
presym_close (handle)
	lt_dlhandle handle;
{
	return 0;
}

static lt_ptr_t
presym_sym (handle, symbol)
	lt_dlhandle handle;
	const char *symbol;
{
	lt_dlsymlist *syms = (lt_dlsymlist*)(handle->handle);

	syms++;
	while (syms->address) {
		if (syms->address && strcmp(syms->name, symbol) == 0)
			return syms->address;
		syms++;
	}
	last_error = symbol_error;
	return 0;
}

static
lt_dltype_t
presym = { LTDL_TYPE_TOP, 0, presym_init, presym_exit,
	   presym_open, presym_close, presym_sym };

#undef LTDL_TYPE_TOP
#define LTDL_TYPE_TOP &presym

static char *user_search_path = 0;
static lt_dlhandle handles = 0;
static int initialized = 0;

static lt_dltype_t *types = LTDL_TYPE_TOP;
#undef LTDL_TYPE_TOP

int
lt_dlinit ()
{
	/* initialize libltdl */
	lt_dltype_t **type = &types;
	int typecount = 0;

	if (initialized) {	/* Initialize only at first call. */
		initialized++;
		return 0;
	}
	handles = 0;
	user_search_path = 0; /* empty search path */

	while (*type) {
		if ((*type)->mod_init())
			*type = (*type)->next; /* Remove it from the list */
		else {
			type = &(*type)->next; /* Keep it */
			typecount++;
		}
	}
	if (typecount == 0) {
		last_error = dlopen_not_supported_error;
		return 1;
	}
	last_error = 0;
	initialized = 1;
	return 0;
}

int
lt_dlpreload (preloaded)
	const lt_dlsymlist *preloaded;
{
	if (preloaded)
		return presym_add_symlist(preloaded);
	presym_free_symlists();
	if (default_preloaded_symbols)
		return lt_dlpreload(default_preloaded_symbols);
	return 0;
}

int
lt_dlpreload_default (preloaded)
	const lt_dlsymlist *preloaded;
{
	default_preloaded_symbols = preloaded;
	return 0;
}

int
lt_dlexit ()
{
	/* shut down libltdl */
	lt_dltype_t *type = types;
	int	errors;
	
	if (!initialized) {
		last_error = shutdown_error;
		return 1;
	}
	if (initialized != 1) { /* shut down only at last call. */
		initialized--;
		return 0;
	}
	/* close all modules */
	errors = 0;
	while (handles) {
		/* FIXME: what if a module depends on another one? */
		if (lt_dlclose(handles))
			errors++;
	}
	initialized = 0;
	while (type) {
		if (type->mod_exit())
			errors++;
		type = type->next;
	}
	return errors;
}

static int
tryall_dlopen (handle, filename)
	lt_dlhandle *handle;
	const char *filename;
{
	lt_dlhandle cur;
	lt_dltype_t *type = types;
	const char *saved_error = last_error;
	
	/* check whether the module was already opened */
	cur = handles;
	while (cur) {
		if (!cur->filename && !filename)
			break;
		if (cur->filename && filename && 
		    strcmp(cur->filename, filename) == 0)
			break;
		cur = cur->next;
	}
	if (cur) {
		cur->usage++;
		*handle = cur;
		return 0;
	}
	
	cur = *handle;
	if (filename) {
		cur->filename = strdup(filename);
		if (!cur->filename) {
			last_error = memory_error;
			return 1;
		}
	} else
		cur->filename = 0;
	while (type) {
		if (type->lib_open(cur, filename) == 0)
			break;
		type = type->next;
	}
	if (!type) {
		if (cur->filename)
			lt_dlfree(cur->filename);
		return 1;
	}
	cur->type = type;
	last_error = saved_error;
	return 0;
}

static int
find_module (handle, dir, libdir, dlname, old_name, installed)
	lt_dlhandle *handle;
	const char *dir;
	const char *libdir;
	const char *dlname;
	const char *old_name;
	int installed;
{
	/* try to open the old library first; if it was dlpreopened, 
	   we want the preopened version of it, even if a dlopenable
	   module is available */
	if (old_name && tryall_dlopen(handle, old_name) == 0)
		return 0;
	/* try to open the dynamic library */
	if (dlname) {
		/* try to open the installed module */
		if (installed && libdir) {
			int ret;
			char *filename = (char*)
				lt_dlmalloc(strlen(libdir)+1+strlen(dlname)+1);

			if (!filename) {
				last_error = memory_error;
				return 1;
			}
			strcpy(filename, libdir);
			strcat(filename, "/");
			strcat(filename, dlname);
			ret = tryall_dlopen(handle, filename) == 0;
			lt_dlfree(filename);
			if (ret)
				return 0;
		}
		/* try to open the not-installed module */
		if (!installed) {
			int ret;
			char *filename = (char*)
				lt_dlmalloc((dir ? strlen(dir) : 0)
				       + strlen(objdir)	+ strlen(dlname) + 1);
			
			if (!filename) {
				last_error = memory_error;
				return 1;
			}
			if (dir)
				strcpy(filename, dir);
			else
				*filename = 0;
			strcat(filename, objdir);
			strcat(filename, dlname);

			ret = tryall_dlopen(handle, filename) == 0;
			lt_dlfree(filename);
			if (ret)
				return 0;
		}
		/* hmm, maybe it was moved to another directory.
                   Should we really support this? */
		{
			int ret;
			char *filename = (char*)
				lt_dlmalloc((dir ? strlen(dir) : 0)
				       + strlen(dlname) + 1);
			if (dir)
				strcpy(filename, dir);
			else
				*filename = 0;
			strcat(filename, dlname);
			ret = tryall_dlopen(handle, filename) == 0;
			lt_dlfree(filename);
			if (ret)
				return 0;
		}
	}
	last_error = file_not_found_error;
	return 1;
}

static lt_ptr_t
find_file (basename, search_path, pdir, handle)
	const char *basename;
	const char *search_path;
	char **pdir;
	lt_dlhandle *handle;
{
	/* when handle != NULL search a library, otherwise a file */
	/* return NULL on failure, otherwise the file/handle */

	char	*filename = 0;
	int     filenamesize = 0;
	const char *next = search_path;
	int	lenbase = strlen(basename);
	
	if (!next || !*next) {
		last_error = file_not_found_error;
		return 0;
	}
	while (next) {
		int lendir;
		const char *cur = next;

		next = strchr(cur, ':');
		if (!next)
			next = cur + strlen(cur);
		lendir = next - cur;
		if (*next == ':')
			++next;
		else
			next = 0;
		if (lendir == 0)
			continue;
		if (lendir + 1 + lenbase >= filenamesize) {
			if (filename)
				lt_dlfree(filename);
			filenamesize = lendir + 1 + lenbase + 1;
			filename = (char*) lt_dlmalloc(filenamesize);
			if (!filename) {
				last_error = memory_error;
				return 0;
			}
		}
		strncpy(filename, cur, lendir);
		if (filename[lendir-1] != '/')
			filename[lendir++] = '/';
		strcpy(filename+lendir, basename);
		if (handle) {
			if (tryall_dlopen(handle, filename) == 0) {
				lt_dlfree(filename);
				return (lt_ptr_t) handle;
			}
		} else {
			FILE *file = fopen(filename, LTDL_READTEXT_MODE);
			if (file) {
				if (*pdir)
					lt_dlfree(*pdir);
				filename[lendir] = '\0';
				*pdir = strdup(filename);
				if (!*pdir) {
					/* We could have even avoided the
					   strdup, but there would be some
					   memory overhead. */
					*pdir = filename;
				} else
					lt_dlfree(filename);
				return (lt_ptr_t) file;
			}
		}
	}
	if (filename)
		lt_dlfree(filename);
	last_error = file_not_found_error;
	return 0;
}

static int
load_deplibs(handle, deplibs)
	lt_dlhandle handle;
	const char *deplibs;
{
	/* FIXME: load deplibs */
	handle->depcount = 0;
	handle->deplibs = 0;
	return 0;
}

static int
unload_deplibs(handle)
	lt_dlhandle handle;
{
	/* FIXME: unload deplibs */
	return 0;
}

static inline int
trim (dest, s)
	char **dest;
	const char *s;
{
	char *tmp;
	char *i = strrchr(s, '\'');
	int len = strlen(s);

	if (*dest)
		lt_dlfree(*dest);
	if (len > 3 && s[0] == '\'') {
		tmp = (char*) lt_dlmalloc(i - s);
		if (!tmp) {
			last_error = memory_error;
			return 1;
		}
		strncpy(tmp, &s[1], (i - s) - 1);
		tmp[len-3] = '\0';
		*dest = tmp;
	} else
		*dest = 0;
	return 0;
}

static inline int
free_vars(dir, name, dlname, oldname, libdir, deplibs)
	char *dir;
	char *name;
	char *dlname;
	char *oldname;
	char *libdir;
	char *deplibs;
{
	if (dir)
		lt_dlfree(dir);
	if (name)
		lt_dlfree(name);
	if (dlname)
		lt_dlfree(dlname);
	if (oldname)
		lt_dlfree(oldname);
	if (libdir)
		lt_dlfree(libdir);
	if (deplibs)
		lt_dlfree(deplibs);
	return 0;
}

lt_dlhandle
lt_dlopen (filename)
	const char *filename;
{
	lt_dlhandle handle, newhandle;
	const char *basename, *ext;
	const char *saved_error = last_error;
	char	*dir = 0, *name = 0;
	
	if (!filename) {
		handle = (lt_dlhandle) lt_dlmalloc(sizeof(lt_dlhandle_t));
		if (!handle) {
			last_error = memory_error;
			return 0;
		}
		handle->usage = 0;
		newhandle = handle;
		if (tryall_dlopen(&handle, 0) != 0) {
			lt_dlfree(newhandle);
			return 0;
		}
		goto register_handle;
	}
	basename = strrchr(filename, '/');
	if (basename) {
		basename++;
		dir = (char*) lt_dlmalloc(basename - filename + 1);
		if (!dir) {
			last_error = memory_error;
			return 0;
		}
		strncpy(dir, filename, basename - filename);
		dir[basename - filename] = '\0';
	} else
		basename = filename;
	/* check whether we open a libtool module (.la extension) */
	ext = strrchr(basename, '.');
	if (ext && strcmp(ext, ".la") == 0) {
		/* this seems to be a libtool module */
		FILE	*file;
		int	i;
		char	*dlname = 0, *old_name = 0;
		char	*libdir = 0, *deplibs = 0;
		int	error = 0;
		/* if we can't find the installed flag, it is probably an
		   installed libtool archive, produced with an old version
		   of libtool */
		int     installed = 1; 

		/* extract the module name from the file name */
		name = (char*) lt_dlmalloc(ext - basename + 1);
		if (!name) {
			last_error = memory_error;
			if (dir)
				lt_dlfree(dir);
			return 0;
		}
		/* canonicalize the module name */
		for (i = 0; i < ext - basename; i++)
			if (isalnum(basename[i]))
				name[i] = basename[i];
			else
				name[i] = '_';
		name[ext - basename] = '\0';
		/* now try to open the .la file */
		file = fopen(filename, LTDL_READTEXT_MODE);
		if (!file)
			last_error = file_not_found_error;
		if (!file && !dir) {
			/* try other directories */
			file = (FILE*) find_file(basename, 
						 user_search_path,
						 &dir, 0);
			if (!file)
				file = (FILE*) find_file(basename,
						 getenv("LTDL_LIBRARY_PATH"),
						 &dir, 0);
#ifdef LTDL_SHLIBPATH_VAR
			if (!file)
				file = (FILE*) find_file(basename,
						 getenv(LTDL_SHLIBPATH_VAR),
						 &dir, 0);
#endif
		}
		if (!file) {
			if (name)
				lt_dlfree(name);
			if (dir)
				lt_dlfree(dir);
			return 0;
		}
		/* read the .la file */
		while (!feof(file)) {
			char	line[LTDL_FILENAME_MAX];
			
			if (!fgets(line, sizeof(line), file))
				break;
			if (line[0] == '\n' || line[0] == '#')
				continue;
			if (strncmp(line, "dlname=", 7) == 0)
				error = trim(&dlname, &line[7]);
			else
			if (strncmp(line, "old_library=", 12) == 0)
				error = trim(&old_name, &line[12]);
			else
			if (strncmp(line, "libdir=", 7) == 0)
				error = trim(&libdir, &line[7]);
			else
			if (strncmp(line, "dl_dependency_libs=", 20) == 0)
				error = trim(&deplibs, &line[20]);
			else
			if (strcmp(line, "installed=yes\n") == 0)
				installed = 1;
			else
			if (strcmp(line, "installed=no\n") == 0)
				installed = 0;
			if (error)
				break;
		}
		fclose(file);
		/* allocate the handle */
		handle = (lt_dlhandle) lt_dlmalloc(sizeof(lt_dlhandle_t));
		if (!handle || error) {
			if (handle)
				lt_dlfree(handle);
			if (!error)
				last_error = memory_error;
			free_vars(name, dir, dlname, old_name, libdir, deplibs);
			return 0;
		}
		handle->usage = 0;
		if (load_deplibs(handle, deplibs) == 0) {
			newhandle = handle;
			/* find_module may replace newhandle */
			if (find_module(&newhandle, dir, libdir, 
					dlname, old_name, installed)) {
				unload_deplibs(handle);
				error = 1;
			}
		} else
			error = 1;
		if (error) {
			lt_dlfree(handle);
			free_vars(name, dir, dlname, old_name, libdir, deplibs);
			return 0;
		}
		if (handle != newhandle) {
			unload_deplibs(handle);
		}
	} else {
		/* not a libtool module */
		handle = (lt_dlhandle) lt_dlmalloc(sizeof(lt_dlhandle_t));
		if (!handle) {
			last_error = memory_error;
			if (dir)
				lt_dlfree(dir);
			return 0;
		}
		handle->usage = 0;
		/* non-libtool modules don't have dependencies */
		handle->depcount = 0;
		handle->deplibs = 0;
		newhandle = handle;
		if (tryall_dlopen(&newhandle, filename)
		    && (!dir
			|| (!find_file(basename, user_search_path,
					  0, &newhandle)
			    && !find_file(basename,
					  getenv("LTDL_LIBRARY_PATH"),
					  0, &newhandle)
#ifdef LTDL_SHLIBPATH_VAR
			    && !find_file(basename,
					  getenv(LTDL_SHLIBPATH_VAR),
					  0, &newhandle)
#endif
				))) {
			lt_dlfree(handle);
			if (dir)
				lt_dlfree(dir);
			return 0;
		}
	}
register_handle:
	if (newhandle != handle) {
		lt_dlfree(handle);
		handle = newhandle;
	}
	if (!handle->usage) {
		handle->usage = 1;
		handle->name = name;
		handle->next = handles;
		handles = handle;
	} else if (name)
		lt_dlfree(name);
	if (dir)
		lt_dlfree(dir);
	last_error = saved_error;
	return handle;
}

lt_dlhandle
lt_dlopenext (filename)
	const char *filename;
{
	lt_dlhandle handle;
	char	*tmp;
	int	len;
	const char *saved_error = last_error;
	
	if (!filename) {
		last_error = file_not_found_error;
		return 0;
	}
	len = strlen(filename);
	if (!len) {
		last_error = file_not_found_error;
		return 0;
	}
	/* try the normal file name */
	handle = lt_dlopen(filename);
	if (handle)
		return handle;
	/* try "filename.la" */
	tmp = (char*) lt_dlmalloc(len+4);
	if (!tmp) {
		last_error = memory_error;
		return 0;
	}
	strcpy(tmp, filename);
	strcat(tmp, ".la");
	handle = lt_dlopen(tmp);
	if (handle) {
		last_error = saved_error;
		lt_dlfree(tmp);
		return handle;
	}
#ifdef LTDL_SHLIB_EXT
	/* try "filename.EXT" */
	if (strlen(shlib_ext) > 3) {
		lt_dlfree(tmp);
		tmp = (char*) lt_dlmalloc(len + strlen(shlib_ext) + 1);
		if (!tmp) {
			last_error = memory_error;
			return 0;
		}
		strcpy(tmp, filename);
	} else
		tmp[len] = '\0';
	strcat(tmp, shlib_ext);
	handle = lt_dlopen(tmp);
	if (handle) {
		last_error = saved_error;
		lt_dlfree(tmp);
		return handle;
	}
#endif	
	last_error = file_not_found_error;
	lt_dlfree(tmp);
	return 0;
}

int
lt_dlclose (handle)
	lt_dlhandle handle;
{
	lt_dlhandle cur, last;
	
	/* check whether the handle is valid */
	last = cur = handles;
	while (cur && handle != cur) {
		last = cur;
		cur = cur->next;
	}
	if (!cur) {
		last_error = invalid_handle_error;
		return 1;
	}
	handle->usage--;
	if (!handle->usage) {
		int	error;
	
		if (handle != handles)
			last->next = handle->next;
		else
			handles = handle->next;
		error = handle->type->lib_close(handle);
		error += unload_deplibs(handle);
		lt_dlfree(handle->filename);
		if (handle->name)
			lt_dlfree(handle->name);
		lt_dlfree(handle);
		return error;
	}
	return 0;
}

lt_ptr_t
lt_dlsym (handle, symbol)
	lt_dlhandle handle;
	const char *symbol;
{
	int	lensym;
	char	lsym[LTDL_SYMBOL_LENGTH];
	char	*sym;
	lt_ptr_t address;

	if (!handle) {
		last_error = invalid_handle_error;
		return 0;
	}
	if (!symbol) {
		last_error = symbol_error;
		return 0;
	}
	lensym = strlen(symbol);
	if (handle->type->sym_prefix)
		lensym += strlen(handle->type->sym_prefix);
	if (handle->name)
		lensym += strlen(handle->name);
	if (lensym + LTDL_SYMBOL_OVERHEAD < LTDL_SYMBOL_LENGTH)
		sym = lsym;
	else
		sym = (char*) lt_dlmalloc(lensym + LTDL_SYMBOL_OVERHEAD + 1);
	if (!sym) {
		last_error = buffer_overflow_error;
		return 0;
	}
	if (handle->name) {
		/* this is a libtool module */
		if (handle->type->sym_prefix) {
			strcpy(sym, handle->type->sym_prefix);
			strcat(sym, handle->name);
		} else
			strcpy(sym, handle->name);
		strcat(sym, "_LTX_");
		strcat(sym, symbol);
		/* try "modulename_LTX_symbol" */
		address = handle->type->find_sym(handle, sym);
		if (address) {
			if (sym != lsym)
				lt_dlfree(sym);
			return address;
		}
	}
	/* otherwise try "symbol" */
	if (handle->type->sym_prefix) {
		strcpy(sym, handle->type->sym_prefix);
		strcat(sym, symbol);
	} else
		strcpy(sym, symbol);
	address = handle->type->find_sym(handle, sym);
	if (sym != lsym)
		lt_dlfree(sym);
	return address;
}

const char *
lt_dlerror ()
{
	const char *error = last_error;
	
	last_error = 0;
	return error;
}

int
lt_dladdsearchdir (search_dir)
	const char *search_dir;
{
	if (!search_dir || !strlen(search_dir))
		return 0;
	if (!user_search_path) {
		user_search_path = strdup(search_dir);
		if (!user_search_path) {
			last_error = memory_error;
			return 1;
		}
	} else {
		char	*new_search_path = (char*)
			lt_dlmalloc(strlen(user_search_path) + 
				strlen(search_dir) + 1);
		if (!new_search_path) {
			last_error = memory_error;
			return 1;
		}
		strcat(new_search_path, ":");
		strcat(new_search_path, search_dir);
		lt_dlfree(user_search_path);
		user_search_path = new_search_path;
	}
	return 0;
}

int
lt_dlsetsearchpath (search_path)
	const char *search_path;
{
	if (user_search_path)
		lt_dlfree(user_search_path);
	user_search_path = 0; /* reset the search path */
	if (!search_path || !strlen(search_path))
		return 0;
	user_search_path = strdup(search_path);
	if (!user_search_path)
		return 1;
	return 0;
}

const char *
lt_dlgetsearchpath ()
{
	return user_search_path;
}
