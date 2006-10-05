/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathPointMoveStrategy.h"

#include "KoPathCommand.h"
#include "KoPathTool.h"

KoPathPointMoveStrategy::KoPathPointMoveStrategy( KoPathTool *tool, KoCanvasBase *canvas, const QPointF &pos )
: KoInteractionStrategy( tool, canvas )
, m_lastPosition( pos )
, m_move( 0, 0 )
, m_tool( tool )
{
}

KoPathPointMoveStrategy::~KoPathPointMoveStrategy() 
{
}

void KoPathPointMoveStrategy::handleMouseMove( const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers )
{
    QPointF docPoint = m_tool->snapToGrid( mouseLocation, modifiers );
    QPointF move = m_tool->m_pathShape->documentToShape( docPoint ) - m_tool->m_pathShape->documentToShape( m_lastPosition );
    // as the last position can change when the top left is changed we have
    // to save it in document pos and not in shape pos
    m_lastPosition = docPoint;

    m_move += move;

    // only multiple nodes can be moved at once
    if( m_tool->m_activePointType == KoPathPoint::Node )
    {
        KoPointMoveCommand cmd( m_tool->m_pathShape, m_tool->m_selectedPoints, move );
        cmd.execute();
    }
    else
    {
        KoPointMoveCommand cmd( m_tool->m_pathShape, m_tool->m_activePoint, move, m_tool->m_activePointType );
        cmd.execute();
    }
}

void KoPathPointMoveStrategy::finishInteraction( Qt::KeyboardModifiers modifiers ) 
{ 
    if( m_move.isNull() )
    {
        m_tool->selectPoints( m_tool->handleRect( m_lastPosition ), !( modifiers & Qt::ControlModifier ) );
    }
}

KCommand* KoPathPointMoveStrategy::createCommand()
{
    KoPointMoveCommand *cmd = 0;
    if( !m_move.isNull() )
    {
        // only multiple nodes can be moved at once
        if( m_tool->m_activePointType == KoPathPoint::Node )
            cmd = new KoPointMoveCommand( m_tool->m_pathShape, m_tool->m_selectedPoints, m_move );
        else
            cmd = new KoPointMoveCommand( m_tool->m_pathShape, m_tool->m_activePoint, m_move, m_tool->m_activePointType );
    }
    return cmd;
}
