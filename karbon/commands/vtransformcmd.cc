/* This file is doc of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <klocale.h>

#include "vcomposite.h"
#include "vpath.h"
#include "vsegment.h"
#include "vselection.h"
#include "vtext.h"
#include "vimage.h"
#include "vtransformcmd.h"
#include "vstroke.h"
#include "vfill.h"
#include "vdocument.h"

#include <kdebug.h>

VTransformCmd::VTransformCmd( VDocument *doc, const QWMatrix& mat, bool duplicate )
	: VCommand( doc, i18n( "Transform Objects" ) ), m_mat( mat ), m_duplicate( duplicate )
{
	m_selection = ( document() && document()->selection() )
		? document()->selection()->clone()
		: new VSelection();

	if( m_duplicate )
	{
		if( !m_selection || m_selection->objects().count() == 1 )
			setName( i18n( "Duplicate Object" ) );
		else
			setName( i18n( "Duplicate Objects" ) );
	}
	else if( !m_selection || m_selection->objects().count() == 1 )
		setName( i18n( "Transform Object" ) );
}

VTransformCmd::VTransformCmd( VDocument *doc, const QString& name, const QString& icon, bool duplicate )
		: VCommand( doc, name, icon ), m_duplicate( duplicate )
{
	m_selection = ( document() && document()->selection() )
		? document()->selection()->clone()
		: new VSelection();

	if( m_duplicate )
	{
		if( !m_selection || m_selection->objects().count() == 1 )
			setName( i18n( "Duplicate Object" ) );
		else
			setName( i18n( "Duplicate Objects" ) );
	}
}

VTransformCmd::~VTransformCmd()
{
	delete( m_selection );
	m_selection = 0L;
}

void
VTransformCmd::execute()
{
	if( !m_selection )
		m_selection = ( document() && document()->selection() )
			? document()->selection()->clone()
			: new VSelection();
	VObjectListIterator itr( m_selection->objects() );

	if( m_duplicate )
	{
		// clone original objects, add duplicates to document, transform and select them
		VObject *copy = 0L;
		for( ; itr.current() ; ++itr )
		{
			copy = itr.current()->clone();
			visit( *copy );
			document()->append( copy );
			document()->selection()->take( *itr.current() );
			document()->selection()->append( copy );
			m_duplicates.append( copy );
		}
	}
	else
	{
		// transform objects
		for( ; itr.current() ; ++itr )
		{
			visit( *itr.current() );
		}
	}

	setSuccess( true );
}

void
VTransformCmd::unexecute()
{
	// inverting the matrix should undo the affine transformation
	m_mat = m_mat.invert();

	if( m_duplicate )
	{
		// remove duplicated objects
		VObjectListIterator itr( m_duplicates );
		for( ; itr.current() ; ++itr )
		{
			document()->selection()->take( *itr.current() );
			itr.current()->setState( VObject::deleted );
		}
		VObjectListIterator jtr( m_selection->objects() );

		// add original selection objects to new selection
		for( ; jtr.current() ; ++jtr )
		{
			document()->selection()->append( jtr.current() );
		}
	}
	else
	{
		// move objects back to original position
		visit( *m_selection );
	}
	// reset
	m_mat = m_mat.invert();
	delete( m_selection );
	m_selection = 0L;
	setSuccess( false );
}

void
VTransformCmd::visitVPath( VPath& composite )
{
	if( composite.state() == VObject::hidden ||
		composite.state() == VObject::normal_locked ||
		composite.state() == VObject::hidden_locked )
		return;

	// Apply transformation to gradients.
	if( composite.stroke()->type() == VStroke::grad )
		composite.stroke()->gradient().transform( m_mat );

	if( composite.fill()->type() == VFill::grad )
		composite.fill()->gradient().transform( m_mat );

	composite.transform( m_mat );

	VVisitor::visitVPath( composite );
}

void
VTransformCmd::visitVSubpath( VSubpath& path )
{
	if( path.state() == VObject::hidden ||
		path.state() == VObject::normal_locked ||
		path.state() == VObject::hidden_locked )
		return;

	VSegment* segment = path.first();

	while( segment )
	{
		for( unsigned short i = 0; i < segment->degree(); ++i )
		{
			if( segment->pointIsSelected( i ) )
				segment->setPoint( i, segment->point( i ).transform( m_mat ) );
		}

		segment = segment->next();
	}

	path.invalidateBoundingBox();
}

void
VTransformCmd::visitVText( VText& text )
{
	if( text.state() == VObject::hidden ||
		text.state() == VObject::normal_locked ||
		text.state() == VObject::hidden_locked )
		return;

	visit( text.basePath() );

	VPathListIterator itr( text.glyphs() );

	for( ; itr.current() ; ++itr )
	{
		visit( *itr.current() );
	}

	text.invalidateBoundingBox();
}

void
VTransformCmd::visitVImage( VImage &img )
{
	if( img.state() == VObject::hidden ||
		img.state() == VObject::normal_locked ||
		img.state() == VObject::hidden_locked )
		return;

	img.transform( m_mat );
}

VTranslateCmd::VTranslateCmd( VDocument *doc, double d1, double d2, bool duplicate )
		: VTransformCmd( doc, i18n( "Translate Objects" ), "14_select", duplicate )
{
	if( !duplicate && ( !m_selection || m_selection->objects().count() == 1 ) )
		setName( i18n( "Translate Object" ) );

	m_mat.translate( d1, d2 );
}


VScaleCmd::VScaleCmd( VDocument *doc, const KoPoint& p, double s1, double s2, bool duplicate )
		: VTransformCmd( doc, i18n( "Scale Objects" ), "14_select", duplicate )
{
	if( !duplicate && ( !m_selection || m_selection->objects().count() == 1 ) )
		setName( i18n( "Scale Object" ) );

	m_mat.translate( p.x(), p.y() );
	m_mat.scale( s1, s2 );
	m_mat.translate( -p.x(), -p.y() );
}


VShearCmd::VShearCmd( VDocument *doc, const KoPoint& p, double s1, double s2, bool duplicate )
		: VTransformCmd( doc, i18n( "Shear Objects" ), "14_shear", duplicate )
{
	if( !duplicate && ( !m_selection || m_selection->objects().count() == 1 ) )
		setName( i18n( "Shear Object" ) );

	m_mat.translate( p.x(), p.y() );
	m_mat.shear( s1, s2 );
	m_mat.translate( -p.x(), -p.y() );
}

VRotateCmd::VRotateCmd( VDocument *doc, const KoPoint& p, double angle, bool duplicate )
		: VTransformCmd( doc, i18n( "Rotate Objects" ), "14_rotate", duplicate )
{
	if( !duplicate && ( !m_selection || m_selection->objects().count() == 1 ) )
		setName( i18n( "Rotate Object" ) );

	m_mat.translate( p.x(), p.y() );
	m_mat.rotate( angle );
	m_mat.translate( -p.x(), -p.y() );
}

VTranslateBezierCmd::VTranslateBezierCmd( VSegment *segment, double d1, double d2, bool firstControl )
		: VCommand( 0L, i18n( "Translate Bezier" ) ), m_segment( segment ), m_firstControl( firstControl )
{
	m_mat.translate( d1, d2 );
	m_segmenttwo = 0L;
}

VTranslateBezierCmd::~VTranslateBezierCmd()
{
}

void
VTranslateBezierCmd::execute()
{
	if( m_segment->type() == VSegment::curve )
	{
		QWMatrix m2( m_mat.m11(), m_mat.m12(), m_mat.m21(), m_mat.m22(), -m_mat.dx(), -m_mat.dy() );
		if( m_firstControl )
		{
			if( m_segment->prev() &&
				m_segment->prev()->type() == VSegment::curve &&
				m_segment->prev()->isSmooth() )
			{
				m_segmenttwo = m_segment->prev();
				for( uint i = 0;i < m_segmenttwo->degree();i++ )
				{
					m_segmenttwo->selectPoint( i, i == 1 );

					if( i == 1 )
						m_segmenttwo->setPoint( i, m_segmenttwo->point( i ).transform( m2 ) );
				}
			}
		}
		else
		{
			m_segmenttwo = ( m_segment->isSmooth() && m_segment->next()->type() == VSegment::curve ) ? m_segment->next() : 0L;
			if( m_segmenttwo )
			{
				for( uint i = 0;i < m_segmenttwo->degree();i++ )
				{
					m_segmenttwo->selectPoint( i, i == 0 );

					if( i == 0 )
						m_segmenttwo->setPoint( i, m_segmenttwo->point( i ).transform( m2 ) );
				}
			}
		}

		for( uint i = 0;i < m_segment->degree();i++ )
		{
			m_segment->selectPoint( i, i == uint( m_firstControl ? 0 : 1 ) );

			if( i == uint( m_firstControl ? 0 : 1 ) )
				m_segment->setPoint( i, m_segment->point( i ).transform( m_mat ) );
		}
	}
	setSuccess( true );
}

void
VTranslateBezierCmd::unexecute()
{
	QWMatrix m2( m_mat.m11(), m_mat.m12(), m_mat.m21(), m_mat.m22(), -m_mat.dx(), -m_mat.dy() );
	if( m_segment )
	{
		for( uint i = 0;i < m_segment->degree();i++ )
		{
			m_segment->selectPoint( i, i == uint( m_firstControl ? 0 : 1 ) );
	
			if( i == uint( m_firstControl ? 0 : 1 ) )
				m_segment->setPoint( i, m_segment->point( i ).transform( m_mat.invert() ) );
		}

		if( m_segmenttwo )
		{
			uint index = m_firstControl ? 1 : 0;
			for( uint i = 0;i < m_segmenttwo->degree();i++ )
			{
				m_segmenttwo->selectPoint( i, i == index );

			if( i == index )
				m_segmenttwo->setPoint( i, m_segmenttwo->point( i ).transform( m2.invert() ) );
			}
		}
	}
	setSuccess( false );
}

