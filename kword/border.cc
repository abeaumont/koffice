/* This file is part of the KDE project
   Copyright (C) 2000 Thomas Zander <zander@earthling.net>

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

#include "border.h"
#include <klocale.h>

Border::Border()
    : color( Qt::black ), style( SOLID ), ptWidth( 1 ) { }

bool Border::operator==( const Border _brd ) const {
    return ( style == _brd.style && color == _brd.color && ptWidth == _brd.ptWidth );
}

bool Border::operator!=( const Border _brd ) const {
    return ( style != _brd.style || color != _brd.color || ptWidth != _brd.ptWidth );
}

Border::BorderStyle Border::getStyle(const QString &style) {
    if ( style == i18n( "dash line ( ---- )" ) )
        return Border::DASH;
    if ( style == i18n( "dot line ( **** )" ) )
        return Border::DOT;
    if ( style == i18n( "dash dot line ( -*-* )" ) )
        return Border::DASH_DOT;
    if ( style == i18n( "dash dot dot line ( -**- )" ) )
        return Border::DASH_DOT_DOT;

    // default
    return Border::SOLID;
}

