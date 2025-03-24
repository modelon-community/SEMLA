/*
    Copyright (C) 2022 Modelica Association
    Copyright (C) 2015 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the BSD style license.

     This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    BSD_License.txt file for more details.

    You should have received a copy of the BSD_License.txt file
    along with this program. If not, contact Modelon AB
   <http://www.modelon.com>.
*/

/* USAGE
 * =====
 * You also need to add "#define _GNU_SOURCE"
 * before "#include <stdio.h>" to make it work on Linux
 * (because we use the usual (GNU extension) 'asprintf()' on Linux)
 */

/* The code in this file is derived from https://stackoverflow.com/a/49873938 */

#ifndef ASPRINTF_H
#define ASPRINTF_H

#include <stdarg.h> /* needed for va_*         */

/*
 * vscprintf:
 * MSVC implements this as _vscprintf, thus we just 'symlink' it here
 * GNU-C-compatible compilers do not implement this, thus we implement it here
 */
#ifdef _MSC_VER
#define vscprintf _vscprintf
#endif

#ifdef __GNUC__
int vscprintf(const char *format, va_list ap);
#endif

/*
 * asprintf, vasprintf:
 * MSVC does not implement these, thus we implement them here
 * GNU-C-compatible compilers implement these with the same names, thus we
 * don't have to do anything
 */
#ifdef _MSC_VER
int vasprintf(char **strp, const char *format, va_list ap);

int asprintf(char **strp, const char *format, ...);

#endif

#endif // ASPRINTF_H