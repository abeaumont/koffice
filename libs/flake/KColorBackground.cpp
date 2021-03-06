/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KColorBackground.h"
#include "KShapeSavingContext.h"
#include <KOdf.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>

#include <QtGui/QColor>
#include <QtGui/QPainter>

class KColorBackground::Private
{
public:
    Private() {
        color = Qt::black;
        style = Qt::SolidPattern;
    };
    Qt::BrushStyle style;
    QColor color;
};

KColorBackground::KColorBackground()
        : d(new Private())
{
}

KColorBackground::KColorBackground(const QColor &color, Qt::BrushStyle style)
        : d(new Private())
{
    if (style < Qt::SolidPattern || style >= Qt::LinearGradientPattern)
        style = Qt::SolidPattern;
    d->style = style;
    d->color = color;
}

KColorBackground::~KColorBackground()
{
    delete d;
}

QColor KColorBackground::color() const
{
    return d->color;
}

void KColorBackground::setColor(const QColor &color)
{
    d->color = color;
}

Qt::BrushStyle KColorBackground::style() const
{
    return d->style;
}

void KColorBackground::paint(QPainter &painter, const QPainterPath &fillPath) const
{
    painter.setBrush(QBrush(d->color, d->style));
    painter.drawPath(fillPath);
}

void KColorBackground::fillStyle(KOdfGenericStyle &style, KShapeSavingContext &context)
{
    KOdf::saveOdfFillStyle(style, context.mainStyles(), QBrush(d->color, d->style));
}

bool KColorBackground::loadStyle(KOdfLoadingContext & context, const QSizeF &)
{
    KOdfStyleStack &styleStack = context.styleStack();
    if (! styleStack.hasProperty(KOdfXmlNS::draw, "fill"))
        return false;

    QString fillStyle = styleStack.property(KOdfXmlNS::draw, "fill");
    if (fillStyle == "solid" || fillStyle == "hatch") {
        QBrush brush = KOdf::loadOdfFillStyle(styleStack, fillStyle, context.stylesReader());
        d->color = brush.color();
        d->style = brush.style();
        return true;
    }

    return false;
}
