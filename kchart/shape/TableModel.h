/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_TABLEMODEL_H
#define KCHART_TABLEMODEL_H

// Interface
#include <interfaces/KoChartModel.h>
#include <QStandardItemModel>

// Qt
#include <QVector>

// Local
#include "ChartShape.h"

class QString;

namespace KChart {

class TableModel : public QObject, public KoChart::ChartModel
{
    Q_OBJECT
    Q_INTERFACES(KoChart::ChartModel)
public:
    TableModel( QObject *parent = 0 );
    ~TableModel();

    // KoChart::ChartModel interface
    virtual QString regionToString( const QVector<QRect> &region ) const;
    virtual QVector<QRect> stringToRegion( const QString &string ) const;
    virtual QHash<QString, QVector<QRect> > cellRegion() const;
    virtual bool setCellRegion(const QString& regionName);
    virtual QStandardItemModel * model() { return m_model; }

    void loadOdf( const KoXmlElement &tableElement, const KoOdfStylesReader &stylesReader );
    bool saveOdf( KoXmlWriter &bodyWriter, KoGenStyles &mainStyles ) const;

private:
    QStandardItemModel *m_model;
};

}

#endif // KCHART_TABLEMODEL_H
