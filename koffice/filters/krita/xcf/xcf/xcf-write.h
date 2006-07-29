/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __XCF_WRITE_H__
#define __XCF_WRITE_H__

#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>

// Write count integers to the file
Q_UINT32 xcf_write_int32 (QFile *fp, Q_INT32 *data, Q_INT32 count);

// Write count of floats to the file
Q_UINT32 xcf_write_float (QFile *fp, float *data, Q_INT32 count);

// Write count chars to the file
Q_UINT32 xcf_write_int8 (QFile *fp, Q_UINT8 *data, Q_INT32 count);

// Write count zero-terminated strings to the file, each string preceded by its length as an integer
Q_UINT32 xcf_write_string (QFile *fp, QCString *data, Q_INT32 count);


#endif  /* __XCF_WRITE_H__ */
