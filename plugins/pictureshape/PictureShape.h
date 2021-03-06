/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PICTURESHAPE_H
#define PICTURESHAPE_H

#include <QPixmap>
#include <KShape.h>
#include <KFrameShape.h>

#define PICTURESHAPEID "PictureShape"

class KImageCollection;
class RenderQueue;

class PictureShape : public KShape, public KFrameShape
{
public:
    enum PictureMode {
        Standard,
        Greyscale,
        Mono,
        Watermark
    };

    PictureShape();
    virtual ~PictureShape();

    // reimplemented
    virtual void paint(QPainter &painter, const KViewConverter &converter);
    // reimplemented
    virtual void saveOdf(KShapeSavingContext &context) const;
    // reimplemented
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    // reimplemented
    virtual void waitUntilReady(const KViewConverter &converter, bool asynchronous) const;

    /**
     * Get the collection used in the shape.
     */
    KImageCollection *imageCollection() const;

    void setImageCollection(KImageCollection *collection) { m_imageCollection = collection; }

    void setMode(PictureMode mode);
    PictureMode mode() const;

protected:
    virtual bool loadOdfFrameElement(const KXmlElement &element, KShapeLoadingContext &context);

    virtual QString saveStyle(KOdfGenericStyle &style, KShapeSavingContext &context) const;

    virtual void loadStyle(const KXmlElement& element, KShapeLoadingContext& context);

private:
    KImageCollection *m_imageCollection;
    RenderQueue *m_renderQueue;
    mutable QImage m_printQualityImage;
    PictureMode m_mode;
};

class RenderQueue : public QObject
{
    Q_OBJECT
public:
    RenderQueue(PictureShape *shape) : m_pictureShape(shape) { }

    void addSize(const QSize &size) { m_wantedImageSize << size; }

public slots:
    void renderImage();
    void updateShape();

private:
    KShape *m_pictureShape;
    QList<QSize> m_wantedImageSize;
};

#endif
