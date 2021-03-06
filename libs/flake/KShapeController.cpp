/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KShapeController.h"
#include "KShapeControllerBase.h"
#include "KShapeRegistry.h"
#include "KShapeManager.h"
#include "KShapeLayer.h"
#include "KShapeSelection.h"
#include "commands/KShapeCreateCommand.h"
#include "commands/KShapeDeleteCommand.h"
#include "KCanvasBase.h"
#include "KShapeConfigWidgetBase.h"
#include "KShapeFactoryBase.h"
#include "KShape.h"

#include <kpagedialog.h>
#include <klocale.h>

class KShapeController::Private
{
public:
    Private()
        : canvas(0),
        shapeController(0),
        dummyRm(0)
    {
    }

    ~Private()
    {
        delete dummyRm;
    }

    KCanvasBase *canvas;
    KShapeControllerBase *shapeController;
    KResourceManager *dummyRm; // only used when there is no shapeController

    QUndoCommand* addShape(KShape *shape, QUndoCommand *parent) {
        Q_ASSERT(canvas->shapeManager());
        // set the active layer as parent if there is not yet a parent.
        if (!shape->parent()) {
            shape->setParent(canvas->shapeManager()->selection()->activeLayer());
        }

        return new KShapeCreateCommand(shapeController, shape, parent);
    }
};

KShapeController::KShapeController(KCanvasBase *canvas, KShapeControllerBase *shapeController)
        : d(new Private())
{
    d->canvas = canvas;
    d->shapeController = shapeController;
}

KShapeController::~KShapeController()
{
    delete d;
}

QUndoCommand* KShapeController::addShape(KShape *shape, QUndoCommand *parent)
{
    return d->addShape(shape, parent);
}

QUndoCommand* KShapeController::removeShape(KShape *shape, QUndoCommand *parent)
{
    return new KShapeDeleteCommand(d->shapeController, shape, parent);
}

QUndoCommand* KShapeController::removeShapes(const QList<KShape*> &shapes, QUndoCommand *parent)
{
    return new KShapeDeleteCommand(d->shapeController, shapes, parent);
}

void KShapeController::setShapeControllerBase(KShapeControllerBase* shapeControllerBase)
{
    d->shapeController = shapeControllerBase;
}

KResourceManager *KShapeController::resourceManager() const
{
    if (!d->shapeController) {
        if (!d->dummyRm) {
            d->dummyRm = new KResourceManager();
            KShapeRegistry *registry = KShapeRegistry::instance();
	    for (KGenericRegistry<KShapeFactoryBase*>::const_iterator it = registry->constBegin(); 
			    it != registry->constEnd(); ++it) {
                KShapeFactoryBase *shapeFactory = it.value();
                shapeFactory->newDocumentResourceManager(d->dummyRm);
            }
        }
        return d->dummyRm;
    }
    return d->shapeController->resourceManager();
}
