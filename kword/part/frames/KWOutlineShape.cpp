/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KWOutlineShape.h"
#include "KWFrame.h"

#include <KCanvasBase.h>
#include <KShapeSelection.h>
#include <KShapeGroup.h>
#include <KShapeManager.h>
#include <KViewConverter.h>

#include <KDebug>
#include <QPen>
#include <QPainter>

KWOutlineShape::KWOutlineShape(KWFrame *frame)
        : m_paintOutline(false)
{
    setShapeId(KoPathShapeId);
    setApplicationData(frame);

    class MyGroup : public KShapeGroup
    {
    public:
        ~MyGroup() {
            setApplicationData(0); // make sure deleting this will not delete the parent frame.
        }
    };
    KShapeGroup *group = new MyGroup();
    group->setSize(QSize(1, 1));
    group->setApplicationData(frame);

    KShape *child = frame->shape();
    group->setTransformation(child->absoluteTransformation(0));
    QTransform matrix;
    child->setTransformation(matrix);

    const QSizeF s = child->size();
    // init with a simple rect as the outline of the original.
    moveTo(QPointF(0, 0));
    lineTo(QPointF(s.width(), 0));
    lineTo(QPointF(s.width(), s.height()));
    lineTo(QPointF(0, s.height()));
    close();
    group->setZIndex(child->zIndex());

    group->addShape(this);
    group->addShape(child);
}

KWOutlineShape::~KWOutlineShape()
{
}

void KWOutlineShape::paintDecorations(QPainter &painter, const KViewConverter &converter, const KCanvasBase *canvas)
{
    if (! canvas->shapeManager()->selection()->isSelected(this))
        return;

    applyConversion(painter, converter);

    QPen pen = QPen(canvas->resourceManager()->colorResource(KWord::FrameOutlineColor));
    QPointF onePixel = converter.viewToDocument(QPointF(1, 1));
    pen.setWidthF(onePixel.x());
    painter.strokePath(outline(), pen);
}

