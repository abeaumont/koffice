/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VPAINTER_H__
#define __VPAINTER_H__

// painter abstraction

#include <qnamespace.h>

class QWMatrix;
class QPointArray;
class QWidget;
class QColor;
class QPen;
class QBrush;

class VStroke;

class VPainter
{
public:
	VPainter( QWidget *, int /*w*/ = 0, int /*h*/ = 0 ) {};
	virtual ~VPainter() {};

	//
	virtual void resize( int w, int h ) = 0;
	virtual void begin() = 0;
	virtual void end() = 0;

	// matrix manipulation
	virtual void setWorldMatrix( const QWMatrix & ) = 0;

	// drawing
	virtual void drawPolygon( const QPointArray &, bool winding = false ) = 0;
	virtual void drawPolyline( const QPointArray & ) = 0;
	virtual void drawRect( double x, double y, double w, double h ) = 0;

	// pen + brush
	virtual void setPen( const VStroke & ) = 0;
	// virtual void setBrush( const VBrush & ) = 0;
	// compatibility, use VPen/VBrush later ?
	virtual void setPen( const QColor & ) = 0;
	virtual void setPen( Qt::PenStyle style ) = 0;
	virtual void setBrush( const QBrush & ) = 0;
	virtual void setBrush( const QColor & ) = 0;
	virtual void setBrush( Qt::BrushStyle style ) = 0;

	// stack management
	virtual void save() = 0;
	virtual void restore() = 0;

	// we have to see how this fits in
	virtual void setRasterOp( Qt::RasterOp ) = 0;
};

#endif
