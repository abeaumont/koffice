/* This file is part of the KDE project
   Copyright (C) 1999 Werner Trobin <wtrobin@carinthia.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// PCD, Piece Descriptor

#ifndef PCD_H
#define PCD_H

struct PCD {
    unsigned short fNoParaLast:1;
    unsigned short fPaphNil:1;
    unsigned short fCopied:1;
    unsigned short reserved:5;
    unsigned short fn:8;
    long fc;
    unsigned short prm;
    bool unicode;
};
#endif // PCD_H
