/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KC_CELL_STORAGE_P
#define KC_CELL_STORAGE_P

// KCells
#include "KCBinding.h"
#include "KCCondition.h"
#include "KCFormula.h"
#include "KCStyle.h"
#include "KCValidity.h"
#include "KCValue.h"

class KCCellStorageUndoData
{
public:
    bool isEmpty() const {
        if (!bindings.isEmpty())
            return false;
        if (!comments.isEmpty())
            return false;
        if (!conditions.isEmpty())
            return false;
        if (!databases.isEmpty())
            return false;
        if (!formulas.isEmpty())
            return false;
        if (!fusions.isEmpty())
            return false;
        if (!links.isEmpty())
            return false;
        if (!matrices.isEmpty())
            return false;
        if (!namedAreas.isEmpty())
            return false;
        if (!styles.isEmpty())
            return false;
        if (!userInputs.isEmpty())
            return false;
        if (!validities.isEmpty())
            return false;
        if (!values.isEmpty())
            return false;
        if (!richTexts.isEmpty())
            return false;
        return true;
    }

    QList< QPair<QRectF, KCBinding> >          bindings;
    QList< QPair<QRectF, QString> >          comments;
    QList< QPair<QRectF, KCConditions> >       conditions;
    QList< QPair<QRectF, Database> >         databases;
    QVector< QPair<QPoint, KCFormula> >        formulas;
    QList< QPair<QRectF, bool> >             fusions;
    QVector< QPair<QPoint, QString> >        links;
    QList< QPair<QRectF, bool> >             matrices;
    QList< QPair<QRectF, QString> >          namedAreas;
    QList< QPair<QRectF, KCSharedSubStyle> >   styles;
    QVector< QPair<QPoint, QString> >        userInputs;
    QList< QPair<QRectF, KCValidity> >         validities;
    QVector< QPair<QPoint, KCValue> >          values;
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts;
};

#endif // KC_CELL_STORAGE_P
