/* This file is part of the KDE project
   Copyright (C) 2002-2004 Alexander Dymo <adymo@mksat.net>

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
#include "reportitem.h"

#ifndef PURE_QT
#include <klocale.h>
#include <kglobalsettings.h>
#else
#include <compat_tools.h>
#endif

#include <qrect.h>
#include <qpainter.h>
#include <qregexp.h>

#include <property.h>

#include "canvas.h"
#include "band.h"
#include "propertyserializer.h"

namespace Kudesigner {

QRect ReportItem::topLeftResizableRect()
{
    return QRect((int)x(), (int)y(), HolderSize, HolderSize);
}

QRect ReportItem::bottomLeftResizableRect()
{
    return QRect((int)x(), (int)(y()+height()-HolderSize), HolderSize, HolderSize);
}

QRect ReportItem::topRightResizableRect()
{
    return QRect((int)(x()+width()-HolderSize), (int)y(), HolderSize, HolderSize);
}

QRect ReportItem::bottomRightResizableRect()
{
    return QRect((int)(x()+width()-HolderSize), (int)(y()+height()-HolderSize), HolderSize, HolderSize);
}

QRect ReportItem::topMiddleResizableRect()
{
    return QRect((int)(x()+width()/2-HolderSize/2.), (int)y(), HolderSize, HolderSize);
}

QRect ReportItem::bottomMiddleResizableRect()
{
    return QRect((int)(x()+width()/2-HolderSize/2.), (int)(y()+height()-HolderSize), HolderSize, HolderSize);
}

QRect ReportItem::leftMiddleResizableRect()
{
    return QRect((int)x(), (int)(y()+height()/2-HolderSize/2.), HolderSize, HolderSize);
}

QRect ReportItem::rightMiddleResizableRect()
{
    return QRect((int)(x()+width()-HolderSize), (int)(y()+height()/2-HolderSize/2.), HolderSize, HolderSize);
}

void ReportItem::updateGeomProps()
{
    if (!section()) return;
    props["X"].setValue((int)(x()-section()->x()));
    props["Y"].setValue((int)(y()-section()->y()));
    props["Width"].setValue(width());
    props["Height"].setValue(height());
}

Band *ReportItem::section()
{
    return parentSection;
}

void ReportItem::setSection(Band *section)
{
    props["X"].setValue((int)(x() - section->x()));
    props["Y"].setValue((int)(y() - section->y()));
    parentSection = section;
}

void ReportItem::setSectionUndestructive(Band *section)
{
    parentSection = section;
}

QString ReportItem::getXml()
{
    QString result = "";

    int i = 1;
    for (PtrList::Iterator it( props ); it.current(); ++it)
    {
        if (!(i%3)) result += "\n\t\t  ";
        result += " " + it.currentKey() + "=" + "\"" + escape(PropertySerializer::toString(it.current())) + "\"";
        i++;
    }

    return result;
}

int ReportItem::isInHolder(const QPoint p)
{
    if (topLeftResizableRect().contains(p)) return (ResizeTop | ResizeLeft);
    if (bottomLeftResizableRect().contains(p)) return (ResizeBottom | ResizeLeft);
    if (leftMiddleResizableRect().contains(p)) return (ResizeLeft);
    if (bottomMiddleResizableRect().contains(p)) return (ResizeBottom);
    if (topMiddleResizableRect().contains(p)) return (ResizeTop);
    if (topRightResizableRect().contains(p)) return (ResizeTop | ResizeRight);
    if (bottomRightResizableRect().contains(p)) return (ResizeBottom | ResizeRight);
    if (rightMiddleResizableRect().contains(p)) return (ResizeRight);

    return ResizeNothing;
}

void ReportItem::drawHolders(QPainter &painter)
{
    painter.setPen(QColor(0, 0, 0));
#ifndef PURE_QT
    painter.setBrush(KGlobalSettings::highlightColor());
#else
    painter.setBrush(QColor(160, 160, 160));
#endif
    painter.drawRect(topLeftResizableRect());
    painter.drawRect(topRightResizableRect());
    painter.drawRect(bottomLeftResizableRect());
    painter.drawRect(bottomRightResizableRect());
    painter.drawRect(topMiddleResizableRect());
    painter.drawRect(bottomMiddleResizableRect());
    painter.drawRect(leftMiddleResizableRect());
    painter.drawRect(rightMiddleResizableRect());
}

QString ReportItem::escape( QString string )
{
    string.replace(QRegExp("&"), "&amp;");
    string.replace(QRegExp("<"), "&lt;");
    string.replace(QRegExp(">"), "&gt;");
    return string;
}

}
