/* This file is part of the KDE project
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002-2005,2007 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2005-2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2005,2007 David Faure <faure@kde.org>
   Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>

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

#ifndef ARTWORKDOCUMENT_H
#define ARTWORKDOCUMENT_H

#include <QString>
#include <QMap>
#include <QRectF>

#include <artworkcommon_export.h>
#include <KShapeLoadingContext.h>
#include <KOdfGenericStyle.h>
#include <KXmlReader.h>
#include <KoDocument.h>

class KShape;
class KShapeSavingContext;
class KShapeLayer;
class KImageCollection;
class KOdfStore;
class KDataCenterBase;
class KResourceManager;

/**
 * All non-visual, static doc info is in here.
 * The artwork part uses this class.
 * Filters can use this class as well instead of
 * the visually oriented artwork part.
 */
class ARTWORKCOMMON_EXPORT ArtworkDocument
{
public:

    /**
     * Constructs a new document.
     */
    ArtworkDocument();

    /**
     * Copy constructor.
     *
     * @param document the document to copy properties from
     */
    ArtworkDocument(const ArtworkDocument& document);

    /**
     * Destroys the document and all of the layers.
     */
    virtual ~ArtworkDocument();

    /**
     * Checks if specified layer can be raised.
     *
     * A layer can be raised if there is more than one layer and the specified layer
     * is not already at the top.
     *
     * @param layer the layer to check
     * @return true if layer can be raised, else false
     */
    bool canRaiseLayer(KShapeLayer* layer);

    /**
     * Checks if specified layer can be lowered.
     *
     * A layer can be lowered if there is more than one layer and the specified layer
     * is not already at the bottom.
     *
     * @param layer the layer to check
     * @return true if layer can be lowered, else false
     */
    bool canLowerLayer(KShapeLayer* layer);

    /**
     * Raises the layer.
     *
     * @param layer the layer to raise
     */
    void raiseLayer(KShapeLayer* layer);

    /**
     * Lowers the layer.
     *
     * @param layer the layer to lower
     */
    void lowerLayer(KShapeLayer* layer);

    /**
     * Returns the position of the specified layer.
     *
     * @param layer the layer to retrieve the position for
     * @return the layer position
     */
    int layerPos(KShapeLayer* layer);

    /**
     * Inserts a new layer.
     *
     * The layer is appended at the end, on top of all other layers, and is activated.
     *
     * @param layer the layer to insert
     */
    void insertLayer(KShapeLayer* layer);

    /**
     * Removes the layer.
     *
     * If there is no layer left, a new layer is created, inserted and activated.
     *
     * @param layer the layer to remove
     */
    void removeLayer(KShapeLayer* layer);

    /**
     * Returns the list of layers.
     * The layer list provides a hierarchical view/access of the document data.
     * All the documents shapes are children of a shape container, where a layer
     * resembles a root container which can contain other containers in an
     * arbitrary nesting depth.
     */
    const QList<KShapeLayer*> layers() const;

    /**
     * Returns the list of all shapes of the document.
     * This list provides a flat view/access to all the documents shapes.
     * For an hierarchical view/access one should retrieve the documents
     * layers with layers().
     */
    const QList<KShape*> shapes() const;

    void saveOasis(KShapeSavingContext & context) const;
    bool saveOdf(KoDocument::SavingContext & documentContext, const KOdfPageLayoutData &layout);
    bool loadOasis(const KXmlElement &element, KShapeLoadingContext &context);
    void loadOdfStyles(KShapeLoadingContext & context);
    void saveOdfStyles(KShapeSavingContext & context);

    /**
    * Adds an object to the document.
    *
    * @param shape the object to append
    */
    void add(KShape* shape);

    /**
    * Removes an object from the document.
    *
    * @param shape the object to append
    */
    void remove(KShape* shape);

    /**
     * Fetch the current resourceManager.
     * See KShapeController::resourceManager() for more details.
     */
    KResourceManager *resourceManager() const;
    /**
     * Set a new resource manager.
     */
    void setResourceManager(KResourceManager *rm);

    /// Returns the united bounding rectangle of the documents content and the document page
    QRectF boundingRect() const;

    /// Returns the bounding rectangle of the documents content
    QRectF contentRect() const;

    /// Returns the documents page size
    QSizeF pageSize() const;

    /// Sets the documents page size
    void setPageSize(QSizeF pageSize);

    /// Returns the documents image collection
    KImageCollection * imageCollection();

    /// Returns the documents data centers
    QMap<QString, KDataCenterBase*> dataCenterMap() const;

    /// Sets the data centers to be used by this document
    void useExternalDataCenterMap(QMap<QString, KDataCenterBase*> dataCenters);

    void addToDataCenterMap(const QString &key, KDataCenterBase* dataCenter);

private:

    class Private;
    Private * const d;
};

#endif // ARTWORKDOCUMENT_H

