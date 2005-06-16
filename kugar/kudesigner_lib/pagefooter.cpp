/* This file is part of the KDE project
   Copyright (C) 2003-2004 Alexander Dymo <adymo@mksat.net>

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
#include "pagefooter.h"

#ifndef PURE_QT
#include <klocale.h>
#else
#include <compat_tools.h>
#endif

#include <qpainter.h>

#include <property.h>

namespace Kudesigner {

PageFooter::PageFooter(int x, int y, int width, int height, Canvas *canvas):
    Band(x, y, width, height, canvas)
{
    props.addProperty(new Property("Height", i18n("Height"), i18n("Height"), 50, KOProperty::Integer));
    QMap<QString, QVariant> m;

    m[i18n("First Page")] = "0";
    m[i18n("Every Page")] = "1";
    m[i18n("Last Page")] = "2";

    props.addProperty(new Property("PrintFrequency", i18n("Print Frequency"), "", m, "1"));
}

void PageFooter::draw(QPainter &painter)
{
    painter.drawText(rect(), AlignVCenter | AlignLeft, i18n("Page Footer"));
    Band::draw(painter);
}

QString PageFooter::getXml()
{
    return "\t<PageFooter" + Band::getXml() + "\t</PageFooter>\n\n";
}

}
