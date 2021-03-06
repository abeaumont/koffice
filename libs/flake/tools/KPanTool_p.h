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
#ifndef KPANTOOL_H
#define KPANTOOL_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "KToolBase.h"

#include <QPointF>

class KCanvasController;

#define KoPanTool_ID "PanTool"

/**
 * This is the tool that allows you to move the canvas by dragging it and 'panning' around.
 */
class KPanTool : public KToolBase
{
public:
    /**
     * Constructor.
     * @param canvas the canvas this tool works on.
     */
    explicit KPanTool(KCanvasBase *canvas);

    /// reimplemented from superclass
    virtual void mousePressEvent(KPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseMoveEvent(KPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseReleaseEvent(KPointerEvent *event);
    /// reimplemented from superclass
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented from superclass
    virtual void paint(QPainter &, const KViewConverter &) {}
    /// reimplemented from superclass
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    /// reimplemented method
    virtual void customMoveEvent(KPointerEvent *event);

    /// set the canvasController this tool works on.
    void setCanvasController(KCanvasController *controller) {
        m_controller = controller;
    }

private:
    QPointF documentToViewport(const QPointF &p);
    KCanvasController *m_controller;
    QPointF m_lastPosition;
    bool m_temporary;
    Q_DECLARE_PRIVATE(KToolBase)
};

#endif
