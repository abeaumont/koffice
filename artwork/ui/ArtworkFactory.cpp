/* This file is part of the KDE project
   Copyright (C) 2001-2003 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002,2004-2005,2007 David Faure <faure@kde.org>
   Copyright (C) 2002 Benoît Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2003 Lukáš Tinkl <lukas@kde.org>
   Copyright (C) 2004,2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005,2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
   Copyright (C) 2007 Stephan Kulow <coolo@kde.org>

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

#include "ArtworkFactory.h"
#include "ArtworkPart.h"
#include "ArtworkAboutData.h"

#include <kaboutdata.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>
#include <kparts/plugin.h>

#include <kdebug.h>

KComponentData* ArtworkFactory::s_instance = 0L;
KAboutData* ArtworkFactory::s_aboutData = 0L;

ArtworkFactory::ArtworkFactory(QObject* parent)
        : KPluginFactory(*aboutData(), parent)
{
    componentData();
}

ArtworkFactory::~ArtworkFactory()
{
    delete s_instance;
    s_instance = 0L;
    delete s_aboutData;
    s_aboutData = 0L;
}

QObject* ArtworkFactory::create(const char* iface, QWidget* parentWidget, QObject *parent, const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(args);
    Q_UNUSED(keyword);

    // If classname is "KoDocument", our host is a koffice application
    // otherwise, the host wants us as a simple part, so switch to readonly and
    // single view.
    bool bWantKoDocument = (strcmp(iface, "KoDocument") == 0);

    // parentWidget and widgetName are used by KoDocument for the
    // "readonly+singleView" case.
    ArtworkPart* part = new ArtworkPart(parentWidget, 0, parent, 0, !bWantKoDocument);

    if (!bWantKoDocument)
        part->setReadWrite(false);

    return part;
}

KAboutData * ArtworkFactory::aboutData()
{
    if (!s_aboutData)
        s_aboutData = newArtworkAboutData();
    return s_aboutData;
}

const KComponentData &ArtworkFactory::componentData()
{
    if (!s_instance) {
        s_instance = new KComponentData(aboutData());
        // Add any application-specific resource directories here

        s_instance->dirs()->addResourceType("kis_brushes", "data", "krita/brushes/");
        s_instance->dirs()->addResourceType("kis_pattern", "data", "krita/patterns/");
        s_instance->dirs()->addResourceType("artwork_gradient", "data", "krita/gradients/");
        s_instance->dirs()->addResourceType("artwork_clipart", "data", "artwork/cliparts/");
        s_instance->dirs()->addResourceType("artwork_template", "data", "artwork/templates/");
        s_instance->dirs()->addResourceType("artwork_effects", "data", "artwork/effects/");
        // Tell the iconloader about share/apps/koffice/icons
        KIconLoader::global()->addAppDir("koffice");
    }

    return *s_instance;
}

#include "ArtworkFactory.moc"

