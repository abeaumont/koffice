/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_TABLE_SHAPE_FACTORY
#define KSPREAD_TABLE_SHAPE_FACTORY

#include <QStringList>

#include <KoShapeFactory.h>

class KoShape;
class KoXmlElement;

class TableShapePlugin : public QObject
{
    Q_OBJECT
public:

    TableShapePlugin( QObject * parent,  const QStringList & );
    ~TableShapePlugin() {}
};

class TableShapeFactory : public KoShapeFactory
{
    Q_OBJECT
public:
    TableShapeFactory( QObject* parent );
    ~TableShapeFactory() {}

    virtual KoShape* createDefaultShape(KoShapeControllerBase *shapeController);
    virtual KoShape* createShape(const KoProperties* params, KoShapeControllerBase *shapeController);

    virtual bool supports(const KoXmlElement &element) const;

protected:
    virtual KoShape* createDefaultShape() const;
    virtual KoShape* createShape(const KoProperties* params) const;
};

#endif // KSPREAD_TABLE_SHAPE_FACTORY
