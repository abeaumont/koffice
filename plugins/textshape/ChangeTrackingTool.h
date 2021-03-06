/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef CHANGETRACKINGTOOL_H
#define CHANGETRACKINGTOOL_H

#include <KToolBase.h>

class KCanvasBase;
class KPointerEvent;
class KoTextEditor;
class KTextShapeData;
class KViewConverter;
class TextShape;
class TrackedChangeManager;
class TrackedChangeModel;

class QModelIndex;
class QPainter;
class QRectF;
class QKeyEvent;
class QTreeView;
template <class T> class QVector;
/// This tool allows to manipulate the tracked changes of a document. You can accept or reject changes.

class ChangeTrackingTool : public KToolBase
{
    Q_OBJECT
public:
    ChangeTrackingTool(KCanvasBase *canvas);

    ~ChangeTrackingTool();

    virtual void paint(QPainter& painter, const KViewConverter& converter);
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    virtual void deactivate();

protected:
    virtual QWidget* createOptionWidget();
    virtual void mouseMoveEvent(KPointerEvent* event);
    virtual void mousePressEvent(KPointerEvent* event);

private slots:
    void acceptChange();
    void rejectChange();
    void selectedChangeChanged(QModelIndex newItem, QModelIndex previousItem);
    void setShapeData(KTextShapeData *data);
    void showTrackedChangeManager();

private:
    int pointToPosition(const QPointF & point) const;
    QVector<QRectF> *textRect(int startPosition, int endPosition);
    void updateSelectedShape(const QPointF &point);

    bool m_disableShowChangesOnExit;
    KoTextEditor *m_textEditor;
    KTextShapeData *m_textShapeData;
    KCanvasBase *m_canvas;
    TextShape *m_textShape;
    TrackedChangeModel *m_model;
    TrackedChangeManager *m_trackedChangeManager;
    QTreeView *m_changesTreeView;
};

#endif // CHANGETRACKINGTOOL_H
