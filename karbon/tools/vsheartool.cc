/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <math.h>

#include <qcursor.h>

#include <koRect.h>

#include "karbon_part.h"
#include "karbon_view.h"
#include "vhandletool.h"
#include "vpainter.h"
#include "vpainterfactory.h"
#include "vsheartool.h"
#include "vtransformcmd.h"


VShearTool* VShearTool::s_instance = 0L;

VShearTool::VShearTool( KarbonPart* part )
	: VTool( part ), m_isDragging( false )
{
}

VShearTool::~VShearTool()
{
}

VShearTool*
VShearTool::instance( KarbonPart* part )
{
	if ( s_instance == 0L )
	{
		s_instance = new VShearTool( part );
	}

	s_instance->m_part = part;
	return s_instance;
}

void
VShearTool::setCursor( KarbonView* view ) const
{
	switch( VHandleTool::instance( m_part )->activeNode() )
	{
		case node_lt:
		case node_rb:
			view->canvasWidget()->viewport()->
				setCursor( QCursor( Qt::SizeFDiagCursor ) );
			break;
		case node_rt:
		case node_lb:
			view->canvasWidget()->viewport()->
				setCursor( QCursor( Qt::SizeBDiagCursor ) );
			break;
		case node_lm:
		case node_rm:
			view->canvasWidget()->viewport()->
				setCursor( QCursor( Qt::SizeHorCursor ) );
			break;
		case node_mt:
		case node_mb:
			view->canvasWidget()->viewport()->
				setCursor( QCursor( Qt::SizeVerCursor ) );
			break;
		default:
			view->canvasWidget()->viewport()->
				setCursor( QCursor( Qt::arrowCursor ) );
	}
}

void
VShearTool::drawTemporaryObject( KarbonView* view )
{
	VPainter *painter = view->painterFactory()->editpainter();
	painter->setRasterOp( Qt::NotROP );

	// already selected, so must be a handle operation (move, scale etc.)
	if( !part()->document().selection().isEmpty() && VHandleTool::instance( m_part )->activeNode() != node_mm )
	{
		KoRect rect = part()->document().selection().boundingBox();

		if( VHandleTool::instance( m_part )->activeNode() == node_lt )
		{
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_mt )
		{
			m_s1 = 0;
			m_s2 = ( m_lp.y() - m_fp.y() ) / double( ( rect.height() / 2 ) * view->zoom() );
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_rt )
		{
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_rm)
		{
			m_s1 = ( m_lp.x() - m_fp.x() ) / double( ( rect.width() / 2 ) * view->zoom() );
			m_s2 = 0;
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_rb )
		{
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_mb )
		{
			m_s1 = 0;
			m_s2 = ( m_lp.y() - m_fp.y() ) / double( ( rect.height() / 2 ) * view->zoom() );
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_lb )
		{
		}
		else if( VHandleTool::instance( m_part )->activeNode() == node_lm )
		{
			m_s1 = ( m_lp.x() - m_fp.x() ) / double( ( rect.width() / 2 ) * view->zoom() );
			m_s2 = 0;
		}
		// shear operation
		QWMatrix mat;
		mat.translate( m_fp.x() / view->zoom(), m_fp.y() / view->zoom() );
		mat.shear( m_s1, m_s2 );
		mat.translate(	- ( m_fp.x() + view->canvasWidget()->contentsX() ) / view->zoom(),
						- ( m_fp.y() + view->canvasWidget()->contentsY() ) / view->zoom() );

		// TODO :  makes a copy of the selection, do assignment operator instead
		VObjectListIterator itr = part()->document().selection();
		VObjectList list;
		list.setAutoDelete( true );
	    for( ; itr.current() ; ++itr )
		{
			list.append( itr.current()->clone() );
		}
		painter->setZoomFactor( view->zoom() );
		VObjectListIterator itr2 = list;
		for( ; itr2.current() ; ++itr2 )
		{
			itr2.current()->transform( mat );
			itr2.current()->setState( state_edit );
			itr2.current()->draw( painter, itr2.current()->boundingBox() );
		}
		painter->setZoomFactor( 1.0 );
	}
	else
		m_isDragging = false;
}

bool
VShearTool::eventFilter( KarbonView* view, QEvent* event )
{
	if ( event->type() == QEvent::MouseMove )
	{
		if( m_isDragging )
		{
			// erase old object:
			drawTemporaryObject( view );

			QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
			m_lp.setX( mouse_event->pos().x() );
			m_lp.setY( mouse_event->pos().y() );

			// paint new object:
			drawTemporaryObject( view );
		}
		else
		{
			VHandleTool::instance( m_part )->eventFilter( view, event );
			setCursor( view );
		}

		return true;
	}

	if ( event->type() == QEvent::MouseButtonRelease && m_isDragging )
	{
		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		KoPoint fp = view->canvasWidget()->viewportToContents( QPoint( m_fp.x(), m_fp.y() ) );

		part()->addCommand(
			new VShearCmd( &part()->document(), fp * (1.0 / view->zoom() ), m_s1, m_s2 ),
			true );

		m_isDragging = false;

		return true;
	}

	// handle pressing of keys:
	if ( event->type() == QEvent::KeyPress )
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*>( event );

		// cancel dragging with ESC-key:
		if ( key_event->key() == Qt::Key_Escape && m_isDragging )
		{
			m_isDragging = false;

			// erase old object:
			drawTemporaryObject( view );

			return true;
		}
	}

	// the whole story starts with this event:
	if ( event->type() == QEvent::MouseButtonPress )
	{
		view->painterFactory()->painter()->end();
        VHandleTool::instance( m_part )->eventFilter( view, event );
		QMouseEvent* mouse_event = static_cast<QMouseEvent*>( event );
		m_fp.setX( mouse_event->pos().x() );
		m_fp.setY( mouse_event->pos().y() );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		// draw initial object:
		drawTemporaryObject( view );
		m_isDragging = true;

		return true;
	}

	return false;
}

