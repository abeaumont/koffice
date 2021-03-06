/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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

#ifndef KCANVASOBSERVERBASE_H
#define KCANVASOBSERVERBASE_H

class KCanvasBase;

#include "flake_export.h"

/**
 * An abstract canvas observer interface class.
 * Dockers that want to be notified of active canvas changes
 * should implement that interface so that the tool controller
 * can give them the active canvas.
 */
class FLAKE_EXPORT KCanvasObserverBase
{
public:
    KCanvasObserverBase();
    virtual ~KCanvasObserverBase();
    virtual void setCanvas(KCanvasBase *canvas) = 0;
    virtual void clearCanvas();
};

#endif
