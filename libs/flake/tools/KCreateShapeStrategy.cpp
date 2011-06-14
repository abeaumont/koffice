/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KCreateShapeStrategy_p.h"
#include "KoShapeRubberSelectStrategy_p.h"
#include "KCreateShapesTool.h"
#include "KoShape.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KCanvasBase.h"
#include "KoSelection.h"
#include "KoShapeFactoryBase.h"
#include "KoShapeController.h"

#include <QPainter>

#include <kdebug.h>

KCreateShapeStrategy::KCreateShapeStrategy(KCreateShapesTool *tool, const QPointF &clicked)
        : KoShapeRubberSelectStrategy(tool, clicked, tool->canvas()->snapToGrid())
{
    KCreateShapesTool *parent = static_cast<KCreateShapesTool*>(d_ptr->tool);
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(parent->shapeId());
    if (factory) {
        const KProperties *props = parent->shapeProperties();
        KoShape *shape;
        if (props) {
            shape = factory->createShape(props);
        } else {
            shape = factory->createDefaultShape();
        }

        m_outline = shape->outline();
        m_outlineBoundingRect = m_outline.boundingRect();
        delete shape;
    }
}

QUndoCommand* KCreateShapeStrategy::createCommand(QUndoCommand *parentCommand)
{
    Q_D(KoShapeRubberSelectStrategy);
    KCreateShapesTool *parent = static_cast<KCreateShapesTool*>(d_ptr->tool);
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(parent->shapeId());
    if (! factory) {
        kWarning(30006) << "Application requested a shape that is not registered" << parent->shapeId();
        return 0;
    }

    const KProperties *props = parent->shapeProperties();
    KoShape *shape;
    if (props)
        shape = factory->createShape(props, parent->canvas()->shapeController()->resourceManager());
    else
        shape = factory->createDefaultShape(parent->canvas()->shapeController()->resourceManager());
    if (shape->shapeId().isEmpty())
        shape->setShapeId(factory->id());
    QRectF rect = d->selectedRect();
    shape->setPosition(rect.topLeft());
    QSizeF newSize = rect.size();
    // if the user has dragged when creating the shape,
    // resize the shape to the dragged size
    if (newSize.width() > 1.0 && newSize.height() > 1.0)
        shape->setSize(newSize);

    QUndoCommand * cmd = parent->canvas()->shapeController()->addShape(shape, parentCommand);
    if (cmd) {
        KoSelection *selection = parent->canvas()->shapeManager()->selection();
        selection->deselectAll();
        selection->select(shape);
    }
    return cmd;
}

void KCreateShapeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    Q_D(KoShapeRubberSelectStrategy);
    d->tool->canvas()->updateCanvas(d->selectedRect());
}

void KCreateShapeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoShapeRubberSelectStrategy);
    if (m_outline.isEmpty())
        KoShapeRubberSelectStrategy::paint(painter, converter);
    else {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, false);

        QColor selectColor(Qt::blue);   // TODO make configurable
        selectColor.setAlphaF(0.5);
        QBrush sb(selectColor, Qt::SolidPattern);
        painter.setPen(QPen(sb, 0));
        painter.setBrush(sb);
        QRectF paintRect = converter.documentToView(d->selectedRect());

        qreal xscale = paintRect.width() / m_outlineBoundingRect.width();
        qreal yscale = paintRect.height() / m_outlineBoundingRect.height();
        QTransform matrix;
        matrix.translate(-m_outlineBoundingRect.left(), -m_outlineBoundingRect.top());
        matrix.scale(xscale, yscale);
        painter.translate(paintRect.left(), paintRect.top());

        if (painter.hasClipping())
            paintRect = paintRect.intersect(painter.clipRegion().boundingRect());

        painter.setTransform(matrix, true);
        painter.drawPath(m_outline);
        painter.restore();
    }
}

void KCreateShapeStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_D(KoShapeRubberSelectStrategy);
    QPointF p(point);
    if (modifiers & Qt::ControlModifier) { // keep aspect ratio
        QRectF newRect(d->selectRect);
        newRect.setBottomRight(point);
        if (newRect.width() > newRect.height())
            newRect.setWidth(newRect.height());
        else
            newRect.setHeight(newRect.width());
        p = newRect.bottomRight();
    }

    KoShapeRubberSelectStrategy::handleMouseMove(p, modifiers);
    if (! m_outline.isEmpty())
        d->tool->canvas()->updateCanvas(d->selectedRect());
}