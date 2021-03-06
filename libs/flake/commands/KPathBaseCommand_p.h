/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPATHBASECOMMAND_H
#define KPATHBASECOMMAND_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QUndoCommand>
#include <QSet>

class KPathShape;

/// the base command for commands altering a path shape
class KPathBaseCommand : public QUndoCommand
{
public:
    /**
     * @param parent the parent command used for macro commands
     */
    explicit KPathBaseCommand(QUndoCommand *parent = 0);

    /** initialize the base command with a single shape
     * @param parent the parent command used for macro commands
     */
    explicit KPathBaseCommand(KPathShape *shape, QUndoCommand *parent = 0);

protected:
    /**
     * Shedules repainting of all shapes control point rects.
     * @param normalizeShapes controls if paths are normalized before painting
     */
    void repaint(bool normalizeShapes);

    QSet<KPathShape*> m_shapes; ///< the shapes the command operates on
};

#endif // KPATHBASECOMMAND_H
