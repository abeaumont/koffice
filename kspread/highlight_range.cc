/* This file is part of the KDE project

   Copyright 2005 Robert Knight <robertknight@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include "highlight_range.h"


using namespace KSpread;

HighlightRange::HighlightRange(const HighlightRange& rhs)
{
    rhs._firstCell ? _firstCell=new Point(*(rhs._firstCell )) : _firstCell=0;
    rhs._lastCell  ? _lastCell=new Point(*(rhs._lastCell )) : _lastCell=0;
    _color=QColor(rhs._color);
}

void HighlightRange::getRange(Range& rg)
{
    if (!_firstCell)
    {
        rg=Range();
        return;
    }

    if (_lastCell)
    {
        rg=Range(*_firstCell,*_lastCell); 	
    }
    else
    {
        rg=Range(*_firstCell,*_firstCell);
    }
}

void HighlightRange::setArea(const QRect& newArea)
{
    Range rg;
    getRange(rg);
    
   // emit areaChanged(firstCell()->sheet,rg.range,newArea);
}


