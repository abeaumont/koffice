/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHTOOL_H
#define KOPATHTOOL_H

#include "KoPathShape.h"

#include <KoTool.h>
#include <QMap>
#include <QSet>

class KoCanvasBase;
class KoInteractionStrategy;
class KoPathPointMoveStrategy;
class KoPathPointRubberSelectStrategy;

class KoPathTool : public KoTool {
public:
    KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool();

    void paint( QPainter &painter, KoViewConverter &converter );

    void mousePressEvent( KoPointerEvent *event );
    void mouseDoubleClickEvent( KoPointerEvent *event );
    void mouseMoveEvent( KoPointerEvent *event );
    void mouseReleaseEvent( KoPointerEvent *event );
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void activate (bool temporary=false);
    void deactivate();

private:
    /**
     * @brief Select points in rect
     *
     * @param rect in document coordinated 
     * @param clearSelection if set clear the current selection before the selection
     */
    void selectPoints( const QRectF &rect, bool clearSelection );

    /// repaints the specified rect
    void repaint( const QRectF &repaintRect );
    /// returns a handle rect at the given position
    QRectF handleRect( const QPointF &p );

    // needed for interaction strategy
    QPointF m_lastPoint;
    /// snaps given point to grid point
    QPointF snapToGrid( const QPointF &p, Qt::KeyboardModifiers modifiers );
private:
    class ActiveHandle
    {
    public:    
        ActiveHandle( KoPathTool *tool, KoPathPoint *activePoint, KoPathPoint::KoPointType activePointType )
        : m_tool( tool )
        , m_activePoint( activePoint )
        , m_activePointType( activePointType )
        {}
        bool isActive() { return m_activePoint != 0; }
        void deactivate() { m_activePoint = 0; }
        void paint( QPainter &painter, KoViewConverter &converter ); 
        KoPathTool *m_tool;
        KoPathPoint *m_activePoint;
        KoPathPoint::KoPointType m_activePointType;
    };

    /**
     * @brief Handle the selection of points
     *
     * This class handles the selection of points. It makes sure 
     * the canvas is repainted when the selection changes.
     */
    class KoPathPointSelection
    {
    public:    
        KoPathPointSelection( KoPathTool * tool )
        : m_tool( tool )
        {}
        ~KoPathPointSelection() {}

        /// @brief Draw the selected points
        void paint( QPainter &painter, KoViewConverter &converter ); 
        
        /**
         * @brief Add a point to the selection
         *
         * @param point to add to the selection
         * @param clear if true the selection will be cleared before adding the point
         */
        void add( KoPathPoint * point, bool clear );
        
        /**
         * @brief Remove a point form the selection
         *
         * @param point to remove from the selection
         */
        void remove( KoPathPoint * point );
        
        /**
         * @brief Clear the selection
         */
        void clear();

        /**
         * @brief Get the number of path objects in the selection
         *
         * @return number of path object in the point selection
         */
        int objectCount() const { return m_shapePointMap.size(); }

        /**
         * @brief Get the number of path objects in the selection
         *
         * @return number of points in the selection
         */
        int size() const { return m_selectedPoints.size(); }

        /**
         * @brief Check if a point is in the selection
         *
         * @return true when the point is in the selection, false otherwise
         */
        bool contains( KoPathPoint * point ) { return m_selectedPoints.contains( point ); }

        /**
         * @brief Get all selected points
         *
         * @return set of selected points
         */
        const QSet<KoPathPoint *> & selectedPoints() const { return m_selectedPoints; }

        /**
         * @brief Get the selected point map
         *
         * @return KoSelectedPointMap containing all objects and selected points 
         * typedef QMap<KoPathShape *, QSet<KoPathPoint *> > KoSelectedPointMap;
         */
        const KoPathShapePointMap & selectedPointMap() const { return m_shapePointMap; }

        /**
         * @brief trigger a repaint
         */
        void repaint();

    private:
        QSet<KoPathPoint *> m_selectedPoints;
        KoPathShapePointMap m_shapePointMap;
        KoPathTool * m_tool;
    };

    
    ActiveHandle m_activeHandle;       ///< the currently active handle
    int m_handleRadius;                ///< the radius of the control point handles
    /// the point selection 
    KoPathPointSelection m_pointSelection;

    friend class ActiveHandle;
    friend class KoPathPointSelection;
    friend class KoPathPointMoveStrategy;
    friend class KoPathPointRubberSelectStrategy;

    KoInteractionStrategy *m_currentStrategy; ///< the rubber selection strategy
};

#endif
