/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#ifndef KCUTCONTROLLER_H
#define KCUTCONTROLLER_H

#include "KCopyController.h"
#include "flake_export.h"

/**
 * This class takes care of the cut actions integration into flake.
 * Whenever the copy (KStandardAction::Cut) action is triggered the controller
 * will use the currently selected tool and try to cut to the clipboard using that tool.
 * Additionally; when the tool does not allow copying (KToolBase::hasSelection() returns false)
 * the signal copyRequested will be emitted for applications to connect to.
 */
class FLAKE_EXPORT KCutController : public KCopyController
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param canvas the canvas this controller will work on.
     * @param cutAction the action that we will listen to and respond to when it is activated.  Additionally, the
     *     action is used as a parent for the QObject for memory management purposes.
     */
    KCutController(KCanvasBase *canvas, QAction *cutAction);
};

#endif
