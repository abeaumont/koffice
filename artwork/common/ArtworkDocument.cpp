/* This file is part of the KDE project
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002-2007 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2005 David Faure <faure@kde.org>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 2004 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Gabor Lehel <illissius@gmail.com>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "ArtworkDocument.h"

#include <KOdfStore.h>
#include <KOdfPageLayoutData.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KOdfLoadingContext.h>
#include <KOdfStylesReader.h>
#include <KShapeSavingContext.h>
#include <KResourceManager.h>
#include <KShapeLoadingContext.h>
#include <KShapeLayer.h>
#include <KShapeRegistry.h>
#include <KOdfStorageDevice.h>
#include <KOdfWriteStore.h>
#include <KOdfEmbeddedDocumentSaver.h>
#include <KImageCollection.h>
#include <KDataCenterBase.h>
#include <KOdfText.h>
#include <KStyleManager.h>
#include <KTextSharedLoadingData.h>
#include <KInlineTextObjectManager.h>
#include <KVariableManager.h>

#include <ktemporaryfile.h>
#include <kdebug.h>

#include <qdom.h>
#include <QRectF>

class ArtworkDocument::Private
{
public:
    Private()
            : pageSize(0.0, 0.0)
            , hasExternalDataCenterMap(false),
            resourceManager(0)
    {
    }

    ~Private()
    {
        qDeleteAll(layers);
        layers.clear();
        qDeleteAll(objects);
        objects.clear();
        if (!hasExternalDataCenterMap)
            qDeleteAll(dataCenterMap);
    }

    QSizeF pageSize; ///< the documents page size

    QList<KShape*> objects;     ///< The list of all object of the document.
    QList<KShapeLayer*> layers; ///< The layers in this document.

    QMap<QString, KDataCenterBase*> dataCenterMap;
    bool hasExternalDataCenterMap;
    KResourceManager *resourceManager;
};

ArtworkDocument::ArtworkDocument()
        : d(new Private)
{
    // create a layer. we need at least one:
    insertLayer(new KShapeLayer());
}

ArtworkDocument::ArtworkDocument(const ArtworkDocument& document)
        : d(new Private)
{
    d->layers = document.layers();
// TODO
}

ArtworkDocument::~ArtworkDocument()
{
    delete d;
}

void ArtworkDocument::insertLayer(KShapeLayer* layer)
{
    if (!d->layers.contains(layer)) {
        if (d->layers.count()) {
            layer->setZIndex(d->layers.last()->zIndex() + 1);
        } else {
            layer->setZIndex(d->layers.count());
        }
        d->layers.append(layer);
    }
}

void ArtworkDocument::removeLayer(KShapeLayer* layer)
{
    d->layers.removeAt(d->layers.indexOf(layer));
    if (d->layers.count() == 0)
        d->layers.append(new KShapeLayer());
}

bool ArtworkDocument::canRaiseLayer(KShapeLayer* layer)
{
    int pos = d->layers.indexOf(layer);
    return (pos != int(d->layers.count()) - 1 && pos >= 0);
}

bool ArtworkDocument::canLowerLayer(KShapeLayer* layer)
{
    int pos = d->layers.indexOf(layer);
    return (pos > 0);
}

void ArtworkDocument::raiseLayer(KShapeLayer* layer)
{
    int pos = d->layers.indexOf(layer);
    if (pos != int(d->layers.count()) - 1 && pos >= 0) {
        KShapeLayer * layerAbove = d->layers.at(pos + 1);
        int lowerZIndex = layer->zIndex();
        int upperZIndex = layerAbove->zIndex();
        layer->setZIndex(upperZIndex);
        layerAbove->setZIndex(lowerZIndex);
        d->layers.move(pos, pos + 1);
    }
}

void ArtworkDocument::lowerLayer(KShapeLayer* layer)
{
    int pos = d->layers.indexOf(layer);
    if (pos > 0) {
        KShapeLayer * layerBelow = d->layers.at(pos - 1);
        int upperZIndex = layer->zIndex();
        int lowerZIndex = layerBelow->zIndex();
        layer->setZIndex(lowerZIndex);
        layerBelow->setZIndex(upperZIndex);
        d->layers.move(pos, pos - 1);
    }
}

int ArtworkDocument::layerPos(KShapeLayer* layer)
{
    return d->layers.indexOf(layer);
}

void ArtworkDocument::add(KShape* shape)
{
    if (! d->objects.contains(shape))
        d->objects.append(shape);
}

void ArtworkDocument::remove(KShape* shape)
{
    d->objects.removeAt(d->objects.indexOf(shape));
}

void ArtworkDocument::saveOasis(KShapeSavingContext &context) const
{
    context.xmlWriter().startElement("draw:page");
    context.xmlWriter().addAttribute("draw:name", "");
    context.xmlWriter().addAttribute("draw:id", "page1");
    context.xmlWriter().addAttribute("draw:master-page-name", "Default");

    foreach(KShapeLayer *layer, d->layers) {
        context.addLayerForSaving(layer);
    }
    context.saveLayerSet(context.xmlWriter());

    foreach(KShapeLayer *layer, d->layers) {
        layer->saveOdf(context);
        context.writeConnectors();
    }

    context.xmlWriter().endElement(); // draw:page
}

bool ArtworkDocument::loadOasis(const KXmlElement &element, KShapeLoadingContext &context)
{
    // load text styles used by text shapes
    loadOdfStyles(context);

    qDeleteAll(d->layers);
    d->layers.clear();
    qDeleteAll(d->objects);
    d->objects.clear();

    const KXmlElement & pageLayerSet = KoXml::namedItemNS(element, KOdfXmlNS::draw, "layer-set");
    const KXmlElement & usedPageLayerSet = pageLayerSet.isNull() ? context.odfLoadingContext().stylesReader().layerSet() : pageLayerSet;

    KXmlElement layerElement;
    forEachElement(layerElement, usedPageLayerSet) {
        KShapeLayer * l = new KShapeLayer();
        if (l->loadOdf(layerElement, context))
            insertLayer(l);
    }

    KShapeLayer * defaultLayer = 0;

    // check if we have to insert a default layer
    if (d->layers.count() == 0)
        defaultLayer = new KShapeLayer();

    KXmlElement child;
    forEachElement(child, element) {
        kDebug(38000) << "loading shape" << child.localName();

        KShape * shape = KShapeRegistry::instance()->createShapeFromOdf(child, context);
        if (shape)
            d->objects.append(shape);
    }

    // add all toplevel shapes to the default layer
    foreach(KShape * shape, d->objects) {
        if (! shape->parent()) {
            if (! defaultLayer)
                defaultLayer = new KShapeLayer();

            defaultLayer->addShape(shape);
        }
    }

    if (defaultLayer)
        insertLayer(defaultLayer);

    KOdfStylesReader & styleReader = context.odfLoadingContext().stylesReader();
    QHash<QString, KXmlElement*> masterPages = styleReader.masterPages();

    KXmlElement * master = 0;
    if( masterPages.contains( "Standard" ) )
        master = masterPages.value( "Standard" );
    else if( masterPages.contains( "Default" ) )
        master = masterPages.value( "Default" );
    else if( ! masterPages.empty() )
        master = masterPages.begin().value();

    if (master) {
        context.odfLoadingContext().setUseStylesAutoStyles( true );

        QList<KShape*> masterPageShapes;
        KXmlElement child;
        forEachElement(child, (*master)) {
            kDebug(38000) <<"loading master page shape" << child.localName();
            KShape * shape = KShapeRegistry::instance()->createShapeFromOdf( child, context );
            if( shape )
                masterPageShapes.append( shape );
        }

        KShapeLayer * masterPageLayer = 0;
        // add all toplevel shapes to the master page layer
        foreach(KShape * shape, masterPageShapes) {
            d->objects.append( shape );
            if(!shape->parent()) {
                if( ! masterPageLayer ) {
                    masterPageLayer = new KShapeLayer();
                    masterPageLayer->setName(i18n("Master Page"));
                }

                masterPageLayer->addShape(shape);
            }
        }

        if( masterPageLayer )
            insertLayer( masterPageLayer );

        context.odfLoadingContext().setUseStylesAutoStyles( false );
    }

    return true;
}

QRectF ArtworkDocument::boundingRect() const
{
    return contentRect().united(QRectF(QPointF(0, 0), d->pageSize));
}

QRectF ArtworkDocument::contentRect() const
{
    QRectF bb;
    foreach(KShape* layer, d->layers) {
        if (bb.isNull())
            bb = layer->boundingRect();
        else
            bb = bb.united(layer->boundingRect());
    }

    return bb;
}

QSizeF ArtworkDocument::pageSize() const
{
    return d->pageSize;
}

void ArtworkDocument::setPageSize(QSizeF pageSize)
{
    d->pageSize = pageSize;
}

const QList<KShape*> ArtworkDocument::shapes() const
{
    return d->objects;
}

const QList<KShapeLayer*> ArtworkDocument::layers() const
{
    return d->layers;
}

KImageCollection * ArtworkDocument::imageCollection()
{
    return resourceManager()->imageCollection();
}

QMap<QString, KDataCenterBase*> ArtworkDocument::dataCenterMap() const
{
    return d->dataCenterMap;
}

void ArtworkDocument::useExternalDataCenterMap(QMap<QString, KDataCenterBase*> dataCenters)
{
    qDeleteAll(d->dataCenterMap);
    d->dataCenterMap = dataCenters;
    d->hasExternalDataCenterMap = true;
}

//#############################################################################
// ODF saving
//#############################################################################

bool ArtworkDocument::saveOdf(KoDocument::SavingContext &documentContext, const KOdfPageLayoutData &layout)
{
    KOdfStore * store = documentContext.odfStore.store();
    KXmlWriter* contentWriter = documentContext.odfStore.contentWriter();
    if (!contentWriter)
        return false;

    KOdfGenericStyles mainStyles;
    KXmlWriter * bodyWriter = documentContext.odfStore.bodyWriter();

    KShapeSavingContext shapeContext(*bodyWriter, mainStyles, documentContext.embeddedSaver);

    // save text styles
    saveOdfStyles(shapeContext);

    // save page
    QString layoutName = mainStyles.insert(layout.saveOdf(), "PL");
    KOdfGenericStyle masterPage(KOdfGenericStyle::MasterPageStyle);
    masterPage.addAttribute("style:page-layout-name", layoutName);
    mainStyles.insert(masterPage, "Default", KOdfGenericStyles::DontAddNumberToName);

    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:drawing");

    if (resourceManager()->hasResource(KOdfText::InlineTextObjectManager)) {
        KInlineTextObjectManager* object_manager = (resourceManager()->resource(KOdfText::InlineTextObjectManager)).value<KInlineTextObjectManager*>();
        if (object_manager) {
            KVariableManager* variable_manager = object_manager->variableManager();
            if (variable_manager) {
                variable_manager->saveOdf(bodyWriter);
            }
        }
    }

    saveOasis(shapeContext);   // Save contents

    bodyWriter->endElement(); // office:drawing
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);

    documentContext.odfStore.closeContentWriter();

    //add manifest line for content.xml
    documentContext.odfStore.manifestWriter()->addManifestEntry("content.xml", "text/xml");

    if (!shapeContext.saveDataCenter(store, documentContext.odfStore.manifestWriter())) {
        return false;
    }

    return mainStyles.saveOdfStylesDotXml(store, documentContext.odfStore.manifestWriter());
}

void ArtworkDocument::loadOdfStyles(KShapeLoadingContext & context)
{
    KStyleManager *styleManager = resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();

    if (! styleManager)
        return;

    KTextSharedLoadingData * sharedData = new KTextSharedLoadingData();
    if (! sharedData)
        return;

    sharedData->loadOdfStyles(context, styleManager);
    context.addSharedData(KODFTEXT_SHARED_LOADING_ID, sharedData);
}

void ArtworkDocument::saveOdfStyles(KShapeSavingContext & context)
{
    KStyleManager * styleManager = dynamic_cast<KStyleManager*>(dataCenterMap()["StyleManager"]);
    if (! styleManager)
        return;

    styleManager->saveOdf(context.mainStyles());
}

void ArtworkDocument::addToDataCenterMap(const QString &key, KDataCenterBase* dataCenter)
{
    d->dataCenterMap.insert(key, dataCenter);
}

KResourceManager *ArtworkDocument::resourceManager() const
{
    return d->resourceManager;
}

void ArtworkDocument::setResourceManager(KResourceManager *rm)
{
    d->resourceManager = rm;
}
