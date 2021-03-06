/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#ifndef KO_RDF_CALENDAR_EVENT_TREE_WIDGET_ITEM_H
#define KO_RDF_CALENDAR_EVENT_TREE_WIDGET_ITEM_H

#include "RdfForward.h"
#include "KoRdfCalendarEvent.h"
#include "KoRdfSemanticTreeWidgetItem.h"
#include "KoRdfFoaFTreeWidgetItem.h"

class KCanvasBase;

/**
 * @short Display ical/vevent semantic data with a context menu tailored
 *        to such infomartion.
 * @author Ben Martin <ben.martin@kogmbh.com>
 */
class KoRdfCalendarEventTreeWidgetItem : public KoRdfSemanticTreeWidgetItem
{
    Q_OBJECT

public:
    enum {
        Type = KoRdfFoaFTreeWidgetItem::Type + 1
    };
    KoRdfCalendarEventTreeWidgetItem(QTreeWidgetItem *parent, KoRdfCalendarEvent *ev);

    // inherited and reimplemented...

    KoRdfCalendarEvent *semanticObject() const;
    virtual QList<KAction *> actions(QWidget *parent, KCanvasBase *host = 0);
    virtual void insert(KCanvasBase *host);

public slots:
    void saveToKCal();
    void exportToFile();

private:
    KoRdfCalendarEvent *m_semanticObject;

protected:
    virtual KoRdfSemanticItem *semanticItem() const;
    virtual QString uIObjectName() const;
};

#endif
