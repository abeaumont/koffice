/* This file is part of the KDE project

   Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>

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

#include "KShapeControllerBase.h"
#include "KResourceManager.h"
#include "KShapeRegistry.h"

class KShapeControllerBasePrivate
{
public:
    KShapeControllerBasePrivate()
        : resourceManager(new KResourceManager())
    {
        KShapeRegistry *registry = KShapeRegistry::instance();
	for (KGenericRegistry<KShapeFactoryBase*>::const_iterator it = registry->constBegin(); 
			it != registry->constEnd(); ++it) {
            KShapeFactoryBase *shapeFactory = it.value();
            shapeFactory->newDocumentResourceManager(resourceManager);
        }
    }

    ~KShapeControllerBasePrivate()
    {
        delete resourceManager;
    }

    KResourceManager *resourceManager;
};

KShapeControllerBase::KShapeControllerBase()
    : d(new KShapeControllerBasePrivate())
{
}

KShapeControllerBase::~KShapeControllerBase()
{
    delete d;
}

KResourceManager *KShapeControllerBase::resourceManager() const
{
    return d->resourceManager;
}
