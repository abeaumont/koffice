/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KShapeShadowCommand.h"
#include "KShape.h"
#include "KShapeShadow.h"

#include <klocale.h>

class KShapeShadowCommand::Private
{
public:
    Private() {}
    ~Private() {
        foreach(KShapeShadow* shadow, oldShadows) {
            if (shadow && !shadow->deref())
                delete shadow;
        }
    }

    void addOldShadow(KShapeShadow * oldShadow)
    {
        if (oldShadow)
            oldShadow->ref();
        oldShadows.append(oldShadow);
    }

    void addNewShadow(KShapeShadow * newShadow)
    {
        if (newShadow)
            newShadow->ref();
        newShadows.append(newShadow);
    }

    QList<KShape*> shapes;           ///< the shapes to set shadow for
    QList<KShapeShadow*> oldShadows; ///< the old shadows, one for each shape
    QList<KShapeShadow*> newShadows; ///< the new shadows to set
};

KShapeShadowCommand::KShapeShadowCommand(const QList<KShape*> &shapes, KShapeShadow *shadow,  QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes = shapes;
    // save old shadows
    foreach(KShape *shape, d->shapes) {
        d->addOldShadow(shape->shadow());
        d->addNewShadow(shadow);
    }

    setText(i18n("Set Shadow"));
}

KShapeShadowCommand::KShapeShadowCommand(const QList<KShape*> &shapes, const QList<KShapeShadow*> &shadows, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    Q_ASSERT(shapes.count() == shadows.count());

    d->shapes = shapes;

    // save old shadows
    foreach(KShape *shape, shapes)
        d->addOldShadow(shape->shadow());
    foreach(KShapeShadow * shadow, shadows)
        d->addNewShadow(shadow);

    setText(i18n("Set Shadow"));
}

KShapeShadowCommand::KShapeShadowCommand(KShape* shape, KShapeShadow *shadow, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes.append(shape);
    d->addNewShadow(shadow);
    d->addOldShadow(shape->shadow());

    setText(i18n("Set Shadow"));
}

KShapeShadowCommand::~KShapeShadowCommand()
{
    delete d;
}

void KShapeShadowCommand::redo()
{
    QUndoCommand::redo();
    int shapeCount = d->shapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        KShape *shape = d->shapes[i];
        shape->update();
        shape->setShadow(d->newShadows[i]);
        shape->update();
    }
}

void KShapeShadowCommand::undo()
{
    QUndoCommand::undo();
    int shapeCount = d->shapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        KShape *shape = d->shapes[i];
        shape->update();
        shape->setShadow(d->oldShadows[i]);
        shape->update();
    }
}
