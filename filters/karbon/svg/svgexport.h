/* This file is part of the KDE project
   Copyright (C) 2002 Lars Siebold <khandha5@gmx.net>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005,2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef SVGEXPORT_H
#define SVGEXPORT_H

#include <KoFilter.h>
#include <QtGui/QGradient>

class VDocument;
class KoShapeLayer;
class KoShapeContainer;
class KoShape;
class KoPathShape;
class KoShapeBorderModel;
class QTextStream;
class QPixmap;
class QImage;
class QColor;
class QBrush;

class SvgExport : public KoFilter
{
    Q_OBJECT

public:
    SvgExport( QObject* parent, const QStringList& );
    virtual ~SvgExport() {}

    virtual KoFilter::ConversionStatus convert( const QByteArray& from, const QByteArray& to );

private:
    void saveDocument( VDocument& document );
    void saveLayer( KoShapeLayer * layer );
    void saveGroup( KoShapeContainer * group );
    void saveShape( KoShape * shape );
    void savePath( KoPathShape * path );

    void saveImage( QImage& image );
    //void saveText( VText& text );

    void getFill( const QBrush & fill, QTextStream *stream );

    void getStroke( const KoShapeBorderModel * stroke, QTextStream *stream );
    void getColorStops( const QGradientStops & colorStops );
    void getGradient( const QGradient * grad );
    void getPattern( const QPixmap& patt );
    void getHexColor( QTextStream *, const QColor & color  );
    QString getID( KoShape *obj );

    QTextStream* m_stream;
    QTextStream* m_defs;
    QTextStream* m_body;

    unsigned int m_indent;
    unsigned int m_indent2;
};

#endif

