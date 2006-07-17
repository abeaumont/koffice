/*
 * KoShapeRegistry.h -- Part of KOffice
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstaticdeleter.h>

#include <KoShapeRegistry.h>
#include <KoRectangleShapeFactory.h>
#include <KoPathShapeFactory.h>

#include <QString>

KoShapeRegistry::KoShapeRegistry()
{
}

void KoShapeRegistry::init() {
    const KService::List offers = KServiceTypeTrader::self()->query(
        QString::fromLatin1("KOffice/Shape"),
        QString::fromLatin1("(Type == 'Service') and ([X-Flake-Version] == 1)"));
    kDebug(30008) << "KoShapeRegistry searching for plugins, " << offers.count() << " found\n";

    foreach(KService::Ptr service, offers) {
        int errCode = 0;
        KoShapeFactory* plugin =
            KService::createInstance<KoShapeFactory>(
                service, this, QStringList(), &errCode );
        if ( plugin ) {
            kDebug(30008) <<"found plugin '"<< service->name() << "'\n";
            add(plugin);
        }
        else
            kWarning(30008) <<"loading plugin '" << service->name() <<
                "' failed, "<< KLibLoader::errorString( errCode ) << " ("<< errCode << ")\n";
    }

    // Also add our hard-coded dumb test shapes
    add( new KoRectangleShapeFactory(this, QStringList()) );
    add( new KoPathShapeFactory(this, QStringList()) );
}


KoShapeRegistry::~KoShapeRegistry()
{
}

KoShapeRegistry *KoShapeRegistry::m_singleton = 0;
static KStaticDeleter<KoShapeRegistry> staticShapeRegistryDeleter;

KoShapeRegistry* KoShapeRegistry::instance()
{
    if(KoShapeRegistry::m_singleton == 0)
    {
        staticShapeRegistryDeleter.setObject(m_singleton, new KoShapeRegistry());
        KoShapeRegistry::m_singleton->init();
    }
    return KoShapeRegistry::m_singleton;
}

#include "KoShapeRegistry.moc"
