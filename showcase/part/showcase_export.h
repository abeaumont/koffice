/*  This file is part of the KDE project
    Copyright (C) 2006 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SHOWCASE_EXPORT_H
#define SHOWCASE_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef SHOWCASE_EXPORT
# if defined(MAKE_SHOWCASEPRIVATE_LIB)
   /* We are building this library */
#  define SHOWCASE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define SHOWCASE_EXPORT KDE_IMPORT
# endif
#endif

/* Now the same for SHOWCASE_TEST_EXPORT, if compiling with unit tests enabled */

#ifdef COMPILING_TESTS
#if defined _WIN32 || defined _WIN64
# if defined(MAKE_SHOWCASEPRIVATE_LIB)
#       define SHOWCASE_TEST_EXPORT KDE_EXPORT
#   else
#       define SHOWCASE_TEST_EXPORT KDE_IMPORT
#   endif
# else /* not windows */
#   define SHOWCASE_TEST_EXPORT KDE_EXPORT
# endif
#else /* not compiling tests */
#   define SHOWCASE_TEST_EXPORT
#endif

#endif
