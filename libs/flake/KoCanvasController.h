/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KOCANVASVIEW_H
#define KOCANVASVIEW_H

#include <flake_export.h>

#include "KoCanvasBase.h"

#include <QWidget>
#include <QAbstractScrollArea>

class QGridLayout;
class QPaintEvent;
class QEvent;
class KoShape;
class KoToolDocker;

/**
 * This widget is a wrapper around your canvas providing scrollbars.
 * Flake does not provide a canvas, the application will have to
 * extend a QWidget and implement that themselves; but Flake does make
 * it a lot easier to do so. One of those things is this widget that
 * acts as a decorator around the canvas widget and provides
 * scrollbars and allows the canvas to be centered in the viewArea
 * <p>The using application can intantiate this class and add its
 * canvas using the setCanvas() call. Which is designed so it can be
 * called multiple times for those that wish to exchange one canvas
 * widget for another.
 *
 * Effectively, there is _one_ KoCanvasController per KoView in your
 * application.
 *
 * The canvas widget is at most as big as the viewport of the scroll
 * area, and when the view on th edocument is near its edges, smaller.
 * In your canvas widget code, you can find the right place in your
 * document in pixel coordinates by adding the documentOffset
 *
 * XXX: Maybe have different sized margins for top, left, right and
 * bottom? Now it's one config setting.
 *
 */
class FLAKE_EXPORT KoCanvasController : public QAbstractScrollArea {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent the parent this widget will belong to
     */
    explicit KoCanvasController(QWidget *parent);
    virtual ~KoCanvasController();

    /**
     * Reimplemented from QAbstractScrollArea.
     */
    void scrollContentsBy ( int dx, int dy );


    /**
     * Set the new canvas to be shown as a child
     * Calling this will emit canvasRemoved() if there was a canvas before, and will emit
     * canvasSet() with the new canvas.
     * @param canvas the new canvas. The KoCanvasBase::canvas() will be called to retrieve the
     *        actual widget which will then be added as child of this one.
     */
    void setCanvas(KoCanvasBase *canvas);
    /**
     * Return the curently set canvas
     * @return the curently set canvas
     */
    KoCanvasBase* canvas() const;

    /**
     * return the amount of pixels vertically visible of the child canvas.
     * @return the amount of pixels vertically visible of the child canvas.
     */
    int visibleHeight() const;
    /**
     * return the amount of pixels horizontally visible of the child canvas.
     * @return the amount of pixels horizontally visible of the child canvas.
     */
    int visibleWidth() const;
    /**
     * return the amount of pixels that are not visible on the left side of the canvas.
     * The leftmost pixel that is shown is returned.
     */
    int canvasOffsetX() const;
    /**
     * return the amount of pixels that are not visible on the top side of the canvas.
     * The topmost pixel that is shown is returned.
     */
    int canvasOffsetY() const;

    /**
     * Set the canvas to be displayed centered in this widget.
     * In the case that the canvas widget is smaller then this one the canvas will be centered
     * and a contrasting color used for the background.
     * @param centered center canvas if true, or aligned to the left (LTR) if false.
     *        Centered is the default value.
     */
    void centerCanvas(bool centered);
    /**
     * return the canvas centering value.
     * @return the canvas centering value
     */
    bool isCanvasCentered() const;

    /// Reimplemented from QObject
    virtual bool eventFilter(QObject* watched, QEvent* event);

    /**
     * @brief Scrolls the content of the canvas so that the given rect is visible.
     *
     * The rect is to be specified in document coordinates. The scrollbar positions
     * are changed so that the centerpoint of the rectangle is centered if possible.
     *
     * @param rect the rectangle to make visible
     */
    void ensureVisible( const QRectF &rect );

    /**
     * @brief Scrolls the content of the canvas so that the given shape is visible.
     *
     * This is just a wrapper function of the above function.
     *
     * @param shape the shape to make visible
     */
    void ensureVisible( KoShape *shape );

    /**
     * XXX
     */
    void setToolOptionWidget(QWidget *widget);

signals:
    /**
     * Emitted when a previously added canvas is about to be removed.
     * @param cv this object
     */
    void canvasRemoved(KoCanvasController* cv);
    /**
     * Emitted when a canvas is set on this widget
     * @param cv this object
     */
    void canvasSet(KoCanvasController* cv);

    /**
     * Emitted when canvasOffsetX() changes
     * @param offset the new canvas offset
     */
    void canvasOffsetXChanged(int offset);

    /**
     * Emitted when canvasOffsetY() changes
     * @param offset the new canvas offset
     */
    void canvasOffsetYChanged(int offset);

    /**
     * Emitted when the cursor is moved over the canvas widget.
     * @param pos the position in widget pixels.
     */
    void canvasMousePositionChanged(const QPoint & pos );

    /**
     * Emitted when the entire controller size changes
     * @param size the size in widget pixels.
     */
    void sizeChanged(const QSize & size );

    /**
     * XXX
     */
    void toolOptionWidgetChanged(QWidget *widget);

    /**
     * Emitted whenever the document is scrolled.
     *
     * @param point the new top-left point from which the document should
     * be drawn.
     */
    void moveDocumentOffset( const QPoint &point );


public slots:

    /**
     * Call this slot whenever the size of your document in pixels
     * changes, for instance when zooming.
     *
     * XXX: Should this be the size in document coordinates and use
     *      the viewconverter internally to resize?
     */
    void setDocumentSize( const QSize & sz );

protected slots:

    /// Called by the horizontal scrollbar when its value changes
    void updateCanvasOffsetX();

    /// Called by the vertical scrollbar when its value changes
    void updateCanvasOffsetY();

protected:

    void paintEvent( QPaintEvent * event );
    void resizeEvent(QResizeEvent * resizeEvent);
    void dragEnterEvent( QDragEnterEvent * event );
    void dropEvent( QDropEvent *event );
    void dragMoveEvent ( QDragMoveEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );

private:

    void setDocumentOffset();

    void resetScrollBars();

private:

    class Private;
    Private * const m_d;


};

class Viewport : public QWidget {

    Q_OBJECT

public:

    Viewport(KoCanvasController *parent);
    ~Viewport() {};

    void setCanvas( QWidget *canvas );
    void setDocumentSize( QSize size );

                        public slots:

                        void documentOffsetMoved( QPoint );

public:

    void handleDragEnterEvent( QDragEnterEvent *event );
    void handleDropEvent( QDropEvent *event );
    void handleDragMoveEvent ( QDragMoveEvent *event );
    void handleDragLeaveEvent( QDragLeaveEvent *event );
    void handlePaintEvent( QPainter & gc, QPaintEvent *event );

private:

    QPointF correctPosition( const QPoint &point ) const;
    void repaint( KoShape *shape );

    /**
       Decides whether the containing canvas widget should be as
       big as the viewport (i.e., no margins are visible) or whether
       there are margins to be left blank, and then places the canvas
       widget accordingly.
    */
    void resetLayout();

private:

    KoCanvasController *m_parent;
    KoShape *m_draggedShape;

    QWidget * m_canvas;
    QSize m_documentSize; // Size in pixels of the document
    QPoint m_documentOffset; // Place where the canvas widget should
    // start painting the document.
    int m_margin; // The viewport margin around the document

};

#endif
