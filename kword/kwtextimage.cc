/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "kwtextimage.h"
#include "kwtextframeset.h"
#include "kwdoc.h"
#include "defs.h"
#include <kdebug.h>

KWTextImage::KWTextImage( KWTextDocument *textdoc, const QString & filename )
    : KoTextCustomItem( textdoc ), place( PlaceInline )
{
    KWDocument * doc = textdoc->textFrameSet()->kWordDocument();
    if ( !filename.isEmpty() )
    {
        m_image = doc->imageCollection()->loadPicture( filename );
        Q_ASSERT( !m_image.isNull() );
        resize(); // Zoom if necessary
    }
}

void KWTextImage::setImage( const KoPicture &image )
{
    kdDebug() << "KWTextImage::setImage" << endl;
    m_image = image;
    resize();
}

void KWTextImage::resize()
{
    if ( m_deleted )
        return;
    if ( !m_image.isNull() ) {
        KWDocument * doc = static_cast<KWTextDocument *>(parent)->textFrameSet()->kWordDocument();
        width = m_image.getOriginalSize().width();
        width = (int)( doc->zoomItX( (double)width ) / POINT_TO_INCH( QPaintDevice::x11AppDpiX() ) );
        height = m_image.getOriginalSize().height();
        height = (int)( doc->zoomItY( (double)height ) / POINT_TO_INCH( QPaintDevice::x11AppDpiY() ) );
        // This ensures 1-1 at 100% on screen, but allows zooming and printing with correct DPI values
        //kdDebug() << "KWTextImage::resize scaling to " << width << ", " << height << endl;
        m_image.setSize( QSize( width, height ) );
    }
}

void KWTextImage::drawCustomItem( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected , const int /*offset*/)
{
    if ( placement() != PlaceInline ) {
        x = xpos;
        y = ypos;
    }

    if ( m_image.isNull() ) {
        kdDebug() << "KWTextImage::draw null image!" << endl;
        p->fillRect( x, y, 50, 50, cg.dark() );
        return;
    }

    QRect rect( QPoint(x, y) , m_image.getSize() );
    if ( !rect.intersects( QRect( cx, cy, cw, ch ) ) )
        return;

    QPixmap pixmap=m_image.generatePixmap(m_image.getSize());
    if ( placement() == PlaceInline )
        p->drawPixmap( x, y, pixmap );
    else
        p->drawPixmap( cx, cy, pixmap, cx - x, cy - y, cw, ch );

    if ( selected && placement() == PlaceInline && p->device()->devType() != QInternal::Printer ) {
        p->fillRect( rect , QBrush( cg.highlight(), QBrush::Dense4Pattern) );
    }
}

void KWTextImage::save( QDomElement & parentElem )
{
    QDomElement imageElem = parentElem.ownerDocument().createElement( "IMAGE" );
    parentElem.appendChild( imageElem );
    QDomElement elem = parentElem.ownerDocument().createElement( "FILENAME" );
    imageElem.appendChild( elem );
    elem.setAttribute( "value", image().getKey().filename() );
}

void KWTextImage::load( QDomElement & parentElem )
{
    // <IMAGE>
    QDomElement image = parentElem.namedItem( "IMAGE" ).toElement();
    if ( !image.isNull() )
	parentElem = image;
    // The FILENAME tag can be under IMAGE, or directly under parentElement in old koffice-1.0 docs.
    // <FILENAME>
    QDomElement filenameElement = parentElem.namedItem( "FILENAME" ).toElement();
    if ( !filenameElement.isNull() )
    {
        QString filename = filenameElement.attribute( "value" );
        KWDocument * doc = static_cast<KWTextDocument *>(parent)->textFrameSet()->kWordDocument();
        doc->addImageRequest( KoPictureKey( filename, QDateTime::currentDateTime() ), this );
    }
    else
        kdError(32001) << "Missing FILENAME tag in IMAGE" << endl;
}
