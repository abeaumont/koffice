/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kppixmapcollection.h"

#include <qstring.h>
#include <qwmatrix.h>

#include <stdio.h>

/******************************************************************/
/* Class: KPPixmapDataCollection				  */
/******************************************************************/

/*================================================================*/
KPPixmapDataCollection::~KPPixmapDataCollection()
{
    data.clear();
    refs.clear();
}

/*================================================================*/
QImage *KPPixmapDataCollection::findPixmapData( const Key &key )
{
    //printf("	KPPixmapDataCollection::findPixmapData( key = %s )\n", key.toString().latin1() );
    //printf("	  data.count = %d\n", data.count() );

    QMap< Key, QImage >::Iterator it = data.find ( key );

    if ( it != data.end() && it.key() == key ) {
	//printf( "    fond pixmap data %s\n", it.key().toString().latin1() );
	return &it.data();
    } else {
	//printf( "    did NOT find pixmap data\n" );
	return 0L;
    }
}

/*================================================================*/
QImage *KPPixmapDataCollection::insertPixmapData( const Key &key, const QImage &img )
{
    //printf("	KPPixmapDataCollection::insertPixmapData( key = %s, img = %d )\n", key.toString().latin1(), !img.isNull() );

    QImage *tmp = findPixmapData( key );
    if ( tmp )
	return tmp;
    QImage *image = new QImage( img );

    image->detach();
    data.insert( Key( key ), *image );

    int ref = 1;
    refs.insert( Key( key ), ref );

    return image;
    return &data[ key ];
}

/*================================================================*/
void KPPixmapDataCollection::setPixmapOldVersion( const Key &key, const char *_data )
{
    if ( data.contains( key ) )
	return;

    QCString s( _data );
    int i = s.find( ( char )1, 0 );

    while ( i != -1 ) {
	s[ i ] = '\"';
	i = s.find( ( char )1, i + 1 );
    }

    QImage img;
    img.loadFromData( s, "XPM" );
    insertPixmapData( key, img );
}

/*================================================================*/
void KPPixmapDataCollection::setPixmapOldVersion( const Key &key )
{
    if ( data.contains( key ) )
	return;

    QImage img( key.filename );
    insertPixmapData( key, img );
}

/*================================================================*/
void KPPixmapDataCollection::addRef( const Key &key )
{
    if ( !allowChangeRef )
	return;

//     printf( "	 KPPixmapDataCollection::addRef( key = %s )\n", key.toString().latin1() );

    if ( refs.contains( key ) ) {
	int ref = refs[ key ];
	refs[ key ] = ++ref;
// 	printf( "    ref: %d\n", refs[ key ] );
    }
}

/*================================================================*/
void KPPixmapDataCollection::removeRef( const Key &key )
{
    if ( !allowChangeRef )
	return;

//     printf( "	 KPPixmapDataCollection::removeRef( key = %s )\n", key.toString().latin1() );

    if ( refs.contains( key ) ) {
	int ref = refs[ key ];
	refs[ key ] = --ref;
// 	printf( "    ref: %d\n", refs[ key ] );

	if ( ref == 0 ) {
	    refs.remove( key );
	    data.remove( key );
//  	    printf( "    remove %s\n", key.toString().latin1() );
	}
    }
}

/******************************************************************/
/* Class: KPPixmapCollection					  */
/******************************************************************/

/*================================================================*/
KPPixmapCollection::~KPPixmapCollection()
{
    pixmaps.clear();
}

/*================================================================*/
QPixmap* KPPixmapCollection::findPixmap( Key &key )
{
    //printf( "	 KPPixmapCollection::findPixmap( key = %s )\n", key.toString().latin1() );

    if ( key.size == orig_size ) {
	QImage *i = dataCollection.findPixmapData( key.dataKey );
	if ( i )
	    key.size = i->size();
	else {
	    QImage img( key.dataKey.filename );
	    key.size = img.size();
	}
    }

    //printf( "	   key = %s )\n", key.toString().latin1() );

    QMap< Key, QPixmap >::Iterator it = pixmaps.begin();
    it = pixmaps.find( key );

    if ( it != pixmaps.end() && it.key() == key ) {
	//printf( "    pixmap found in pixmaps: %s\n", it.key().toString().latin1() );
	addRef( key );
	return &it.data();
    } else {
	QImage *img = dataCollection.findPixmapData( key.dataKey );
	if ( img ) {
	    //printf( "	   pixmap found in data collection: %s\n", key.dataKey.toString().latin1() );
	    dataCollection.addRef( key.dataKey );
	    return loadPixmap( *img, key, true );
	} else {
	    QImage image( key.dataKey.filename );

	    //printf( "	   pixmap not found anywhere\n" );
	    dataCollection.insertPixmapData( key.dataKey, image );
	    return loadPixmap( image, key, true );
	}
    }
}

/*================================================================*/
void KPPixmapCollection::addRef( const Key &key )
{
    if ( !allowChangeRef )
	return;

//     printf( "	 KPPixmapCollection::addRef( key = %s )\n", key.toString().latin1() );

    if ( refs.contains( key ) ) {
	int ref = refs[ key ];
	refs[ key ] = ++ref;
// 	printf( "    ref: %d\n", refs[ key ] );
    }

    dataCollection.addRef( key.dataKey );
}

/*================================================================*/
void KPPixmapCollection::removeRef( const Key &key )
{
    if ( !allowChangeRef )
	return;

//     printf( "	 KPPixmapCollection::removeRef( key = %s )\n", key.toString().latin1() );

    if ( refs.contains( key ) ) {
	int ref = refs[ key ];
	refs[ key ] = --ref;
// 	printf( "    ref: %d\n", refs[ key ] );

	if ( ref == 0 ) {
	    refs.remove( key );
	    pixmaps.remove( key );
// 	    printf( "	   remove: %s\n", key.toString().latin1() );
	}
    }

    dataCollection.removeRef( key.dataKey );
}

/*================================================================*/
QPixmap *KPPixmapCollection::loadPixmap( const QImage &image, const Key &key, bool insert )
{
    //printf( "	 KPPixmapCollection::loadPixmap( image = %d, key = %s, insert = %d )\n",
    //	      !image.isNull(), key.toString().latin1(), insert );

    QPixmap *pixmap = new QPixmap;
    pixmap->convertFromImage( image );

    if ( !pixmap->isNull() ) {
	QSize size = key.size;
	if ( size != pixmap->size() && size != orig_size && pixmap->width() != 0 && pixmap->height() != 0 ) {
	    QWMatrix m;
	    m.scale( static_cast<float>( size.width() ) / static_cast<float>( pixmap->width() ),
		     static_cast<float>( size.height() ) / static_cast<float>( pixmap->height() ) );
	    *pixmap = pixmap->xForm( m );
	}
    }

    if ( insert ) {
	pixmaps.insert( Key( key ), *pixmap );
	int ref = 1;
	refs.insert( Key( key ), ref );
	return pixmap;
	return &pixmaps[ key ];
    }

    return pixmap;
}

/*================================================================*/
ostream& operator<<( ostream &out, KPPixmapDataCollection::Key &key )
{
    QDate date = key.lastModified.date();
    QTime time = key.lastModified.time();

    out << " filename=\"" << key.filename.latin1() << "\" year=\"" << date.year()
	<< "\" month=\"" << date.month() << "\" day=\"" << date.day()
	<< "\" hour=\"" << time.hour() << "\" minute=\"" << time.minute()
	<< "\" second=\"" << time.second() << "\" msec=\"" << time.msec() << "\" ";

    return out;
}

/*================================================================*/
ostream& operator<<( ostream &out, KPPixmapCollection::Key &key )
{
    out << key.dataKey << " width=\"" << key.size.width() << "\" height=\""
	<< key.size.height() << "\" ";

    return out;
}
