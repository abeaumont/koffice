/* This file is part of the KDE project
 * Copyright (C) 2007 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "KShapeKeepAspectRatioCommand.h"

#include <KLocale>

#include <KShape.h>

class KShapeKeepAspectRatioCommandPrivate
{
public:
    QList<KShape*> shapes;
    QList<bool> oldKeepAspectRatio;
    QList<bool> newKeepAspectRatio;
};


KShapeKeepAspectRatioCommand::KShapeKeepAspectRatioCommand(const QList<KShape*>& shapes,
        const QList<bool>& oldKeepAspectRatio,
        const QList<bool>& newKeepAspectRatio,
        QUndoCommand* parent)
        : QUndoCommand(i18n("Keep Aspect Ratio"), parent),
        d(new KShapeKeepAspectRatioCommandPrivate())
{
    d->shapes = shapes;
    d->oldKeepAspectRatio = oldKeepAspectRatio;
    d->newKeepAspectRatio = newKeepAspectRatio;
}

KShapeKeepAspectRatioCommand::~KShapeKeepAspectRatioCommand()
{
}

void KShapeKeepAspectRatioCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < d->shapes.count(); ++i) {
        d->shapes[i]->setKeepAspectRatio(d->newKeepAspectRatio[i]);
    }
}

void KShapeKeepAspectRatioCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->shapes.count(); ++i) {
        d->shapes[i]->setKeepAspectRatio(d->oldKeepAspectRatio[i]);
    }
}
