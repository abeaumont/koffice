/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#ifndef KPRSHAREDLOADINGDATA_H
#define KPRSHAREDLOADINGDATA_H

#include <KoSharedLoadingData.h>

#include "kpresenter_export.h"

#define KPRESENTER_SHARED_LOADING_ID "KPresenterSharedLoadingId"

class KPrSoundCollection;

class KPRESENTER_EXPORT KPrSharedLoadingData : public KoSharedLoadingData
{
public:
    KPrSharedLoadingData();
    virtual ~KPrSharedLoadingData();

    void setSoundCollection( KPrSoundCollection * soundCollection );
    KPrSoundCollection * soundCollection();

private:
    KPrSoundCollection * m_soundCollection;
};

#endif /* KPRSHAREDLOADINGDATA_H */
