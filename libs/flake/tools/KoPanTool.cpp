/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPanTool.h"
#include "KoTool_p.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"
#include "KoViewConverter.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <kdebug.h>

KoPanTool::KoPanTool(KoCanvasBase *canvas)
        : KoTool(canvas),
        m_controller(0),
        m_temporary(false)
{
}

bool KoPanTool::wantsAutoScroll()
{
    return false;
}

void KoPanTool::mousePressEvent(KoPointerEvent *event)
{
    m_lastPosition = documentToViewport(event->point);
    event->accept();
    useCursor(QCursor(Qt::ClosedHandCursor));
}

void KoPanTool::mouseMoveEvent(KoPointerEvent *event)
{
    Q_ASSERT(m_controller);
    if (event->buttons() == 0)
        return;
    event->accept();

    QPointF actualPosition = documentToViewport(event->point);
    QPointF distance(m_lastPosition - actualPosition);
    m_controller->pan(distance.toPoint());

    m_lastPosition = actualPosition;
}

void KoPanTool::mouseReleaseEvent(KoPointerEvent *event)
{
    event->accept();
    useCursor(QCursor(Qt::OpenHandCursor));
    if (m_temporary)
        emit done();
}

void KoPanTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Up:
            m_controller->pan(QPoint(0, -m_controller->verticalScrollBar()->singleStep()));
            break;
        case Qt::Key_Down:
            m_controller->pan(QPoint(0, m_controller->verticalScrollBar()->singleStep()));
            break;
        case Qt::Key_Left:
            m_controller->pan(QPoint(-m_controller->horizontalScrollBar()->singleStep(), 0));
            break;
        case Qt::Key_Right:
            m_controller->pan(QPoint(m_controller->horizontalScrollBar()->singleStep(), 0));
            break;
    }
    event->accept();
}

void KoPanTool::activate(bool temporary)
{
    if (m_controller == 0)
        emit done();
    m_temporary = temporary;
    useCursor(QCursor(Qt::OpenHandCursor));
}

void KoPanTool::customMoveEvent(KoPointerEvent * event)
{
    m_controller->pan(QPoint(-event->x(), -event->y()));
    event->accept();
}

QPointF KoPanTool::documentToViewport(const QPointF &p)
{
    Q_D(KoTool);
    QPointF viewportPoint = d->canvas->viewConverter()->documentToView(p);
    viewportPoint += d->canvas->documentOrigin();
    viewportPoint += QPoint(m_controller->canvasOffsetX(), m_controller->canvasOffsetY());

    return viewportPoint;
}
