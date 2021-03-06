/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "KToolRegistry.h"
#include <KGlobal>

#include "tools/KCreatePathToolFactory_p.h"
#include "tools/KCreateShapesToolFactory_p.h"
#include "tools/KCreateShapesTool.h"
#include "tools/KPathToolFactory_p.h"
#include "tools/KZoomTool_p.h"
#include "tools/KZoomToolFactory_p.h"
#include "tools/KPanTool_p.h"
#include "tools/KPanToolFactory_p.h"

#include <KoPluginLoader.h>

KToolRegistry::KToolRegistry()
{
}

void KToolRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Flake"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);
    config.whiteList = "ToolPlugins";
    config.blacklist = "ToolPluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Tool"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);

    // register generic tools
    add(new KCreatePathToolFactory(this));
    add(new KCreateShapesToolFactory(this));
    add(new KPathToolFactory(this));
    add(new KZoomToolFactory(this));
    add(new KPanToolFactory(this));
}

KToolRegistry::~KToolRegistry()
{
}

KToolRegistry* KToolRegistry::instance()
{
    K_GLOBAL_STATIC(KToolRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

#include <KToolRegistry.moc>
