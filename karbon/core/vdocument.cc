/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers

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

#include <qdom.h>

#include "vdocument.h"
#include "vfill.h"
#include "vselection.h"
#include "vstroke.h"
#include "vvisitor.h"
#include "vpainter.h"

#include <kdebug.h>


VDocument::VDocument()
	: VObject( 0L ),
	  m_selectionMode( VDocument::ActiveLayer ),
	  m_mime( "application/x-karbon" ),
	  m_version( "0.1" ), m_editor( "karbon14 0.0.1" ),
	  m_syntaxVersion( "0.1" )
{
	m_selection = new VSelection( this );

	// create a layer. we need at least one:
	m_layers.setAutoDelete( true );
	m_layers.append( new VLayer( this ) );
	m_activeLayer = m_layers.getLast();
}

VDocument::VDocument( const VDocument& document )
	: VObject( document )
{
	m_selection = new VSelection( this );
// TODO
}

VDocument::~VDocument()
{
	delete( m_selection );
}

void
VDocument::drawPage( VPainter *p ) const
{
#define LEFT   20
#define RIGHT  550
#define BOTTOM 20
#define TOP    830

	p->setPen( Qt::black );
	p->setBrush( Qt::white );
	p->newPath();
	p->moveTo( KoPoint( LEFT,  BOTTOM ) );
	p->lineTo( KoPoint( RIGHT, BOTTOM ) );
	p->lineTo( KoPoint( RIGHT, TOP ) );
	p->lineTo( KoPoint( LEFT,  TOP ) );
	p->lineTo( KoPoint( LEFT,  BOTTOM ) );
	p->fillPath();
	p->strokePath();

	p->setPen( Qt::NoPen );
	p->setBrush( Qt::black );
	p->newPath();
	p->moveTo( KoPoint( RIGHT,     BOTTOM - 2 ) );
	p->lineTo( KoPoint( RIGHT + 2, BOTTOM - 2 ) );
	p->lineTo( KoPoint( RIGHT + 2, TOP ) );
	p->lineTo( KoPoint( RIGHT,     TOP ) );
	p->fillPath();

	p->newPath();
	p->moveTo( KoPoint( LEFT,  BOTTOM ) );
	p->lineTo( KoPoint( LEFT,  BOTTOM - 2 ) );
	p->lineTo( KoPoint( RIGHT, BOTTOM - 2 ) );
	p->lineTo( KoPoint( RIGHT, BOTTOM ) );
	p->fillPath();

	p->newPath();
	p->moveTo( KoPoint( LEFT,  TOP ) );
	p->lineTo( KoPoint( LEFT,  TOP + 1 ) );
	p->lineTo( KoPoint( RIGHT, TOP + 1 ) );
	p->lineTo( KoPoint( RIGHT, TOP ) );
	p->fillPath();
}

void
VDocument::draw( VPainter *painter, const KoRect* rect ) const
{
	QPtrListIterator<VLayer> itr = m_layers;

	for ( ; itr.current(); ++itr )
	{
		itr.current()->draw( painter, rect );
	}
}

void
VDocument::insertLayer( VLayer* layer )
{
//	if ( pos == -1 || !m_layers.insert( layer, pos ))
		m_layers.append( layer );
	m_activeLayer = layer;
} // VDocument::insertLayer

void 
VDocument::removeLayer( VLayer* layer )
{
	m_layers.remove( layer );
	if ( m_layers.count() == 0 )
		m_layers.append( new VLayer( this ) );
	m_activeLayer = m_layers.getLast();
} // VDocument::removeLayer

void 
VDocument::raiseLayer( VLayer* layer )
{
	int pos = m_layers.find( layer );
	if( pos != int( m_layers.count() ) - 1 && pos >= 0 )
	{
		VLayer* layer = m_layers.take( pos );
		m_layers.insert( pos + 1, layer );
	}
} // VDocument::raiseLayer

void 
VDocument::lowerLayer( VLayer* layer )
{
	int pos = m_layers.find( layer );
	if ( pos > 0 )
	{
		VLayer* layer = m_layers.take( pos );
		m_layers.insert( pos - 1, layer );
	}
} // VDocument::lowerLayer

int 
VDocument::layerPos( VLayer* layer )
{
	return m_layers.find( layer );
} // VDocument::layerPos

void 
VDocument::setActiveLayer( VLayer* layer )
{
	if ( m_layers.find( layer ) != -1 )
		m_activeLayer = layer;
} // VDocument::setActiveLayer

void
VDocument::append( VObject* object )
{
	m_activeLayer->append( object );
}

QDomDocument
VDocument::saveXML() const
{
	QDomDocument doc;
	QDomElement me = doc.createElement( "DOC" );
	doc.appendChild( me );

	// TODO : add paper size/orientation storing code here
	// maybe we need to provide it as param or member var? (Rob)
	save( me );
	return doc;
}

void
VDocument::save( QDomElement& me ) const
{
	me.setAttribute( "mime", m_mime );
	me.setAttribute( "version", m_version );
	me.setAttribute( "editor", m_editor );
	me.setAttribute( "syntaxVersion", m_syntaxVersion );
	me.setAttribute( "width", m_width );
	me.setAttribute( "height", m_height );
	me.setAttribute( "unit", m_unitName );

	// save objects:
	VLayerListIterator itr( m_layers );

	for ( ; itr.current(); ++itr )
		itr.current()->save( me );
}


VDocument*
VDocument::clone() const
{
	return new VDocument( *this );
}

void
VDocument::load( const QDomElement& doc )
{
	loadXML( doc );
}

bool
VDocument::loadXML( const QDomElement& doc )
{
    if( doc.attribute( "mime" ) != "application/x-karbon" ||
		doc.attribute( "syntaxVersion" ) != "0.1" )
	{
		return false;
	}

	m_layers.clear();

	m_mime = doc.attribute( "mime" );
	m_version = doc.attribute( "version" );
	m_editor = doc.attribute( "editor" );
	m_syntaxVersion = doc.attribute( "syntaxVersion" );

	m_width  = doc.attribute( "width", "0.0" ).toDouble();
	m_height = doc.attribute( "height", "0.0" ).toDouble();

	m_unitName = doc.attribute( "unit", "mm" );

	QDomNodeList list = doc.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "LAYER" )
			{
				VLayer* layer = new VLayer( this );
				layer->load( e );
				insertLayer( layer );
			}
		}
	}
	return true;
}

void
VDocument::accept( VVisitor& visitor )
{
	visitor.visitVDocument( *this );
}

