dnl Copyright (C) 2004 The Kaffe.org's developers. See ChangeLog for details.
dnl See the file "license.terms" for information on usage and redistribution
dnl of this file.

dnl Copyright (C) 2000-2002 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

# Test for the GNU C Library, store the version in an encoded manner.
# ac_cv_gnu_library_version=MAJOR*1000 + MINOR


AC_DEFUN([KAFFE_CHECK_GLIBC_VER],
[
    AC_CACHE_CHECK(the GNU C library version number,
      ac_cv_gnu_library_version,
      [AC_TRY_RUN([
#include <features.h>
#include <stdio.h>
#ifndef __GNU_LIBRARY__
#error No GNU Library
#endif

main()
{
  FILE *f = fopen("conftestval", "w");
  if (!f) exit(1);
  fprintf(f, "%d\n", __GLIBC__ * 1000 + __GLIBC_MINOR__);
  exit(0);
}  ], 
	ac_cv_gnu_library_version=`cat conftestval`, 
	ac_cv_gnu_library_version=no
	)
      ]
    )
  ]
)