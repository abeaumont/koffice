/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoImageData.h"

#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kdebug.h>

#include <QBuffer>
#include <QTimer>
#include <QPainter>
#include <QImageWriter>
#include <QTemporaryFile>

#include "KoImageCollection.h"
#include "KoImageData_p.h"

/// the maximum amount of bytes the image can be while we store it in memory instead of
/// spooling it to disk in a temp-file.
#define MAX_MEMORY_IMAGESIZE 90000

KoImageData::KoImageData()
    : d(0)
{
}

KoImageData::KoImageData(const KoImageData &imageData)
    : KoShapeUserData(),
    d(imageData.d)
{
    if (d)
        d->refCount.ref();
}

KoImageData::KoImageData(KoImageDataPrivate *priv)
    : d(priv)
{
    d->refCount.ref();
}

KoImageData::~KoImageData()
{
    if (d && !d->refCount.deref())
        delete d;
}

QPixmap KoImageData::pixmap(const QSize &size)
{
    QSize wantedSize = size;
    if (! wantedSize.isValid()) {
        if (d->pixmap.isNull()) // we have a problem, Houston..
            wantedSize = QSize(100, 100);
        else
            wantedSize = d->pixmap.size();
    }
    if (d->pixmap.isNull() || d->pixmap.size() != wantedSize) {
        switch (d->dataStoreState) {
        case KoImageDataPrivate::StateEmpty: {
            d->pixmap = QPixmap(1, 1);
            QPainter p(&d->pixmap);
            p.setPen(QPen(Qt::gray));
            p.drawPoint(0, 0);
            p.end();
            break;
        }
        case KoImageDataPrivate::StateNotLoaded:
            image(); // forces load
            // fall through
        case KoImageDataPrivate::StateImageLoaded:
        case KoImageDataPrivate::StateImageOnly:
            if (! d->image.isNull()) {
                // create pixmap from image.
                d->pixmap = QPixmap(wantedSize.width(), wantedSize.height());
                QPainter p(&d->pixmap);
                p.drawImage(0, 0, d->image, 0, 0, wantedSize.width(), wantedSize.height());
                p.end();
            }
        }

        if (!d->cleanupTriggered && d->dataStoreState == KoImageDataPrivate::StateImageLoaded) {
            // schedule an auto-unload of the big QImage in a second.
            QTimer::singleShot(1000, this, SLOT(cleanupImageCache()));
            d->cleanupTriggered = true;
        }
    }
    return d->pixmap;
}

bool KoImageData::hasCachedPixmap() const
{
    return d && !d->pixmap.isNull();
}

QSizeF KoImageData::imageSize()
{
    if (!d->imageSize.isValid()) {
        // The imagesize have not yet been calculated
        image(); // make sure the image is loaded

        if (d->image.dotsPerMeterX())
            d->imageSize.setWidth(DM_TO_POINT(d->image.width() / (qreal) d->image.dotsPerMeterX() * 10.0));
        else
            d->imageSize.setWidth(d->image.width() / 72.0);

        if (d->image.dotsPerMeterY())
            d->imageSize.setHeight(DM_TO_POINT(d->image.height() / (qreal) d->image.dotsPerMeterY() * 10.0));
        else
            d->imageSize.setHeight(d->image.height() / 72.0);
    }
    return d->imageSize;
}

QImage KoImageData::image() const
{
    if (d->dataStoreState == KoImageDataPrivate::StateNotLoaded) {
        // load image
        if (d->temporaryFile) {
            d->temporaryFile->open();
            if (d->errorCode == Success && !d->image.load(d->temporaryFile, 0))
                d->errorCode = OpenFailed;
            d->temporaryFile->close();
        } else {
            if (d->errorCode == Success && !d->image.load(d->imageLocation.toLocalFile()))
                d->errorCode = OpenFailed;
        }
        if (d->errorCode == Success)
            d->dataStoreState = KoImageDataPrivate::StateImageLoaded;
    }
    return d->image;
}

bool KoImageData::hasCachedImage() const
{
    return d && !d->image.isNull();
}

void KoImageData::setImage(const QImage &image, KoImageCollection *collection)
{
    Q_ASSERT(!image.isNull());
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KoImageData *other = collection->getImage(image);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KoImageDataPrivate();
            d->refCount.ref();
        }
        delete d->temporaryFile;
        d->errorCode = Success;
        d->suffix = "png"; // good default for non-lossy storage.
        if (image.numBytes() > MAX_MEMORY_IMAGESIZE) {
            d->image = QImage();
            // store image
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            QImageWriter writer(&buffer, d->suffix.toLatin1());
            if (!writer.write(image)) {
                // TODO kWarning
                d->errorCode = StorageFailed;
                delete d->temporaryFile;
                return;
            }
            buffer.close();
            d->copyToTemporary(buffer);
            return;
        } else {
            d->image = image;
        }
        d->imageLocation.clear();
        d->key = QString::number(image.cacheKey()).toLatin1();
        d->dataStoreState = KoImageDataPrivate::StateImageOnly;
    }
}

void KoImageData::setExternalImage(const QUrl &location, KoImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KoImageData *other = collection->getExternalImage(location);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KoImageDataPrivate();
            d->refCount.ref();
        }
        d->image = QImage();
        d->errorCode = Success;
        d->imageLocation = location;
        d->setSuffix(location.toEncoded());
        d->key = location.toEncoded();
        d->dataStoreState = KoImageDataPrivate::StateNotLoaded;
    }
}

void KoImageData::setImage(const QString &url, KoStore *store, KoImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KoImageData *other = collection->getImage(url, store);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KoImageDataPrivate();
            d->refCount.ref();
        } else {
            d->imageLocation.clear();
            d->key.clear();
            d->image = QImage();
            d->errorCode = Success;
        }
        d->setSuffix(url);

        if (store->open(url)) {
            KoStoreDevice device(store);
            if (device.size() > MAX_MEMORY_IMAGESIZE) {
                // TODO test also if the file is lossy, then we should store the bytes too
                if (d->image.load(&device, d->suffix.toLatin1())) {
                    d->dataStoreState = KoImageDataPrivate::StateImageOnly;
                    return;
                }
            }
            if (!device.open(QIODevice::ReadOnly)) {
                kWarning(30006) << "open file from store " << url << "failed";
                d->errorCode = OpenFailed;
                return;
            }
            d->copyToTemporary(device);
        } else {
            kWarning(30006) << "Find file in store " << url << "failed";
            d->errorCode = OpenFailed;
            return;
        }
    }
}

bool KoImageData::isValid() const
{
    return d;
}

bool KoImageData::operator==(const KoImageData &other) const
{
    return other.d == d;
}

KoImageData &KoImageData::operator=(const KoImageData &other)
{
    if (other.d)
        other.d->refCount.ref();
    if (d && !d->refCount.deref())
        delete d;
    d = other.d;
    return *this;
}

QByteArray KoImageData::key() const
{
    return d->key;
}

QString KoImageData::suffix() const
{
    return d->suffix;
}

KoImageData::ErrorCode KoImageData::errorCode() const
{
    return d->errorCode;
}

void KoImageData::cleanupImageCache()
{
    if (d->dataStoreState == KoImageDataPrivate::StateImageLoaded) {
        d->image = QImage();
        d->dataStoreState = KoImageDataPrivate::StateNotLoaded;
    }
    d->cleanupTriggered = false;
}
