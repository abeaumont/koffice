/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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

#ifndef KO_RDF_SEMANTIC_TREE_H
#define KO_RDF_SEMANTIC_TREE_H

#include "komain_export.h"
#include <QtCore/QSharedPointer>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include "KoRdfFoaF.h"			//krazy:exclude=includes
#include "KoRdfCalendarEvent.h"		//krazy:exclude=includes
#include "KoRdfLocation.h"		//krazy:exclude=includes

class KoRdfSemanticTreePrivate;

/**
 * @short Manage a QTreeWidget showing a collection of Semantic Items
 * @author Ben Martin <ben.martin@kogmbh.com>
 *
 * Because a tree viewing people, places, events etc "Semantic Objects"
 * is created and updated in multiple places, it makes sense to collect
 * the code handling it into a central location for easy global updates.
 */
class KOMAIN_EXPORT KoRdfSemanticTree
{
public:
    KoRdfSemanticTree();
    explicit KoRdfSemanticTree(QTreeWidget *tree);
    KoRdfSemanticTree(const KoRdfSemanticTree &orig);
    KoRdfSemanticTree &operator=(const KoRdfSemanticTree &other);
    ~KoRdfSemanticTree();

    /**
     * Setup the TreeWidget with a default sorting order etc and
     * initialize the people, events and locations base tree Items.
     */
    static KoRdfSemanticTree createTree(QTreeWidget *v);

    /**
     * Update the Items shown in the tree to reflect the Rdf in the
     * given model. if the model is not passed in then any Rdf in the
     * KoDocumentRdf is used to populate the tree.
     */
    void update(KoDocumentRdf *rdf, Soprano::Model *model = 0);

private:
    QExplicitlySharedDataPointer<KoRdfSemanticTreePrivate> d;
};

#endif
