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

#include <svgimport.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <koUnit.h>
#include <koGlobal.h>
#include <shapes/vellipse.h>
#include <shapes/vrectangle.h>
#include <shapes/vpolygon.h>
#include <core/vsegment.h>
#include <core/vtext.h>
#include <qcolor.h>
#include <qfile.h>
#include <qregexp.h>
#include <iostream>
#include <zlib.h>

typedef KGenericFactory<SvgImport, KoFilter> SvgImportFactory;
K_EXPORT_COMPONENT_FACTORY( libkarbonsvgimport, SvgImportFactory( "karbonsvgimport" ) );

SvgImport::SvgImport(KoFilter *, const char *, const QStringList&) :
    KoFilter(),
    outdoc( "DOC" )
{
}

SvgImport::~SvgImport()
{
}

KoFilter::ConversionStatus SvgImport::convert(const QCString& from, const QCString& to)
{
	// check for proper conversion
	if( to != "application/x-karbon" || from != "image/svg+xml" )
		return KoFilter::NotImplemented;
	QFile in( m_chain->inputFile() );

	QString path = m_chain->inputFile();
	if( path.right( 3 ).upper() == "SVG" )
    {
	    if( !in.open( IO_ReadOnly ) )
		{
			kdError(30502) << "Unable to open input file" << endl;
			return KoFilter::FileNotFound;
		}
		inpdoc.setContent( &in );
	}
	else
	{
		// svgz loading from kdecore/svgicons
		gzFile svgz = gzopen( path.latin1(), "ro" );
		if( !svgz )
			return KoFilter::FileNotFound;

		QByteArray data;
		bool done = false;

		char *buffer = new char[ 1024 ];

		while( !done )
		{
			memset( buffer, 0, 1024 );

			int ret = gzread( svgz, buffer, 1024 );
			if( ret == 0 )
				done = true;
			else if( ret == -1 )
				return KoFilter::FileNotFound;

			QDataStream dataStream( data, IO_WriteOnly | IO_Append );
	        dataStream.writeRawBytes( buffer, 1024 );
		}

		gzclose( svgz );

		inpdoc.setContent( data );
	}

	// Do the conversion!
	convert();

	KoStoreDevice* out = m_chain->storageFile( "root", KoStore::Write );
	if( !out )
	{
		kdError(30502) << "Unable to open output file!" << endl;
		return KoFilter::StorageCreationError;
	}
	QCString cstring = outdoc.toCString(); // utf-8 already
	out->writeBlock( cstring.data(), cstring.length() );

	in.close();
	return KoFilter::OK; // was successfull
}

void
SvgImport::convert()
{
	QDomElement docElem = inpdoc.documentElement();
	m_document.setWidth( 550.0);
	m_document.setHeight( 841.0 );
	// undo y-mirroring
	GraphicsContext *gc = new GraphicsContext;
	gc->matrix.scale( 1, -1 );
	gc->matrix.translate( 0, -841 );
	m_gc.push( gc );
	parseGroup( 0L, docElem );

	outdoc = m_document.saveXML();
}

#define DPI 90

double
SvgImport::parseUnit( const QString &unit )
{
	bool ok = false;
	double value = unit.toDouble( &ok );

	if( !ok )
	{
		if( unit.right( 2 ) == "pt" )
			value = ( value / 72.0 ) * DPI;
		else if( unit.right( 2 ) == "cm" )
			value = ( value / 2.54 ) * DPI;
		else if( unit.right( 2 ) == "pc" )
			value = ( value / 6.0 ) * DPI;
		else if( unit.right( 2 ) == "mm" )
			value = ( value / 25.4 ) * DPI;
		else if( unit.right( 2 ) == "in" )
			value = value * DPI;
		else if( unit.right( 2 ) == "pt" )
			value = ( value / 72.0 ) * DPI;
	}
	return value;
}

VColor
SvgImport::parseColor( const QString &s )
{
	VColor color;
	if( s.startsWith( "rgb(" ) )
	{
		QString parse = s.stripWhiteSpace();
		QStringList colors = QStringList::split( ',', parse );
		QString r = colors[0].right( ( colors[0].length() - 4 ) );
		QString g = colors[1];
		QString b = colors[2].left( ( colors[2].length() - 1 ) );

		if( r.contains( "%" ) )
		{
			r = r.left( r.length() - 1 );
			r = QString::number( int( ( double( 255 * r.toDouble() ) / 100.0 ) ) );
		}

		if( g.contains( "%" ) )
		{
			g = g.left( g.length() - 1 );
			g = QString::number( int( ( double( 255 * g.toDouble() ) / 100.0 ) ) );
		}

		if( b.contains( "%" ) )
		{
			b = b.left( b.length() - 1 );
			b = QString::number( int( ( double( 255 * b.toDouble() ) / 100.0 ) ) );
		}

		QColor c( r.toInt(), g.toInt(), b.toInt() );
		color.set( c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0 );
	}
	else
	{
		QColor c;
		c.setNamedColor( s );
		color.set( c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0 );
	}
	return color;
}

void
SvgImport::parseColorStops( VGradient *gradient, const QDomElement &e )
{
	VColor c;
	for( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		QDomElement stop = n.toElement();
		if( stop.tagName() == "stop" )
		{
			float offset;
			QString temp = stop.attribute( "offset" );
			if( temp.contains( '%' ) )
			{
				temp = temp.left( temp.length() - 1 );
				offset = temp.toFloat() / 100.0;
			}
			else
				offset = temp.toFloat();
			if( !stop.attribute( "stop-color" ).isEmpty() )
				c = parseColor( stop.attribute( "stop-color" ) );
			else
			{
				// try style attr
				QString style = stop.attribute( "style" ).simplifyWhiteSpace();
				QStringList substyles = QStringList::split( ';', style );
			    for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
				{
					QStringList substyle = QStringList::split( ':', (*it) );
					QString command	= substyle[0].stripWhiteSpace();
					QString params	= substyle[1].stripWhiteSpace();
					if( command == "stop-color" )
						c = parseColor( params );
				}

			}
			gradient->addStop( c, offset, 0.5 );
		}
	}
}

void
SvgImport::parseGradient( const QDomElement &e )
{
	VGradient gradient;
	gradient.clearStops();
	gradient.setRepeatMethod( VGradient::none );
	if( e.tagName() == "linearGradient" )
	{
		gradient.setOrigin( KoPoint( e.attribute( "x1" ).toDouble(), e.attribute( "y1" ).toDouble() ) );
		gradient.setVector( KoPoint( e.attribute( "x2" ).toDouble(), e.attribute( "y2" ).toDouble() ) );
	}
	else
	{
		gradient.setOrigin( KoPoint( e.attribute( "cx" ).toDouble(), e.attribute( "cy" ).toDouble() ) );
		gradient.setVector( KoPoint( e.attribute( "cx" ).toDouble() + e.attribute( "r" ).toDouble(),
									 e.attribute( "cy" ).toDouble() ) );
		gradient.setType( VGradient::radial );
	}
	// handle spread method
	QString spreadMethod = e.attribute( "spreadMethod" );
	if( !spreadMethod.isEmpty() )
	{
		if( spreadMethod == "reflect" )
			gradient.setRepeatMethod( VGradient::reflect );
		else if( spreadMethod == "repeat" )
			gradient.setRepeatMethod( VGradient::repeat );
	}
	parseColorStops( &gradient, e );
	QWMatrix mat = parseTransform( "gradientTransform" );
	gradient.transform( mat );
	m_gradients.insert( e.attribute( "id" ), gradient );
}

void
SvgImport::parsePA( GraphicsContext *gc, const QString &command, const QString &params )
{
	VColor fillcolor = gc->fill.color();
	VColor strokecolor = gc->stroke.color();

	if( command == "fill" )
	{
		if( params == "none" )
			gc->fill.setType( VFill::none );
		else if( params.startsWith( "url(" ) )
		{
			unsigned int start = params.find("#") + 1;
			unsigned int end = params.findRev(")");
			QString key = params.mid( start, end - start );
			gc->fill.gradient() = m_gradients[ key ];
			gc->fill.setType( VFill::grad );
		}
		else
		{
			fillcolor = parseColor( params );
			gc->fill.setType( VFill::solid );
		}
	}
	else if( command == "fill-rule" )
	{
		if( params == "nonzero" )
			gc->fill.setFillRule( VFill::winding );
		else if( params == "evenodd" )
			gc->fill.setFillRule( VFill::evenOdd );
	}
	else if( command == "stroke" )
	{
		if( params == "none" )
			gc->stroke.setType( VStroke::none );
		else if( params.startsWith( "url(" ) )
		{
			unsigned int start = params.find("#") + 1;
			unsigned int end = params.findRev(")");
			QString key = params.mid( start, end - start );
			gc->stroke.gradient() = m_gradients[ key ];
			gc->stroke.setType( VStroke::grad );
		}
		else
		{
			strokecolor = parseColor( params );
			gc->stroke.setType( VStroke::solid );
		}
	}
	else if( command == "stroke-width" )
		gc->stroke.setLineWidth( params.toDouble() );
	else if( command == "stroke-linestyle" )
	{
		if( params == "miter" )
			gc->stroke.setLineJoin( VStroke::joinMiter );
		else if( params == "round" )
			gc->stroke.setLineJoin( VStroke::joinRound );
		else if( params == "bevel" )
			gc->stroke.setLineJoin( VStroke::joinBevel );
	}
	else if( command == "stroke-linecap" )
	{
		if( params == "butt" )
			gc->stroke.setLineCap( VStroke::capButt );
		else if( params == "round" )
			gc->stroke.setLineCap( VStroke::capRound );
		else if( params == "square" )
			gc->stroke.setLineCap( VStroke::capSquare );
	}
	else if( command == "stroke-miterlimit" )
		gc->stroke.setMiterLimit( params.toFloat() );
	else if( command == "stroke-dasharray" )
	{
		QStringList dashes = QStringList::split( ' ', params );
		QValueList<float> array;
	    for( QStringList::Iterator it = dashes.begin(); it != dashes.end(); ++it )
			array.append( (*it).toFloat() );

		gc->stroke.dashPattern().setArray( array );
	}
	else if( command == "stroke-dashoffset" )
		gc->stroke.dashPattern().setOffset( params.toFloat() );
	// handle opacity
	else if( command == "stroke-opacity" )
		strokecolor.setOpacity( params.toFloat() );
	else if( command == "fill-opacity" )
		fillcolor.setOpacity( params.toFloat() );
	else if( command == "opacity" )
	{
		fillcolor.setOpacity( params.toFloat() );
		strokecolor.setOpacity( params.toFloat() );
	}
	else if( command == "font-family" )
	{
		QString family = params;
		family.replace( QRegExp( "'" ) , QChar( ' ' ) );
		gc->font.setFamily( family );
	}
	else if( command == "font-size" )
		gc->font.setPointSize( parseUnit( params ) );

	if( gc->fill.type() == VFill::solid )
		gc->fill.setColor( fillcolor );
	if( gc->stroke.type() == VStroke::solid )
		gc->stroke.setColor( strokecolor );
}

void
SvgImport::parseStyle( VObject *obj, const QDomElement &e )
{
	GraphicsContext *gc = new GraphicsContext;
	// set as default
	if( m_gc.current() )
		*gc = *( m_gc.current() );

	// try normal PA
	if( !e.attribute( "fill" ).isEmpty() )
		parsePA( gc, "fill", e.attribute( "fill" ) );
	if( !e.attribute( "fill-rule" ).isEmpty() )
		parsePA( gc, "fill-rule", e.attribute( "fill-rule" ) );
	if( !e.attribute( "stroke" ).isEmpty() )
		parsePA( gc, "stroke", e.attribute( "stroke" ) );
	if( !e.attribute( "stroke-width" ).isEmpty() )
		parsePA( gc, "stroke-width", e.attribute( "stroke-width" ) );
	if( !e.attribute( "stroke-linestyle" ).isEmpty() )
		parsePA( gc, "stroke-linestyle", e.attribute( "stroke-linestyle" ) );
	if( !e.attribute( "stroke-linecap" ).isEmpty() )
		parsePA( gc, "stroke-linecap", e.attribute( "stroke-linecap" ) );
	if( !e.attribute( "stroke-dasharray" ).isEmpty() )
		parsePA( gc, "stroke-dasharray", e.attribute( "stroke-dasharray" ) );
	if( !e.attribute( "stroke-dashoffset" ).isEmpty() )
		parsePA( gc, "stroke-dashoffset", e.attribute( "stroke-dashoffset" ) );
	if( !e.attribute( "stroke-opacity" ).isEmpty() )
		parsePA( gc, "stroke-opacity", e.attribute( "stroke-opacity" ) );
	if( !e.attribute( "fill-opacity" ).isEmpty() )
		parsePA( gc, "fill-opacity", e.attribute( "fill-opacity" ) );
	if( !e.attribute( "opacity" ).isEmpty() )
		parsePA( gc, "opacity", e.attribute( "opacity" ) );

	// try style attr
	QString style = e.attribute( "style" ).simplifyWhiteSpace();
	QStringList substyles = QStringList::split( ';', style );
    for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
	{
		QStringList substyle = QStringList::split( ':', (*it) );
		QString command	= substyle[0].stripWhiteSpace();
		QString params	= substyle[1].stripWhiteSpace();
		parsePA( gc, command, params );
	}

	obj->setFill( gc->fill );
	obj->setStroke( gc->stroke );
	QWMatrix mat = parseTransform( e.attribute( "transform" ) );
	gc->matrix *= mat;
	obj->transform( gc->matrix );
	m_gc.push( gc );
}

void
SvgImport::parseGroup( VGroup *grp, const QDomElement &e )
{
	for( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		QDomElement b = n.toElement();
		if( b.isNull() ) continue;
		VObject *obj = 0L;
		if( b.tagName() == "g" )
		{
			VGroup *group;
			if ( grp )
				group = new VGroup( grp );
			else
				group = new VGroup( &m_document );

			parseStyle( group, b );
			parseGroup( group, b );
			if( grp )
				grp->append( group );
			else
				m_document.append( group );
			m_gc.pop();
			continue;
		}
		if( b.tagName() == "defs" )
		{
			parseGroup( 0L, b ); 	// try for gradients at least
			continue;
		}
		else if( b.tagName() == "linearGradient" || b.tagName() == "radialGradient" )
		{
			parseGradient( b );
			continue;
		}
		else if( b.tagName() == "rect" )
		{
			double x		= parseUnit( b.attribute( "x" ) );
			double y		= parseUnit( b.attribute( "y" ) );
			double width	= parseUnit( b.attribute( "width" ) );
			double height	= parseUnit( b.attribute( "height" ) );
			obj = new VRectangle( 0L, KoPoint( x, height + y ) , width, height );
		}
		else if( b.tagName() == "ellipse" )
		{
			double rx		= parseUnit( b.attribute( "rx" ) );
			double ry		= parseUnit( b.attribute( "ry" ) );
			double left		= parseUnit( b.attribute( "cx" ) ) - ( rx / 2.0 );
			double top		= parseUnit( b.attribute( "cy" ) ) + ( ry / 2.0 );
			// Append the ellipse to the document
			obj = new VEllipse( 0L, KoPoint( left, top ), rx * 2.0, ry * 2.0 );
		}
		else if( b.tagName() == "circle" )
		{
			double r		= parseUnit( b.attribute( "r" ) );
			double left		= parseUnit( b.attribute( "cx" ) ) - ( r / 2.0 );
			double top		= parseUnit( b.attribute( "cy" ) ) + ( r / 2.0 );
			// Append the ellipse to the document
			obj = new VEllipse( 0L, KoPoint( left, top ), r * 2.0, r * 2.0 );
		}
		else if( b.tagName() == "line" )
		{
			VComposite *path = new VComposite( &m_document );
			double x1 = b.attribute( "x1" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "x1" ) );
			double y1 = b.attribute( "y1" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "y1" ) );
			double x2 = b.attribute( "x2" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "x2" ) );
			double y2 = b.attribute( "y2" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "y2" ) );
			path->moveTo( KoPoint( x1, y1 ) );
			path->lineTo( KoPoint( x2, y2 ) );
			obj = path;
		}
		else if( b.tagName() == "polyline" || b.tagName() == "polygon" )
		{
			VComposite *path = new VComposite( &m_document );
			bool bFirst = true;

			QString points = b.attribute( "points" ).simplifyWhiteSpace();
			points.replace( QRegExp( "," ), " " );
			points.replace( QRegExp( "\r" ), "" );
		    points.replace( QRegExp( "\n" ), "" );
			QStringList pointList = QStringList::split( ' ', points );
			for( QStringList::Iterator it = pointList.begin(); it != pointList.end(); it++ )
			{
				if( bFirst )
				{
					path->moveTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
					bFirst = false;
				}
				else
					path->lineTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
			}
			if( b.tagName() == "polygon" ) path->close();
			obj = path;
		}
		else if( b.tagName() == "path" )
		{
			VComposite *path = new VComposite( &m_document );
			parsePath( path, b );
			obj = path;
		}
		else if( b.tagName() == "text" )
		{
			VText *text = new VText( &m_document );
			text->setText( b.text() );
			VPath base( 0L );
			double x = parseUnit( b.attribute( "x" ) );
			double y = parseUnit( b.attribute( "y" ) );
			base.moveTo( KoPoint( x, y ) );
			base.lineTo( KoPoint( x + 10, y ) );
			text->setBasePath( base );
			parseStyle( text, b );
			text->setFont( m_gc.current()->font );
			if( text )
				grp->append( text );
			else
				m_document.append( text );
			m_gc.pop();
			continue;
		}
		if( !obj ) continue;
		parseStyle( obj, b );
		if( grp )
			grp->append( obj );
		else
			m_document.append( obj );
		m_gc.pop();
	}
}

// parses the coord into number and forwards to the next token
const char *
SvgImport::getCoord( const char *ptr, double &number )
{
	int integer, exponent;
	double decimal, frac;
	int sign, expsign;

	exponent = 0;
	integer = 0;
	frac = 1.0;
	decimal = 0;
	sign = 1;
	expsign = 1;

	// read the sign
	if(*ptr == '+')
		ptr++;
	else if(*ptr == '-')
	{
		ptr++;
		sign = -1;
	}

	// read the integer part
	while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
		integer = (integer * 10) + *(ptr++) - '0';
	if(*ptr == '.') // read the decimals
    {
		ptr++;
		while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
			decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

	if(*ptr == 'e' || *ptr == 'E') // read the exponent part
	{
		ptr++;

		// read the sign of the exponent
		if(*ptr == '+')
			ptr++;
		else if(*ptr == '-')
		{
			ptr++;
			expsign = -1;
		}

		exponent = 0;
		while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
		{
			exponent *= 10;
			exponent += *ptr - '0';
			ptr++;
		}
    }
	number = integer + decimal;
	number *= sign * pow(10, expsign * exponent);

	// skip the following space
	if(*ptr == ' ')
		ptr++;

	return ptr;
}

void
SvgImport::parsePath( VComposite *obj, const QDomElement &e )
{
	QString d = e.attribute( "d" ).replace( QRegExp( "," ), " ");

	if( !d.isEmpty() )
	{
		d = d.simplifyWhiteSpace();

		const char *ptr = d.latin1();
		const char *end = d.latin1() + d.length() + 1;

		double contrlx, contrly, curx, cury, subpathx, subpathy, tox, toy, x1, y1, x2, y2, xc, yc;
		bool relative;
		char command = *(ptr++), lastCommand = ' ';

		curx = cury = contrlx = contrly = 0.0;
		while( ptr < end )
		{
			if( *ptr == ' ' )
				ptr++;

			relative = false;

			std::cout << "Command : " << command << std::endl;
			switch( command )
			{
				case 'm':
					relative = true;
				case 'M':
				{
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					subpathx = curx = relative ? curx + tox : tox;
					subpathy = cury = relative ? cury + toy : toy;

					//if( lastCommand == 'z' || lastCommand == 'Z' )
					//	path->close();
					obj->end();
					obj->moveTo( KoPoint( curx, cury ) );
					break;
				}
				case 'l':
					relative = true;
				case 'L':
				{
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					curx = relative ? curx + tox : tox;
					cury = relative ? cury + toy : toy;

					obj->lineTo( KoPoint( curx, cury ) );
					break;
				}
				case 'h':
				{
					ptr = getCoord( ptr, tox );
					curx += tox;
					obj->lineTo( KoPoint( curx, cury ) );
					break;
				}
				case 'H':
				{
					ptr = getCoord( ptr, tox );
					curx = tox;
					obj->lineTo( KoPoint( curx, cury ) );
					break;
				}
				case 'v':
				{
					ptr = getCoord( ptr, toy );
					cury += toy;
					obj->lineTo( KoPoint( curx, cury ) );
					break;
				}
				case 'V':
				{
					ptr = getCoord( ptr, toy );
					cury = toy;
					obj->lineTo( KoPoint( curx, cury ) );
					break;
				}
				case 'z':
				case 'Z':
				{
					// reset curx, cury for next path
					curx = subpathx;
					cury = subpathy;
					obj->close();
					break;
				}
				case 'c':
					relative = true;
				case 'C':
				{
					ptr = getCoord( ptr, x1 );
					ptr = getCoord( ptr, y1 );
					ptr = getCoord( ptr, x2 );
					ptr = getCoord( ptr, y2 );
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if(relative)
						obj->curveTo( KoPoint( curx + x1, cury + y1 ), KoPoint( curx + x2, cury + y2 ),
									   KoPoint( curx + tox, cury + toy ) );
					else
						obj->curveTo( KoPoint( x1, y1 ), KoPoint( x2, y2 ), KoPoint( tox, toy ) );
					contrlx = relative ? curx + x2 : x2;
					contrly = relative ? cury + y2 : y2;
					curx = relative ? curx + tox : tox;
					cury = relative ? cury + toy : toy;

					break;
				}
				case 's':
					relative = true;
				case 'S':
				{
					ptr = getCoord( ptr, x2 );
					ptr = getCoord( ptr, y2 );
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if(relative)
						obj->curveTo( KoPoint( 2 * curx - contrlx, 2 * cury - contrly ),
									   KoPoint( curx + x2, cury + y2 ),
									   KoPoint( curx + tox, cury + toy ) );
					else
						obj->curveTo( KoPoint( 2 * curx - contrlx, 2 * cury - contrly ),
									   KoPoint( x2, y2 ), KoPoint( tox, toy ) );
					contrlx = relative ? curx + x2 : x2;
					contrly = relative ? cury + y2 : y2;
					curx = relative ? curx + tox : tox;
					cury = relative ? cury + toy : toy;
					break;
				}
				case 'q':
					relative = true;
				case 'Q':
				{
					ptr = getCoord( ptr, x1 );
					ptr = getCoord( ptr, y1 );
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					if(relative)
						obj->curveTo( KoPoint( (curx + 2 * (x1 + curx)) * (1.0 / 3.0), (cury + 2 * (y1 + cury)) * (1.0 / 3.0) ),
									   KoPoint( ((curx + tox) + 2 * (x1 + curx)) * (1.0 / 3.0), ((cury + toy) + 2 * (y1 + cury)) * (1.0 / 3.0) ),
									   KoPoint( curx + tox, cury + toy ) );
					else
						obj->curveTo( KoPoint( (curx + 2 * x1) * (1.0 / 3.0), (cury + 2 * y1) * (1.0 / 3.0) ),
									   KoPoint( (tox + 2 * x1) * (1.0 / 3.0), (toy + 2 * y1) * (1.0 / 3.0) ), KoPoint( tox, toy ) );
					contrlx = relative ? curx + x1 : (tox + 2 * x1) * (1.0 / 3.0);
					contrly = relative ? cury + y1 : (toy + 2 * y1) * (1.0 / 3.0);
					curx = relative ? curx + tox : tox;
					cury = relative ? cury + toy : toy;
					break;
				}
				case 't':
					relative = true;
				case 'T':
				{
					xc = 2 * curx - contrlx;
	                yc = 2 * cury - contrly;

					ptr = getCoord(ptr, tox);
					ptr = getCoord(ptr, toy);

					if(relative)
						obj->curveTo( KoPoint( (curx + 2 * xc) * (1.0 / 3.0), (cury + 2 * yc) * (1.0 / 3.0) ),
									   KoPoint( ((curx + tox) + 2 * xc) * (1.0 / 3.0), ((cury + toy) + 2 * yc) * (1.0 / 3.0) ),
									   KoPoint( curx + tox, cury + toy ) );
					else
						obj->curveTo( KoPoint( (curx + 2 * xc) * (1.0 / 3.0), (cury + 2 * yc) * (1.0 / 3.0) ),
									   KoPoint( (tox + 2 * xc) * (1.0 / 3.0), (toy + 2 * yc) * (1.0 / 3.0) ), KoPoint( tox, toy ) );
					contrlx = xc;
					contrly = yc;
					curx = relative ? curx + tox : tox;
					cury = relative ? cury + toy : toy;
					break;
				}
				// TODO : arc support
				case 'a':
					relative = true;
				case 'A':
				{
					bool largeArc, sweep;
					double angle, rx, ry;
					ptr = getCoord( ptr, rx );
					ptr = getCoord( ptr, ry );
					ptr = getCoord( ptr, angle );
					ptr = getCoord( ptr, tox );

					largeArc = tox == 1;
					ptr = getCoord( ptr, tox );
					sweep = tox == 1;
					ptr = getCoord( ptr, tox );
					ptr = getCoord( ptr, toy );

					/*if(relative)
						pathSegList()->appendItem(createSVGPathSegArcRel(tox, toy, rx, ry, angle, largeArc, sweep));
					else
						pathSegList()->appendItem(createSVGPathSegArcAbs(tox, toy, rx, ry, angle, largeArc, sweep));*/
				}
			}

			lastCommand = command;

			if(*ptr == '+' || *ptr == '-' || (*ptr >= '0' && *ptr <= '9'))
			{
				// there are still coords in this command
				if(command == 'M')
					command = 'L';
				else if(command == 'm')
					command = 'l';
			}
			else
				command = *(ptr++);

			if( lastCommand != 'C' && lastCommand != 'c' &&
				lastCommand != 'S' && lastCommand != 's' &&
				lastCommand != 'Q' && lastCommand != 'q' &&
				lastCommand != 'T' && lastCommand != 't')
			{
				contrlx = curx;
				contrly = cury;
			}
		}
	}
}

const double deg2rad = 0.017453292519943295769; // pi/180

QWMatrix
SvgImport::parseTransform( const QString &transform )
{
	QWMatrix result;

	// Split string for handling 1 transform statement at a time
	QStringList subtransforms = QStringList::split(')', transform);
	QStringList::ConstIterator it = subtransforms.begin();
	QStringList::ConstIterator end = subtransforms.end();
	for(; it != end; ++it)
	{
		QStringList subtransform = QStringList::split('(', (*it));

		subtransform[0] = subtransform[0].stripWhiteSpace().lower();
		subtransform[1] = subtransform[1].simplifyWhiteSpace();
		QRegExp reg("[a-zA-Z,( ]");
		QStringList params = QStringList::split(reg, subtransform[1]);

		if(subtransform[0].startsWith(";") || subtransform[0].startsWith(","))
			subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

		if(subtransform[0] == "rotate")
		{
			if(params.count() == 3)
			{
				float x = params[1].toFloat();
				float y = params[2].toFloat();

				result.translate(x, y);
				result.rotate(params[0].toFloat());
				result.translate(-x, -y);
			}
			else
				result.rotate(params[0].toFloat());
		}
		else if(subtransform[0] == "translate")
		{
			if(params.count() == 2)
				result.translate(params[0].toFloat(), params[1].toFloat());
			else    // Spec : if only one param given, assume 2nd param to be 0
				result.translate(params[0].toFloat() , 0);
		}
		else if(subtransform[0] == "scale")
		{
			if(params.count() == 2)
				result.scale(params[0].toFloat(), params[1].toFloat());
			else    // Spec : if only one param given, assume uniform scaling
				result.scale(params[0].toFloat(), params[0].toFloat());
		}
		else if(subtransform[0] == "skewx")
			result.shear(tan(params[0].toFloat() * deg2rad), 0.0F);
		else if(subtransform[0] == "skewy")
			result.shear(tan(params[0].toFloat() * deg2rad), 0.0F);
		else if(subtransform[0] == "skewy")
			result.shear(0.0F, tan(params[0].toFloat() * deg2rad));
		else if(subtransform[0] == "matrix")
		{
			if(params.count() >= 6)
				result.setMatrix(params[0].toFloat(), params[1].toFloat(), params[2].toFloat(), params[3].toFloat(), params[4].toFloat(), params[5].toFloat());
		}
	}

	return result;
}

#include <svgimport.moc>
