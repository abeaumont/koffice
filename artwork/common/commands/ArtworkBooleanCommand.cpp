/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "ArtworkBooleanCommand.h"
#include <KShapeControllerBase.h>
#include <KPathShape.h>
#include <KShapeContainer.h>
#include <KShapeGroup.h>
#include <KShapeGroupCommand.h>

#include <klocale.h>

#include <QtGui/QPainterPath>

class ArtworkBooleanCommand::Private
{
public:
    Private(KShapeControllerBase * c)
            : controller(c), pathA(0), pathB(0), resultingPath(0)
            , resultParent(0), resultParentCmd(0)
            , operation(Intersection), isExecuted(false)
    {}

    ~Private()
    {
        if (! isExecuted)
            delete resultingPath;
    }

    KShapeControllerBase *controller;
    KPathShape * pathA;
    KPathShape * pathB;
    KPathShape * resultingPath;
    KShapeContainer * resultParent;
    QUndoCommand * resultParentCmd;
    BooleanOperation operation;
    bool isExecuted;
};

ArtworkBooleanCommand::ArtworkBooleanCommand(
    KShapeControllerBase *controller, KPathShape* pathA, KPathShape * pathB,
    BooleanOperation operation, QUndoCommand *parent
)
        : QUndoCommand(parent), d(new Private(controller))
{
    Q_ASSERT(controller);

    d->pathA = pathA;
    d->pathB = pathB;
    d->operation = operation;

    setText(i18n("Boolean Operation"));
}

ArtworkBooleanCommand::~ArtworkBooleanCommand()
{
    delete d;
}

void ArtworkBooleanCommand::redo()
{
    if (! d->resultingPath) {
        QPainterPath pa = d->pathA->absoluteTransformation(0).map(d->pathA->outline());
        QPainterPath pb = d->pathB->absoluteTransformation(0).map(d->pathB->outline());
        QPainterPath pr;
        switch (d->operation) {
        case Intersection:
            pr = pa.intersected(pb);
            break;
        case Subtraction:
            pr = pa.subtracted(pb);
            break;
        case Exclusion:
            pr = pa.subtracted(pb);
            pr.addPath(pb.subtracted(pa));
            break;
        case Union:
            pr = pa.united(pb);
            break;
        }

        QTransform transformation = d->pathA->transformation();
        pr = transformation.inverted().map(pr);
        d->resultingPath = KPathShape::createShapeFromPainterPath(pr);
        d->resultingPath->setBorder(d->pathA->border());
        d->resultingPath->setBackground(d->pathA->background());
        d->resultingPath->setShapeId(d->pathA->shapeId());
        d->resultingPath->setTransformation(transformation);
        d->resultingPath->setName(d->pathA->name());
        d->resultingPath->setZIndex(d->pathA->zIndex());
        d->resultingPath->setFillRule(d->pathA->fillRule());

        KShapeGroup * group = dynamic_cast<KShapeGroup*>(d->pathA->parent());
        if (group) {
            QList<KShape*> children;
            d->resultParentCmd = new KShapeGroupCommand(group, children << d->resultingPath, this);
        }
    }

    if (d->controller) {
        if (d->resultParent)
            d->resultParent->addShape(d->resultingPath);
        d->controller->addShape(d->resultingPath);
    }

    QUndoCommand::redo();

    d->isExecuted = true;
}

void ArtworkBooleanCommand::undo()
{
    QUndoCommand::undo();

    if (d->controller && d->resultingPath) {
        if (! d->resultParentCmd) {
            d->resultParent = d->resultingPath->parent();
            if (d->resultParent)
                d->resultParent->removeShape(d->resultingPath);
        }
        d->controller->removeShape(d->resultingPath);
    }

    d->isExecuted = false;
}
