/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

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

DESCRIPTION
*/

#include <kdebug.h>
#include <ktempfile.h>
#include <kmimetype.h>
#include <kgenericfactory.h>
#include <koFilterChain.h>
#include <qfile.h>
#include <msodimport.h>
#include <qpointarray.h>

typedef KGenericFactory<MSODImport, KoFilter> MSODImportFactory;
K_EXPORT_COMPONENT_FACTORY( libmsodimport, MSODImportFactory( "msodimport" ) );

MSODImport::MSODImport(
    KoFilter *,
    const char *,
    const QStringList&) :
        KoEmbeddingFilter(), Msod(100)
{
}

MSODImport::~MSODImport()
{
}

KoFilter::ConversionStatus MSODImport::convert( const QCString& from, const QCString& to )
{
    if (to != "application/x-kontour" || from != "image/x-msod")
        return KoFilter::NotImplemented;

    // Get configuration data: the shape id, and any delay stream that we were given.
    unsigned shapeId;
    emit commSignalShapeID( shapeId );
    const char *delayStream = 0L;
    emit commSignalDelayStream( delayStream );
    kdDebug( s_area ) << "##################################################################" << endl;
    kdDebug( s_area ) << "shape id: " << shapeId << endl;
    kdDebug( s_area ) << "delay stream: " << delayStream << endl;
    kdDebug( s_area ) << "##################################################################" << endl;
/*
    QString config = ""; // ###### FIXME: We aren't able to pass config data right now
    QStringList args = QStringList::split(";", config);
    unsigned i;

    kdDebug(s_area) << "MSODImport::filter: config: " << config << endl;
    for (i = 0; i < args.count(); i++)
    {
        if (args[i].startsWith("shape-id="))
        {
            shapeId = args[i].mid(9).toUInt();
        }
        else
        if (args[i].startsWith("delay-stream="))
        {
            delayStream = (const char *)args[i].mid(13).toULong();
        }
        else
        {
            kdError(s_area) << "Invalid argument: " << args[i] << endl;
            return KoFilter::StupidError;
        }
    }
*/
    m_text = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    m_text += "<!DOCTYPE kontour>\n";
    m_text += "<kontour mime=\"application/x-kontour\" version=\"1\" editor=\"MSOD import filter\">\n";
    m_text += " <head cpn=\"2\">\n";
    m_text += "  <grid dx=\"20\" dy=\"20\" color=\"#c0c0c0\" align=\"0\" />\n";
    m_text += "  <helplines show=\"1\" align=\"0\"/>\n";
    m_text += "  <stylelist>\n";
    m_text += "   <style oopacity=\"100\" width=\"1\" ocolor=\"#000000\" pattern=\"1\" join=\"128\" ftype=\"0\" id=\"default\" cap=\"32\" stroked=\"1\" fcolor=\"#ffffff\" />\n";
    m_text += "  </stylelist>\n";
    m_text += " </head>\n";
    m_text += " <page id=\"Page 1\">\n";
    m_text += "  <layout width=\"210\" lmargin=\"0\" format=\"a4\" bmargin=\"0\" height=\"297\" rmargin=\"0\" tmargin=\"0\" orientation=\"portrait\"/>\n";
    m_text += "  <layer>\n";

    if (!parse(shapeId, m_chain->inputFile(), delayStream))
        return KoFilter::WrongFormat;
    m_text += "  </layer>\n";
    m_text += " </page>\n";
    m_text += "</kontour>\n";

    emit sigProgress(100);

    KoStoreDevice* dev = m_chain->storageFile( "root", KoStore::Write );
    if (!dev)
    {
        kdError(s_area) << "Cannot open output file" << endl;
        return KoFilter::StorageCreationError;
    }
    QCString cstring = m_text.utf8();
    dev->writeBlock(cstring.data(), cstring.size()-1);

    return KoFilter::OK;
}

void MSODImport::gotEllipse(
    const DrawContext &dc,
    QString type,
    QPoint topLeft,
    QSize halfAxes,
    unsigned startAngle,
    unsigned stopAngle)
{
    m_text += "<ellipse angle1=\"" + QString::number(startAngle) +
                "\" angle2=\"" + QString::number(stopAngle) +
                "\" x=\"" + QString::number(topLeft.x()) +
                "\" y=\"" + QString::number(topLeft.y()) +
                "\" kind=\"" + type +
                "\" rx=\"" + QString::number(halfAxes.width()) +
                "\" ry=\"" + QString::number(halfAxes.height()) +
                "\">\n";
    m_text += " <gobject fillcolor=\"#" + QString::number(dc.m_brushColour, 16) +
                "\" fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</ellipse>\n";
}

void MSODImport::gotPicture(
    unsigned key,
    QString extension,
    unsigned length,
    const char *data)
{
    kdDebug() << "##########################################MSODImport::gotPicture" << endl;
    kdDebug() << "MSODImport::gotPicture -- " << extension << endl;
    if ((extension == "wmf") ||
        (extension == "emf") ||
        (extension == "pict"))
    {
        int partRef = internalPartReference( QString::number( key ) );

        if (partRef == -1)
        {
            m_embeddeeData = data;
            m_embeddeeLength = length;

            QString srcMime( KoEmbeddingFilter::mimeTypeByExtension( extension ) );
            if ( srcMime == KMimeType::defaultMimeType() )
                kdWarning( s_area ) << "Couldn't determine the mimetype from the extension" << endl;

            QCString destMime; // intentionally empty, the filter manager will do the rest
            KoFilter::ConversionStatus status;
            partRef = embedPart( srcMime.latin1(), destMime, status, QString::number( key ) );

            m_embeddeeData = 0;
            m_embeddeeLength = 0;

            if ( status != KoFilter::OK ) {
                kdWarning(s_area) << "Couldn't convert the image!" << endl;
                return;
            }
        }
        m_text += "<object url=\"" + QString::number( partRef ) + "\" mime=\"";
        m_text += internalPartMimeType( QString::number( key ) );
        m_text += "\" x=\"0\" y=\"0\" width=\"100\" height=\"200\"/>\n";
    }
    else
    {
        // We could not import it as a part. Try as an image.
        KTempFile tempFile( QString::null, '.' + extension );
        tempFile.file()->writeBlock( data, length );
        tempFile.close();

        m_text += "<pixmap src=\"" + tempFile.name() + "\">\n"
                    " <gobject fillstyle=\"0\" linewidth=\"1\" strokecolor=\"#000000\" strokestyle=\"1\">\n"
                    "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n"
                    " </gobject>\n"
                    "</pixmap>\n";

        // Note that we cannot delete the file...
    }
}

void MSODImport::gotPolygon(
    const DrawContext &dc,
    const QPointArray &points)
{
    QRect bounds = points.boundingRect();

    m_text += "<polygon width=\"" + QString::number(bounds.width()) +
                "\" x=\"" + QString::number(bounds.x()) +
                "\" y=\"" + QString::number(bounds.y()) +
                "\" height=\"" + QString::number(bounds.height()) +
                "\" rounding=\"0\">\n";
    m_text += "<polyline arrow1=\"0\" arrow2=\"0\">\n";
    pointArray(points);
    m_text += " <gobject fillcolor=\"#" + QString::number(dc.m_brushColour, 16) +
                "\" fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</polyline>\n";
    m_text += "</polygon>\n";
}


void MSODImport::gotPolyline(
    const DrawContext &dc,
    const QPointArray &points)
{
    m_text += "<polyline arrow1=\"0\" arrow2=\"0\">\n";
    pointArray(points);
    m_text += " <gobject fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</polyline>\n";
}

void MSODImport::gotRectangle(
    const DrawContext &dc,
    const QPointArray &points)
{
    QRect bounds = points.boundingRect();

    m_text += "<rectangle width=\"" + QString::number(bounds.width()) +
                "\" x=\"" + QString::number(bounds.x()) +
                "\" y=\"" + QString::number(bounds.y()) +
                "\" height=\"" + QString::number(bounds.height()) +
                "\" rounding=\"0\">\n";
    m_text += "<polyline arrow1=\"0\" arrow2=\"0\">\n";
    pointArray(points);
    m_text += " <gobject fillcolor=\"#" + QString::number(dc.m_brushColour, 16) +
                "\" fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</polyline>\n";
    m_text += "</rectangle>\n";
}

void MSODImport::savePartContents( QIODevice* file )
{
    if ( m_embeddeeData != 0 && m_embeddeeLength != 0 )
        file->writeBlock( m_embeddeeData, m_embeddeeLength );
}

void MSODImport::pointArray(
    const QPointArray &points)
{

    for (unsigned i = 0; i < points.count(); i++)
    {
        m_text += "<point x=\"" + QString::number(points.point(i).x()) +
                    "\" y=\"" + QString::number(points.point(i).y()) +
                     "\"/>\n";
    }
}

#include <msodimport.moc>
