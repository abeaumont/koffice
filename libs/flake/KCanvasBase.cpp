/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KCanvasBase.h"
#include "KResourceManager.h"
#include "KShapeController.h"
#include "KCanvasController.h"
#include "KViewConverter.h"
#include "KSnapGuide.h"
#include "SnapGuideConfigWidget_p.h"

#include <KGlobal>
#include <KConfigGroup>
#include <KSharedPtr>
#include <KSharedConfig>

class KCanvasBase::Private
{
public:
    Private() : shapeController(0),
        resourceManager(0),
        controller(0),
        snapGuide(0),
        readWrite(true)
    {
    }
    ~Private() {
        delete shapeController;
        delete resourceManager;
        delete snapGuide;
    }
    KShapeController *shapeController;
    KResourceManager *resourceManager;
    KCanvasController *controller;
    KSnapGuide *snapGuide;
    bool readWrite;
};

KCanvasBase::KCanvasBase(KShapeControllerBase *shapeControllerBase)
        : d(new Private())
{
    d->resourceManager = new KResourceManager();
    d->shapeController = new KShapeController(this, shapeControllerBase);
    d->snapGuide = new KSnapGuide(this);
}

KCanvasBase::~KCanvasBase()
{
    delete d;
}


KShapeController *KCanvasBase::shapeController() const
{
    return d->shapeController;
}

KResourceManager *KCanvasBase::resourceManager() const
{
    return d->resourceManager;
}

void KCanvasBase::ensureVisible(const QRectF &rect)
{
    if (d->controller && d->controller->canvas())
        d->controller->ensureVisible(
                d->controller->canvas()->viewConverter()->documentToView(rect));
}

void KCanvasBase::setCanvasController(KCanvasController *controller)
{
    d->controller = controller;
}

KCanvasController *KCanvasBase::canvasController() const
{
    return d->controller;
}

void KCanvasBase::clipToDocument(const KShape *, QPointF &) const
{
}

KSnapGuide * KCanvasBase::snapGuide() const
{
    return d->snapGuide;
}

KGuidesData * KCanvasBase::guidesData()
{
    return 0;
}

QWidget *KCanvasBase::createSnapGuideConfigWidget() const
{
    return new SnapGuideConfigWidget(d->snapGuide);
}

void KCanvasBase::setReadWrite(bool readWrite)
{
    d->readWrite = readWrite;
}

bool KCanvasBase::isReadWrite() const
{
    return d->readWrite;
}
