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

#include "kwformat.h"
#include <kdebug.h>
#include <kwdoc.h>

KWTextFormatCollection::KWTextFormatCollection( KWDocument * doc )
    : QTextFormatCollection(), m_cachedFormat( 0L )
{
    delete defaultFormat();

    QColor color = QApplication::palette().color( QPalette::Active, QColorGroup::Text );
    setDefaultFormat( new KWTextFormat( doc->defaultFont(), color, this ) );

}

QTextFormat *KWTextFormatCollection::format( const QFont &f, const QColor &c )
{
    if ( m_cachedFormat && m_cfont == f && m_ccol == c ) {
        m_cachedFormat->addRef();
        return m_cachedFormat;
    }

    QString key = QTextFormat::getKey( f, c, FALSE, QString::null, QString::null, QTextFormat::AlignNormal );
    kdDebug() << "format() textformat=" << this << " pointsizefloat=" << f.pointSizeFloat() << endl;
    key += QString::number( (int)(f.pointSizeFloat() * 10) ); // SYNC with generateKey
    m_cachedFormat = static_cast<KWTextFormat *>( dict().find( key ) );
    m_cfont = f;
    m_ccol = c;

    if ( m_cachedFormat ) {
        m_cachedFormat->addRef();
        return m_cachedFormat;
    }

    m_cachedFormat = static_cast<KWTextFormat *>( createFormat( f, c ) );
    dict().insert( m_cachedFormat->key(), m_cachedFormat );
    return m_cachedFormat;
}

void KWTextFormatCollection::remove( QTextFormat *f )
{
    if ( m_cachedFormat == f )
        m_cachedFormat = 0;
    QTextFormatCollection::remove( f );
}

///

void KWTextFormat::copyFormat( const QTextFormat & nf, int flags )
{
    QTextFormat::copyFormat( nf, flags );
    if ( flags & QTextFormat::Size )
        fn.setPointSizeFloat( nf.font().pointSizeFloat() );
    update();
}

void KWTextFormat::setPointSizeFloat( float size )
{
    if ( fn.pointSizeFloat() == size )
        return;
    fn.setPointSizeFloat( size );
    update();
}

void KWTextFormat::generateKey()
{
    QTextFormat::generateKey();
    QString k = key();
    setKey( k + QString::number( (int)(fn.pointSizeFloat() * 10) ) ); // SYNC with ::format above
    kdDebug() << "generateKey textformat=" << this << " k=" << k << " pointsizefloat=" << fn.pointSizeFloat() << endl;
}


void KWTextFormat::setStrikeOut(bool b)
{
  if ( fn.strikeOut() == b )
        return;
    fn.setStrikeOut( b );
    update();
}
