/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TokenElement.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>
#include <QFontMetricsF>
#include <kdebug.h>

TokenElement::TokenElement( BasicElement* parent ) : BasicElement( parent )
{}

const QList<BasicElement*> TokenElement::childElements()
{
    // only return the mglyph elements
    QList<BasicElement*> tmpList;
    foreach( BasicElement* tmp, m_content )
        if( tmp != this )
            tmpList << tmp;

    return tmpList;
}

void TokenElement::paint( QPainter& painter, AttributeManager* am )
{
     // set the painter to use background and text colors
    //BasicElement::paint(painter, am);
    painter.setBackgroundMode( Qt::OpaqueMode  );
    painter.setBackground( QBrush( am->mathBackground( this ) ) );
    painter.setPen( am->mathColor( this ) );
    //painter.setBrush( Qt::SolidPattern );
    painter.translate( 0, baseLine() );
    painter.drawRect(0, -baseLine(), width(), baseLine() );
    m_contentPath.addRect(0, -baseLine(), width(), baseLine() );
    painter.drawPath( m_contentPath );  // draw content which is buffered as path

}

void TokenElement::layout( const AttributeManager* am )
{
    m_contentPath = QPainterPath();             // save the token in an empty path
    int rawCounter = 0;
    foreach( BasicElement* tmp, m_content )
        if( tmp == this )                       // a normal token
        {
            renderToPath( m_rawStringList[ rawCounter ], m_contentPath );
            rawCounter++;
        }
        else                                    // a mglyph element
        {
            tmp->setOrigin( QPointF( m_contentPath.boundingRect().right(), 0.0 ) );
            m_contentPath.moveTo( tmp->origin().x()+ tmp->width(), 0.0 );
        }
    


    kDebug() << "Bounding rect is at " << m_contentPath.boundingRect().x() << "," <<  m_contentPath.boundingRect().y();
    kDebug() << "Bounding rect has right, bottom  " << m_contentPath.boundingRect().right() << "," <<  m_contentPath.boundingRect().bottom();
    kDebug() << "Bounding rect has width, height  " << m_contentPath.boundingRect().width() << "," <<  m_contentPath.boundingRect().height();
    kDebug() << "current position is  " << m_contentPath.currentPosition().x() << "," <<  m_contentPath.currentPosition().y();
    setWidth( m_contentPath.boundingRect().right() );
    setHeight( m_contentPath.boundingRect().height() );
    // As the text is added to ( 0 / 0 ) the baseline equals the top edge of the
    // elements bounding rect, while translating it down the text's baseline moves too
    setBaseLine( -m_contentPath.boundingRect().y() ); // set baseline accordingly

}

BasicElement* TokenElement::acceptCursor( CursorDirection direction )
{
    return 0;
}

bool TokenElement::readMathMLContent( const KoXmlElement& element )
{
    BasicElement* tmpGlyph;
    KoXmlNode node = element.firstChild();
    while (! node.isNull() ) {
        if( node.isElement() )
        {
            KoXmlElement tmp = node.toElement();
            tmpGlyph = ElementFactory::createElement( tmp.tagName(), this );
            m_content << tmpGlyph;
            tmpGlyph->readMathML( tmp );
        }
        else
        {
            m_rawStringList << node.toText().data().trimmed();
            m_content << this;
        }
        node = node.nextSibling();
    }
    return true;
}

void TokenElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    int rawCounter = 0;
    foreach( BasicElement* tmp, m_content )
        if( tmp == this )
        {
            writer->addTextNode( m_rawStringList[ rawCounter ] );
            rawCounter++;
        }
        else
            tmp->writeMathML( writer );
}

