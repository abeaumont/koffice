/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#include <kdebug.h>

#include <qpainter.h>
#include <math.h>

#include "vaffinemap.h"
#include "vpath.h"
#include "vpoint.h"

// TODO: make sure that lastpoint==currenpoint doesnt get removed

VSegment::VSegment( const double lp_x, const double lp_y )
	: m_lastPoint( lp_x, lp_y ), m_isDirty( true )
{
}

const QPointArray&
VSegment::getQPointArray( const VSegment& prevSeg,
	const double& zoomFactor ) const
{
	if ( m_isDirty )
	{
// xxxxxx
		m_isDirty = false;
	}

	return m_QPointArray;
}

void
VSegment::transform( const VAffineMap& affmap )
{
	m_lastPoint = affmap.map( m_lastPoint );
}

// -----------------------------------

VLine::VLine( const double lp_x, const double lp_y )
	: VSegment( lp_x, lp_y )
{
}

// -----------------------------------

VCurve::VCurve(
		const double fcp_x, const double fcp_y,
		const double lcp_x, const double lcp_y,
		const double lp_x, const double lp_y )
	: VSegment( lp_x, lp_y ),
	  m_firstCtrlPoint( fcp_x, fcp_y ),
	  m_lastCtrlPoint( lcp_x, lcp_y )
{
}

void
VCurve::transform( const VAffineMap& affmap )
{
	m_firstCtrlPoint	= affmap.map( m_firstCtrlPoint );
	m_lastCtrlPoint		= affmap.map( m_lastCtrlPoint );
	m_lastPoint			= affmap.map( m_lastPoint );
}

// -----------------------------------


VCurve1::VCurve1(
		const double lcp_x, const double lcp_y,
		const double lp_x, const double lp_y )
	: VSegment( lp_x, lp_y ),
	  m_lastCtrlPoint( lcp_x, lcp_y )
{
}

void
VCurve1::transform( const VAffineMap& affmap )
{
	m_lastCtrlPoint		= affmap.map( m_lastCtrlPoint );
	m_lastPoint			= affmap.map( m_lastPoint );
}

// -----------------------------------


VCurve2::VCurve2(
		const double fcp_x, const double fcp_y,
		const double lp_x, const double lp_y )
	: VSegment( lp_x, lp_y ),
	  m_firstCtrlPoint( fcp_x, fcp_y )
{
}

void
VCurve2::transform( const VAffineMap& affmap )
{
	m_firstCtrlPoint	= affmap.map( m_firstCtrlPoint );
	m_lastPoint			= affmap.map( m_lastPoint );
}

// -----------------------------------

VPath::VPath()
	: VObject(), m_isClosed( false )
{
	// create a current point [0.0,0.0], we abuse a line for this:
	m_segments.append( new VLine );
}

VPath::VPath( const VPath& path )
	: VObject()
{
// TODO: implement copy-ctor
}

VPath::~VPath()
{
// TODO: should we be polite and destruct the QLists as well ?
	QListIterator<VSegment> i( m_segments );
	for ( ; i.current() ; ++i )
		delete( i.current() );
}

void
VPath::draw( QPainter& painter, const QRect& rect, const double zoomFactor )
{
// TODO:
// - have a deep look at Qt's quadBezier()
/*
	painter.save();

	// walk down all Segments and add their QPoints into a QPointArray
	QListIterator<Segment> i( m_segments );
	VPoint* prevPoint( 0L );	// previous point (for calculating beziers)
	QPointArray qpa;

	if ( i.current() )
	{
		prevPoint = i.current()->p3;

		if ( !isClosed() ) // only paint first point if path isnt closed:
		{
			qpa.resize( qpa.size()+1 );
			qpa.setPoint( qpa.size()-1, segment->p3->getQPoint( zoomFactor ) );

// demo-hack: draw controll points
			painter.setPen( Qt::black );
			painter.drawRect(
				segment->p3->getQPoint( zoomFactor ).x()-3,
				segment->p3->getQPoint( zoomFactor ).y()-3, 6, 6 );

		}
	}

	for ( segment=m_segments.next(); segment!=0L; segment=m_segments.next() )
	{
		// draw bezier-curve if all points are available:
		if ( prevPoint!=0L && segment->p1!=0L && segment->p2!=0L && segment->p3!=0L )
		{
			// let qt calculate bezier-qpoints for us:
			QPointArray pa(4);
			pa.setPoint( 0, prevPoint->getQPoint( zoomFactor ) );
			pa.setPoint( 1, segment->p1->getQPoint( zoomFactor ) );
			pa.setPoint( 2, segment->p2->getQPoint( zoomFactor ) );
			pa.setPoint( 3, segment->p3->getQPoint( zoomFactor ) );
			pa = pa.quadBezier(); // is this a memory leak ?

			// can this part be made more efficient ? i bet it could...
			unsigned int size1(qpa.size()), size2(pa.size());
			qpa.resize( size1+size2 );
			for ( unsigned int i=0; i<size2; i++ )
				qpa.setPoint( size1+i, pa.point(i) );
// demo-hack: draw controll points
			painter.setPen( Qt::black );
			painter.drawRect(
				prevPoint->getQPoint( zoomFactor ).x()-3,
				prevPoint->getQPoint( zoomFactor ).y()-3, 6, 6 );
			painter.drawRect(
				segment->p1->getQPoint( zoomFactor ).x()-3,
				segment->p1->getQPoint( zoomFactor ).y()-3, 6, 6 );
			painter.drawRect(
				segment->p2->getQPoint( zoomFactor ).x()-3,
				segment->p2->getQPoint( zoomFactor ).y()-3, 6, 6 );
			painter.drawRect(
				segment->p3->getQPoint( zoomFactor ).x()-3,
				segment->p3->getQPoint( zoomFactor ).y()-3, 6, 6 );


// demo-hack: draw bezier control-lines
			painter.setPen( QPen( Qt::black, 1, Qt::DotLine ) );
			painter.drawLine(
				prevPoint->getQPoint( zoomFactor ),
				segment->p1->getQPoint( zoomFactor ) );
			painter.drawLine(
				segment->p2->getQPoint( zoomFactor ),
				segment->p3->getQPoint( zoomFactor ) );

		}
		else
		{
			// draw a line:
			qpa.resize( qpa.size()+1 );
			qpa.setPoint( qpa.size()-1, segment->p3->getQPoint( zoomFactor ) );

// demo-hack: draw controll points
			painter.setPen( Qt::black );
			painter.drawPoint( segment->p3->getQPoint( zoomFactor ) );

		}
		prevPoint = segment->p3; // need previous point to calculate bezier-qpoints
	}

	painter.setPen( Qt::black );
	painter.setBrush( QColor( 205, 201, 165 ) );

	// draw open or closed path ?
	if ( isClosed() )
		painter.drawPolygon( qpa );
	else
		painter.drawPolyline( qpa );

	painter.restore();
*/
}

const VPoint*
VPath::currentPoint() const
{
	return( m_segments.getLast()->lastPoint() );
}

VPath&
VPath::moveTo( const double x, const double y )
{
// TODO: what is the exact postscript-beaviour?
	if ( isClosed() ) return *this;
/*
	m_segments.getLast()->lastPoint()->moveTo( x, y ); */
	return *this;
}

VPath&
VPath::lineTo( const double x, const double y )
{

	if ( isClosed() ) return *this;
/*
	VPoint* fp = lastSegment()->lastPoint();
	fp->ref();

	VPoint* lp = new VPoint( x, y );
	m_pointPool.append( lp );

	m_segments.append( new VLine( fp, lp ) ); */
	return *this;
}

VPath&
VPath::curveTo( const double x1, const double y1,
	const double x2, const double y2, const double x3, const double y3 )
{
	if ( isClosed() ) return *this;
/*
	VPoint* fp = lastSegment()->lastPoint();
	fp->ref();

	VPoint* fcp = new VPoint( x1, y1 );
	VPoint* lcp = new VPoint( x2, y2 );
	VPoint* lp  = new VPoint( x3, y3 );
	m_pointPool.append( fcp );
	m_pointPool.append( lcp );
	m_pointPool.append( lp );

	m_segments.append( new VBezier( fp, fcp, lcp, lp ) );
*/
	return *this;
}

VPath&
VPath::curve1To( const double x2, const double y2,
	const double x3, const double y3 ) {
	if ( isClosed() ) return *this;
//
	return *this;
}

VPath&
VPath::curve2To( const double x1, const double y1,
	const double x3, const double y3 )
{
	if ( isClosed() ) return *this;
//
	return *this;
}

VPath&
VPath::arcTo( const double x1, const double y1,
	const double x2, const double y2, const double r )
{
	// parts of this routine are inspired by GNU ghostscript

	if ( isClosed() ) return *this;
/*
	// we need to calculate the tangent points. therefore calculate tangents
	// D10=P1P0 and D12=P1P2 first:
	double dx10 = m_segments.getLast()->p3->x() - x1;
	double dy10 = m_segments.getLast()->p3->y() - y1;
	double dx12 = x2 - x1;
	double dy12 = y2 - y1;

	// calculate distance squares:
	double dsq10 = dx10*dx10 + dy10*dy10;
	double dsq12 = dx12*dx12 + dy12*dy12;

	// we now calculate tan(a/2) where a is the angular between D10 and D12.
	// we take advantage of D10*D12=d10*d12*cos(a), |D10xD12|=d10*d12*sin(a)
	// and tan(a/2)=sin(a)/[1-cos(a)].
	double num   = dx10*dy12 - dy10*dx12;
	double denom = sqrt( dsq10*dsq12 ) - dx10*dx12 + dy10*dy12;

	if ( denom==0 )			// points are co-linear
		lineTo( x1, y1 );	// just add a line to first point
    else
    {
		// calculate distances from P1 to tangent points:
		double dist = fabs( r*num / denom );
		double d1t0 = dist / sqrt(dsq10);
		double d1t1 = dist / sqrt(dsq12);

// TODO: check for r<0

		double bx0 = x1 + dx10*d1t0;
		double by0 = y1 + dy10*d1t0;

		// if (bx0,by0) deviates from current point, add a line to it:
// TODO: decide via radius<XXX or sthg?
		if ( bx0 != m_segments.getLast()->p3->x() || by0 !=
 m_segments.getLast()->p3->y() )
			lineTo( bx0, by0 );

		double bx3 = x1 + dx12*d1t1;
		double by3 = y1 + dy12*d1t1;

		// the two bezier-control points are located on the tangents at a fraction
		// of the distance [tangent points<->tangent intersection].
		double distsq = (x1 - bx0)*(x1 - bx0) + (y1 - by0)*(y1 - by0);
		double rsq = r*r;
		double fract;

// TODO: make this nicer? check the exact meaning of this formula

		if ( distsq >= rsq * VPoint::s_fractScale ) // r is very small
			fract = 0.0; // dist==r==0
		else
			fract = (4.0 / 3.0) / (1 + sqrt(1 + distsq / rsq));

		double bx1 = bx0 + (x1 - bx0) * fract;
		double by1 = by0 + (y1 - by0) * fract;
		double bx2 = bx3 + (x1 - bx3) * fract;
		double by2 = by3 + (y1 - by3) * fract;

		// finally add the bezier-segment:
		m_segments.append( new Segment );
		m_segments.getLast()->p1 = new VPoint( bx1, by1 );
		m_segments.getLast()->p2 = new VPoint( bx2, by2 );
		m_segments.getLast()->p3 = new VPoint( bx3, by3 );
	} */
	return *this;
}

VPath&
VPath::close()
{
	if ( isClosed() ) return *this;
/*
// TODO: dont "close" a single line
	// draw a line if last point differs from first point
	if ( *(m_segments.getFirst()->p3) != *(m_segments.getLast()->p3) )
		lineTo( m_segments.getFirst()->p3->x(), m_segments.getFirst()->p3->y() );

	// do nothing if first and last point are the same (eg only first point
	// exists):
	if ( m_segments.getFirst()->p3!=m_segments.getLast()->p3 )
	{
		if ( m_segments.getLast()->p3->unref()==0 )
			delete m_segments.getLast()->p3;

		m_segments.getLast()->p3 = m_segments.getFirst()->p3;

		m_isClosed = true;
	} */
	return *this;
}

VObject&
VPath::transform( const VAffineMap& affmap )
{
	QListIterator<VSegment> i( m_segments );
	for ( ; i.current() ; ++i )
		i.current()->transform( affmap );

	return *this;
}
