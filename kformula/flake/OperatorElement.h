/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef OPERATORELEMENT_H
#define OPERATORELEMENT_H

#include "TokenElement.h"
#include "kformula_export.h"

/**
 * @short Implementation of the MathML mo element
 *
 * 
 */
class KOFORMULA_EXPORT OperatorElement : public TokenElement {
public:
    /// The standart constructor
    OperatorElement( BasicElement* parent = 0 );

    /// @return The string to be painted - probably a parsed rawString
    QString stringToRender( const QString& rawString ) const;

    /// @return The element's ElementType
    ElementType elementType() const;
};

#endif // OPERATORELEMENT_H
