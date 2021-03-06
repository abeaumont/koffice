/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "RectangleShapeFactory.h"
#include "RectangleShape.h"
#include "RectangleShapeConfigWidget.h"
#include "KLineBorder.h"
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KGradientBackground.h>
#include <KShapeLoadingContext.h>

#include <klocale.h>

RectangleShapeFactory::RectangleShapeFactory(QObject *parent)
: KShapeFactoryBase(parent, RectangleShapeId, i18n("Rectangle"))
{
    setToolTip(i18n("A rectangle"));
    setIcon("rectangle-shape");
    setFamily("geometric");
    setOdfElementNames(KOdfXmlNS::draw, QStringList("rect"));
    setLoadingPriority(1);
}

KShape *RectangleShapeFactory::createDefaultShape(KResourceManager *) const
{
    RectangleShape *rect = new RectangleShape();

    rect->setBorder(new KLineBorder(1.0));
    rect->setShapeId(KoPathShapeId);

    QLinearGradient *gradient = new QLinearGradient(QPointF(0,0), QPointF(1,1));
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);

    gradient->setColorAt(0.0, Qt::white);
    gradient->setColorAt(1.0, Qt::green);
    rect->setBackground(new KGradientBackground(gradient));

    return rect;
}

bool RectangleShapeFactory::supports(const KXmlElement & e, KShapeLoadingContext &/*context*/) const
{
    Q_UNUSED(e);
    return (e.localName() == "rect" && e.namespaceURI() == KOdfXmlNS::draw);
}

KShapeConfigWidgetBase *RectangleShapeFactory::createConfigWidget(KCanvasBase *canvas)
{
    return new RectangleShapeConfigWidget(canvas);
}
