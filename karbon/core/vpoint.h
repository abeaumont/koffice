/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VPOINT_H__
#define __VPOINT_H__

#include <qpoint.h>

/**
 * A VPoint acts like a vector and is used to describe coordinates.
 * It includes a QPoint for direct use in painting on a QPainter.
 * Before 
 */

class VPoint
{
public:
	VPoint( const double x = 0.0, const double y = 0.0 );
	VPoint( const VPoint& point );

	// convert to QPoint and recalculate if necessary:
	const QPoint& getQPoint( const double zoomFactor ) const;

	void moveTo( const double x = 0.0, const double y = 0.0 );
	void rmoveTo( const double x = 0.0, const double y = 0.0 );

	const double& x() const { return m_x; }
	void setX( const double x = 0.0 );
	const double& y() const { return m_y; }
	void setY( const double y = 0.0 );

	/**
	* we scale QPoint with fractScale, i.e. we consider fractBits bits
	* of the fraction of each double-coordinate.
	*/
// TODO: is the follwing obsolete?
//	static const char s_fractBits = 12;
//	static const unsigned int s_fractScale = 1 << s_fractBits;
//	static const double s_fractInvScale; // = 1/s_fractScale

private:
	double m_x;
	double m_y;

	mutable bool m_isDirty;				// have to recalc QPoint?
	static double m_lastZoomFactor;		// cache last zoomFactor
	mutable QPoint m_QPoint;			// for painting
};

inline bool operator ==( const VPoint& l, const VPoint& r )
	{ return ( l.x()==r.x() && l.y()==r.y() ); }

inline bool operator !=( const VPoint& l, const VPoint& r )
	{ return !operator==( l, r ); }

#endif
