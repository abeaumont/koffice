/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#include "KShapeLoadingContext.h"
#include "KShape.h"
#include "KShapeContainer.h"
#include "KSharedLoadingData.h"
#include "KShapeControllerBase.h"
#include "KImageCollection.h"
#include "KResourceManager.h"
#include "KLoadingShapeUpdater.h"

#include <kdebug.h>

uint qHash(const KShapeLoadingContext::AdditionalAttributeData & attributeData)
{
    return qHash(attributeData.name);
}

static QSet<KShapeLoadingContext::AdditionalAttributeData> s_additionlAttributes;

class KShapeLoadingContext::Private
{
public:
    Private(KOdfLoadingContext &c, KResourceManager *resourceManager)
            : context(c),
            zIndex(0),
            documentResources(resourceManager)
    {
    }
    ~Private() {
        foreach(KSharedLoadingData * data, sharedData) {
            delete data;
        }
    }
    KOdfLoadingContext &context;
    QMap<QString, KShapeLayer*> layers;
    QMap<QString, KShape*> drawIds;
    QMap<QString, QPair<KShape *, QVariant> > subIds;
    QMap<QString, KSharedLoadingData*> sharedData;
    int zIndex;
    QMap<QString, KLoadingShapeUpdater*> updaterById;
    QMap<KShape *, KLoadingShapeUpdater*> updaterByShape;
    KResourceManager *documentResources;
};

KShapeLoadingContext::KShapeLoadingContext(KOdfLoadingContext & context, KResourceManager *documentResources)
        : d(new Private(context, documentResources))
{
}

KShapeLoadingContext::~KShapeLoadingContext()
{
    delete d;
}

KOdfLoadingContext & KShapeLoadingContext::odfLoadingContext()
{
    return d->context;
}

KShapeLayer * KShapeLoadingContext::layer(const QString & layerName)
{
    return d->layers.value(layerName, 0);
}

void KShapeLoadingContext::addLayer(KShapeLayer * layer, const QString & layerName)
{
    d->layers[ layerName ] = layer;
}

void KShapeLoadingContext::clearLayers()
{
    d->layers.clear();
}

void KShapeLoadingContext::addShapeId(KShape * shape, const QString & id)
{
    d->drawIds.insert(id, shape);
    QMap<QString, KLoadingShapeUpdater*>::iterator it(d->updaterById.find(id));
    while (it != d->updaterById.end() && it.key() == id) {
        d->updaterByShape.insertMulti(shape, it.value());
        it = d->updaterById.erase(it);
    }
}

KShape * KShapeLoadingContext::shapeById(const QString &id)
{
    return d->drawIds.value(id, 0);
}

void KShapeLoadingContext::addShapeSubItemId(KShape *shape, const QVariant &subItem, const QString &id)
{
    d->subIds.insert(id, QPair<KShape *, QVariant>(shape, subItem));
}

QPair<KShape *, QVariant> KShapeLoadingContext::shapeSubItemById(const QString &id)
{
    return d->subIds.value(id);
}


// TODO make sure to remove the shape from the loading context when loading for it failed and it was deleted. This can also happen when the parent is deleted
void KShapeLoadingContext::updateShape(const QString & id, KLoadingShapeUpdater * shapeUpdater)
{
    d->updaterById.insertMulti(id, shapeUpdater);
}

void KShapeLoadingContext::shapeLoaded(KShape * shape)
{
    QMap<KShape*, KLoadingShapeUpdater*>::iterator it(d->updaterByShape.find(shape));
    while (it != d->updaterByShape.end() && it.key() == shape) {
        it.value()->update(shape);
        delete it.value();
        it = d->updaterByShape.erase(it);
    }
}

KImageCollection * KShapeLoadingContext::imageCollection()
{
    return d->documentResources ? d->documentResources->imageCollection() : 0;
}

int KShapeLoadingContext::zIndex()
{
    return d->zIndex++;
}

void KShapeLoadingContext::setZIndex(int index)
{
    d->zIndex = index;
}

void KShapeLoadingContext::addSharedData(const QString & id, KSharedLoadingData * data)
{
    QMap<QString, KSharedLoadingData*>::iterator it(d->sharedData.find(id));
    // data will not be overwritten
    if (it == d->sharedData.end()) {
        d->sharedData.insert(id, data);
    } else {
        kWarning(30006) << "The id" << id << "is already registered. Data not inserted";
        Q_ASSERT(it == d->sharedData.end());
    }
}

KSharedLoadingData * KShapeLoadingContext::sharedData(const QString & id) const
{
    KSharedLoadingData * data = 0;
    QMap<QString, KSharedLoadingData*>::const_iterator it(d->sharedData.find(id));
    if (it != d->sharedData.constEnd()) {
        data = it.value();
    }
    return data;
}

void KShapeLoadingContext::addAdditionalAttributeData(const AdditionalAttributeData & attributeData)
{
    s_additionlAttributes.insert(attributeData);
}

QSet<KShapeLoadingContext::AdditionalAttributeData> KShapeLoadingContext::additionalAttributeData()
{
    return s_additionlAttributes;
}

KResourceManager *KShapeLoadingContext::documentResourceManager() const
{
    return d->documentResources;
}
