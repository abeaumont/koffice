/* This file is part of the KDE project
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

#include <kdebug.h>
#include <koPoint.h>
#include <koRect.h>

#include "karbon_view.h"
#include "vpath.h"
#include "vtext.h"
#include "vstroke.h"
#include "vfill.h"
#include "vvisitor.h"
#include "vsegment.h"

#ifdef HAVE_FREETYPE

#include <freetype2/freetype/freetype.h>
#include <fontconfig/fontconfig.h>

#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H

#define FT_TOFLOAT(x) ((x) * (1.0 / 64.0))
#define FT_FROMFLOAT(x) ((int) floor ((x) * 64.0 + 0.5))



// Trace routines for ttf / ps font -> VPath

int traceMoveto( FT_Vector *to, VComposite *composite )
{
	double tox = (to->x / 64);
	double toy = (-to->y / 64);

	//QString add = "M" + QString::number(tox) + "," + QString::number(toy) + " ";
	//kdDebug() << add.latin1() << endl;
	composite->moveTo( KoPoint( tox, toy ) );
	
	return 0;
}

int traceLineto( FT_Vector *to, VComposite *composite )
{
	double tox = (to->x / 64);
	double toy = (-to->y / 64);

	//QString add = "L" + QString::number(tox) + "," + QString::number(toy) + " ";
	//kdDebug() << add.latin1() << endl;

	composite->lineTo( KoPoint( tox, toy ) );
	
	return 0;
};

int traceQuadraticBezier( FT_Vector *control, FT_Vector *to, VComposite *composite )
{
	double x1 = (control->x / 64);
	double y1 = (-control->y / 64);
	double x2 = (to->x / 64);
	double y2 = (-to->y / 64);

	//QString add = "Q" + QString::number(x1) + "," + QString::number(y1) + "," + QString::number(x2) + "," + QString::number(y2) + " ";
	//kdDebug() << add.latin1() << endl;
//path->curveTo( x1, y1, x1, y1, x2, y2 );
	composite->curve2To( KoPoint( x1, y1 ), KoPoint( x2, y2 ) );
	
	return 0;
};

int traceCubicBezier( FT_Vector *p, FT_Vector *q, FT_Vector *to, VComposite *composite )
{
	double x1 = (p->x / 64);
	double y1 = (-p->y / 64);
	double x2 = (q->x / 64);
	double y2 = (-q->y / 64);
	double x3 = (to->x / 64);
	double y3 = (-to->y / 64);

	//QString add = "C" + QString::number(x1) + "," + QString::number(y1) + "," + QString::number(x2) + "," + QString::number(y2) + "," + QString::number(x3) + "," + QString::number(y3);
	//kdDebug() << add.latin1() << endl;

	composite->curveTo( KoPoint( x1, y1 ), KoPoint( x2, y2 ), KoPoint( x3, y3 ) );
	
	return 0;
};

FT_Outline_Funcs OutlineMethods =
{
	(FT_Outline_MoveTo_Func) traceMoveto,
	(FT_Outline_LineTo_Func) traceLineto,
	(FT_Outline_ConicTo_Func) traceQuadraticBezier,
	(FT_Outline_CubicTo_Func) traceCubicBezier,
	0,
	0
};

#endif // HAVE_FREETYPE

VText::VText( VObject* parent, VState state )
	: VObject( parent, state ), m_basePath( 0L )
{
	m_glyphs.setAutoDelete( true );
	m_boundingBoxIsInvalid = true;
	m_stroke = new VStroke( this );
	m_fill = new VFill();
}


VText::VText( const QFont &font, const VPath& basePath, Position position, const QString& text )
	: VObject( 0L ), m_font( font ), m_basePath( basePath ), m_position( position ), m_text( text )
{
	m_glyphs.setAutoDelete( true );
	m_boundingBoxIsInvalid = true;
	m_stroke = new VStroke( this );
	m_fill = new VFill();
}

VText::VText( const VText& text )
	: VObject( text ), m_font( text.m_font ), m_basePath( text.m_basePath ), m_position( text.m_position ), m_text( text.m_text )
{
	m_stroke = new VStroke( *text.m_stroke );
	m_stroke->setParent( this );
	m_fill = new VFill( *text.m_fill );

	// copy glyphs
	VCompositeListIterator itr( text.m_glyphs );
	for( ; itr.current() ; ++itr )
		m_glyphs.append( itr.current()->clone() );
	
	m_boundingBoxIsInvalid = true;
}

VText::~VText()
{
}

void
VText::draw( VPainter* painter, const KoRect* /*rect*/ ) const
{
	if(
		state() == deleted ||
		state() == hidden ||
		state() == hidden_locked )
	{
		return;
	}

	painter->save();

	VCompositeListIterator itr( m_glyphs );

	if( state() != edit )
	{
		// paint fill:
		painter->newPath();

		for( itr.toFirst(); itr.current(); ++itr )
		{
			itr.current()->setFill( *m_fill );
			itr.current()->setStroke( *m_stroke );
			itr.current()->draw( painter );
		}
	}

	// draw simplistic contour:
	if( state() == edit )//|| state() == selected )
	{
		painter->newPath();
		painter->setRasterOp( Qt::XorROP );
		painter->setPen( Qt::yellow );
		painter->setBrush( Qt::NoBrush );

		for( itr.toFirst(); itr.current(); ++itr )
			itr.current()->draw( painter );

		painter->strokePath();
	}

	painter->restore();
}

void
VText::transform( const QWMatrix& m )
{
	m_basePath.transform( m );

	VCompositeListIterator itr( m_glyphs );
	for( ; itr.current() ; ++itr )
		itr.current()->transform( m );

	m_boundingBoxIsInvalid = true;
}

const KoRect&
VText::boundingBox() const
{
	if( m_boundingBoxIsInvalid )
	{
		// clear:
		m_boundingBox = KoRect(); 

		VCompositeListIterator itr( m_glyphs );
		for( itr.toFirst(); itr.current(); ++itr )
		{
			m_boundingBox |= itr.current()->boundingBox();
		}

		// take line width into account:
		m_boundingBox.setCoords(
			m_boundingBox.left()   - 0.5 * stroke()->lineWidth(),
			m_boundingBox.top()    - 0.5 * stroke()->lineWidth(),
			m_boundingBox.right()  + 0.5 * stroke()->lineWidth(),
			m_boundingBox.bottom() + 0.5 * stroke()->lineWidth() );

		m_boundingBoxIsInvalid = false;
	}

	return m_boundingBox;
} 

VText*
VText::clone() const
{
	return new VText( *this );
}

void
VText::save( QDomElement& element ) const
{
	if( state() != deleted )
	{
		QDomElement me = element.ownerDocument().createElement( "TEXT" );

		VObject::save( me );

		// save font properties
		me.setAttribute( "text", m_text );
		me.setAttribute( "family", m_font.family() );
		me.setAttribute( "size", m_font.pointSize() );
		me.setAttribute( "italic", m_font.italic() );
		me.setAttribute( "bold", m_font.bold() );
		me.setAttribute( "position", m_position );

		element.appendChild( me );
		
		m_basePath.save( me );

		// save all glyphs / paths
		VCompositeListIterator itr = m_glyphs;
		for( ; itr.current() ; ++itr )
			itr.current()->save( me );
	}
}

void
VText::load( const QDomElement& element )
{
	m_glyphs.clear();

	m_font.setFamily( element.attribute( "family", "Times" ) );
	m_font.setPointSize( element.attribute( "size", "12" ).toInt() );
	m_font.setItalic( element.attribute( "italic" ) == 0 ? false : true );
	m_font.setWeight( QFont::Normal );
	m_font.setBold( element.attribute( "bold" ) == 0 ? false : true );
	m_position = (Position)element.attribute( "position", "0" ).toInt();

	m_text = element.attribute( "text", "" );

    // load text glyphs:
	QDomNodeList list = element.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();
			if( e.tagName() == "COMPOSITE" )
			{
				VComposite *composite = new VComposite( this );
				composite->load( e );
				m_glyphs.append( composite );
			}
			if( e.tagName() == "PATH" )
			{
				m_basePath.load( e );
			}
		}
	}
	m_boundingBoxIsInvalid = true;
}

void 
VText::setState( const VState state )
{
	VObject::setState( state );
	
	VCompositeListIterator itr( m_glyphs );
	for( itr.toFirst(); itr.current(); ++itr )
	{
		itr.current()->setState( state );
	}
}

void
VText::accept( VVisitor& visitor )
{
	visitor.visitVText( *this );
}

#ifdef HAVE_FREETYPE

void
VText::traceText( const KarbonView* view )
{
	if( !view )
	{
		kdDebug() << "Can't trace a text without a view." << endl;
		return;
	}
	if( m_basePath.count() == 0 )
	{
		kdDebug() << "Can't draw a text without base path (was: " << m_text << ")." << endl;
		return;
	}

	// TODO : set more options
	int slant = FC_SLANT_ROMAN;
	if( m_font.italic() )
		slant = FC_SLANT_ITALIC;

	int weight = 0;
	if( m_font.bold() )
		weight = FC_WEIGHT_BOLD;

	// Build FontConfig request pattern
	int id = -1;
	QString filename = buildRequest( m_font.family(), weight, slant, m_font.pointSize(), id );
	m_glyphs.clear();

	kdDebug() << "Loading " << filename.latin1() << " for requested font \"" << m_font.family().latin1() << "\", " << m_font.pointSize() << " pt." << endl;

	FT_UInt glyphIndex;
	FT_Face fontFace;
	FT_Library library;
	FT_Init_FreeType( &library );
	FT_Error error = FT_New_Face( library, filename.latin1(), id, &fontFace );

	if( error )
	{
		kdDebug() << "traceText(), could not load font. Aborting!" << endl;
		return;
	}

	// Choose unicode charmap
	for( int charmap = 0; charmap < fontFace->num_charmaps; charmap++ )
	{
		if( fontFace->charmaps[charmap]->encoding == ft_encoding_unicode )
		{
			FT_Error error = FT_Set_Charmap( fontFace, fontFace->charmaps[charmap] );
			if( error )
			{
				kdDebug() << "traceText(), unable to select unicode charmap. Aborting!" << endl;
				FT_Done_Face( fontFace );
				return;
			}
		}
	}

	FT_Set_Char_Size( fontFace, FT_FROMFLOAT( m_font.pointSize() ), FT_FROMFLOAT( m_font.pointSize() ), 0, 0 );

	float x = 0;
	float y = 0;
	float dx = 0;
	float sp = 0;
	KoPoint point;
	KoPoint normal;
	KoPoint tangent;
	VPathIterator pathIt( m_basePath );
	VSegment* oldSeg = pathIt.current();
	VSegment* seg = ++pathIt;
	KoPoint extPoint;
	bool ext = false;
	float fsx = 0;
	int yoffset = ( m_position == Above ? 0 : ( m_position == On ? m_font.pointSize() / 3 : m_font.pointSize() / 1.5 ) );
	kdDebug() << "Position: " << m_position << " -> " << yoffset << endl;
	for( int i = 0; i < m_text.length(); i++ )
	{
		// get the glyph index for the current character
		QChar character = m_text.at( i );
		glyphIndex = FT_Get_Char_Index( fontFace, character.unicode() );
		//kdDebug() << "glyphIndex : " << glyphIndex << endl;
		FT_Error error = FT_Load_Glyph( fontFace, glyphIndex, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP );
		if( error )
		{
			kdDebug() << "Houston, we have a problem : " << error << endl;
			continue;
		}

		// decompose to vpaths
		FT_OutlineGlyph g;
		FT_Get_Glyph( fontFace->glyph, reinterpret_cast<FT_Glyph *>( &g ) );
		VComposite *composite = new VComposite( this );
		VFill *fill = composite->fill();
		fill->setFillRule( VFill::evenOdd );
		composite->setFill( *fill );
		FT_Outline_Decompose(&g->outline, &OutlineMethods, composite );
		//composite->close();

		// Step 1: place (0, 0) to the rotation center of the glyph.
		dx = FT_TOFLOAT( fontFace->glyph->advance.x ) / 2;
		x += dx;
		composite->transform( QWMatrix( 1, 0, 0, 1, -dx, y + yoffset ) );

		// Step 2: find the position where to draw.
		while ( seg && x > fsx + seg->length() )
		{
			fsx += seg->length();
			oldSeg = seg;
			seg = ++pathIt;
		}
		if( seg )
		{
			sp = ( x - fsx ) / seg->length();
			seg->pointTangentNormal( sp, &point, &tangent, &normal );
		}
		else
		{
			ext = true;
			if( ext )
				oldSeg->pointTangentNormal( 1, &extPoint, &tangent, &normal );
			point = extPoint + ( x - fsx ) * tangent;
		}

		// Step 3: transform glyph and append it. That's it, we've got
		// text following a path. Really easy, isn't it ;) ?
		composite->transform( QWMatrix( tangent.x(), tangent.y(), tangent.y(), -tangent.x(), point.x(), point.y() ) );
		composite->setState( state() );
		m_glyphs.append( composite );

		//kdDebug() << "Glyph: " << (QString)character << " [String pos: " << x << ", " << y << " / Canvas pos: " << point.x() << ", " << point.y() << "]" << endl;

		x += dx;
		y += FT_TOFLOAT( fontFace->glyph->advance.y );
	}
	FT_Done_Face( fontFace );
	m_boundingBoxIsInvalid = true;
}

// This routine is copied from KSVGFont (Rob)
QString
VText::buildRequest( QString family, int weight, int slant, double size, int &id )
{
	// Use FontConfig to locate & select fonts and use  FreeType2 to open them
	FcPattern *pattern;
	QString fileName;

	pattern = FcPatternBuild( 0, FC_WEIGHT, FcTypeInteger, weight,
							  FC_SLANT, FcTypeInteger, slant,
							  FC_SIZE, FcTypeDouble, size, 0 );

	// Add font name
	FcPatternAddString( pattern, FC_FAMILY, reinterpret_cast<const FcChar8 *>( family.latin1() ) );

	// Disable hinting
	FcPatternAddBool( pattern, FC_HINTING, false );

	// Perform the default font pattern modification operations.
	FcDefaultSubstitute( pattern );
	FcConfigSubstitute( FcConfigGetCurrent(), pattern, FcMatchPattern );

	// Match the pattern!
	FcResult result;
	FcPattern *match = FcFontMatch( 0, pattern, &result );

	// Destroy pattern
	FcPatternDestroy( pattern );

	// Get index & filename
	FcChar8 *temp;

	if(match)
	{
		FcPattern *pattern = FcPatternDuplicate( match );

		// Get index & filename
		if(	FcPatternGetString(pattern, FC_FILE, 0, &temp) != FcResultMatch ||
		   	FcPatternGetInteger(pattern, FC_INDEX, 0, &id) != FcResultMatch )
		{
			kdDebug() << "VText::buildRequest(), could not load font file for requested font \"" << family.latin1() << "\"" << endl;
			return QString::null;
		}

		fileName = QString::fromLatin1( reinterpret_cast<const char *>( temp ) );

		// Kill pattern
		FcPatternDestroy( pattern );
	}

	// Kill pattern
	FcPatternDestroy( match ); 

	return fileName;
}

#endif // HAVE_FREETYPE
