#include <qpainter.h>
#include <kdebug.h>

#include "karbon_view.h"
#include "karbon_part.h"
#include "vcanvas.h"

VCanvas::VCanvas( KarbonView* view, KarbonPart* part )
	: QScrollView( view, "canvas", WNorthWestGravity | WResizeNoErase | WRepaintNoErase ),
	m_part( part ), m_view( view ), m_zoomFactor( 1.0 )
{
	viewport()->setFocusPolicy( QWidget::StrongFocus );
	viewport()->setMouseTracking( true );
	setMouseTracking( true );
	setFocus();
	viewport()->setBackgroundMode( QWidget::NoBackground );
}

void
VCanvas::drawContents( QPainter* painter, int clipx, int clipy, int clipw, int cliph  )
{
	drawDocument( painter, QRect( clipx, clipy, clipw, cliph ) );
}

void
VCanvas::drawDocument( QPainter* painter, const QRect& rect )
{
	QListIterator<VObject> i = m_part->m_objects;
	for ( ; i.current() ; ++i )
	{
		i.current()->draw( *painter, rect, m_zoomFactor );
	}
}

void
VCanvas::resizeEvent( QResizeEvent* event )
{
	QScrollView::resizeEvent( event );
}

#include <vcanvas.moc>
