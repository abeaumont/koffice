/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCViewModeNotes.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include <KDebug>

#include <KResourceManager.h>
#include <KoRuler.h>
#include <KShapeSelection.h>
#include <KShapeLayer.h>
#include <KShapeManager.h>
#include <KOdfText.h>
#include <KToolManager.h>
#include <KToolProxy.h>
#include <KoZoomController.h>

#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAPage.h>
#include <KoPAMasterPage.h>
#include <KoPAView.h>

#include "SCNotes.h"
#include "SCPage.h"

SCViewModeNotes::SCViewModeNotes(KoPAView *view, KoPACanvas *canvas)
    : KoPAViewMode(view, canvas)
{
}

SCViewModeNotes::~SCViewModeNotes()
{
}

void SCViewModeNotes::paint(KoPACanvas* canvas, QPainter &painter, const QRectF &paintRect)
{
#ifdef NDEBUG
    Q_UNUSED(canvas);
#endif
    Q_ASSERT(m_canvas == canvas);

    painter.translate(-m_canvas->documentOffset());
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF clipRect = paintRect.translated(m_canvas->documentOffset());
    painter.setClipRect(clipRect);

    KViewConverter *converter = m_view->viewConverter(m_canvas);
    m_canvas->shapeManager()->paint(painter, *converter, false);
    m_toolProxy->paint(painter, *converter);

}

void SCViewModeNotes::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    m_toolProxy->tabletEvent(event, point);
}

void SCViewModeNotes::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mousePressEvent(event, point);
}

void SCViewModeNotes::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseDoubleClickEvent(event, point);
}

void SCViewModeNotes::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseMoveEvent(event, point);
}

void SCViewModeNotes::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseReleaseEvent(event, point);
}

void SCViewModeNotes::keyPressEvent(QKeyEvent *event)
{
    m_toolProxy->keyPressEvent(event);
    KoPageApp::PageNavigation pageNavigation;

    if (!event->isAccepted()) {
        event->accept();

        switch (event->key()) {
            case Qt::Key_Home:
                pageNavigation = KoPageApp::PageFirst;
                break;
            case Qt::Key_PageUp:
                pageNavigation = KoPageApp::PagePrevious;
                break;
            case Qt::Key_PageDown:
                pageNavigation = KoPageApp::PageNext;
                break;
            case Qt::Key_End:
                pageNavigation = KoPageApp::PageLast;
                break;
            default:
                event->ignore();
                return;
        }

        KoPAPage *activePage = m_view->activePage();
        KoPAPage *newPage = m_view->kopaDocument()->pageByNavigation(activePage, pageNavigation);

        if (newPage != activePage) {
            updateActivePage(newPage);
        }
    }
}

void SCViewModeNotes::keyReleaseEvent(QKeyEvent *event)
{
    m_toolProxy->keyReleaseEvent(event);
}

void SCViewModeNotes::wheelEvent(QWheelEvent *event, const QPointF &point)
{
    m_toolProxy->wheelEvent(event, point);
}

void SCViewModeNotes::activate(KoPAViewMode *previousViewMode)
{
    Q_UNUSED(previousViewMode);
    m_canvas->resourceManager()->setResource(KOdfText::ShowTextFrames, true);
    m_view->setActionEnabled(KoPAView::AllActions, false);
    updateActivePage(m_view->activePage());
}

void SCViewModeNotes::deactivate()
{
    m_canvas->resourceManager()->setResource(KOdfText::ShowTextFrames, 0);
    m_view->setActionEnabled(KoPAView::AllActions, true);
    m_view->doUpdateActivePage(m_view->activePage());
}

void SCViewModeNotes::updateActivePage(KoPAPage *page)
{
    if (m_view->activePage() != page) {
        m_view->setActivePage(page);
    }

    SCPage *prPage = dynamic_cast<SCPage *>(page);
    if (!prPage) return;

    SCNotes *notes = prPage->pageNotes();
    notes->updatePageThumbnail();
    KShapeLayer* layer = dynamic_cast<KShapeLayer*>(notes->shapes().last());

    KOdfPageLayoutData layout = notes->pageLayout();
    QSize size(layout.width, layout.height);

    KoPAView *view = dynamic_cast<KoPAView*>(m_view);
    if (view) {
        view->horizontalRuler()->setRulerLength(layout.width);
        view->verticalRuler()->setRulerLength(layout.height);
        view->horizontalRuler()->setActiveRange(layout.leftMargin, layout.width - layout.rightMargin);
        view->verticalRuler()->setActiveRange(layout.topMargin, layout.height - layout.bottomMargin);
    }

    m_canvas->setDocumentOrigin(QPointF(0, 0));
    m_view->zoomController()->setPageSize(size);
    m_view->zoomController()->setDocumentSize(size);
    m_canvas->repaint();

    m_canvas->shapeManager()->setShapes(layer->shapes());

    m_view->updatePageNavigationActions();

    KShapeSelection *selection = m_canvas->shapeManager()->selection();
    selection->select(notes->textShape());
    selection->setActiveLayer(layer);
    QString tool = KToolManager::instance()->preferredToolForSelection(selection->selectedShapes());
    KToolManager::instance()->switchToolRequested(tool);
}

void SCViewModeNotes::addShape(KShape *shape)
{
    KShape *parent = shape;
    SCNotes *notes = 0;
    // similar to KoPADocument::pageByShape()
    while (!notes && (parent = parent->parent())) {
        notes = dynamic_cast<SCNotes *>(parent);
    }

    if (notes) {
        SCPage *activePage = static_cast<SCPage *>(m_view->activePage());
        if (notes == activePage->pageNotes()) {
            m_view->kopaCanvas()->shapeManager()->addShape(shape);
        }
    }
}
