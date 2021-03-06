/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KSHAPEBORDERREGISTRY_H
#define KSHAPEBORDERREGISTRY_H

#include "flake_export.h"

#include <KGenericRegistry.h>
#include <KShapeBorderFactoryBase.h>

#include <QObject>

class FLAKE_EXPORT KShapeBorderRegistry : public QObject,  public KGenericRegistry<KShapeBorderFactoryBase*>
{
    Q_OBJECT

public:
    ~KShapeBorderRegistry();

    /**
     * Return an instance of the KShapeBorderRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KShapeBorderRegistry *instance();

private:
    KShapeBorderRegistry();
    KShapeBorderRegistry(const KShapeBorderRegistry&);
    KShapeBorderRegistry operator=(const KShapeBorderRegistry&);
    void init();

    class Private;
    Private *d;
};

#endif
