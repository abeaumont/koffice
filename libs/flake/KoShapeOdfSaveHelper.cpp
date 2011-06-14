/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeOdfSaveHelper.h"
#include "KDragOdfSaveHelper_p.h"

#include <KXmlWriter.h>
#include <KOdf.h>
#include <KoShape.h>

class KoShapeOdfSaveHelperPrivate : public KDragOdfSaveHelperPrivate
{
public:
    KoShapeOdfSaveHelperPrivate(QList<KoShape *> shapes)
    : shapes(shapes) {}

    QList<KoShape *> shapes;
};

KoShapeOdfSaveHelper::KoShapeOdfSaveHelper(QList<KoShape *> shapes)
        : KDragOdfSaveHelper(*(new KoShapeOdfSaveHelperPrivate(shapes)))
{
}

bool KoShapeOdfSaveHelper::writeBody()
{
    Q_D(KoShapeOdfSaveHelper);
    d->context->addOption(KoShapeSavingContext::DrawId);

    KXmlWriter &bodyWriter = d->context->xmlWriter();
    bodyWriter.startElement("office:body");
    bodyWriter.startElement(KOdf::bodyContentElement(KOdf::TextDocument, true));

    qSort(d->shapes.begin(), d->shapes.end(), KoShape::compareShapeZIndex);
    foreach (KoShape *shape, d->shapes) {
        shape->saveOdf(*d->context);
    }
    d->context->writeConnectors();

    bodyWriter.endElement(); // office:element
    bodyWriter.endElement(); // office:body

    return true;
}
