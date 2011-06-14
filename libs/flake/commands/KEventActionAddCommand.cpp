/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KEventActionAddCommand.h"
#include <klocale.h>

#include "KoShape.h"
#include "KEventAction.h"

class KEventActionAddCommandPrivate
{
public:
    KEventActionAddCommandPrivate(KoShape *s, KEventAction *a)
        : shape(s), eventAction(a), deleteEventAction(true)
    {
    }

    ~KEventActionAddCommandPrivate() {
        if (deleteEventAction)
            delete eventAction;
    }
    KoShape *shape;
    KEventAction *eventAction;
    bool deleteEventAction;
};


KEventActionAddCommand::KEventActionAddCommand(KoShape *shape, KEventAction *eventAction, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KEventActionAddCommandPrivate(shape, eventAction))
{
}

KEventActionAddCommand::~KEventActionAddCommand()
{
    delete d;
}

void KEventActionAddCommand::redo()
{
    d->shape->addEventAction(d->eventAction);
    d->deleteEventAction = false;
}

void KEventActionAddCommand::undo()
{
    d->shape->removeEventAction(d->eventAction);
    d->deleteEventAction = true;
}